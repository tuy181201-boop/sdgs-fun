#include "file_loader.h"
#include "buffer/buffer_manager.h"
#include <fstream>
#include <filesystem>
namespace sdgs {
std::string FileLoader::loadText(const std::string& path)
{
	std::string extracted = BufferManager::instance().extractArchive(path);
	std::string filePath = extracted + "/" + std::filesystem::path(path).filename().string();
	std::ifstream file(filePath);
	if (!file)
		throw std::runtime_error("Cannot open file: " + path);
	return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}
ScriptLanguage FileLoader::detectLanguage(const std::string& path, const std::string& content)
{
	std::string firstLine = content.substr(0, content.find('\n'));
	if (firstLine.find("SDGS:") != std::string::npos)
	{
		std::string lang = firstLine.substr(firstLine.find("SDGS:") + 5);
		lang.erase(remove_if(lang.begin(), lang.end(), ::isspace), lang.end());
		if (lang == "LISP") return ScriptLanguage::LISP;
		if (lang == "FORTRAN") return ScriptLanguage::FORTRAN;
	}
	std::string ext = std::filesystem::path(path).extension().string();
	if (ext == ".lsp") return ScriptLanguage::LISP;
	if (ext == ".f") return ScriptLanguage::FORTRAN;
	return ScriptLanguage::UNKNOWN;
}
void FileLoader::registerHandler(const std::string& ext, ScriptHandler* handler)
{
	handlers[ext] = handler;
}
bool FileLoader::loadAndExecute(const std::string& path, const nlohmann::json& context)
{
	std::string content = loadText(path);
	auto lang = detectLanguage(path, content);
	std::string ext = std::filesystem::path(path).extension().string();
	auto it = handlers.find(ext);
	if (it == handlers.end()) return false;
	it->second->execute(content, context);
	return true;
}
}