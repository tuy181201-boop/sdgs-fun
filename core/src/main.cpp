#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <csignal>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "core/timer.h"
#include "buffer/buffer_manager.h"
#include "land/land_module.h"
#include "life/life_module.h"
#include "mind/mind_module.h"
#include "network/network_module.h"
#include "ui/ui_module.h"
#include "script/script_manager.h"
#include "python/python_bridge.h"

using namespace sdgs;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

// ============================================================
//  Логирование
// ============================================================
enum class LogLevel { Debug = 0, Info = 1, Warn = 2, Error = 3 };

class Logger {
public:
    static void init(LogLevel lvl, const std::string& path = "") {
        level_ = lvl;
        if (!path.empty()) file_.open(path, std::ios::app);
    }
    static void log(LogLevel lvl, const std::string& msg) {
        if (static_cast<int>(lvl) < static_cast<int>(level_)) return;
        std::lock_guard<std::mutex> lk(mtx_);
        auto now = std::chrono::system_clock::now();
        auto t   = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        static const char* names[] = {"DBG", "INF", "WRN", "ERR"};
        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S")
            << " [" << names[static_cast<int>(lvl)] << "] " << msg;
        std::cout << oss.str() << std::endl;
        if (file_.is_open()) file_ << oss.str() << std::endl;
    }
    static void debug(const std::string& m) { log(LogLevel::Debug, m); }
    static void info (const std::string& m) { log(LogLevel::Info,  m); }
    static void warn (const std::string& m) { log(LogLevel::Warn,  m); }
    static void error(const std::string& m) { log(LogLevel::Error, m); }

private:
    static std::mutex         mtx_;
    static LogLevel           level_;
    static std::ofstream      file_;
};
std::mutex    Logger::mtx_;
LogLevel      Logger::level_ = LogLevel::Info;
std::ofstream Logger::file_;

// ============================================================
//  Конфигурация приложения
// ============================================================
struct AppConfig {
    std::string worldPath         = "worlds/default";
    double      tickRate          = 60.0;
    int         pythonEveryNFrames = 10;
    bool        headless          = false;
    bool        enableNetwork     = true;
    bool        enableUI          = true;
    bool        enablePython      = true;
    LogLevel    logLevel          = LogLevel::Info;
};

// ============================================================
//  Статистика
// ============================================================
struct FrameStats {
    std::atomic<uint64_t> totalFrames{0};
    std::atomic<double>   lastFrameTime{0.0};
    std::atomic<double>   avgFrameTime{0.0};
    std::atomic<double>   maxFrameTime{0.0};
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

    void record(double ms) {
        uint64_t n = ++totalFrames;
        lastFrameTime = ms;
        double prev = avgFrameTime.load();
        avgFrameTime = prev + (ms - prev) / static_cast<double>(n);
        if (ms > maxFrameTime.load()) maxFrameTime = ms;
    }
    double fps() const {
        auto sec = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - startTime).count();
        return sec > 0 ? static_cast<double>(totalFrames.load()) / sec : 0.0;
    }
};

// ============================================================
//  Глобальные объекты
// ============================================================
BufferManager bufferManager;
LandModule    land;
LifeModule    life;
MindModule    mind;
NetworkModule network;
UIModule      ui;
ScriptManager scriptManager;
PythonBridge  pythonBridge;

std::atomic<bool> running{true};
std::atomic<bool> paused{false};
std::jthread      gameLoopThread;
FrameStats        stats;

// ============================================================
//  Обработка сигналов
// ============================================================
void signalHandler(int /*sig*/) {
    running = false;
}

// ============================================================
//  Игровой цикл
// ============================================================
void gameLoop(const AppConfig& cfg) {
    HighResTimer timer(cfg.tickRate);
    timer.reset();
    stats.startTime = std::chrono::steady_clock::now();

    const float fixedDt = 1.0f / static_cast<float>(cfg.tickRate);
    int  frameCounter   = 0;
    auto lastStatsPrint = std::chrono::steady_clock::now();

    Logger::info("Game loop started @ " + std::to_string(cfg.tickRate) + " Hz");

    while (running) {
        timer.wait();

        if (paused.load()) {
            std::this_thread::sleep_for(1ms);
            continue;
        }

        auto frameStart = std::chrono::steady_clock::now();
        try {
            mind.update(fixedDt);
            life.update(fixedDt);
            land.update(fixedDt);

            if (cfg.enablePython &&
                (++frameCounter % cfg.pythonEveryNFrames == 0)) {
                pythonBridge.update();
            }
            if (cfg.enableNetwork) network.update();
            if (cfg.enableUI)      ui.update();
        }
        catch (const std::exception& e) {
            Logger::error(std::string("Exception in loop: ") + e.what());
        }
        catch (...) {
            Logger::error("Unknown exception in loop");
        }

        double ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - frameStart).count();
        stats.record(ms);

        auto now = std::chrono::steady_clock::now();
        if (now - lastStatsPrint >= 5s) {
            std::ostringstream oss;
            oss << "FPS=" << std::fixed << std::setprecision(1) << stats.fps()
                << " avg=" << std::setprecision(2) << stats.avgFrameTime.load() << "ms"
                << " max=" << stats.maxFrameTime.load() << "ms"
                << " frames=" << stats.totalFrames.load();
            Logger::info(oss.str());
            lastStatsPrint = now;
        }
    }

    Logger::info("Game loop stopped. Total frames: " +
                 std::to_string(stats.totalFrames.load()));
}

// ============================================================
//  Аргументы командной строки
// ============================================================
static void printHelp(const char* prog) {
    std::cout <<
        "Usage: " << prog << " [options] [world_path]\n"
        "Options:\n"
        "  -w, --world <path>       World directory (default: worlds/default)\n"
        "  -r, --rate  <hz>         Tick rate in Hz (default: 60)\n"
        "  -p, --python-every <n>   Python bridge every N frames (default: 10)\n"
        "      --headless           Disable UI module\n"
        "      --no-network         Disable network module\n"
        "      --no-python          Disable Python bridge\n"
        "  -l, --log   <level>      debug|info|warn|error (default: info)\n"
        "  -h, --help               Show this help\n";
}

static bool parseArgs(int argc, char* argv[], AppConfig& cfg) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto needNext = [&](const char* name) -> std::string {
            if (i + 1 >= argc)
                throw std::runtime_error(std::string("Missing value for ") + name);
            return argv[++i];
        };
        try {
            if (a == "-h" || a == "--help") { printHelp(argv[0]); return false; }
            else if (a == "-w" || a == "--world")        cfg.worldPath = needNext("--world");
            else if (a == "-r" || a == "--rate")         cfg.tickRate = std::stod(needNext("--rate"));
            else if (a == "-p" || a == "--python-every") cfg.pythonEveryNFrames = std::stoi(needNext("--python-every"));
            else if (a == "--headless")                  { cfg.headless = true; cfg.enableUI = false; }
            else if (a == "--no-network")                cfg.enableNetwork = false;
            else if (a == "--no-python")                 cfg.enablePython  = false;
            else if (a == "-l" || a == "--log") {
                std::string v = needNext("--log");
                if      (v == "debug") cfg.logLevel = LogLevel::Debug;
                else if (v == "info")  cfg.logLevel = LogLevel::Info;
                else if (v == "warn")  cfg.logLevel = LogLevel::Warn;
                else if (v == "error") cfg.logLevel = LogLevel::Error;
                else Logger::warn("Unknown log level: " + v);
            }
            else if (!a.empty() && a[0] != '-') cfg.worldPath = a;
            else { Logger::error("Unknown option: " + a); printHelp(argv[0]); return false; }
        } catch (const std::exception& e) {
            Logger::error(std::string("Arg parse error: ") + e.what());
            return false;
        }
    }
    if (cfg.tickRate <= 0.0 || cfg.tickRate > 1000.0) {
        Logger::error("Invalid tick rate (must be in (0, 1000])");
        return false;
    }
    if (cfg.pythonEveryNFrames < 1) {
        Logger::error("--python-every must be >= 1");
        return false;
    }
    return true;
}

// ============================================================
//  Сохранение состояния
// ============================================================
bool saveState(const std::string& path) {
    try {
        nlohmann::json j;
        j["timestamp_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        j["total_frames"] = stats.totalFrames.load();
        j["avg_frame_ms"] = stats.avgFrameTime.load();
        j["max_frame_ms"] = stats.maxFrameTime.load();

        std::ofstream f(path);
        if (!f) return false;
        f << j.dump(2);
        Logger::info("State saved to " + path);
        return true;
    } catch (const std::exception& e) {
        Logger::error(std::string("Save failed: ") + e.what());
        return false;
    }
}

// ============================================================
//  main
// ============================================================
int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "  SDGS Core v1.1 (Win7+, x86-64)"        << std::endl;
    std::cout << "========================================" << std::endl;

    AppConfig cfg;
    if (!parseArgs(argc, argv, cfg)) return 0;
    Logger::init(cfg.logLevel, "sdgs.log");

    // Обработка сигналов — на Ctrl+C завершаемся корректно.
    std::signal(SIGINT,  signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Проверка существования мира.
    fs::path worldPath = cfg.worldPath;
    if (!fs::exists(worldPath)) {
        Logger::error("World path does not exist: " + worldPath.string());
        return 1;
    }

    // Чтение конфига мира.
    nlohmann::json worldConfig;
    {
        fs::path cfgFile = worldPath / "config" / "world.lg";
        std::ifstream configFile(cfgFile);
        if (!configFile) {
            Logger::error("Config not found: " + cfgFile.string());
            return 1;
        }
        try {
            configFile >> worldConfig;
        } catch (const std::exception& e) {
            Logger::error(std::string("Config parse error: ") + e.what());
            return 1;
        }
    }
    Logger::info("Config loaded: " + cfg.worldPath);

    // Буфер.
    bufferManager.setBaseDir("buffer");
    bufferManager.clearAll();

    // Универсальный инициализатор с логированием и отловом исключений.
    auto initModule = [](const char* name, auto&& fn) -> bool {
        try {
            fn();
            Logger::info(std::string(name) + " module initialized");
            return true;
        } catch (const std::exception& e) {
            Logger::error(std::string(name) + " init failed: " + e.what());
            return false;
        }
    };

    if (!initModule("Land", [&]{ land.init(cfg.worldPath, worldConfig); })) return 1;
    if (!initModule("Life", [&]{ life.init(cfg.worldPath, worldConfig); })) return 1;
    if (!initModule("Mind", [&]{ mind.init(cfg.worldPath, worldConfig); })) return 1;
    if (cfg.enableNetwork && !initModule("Network", [&]{ network.init(cfg.worldPath); })) return 1;
    if (cfg.enableUI      && !initModule("UI",      [&]{ ui.init(cfg.worldPath); }))      return 1;

    // Скрипты.
    try {
        scriptManager.loadHandlers();
        scriptManager.loadAllScripts(cfg.worldPath + "/scripts");
        Logger::info("Scripts loaded");
    } catch (const std::exception& e) {
        Logger::warn(std::string("Script load issue: ") + e.what());
    }

    if (cfg.enablePython) {
        if (!initModule("Python", [&]{ pythonBridge.init(cfg.worldPath); })) return 1;
    }

    Logger::info("World initialized. Starting game loop...");

    // Запускаем цикл в отдельном потоке.
    gameLoopThread = std::jthread([&cfg]{ gameLoop(cfg); });

    // Консольные команды.
    Logger::info("Commands: [s]tats  [p]ause  [r]esume  [w]save  [q]uit");
    std::string line;
    while (running && std::getline(std::cin, line)) {
        if      (line == "q" || line == "quit"   || line == "exit") break;
        else if (line == "p" || line == "pause")  { paused = true;  Logger::info("Paused"); }
        else if (line == "r" || line == "resume") { paused = false; Logger::info("Resumed"); }
        else if (line == "w" || line == "save")   saveState("save.json");
        else if (line == "s" || line == "stats") {
            std::ostringstream oss;
            oss << "FPS=" << stats.fps()
                << " frames=" << stats.totalFrames.load()
                << " avg=" << stats.avgFrameTime.load() << "ms"
                << " max=" << stats.maxFrameTime.load() << "ms";
            Logger::info(oss.str());
        }
        else if (!line.empty()) {
            Logger::warn("Unknown command: " + line);
        }
    }

    Logger::info("Shutting down...");
    running = false;
    paused  = false;
    if (gameLoopThread.joinable()) gameLoopThread.join();

    saveState("save.json");
    bufferManager.clearAll();
    Logger::info("Goodbye!");
    return 0;
}