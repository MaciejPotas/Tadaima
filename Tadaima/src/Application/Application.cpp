/**
 * @file Application.cpp
 * @brief All definitions of the backend Application class.
 */

#include "Application.h"
#include "Gui/Gui.h"
#include <iostream>
#include <chrono>
#include "LessonLoader.h"

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

                if( m_running && m_event.eventOccured() )
                {
                    try
                    {
                        //if( m_event.changePathEvent )
                        //{
                        //    auto* filesystem = getFileSystemByKey(m_event.callerId);
                        //    if( !filesystem )
                        //    {
                        //        throw std::runtime_error("No filesystem found for callerId: " + m_event.callerId);
                        //    }

                        //    const bool result = filesystem->changeDirectory(m_event.newPath);

                        //    if( !result )
                        //    {
                        //        throw std::runtime_error("Failed to change directory in filesystem for callerId: " + m_event.callerId);
                        //    }

                        //    m_data.set(gui::widget::FileSystemWidget::WidgetDataPackage::Alias::dirPath, m_event.newPath);
                        //    m_data.set(gui::widget::FileSystemWidget::WidgetDataPackage::Alias::dirEntries, filesystem->directoryEntries());
                        //    m_gui->initializeWidget(m_event.callerId, m_data);
                        //    m_event.changePathEvent = false;
                        //}

                        //if( m_event.buttonClick )
                        //{
                        //    m_fileSystemMap[0].second->openFile("3/DS/stats.txt", std::ios::in); // TMP test code for CDI
                        //    m_event.buttonClick = false;
                        //}
                    }
                    catch( const std::exception& ex )
                    {
                        // Handle specific exceptions thrown by filesystem operations
                        // You can log the error or rethrow the exception if needed
                        std::cerr << "Exception caught during filesystem operation: " << ex.what() << std::endl;
                    }
                    catch( ... ) // Catch any other unexpected exceptions
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

        void Application::Initialize()
        {
            LessonDataLoader loader(m_database);
            m_gui->initializeWidget(gui::Gui::LessonTreeView, loader.loadAllLessons());
        }
    }
}
