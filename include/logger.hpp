#pragma once

#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <source_location>
#include <string>
#include <string_view>

namespace lap {

namespace logger {

#define _LAP_LOGGER_LEVEL(f) \
    f(trace) f(info) f(debug) f(warning) f(error) f(fatal)

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
inline char level_ansi_colors[static_cast< std::uint8_t >(LogLevel::fatal) + 1]
                             [6] = {
                                 "37",   // trace
                                 "38",   // info
                                 "35",   // debug
                                 "33",   // warning
                                 "31",   // error
                                 "31;1", // fatal
};

// use this marco to get format color with the value
#define LOG_ANSI_COLOR(X) std::format("\E[{}m", X)

// only print when you have a higher log level
// the level is set to info by default
inline auto maxLogLevel_Limit = []() -> LogLevel {
    /**
     * @brief: get from your environment variable
     * if not set, return info
     * just set LAP_MAXLOG_LEVEL=debug or else, then you can see the log
     * you can change LAP_MAXLOG_LEVEL to another name
     */
    std::cout << LOG_ANSI_COLOR(level_ansi_colors[0]);
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
    std::source_location location;
    _Arg fmt;

  public:
    template < typename U >
        requires(std::constructible_from< _Arg, U >)
    consteval fmt_with_source_location(U&& _fmt,
        std::source_location loc =
            std::source_location::current()) // must be there
        : fmt(std::forward< U >(_fmt)), location(std::move(loc))
    {
    }

    constexpr _Arg const& format_string() const { return fmt; }

    constexpr auto const& loc() const { return location; }
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
            auto msg = std::format("{}:{}: [{}] {}\n",                       \
                loc.file_name(),                                             \
                loc.line(),                                                  \
                #name,                                                       \
                std::format(                                                 \
                    fmt.format_string(), std::forward< Args >(args)...));    \
            if (__details::ofs) __details::ofs << msg;                       \
            std::cout << std::format("{}{}\E[0m",                            \
                LOG_ANSI_COLOR(__details::level_ansi_colors[(                \
                    std::uint8_t)logger::LogLevel::name]),                   \
                msg);                                                        \
        }                                                                    \
    }
_LAP_LOGGER_LEVEL(_FUNCTION)
#undef _FUNCTION

// just print the value with the name
// reflection
#define LOG_Println(X) ::lap::logger::log_debug(#X "={}", X);

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
        auto msg = std::format("{}:{}: [{}] {}\n",
            loc.file_name(),
            loc.line(),
            level2String(level),
            std::format(fmt.format_string(), std::forward< Args >(args)...));
        if (__details::ofs)
        {
            __details::ofs << msg;
        }

        std::cout << std::format("{}{}\E[0m",
            LOG_ANSI_COLOR(__details::level_ansi_colors[(std::uint8_t)level]),
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
