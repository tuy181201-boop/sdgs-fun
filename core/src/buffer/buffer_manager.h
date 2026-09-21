#pragma once
#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <filesystem>
namespace sdgs {
class BufferManager
{
public:
	static BufferManager& instance();
	std::string extractArchive(const std::string& archivePath, bool recursive = false);
	void releaseResource(const std::string& archivePath);
	void cleanUp();
	void clearAll();
	void setBaseDir(const std::string& dir);
private:
	BufferManager();
	~BufferManager();
	struct BufferEntry
	{
		std::string extractedPath;
		std::chrono::steady_clock::time_point lastAccess;
		int refCount = 0;
		size_t sizeBytes = 0;
	};
	std::string baseDir_ = "buffer";
	std::unordered_map<std::string, BufferEntry> entries_;
	mutable std::mutex mutex_;
	std::chrono::minutes expirationTime_ = std::chrono::minutes(10);
	size_t maxCacheSize_ = 1024 * 1024 * 1024;
	std::string getHash(const std::string& path);
	bool isArchiveUpToDate(const std::string& archivePath, const std::string& extractedPath);
	void extractZip(const std::string& archivePath, const std::string& destDir);
	void removeDirectory(const std::filesystem::path& path);
	size_t getDirectorySize(const std::filesystem::path& path);
};
}