/**
 * @file ScriptQuizRunnerWidget.h
 * @brief Defines the ScriptQuizRunnerWidget class for running and interacting with script-based quizzes.
 *
 * The ScriptQuizRunnerWidget class provides a graphical interface for selecting and running Python scripts.
 * It allows users to select a script from a list, execute it, and interact with it by sending input and
 * receiving output in real-time. The class utilizes ImGui for the graphical interface and manages the
 * script execution using the ScriptRunner class.
 */

#pragma once

#include "Widget.h"
#include "Tools/ScriptRunner.h"
#include <string>
#include <vector>
#include <atomic>

namespace tools { class Logger; }

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @class ScriptQuizRunnerWidget
             * @brief A widget for running and interacting with script-based quizzes.
             */
            class ScriptQuizRunnerWidget : public Widget
            {
            public:
                /**
                 * @brief Constructor for ScriptQuizRunnerWidget.
                 * @param logger Reference to a Logger instance for logging purposes.
                 */
                ScriptQuizRunnerWidget(tools::Logger& logger);

                /**
                 * @brief Destructor for ScriptQuizRunnerWidget.
                 */
                virtual ~ScriptQuizRunnerWidget();

                /**
                 * @brief Draw the widget.
                 * @param p_open Pointer to a boolean value indicating whether the widget is open.
                 */
                virtual void draw(bool* p_open) override;

                /**
                 * @brief Initialize the widget with a data package.
                 * @param r_package Reference to the data package.
                 */
                void initialize(const tools::DataPackage& r_package) override;

            private:
                /**
                 * @brief Check if a given path points to a Python script.
                 * @param path The file path to check.
                 * @return True if the path points to a Python script, false otherwise.
                 */
                bool isPythonScript(const std::string& path);

                /**
                 * @brief Get a list of Python scripts from a given root path.
                 * @param rootPath The root directory to search for Python scripts.
                 * @return A vector of Python script file paths.
                 */
                std::vector<std::string> getPythonScripts(const std::string& rootPath);

                /**
                 * @brief Get the file name from a full path.
                 * @param fullPath The full file path.
                 * @return The file name extracted from the path.
                 */
                std::string getFileName(const std::string& fullPath);

                std::vector<std::string> m_scripts; ///< List of available scripts.
                tools::Logger& m_logger; ///< Reference to the logger instance.
                tools::ScriptRunner m_scriptRunner; ///< Instance of the ScriptRunner for running scripts.
                std::vector<std::string> output; ///< Output from the running script.
                std::string current_script; ///< The currently selected script.
                std::mutex output_mutex; ///< Mutex for protecting access to the output.
                std::atomic<bool> send_input_flag; ///< Flag indicating if input should be sent to the script.
                char user_input[128] = ""; ///< Buffer for user input.
            };
        }
    }
}