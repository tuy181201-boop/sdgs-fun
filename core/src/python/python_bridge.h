#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>

namespace sdgs {

class PythonBridge {
public:
    PythonBridge();
    ~PythonBridge();

    bool init(const std::string& worldPath);
    void update();
    void shutdown();

    // Синхронный запрос к AI Core
    nlohmann::json request(const std::string& method, const nlohmann::json& params);

private:
    void launchPythonProcess();
    void connect();

    boost::asio::io_context io_;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
    std::string pythonExe_;
    std::string scriptPath_;
    int nextRequestId_ = 1;
    bool running_ = false;
};

} // namespace sdgs
