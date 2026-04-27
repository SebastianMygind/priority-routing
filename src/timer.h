#pragma once
#include <chrono>

class Timer {
public:
    // The frequency in milliseconds.
    explicit Timer(const int frequency) {

        const std::chrono::duration<double> period_sec{1.0 / frequency};
        nsPerTick = std::chrono::duration_cast<std::chrono::nanoseconds>(period_sec);

        lastTick = std::chrono::steady_clock::now();
    };

    bool IsActive();

private:
    std::chrono::nanoseconds nsPerTick{};
    std::chrono::steady_clock::time_point lastTick;
};
