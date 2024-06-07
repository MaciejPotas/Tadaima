/**
 * @file PythonTranslator.h
 * @brief Contains the declaration of the PythonTranslator and Dictionary classes used for translating words using a Python script and handling the translations.
 */

#include <string>
#include <array>
#include <memory>
#include <stdexcept>
#include <cstdio>
#include <unordered_map>
#include "tools/pugixml.hpp"
#include "tools/SystemTools.h"
#include "Lessons/Lesson.h"
#include <filesystem>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace tadaima
{
    /**
     * @class Dictionary
     * @brief Class responsible for managing translations and caching them.
     */
    class Dictionary
    {
    public:
        /**
         * @brief Sets the path for the translator script.
         * @param scriptPath The relative path to the Python script used for translation.
         */
        void setPathForTranslator(const std::string& scriptPath)
        {
            translator.setPathForTranslator(scriptPath);
        }

        /**
         * @brief Gets the translation of an English word.
         * @param englishWord The English word to be translated.
         * @return A Word object containing the translation and related information.
         */
        Word getTranslation(const std::string& englishWord)
        {
            // Translate the word and store it in the cache
            std::string xmlStr = translator.translate(englishWord);
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_string(xmlStr.c_str());
            if( !result )
            {
                throw std::runtime_error("Failed to parse XML: " + std::string(result.description()));
            }

            // Extract data from the XML
            pugi::xml_node root = doc.child("translation");
            if( !root )
            {
                throw std::runtime_error("Invalid XML format");
            }

            Word word;
            word.translation = root.child("trs").child_value();
            word.romaji = root.child("romaji").child_value();
            word.kana = root.child("hiragana").child_value();
            word.exampleSentence = ""; // This example doesn't provide sentences
            word.tags = {}; // This example doesn't provide tags

            return word;
        }

    private:

        /**
         * @class PythonTranslator
         * @brief Class responsible for handling the translation of words using a Python script.
         */
        class PythonTranslator
        {
        public:
            /**
             * @brief Sets the path for the translator script.
             * @param scriptPath The relative path to the Python script used for translation.
             */
            void setPathForTranslator(const std::string& scriptPath)
            {
                std::filesystem::path exePath(getexepath());
                std::filesystem::path script(scriptPath);
                m_venvPath = (exePath / std::string("scripts\\venv")).string();
                m_scriptPath = ((exePath) / scriptPath).string();
            }

            /**
             * @brief Translates a given word by executing the Python script.
             * @param word The word to be translated.
             * @return The translation of the word.
             */
            std::string translate(const std::string& word)
            {
                if( !m_scriptPath.empty() )
                {
                    std::string command;
                    if( !m_venvPath.empty() )
                    {
                        if( std::filesystem::exists(m_venvPath) )
                        {
#ifdef _WIN32
                            command = m_venvPath + "\\Scripts\\activate && python " + m_scriptPath + " " + word;
#else
                            command = ". " + m_venvPath + "/bin/activate && python " + m_scriptPath + " " + word;
#endif
                        }
                        else
                        {
                            throw std::runtime_error("Virtual environment path does not exist!");
                        }
                    }
                    else
                    {
                        command = "python " + m_scriptPath + " " + word;
                    }
                    return exec(command.c_str());
                }

                return "";
            }

        private:
            /**
             * @brief Executes a system command and returns the output.
             * @param cmd The command to be executed.
             * @return The output of the command execution.
             */
            std::string exec(const char* cmd)
            {
                std::array<char, 128> buffer;
                std::string result;
                std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
                if( !pipe )
                {
                    throw std::runtime_error("popen() failed!");
                }
                while( fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr )
                {
                    result += buffer.data();
                }
                int returnCode = pclose(pipe.release());
                if( returnCode != 0 )
                {
                    throw std::runtime_error("Command execution failed with code " + std::to_string(returnCode));
                }
                return result;
            }

            std::string m_scriptPath; ///< Path to the Python script used for translation.
            std::string m_venvPath; ///< Path to the virtual environment.
        };

        PythonTranslator translator; ///< Instance of PythonTranslator to handle translations.
        std::unordered_map<std::string, std::unique_ptr<pugi::xml_document>> cache; ///< Cache for storing translations.
    };

}
