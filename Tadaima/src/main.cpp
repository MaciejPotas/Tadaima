/**
 * @file main.cpp
 * @brief Entry point for the application.
 */

#include "Tools/CommandLineParser.h" 
#include "Gui/Gui.h"
#include "Application/Application.h"
#include <iostream>
#include <iostream>
#include <filesystem>
#include "Lessons/Lesson.h"
#include "Lessons/LessonManager.h"

int main(int argc, char* argv[])
{
    try
    {
        // Command line argument parsing
        tools::CommandLineParser parser;
        parser.parse(argc, argv);

        // Create GUI with configuration
        tadaima::gui::Gui::config config;
        tadaima::gui::Gui gui(config);

        // Create and configure the application
        tadaima::application::Application application;
        application.addGui(gui);
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
