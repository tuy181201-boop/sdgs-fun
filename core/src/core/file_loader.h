#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include "../script/script_handler.h"
namespace sdgs {
class FileLoader
{
public:
	std::string loadText(const std::string& path);
	ScriptLanguage detectLanguage(const std::string& path, const std::string& content);
	void registerHandler(const std::string& ext, ScriptHandler* handler);
	bool loadAndExecute(const std::string& path, const nlohmann::json& context);
private:
	std::unordered_map<std::string, ScriptHandler*> handlers;
};
}