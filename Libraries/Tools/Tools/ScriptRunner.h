/**
 * @file ScriptRunner.h
 * @brief Defines the ScriptRunner class for running and interacting with scripts.
 *
 * The ScriptRunner class provides functionality to execute scripts, handle input and output, and manage
 * script execution using threads and synchronization mechanisms. It allows for real-time interaction
 * with scripts and provides callbacks for output updates.
 */

#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <windows.h>
#include <condition_variable>
#include <functional>

namespace tools
{
    class Logger;

    /**
     * @class ScriptRunner
     * @brief Manages the execution of scripts and handles input/output interactions.
     */
    class ScriptRunner
    {
    public:
        using OutputCallback = std::function<void(const std::string&)>;

        /**
         * @brief Constructor for ScriptRunner.
         * @param logger Reference to a Logger instance for logging purposes.
         */
        ScriptRunner(tools::Logger& logger);

        /**
         * @brief Destructor for ScriptRunner.
         */
        ~ScriptRunner();

        /**
         * @brief Runs a script specified by the script path.
         * @param script_path The path to the script to be executed.
         */
        void runScript(const std::string& script_path);

        /**
         * @brief Stops the currently running script.
         */
        void stopScript();

        /**
         * @brief Sends input to the running script.
         * @param input The input string to send to the script.
         */
        void sendInput(const std::string& input);

        /**
         * @brief Sets the callback function to be called with script output.
         * @param callback The callback function to handle script output.
         */
        void setOutputCallback(OutputCallback callback);

        /**
         * @brief Checks if a script is currently running.
         * @return True if a script is running, false otherwise.
         */
        bool isScriptRunning();

        /**
         * @brief Clears the script output.
         */
        void clearOutput();

        /**
         * @brief Gets the current output of the script.
         * @return A vector of strings containing the script output.
         */
        std::vector<std::string> getOutput();

        /**
         * @brief Checks if the script has completed.
         */
        void checkScriptCompletion();

    private:
        tools::Logger& m_logger; ///< Reference to the logger instance.
        HANDLE hChildStd_IN_Rd = NULL; ///< Handle for reading from the child process's stdin.
        HANDLE hChildStd_IN_Wr = NULL; ///< Handle for writing to the child process's stdin.
        HANDLE hChildStd_OUT_Rd = NULL; ///< Handle for reading from the child process's stdout.
        HANDLE hChildStd_OUT_Wr = NULL; ///< Handle for writing to the child process's stdout.
        HANDLE hChildProcess = NULL; ///< Handle for the child process.
        std::vector<std::string> output; ///< Output from the running script.
        std::thread script_thread; ///< Thread for running the script.
        std::thread input_thread; ///< Thread for sending input to the script.
        std::thread output_thread; ///< Thread for fetching output from the script.
        std::mutex output_mutex; ///< Mutex for protecting access to the output.
        std::condition_variable cv; ///< Condition variable for synchronizing output handling.
        std::atomic<bool> script_running; ///< Flag indicating if a script is running.
        std::atomic<bool> fetch_output; ///< Flag indicating if output should be fetched.
        std::atomic<bool> stop_script_flag; ///< Flag indicating if the script should be stopped.
        bool output_thread_done; ///< Flag indicating if the output thread is done.
        OutputCallback output_callback; ///< Callback function for handling script output.

        /**
         * @brief Fetches the output from the running script.
         */
        void fetch_script_output();

        /**
         * @brief Cleans up resources used by the script process.
         */
        void cleanup();

        /**
         * @brief Converts a string to a wide string.
         * @param str The string to convert.
         * @return The converted wide string.
         */
        std::wstring string_to_wstring(const std::string& str);
    };
}
