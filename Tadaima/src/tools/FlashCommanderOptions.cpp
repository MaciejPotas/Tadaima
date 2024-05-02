#include "FlashCommanderOptions.h"
#include "tools/CommandLineParser.h"
#include "tools/DataPackage.h"

namespace application
{
    void FlashCommanderOptions::parse(int argc, char* argv[])
    {
        tools::CommandLineParser::parse(argc, argv);
        const std::list<std::pair<std::string, std::string>>& options = getAllArguments();

        int8_t u8_index = 0;
        for( const auto& option : options )
        {
            if( option.first == "h" || option.first == "help" )
            {
                m_help = true;
                return;
            }

            else if( option.first == "floating" )
            {
                m_floating = true;
            }

            else if( option.first == "efs" )
            {
                m_fileSystemOptions.push_back(FileSystemOptions());
                m_fileSystemOptions.back().name = option.second;
            }
            else
            {
                m_fileSystemOptions.back().options.set<std::string>(option.first, option.second);
            }
        }
    }

    void FlashCommanderOptions::displayHelp(const std::string& programName)
    {
        tools::CommandLineParser::displayHelp(programName);

        std::cout << " --efs Specify the external file system library. For example, passing 'example.dll' will initialize the file system from this DLL.\n";
        std::cout << " Everything that comes after that becomes an option for the file. For instance: passing option --port 443 after --efs example.dll \n";
        std::cout << " will bind the port option to this file. Every option will be binded, except mentioned below.\n";
        std::cout << " --floating Set the application's graphics to a floating state. \n";
        std::cout << " --help, -h Prints application help. \n";
    }