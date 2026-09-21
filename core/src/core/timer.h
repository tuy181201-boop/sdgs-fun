#pragma once
#include <chrono>
#include <thread>
class HighResTimer
{
public:
	HighResTimer(double freq_hz = 60.0)
	: period_(std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::duration<double>(1.0 / freq_hz)))
	{
		start_ = std::chrono::steady_clock::now();
		counter_ = 0;
	}
	void wait()
	{
		auto now = std::chrono::steady_clock::now();
		auto next = start_ + period_ * (++counter_);
		std::this_thread::sleep_until(next);
	}
	void reset()
	{
		start_ = std::chrono::steady_clock::now();
		counter_ = 0;
	}
private:
	std::chrono::microseconds period_;
	std::chrono::time_point<std::chrono::steady_clock> start_;
	uint64_t counter_ = 0;
};