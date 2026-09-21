#pragma once
#include <string>
#include <nlohmann/json.hpp>
namespace sdgs {
class LifeModule
{
public:
	void init(const std::string& worldPath, const nlohmann::json& config);
	void update(float dt);
	void shutdown();
};
}