#include "EventBridge.h"
#include "application/Application.h"
#include "Gui/Gui.h"
#include <stdexcept>
#include "widgets/packages/LessonDataPackage.h"
#include "Widgets/LessonTreeViewWidget.h"
#include "widgets/packages/SettingsDataPackage.h"
#include "widgets/ApplicationSettingsWidget.h"

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
        gui::widget::LessonDataPackage lessonsPackage(lessons);
        m_gui->initializeWidget(lessonsPackage);
    }

    void EventBridge::initializeSettings(const application::ApplicationSettings& settings)
    {
        gui::widget::SettingsDataPackage package;

        package.set(gui::widget::SettingsPackageKey::Username, settings.userName);
        package.set(gui::widget::SettingsPackageKey::DictionaryPath, settings.dictionaryPath);
        package.set(gui::widget::SettingsPackageKey::ConjugationPath, settings.conjugationPath);
        package.set(gui::widget::SettingsPackageKey::QuizzesScriptsPath, settings.quizzesPaths);
        package.set(gui::widget::SettingsPackageKey::AskedWordType, stringToWordType(settings.inputWord));
        package.set(gui::widget::SettingsPackageKey::AnswerWordType, stringToWordType(settings.translatedWord));
        package.set(gui::widget::SettingsPackageKey::ShowLogs, settings.showLogs);
        package.set(gui::widget::SettingsPackageKey::TriesForQuiz, settings.maxTriesForQuiz);
        package.set(gui::widget::SettingsPackageKey::ConjugationMask, settings.conjugationMask);

        m_gui->initializeWidget(package);
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
        const gui::widget::LessonDataPackage* package = dynamic_cast<const gui::widget::LessonDataPackage*>(dataPackage);
        if( nullptr != package )
        {
            m_app->setEvent(application::ApplicationEvent::OnLessonCreated, package->decode());
        }
    }

    void EventBridge::onLessonRename(const tools::DataPackage* dataPackage)
    {
        const gui::widget::LessonDataPackage* package = dynamic_cast<const gui::widget::LessonDataPackage*>(dataPackage);
        if( nullptr != package )
        {
            m_app->setEvent(application::ApplicationEvent::OnLessonUpdate, package->decode());
        }
    }

    void EventBridge::onLessonRemove(const tools::DataPackage* dataPackage)
    {
        const gui::widget::LessonDataPackage* package = dynamic_cast<const gui::widget::LessonDataPackage*>(dataPackage);
        if( nullptr != package )
        {
            m_app->setEvent(application::ApplicationEvent::OnLessonDelete, package->decode());
        }
    }

    void EventBridge::onLessonEdited(const tools::DataPackage* dataPackage)
    {
        const gui::widget::LessonDataPackage* package = dynamic_cast<const gui::widget::LessonDataPackage*>(dataPackage);
        if( nullptr != package )
        {
            m_app->setEvent(application::ApplicationEvent::OnLessonEdited, package->decode());
        }
    }

    void EventBridge::onSettingsChanged(const tools::DataPackage* dataPackage)
    {
        const gui::widget::SettingsDataPackage* package = dynamic_cast<const gui::widget::SettingsDataPackage*>(dataPackage);
        if( nullptr != package )
        {
            application::ApplicationSettings settings;
            settings.userName = package->get<std::string>(gui::widget::SettingsPackageKey::Username);
            settings.dictionaryPath = package->get<std::string>(gui::widget::SettingsPackageKey::DictionaryPath);
            settings.conjugationPath = package->get<std::string>(gui::widget::SettingsPackageKey::ConjugationPath);
            settings.quizzesPaths = package->get<std::string>(gui::widget::SettingsPackageKey::QuizzesScriptsPath);
            settings.inputWord = wordTypeToString(package->get<gui::quiz::WordType>(gui::widget::SettingsPackageKey::AskedWordType));
            settings.translatedWord = wordTypeToString(package->get<gui::quiz::WordType>(gui::widget::SettingsPackageKey::AnswerWordType));
            settings.showLogs = package->get<bool>(gui::widget::SettingsPackageKey::ShowLogs);
            settings.maxTriesForQuiz = package->get<std::string>(gui::widget::SettingsPackageKey::TriesForQuiz);
            settings.conjugationMask = package->get<uint16_t>(gui::widget::SettingsPackageKey::ConjugationMask);

            m_app->setEvent(application::ApplicationEvent::OnSettingsChanged, settings);
        }
    }

    gui::quiz::WordType EventBridge::stringToWordType(const std::string& str)
    {
        static const std::unordered_map<std::string, gui::quiz::WordType> stringToWordTypeMap = {
            {"BaseWord", gui::quiz::WordType::BaseWord},
            {"Kana", gui::quiz::WordType::Kana},
            {"Romaji", gui::quiz::WordType::Romaji}
        };

        auto it = stringToWordTypeMap.find(str);
        if( it != stringToWordTypeMap.end() )
        {
            return it->second;
        }
        else
        {
            throw std::invalid_argument("Invalid WordType string: " + str);
        }
    }

    std::string EventBridge::wordTypeToString(gui::quiz::WordType type)
    {
        switch( type )
        {
            case gui::quiz::WordType::BaseWord: return "BaseWord";
            case gui::quiz::WordType::Kana: return "Kana";
            case gui::quiz::WordType::Romaji: return "Romaji";
            default: throw std::invalid_argument("Invalid WordType value");
        }
    }
}

