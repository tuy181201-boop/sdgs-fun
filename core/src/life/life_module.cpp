#include "life_module.h"
#include <iostream>
namespace sdgs {
void LifeModule::init(const std::string& worldPath, const nlohmann::json& config)
{
	std::cout << "[Life] Initialized" << std::endl;
}
void LifeModule::update(float dt)
{
}
void LifeModule::shutdown()
{
	std::cout << "[Life] Shutdown" << std::endl;
}
}