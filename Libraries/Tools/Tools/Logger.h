#pragma once

#include <string>

namespace tools
{
    /**
     * @brief Enumeration for log levels.
     */
    enum class LogLevel
    {
        VERBOSE, /**< Verbose messages */
        DEBUG,   /**< Debug messages */
        INFO,    /**< Informational messages */
        WARNING, /**< Warning messages */
        PROBLEM
    };

    /**
     * @brief Abstract base class for a logger.
     */
    class Logger
    {
    public:
        /**
         * @brief Logs a message with a specific log level.
         *
         * @param message The message to log.
         * @param level The log level of the message.
         */
        virtual void log([[maybe_unused]] const std::string& message, [[maybe_unused]] LogLevel level = LogLevel::INFO) {}

        /**
         * @brief Virtual destructor.
         */
        virtual ~Logger() = default;
    };

    /**
     * @brief Console logger implementation.
     */
    class ConsoleLogger : public Logger
    {
    public:
        /**
         * @brief Constructor to set the verbosity level.
         *
         * @param verbosity The verbosity level of the logger.
         */
        ConsoleLogger(LogLevel verbosity = LogLevel::INFO) : verbosityLevel(verbosity) {}

        /**
         * @brief Logs a message to the console with a specific log level.
         *
         * @param message The message to log.
         * @param level The log level of the message.
         */
        void log(const std::string& message, LogLevel level) override;

    private:
        LogLevel verbosityLevel; /**< The verbosity level of the logger */

        /**
         * @brief Gets the current time formatted as a string.
         *
         * @return The current time as a string in the format YYYY-MM-DD HH:MM:SS.mmm.
         */
        std::string getCurrentTime();
    };
}