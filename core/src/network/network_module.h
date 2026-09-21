#pragma once
#include <string>
namespace sdgs {
class NetworkModule
{
public:
	void init(const std::string& worldPath);
	void update();
	void shutdown();
};
}