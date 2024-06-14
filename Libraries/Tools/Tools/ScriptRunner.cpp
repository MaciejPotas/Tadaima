#include "ScriptRunner.h"
#include "Logger.h"
#include <filesystem>
#include <sstream>
#include <chrono>
#include <iostream>
#include <fcntl.h>
#include <io.h>
#include <tchar.h>

namespace tools
{
    ScriptRunner::ScriptRunner(tools::Logger& logger)
        : m_logger(logger), script_running(false), fetch_output(false), stop_script_flag(false), output_thread_done(false)
    {
    }

    ScriptRunner::~ScriptRunner()
    {
        stopScript();
        if( script_thread.joinable() )
        {
            script_thread.join();
        }

        cleanup();
    }

    std::wstring ScriptRunner::string_to_wstring(const std::string& str)
    {
        int len;
        int str_len = (int)str.length() + 1;
        len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str_len, 0, 0);
        std::wstring wstr(len, L'\0');
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), str_len, &wstr[0], len);
        return wstr;
    }

    void ScriptRunner::runScript(const std::string& script_path)
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
        output_thread = std::thread(&ScriptRunner::fetch_script_output, this);
    }

    void ScriptRunner::stopScript()
    {
        if( script_running )
        {
            stop_script_flag = true;
            TerminateProcess(hChildProcess, 1);
            cleanup();
            m_logger.log("Script process terminated", tools::LogLevel::INFO);
        }
    }

    void ScriptRunner::sendInput(const std::string& input)
    {
        if( hChildStd_IN_Wr != NULL )
        {
            DWORD written;
            std::stringstream ss(input);
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
            m_logger.log("Sent input to script: " + input, tools::LogLevel::INFO);
        }
        else
        {
            m_logger.log("No active script process to send input.", tools::LogLevel::WARNING);
        }
    }

    void ScriptRunner::setOutputCallback(OutputCallback callback)
    {
        std::lock_guard<std::mutex> lock(output_mutex);
        output_callback = std::move(callback);
    }

    std::vector<std::string> ScriptRunner::getOutput()
    {
        std::lock_guard<std::mutex> lock(output_mutex);
        return output;
    }

    bool ScriptRunner::isScriptRunning()
    {
        return script_running;
    }

    void ScriptRunner::fetch_script_output()
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
                        std::string outputbuffer(buffer);
                        output.push_back(outputbuffer);
                        if( output_callback )
                        {
                            output_callback(outputbuffer);
                        }
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

    void ScriptRunner::checkScriptCompletion()
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

    void ScriptRunner::cleanup()
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

        m_logger.log("Cleaned up script process resources", tools::LogLevel::INFO);
    }

    void ScriptRunner::clearOutput()
    {
        std::lock_guard<std::mutex> lock(output_mutex);
        output.clear();
        if( output_callback )
        {
            output_callback("");  // Notify the subscriber to clear the output
        }
    }
}