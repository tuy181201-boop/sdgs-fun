#include "buffer_manager.h"
#include <fstream>
#include <filesystem>
namespace sdgs {
void BufferManager::extractZip(const std::string& archivePath, const std::string& destDir)
{
	try { std::filesystem::create_directories(destDir); } catch (...) {}
}
std::string BufferManager::extractArchive(const std::string& archivePath, bool recursive)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (entries_.find(archivePath) != entries_.end())
		return entries_[archivePath].extractedPath;
	std::string hash = getHash(archivePath);
	std::string destDir = baseDir_ + "/" + hash;
	std::filesystem::create_directories(destDir);
	std::ifstream src(archivePath, std::ios::binary);
	if (src)
	{
		std::string destFile = destDir + "/content.bin";
		std::ofstream dst(destFile, std::ios::binary);
		dst << src.rdbuf();
	}
	BufferEntry entry;
	entry.extractedPath = destDir;
	entry.lastAccess = std::chrono::steady_clock::now();
	entry.refCount = 1;
	entry.sizeBytes = 0;
	entries_[archivePath] = entry;
	return destDir;
}
}