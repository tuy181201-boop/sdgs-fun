#pragma once
#include <string>
#include <nlohmann/json.hpp>
namespace sdgs {
class PythonBridge
{
public:
	void init(const std::string& worldPath);
	void update();
	nlohmann::json callFunction(const std::string& funcName, const nlohmann::json& args);
};
}