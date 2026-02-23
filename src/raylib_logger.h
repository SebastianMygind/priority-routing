//
// Created by seb on 23/02/2026.
//

#ifndef PRIORITY_ROUTING_RAYLIB_LOGGER_H
#define PRIORITY_ROUTING_RAYLIB_LOGGER_H

#include "raylib.h"
#include "spdlog/spdlog.h"

inline void SPDLogger(const int msgType, const char *msg, va_list args) {

    // 1. Determine the length of the formatted string
    va_list argsCopy;
    va_copy(argsCopy, args);
    const int size = std::vsnprintf(nullptr, 0, msg, argsCopy);
    va_end(argsCopy);

    if (size <= 0) return;

    // 2. Create a buffer and format the string
    std::vector<char> buffer(size + 1);
    std::vsnprintf(buffer.data(), buffer.size(), msg, args);
    const std::string formattedMsg(buffer.data());

    // 3. Pass to spdlog
    switch (msgType) {
        case LOG_TRACE:   spdlog::trace(formattedMsg);   break;
        case LOG_DEBUG:   spdlog::debug(formattedMsg);   break;
        case LOG_INFO:    spdlog::info(formattedMsg);    break;
        case LOG_WARNING: spdlog::warn(formattedMsg);    break;
        case LOG_ERROR:   spdlog::error(formattedMsg);   break;
        case LOG_FATAL:   spdlog::critical(formattedMsg); break;
        default: break;
    }
}
#endif //PRIORITY_ROUTING_RAYLIB_LOGGER_H