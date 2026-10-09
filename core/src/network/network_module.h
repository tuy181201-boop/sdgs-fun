#pragma once
#include <string>
#include <atomic>
#include <memory>
#include <thread>

namespace sdgs {

class NetworkModule {
public:
    NetworkModule();
    ~NetworkModule();

    bool init(const std::string& worldPath);
    void update();
    void shutdown();

    // Отправить сообщение всем подключённым клиентам
    void broadcast(const std::string& message);

    // Отправить бинарные данные (воксели, состояние)
    void broadcast_binary(const void* data, size_t size);

private:
    struct Impl;                    // PIMPL — скрываем Boost.Beast из заголовка
    std::unique_ptr<Impl> impl_;
    std::atomic<bool> running_{false};
    std::thread server_thread_;
};

} // namespace sdgs
