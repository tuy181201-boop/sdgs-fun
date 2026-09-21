#pragma once
#include <string>
namespace sdgs {
class UIModule
{
public:
	void init(const std::string& worldPath);
	void update();
	void shutdown();
};
}