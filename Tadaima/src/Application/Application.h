/**
 * @file Application.h
 * @brief Declaration of the backend Application class.
 */

#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include "LessonsDatabase.h"
#include "Lessons/LessonManager.h"
#include "Tools/EventsData.h"

namespace tadaima
{
    namespace gui { class Gui; }
    namespace gui { namespace widget { class WidgetEvent; class Widget; } }

    namespace application
    {

        /**
         * @brief The Application class represents the backend application logic.
         *
         * This class manages the interaction between the GUI and the filesystem.
         */
        class Application
        {
        public:

            /**
             * @brief Constructor.
             *
             * Initializes a new instance of the Application class.
             */
            Application();

            /**
             * @brief Destructor.
             *
             * Cleans up resources and stops the worker thread.
             */
            ~Application();

            /**
             * @brief Runs the application.
             *
             * This method starts the main application logic.
             */
            void run();

            /**
             * @brief Adds a GUI instance to the application.
             *
             * @param gui The GUI instance to add.
             */
            void addGui(gui::Gui& gui);

            /**
             * @brief Initializes the application.
             *
             * This method initializes the application and sets up necessary components.
             */
            void Initialize();

        private:

            /**
             * @brief Worker thread function.
             *
             * This method is the entry point for the worker thread.
             * It periodically performs tasks based on notifications.
             */
            void runThread();

            /**
             * @brief Stops the worker thread.
             *
             * This method stops the worker thread.
             */
            void stopThread();

            /**
             * @brief Sends data to the GUI widget.
             *
             * This method sends data to the GUI widget.
             */
            void sendToWidget();

            /**
             * @brief Processes filesystem events.
             *
             * This method is called to process filesystem events received by the application.
             *
             * @param data Pointer to the WidgetEvent containing filesystem event data.
             */
            void processLessonTreeViewWidgetEvent(const gui::widget::WidgetEvent* data);

            LessonsDatabase m_database;
            LessonManager m_lessonManager;
            tools::EventsData<std::vector<Lesson>> m_event; /**< Event data structure. */
            gui::Gui* m_gui = nullptr; /**< Pointer to the GUI instance. */
            std::thread workerThread; /**< Worker thread for background tasks. */
            std::atomic<bool> m_running; /**< Atomic flag to control the worker thread's execution. */
            std::string m_newDirectory; /**< The path to the new directory. */
            std::condition_variable m_threadRaise; /**< Condition variable for thread synchronization. */
        };
    }
}