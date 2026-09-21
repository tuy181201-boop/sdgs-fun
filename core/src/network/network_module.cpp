#include "network_module.h"
#include <iostream>
namespace sdgs {
void NetworkModule::init(const std::string& worldPath)
{
	std::cout << "[Network] Initialized" << std::endl;
}
void NetworkModule::update()
{
}
void NetworkModule::shutdown()
{
	std::cout << "[Network] Shutdown" << std::endl;
}
}