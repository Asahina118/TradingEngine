#pragma once

#include <chrono>
#include <ctime>
#include <string>

namespace Common
{
    typedef int64_t Nanos;

    constexpr Nanos NANOS_TO_MICROS = 1000;
    constexpr Nanos MICROS_TO_MILLIS = 1000;
    constexpr Nanos MILLIS_TO_SECONDS = 1000;
    constexpr Nanos NANOS_TO_MILLIS = NANOS_TO_MICROS * MICROS_TO_MILLIS;
    constexpr Nanos NANOS_TO_SECONDS = NANOS_TO_MICROS * MILLIS_TO_SECONDS;

    inline auto getCurrentNanos() noexcept
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    inline std::string& getCurrentTimeStr(std::string* timeStr)
    {
        const auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        timeStr->assign(ctime(&time));
        if (!timeStr->empty())
            timeStr->at(timeStr->length() - 1) = '\0';
        return *timeStr;
    }
}