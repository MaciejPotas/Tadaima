#pragma once
#include "Tools/CommandLineParser.h"
#include "Tools/DataPackage.h"

namespace application
{
    class FlashCommanderOptions : public tools::CommandLineParser
    {
    public:

        virtual void parse(int argc, char* argv[]) override;

        virtual void displayHelp(const std::string& programName) override;

        struct FileSystemOptions
        {
            std::string name;
            tools::ComplexDataPackage<std::string, std::string> options;
        };

    private:
        bool m_floating = false;
        bool m_help = false;
        std::vector<FileSystemOptions> m_fileSystemOptions;
    };
}