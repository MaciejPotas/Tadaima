#include "ApplicationSettingsWidget.h"
#include "packages/SettingsDataPackage.h"
#include "imgui.h"
#include "Tools/Logger.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {

            ApplicationSettingsWidget::ApplicationSettingsWidget(tools::Logger& logger) : Widget(Type::ApplicationSettings), m_logger(logger)
            {

            }

            void ApplicationSettingsWidget::Open()
            {
                ImGui::OpenPopup("Application settings");
            }

            quiz::WordType ApplicationSettingsWidget::stringToWordType(const std::string& str)
            {
                static const std::unordered_map<std::string, quiz::WordType> stringToWordTypeMap = {
                    {"BaseWord", quiz::WordType::BaseWord},
                    {"Kana", quiz::WordType::Kana},
                    {"Romaji", quiz::WordType::Romaji}
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

            // Function to convert WordType to string
            std::string ApplicationSettingsWidget::wordTypeToString(quiz::WordType type)
            {
                switch( type )
                {
                    case quiz::WordType::BaseWord: return "BaseWord";
                    case quiz::WordType::Kana: return "Kana";
                    case quiz::WordType::Romaji: return "Romaji";
                    default: throw std::invalid_argument("Invalid WordType value");
                }
            }

            void ApplicationSettingsWidget::ApplySettings()
            {
                m_logger.log("ApplicationSettingsWidget::ApplySettings: changing settings.", tools::LogLevel::INFO);

                try
                {
                    SettingsDataPackage package;
                    package.set(SettingsPackageKey::Username, std::string(m_username));
                    package.set(SettingsPackageKey::DictionaryPath, std::string(m_dictionaryPath));
                    package.set(SettingsPackageKey::QuizzesScriptsPath, std::string(m_scriptPaths));
                    package.set(SettingsPackageKey::AskedWordType, static_cast<quiz::WordType>(m_inputOption));
                    package.set(SettingsPackageKey::AnswerWordType, static_cast<quiz::WordType>(m_translationOption));

                    emitEvent(WidgetEvent(*this, ApplicationSettingsWidgetEvent::OnSettingsChanged, &package));
                }
                catch( std::exception& exception )
                {
                    m_logger.log("Exception caught in ApplicationSettingsWidget::ApplySettings: " + std::string(exception.what()), tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log("Exception caught in ApplicationSettingsWidget::ApplySettings: Unkown problem.", tools::LogLevel::PROBLEM);
                }
            }

            void ApplicationSettingsWidget::initialize(const tools::DataPackage& r_package)
            {
                try
                {
                    const SettingsDataPackage* package = dynamic_cast<const SettingsDataPackage*>(&r_package);
                    if( package )
                    {
                        m_logger.log("ApplicationSettingsWidget::initialize: Initializing.", tools::LogLevel::INFO);

                        const std::string userName = package->get<std::string>(SettingsPackageKey::Username);
                        const std::string dictionaryPath = package->get<std::string>(SettingsPackageKey::DictionaryPath);
                        const std::string quizzesScripts = package->get<std::string>(SettingsPackageKey::QuizzesScriptsPath);

                        memset(m_username, 0, sizeof(m_username));
                        memset(m_dictionaryPath, 0, sizeof(m_dictionaryPath));
                        memset(m_scriptPaths, 0, sizeof(m_scriptPaths));
                        memcpy(m_username, userName.c_str(), userName.size());
                        memcpy(m_dictionaryPath, dictionaryPath.c_str(), dictionaryPath.size());
                        memcpy(m_scriptPaths, quizzesScripts.c_str(), quizzesScripts.size());

                        m_inputOption = package->get< quiz::WordType>(SettingsPackageKey::AskedWordType);
                        m_translationOption = package->get<quiz::WordType>(SettingsPackageKey::AnswerWordType);

                        m_logger.log("ApplicationSettingsWidget: Initialized.", tools::LogLevel::INFO);
                    }
                }
                catch( std::exception& exception )
                {
                    m_logger.log("Exception caught in ApplicationSettingsWidget::initialize: " + std::string(exception.what()), tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log("Exception caught in ApplicationSettingsWidget::initialize.Unkown problem.", tools::LogLevel::PROBLEM);
                }
            }

            void ApplicationSettingsWidget::draw(bool* p_open)
            {
                if( *p_open )
                {
                    Open();
                    ImGui::SetNextWindowSize(ImVec2(550, 380), ImGuiCond_Always);  // Adjust size as needed
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

                    if( ImGui::BeginPopupModal("Application settings", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize) )
                    {
                        // Application Settings Section
                        ImGui::Text("Application Settings");
                        ImGui::Separator();
                        ImGui::Spacing();

                        ImGui::InputText("User name", m_username, IM_ARRAYSIZE(m_username));
                        ShowFieldHelp("Enter your user name for personalized settings.");

                        ImGui::InputText("Path to the directory", m_dictionaryPath, IM_ARRAYSIZE(m_dictionaryPath));
                        ShowFieldHelp("Select the directory where your Japanese word files are stored.");


                        ImGui::InputText("Path to scripted quizes", m_scriptPaths, IM_ARRAYSIZE(m_scriptPaths));
                        ShowFieldHelp("Select the directory where all the scripted quizes are. Usually should be script/quizes.");       

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        // Quiz Settings Section
                        ImGui::Text("Quiz Settings");
                        ImGui::Separator();
                        ImGui::Spacing();

                        const char* translationOptions[] = { "BaseWord", "Kana", "Romaji" };

                        ImGui::Text(" Word option you will write ( Base, if you want to use your language ):");
                        ImGui::Combo("##input_type", &m_inputOption, translationOptions, IM_ARRAYSIZE(translationOptions));
                        ShowFieldHelp("Choose how you want to input words during the quiz.");

                        // Add the Flip button
                        if( ImGui::Button("Flip", ImVec2(120, 0)) )
                        {
                            std::swap(m_inputOption, m_translationOption);
                        }
                        ImGui::SameLine();
                        ShowFieldHelp("Click to swap the word input and translation options.");

                        ImGui::Text(" Word option you will be asked for:");
                        ImGui::Combo("##translation_type", &m_translationOption, translationOptions, IM_ARRAYSIZE(translationOptions));
                        ShowFieldHelp("Choose how you want the words to be translated during the quiz.");

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        if( ImGui::Button("OK", ImVec2(120, 0)) )
                        {
                            ApplySettings();
                            ImGui::CloseCurrentPopup();
                            *p_open = false;
                        }
                        ImGui::SameLine();
                        if( ImGui::Button("Cancel", ImVec2(120, 0)) )
                        {
                            ImGui::CloseCurrentPopup();
                            *p_open = false;
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::PopStyleColor();  // Restore previous style
                }
            }


            void ApplicationSettingsWidget::ShowFieldHelp(const char* desc)
            {
                if( ImGui::IsItemHovered() )
                {
                    // Get the current background color
                    ImVec4 bg_color = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);

                    // Change tooltip background to match the application's background
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, bg_color);
                    ImGui::BeginTooltip();
                    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                    ImGui::TextUnformatted(desc);
                    ImGui::PopTextWrapPos();
                    ImGui::EndTooltip();
                    ImGui::PopStyleColor();
                }
            }
        }
    }
}
