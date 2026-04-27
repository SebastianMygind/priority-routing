#include "timer.h"

bool Timer::IsActive() {

    const auto timeThreshold = lastTick + nsPerTick;

    const auto curretTime = std::chrono::high_resolution_clock::now();

    if (curretTime >= timeThreshold) {
        lastTick = curretTime;
        return true;
    }

    return false;
}