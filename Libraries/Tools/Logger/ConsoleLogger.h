// -----------------------------------------------------------------------------
// File:        ConsoleLogger.h
// Description: Header file with the console logger implementation.
// -----------------------------------------------------------------------------
#pragma once

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "Logger.h"

//------------------------------------------------------------------------------
// Typedefs, Classes and Structures
//------------------------------------------------------------------------------
namespace tools
{
    class ConsoleLogger : public Logger
    {
    public:
        ConsoleLogger(Level _level = Level::Debug)
            : Logger{}
        {
            SetLevel(_level);
        }

        class ConsoleScope : public Scope
        {
        public:
            ConsoleScope(Logger* logger, Level level, const std::string& name);
            ~ConsoleScope();
        protected:
            const long long started;
        };

        std::shared_ptr<Scope> GetScope(Level level, const std::string& name) override;

    protected:
        void Print(Level level, const char* cstring) override;
    };
}
