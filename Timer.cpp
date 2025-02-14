#include "Timer.h"

using namespace std::chrono;

Timer::Timer() {
	last = steady_clock::now();
}

float Timer::Mark() {													//Will give you time elapsed since last call
	const auto old = last;
	last = steady_clock::now();
	const duration<float> frameTime = last - old;
	return frameTime.count();
}

float Timer::Peek() {													//Will give you time elapsed since last call of Mark without resetting the mark point
	return duration<float>(steady_clock::now() - last).count();
}