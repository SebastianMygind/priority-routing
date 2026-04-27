#include "timer.h"

bool Timer::IsActive() {

    const auto timeThreshold = lastTick + nsPerTick;

    const auto curretTime = std::chrono::steady_clock::now();

    if (curretTime >= timeThreshold) {
        lastTick = curretTime;
        return true;
    }

    return false;
}