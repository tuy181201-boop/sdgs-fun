#pragma once
#include <string>
#include <nlohmann/json.hpp>
namespace sdgs {
class ScriptHandler
{
public:
	virtual ~ScriptHandler() = default;
	virtual void execute(const std::string& code, const nlohmann::json& context) = 0;
};
}