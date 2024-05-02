// ----------------------------------------------------------------------------
// File:        CommandLineParser.h
// Description: Header file for the main thread. Provides functionality for
//              parsing command-line arguments and displaying help messages.
// ----------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <string>
#include <map>
#include <stdexcept>
#include <any>
#include <vector>
#include <list>
#include <utility>

namespace tools
{
    class CommandLineParser
    {
    public:

        virtual void parse(int argc, char* argv[])
        {
            for( int i = 1; i < argc; ++i )
            {
                std::string arg = argv[i];
                if( arg.substr(0, 2) == "--" )
                {
                    std::string key = arg.substr(2);
                    if( i + 1 < argc && argv[i + 1][0] != '-' )
                    {
                        std::string value = argv[i + 1];
                        args.push_back(std::make_pair(std::move(key), std::move(value)));
                        ++i; // skip the value
                    }
                    else
                    {
                        args.push_back(std::make_pair(std::move(key), ""));
                    }
                }

                else if( arg[0] == '-' )
                {
                    std::string key = arg.substr(1);
                    args.push_back(std::make_pair(std::move(key), ""));
                }
            }
        }

        virtual void displayHelp(const std::string& programName)
        {
            std::cout << "Usage: " << programName << " [options]\n";
        }

    protected:
        const std::list<std::pair<std::string, std::string>>& getAllArguments() const
        {
            return args;
        }

    private:
        std::list<std::pair<std::string, std::string>> args;
    };
}
