#include "mind_module.h"
#include <iostream>
namespace sdgs {
void MindModule::init(const std::string& worldPath, const nlohmann::json& config)
{
	std::cout << "[Mind] Initialized" << std::endl;
}
void MindModule::update(float dt)
{
}
void MindModule::shutdown()
{
	std::cout << "[Mind] Shutdown" << std::endl;
}
}