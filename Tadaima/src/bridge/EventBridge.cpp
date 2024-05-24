#include "EventBridge.h"
#include "application/Application.h"
#include "Gui/Gui.h"
#include "Gui/helpers/LessonDataDecoder.h"
#include <stdexcept>
#include <iostream>
#include "LessonLoader.h"

namespace tadaima
{
    void EventBridge::initialize(application::Application& app, gui::Gui& gui)
    {
        m_app = &app;
        m_gui = &gui;

        m_gui->addListener(gui::widget::Type::LessonTreeView, std::bind(&EventBridge::handleEvent, this, std::placeholders::_1));
    }

    void EventBridge::initializeGui(const std::vector<Lesson>& lessons)
    {
        auto lessonsPackage = LessonDataLoader(lessons).loadAllLessons();
        m_gui->initializeWidget(gui::widget::Type::LessonTreeView, lessonsPackage);
    }

    void EventBridge::handleEvent(const gui::widget::WidgetEvent* data)
    {
        if( data == nullptr )
        {
            throw std::invalid_argument("Invalid argument in handleEvent: data pointer is null");
        }

        switch( data->getEventType() )
        {
            case gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnLessonCreated:
            {
                onLessonCreated(data->getEventData());
                break;
            }
            case gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnLessonRename:
            {
                onLessonRename(data->getEventData());
                break;
            }

            case gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnLessonDelete:
            {
                onLessonRemove(data->getEventData());
                break;
            }

            case gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnLessonEdited:
            {
                onLessonEdited(data->getEventData());
                break;
            }

            default:
                throw std::invalid_argument("Unhandled event type in handleEvent.");
        }
    }

    void EventBridge::onLessonCreated(const tools::DataPackage* dataPackage)
    {
        auto lessons = gui::widget::LessonDataDecoder().decodeLessonDataPackage(dataPackage);
        m_app->setEvent(application::ApplicationEvent::OnLessonCreated, lessons);
    }

    void EventBridge::onLessonRename(const tools::DataPackage* dataPackage)
    {
        auto lessons = gui::widget::LessonDataDecoder().decodeLessonDataPackage(dataPackage);
        m_app->setEvent(application::ApplicationEvent::OnLessonUpdate, lessons);
    }

    void EventBridge::onLessonRemove(const tools::DataPackage* dataPackage)
    {
        auto lessons = gui::widget::LessonDataDecoder().decodeLessonDataPackage(dataPackage);
        m_app->setEvent(application::ApplicationEvent::OnLessonDelete, lessons);
    }

    void EventBridge::onLessonEdited(const tools::DataPackage* dataPackage)
    {
        auto lessons = gui::widget::LessonDataDecoder().decodeLessonDataPackage(dataPackage);
        m_app->setEvent(application::ApplicationEvent::OnLessonEdited, lessons);
    }
}

