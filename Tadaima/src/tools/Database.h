/**
 * @file Database.h
 * @brief Defines the Database class interface for interacting with a database.
 */

#pragma once

#include "lessons/Lesson.h"
#include <vector>
#include <string>

namespace tadaima
{
    namespace application
    {
        struct ApplicationSettings;
    }

    /**
     * @brief The Database class provides an interface for interacting with a database.
     */
    class Database
    {
    public:
        /**
         * @brief Virtual destructor to ensure proper cleanup of derived classes.
         */
        virtual ~Database() = default;

        /**
         * @brief Adds a new lesson to the database.
         * @param mainName The main name of the lesson.
         * @param subName The sub name of the lesson.
         * @return The ID of the added lesson.
         */
        virtual int addLesson(const std::string& mainName, const std::string& subName) = 0;

        /**
         * @brief Edits an existing lesson in the database.
         * @param lesson The lesson to edit.
         * @return True if the lesson was successfully edited, false otherwise.
         */
        virtual bool editLesson(const Lesson& lesson) = 0;

        /**
         * @brief Adds a word to a lesson in the database.
         * @param lessonId The ID of the lesson.
         * @param word The word to add to the lesson.
         * @return The ID of the added word.
         */
        virtual int addWord(int lessonId, const Word& word) = 0;

        /**
         * @brief Adds a tag to a word in the database.
         * @param wordId The ID of the word.
         * @param tag The tag to add to the word.
         */
        virtual void addTag(int wordId, const std::string& tag) = 0;

        /**
         * @brief Updates an existing lesson in the database.
         * @param lessonId The ID of the lesson.
         * @param newMainName The new main name of the lesson.
         * @param newSubName The new sub name of the lesson.
         */
        virtual void updateLesson(int lessonId, const std::string& newMainName, const std::string& newSubName) = 0;

        /**
         * @brief Updates an existing word in the database.
         * @param wordId The ID of the word.
         * @param updatedWord The updated word.
         */
        virtual void updateWord(int wordId, const Word& updatedWord) = 0;

        /**
         * @brief Deletes a lesson from the database.
         * @param lessonId The ID of the lesson to delete.
         */
        virtual void deleteLesson(int lessonId) = 0;

        /**
         * @brief Deletes a word from the database.
         * @param wordId The ID of the word to delete.
         */
        virtual void deleteWord(int wordId) = 0;

        /**
         * @brief Retrieves the names of all lessons from the database.
         * @return A vector containing the names of all lessons.
         */
        virtual std::vector<std::string> getLessonNames() const = 0;

        /**
         * @brief Retrieves all words in a lesson from the database.
         * @param lessonId The ID of the lesson.
         * @return A vector containing all words in the specified lesson.
         */
        virtual std::vector<Word> getWordsInLesson(int lessonId) const = 0;

        /**
         * @brief Retrieves all lessons from the database.
         * @return A vector containing all lessons.
         */
        virtual std::vector<Lesson> getAllLessons() const = 0;

        /**
         * @brief Saves the application settings to the database.
         * @param settings The application settings to save.
         */
        virtual void saveSettings(const application::ApplicationSettings& settings) = 0;

        /**
         * @brief Loads the application settings from the database.
         * @return The loaded application settings.
         */
        virtual application::ApplicationSettings loadSettings() = 0;
    };
}
