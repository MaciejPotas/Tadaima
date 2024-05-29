#include "EventBridge.h"
#include "application/Application.h"
#include "Gui/Gui.h"
#include "Gui/helpers/LessonDataDecoder.h"
#include <stdexcept>
#include <iostream>
#include "LessonLoader.h"
#include "SettingsLoader.h"
#include "helpers/SettingsDataDecoder.h"

namespace tadaima
{
    void EventBridge::initialize(application::Application& app, gui::Gui& gui)
    {
        m_app = &app;
        m_gui = &gui;

        m_gui->addListener(gui::widget::Type::LessonTreeView, std::bind(&EventBridge::handleEvent, this, std::placeholders::_1));
        m_gui->addListener(gui::widget::Type::ApplicationSettings, std::bind(&EventBridge::handleEvent, this, std::placeholders::_1));
    }

    void EventBridge::initializeGui(const std::vector<Lesson>& lessons)
    {
        auto lessonsPackage = LessonDataLoader(lessons).loadAllLessons();
        m_gui->initializeWidget(gui::widget::Type::LessonTreeView, lessonsPackage);
    }

    void EventBridge::initializeSettings(const application::ApplicationSettings& settings)
    {
        auto package = SettingsLoader(settings).load();
        m_gui->initializeWidget(gui::widget::Type::MenuBar, package);
    }

    void EventBridge::handleEvent(const gui::widget::WidgetEvent* data)
    {
        if( data == nullptr )
        {
            throw std::invalid_argument("Invalid argument in handleEvent: data pointer is null");
        }

        if( gui::widget::Type::LessonTreeView == data->getWidget().getType() )
        {
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

        if( gui::widget::Type::ApplicationSettings == data->getWidget().getType() )
        {
            switch( data->getEventType() )
            {
                case gui::widget::ApplicationSettingsWidget::ApplicationSettingsWidgetEvent::OnSettingsChanged:
                {
                    onSettingsChanged(data->getEventData());
                    break;
                }

                default:
                    throw std::invalid_argument("Unhandled event type in handleEvent.");
            }
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

    void EventBridge::onSettingsChanged(const tools::DataPackage* dataPackage)
    {
        auto package = gui::widget::SettingsDataDecoder().decodeDataPackage(dataPackage);
        m_app->setEvent(application::ApplicationEvent::OnSettingsChanged, package);
    }

}

