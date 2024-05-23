/**
 * @file Application.h
 * @brief Header file for the Application class.
 *
 * This file contains the declaration of the Application class which represents the backend
 * application logic. The class manages the interaction between the GUI and the filesystem,
 * handles application initialization, and manages background tasks through a worker thread.
 */

#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include "LessonsDatabase.h"
#include "Lessons/LessonManager.h"
#include "Tools/EventsData.h"
#include "bridge/EventBridge.h"

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
             *
             * @param eventBridge Reference to an EventBridge instance for event handling.
             */
            Application(EventBridge& eventBridge);

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

            /**
             * @brief Sets the event and corresponding lessons.
             *
             * @param event The application event to set.
             * @param lessons The list of lessons associated with the event.
             */
            void setEvent(ApplicationEvent event, const std::vector<Lesson>& lessons);

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

            LessonsDatabase m_database; /**< Database for managing lessons. */
            LessonManager m_lessonManager; /**< Manager for handling lesson operations. */
            EventBridge& m_eventBridge; /**< Reference to the EventBridge for event handling. */

            tools::EventsData<std::vector<Lesson>> m_event; /**< Event data structure. */
            gui::Gui* m_gui = nullptr; /**< Pointer to the GUI instance. */
            std::thread workerThread; /**< Worker thread for background tasks. */
            std::atomic<bool> m_running; /**< Atomic flag to control the worker thread's execution. */
            std::string m_newDirectory; /**< The path to the new directory. */
            std::condition_variable m_threadRaise; /**< Condition variable for thread synchronization. */
            std::mutex mtx; /**< Mutex for thread synchronization. */
        };
    }
}
