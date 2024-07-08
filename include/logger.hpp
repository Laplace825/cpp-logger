#pragma once

#include <chrono>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>

namespace lap {

namespace logger {

#define _LAP_LOGGER_LEVEL(f) f(trace) f(info) f(debug) f(warn) f(error) f(fatal)

enum class LogLevel : std::uint8_t
{

#define _FUNCTION(name) name,
    _LAP_LOGGER_LEVEL(_FUNCTION)
#undef _FUNCTION

};

inline std::string level2String(LogLevel level)
{
    /**
     * @brief: Convert LogLevel to string
     */
    switch (level)
    {
#define _FUNCTION(name) \
    case LogLevel::name: return #name;
        _LAP_LOGGER_LEVEL(_FUNCTION)
#undef _FUNCTION
    }
    return "unkown";
}

inline LogLevel string2Level(std::string_view lev)
{
    /**
     * @brief: Convert string to logLevel
     * @return: default is info
     */
#define _FUNCTION(name) \
    if (#name == lev) return LogLevel::name;
    _LAP_LOGGER_LEVEL(_FUNCTION)
#undef _FUNCTION
    return LogLevel::info;
}

namespace __details {

// the color to set for different log levels
inline static char
    level_ansi_colors[static_cast< std::uint8_t >(LogLevel::fatal) + 1][6] = {
        "37",   // trace
        "32",   // info
        "35",   // debug
        "33",   // warning
        "31",   // error
        "31;1", // fatal
};

// use this marco to get format color with the value
#define __LOG_ANSI_COLOR(X) std::format("\E[{}m", X)

// only print when you have a higher log level
// the level is set to info by default
inline auto static maxLogLevel_Limit = []() -> LogLevel {
    /**
     * @brief: get from your environment variable
     * if not set, return info
     * just set LAP_MAXLOG_LEVEL=debug or else, then you can see the log
     * you can change LAP_MAXLOG_LEVEL to another name
     */
    std::cout << __LOG_ANSI_COLOR(level_ansi_colors[0]);
    if (auto envLoglevel = std::getenv("MAXLOG_LEVEL"); envLoglevel)
    {
        std::cout << std::format("[MAXLOG_LEVEL is set to {}]\n", envLoglevel);
        std::cout << "\E[m";
        return string2Level(envLoglevel);
    }
    std::cout << "[MAXLOG_LEVEL is set to info]\n";
    std::cout << "\E[m";
    return LogLevel::info;
}();

/**
 * @brief: use format with default source_location parameter
 */
template < typename _Arg >
class fmt_with_source_location
{
  private:
    std::source_location m_location;
    _Arg m_fmt;

  public:
    template < typename U >
        requires(std::constructible_from< _Arg, U >)
    //   U  => const char * || const char[&] || string ......
    // _Arg => std::format_string<...>
    consteval fmt_with_source_location(U&& fmt,
        std::source_location loc =
            std::source_location::current()) // must be there
        : m_fmt(std::forward< U >(fmt)), m_location(std::move(loc))
    {
    }

    constexpr _Arg const& format_string() const { return m_fmt; }

    constexpr auto const& loc() const { return m_location; }
};

inline static std::ofstream ofs("./log.txt", std::ios::app);

} // namespace __details

/**
 * @brief: generate different log functions for levels
 */
#define _FUNCTION(name)                                                      \
    template < typename... Args >                                            \
    void log_##name(                                                         \
        __details::fmt_with_source_location< std::format_string< Args... > > \
            fmt,                                                             \
        Args&&... args)                                                      \
    {                                                                        \
        if (__details::maxLogLevel_Limit <= logger::LogLevel::name)          \
        {                                                                    \
            auto loc = fmt.loc();                                            \
            auto now = std::chrono::system_clock::to_time_t(                 \
                std::chrono::system_clock::now());                           \
            std::stringstream now_str;                                       \
            now_str << std::put_time(                                        \
                std::localtime(&now), "%Y-%m-%dT%H:%M:%S");                  \
            auto msg = std::format("[ {:^5} ] {} * {}:{} -> {}\n",           \
                #name,                                                       \
                now_str.str(),                                               \
                loc.file_name(),                                             \
                loc.line(),                                                  \
                std::format(                                                 \
                    fmt.format_string(), std::forward< Args >(args)...));    \
            if (__details::ofs) __details::ofs << msg;                       \
            std::cout << std::format("{}{}\E[0m",                            \
                __LOG_ANSI_COLOR(__details::level_ansi_colors[(              \
                    std::uint8_t)logger::LogLevel::name]),                   \
                msg);                                                        \
        }                                                                    \
    }
_LAP_LOGGER_LEVEL(_FUNCTION)
#undef _FUNCTION

// just print the value with the name
// marco to generate log_with_value_name
#define __log_with_value_name_1(level, X) \
    ::lap::logger::log_##level(#X " = {}", X)
#define __log_with_value_name_2(level, X, Y) \
    ::lap::logger::log_##level(#X " = {}, " #Y " = {}", X, Y)
#define __log_with_value_name_3(level, X, Y, Z) \
    ::lap::logger::log_##level(#X " = {}, " #Y " = {}, " #Z " = {}", X, Y, Z)
#define __log_with_value_NARGS_IMPL(_1, _2, _3, N, ...) N
#define __log_with_value_NARGS(...) \
    __log_with_value_NARGS_IMPL(__VA_ARGS__, 3, 2, 1)

#define __log_with_value_CONCANT_2(X, Y) X##Y
#define __log_with_value_CONCANT(X, Y) __log_with_value_CONCANT_2(X, Y)
#define __log_with_value_EXPAND_2(X) X
#define __log_with_value_EXPAND(X) __log_with_value_EXPAND_2(X)

// @brief: you can print three values at most with their names
#define log_with_value_name(level, ...)                                      \
    __log_with_value_EXPAND(__log_with_value_CONCANT(__log_with_value_name_, \
        __log_with_value_NARGS(__VA_ARGS__))(level, __VA_ARGS__))

template < typename... Args >
void log(LogLevel level,
    __details::fmt_with_source_location< std::format_string< Args... > > fmt,
    Args&&... args)
{
    /**
     * @param level : the log level for you to specify
     * @param fmt : format string , example "{}"
     * @param args : arguments for fmt, these message you want
     * to print
     */
    if (__details::maxLogLevel_Limit <= level)
    {
        auto loc = fmt.loc();
        auto now = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        std::stringstream now_str;

        now_str << std::put_time(
            std::localtime(&now), "%Y-%m-%dT%H:%M:%S"); // "%Y-%m-%d %X"

        auto msg = std::format("[ {:^5} ] {} * {}:{} -> {}\n",
            level2String(level),
            now_str.str(),
            loc.file_name(),
            loc.line(),
            std::format(fmt.format_string(), std::forward< Args >(args)...));
        if (__details::ofs)
        {
            __details::ofs << msg;
        }

        std::cout << std::format("{}{}\E[0m",
            __LOG_ANSI_COLOR(__details::level_ansi_colors[(std::uint8_t)level]),
            msg);
    }
}

inline void set_log_file(const std::string& opath)
{
    /**
     * @brief: set the log file path
     * by default, the output path is the current directory in your shell
     */
    __details::ofs = std::move(std::ofstream(opath, std::ios::app));
}

} // namespace logger

} // namespace lap
