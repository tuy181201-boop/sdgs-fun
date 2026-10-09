#include "python_bridge.h"
#include <boost/process.hpp>
#include <iostream>

namespace bp = boost::process;
using boost::asio::ip::tcp;
using json = nlohmann::json;

namespace sdgs {

PythonBridge::PythonBridge() = default;
PythonBridge::~PythonBridge() { shutdown(); }

bool PythonBridge::init(const std::string& worldPath) {
    // Путь к системному Python из вашего venv
    pythonExe_ = "C:\\venvs\\llm_nlp_env\\Scripts\\python.exe";
    scriptPath_ = worldPath + "/python/ai_server.py";

    try {
        launchPythonProcess();
        // Даём Python время на запуск сервера
        std::this_thread::sleep_for(std::chrono::seconds(2));
        connect();
        running_ = true;
        std::cout << "[PythonBridge] AI Core подключён\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[PythonBridge] Ошибка init: " << e.what() << "\n";
        return false;
    }
}

void PythonBridge::launchPythonProcess() {
    // Запускаем Python как отдельный процесс
    bp::child c(
        bp::search_path("cmd"),
        bp::args = {"/c", "start", "/B", pythonExe_, scriptPath_},
        bp::std_out > bp::null,
        bp::std_err > bp::null
    );
    c.detach(); // Отсоединяем — Python живёт своей жизнью
}

void PythonBridge::connect() {
    socket_ = std::make_unique<tcp::socket>(io_);
    tcp::endpoint ep(boost::asio::ip::make_address("127.0.0.1"), 5555);
    
    // Пытаемся подключиться с retry
    for (int i = 0; i < 10; ++i) {
        boost::system::error_code ec;
        socket_->connect(ep, ec);
        if (!ec) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    throw std::runtime_error("Не удалось подключиться к AI Core");
}

json PythonBridge::request(const std::string& method, const json& params) {
    json req = {
        {"id", nextRequestId_++},
        {"method", method},
        {"params", params}
    };
    
    std::string reqStr = req.dump();
    uint32_t len = static_cast<uint32_t>(reqStr.size());
    
    // Отправляем: 4 байта длины + само сообщение
    boost::asio::write(*socket_, boost::asio::buffer(&len, 4));
    boost::asio::write(*socket_, boost::asio::buffer(reqStr));
    
    // Читаем ответ
    uint32_t respLen = 0;
    boost::asio::read(*socket_, boost::asio::buffer(&respLen, 4));
    
    std::string respStr(respLen, '\0');
    boost::asio::read(*socket_, boost::asio::buffer(respStr));
    
    return json::parse(respStr);
}

void PythonBridge::update() {
    if (!running_) return;
    // Здесь можно вызывать AI периодически
    // Например, каждые N кадров — предсказание поведения NPC
}

void PythonBridge::shutdown() {
    running_ = false;
    if (socket_) {
        boost::system::error_code ec;
        socket_->close(ec);
    }
}

} // namespace sdgs
