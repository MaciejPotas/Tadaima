#pragma once

#include "Widget.h"
#include <string>
#include <vector>
#include <cstdio>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

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
                virtual void draw(bool* p_open) override;
                void initialize(const tools::DataPackage& r_package) override;

            private:
                bool isPythonScript(const std::string& path);
                std::vector<std::string> getPythonScripts(const std::string& rootPath);
                std::string getFileName(const std::string& fullPath);

                std::vector<std::string> m_scripts;
                tools::Logger& m_logger;

                std::string current_script;
                int master_fd;
                FILE* pipe = nullptr;
                std::vector<std::string> output;
                char user_input[128] = "";

                std::thread script_thread;
                std::thread input_thread;
                std::mutex output_mutex;
                std::atomic<bool> script_running;
                std::atomic<bool> fetch_output;
                std::atomic<bool> send_input_flag;

                void run_script(const std::string& script_path);
                void send_input();
                void fetch_script_output();
            };
        }
    }
}
