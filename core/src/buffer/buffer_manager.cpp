#include "buffer_manager.h"
#include <zip.h>
#include <fstream>
#include <iostream>
namespace sdgs {
    BufferManager::BufferManager() {
        std::filesystem::create_directories(baseDir_);
    }
    BufferManager::~BufferManager() {
        clearAll();
    }
    BufferManager& BufferManager::instance() {
        static BufferManager inst; return inst;
    }
    void BufferManager::setBaseDir(const std::string& dir) {
        baseDir_ = dir;
    }
    void BufferManager::clearAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& p : entries_) removeDirectory(p.second.extractedPath);
        entries_.clear(); 
        try {
            std::filesystem::remove_all(baseDir_);
        } catch (...) {
        }
    }
    void BufferManager::cleanUp() {
        /* stub */
    }
    void BufferManager::releaseResource(const std::string& archivePath) {
        /* stub */
    }
    std::string BufferManager::extractArchive(const std::string& archivePath, bool recursive) {
        return "extracted/" + archivePath;
    }
    void BufferManager::extractZip(const std::string& archivePath, const std::string& destDir) {
        /* stub */
    }
    void BufferManager::removeDirectory(const std::filesystem::path& path) {
        try {
            std::filesystem::remove_all(path);
        } catch (...) {
        }
    }
    size_t BufferManager::getDirectorySize(const std::filesystem::path& path) {
        return 0;
    }
    std::string BufferManager::getHash(const std::string& path) {
        return std::to_string(std::hash<std::string>{}(path));
    }
    bool BufferManager::isArchiveUpToDate(const std::string& archivePath, const std::string& extractedPath) {
        return true;
    }
}