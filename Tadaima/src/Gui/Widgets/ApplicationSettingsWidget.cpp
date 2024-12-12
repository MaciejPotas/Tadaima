#pragma once

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

            ApplicationSettingsWidget::ApplicationSettingsWidget(tools::Logger& logger)
                : Widget(Type::ApplicationSettings), m_logger(logger)
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
                    package.set(SettingsPackageKey::ShowLogs, m_showlogs);
                    package.set(SettingsPackageKey::TriesForQuiz, std::to_string(m_numberOfTries));

                    emitEvent(WidgetEvent(*this, ApplicationSettingsWidgetEvent::OnSettingsChanged, &package));
                }
                catch( std::exception& exception )
                {
                    m_logger.log("Exception caught in ApplicationSettingsWidget::ApplySettings: " + std::string(exception.what()), tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log("Exception caught in ApplicationSettingsWidget::ApplySettings: Unknown problem.", tools::LogLevel::PROBLEM);
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
                        const std::string numberOfTries = package->get<std::string>(SettingsPackageKey::TriesForQuiz);

                        memset(m_username, 0, sizeof(m_username));
                        memset(m_dictionaryPath, 0, sizeof(m_dictionaryPath));
                        memset(m_scriptPaths, 0, sizeof(m_scriptPaths));
                        memcpy(m_username, userName.c_str(), userName.size());
                        memcpy(m_dictionaryPath, dictionaryPath.c_str(), dictionaryPath.size());
                        memcpy(m_scriptPaths, quizzesScripts.c_str(), quizzesScripts.size());

                        m_inputOption = package->get<quiz::WordType>(SettingsPackageKey::AskedWordType);
                        m_translationOption = package->get<quiz::WordType>(SettingsPackageKey::AnswerWordType);
                        m_numberOfTries = std::stoi(numberOfTries);
                        m_showlogs = package->get<bool>(SettingsPackageKey::ShowLogs);

                        m_logger.log("ApplicationSettingsWidget: Initialized.", tools::LogLevel::INFO);
                    }
                }
                catch( std::exception& exception )
                {
                    m_logger.log("Exception caught in ApplicationSettingsWidget::initialize: " + std::string(exception.what()), tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log("Exception caught in ApplicationSettingsWidget::initialize: Unknown problem.", tools::LogLevel::PROBLEM);
                }
            }

            void ApplicationSettingsWidget::draw(bool* p_open)
            {
                if( *p_open )
                {
                    Open();
                    ImGui::SetNextWindowSize(ImVec2(550, 330), ImGuiCond_Always);  // Adjust size as needed
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.98f, 0.92f, 0.84f, 1.0f)); // Light peach background
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10)); // Add padding

                    if( ImGui::BeginPopupModal("Application settings", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize) )
                    {
                        if( ImGui::BeginTabBar("SettingsTabs") )
                        {
                            if( ImGui::BeginTabItem("General Settings") )
                            {
                                // General Settings Section
                                ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "General Settings");
                                ImGui::Separator();
                                ImGui::Spacing();

                                ImGui::InputText("##Username", m_username, IM_ARRAYSIZE(m_username));
                                ImGui::SameLine();
                                ImGui::Text("User name");
                                ShowFieldHelp("Enter your user name for personalized settings.");

                                ImGui::InputText("##DictionaryPath", m_dictionaryPath, IM_ARRAYSIZE(m_dictionaryPath));
                                ImGui::SameLine();
                                ImGui::Text("Path to the directory");
                                ShowFieldHelp("Select the directory where your Japanese word files are stored.");

                                ImGui::InputText("##ScriptPaths", m_scriptPaths, IM_ARRAYSIZE(m_scriptPaths));
                                ImGui::SameLine();
                                ImGui::Text("Path to scripted quizzes");
                                ShowFieldHelp("Select the directory where all the scripted quizzes are. Usually should be script/quizzes.");

                                ImGui::Checkbox("Show Logs", &m_showlogs);
                                ShowFieldHelp("Enable or disable logging.");

                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Spacing();

                                ImGui::EndTabItem();
                            }

                            if( ImGui::BeginTabItem("Quiz Settings") )
                            {
                                // Quiz Settings Section
                                ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Quiz Settings");
                                ImGui::Separator();
                                ImGui::Spacing();

                                const char* translationOptions[] = { "BaseWord", "Kana", "Romaji" };

                                ImGui::Text("Word option you will write (Base, if you want to use your language):");
                                ImGui::Combo("##input_type", &m_inputOption, translationOptions, IM_ARRAYSIZE(translationOptions));
                                ShowFieldHelp("Choose how you want to input words during the quiz.");

                                // Add the Flip button
                                if( ImGui::Button("Flip", ImVec2(120, 0)) )
                                {
                                    std::swap(m_inputOption, m_translationOption);
                                }
                                ImGui::SameLine();
                                ShowFieldHelp("Click to swap the word input and translation options.");

                                ImGui::Text("Word option you will be asked for:");
                                ImGui::Combo("##translation_type", &m_translationOption, translationOptions, IM_ARRAYSIZE(translationOptions));
                                ShowFieldHelp("Choose how you want the words to be translated during the quiz.");

                                ImGui::Text("Set the number of tries allowed for each word during the quiz.");
                                ImGui::SliderInt(" ", &m_numberOfTries, 1, 10, "%d");

                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Spacing();

                                ImGui::EndTabItem();
                            }

                            ImGui::EndTabBar();
                        }

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
                    ImGui::PopStyleVar();  // Restore previous padding
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
