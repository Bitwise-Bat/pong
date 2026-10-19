#pragma once
#include <format>
#include <print>

namespace pong
{
    template <class... Args>
    [[noreturn]] void fatal(
        std::string_view cond,
        std::string_view file,
        int32_t line,
        std::format_string<Args...> fmt, 
        Args&&... args )
    {
        std::print(stderr, "[FATAL] {} {}:{} ", cond, file, line);
        std::println(stderr, fmt, std::forward<Args>(args)...);
        std::abort();
    }
}

#define P_ASSERT(cond,  msg, ...) \
    do { \
        if (!(cond)) ::pong::fatal(#cond, __FILE__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__); \
    } while(0)