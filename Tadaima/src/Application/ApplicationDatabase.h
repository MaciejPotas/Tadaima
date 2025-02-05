/**
 * @file ApplicationDatabase.h
 * @brief Defines the ApplicationDatabase class, providing functionality to interact with a SQLite database for managing lessons, words, and application settings.
 */

#pragma once

#include "Tools/Database.h"
#include "Lessons/Lesson.h"
#include "Dictionary/Conjugations.h"
#include <vector>
#include <string>

struct sqlite3;
namespace tools { class Logger; }

namespace tadaima
{
    namespace application
    {
        /**
         * @class ApplicationDatabase
         * @brief Provides functionality to interact with a SQLite database for managing lessons, words, and application settings.
         */
        class ApplicationDatabase : public Database
        {
        public:
            /**
             * @brief Constructs an ApplicationDatabase object.
             * @param dbPath The file path to the SQLite database.
             * @param logger Reference to a Logger instance for logging activities.
             */
            ApplicationDatabase(const std::string& dbPath, tools::Logger& logger);

            /**
             * @brief Destroys the ApplicationDatabase object and closes the database connection.
             */
            ~ApplicationDatabase();

            /**
             * @brief Initializes the SQLite database by creating necessary tables and ensuring schema integrity.
             * @return True if initialization is successful, false otherwise.
             */
            bool initDatabase();

            /**
             * @brief Adds a new lesson to the database.
             * @param mainName The main name of the lesson.
             * @param subName The sub name of the lesson.
             * @param groupName The group name of the lesson.
             * @return The ID of the added lesson, or -1 if the operation fails.
             */
            int addLesson(const std::string& mainName, const std::string& subName, const std::string& groupName) override;

            /**
             * @brief Edits an existing lesson in the database.
             * @param lesson The lesson object containing updated information.
             * @return True if the lesson was successfully edited, false otherwise.
             */
            bool editLesson(const Lesson& lesson) override;

            /**
             * @brief Adds a new word to a specific lesson in the database.
             * @param lessonId The ID of the lesson to which the word will be added.
             * @param word The word object to add.
             * @return The ID of the added word, or -1 if the operation fails.
             */
            int addWord(int lessonId, const Word& word) override;

            /**
             * @brief Adds a tag to a specific word in the database.
             * @param wordId The ID of the word to which the tag will be added.
             * @param tag The tag string to add.
             */
            void addTag(int wordId, const std::string& tag) override;

            /**
             * @brief Updates an existing lesson's information in the database.
             * @param lessonId The ID of the lesson to update.
             * @param newGroupName The new group name for the lesson.
             * @param newMainName The new main name for the lesson.
             * @param newSubName The new sub name for the lesson.
             */
            void updateLesson(int lessonId, const std::string& newGroupName, const std::string& newMainName, const std::string& newSubName) override;

            /**
             * @brief Updates an existing word's details in the database.
             * @param wordId The ID of the word to update.
             * @param updatedWord The updated word object containing new data.
             */
            void updateWord(int wordId, const Word& updatedWord) override;

            /**
             * @brief Deletes a lesson from the database.
             * @param lessonId The ID of the lesson to delete.
             */
            void deleteLesson(int lessonId) override;

            /**
             * @brief Deletes a word from the database.
             * @param wordId The ID of the word to delete.
             */
            void deleteWord(int wordId) override;

            /**
             * @brief Retrieves the names of all lessons in the database.
             * @return A vector of strings containing the names of all lessons.
             */
            std::vector<std::string> getLessonNames() const override;

            /**
             * @brief Retrieves all words in a specific lesson.
             * @param lessonId The ID of the lesson.
             * @return A vector of Word objects representing all words in the specified lesson.
             */
            std::vector<Word> getWordsInLesson(int lessonId) const override;

            /**
             * @brief Retrieves all lessons from the database.
             * @return A vector of Lesson objects representing all lessons in the database.
             */
            std::vector<Lesson> getAllLessons() const override;

            /**
             * @brief Saves the application settings to the database.
             * @param settings The ApplicationSettings object containing settings to save.
             */
            void saveSettings(const ApplicationSettings& settings);

            /**
             * @brief Loads the application settings from the database.
             * @return An ApplicationSettings object populated with settings from the database.
             */
            ApplicationSettings loadSettings();

        private:

            /**
             * @brief Adds a conjugation entry to the database for a specific word.
             * @param wordId The ID of the word to which the conjugation belongs.
             * @param type The type of the conjugation (e.g., past tense, negative).
             * @param conjugatedWord The conjugated form of the word.
             */
            void addConjugation(int wordId, ConjugationType type, const std::string& conjugatedWord);

            /**
             * @brief Retrieves all conjugations for a specific word.
             * @param wordId The ID of the word for which to retrieve conjugations.
             * @return An array of strings, each representing a conjugated form of the word.
             */
            std::array<std::string, CONJUGATION_COUNT> getConjugations(int wordId) const;

            sqlite3* db; /**< Pointer to the SQLite database connection. */
            tools::Logger& m_logger; /**< Reference to the Logger instance for logging operations and errors. */
        };
    }
}
