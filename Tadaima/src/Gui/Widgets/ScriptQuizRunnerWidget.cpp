#include "ScriptQuizRunnerWidget.h"
#include "packages/SettingsDataPackage.h"
#include <filesystem>
#include "Tools/Logger.h"
#include "imgui.h"
#include "tools/Dictionary.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <io.h>
#include <tchar.h>

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            ScriptQuizRunnerWidget::ScriptQuizRunnerWidget(tools::Logger& logger)
                : m_logger(logger), script_running(false), fetch_output(false), send_input_flag(false)
            {
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
                    ImGui::SetNextWindowSize(ImVec2(600, 360), ImGuiCond_Always);
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

                    if( ImGui::BeginPopupModal("Script Runner", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize) )
                    {
                        // Script selection
                        ImGui::BeginChild("Scripts", ImVec2(200, 200), true);
                        for( const auto& script : m_scripts )
                        {
                            std::string scriptName = getFileName(script);
                            if( ImGui::Selectable(scriptName.c_str(), current_script == script, ImGuiSelectableFlags_AllowDoubleClick) )
                            {
                                if( ImGui::IsMouseDoubleClicked(0) )
                                {
                                    current_script = script;
                                    m_logger.log("Script selected: " + scriptName, tools::LogLevel::INFO);
                                    if( script_thread.joinable() )
                                    {
                                        script_running = false;
                                        script_thread.join();
                                    }
                                    script_thread = std::thread(&ScriptQuizRunnerWidget::run_script, this, script);
                                }
                            }
                        }
                        ImGui::EndChild();

                        ImGui::SameLine();

                        // Output display
                        ImGui::BeginChild("Output", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 40), true);
                        {
                            std::lock_guard<std::mutex> lock(output_mutex);
                            for( const auto& line : output )
                            {
                                ImGui::TextUnformatted(line.c_str());
                            }
                        }
                        ImGui::EndChild();

                        // Input box
                        ImGui::PushItemWidth(-1);
                        if( ImGui::InputText("##input", user_input, sizeof(user_input), ImGuiInputTextFlags_EnterReturnsTrue) )
                        {
                            send_input_flag = true;
                            m_logger.log("User input ready to send: " + std::string(user_input), tools::LogLevel::INFO);
                        }
                        ImGui::PopItemWidth();

                        // Run script button
                        if( ImGui::Button("Run Script") )
                        {
                            if( !current_script.empty() )
                            {
                                m_logger.log("Running script: " + current_script, tools::LogLevel::INFO);
                                if( script_thread.joinable() )
                                {
                                    script_running = false;
                                    script_thread.join();
                                }
                                script_thread = std::thread(&ScriptQuizRunnerWidget::run_script, this, current_script);
                            }
                            else
                            {
                                m_logger.log("No script selected to run.", tools::LogLevel::WARNING);
                            }
                        }

                        ImGui::EndPopup();
                    }
                    ImGui::PopStyleColor();
                }

                if( send_input_flag )
                {
                    if( input_thread.joinable() )
                    {
                        input_thread.join();
                    }
                    input_thread = std::thread(&ScriptQuizRunnerWidget::send_input, this);
                    send_input_flag = false;
                }
            }

            void ScriptQuizRunnerWidget::run_script(const std::string& script_path)
            {
                if( pipe )
                {
                    pclose(pipe);
                    pipe = nullptr;
                    {
                        std::lock_guard<std::mutex> lock(output_mutex);
                        output.clear();
                    }
                    m_logger.log("Closed previous script process.", tools::LogLevel::INFO);
                }

                const char* pythonPath = "py";
                std::string command = "py " + script_path;
                pipe = _popen(command.c_str(), "r");
                if( !pipe )
                {
                    m_logger.log("Error: Unable to open pipe to Python script: " + script_path, tools::LogLevel::PROBLEM);
                    return;
                }

                m_logger.log("Opened pipe to Python script: " + script_path, tools::LogLevel::INFO);

                script_running = true;
                fetch_output = true;
                std::thread output_thread(&ScriptQuizRunnerWidget::fetch_script_output, this);
                output_thread.detach();
            }

            void ScriptQuizRunnerWidget::send_input()
            {
                if( pipe )
                {
                    std::stringstream ss(user_input);
                    std::string line;
                    while( std::getline(ss, line) )
                    {
                        m_logger.log("Attempting to write to pipe: " + line, tools::LogLevel::DEBUG);
                        int result = fprintf(pipe, "%s\n", line.c_str());
                        fflush(pipe);
                        m_logger.log("Write result: " + std::to_string(result), tools::LogLevel::DEBUG);

                        if( result < 0 )
                        {
                            m_logger.log("Error writing to pipe", tools::LogLevel::PROBLEM);
                            break;
                        }
                    }
                    memset(user_input, 0, sizeof(user_input));
                    m_logger.log("Sent input to script: " + std::string(user_input), tools::LogLevel::INFO);
                }
                else
                {
                    m_logger.log("No active script process to send input.", tools::LogLevel::WARNING);
                }
            }

            void ScriptQuizRunnerWidget::fetch_script_output()
            {
                char buffer[128];
                while( fetch_output && fgets(buffer, sizeof(buffer), pipe) != NULL )
                {
                    {
                        std::lock_guard<std::mutex> lock(output_mutex);
                        output.push_back(buffer);
                    }
                    m_logger.log("Script output: " + std::string(buffer), tools::LogLevel::DEBUG);
                }
                script_running = false;
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
