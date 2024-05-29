#include "Application.h"
#include "Gui/Gui.h"
#include "Gui/Widgets/WidgetTypes.h"
#include "ApplicationEventList.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Tools/Logger.h"
#include "ApplicationDatabase.h"
#include "ApplicationSettings.h"

namespace tadaima
{
    namespace application
    {
        Application::Application(tools::Logger& logger, EventBridge& eventBridge)
            : m_running(false),
            m_database("lessons.db", logger),
            m_lessonManager(m_database),
            m_eventBridge(eventBridge),
            m_logger(logger)
        {
        }

        Application::~Application()
        {
            stopThread();
        }

        void Application::run()
        {
            m_running = true;
            workerThread = std::thread(&Application::runThread, this);
            m_logger.log("Application started running.", tools::LogLevel::INFO);
        }

        void Application::runThread()
        {
            std::unique_lock<std::mutex> lock(mtx);

            while( m_running )
            {
                m_threadRaise.wait_for(lock, std::chrono::milliseconds(100));

                if( m_running && m_event.anyEventChanged() )
                {
                    try
                    {
                        if( m_event.isEventOccurred(ApplicationEvent::OnLessonCreated) )
                        {
                            std::vector<Lesson> lessons = m_event.getEventData<std::vector<Lesson>>(ApplicationEvent::OnLessonCreated);
                            m_logger.log("OnLessonCreated event occurred. Lessons added: " + lessonsToString(lessons), tools::LogLevel::INFO);
                            m_lessonManager.addLessons(lessons);
                            m_eventBridge.initializeGui(m_lessonManager.getAllLessons());
                            m_event.clearEvent(ApplicationEvent::OnLessonCreated);
                        }

                        if( m_event.isEventOccurred(ApplicationEvent::OnLessonUpdate) )
                        {
                            std::vector<Lesson> lessons = m_event.getEventData<std::vector<Lesson>>(ApplicationEvent::OnLessonUpdate);
                            m_logger.log("OnLessonUpdate event occurred. Lessons updated: " + lessonsToString(lessons), tools::LogLevel::INFO);
                            m_lessonManager.renameLessons(lessons);
                            m_eventBridge.initializeGui(m_lessonManager.getAllLessons());
                            m_event.clearEvent(ApplicationEvent::OnLessonUpdate);

                        }

                        if( m_event.isEventOccurred(ApplicationEvent::OnLessonDelete) )
                        {
                            std::vector<Lesson> lessons = m_event.getEventData<std::vector<Lesson>>(ApplicationEvent::OnLessonDelete);
                            m_logger.log("OnLessonDelete event occurred. Lessons deleted: " + lessonsToString(lessons), tools::LogLevel::INFO);
                            m_lessonManager.removeLessons(lessons);
                            m_eventBridge.initializeGui(m_lessonManager.getAllLessons());
                            m_event.clearEvent(ApplicationEvent::OnLessonDelete);
                        }

                        if( m_event.isEventOccurred(ApplicationEvent::OnLessonEdited) )
                        {
                            std::vector<Lesson> lessons = m_event.getEventData<std::vector<Lesson>>(ApplicationEvent::OnLessonEdited);
                            m_logger.log("OnLessonEdited event occurred. Lessons deleted: " + lessonsToString(lessons), tools::LogLevel::INFO);
                            m_lessonManager.editLessons(lessons);
                            m_eventBridge.initializeGui(m_lessonManager.getAllLessons());
                            m_event.clearEvent(ApplicationEvent::OnLessonEdited);
                        }

                        if( m_event.isEventOccurred(ApplicationEvent::OnSettingsChanged) )
                        {
                            ApplicationSettings applicationSettings = m_event.getEventData<ApplicationSettings>(ApplicationEvent::OnSettingsChanged);
                            m_logger.log("OnSettingsChanged event occurred", tools::LogLevel::INFO);
                            m_logger.log(applicationSettings.toString(), tools::LogLevel::INFO);
                            m_database.saveSettings(applicationSettings);
                            m_event.clearEvent(ApplicationEvent::OnSettingsChanged);
                        }
                    }
                    catch( const std::exception& ex )
                    {
                        m_logger.log(std::string("Exception caught during event handling: ") + ex.what(), tools::LogLevel::PROBLEM);
                    }
                    catch( ... )
                    {
                        m_logger.log("Unexpected exception caught during event handling", tools::LogLevel::PROBLEM);
                    }
                }
            }
        }

        void Application::stopThread()
        {
            if( m_running )
            {
                m_running = false;
                m_threadRaise.notify_one();
                if( workerThread.joinable() )
                {
                    workerThread.join();
                }
                m_logger.log("Application stopped running.", tools::LogLevel::INFO);
            }
        }

        void Application::Initialize()
        {
            m_eventBridge.initializeGui(m_lessonManager.getAllLessons());
            m_eventBridge.initializeSettings(m_database.loadSettings());

            m_logger.log("Application initialized.", tools::LogLevel::INFO);
        }

        //void Application::setEvent(ApplicationEvent event, const std::vector<Lesson>& lessons)
        //{
        //    m_event.setEvent(event, lessons);
        //    m_threadRaise.notify_one();
        //    m_logger.log("Event set: " + eventToString(event) + ". Lessons: " + lessonsToString(lessons), tools::LogLevel::DEBUG);
        //}

        std::string Application::lessonsToString(const std::vector<Lesson>& lessons)
        {
            std::ostringstream oss;
            for( const auto& lesson : lessons )
            {
                oss << "{" << lesson.id << ", " << lesson.mainName << "}, ";
            }
            std::string result = oss.str();
            return result.substr(0, result.length() - 2); // Remove the last comma and space
        }

        std::string Application::eventToString(ApplicationEvent event)
        {
            switch( event )
            {
                case ApplicationEvent::OnLessonCreated:
                    return "OnLessonCreated";
                case ApplicationEvent::OnLessonUpdate:
                    return "OnLessonUpdate";
                case ApplicationEvent::OnLessonDelete:
                    return "OnLessonDelete";
                case ApplicationEvent::OnSettingsChanged:
                    return "OnSettingschanged";
                default:
                    return "UnknownEvent";
            }
        }
    }
}
