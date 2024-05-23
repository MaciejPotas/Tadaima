/**
 * @file Application.cpp
 * @brief All definitions of the backend Application class.
 */

#include "Application.h"
#include "Gui/Gui.h"
#include <iostream>
#include <chrono>
#include "LessonLoader.h"
#include "Widgets/WidgetTypes.h"
#include "ApplicationEventList.h"

namespace tadaima
{
    namespace application
    {
        Application::Application() : m_running(false), m_database("lessons.db"), m_lessonManager(m_database)
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
            std::mutex mtx;
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
                            std::vector<Lesson> lessons = m_event.getEventData< std::vector<Lesson>>(ApplicationEvent::OnLessonCreated);
                            m_lessonManager.addLessons(lessons);
                            LessonDataLoader loader(m_database);
                            m_gui->initializeWidget(gui::widget::Type::LessonTreeView, loader.loadAllLessons());
                            m_event.clearEvent(ApplicationEvent::OnLessonCreated);
                        }

                        if( m_event.isEventOccurred(ApplicationEvent::OnLessonUpdate) )
                        {
                            std::vector<Lesson> lessons = m_event.getEventData<std::vector<Lesson>>(ApplicationEvent::OnLessonUpdate);
                            m_lessonManager.renameLessons(lessons);

                            LessonDataLoader loader(m_database);
                            m_gui->initializeWidget(gui::widget::Type::LessonTreeView, loader.loadAllLessons());
                            m_event.clearEvent(ApplicationEvent::OnLessonUpdate);
                        }

                    }
                    catch( const std::exception& ex )
                    {
                        // Handle specific exceptions thrown by filesystem operations
                        // You can log the error or rethrow the exception if needed
                        std::cerr << "Exception caught during filesystem operation: " << ex.what() << std::endl;

                    }
                    catch( ... )
                    {
                        // Handle unexpected exceptions
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
                m_threadRaise.notify_one(); // Notify the thread to wake up and exit
                if( workerThread.joinable() )
                {
                    workerThread.join();
                }
            }
        }

        // Function to add GUI instance
        void Application::addGui(gui::Gui& gui)
        {
            m_gui = &gui;
        }

        std::vector<Lesson> decodeLessonDataPackage(const gui::widget::LessonTreeViewWidget::LessonDataPackage& lessonDataPackage)
        {
            //// Check if the LessonDataPackage contains lesson data
            //if( !lessonDataPackage.has(LessonTreeViewWidget::PackageKey::LessonsPackage) )
            //{
            //    throw std::runtime_error("LessonDataPackage does not contain lesson data.");
            //}

            // Get the lessons package
            auto lessonPackages = lessonDataPackage.get<std::vector<gui::widget::LessonTreeViewWidget::LessonPackage>>(gui::widget::LessonTreeViewWidget::PackageKey::LessonsPackage);

            // Vector to hold the decoded lessons
            std::vector<Lesson> lessons;

            // Iterate over each LessonPackage
            for( const auto& lessonPackage : lessonPackages )
            {
                // Create a new Lesson object
                Lesson lesson;
                lesson.id = lessonPackage.get<int>(gui::widget::LessonTreeViewWidget::LessonDataKey::id);
                lesson.mainName = lessonPackage.get<std::string>(gui::widget::LessonTreeViewWidget::LessonDataKey::MainName);
                lesson.subName = lessonPackage.get<std::string>(gui::widget::LessonTreeViewWidget::LessonDataKey::SubName);

                // Get the word packages
                auto wordPackages = lessonPackage.get<std::vector<gui::widget::LessonTreeViewWidget::WordDataPackage>>(gui::widget::LessonTreeViewWidget::LessonDataKey::Words);

                // Iterate over the word packages and populate the Lesson's words
                for( const auto& wordPackage : wordPackages )
                {
                    Word word;
                    word.id = wordPackage.get<int>(gui::widget::LessonTreeViewWidget::WordDataKey::id);
                    word.kana = wordPackage.get<std::string>(gui::widget::LessonTreeViewWidget::WordDataKey::Kana);
                    word.translation = wordPackage.get<std::string>(gui::widget::LessonTreeViewWidget::WordDataKey::Translation);
                    word.romaji = wordPackage.get<std::string>(gui::widget::LessonTreeViewWidget::WordDataKey::Romaji);
                    word.exampleSentence = wordPackage.get<std::string>(gui::widget::LessonTreeViewWidget::WordDataKey::ExampleSentence);
                    word.tags = wordPackage.get<std::vector<std::string>>(gui::widget::LessonTreeViewWidget::WordDataKey::Tags);

                    lesson.words.push_back(word);
                }

                // Add the lesson to the vector
                lessons.push_back(lesson);
            }

            return lessons;
        }

        void Application::processLessonTreeViewWidgetEvent(const gui::widget::WidgetEvent* data)
        {
            try
            {
                if( data == nullptr )
                {
                    throw std::invalid_argument("Invalid argument in processLessonTreeViewWidgetEvent: data pointer is null");
                }

                switch( data->getEventType() )
                {
                    case gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnLessonCreated:
                    {
                        gui::widget::LessonTreeViewWidget::LessonDataPackage* New_pointer = dynamic_cast<gui::widget::LessonTreeViewWidget::LessonDataPackage*>(data->getEventData());
                        if( nullptr != New_pointer )
                        {
                            auto z = decodeLessonDataPackage(*New_pointer);
                            m_event.setEvent(ApplicationEvent::OnLessonCreated, z);
                        }
                    }

                    break;

                    case gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnLessonRename:
                    {
                        gui::widget::LessonTreeViewWidget::LessonDataPackage* New_pointer = dynamic_cast<gui::widget::LessonTreeViewWidget::LessonDataPackage*>(data->getEventData());
                        if( nullptr != New_pointer )
                        {
                            auto z = decodeLessonDataPackage(*New_pointer);
                            m_event.setEvent(ApplicationEvent::OnLessonUpdate, z);
                        }
                    }

                    break;

                    default:
                        throw std::invalid_argument("Handled event in the processLessonTreeViewWidgetEvent that doesnt exist. Shouldn't be the case in this architecture.");
                }

                m_threadRaise.notify_one();
            }
            catch( std::exception& exception )
            {
                std::cerr << exception.what();
            }
        }

        void Application::Initialize()
        {
            LessonDataLoader loader(m_database);

            m_gui->addListener(gui::widget::Type::LessonTreeView, std::bind(&Application::processLessonTreeViewWidgetEvent, this, std::placeholders::_1));
            m_gui->initializeWidget(gui::widget::Type::LessonTreeView, loader.loadAllLessons());
        }
    }
}
