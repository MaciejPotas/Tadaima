#include "EventBridge.h"
#include "application/Application.h"
#include "Gui/Gui.h"
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
                auto* newPointer = dynamic_cast<const gui::widget::LessonTreeViewWidget::LessonDataPackage*>(data->getEventData());
                if( newPointer != nullptr )
                {
                    onLessonCreated(*newPointer);
                }
                break;
            }
            case gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnLessonRename:
            {
                auto* newPointer = dynamic_cast<const gui::widget::LessonTreeViewWidget::LessonDataPackage*>(data->getEventData());
                if( newPointer != nullptr )
                {
                    onLessonRename(*newPointer);
                }
                break;
            }

            case gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnLessonDelete:
            {
                auto* newPointer = dynamic_cast<const gui::widget::LessonTreeViewWidget::LessonDataPackage*>(data->getEventData());
                if( newPointer != nullptr )
                {
                    onLessonRemove(*newPointer);
                }
                break;
            }

            default:
                throw std::invalid_argument("Unhandled event type in handleEvent.");
        }
    }

    void EventBridge::onLessonCreated(const gui::widget::LessonTreeViewWidget::LessonDataPackage& dataPackage)
    {
        auto lessons = decodeLessonDataPackage(dataPackage);
        m_app->setEvent(application::ApplicationEvent::OnLessonCreated, lessons);
    }

    void EventBridge::onLessonRename(const gui::widget::LessonTreeViewWidget::LessonDataPackage& dataPackage)
    {
        auto lessons = decodeLessonDataPackage(dataPackage);
        m_app->setEvent(application::ApplicationEvent::OnLessonUpdate, lessons);
    }

    void EventBridge::onLessonRemove(const gui::widget::LessonTreeViewWidget::LessonDataPackage& dataPackage)
    {
        auto lessons = decodeLessonDataPackage(dataPackage);
        m_app->setEvent(application::ApplicationEvent::OnLessonDelete, lessons);
    }

    std::vector<Lesson> EventBridge::decodeLessonDataPackage(const gui::widget::LessonTreeViewWidget::LessonDataPackage& lessonDataPackage)
    {
        auto lessonPackages = lessonDataPackage.get<std::vector<gui::widget::LessonTreeViewWidget::LessonPackage>>(gui::widget::LessonTreeViewWidget::PackageKey::LessonsPackage);

        std::vector<Lesson> lessons;

        for( const auto& lessonPackage : lessonPackages )
        {
            Lesson lesson;
            lesson.id = lessonPackage.get<int>(gui::widget::LessonTreeViewWidget::LessonDataKey::id);
            lesson.mainName = lessonPackage.get<std::string>(gui::widget::LessonTreeViewWidget::LessonDataKey::MainName);
            lesson.subName = lessonPackage.get<std::string>(gui::widget::LessonTreeViewWidget::LessonDataKey::SubName);

            auto wordPackages = lessonPackage.get<std::vector<gui::widget::LessonTreeViewWidget::WordDataPackage>>(gui::widget::LessonTreeViewWidget::LessonDataKey::Words);

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

            lessons.push_back(lesson);
        }

        return lessons;
    }
}

