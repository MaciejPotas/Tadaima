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
     * @class Database
     * @brief Abstract base class for interacting with a database.
     *
     * The Database class provides an interface for managing lessons, words, and application settings.
     * Derived classes are expected to implement the functionality for a specific database system.
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
         * @param groupName The group name of the lesson.
         * @return The ID of the added lesson.
         */
        virtual int addLesson(const std::string& mainName, const std::string& subName, const std::string& groupName) = 0;

        /**
         * @brief Edits an existing lesson in the database.
         * @param lesson The Lesson object containing updated details.
         * @return True if the lesson was successfully edited, false otherwise.
         */
        virtual bool editLesson(const Lesson& lesson) = 0;

        /**
         * @brief Adds a word to a specific lesson in the database.
         * @param lessonId The ID of the lesson.
         * @param word The Word object to add.
         * @return The ID of the added word.
         */
        virtual int addWord(int lessonId, const Word& word) = 0;

        /**
         * @brief Adds a tag to a word in the database.
         * @param wordId The ID of the word.
         * @param tag The tag to associate with the word.
         */
        virtual void addTag(int wordId, const std::string& tag) = 0;

        /**
         * @brief Updates an existing lesson in the database.
         * @param lessonId The ID of the lesson to update.
         * @param newGroupName The new group name for the lesson.
         * @param newMainName The new main name for the lesson.
         * @param newSubName The new sub name for the lesson.
         */
        virtual void updateLesson(int lessonId, const std::string& newGroupName, const std::string& newMainName, const std::string& newSubName) = 0;

        /**
         * @brief Updates an existing word in the database.
         * @param wordId The ID of the word to update.
         * @param updatedWord The updated Word object.
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
         * @brief Retrieves the names of all lessons in the database.
         * @return A vector of strings containing the names of all lessons.
         */
        virtual std::vector<std::string> getLessonNames() const = 0;

        /**
         * @brief Retrieves all words in a specific lesson from the database.
         * @param lessonId The ID of the lesson.
         * @return A vector of Word objects in the specified lesson.
         */
        virtual std::vector<Word> getWordsInLesson(int lessonId) const = 0;

        /**
         * @brief Retrieves all lessons from the database.
         * @return A vector of Lesson objects representing all lessons.
         */
        virtual std::vector<Lesson> getAllLessons() const = 0;

        /**
         * @brief Saves application settings to the database.
         * @param settings The ApplicationSettings object to save.
         */
        virtual void saveSettings(const application::ApplicationSettings& settings) = 0;

        /**
         * @brief Loads application settings from the database.
         * @return An ApplicationSettings object containing the loaded settings.
         */
        virtual application::ApplicationSettings loadSettings() = 0;
    };
}
