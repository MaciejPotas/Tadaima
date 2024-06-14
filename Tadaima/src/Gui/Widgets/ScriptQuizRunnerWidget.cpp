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
                : m_logger(logger), script_running(false), fetch_output(false), send_input_flag(false), stop_script_flag(false), output_thread_done(false)
            {
            }

            ScriptQuizRunnerWidget::~ScriptQuizRunnerWidget()
            {
                stop_script();
                if( script_thread.joinable() )
                {
                    script_thread.join();
                }

                cleanup();
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
                    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);

                    // Set the window padding to 10 pixels
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

                    if( ImGui::BeginPopupModal("Script Runner", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize) )
                    {
                        // Add padding inside the window
                        ImGui::Dummy(ImVec2(0, 10));
                        ImGui::Columns(2);
                        ImGui::SetColumnWidth(0, 200); // Set the width of the first column to 200 pixels (1/4 of 800)

                        // Script selection
                        ImGui::BeginChild("Scripts", ImVec2(0, 0), true);
                        ImGui::Text("Available Scripts");
                        ImGui::Separator();
                        for( const auto& script : m_scripts )
                        {
                            std::string scriptName = getFileName(script);
                            if( script_running )
                            {
                                ImGui::BeginDisabled();
                            }

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

                            if( script_running )
                            {
                                ImGui::EndDisabled();
                            }
                        }
                        ImGui::EndChild();

                        ImGui::NextColumn();

                        // Output display
                        ImGui::BeginChild("Output", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 40), true);
                        ImGui::Text("Script Output");
                        ImGui::Separator();
                        {
                            std::lock_guard<std::mutex> lock(output_mutex);
                            for( const auto& line : output )
                            {
                                ImGui::Text(line.c_str());
                            }

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

                        // Buttons in one line
                        ImGui::BeginChild("Buttons", ImVec2(0, 40), false);
                        ImGui::PushItemWidth(-1);

                        if( ImGui::Button("Run Script") && !script_running )
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

                        ImGui::SameLine();

                        if( ImGui::Button("Stop Script") && script_running )
                        {
                            stop_script();
                        }

                        ImGui::SameLine();

                        if( ImGui::Button("Clear window") )
                        {
                            output.clear();
                        }

                        ImGui::PopItemWidth();
                        ImGui::EndChild();

                        ImGui::EndPopup();
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();
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

                // Check if script has finished
                check_script_completion();
            }



            void ScriptQuizRunnerWidget::check_script_completion()
            {
                if( script_running && WaitForSingleObject(hChildProcess, 0) == WAIT_OBJECT_0 )
                {
                    script_running = false;
                    fetch_output = false;

                    // Add a small delay to ensure all output is captured
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));

                    if( output_thread.joinable() )
                    {
                        output_thread.join();
                    }
                    cleanup();
                }
            }

            std::wstring string_to_wstring(const std::string& str)
            {
                int len;
                int str_len = (int)str.length() + 1;
                len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str_len, 0, 0);
                std::wstring wstr(len, L'\0');
                MultiByteToWideChar(CP_ACP, 0, str.c_str(), str_len, &wstr[0], len);
                return wstr;
            }

            void ScriptQuizRunnerWidget::run_script(const std::string& script_path)
            {
                cleanup();

                SECURITY_ATTRIBUTES saAttr;
                saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
                saAttr.bInheritHandle = TRUE;
                saAttr.lpSecurityDescriptor = NULL;

                // Create named pipe for the child process's STDOUT with FILE_FLAG_OVERLAPPED.
                hChildStd_OUT_Rd = CreateNamedPipe(
                    TEXT("\\\\.\\pipe\\ChildStdOUT"),
                    PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                    1, 4096, 4096, 0, &saAttr);

                hChildStd_OUT_Wr = CreateFile(
                    TEXT("\\\\.\\pipe\\ChildStdOUT"),
                    GENERIC_WRITE,
                    0, &saAttr, OPEN_EXISTING, 0, NULL);

                if( hChildStd_OUT_Rd == INVALID_HANDLE_VALUE || hChildStd_OUT_Wr == INVALID_HANDLE_VALUE )
                {
                    m_logger.log("CreateNamedPipe for stdout failed with error: " + std::to_string(GetLastError()), tools::LogLevel::PROBLEM);
                    return;
                }

                // Ensure the read handle to the pipe for STDOUT is not inherited.
                if( !SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
                {
                    m_logger.log("Stdout SetHandleInformation failed with error: " + std::to_string(GetLastError()), tools::LogLevel::PROBLEM);
                    return;
                }

                // Create named pipe for the child process's STDIN with FILE_FLAG_OVERLAPPED.
                hChildStd_IN_Wr = CreateNamedPipe(
                    TEXT("\\\\.\\pipe\\ChildStdIN"),
                    PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
                    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                    1, 4096, 4096, 0, &saAttr);

                hChildStd_IN_Rd = CreateFile(
                    TEXT("\\\\.\\pipe\\ChildStdIN"),
                    GENERIC_READ,
                    0, &saAttr, OPEN_EXISTING, 0, NULL);

                if( hChildStd_IN_Rd == INVALID_HANDLE_VALUE || hChildStd_IN_Wr == INVALID_HANDLE_VALUE )
                {
                    m_logger.log("CreateNamedPipe for stdin failed with error: " + std::to_string(GetLastError()), tools::LogLevel::PROBLEM);
                    return;
                }

                // Ensure the write handle to the pipe for STDIN is not inherited.
                if( !SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
                {
                    m_logger.log("Stdin SetHandleInformation failed with error: " + std::to_string(GetLastError()), tools::LogLevel::PROBLEM);
                    return;
                }

                // Create the child process.
                PROCESS_INFORMATION piProcInfo;
                STARTUPINFO siStartInfo;
                bool bSuccess = FALSE;

                // Set up members of the PROCESS_INFORMATION structure.
                ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

                // Set up members of the STARTUPINFO structure.
                ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
                siStartInfo.cb = sizeof(STARTUPINFO);
                siStartInfo.hStdError = hChildStd_OUT_Wr;
                siStartInfo.hStdOutput = hChildStd_OUT_Wr;
                siStartInfo.hStdInput = hChildStd_IN_Rd;
                siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

                // Create the child process.
                std::string command = "python " + script_path;
                std::wstring wcommand = string_to_wstring(command);

                bSuccess = CreateProcessW(NULL,
                    &wcommand[0],   // command line
                    NULL,          // process security attributes
                    NULL,          // primary thread security attributes
                    TRUE,          // handles are inherited
                    CREATE_NO_WINDOW,  // creation flags
                    NULL,          // use parent's environment
                    NULL,          // use parent's current directory
                    &siStartInfo,  // STARTUPINFO pointer
                    &piProcInfo);  // receives PROCESS_INFORMATION

                // If an error occurs, exit the application.
                if( !bSuccess )
                {
                    m_logger.log("CreateProcess failed with error: " + std::to_string(GetLastError()), tools::LogLevel::PROBLEM);
                    return;
                }
                else
                {
                    // Close handles to the child process and its primary thread.
                    // Some applications might keep these handles to monitor the status
                    // of the child process.
                    hChildProcess = piProcInfo.hProcess;
                    CloseHandle(piProcInfo.hThread);
                }

                m_logger.log("Opened pipe to Python script: " + script_path, tools::LogLevel::INFO);

                script_running = true;
                fetch_output = true;
                output_thread_done = false;
                output_thread = std::thread(&ScriptQuizRunnerWidget::fetch_script_output, this);
            }

            void ScriptQuizRunnerWidget::stop_script()
            {
                if( script_running )
                {
                    stop_script_flag = true;
                    TerminateProcess(hChildProcess, 1);
                    cleanup();
                    m_logger.log("Script process terminated", tools::LogLevel::INFO);
                }
            }

            void ScriptQuizRunnerWidget::send_input()
            {
                if( hChildStd_IN_Wr != NULL )
                {
                    DWORD written;
                    std::stringstream ss(user_input);
                    std::string line;
                    while( std::getline(ss, line) )
                    {
                        m_logger.log("Attempting to write to pipe: " + line, tools::LogLevel::DEBUG);
                        line += "\n";
                        if( !WriteFile(hChildStd_IN_Wr, line.c_str(), line.size(), &written, NULL) )
                        {
                            m_logger.log("Error writing to pipe with error: " + std::to_string(GetLastError()), tools::LogLevel::PROBLEM);
                            break;
                        }
                    }
                    FlushFileBuffers(hChildStd_IN_Wr);  // Ensure the input is flushed
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
                DWORD read;
                CHAR buffer[4096];  // Increased buffer size to capture more data
                DWORD available;

                while( fetch_output )
                {
                    if( PeekNamedPipe(hChildStd_OUT_Rd, NULL, 0, NULL, &available, NULL) && available > 0 )
                    {
                        if( ReadFile(hChildStd_OUT_Rd, buffer, sizeof(buffer) - 1, &read, NULL) || GetLastError() == ERROR_MORE_DATA )
                        {
                            buffer[read] = '\0';  // Null-terminate the buffer
                            {
                                std::lock_guard<std::mutex> lock(output_mutex);
                                output.push_back(buffer);  // Push the captured output to the vector
                            }
                            m_logger.log("Script output: " + std::string(buffer), tools::LogLevel::DEBUG);
                        }
                        else
                        {
                            DWORD error = GetLastError();
                            m_logger.log("ReadFile failed with error: " + std::to_string(error), tools::LogLevel::PROBLEM);
                            break;
                        }
                    }
                    else
                    {
                        // Sleep for a short time to prevent busy waiting
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }

                {
                    std::lock_guard<std::mutex> lock(output_mutex);
                    output_thread_done = true;
                }
                cv.notify_all();
                script_running = false;
            }

            void ScriptQuizRunnerWidget::cleanup()
            {
                fetch_output = false;

                if( hChildStd_IN_Wr != NULL )
                {
                    CloseHandle(hChildStd_IN_Wr);
                    hChildStd_IN_Wr = NULL;
                }
                if( hChildStd_OUT_Rd != NULL )
                {
                    std::unique_lock<std::mutex> lock(output_mutex);
                    cv.wait(lock, [this] { return output_thread_done; });
                    CloseHandle(hChildStd_OUT_Rd);
                    hChildStd_OUT_Rd = NULL;
                }
                if( hChildProcess != NULL )
                {
                    CloseHandle(hChildProcess);
                    hChildProcess = NULL;
                }

                if( output_thread.joinable() )
                {
                    output_thread.join();
                }

                if( input_thread.joinable() )
                {
                    input_thread.join();
                }


                m_logger.log("Cleaned up script process resources", tools::LogLevel::INFO);
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
