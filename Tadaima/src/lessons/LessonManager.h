/**
 * @file LessonManager.h
 * @brief Defines the LessonManager class, responsible for managing lessons in the database.
 */

#pragma once

#include "Tools/Database.h"
#include "Lesson.h"
#include <vector>
#include <string>

namespace tadaima
{
    /**
     * @brief The LessonManager class manages lessons in the database.
     */
    class LessonManager
    {
    public:

        /**
         * @brief Constructs a LessonManager object with the specified database.
         * @param db The database to manage lessons.
         */
        LessonManager(Database& db);

        /**
         * @brief Adds a new lesson to the database.
         * @param lesson The lesson to add.
         * @return The ID of the added lesson.
         */
        int addLesson(const Lesson& lesson);

        /**
         * @brief Adds a word to a lesson in the database.
         * @param lessonId The ID of the lesson.
         * @param word The word to add to the lesson.
         */
        void addWordToLesson(int lessonId, const Word& word);

        /**
         * @brief Adds a vector of lessons to the database.
         * @param lessons The vector of lessons to add.
         */
        void addLessons(const std::vector<Lesson>& lessons);

        /**
         * @brief Renames the lessons in the database.
         * @param lessons The vector of lessons to rename.
         */
        void renameLessons(const std::vector<Lesson>& lessons);

        /**
         * @brief Remove the lessons from the database.
         * @param lessons The vector of lessons to remove.
         */
        void removeLessons(const std::vector<Lesson>& lessons);

        /**
         * @brief Retrieves all lessons from the database.
         * @return A vector containing all lessons.
         */
        std::vector<Lesson> getAllLessons() const;

        /**
         * @brief Retrieves the names of all lessons from the database.
         * @return A vector containing the names of all lessons.
         */
        std::vector<std::string> getLessonNames() const;

        /**
         * @brief Retrieves all words in a lesson from the database.
         * @param lessonId The ID of the lesson.
         * @return A vector containing all words in the specified lesson.
         */
        std::vector<Word> getWordsInLesson(int lessonId) const;

    private:

        /**
         * @brief Adds a new lesson to the database.
         * @param mainName The main name of the lesson.
         * @param subName The sub name of the lesson.
         * @return The ID of the added lesson.
         */
        int addLesson(const std::string& mainName, const std::string& subName);

        /**
         * @brief Renames an existing lesson in the database.
         * @param lessonId The ID of the lesson to rename.
         * @param newMainName The new main name of the lesson.
         * @param newSubName The new sub name of the lesson.
         */
        void renameLesson(int lessonId, const std::string& newMainName, const std::string& newSubName);

        /**
         * @brief Reference to the database object.
         */
        Database& m_database;
    };
}
