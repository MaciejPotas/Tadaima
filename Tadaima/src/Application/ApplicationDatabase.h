/**
 * @file ApplicationDatabase.h
 * @brief Defines the ApplicationDatabase class, providing functionality to interact with a SQLite database for managing lessons and words.
 */

#pragma once

#include "Tools/Database.h"
#include "Lessons/Lesson.h"
#include <vector>
#include <string>

struct sqlite3;
namespace tools { class Logger; }

namespace tadaima
{
    namespace application
    {
        /**
         * @brief The ApplicationDatabase class provides functionality to interact with a SQLite database
         * for managing lessons and words.
         */
        class ApplicationDatabase : public Database
        {
        public:
            /**
             * @brief Constructs an ApplicationDatabase object.
             * @param dbPath The file path to the SQLite database.
             * @param logger Reference to a Logger instance for logging.
             */
            ApplicationDatabase(const std::string& dbPath, tools::Logger& logger);

            /**
             * @brief Destroys the ApplicationDatabase object.
             */
            ~ApplicationDatabase();

            /**
             * @brief Initializes the SQLite database.
             * @return True if initialization is successful, false otherwise.
             */
            bool initDatabase();

            /**
             * @brief Adds a new lesson to the database.
             * @param mainName The main name of the lesson.
             * @param subName The sub name of the lesson.
             * @return The ID of the added lesson, or -1 on failure.
             */
            int addLesson(const std::string& mainName, const std::string& subName) override;

            /**
             * @brief Edits an existing lesson in the database.
             * @param lesson The lesson to edit.
             * @return True if the lesson was successfully edited, false otherwise.
             */
            bool editLesson(const Lesson& lesson) override;

            /**
             * @brief Adds a word to a lesson in the database.
             * @param lessonId The ID of the lesson.
             * @param word The word to add to the lesson.
             * @return The ID of the added word, or -1 on failure.
             */
            int addWord(int lessonId, const Word& word) override;

            /**
             * @brief Adds a tag to a word in the database.
             * @param wordId The ID of the word.
             * @param tag The tag to add to the word.
             */
            void addTag(int wordId, const std::string& tag) override;

            /**
             * @brief Updates an existing lesson in the database.
             * @param lessonId The ID of the lesson to update.
             * @param newMainName The new main name of the lesson.
             * @param newSubName The new sub name of the lesson.
             */
            void updateLesson(int lessonId, const std::string& newMainName, const std::string& newSubName) override;

            /**
             * @brief Updates an existing word in the database.
             * @param wordId The ID of the word to update.
             * @param updatedWord The updated word data.
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
             * @brief Retrieves the names of all lessons from the database.
             * @return A vector containing the names of all lessons.
             */
            std::vector<std::string> getLessonNames() const override;

            /**
             * @brief Retrieves all words in a lesson from the database.
             * @param lessonId The ID of the lesson.
             * @return A vector containing all words in the specified lesson.
             */
            std::vector<Word> getWordsInLesson(int lessonId) const override;

            /**
             * @brief Retrieves all lessons from the database.
             * @return A vector containing all lessons.
             */
            std::vector<Lesson> getAllLessons() const override;

            /**
             * @brief Saves the application settings to the database.
             * @param settings The application settings to save.
             */
            void saveSettings(const ApplicationSettings& settings);

            /**
             * @brief Loads the application settings from the database.
             * @return The loaded application settings.
             */
            ApplicationSettings loadSettings();

        private:
            sqlite3* db; /**< Pointer to the SQLite database. */
            tools::Logger& m_logger; /**< Reference to the Logger instance for logging. */
        };
    }
}
