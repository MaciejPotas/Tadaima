/**
 * @file ApplicationSettings.h
 * @brief Defines the ApplicationSettings struct for managing application and quiz settings.
 */

#pragma once

#include <format>
#include <string>

namespace tadaima
{
    namespace application
    {
        /**
         * @brief The ApplicationSettings struct holds the settings for the application and quiz.
         */
        struct ApplicationSettings
        {
            /// Constants for default values
            static constexpr const char* DEFAULT_USER_NAME = "";
            static constexpr const char* DEFAULT_DICTIONARY_PATH = "scripts/Dictionaries/Tangorin.py";
            static constexpr const char* DEFAULT_CONJUGATION_PATH = "scripts/Dictionaries/Conjugation.py";
            static constexpr const char* DEFAULT_SCRIPTED_QUIZ_PATH = "scripts/Quizzes";
            static constexpr const char* DEFAULT_INPUT_WORD = "BaseWord";
            static constexpr const char* DEFAULT_TRANSLATED_WORD = "Romaji";
            static constexpr const char* DEFAULT_MAX_TRIES_FOR_QUIZ = "2";
            static constexpr const uint8_t DEFAULT_CONJUGATION_MASK = 0;

            /// Application settings
            std::string userName = DEFAULT_USER_NAME;           /**< The username for the application. */
            std::string dictionaryPath = DEFAULT_DICTIONARY_PATH; /**< The path to the dictionary file. */
            std::string conjugationPath = DEFAULT_CONJUGATION_PATH; /**< The path to the dictionary file. */
            std::string quizzesPaths = DEFAULT_SCRIPTED_QUIZ_PATH; /**< The path to the quizzes directory. */
            bool showLogs = false; /**< Flag to determine if logs should be shown. */

            /// Quiz settings
            std::string inputWord = DEFAULT_INPUT_WORD;         /**< The type of input word for the quiz. */
            std::string translatedWord = DEFAULT_TRANSLATED_WORD; /**< The type of translated word for the quiz. */
            std::string maxTriesForQuiz = DEFAULT_MAX_TRIES_FOR_QUIZ;
            uint16_t conjugationMask = DEFAULT_CONJUGATION_MASK;

            /**
             * @brief Converts the application settings to a string representation.
             * @return A string representation of the application settings.
             */
            std::string toString() const
            {
                std::string log;
                log += "Application settings:\n";
                log += std::format("  -> Username: {}\n", userName);
                log += std::format("  -> Dictionary Path: {}\n", dictionaryPath);
                log += std::format("  -> Conjugation Path: {}\n", conjugationPath);
                log += std::format("  -> Quizzes Path: {}\n", quizzesPaths);
                log += std::format("  -> Show Logs: {}\n", showLogs ? "true" : "false");
                log += "General Quiz settings:\n";
                log += std::format("  -> Number of tries to accept the word : {}\n", maxTriesForQuiz);
                log += "Vocabulary Quiz settings:\n";
                log += std::format("  -> Input Word: {}\n", inputWord);
                log += std::format("  -> Translated Word: {}\n", translatedWord);
                log += "Conjugation Quiz settings:\n";
                log += std::format("  -> Conjugation mask: {}\n", conjugationMask);

                return log;
            }
        };
    }
}
