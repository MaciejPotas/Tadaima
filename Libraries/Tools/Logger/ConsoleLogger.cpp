// -----------------------------------------------------------------------------
// File:        ConsoleLogger.cpp
// Description: Source file with the console logger implementation.
// -----------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "ConsoleLogger.h"
#include <chrono>
#include <format>
#include <iostream>

//------------------------------------------------------------------------------
// Prototypes
//------------------------------------------------------------------------------'
static inline  std::string DateTimeToStr()
{
    return std::format("{:%d-%m-%Y %H:%M:%OS}", std::chrono::system_clock::now());
}

//------------------------------------------------------------------------------
// Definitions and Macros
//------------------------------------------------------------------------------
using tools::Logger;
using tools::ConsoleLogger;

//------------------------------------------------------------------------------
// Function Implementation
//------------------------------------------------------------------------------
ConsoleLogger::ConsoleScope::ConsoleScope(Logger* logger, Level level, const std::string& name)
    : Logger::Scope(logger, level, name)
    , started(std::chrono::system_clock::now().time_since_epoch().count())
{ 
    xLog(p_logger, level, ">>> enter >>>", name);
}

ConsoleLogger::ConsoleScope::~ConsoleScope()
{
    const auto elapsed = std::chrono::microseconds((std::chrono::system_clock::now().time_since_epoch().count() - started) / 10);
    std::stringstream ss;
    ss  << "[elapsed] = " << std::setfill('0')
        << std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() << "."
        << std::setw(3) << (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() % 1000) << "\'"
        << std::setw(3) << (std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() % 1000) << "s";
    xLog(p_logger, level, ss.str().c_str());
    xLog(p_logger, level, "<<< leave <<<", name);
}

std::shared_ptr<Logger::Scope> ConsoleLogger::GetScope(Level level, const std::string& name)
{
    return std::make_shared<ConsoleScope>(this, level, name);
}

void ConsoleLogger::Print(Level level, const char* cstring)
{
    std::cout << DateTimeToStr() << " # " << Logger::ToString(level) << " " << cstring << std::endl;
}
