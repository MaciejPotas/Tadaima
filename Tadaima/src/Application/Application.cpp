#include "Application.h"
#include "Gui/Gui.h"
#include "gui/Widgets/WidgetTypes.h"
#include "ApplicationEventList.h"
#include <iostream>
#include <chrono>

namespace tadaima
{
    namespace application
    {
        Application::Application(EventBridge& eventBridge)
            : m_running(false),
            m_database("lessons.db"),
            m_lessonManager(m_database),
            m_eventBridge(eventBridge)
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
                            m_lessonManager.addLessons(lessons);
                            m_eventBridge.initializeGui(m_lessonManager.getAllLessons());
                            m_event.clearEvent(ApplicationEvent::OnLessonCreated);
                        }

                        if( m_event.isEventOccurred(ApplicationEvent::OnLessonUpdate) )
                        {
                            std::vector<Lesson> lessons = m_event.getEventData<std::vector<Lesson>>(ApplicationEvent::OnLessonUpdate);
                            m_lessonManager.renameLessons(lessons);
                            m_eventBridge.initializeGui(m_lessonManager.getAllLessons());
                            m_event.clearEvent(ApplicationEvent::OnLessonUpdate);
                        }

                    }
                    catch( const std::exception& ex )
                    {
                        std::cerr << "Exception caught during filesystem operation: " << ex.what() << std::endl;
                    }
                    catch( ... )
                    {
                        std::cerr << "Unexpected exception caught during filesystem operation" << std::endl;
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
            }
        }

        void Application::Initialize()
        {
            m_eventBridge.initializeGui(m_lessonManager.getAllLessons());
        }

        void Application::setEvent(ApplicationEvent event, const std::vector<Lesson>& lessons)
        {
            m_event.setEvent(event, lessons);
            m_threadRaise.notify_one();
        }

    }
}
