#pragma once

#include "Word.h"
#include "Conjugations.h"
#include "tools/pugixml.hpp"
#include "tools/SystemTools.h"
#include <string>
#include <array>
#include <memory>
#include <stdexcept>
#include <cstdio>
#include <filesystem>
#include <future>
#include <chrono>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace tadaima
{
    /**
     * @class Dictionary
     * @brief Provides functionalities for translating words and retrieving conjugations using Python scripts.
     *
     * The `Dictionary` class handles communication with external Python scripts for translations and conjugations.
     * It also parses the XML responses and converts them into appropriate data structures.
     */
    class Dictionary
    {
    public:

        /**
         * @brief Sets the file path for the translation script.
         * @param scriptPath The file path to the Python script for translations.
         */
        void setPathForTranslator(const std::string& scriptPath)
        {
            m_translationScriptPath = scriptPath;
        }

        /**
         * @brief Sets the file path for the conjugation script.
         * @param scriptPath The file path to the Python script for conjugations.
         */
        void setPathForConjugationTranslator(const std::string& scriptPath)
        {
            m_conjugationScriptPath = scriptPath;
        }

        /**
         * @brief Translates a given word into multiple formats (kanji, kana, romaji, etc.).
         * @param wordToTranslate The word to translate.
         * @return A `Word` object containing the translated forms and additional information.
         * @throws std::runtime_error if translation or XML parsing fails.
         */
        Word getTranslation(const std::string& wordToTranslate)
        {
            translator.setPathForTranslator(m_translationScriptPath);
            std::string xmlStr = translator.translate(wordToTranslate);
            return parseTranslationXml(xmlStr);
        }

        /**
          * @brief Retrieves conjugations for a given word.
          * @param wordToConjugate The word to conjugate.
          * @return An array of strings representing the conjugations for each `ConjugationType`.
          * @throws std::runtime_error if conjugation or XML parsing fails.
          */
        std::array<std::string, CONJUGATION_COUNT> getConjugations(const std::string& wordToConjugate)
        {
            translator.setPathForTranslator(m_conjugationScriptPath);
            std::string xmlStr = translator.translate(wordToConjugate);
            return parseConjugationsXml(xmlStr);
        }

    private:

        /**
         * @class PythonTranslator
         * @brief Handles communication with Python scripts for translations and conjugations.
         */
        class PythonTranslator
        {
        public:

            /**
             * @brief Sets the file path for the translation script.
             * @param scriptPath The file path to the Python script.
             */
            void setPathForTranslator(const std::string& scriptPath)
            {
                std::filesystem::path exePath(getexepath());
                std::filesystem::path script(scriptPath);
                m_translationScriptPath = ((exePath) / scriptPath).string();
            }

            /**
             * @brief Executes the translation or conjugation script with the given input.
             * @param input The word to translate or conjugate.
             * @return The script output as a string.
             * @throws std::runtime_error if the script execution fails.
             */
            std::string translate(const std::string& input)
            {
                if( !m_translationScriptPath.empty() )
                {
                    std::string command = "python " + m_translationScriptPath + " " + input;
                    return exec(command.c_str());
                }
                return "";
            }

        private:

            /**
             * @brief Executes a shell command and captures its output.
             * @param cmd The command to execute.
             * @param timeoutSeconds The timeout for the command in seconds.
             * @return The output of the command as a string.
             * @throws std::runtime_error if the command fails or times out.
             */
            std::string exec(const char* cmd, int timeoutSeconds = 10)
            {
                auto future = std::async(std::launch::async, [cmd]()
                    {
                        std::array<char, 5000> buffer;
                        std::string result;
                        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
                        if( !pipe )
                            throw std::runtime_error("popen() failed!");
                        while( fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr )
                        {
                            result += buffer.data();
                        }
                        int returnCode = pclose(pipe.release());
                        if( returnCode != 0 )
                            throw std::runtime_error("Command failed with code " + std::to_string(returnCode));
                        return result;
                    });

                if( future.wait_for(std::chrono::seconds(timeoutSeconds)) == std::future_status::timeout )
                    throw std::runtime_error("Command timed out!");

                return future.get();
            }

            std::string m_translationScriptPath;
            std::string m_conjugationScriptPath;
        };

        /**
         * @brief Parses the XML string returned by the translation script.
         * @param xmlStr The XML string to parse.
         * @return A `Word` object populated with translation data.
         * @throws std::runtime_error if XML parsing fails.
         */
        Word parseTranslationXml(const std::string& xmlStr)
        {
            pugi::xml_document doc;
            if( !doc.load_string(xmlStr.c_str()) )
                throw std::runtime_error("Failed to parse translation XML.");

            Word word;
            word.translation = doc.child("translation").child("trs").child_value();
            word.kana = doc.child("translation").child("hiragana").child_value();
            word.kanji = doc.child("translation").child("kanji").child_value();
            word.romaji = doc.child("translation").child("romaji").child_value();
            word.exampleSentence = doc.child("translation").child("example").child_value();
            return word;
        }

        /**
         * @brief Parses the XML string returned by the conjugation script.
         * @param xmlStr The XML string to parse.
         * @return An array of strings containing conjugations for each `ConjugationType`.
         * @throws std::runtime_error if XML parsing fails.
         */
        std::array<std::string, CONJUGATION_COUNT> parseConjugationsXml(const std::string& xmlStr)
        {
            pugi::xml_document doc;
            if( !doc.load_string(xmlStr.c_str()) )
                throw std::runtime_error("Failed to parse conjugation XML.");

            std::array<std::string, CONJUGATION_COUNT> conjugations;
            conjugations.fill("");

            // Find all <tense> elements under <conjugations>
            auto tenseNodes = doc.child("conjugations").children("tense");
            int index = 0;

            for( auto& tenseNode : tenseNodes )
            {
                if( index >= CONJUGATION_COUNT )
                    break;

                conjugations[index] = tenseNode.child("romaji").child_value();
                ++index;
            }

            return conjugations;
        }

        PythonTranslator translator;
        std::string m_translationScriptPath;
        std::string m_conjugationScriptPath;
    };
}
