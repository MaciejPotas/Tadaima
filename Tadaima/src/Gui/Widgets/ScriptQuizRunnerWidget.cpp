#include "ScriptQuizRunnerWidget.h"
#include "packages/SettingsDataPackage.h"
#include <filesystem>
#include "Tools/Logger.h"
#include "imgui.h"
#include "tools/Dictionary.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            ScriptQuizRunnerWidget::ScriptQuizRunnerWidget(tools::Logger& logger)
                : m_logger(logger), m_scriptRunner(logger), send_input_flag(false)
            {
                m_scriptRunner.setOutputCallback([this](const std::string& output)
                    {
                        std::lock_guard<std::mutex> lock(output_mutex);
                        this->output.push_back(output);
                    });
            }

            ScriptQuizRunnerWidget::~ScriptQuizRunnerWidget()
            {
                m_scriptRunner.stopScript();
            }

            void ScriptQuizRunnerWidget::initialize(const tools::DataPackage& r_package)
            {
                try
                {
                    const SettingsDataPackage* package = dynamic_cast<const SettingsDataPackage*>(&r_package);
                    if( package )
                    {
                        m_logger.log("ScriptQuizRunnerWidget::initialize: Initializing.", tools::LogLevel::INFO);
                        auto scriptPaths = package->get<std::string>(SettingsPackageKey::QuizzesScriptsPath);

                        std::filesystem::path exePath(getexepath());
                        std::filesystem::path script(scriptPaths);
                        auto fullPath = ((exePath) / script).string();

                        m_scripts = getPythonScripts(fullPath);
                    }
                }
                catch( std::exception& exception )
                {
                    m_logger.log("Exception caught in ScriptQuizRunnerWidget::initialize: " + std::string(exception.what()), tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log("Exception caught in ScriptQuizRunnerWidget::initialize. Unknown problem.", tools::LogLevel::PROBLEM);
                }
            }

            void ScriptQuizRunnerWidget::draw(bool* p_open)
            {
                if( *p_open == true )
                {
                    ImGui::OpenPopup("Script Runner");
                    ImGui::SetNextWindowSize(ImVec2(600, 420), ImGuiCond_Always); // Keep the window size

                    // Set the window padding to 10 pixels and apply a cheerful color scheme
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.98f, 0.92f, 0.84f, 1.0f)); // Light peach background
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); // Darker text color

                    if( ImGui::BeginPopupModal("Script Runner", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize) )
                    {
                        // Draw a colorful header with a title and close button
                        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Script Runner");

                        // Close button with a more visually appealing design
                        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 25);
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.6f)); // Semi-transparent red button
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 0.8f)); // More opaque when hovered
                        if( ImGui::Button("X", ImVec2(25, 25)) )
                        {
                            *p_open = false;
                            m_scriptRunner.stopScript();
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::PopStyleColor(2);


                        // Add padding inside the window
                        ImGui::Dummy(ImVec2(0, 10));
                        ImGui::Columns(2, "##columns", false);

                        // Script selection with a cheerful border and scrolling area
                        ImGui::SetColumnWidth(0, 150); // Set the width of the script list column
                        ImGui::BeginChild("Scripts", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                        ImGui::TextColored(ImVec4(0.2f, 0.4f, 0.8f, 1.0f), "Available Scripts");
                        ImGui::Separator();
                        for( const auto& script : m_scripts )
                        {
                            std::string scriptName = getFileName(script);
                            if( m_scriptRunner.isScriptRunning() )
                            {
                                ImGui::BeginDisabled();
                            }

                            if( ImGui::Selectable(scriptName.c_str(), current_script == script, ImGuiSelectableFlags_AllowDoubleClick) )
                            {
                                if( ImGui::IsMouseDoubleClicked(0) )
                                {
                                    if( m_scriptRunner.isScriptRunning() )
                                    {
                                        m_scriptRunner.stopScript();
                                    }

                                    current_script = script;
                                    m_logger.log("Script selected: " + scriptName, tools::LogLevel::INFO);
                                    m_scriptRunner.runScript(script);
                                    if( m_scriptRunner.isScriptRunning() )
                                    {
                                        ImGui::BeginDisabled();
                                    }
                                }
                            }

                            if( ImGui::IsItemHovered() )
                            {
                                ImGui::SetTooltip("%s", scriptName.c_str());
                            }

                            if( m_scriptRunner.isScriptRunning() )
                            {
                                ImGui::EndDisabled();
                            }
                        }
                        ImGui::EndChild();

                        ImGui::NextColumn();

                        // Learning header section
                        ImGui::TextColored(ImVec4(0.2f, 0.4f, 0.8f, 1.0f), "Learn how to run scripts efficiently!");
                        ImGui::TextWrapped("Here, you can select and run scripts. Double-click on a script to start it. The output will be displayed on the right. "
                            "You can input commands in the box below and manage the script execution using the buttons.");


                        // Output display with word wrapping and cheerful text color
                        ImGui::BeginChild("Output", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 40), true);
                        ImGui::TextColored(ImVec4(0.2f, 0.4f, 0.8f, 1.0f), "Script Output");
                        ImGui::Separator();
                        {
                            ImGui::PushTextWrapPos();
                            std::lock_guard<std::mutex> lock(output_mutex);
                            for( const auto& line : output )
                            {
                                ImGui::TextUnformatted(line.c_str());
                            }
                            ImGui::PopTextWrapPos();

                            // Auto-scroll to the bottom
                            if( ImGui::GetScrollY() >= ImGui::GetScrollMaxY() )
                            {
                                ImGui::SetScrollHereY(1.0f);
                            }
                        }
                        ImGui::EndChild();

                        // Input box
                        ImGui::PushItemWidth(-1);
                        if( ImGui::InputText("##input", user_input, sizeof(user_input), ImGuiInputTextFlags_EnterReturnsTrue) )
                        {
                            send_input_flag = true;
                            m_logger.log("User input ready to send: " + std::string(user_input), tools::LogLevel::INFO);
                            ImGui::SetKeyboardFocusHere(-1); // Set focus back to the input field
                        }
                        ImGui::PopItemWidth();

                        // Buttons with icons in one line
                        ImGui::BeginChild("Buttons", ImVec2(0, 40), false);
                        ImGui::PushItemWidth(-1);

                        if( ImGui::Button("Run Script") && !m_scriptRunner.isScriptRunning() )
                        {
                            if( !current_script.empty() )
                            {
                                m_logger.log("Running script: " + current_script, tools::LogLevel::INFO);
                                m_scriptRunner.runScript(current_script);
                            }
                            else
                            {
                                m_logger.log("No script selected to run.", tools::LogLevel::WARNING);
                            }
                        }

                        ImGui::SameLine();

                        if( ImGui::Button("Stop Script") && m_scriptRunner.isScriptRunning() )
                        {
                            m_scriptRunner.stopScript();
                        }

                        ImGui::SameLine();

                        if( ImGui::Button("Clear window") )
                        {
                            std::lock_guard<std::mutex> lock(output_mutex);
                            output.clear();
                        }

                        ImGui::PopItemWidth();
                        ImGui::EndChild();

                        ImGui::EndPopup();
                    }
                    ImGui::PopStyleColor(2);
                    ImGui::PopStyleVar();
                }

                if( send_input_flag )
                {
                    m_scriptRunner.sendInput(user_input);
                    memset(user_input, 0, sizeof(user_input)); // Clear the input field
                    send_input_flag = false;
                }

                // Check if script has finished
                m_scriptRunner.checkScriptCompletion();
            }

            std::string ScriptQuizRunnerWidget::getFileName(const std::string& fullPath)
            {
                std::filesystem::path pathObj(fullPath);
                return pathObj.filename().string();
            }

            bool ScriptQuizRunnerWidget::isPythonScript(const std::string& path)
            {
                return std::filesystem::path(path).extension() == ".py";
            }

            std::vector<std::string> ScriptQuizRunnerWidget::getPythonScripts(const std::string& path)
            {
                std::vector<std::string> pythonScripts;

                for( const auto& entry : std::filesystem::recursive_directory_iterator(path) )
                {
                    if( entry.is_regular_file() && isPythonScript(entry.path().string()) )
                    {
                        pythonScripts.push_back(entry.path().string());
                        m_logger.log("Found Python script: " + entry.path().string(), tools::LogLevel::INFO);
                    }
                }

                return pythonScripts;
            }
        }
    }
}
