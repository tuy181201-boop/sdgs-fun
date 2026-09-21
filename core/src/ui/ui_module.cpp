#include "ui_module.h"
#include <iostream>
namespace sdgs {
void UIModule::init(const std::string& worldPath)
{
	std::cout << "[UI] Initialized" << std::endl;
}
void UIModule::update()
{
}
void UIModule::shutdown()
{
	std::cout << "[UI] Shutdown" << std::endl;
}
}