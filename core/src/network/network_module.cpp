#include "network_module.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <mutex>
#include <set>

namespace sdgs {

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

struct NetworkModule::Impl {
    net::io_context ioc;
    std::unique_ptr<tcp::acceptor> acceptor;
    std::mutex clients_mutex;
    std::set<std::shared_ptr<websocket::stream<tcp::socket>>> clients;
};

NetworkModule::NetworkModule() : impl_(std::make_unique<Impl>()) {}
NetworkModule::~NetworkModule() { shutdown(); }

bool NetworkModule::init(const std::string& worldPath) {
    try {
        tcp::endpoint endpoint(tcp::v4(), 8080);
        impl_->acceptor = std::make_unique<tcp::acceptor>(impl_->ioc, endpoint);
        running_ = true;

        server_thread_ = std::thread([this]() {
            std::cout << "[Network] WebSocket server on port 8080\n";
            while (running_) {
                try {
                    tcp::socket socket(impl_->ioc);
                    impl_->acceptor->accept(socket);
                    // Обработка в отдельном потоке
                    std::thread([this, s = std::move(socket)]() mutable {
                        handle_client(std::move(s));
                    }).detach();
                } catch (const std::exception& e) {
                    if (running_) std::cerr << "[Network] Accept error: " << e.what() << "\n";
                }
            }
        });

        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Network] Init failed: " << e.what() << "\n";
        return false;
    }
}

void NetworkModule::handle_client(tcp::socket socket) {
    try {
        auto ws = std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));
        ws->accept();

        {
            std::lock_guard<std::mutex> lock(impl_->clients_mutex);
            impl_->clients.insert(ws);
        }

        beast::flat_buffer buffer;
        while (running_) {
            ws->read(buffer);

            // Здесь — обработка входящего сообщения
            // Например, парсинг команд от игрока
            auto msg = beast::buffers_to_string(buffer.data());
            buffer.consume(buffer.size());

            // Пока просто эхо
            ws->text(ws->got_text());
            ws->write(boost::asio::buffer(msg));
        }
    } catch (const beast::error::closed&) {
        // Клиент отключился — нормально
    } catch (const std::exception& e) {
        std::cerr << "[Network] Client error: " << e.what() << "\n";
    }

    // Удаляем клиента
    // (в реальном коде нужно найти ws по указателю и удалить)
}

void NetworkModule::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(impl_->clients_mutex);
    for (auto& ws : impl_->clients) {
        try {
            ws->text(true);
            ws->write(boost::asio::buffer(message));
        } catch (...) {
            // Клиент мёртв — пометить на удаление
        }
    }
}

void NetworkModule::broadcast_binary(const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(impl_->clients_mutex);
    for (auto& ws : impl_->clients) {
        try {
            ws->binary(true);
            ws->write(boost::asio::buffer(data, size));
        } catch (...) {}
    }
}

void NetworkModule::update() {
    // Здесь можно вызывать broadcast() с состоянием мира
    // Но НЕ из игрового потока — лучше через очередь
}

void NetworkModule::shutdown() {
    if (!running_.exchange(false)) return;

    if (impl_->acceptor) {
        boost::system::error_code ec;
        impl_->acceptor->close(ec);
    }
    impl_->ioc.stop();

    if (server_thread_.joinable()) server_thread_.join();

    std::cout << "[Network] Shutdown\n";
}

} // namespace sdgs
