#pragma once

#include "Widget.h"
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <windows.h>
#include <condition_variable>

namespace tools { class Logger; }

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            class ScriptQuizRunnerWidget : public Widget
            {
            public:
                ScriptQuizRunnerWidget(tools::Logger& logger);
                virtual ~ScriptQuizRunnerWidget();
                virtual void draw(bool* p_open) override;
                void initialize(const tools::DataPackage& r_package) override;

            private:
                bool isPythonScript(const std::string& path);
                std::vector<std::string> getPythonScripts(const std::string& rootPath);
                std::string getFileName(const std::string& fullPath);

                std::vector<std::string> m_scripts;
                tools::Logger& m_logger;

                std::string current_script;
                HANDLE hChildStd_IN_Rd = NULL;
                HANDLE hChildStd_IN_Wr = NULL;
                HANDLE hChildStd_OUT_Rd = NULL;
                HANDLE hChildStd_OUT_Wr = NULL;
                HANDLE hChildProcess = NULL;
                HANDLE stop_event = NULL;
                std::vector<std::string> output;
                char user_input[128] = "";
                void check_script_completion();
                std::thread script_thread;
                std::thread input_thread;
                std::thread output_thread;
                std::mutex output_mutex;
                std::condition_variable cv;
                std::atomic<bool> script_running;
                std::atomic<bool> fetch_output;
                std::atomic<bool> send_input_flag;
                std::atomic<bool> stop_script_flag;
                bool output_thread_done;
                void run_script(const std::string& script_path);
                void stop_script();
                void send_input();
                void fetch_script_output();
                void cleanup();
            };
        }
    }
}
