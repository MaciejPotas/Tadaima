/**
 * @file EventBridge.h
 * @brief Header file for the EventBridge class.
 *
 * This file contains the declaration of the EventBridge class which is responsible
 * for bridging events between the GUI and the application logic. It includes methods
 * for initializing the connection, handling GUI events, and processing lesson data packages.
 */

#pragma once

#include <vector>
#include "lessons/Lesson.h"
#include "Widgets/Widget.h"
#include "quiz/QuizType.h"

namespace tadaima
{
    namespace gui { class Gui; }
    namespace application { class Application; struct ApplicationSettings; }

    /**
     * @brief The EventBridge class bridges events between the GUI and the application logic.
     *
     * This class is responsible for initializing the connection between the GUI and the application,
     * and handling events that occur within the GUI.
     */
    class EventBridge
    {
    public:
        /**
         * @brief Initializes the EventBridge with the application and GUI instances.
         * @param app Reference to the Application instance.
         * @param gui Reference to the GUI instance.
         */
        void initialize(application::Application& app, gui::Gui& gui);

        /**
         * @brief Initializes the GUI with a list of lessons.
         *
         * This method sets up the GUI components with the provided lessons data.
         *
         * @param lessons Vector containing the lessons to initialize in the GUI.
         */
        void initializeGui(const std::vector<Lesson>& lessons);

        /**
         * @brief Initializes the GUI with application settings.
         * @param settings The application settings to initialize in the GUI.
         */
        void initializeSettings(const application::ApplicationSettings& settings);

        /**
         * @brief Handles an event from the GUI.
         *
         * This method processes events that originate from the GUI widgets.
         *
         * @param data Pointer to the WidgetEvent data.
         */
        void handleEvent(const gui::widget::WidgetEvent* data);

    protected:

        gui::quiz::WordType stringToWordType(const std::string& str);

        std::string wordTypeToString(gui::quiz::WordType type);

        application::Application* m_app = nullptr; /**< Pointer to the Application instance. */
        gui::Gui* m_gui = nullptr; /**< Pointer to the GUI instance. */

        /**
         * @brief Handles the creation of a new lesson.
         *
         * This method processes the data package when a new lesson is created in the GUI.
         *
         * @param dataPackage The data package containing the new lesson information.
         */
        void onLessonCreated(const tools::DataPackage* dataPackage);

        /**
         * @brief Handles the renaming of a lesson.
         *
         * This method processes the data package when a lesson is renamed in the GUI.
         *
         * @param dataPackage The data package containing the renamed lesson information.
         */
        void onLessonRename(const tools::DataPackage* dataPackage);

        /**
         * @brief Handles the deletion of a lesson.
         *
         * This method processes the data package when a lesson is removed in the GUI.
         *
         * @param dataPackage The data package containing the deleted lesson information.
         */
        void onLessonRemove(const tools::DataPackage* dataPackage);

        /**
         * @brief Handles the edition of a lesson.
         *
         * This method processes the data package when a lesson is edited in the GUI.
         *
         * @param dataPackage The data package containing the edited lesson information.
         */
        void onLessonEdited(const tools::DataPackage* dataPackage);

        /**
         * @brief Handles the change of application settings.
         *
         * This method processes the data package when application settings are changed in the GUI.
         *
         * @param dataPackage The data package containing the changed settings information.
         */
        void onSettingsChanged(const tools::DataPackage* dataPackage);
    };
}
