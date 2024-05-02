// -----------------------------------------------------------------------------
// File:        Logger.h
// Description: Header file with the logger interface.
// -----------------------------------------------------------------------------
#pragma once

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <string>
#include <sstream>
#include <memory>

//------------------------------------------------------------------------------
// Definitions and Macros
//------------------------------------------------------------------------------
#define TEXT_COLOR_BLACK        "\033[30m"
#define TEXT_COLOR_RED          "\033[31m"
#define TEXT_COLOR_GREEN        "\033[32m"
#define TEXT_COLOR_YELLOW       "\033[33m"
#define TEXT_COLOR_BLUE         "\033[34m"
#define TEXT_COLOR_MAGENTA      "\033[35m"
#define TEXT_COLOR_CYAN         "\033[36m"
#define TEXT_COLOR_WHITE        "\033[37m"
#define TEXT_COLOR_RESET        "\033[39m"

#define xLog(p_logger, level, ...)          { if(p_logger != nullptr) (p_logger)->Put((level), __VA_ARGS__); }

#define xCritical(p_logger, ...)            xLog(p_logger, tools::Logger::Level::Critical, __VA_ARGS__)
#define xError(p_logger, ...)               xLog(p_logger, tools::Logger::Level::Error, __VA_ARGS__)
#define xWarning(p_logger, ...)             xLog(p_logger, tools::Logger::Level::Warning, __VA_ARGS__)
#define xMessage(p_logger, ...)             xLog(p_logger, tools::Logger::Level::Message, __VA_ARGS__)
#define xInfo(p_logger, ...)                xLog(p_logger, tools::Logger::Level::Info, __VA_ARGS__)
#define xDebug(p_logger, ...)               xLog(p_logger, tools::Logger::Level::Debug, __VA_ARGS__)

#define xVariable(p_logger, variable)       xDebug(p_logger, (std::stringstream() << "[" << #variable << "] = " << (variable)).str().c_str())
#define xResultInfo(p_logger, result)       xInfo(p_logger, "[result]", "=", (result))
#define xFunctionInfo(p_logger)             const auto fn_scope = ((p_logger) ? (p_logger)->GetScope(tools::Logger::Level::Info, __FUNCTION__) : nullptr)
#define xResultMessage(p_logger, result)    xMessage(p_logger, "[result]", "=", (result))
#define xFunctionMessage(p_logger)          const auto if_scope = ((p_logger) ? (p_logger)->GetScope(tools::Logger::Level::Message, __FUNCTION__) : nullptr)
#define xScope(p_logger, name)              const auto nm_scope = ((p_logger) ? (p_logger)->GetScope(tools::Logger::Level::Info, (name)) : nullptr)

//------------------------------------------------------------------------------
// Typedefs, Classes and Structures
//------------------------------------------------------------------------------

namespace tools
{
    class Logger
    {
    public:
        enum class Level
        {
            None,
            Critical,
            Error,
            Warning,
            Message,
            Info,
            Debug
        };
        static const std::string ToString(Level level)
        {
            switch (level)
            {
            case Level::None:       return                      "[...]";
            case Level::Critical:   return TEXT_COLOR_MAGENTA   "[!!!]"     TEXT_COLOR_RESET;
            case Level::Error:      return TEXT_COLOR_RED       "[err]"     TEXT_COLOR_RESET;
            case Level::Warning:    return TEXT_COLOR_YELLOW    "[war]"     TEXT_COLOR_RESET;
            case Level::Message:    return TEXT_COLOR_BLUE      "[msg]"     TEXT_COLOR_RESET;
            case Level::Info:       return TEXT_COLOR_CYAN      "[inf]"     TEXT_COLOR_RESET;
            case Level::Debug:      return                      "[dbg]";
            }
            return "";
        }

        virtual ~Logger() = default;

        void SetLevel(Level _level) { m_level = _level; }

        template<typename... Args>
        void Put(Level level, const Args&... args)
        {
            if (level <= m_level)
            {
                std::stringstream ss;
                (Join(ss, args), ...);
                Print(level, ss.str().c_str());
            }
        }

        class Scope
        {
        public:
            Scope(Logger* _logger, Logger::Level _level, const std::string& _name)
                : p_logger{ _logger }
                , level{ _level }
                , name{ _name }
            { }

        protected:
            Logger* p_logger{ nullptr };
            Logger::Level level{ Logger::Level::None };
            std::string name{};
        };

        virtual std::shared_ptr<Scope> GetScope(Level level, const std::string& name)
        {
            return std::make_shared<Scope>(this, level, name);
        }

    protected:
        Level m_level;

        virtual void Print(Level, const char*) {};

    private:
        template <typename Arg>
        void Join(std::ostream& stream, Arg&& arg)
        {
            stream << std::forward<Arg>(arg) << " ";
        }
    };

}
