#pragma once

#include <format>
#include <print>

namespace pong
{
    template <class... Args>
    void log(std::format_string<Args...> fmt, Args&&... args)
    {
        std::print("[INFO] ");
        std::println(fmt, std::forward<Args...>(args)...);
    }
}


#define P_LOG_INFO(fmt, ...)     \
    do { ::pong::log(fmt __VA_OPT__(,) __VA_ARGS__); } \
    while(0)
