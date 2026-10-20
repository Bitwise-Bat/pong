#pragma once
#include <format>
#include <print>

namespace pong
{
    [[noreturn]] inline void fatal_impl(std::string_view msg)
    {
        std::println(stderr, "[FATAL] {}", msg);
        std::abort();
    }
    
    template <class... Args>
    [[noreturn]] void fatal(
        std::string_view cond,
        std::string_view file,
        int32_t line,
        std::format_string<Args...> fmt,
        Args&&... args)
    {
        fatal_impl(std::format("{} {}:{} {}", cond, file, line,
                               std::format(fmt, std::forward<Args>(args)...)));
    }
    
    [[noreturn]] inline void fatal(std::string_view cond, std::string_view file, int32_t line)
    {
        fatal_impl(std::format("{} {}:{}", cond, file, line));
    }
}

#define P_ASSERT(cond, ...) \
    do { \
        if (!(cond)) ::pong::fatal(#cond, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
    } while(0)