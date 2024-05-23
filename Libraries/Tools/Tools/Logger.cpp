#pragma once

#include "Logger.h"
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace tools
{
    void ConsoleLogger::log(const std::string& message, LogLevel level) 
    {
        if( level >= verbosityLevel )
        {
            std::string timestamp = getCurrentTime();
            switch( level )
            {
                case LogLevel::VERBOSE:
                    std::cout << "\033[35m[" << timestamp << "][VERBOSE]: " << message << "\033[0m" << std::endl;
                    break;
                case LogLevel::DEBUG:
                    std::cout << "\033[34m[" << timestamp << "][DEBUG]: " << message << "\033[0m" << std::endl;
                    break;
                case LogLevel::INFO:
                    std::cout << "\033[32m[" << timestamp << "][INFO]: " << message << "\033[0m" << std::endl;
                    break;
                case LogLevel::WARNING:
                    std::cout << "\033[33m[" << timestamp << "][WARNING]: " << message << "\033[0m" << std::endl;
                    break;

            }
        }
    }

    std::string ConsoleLogger::getCurrentTime()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S")
            << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();
        return ss.str();
    }
};
