/**
 * @file main.cpp
 * @brief Entry point for the application.
 */

#include "Tools/CommandLineParser.h" 
#include "Tools/Logger.h"
#include "Gui/Gui.h"
#include "Application/Application.h"
#include <iostream>
#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        tools::ConsoleLogger logger;

        // Command line argument parsing
        tools::CommandLineParser parser;
        parser.parse(argc, argv);

        // Create EventBridge
        tadaima::EventBridge bridge;

        // Create GUI with configuration
        tadaima::gui::Gui::config config;
        tadaima::gui::Gui gui(config);

        // Create and configure the application
        tadaima::application::Application application(logger, bridge);

        bridge.initialize(application, gui);
        application.Initialize();
        application.run();

        // Run the GUI
        gui.run();
    }

    // Catch any exceptions and display error message
    catch( const std::exception& e )
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
