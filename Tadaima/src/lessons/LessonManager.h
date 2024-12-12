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
     * @class LessonManager
     * @brief A class for managing lessons and their associated data in the database.
     *
     * The LessonManager class provides functionality for adding, editing, renaming, and removing lessons
     * and their associated words from the database. It acts as a high-level interface to interact with lesson data.
     */
    class LessonManager
    {
    public:

        /**
         * @brief Constructs a LessonManager object with the specified database.
         * @param db A reference to the database object used for lesson management.
         */
        LessonManager(Database& db);

        /**
         * @brief Adds a new lesson to the database.
         * @param lesson The Lesson object containing the lesson details to add.
         * @return The ID of the newly added lesson.
         */
        int addLesson(const Lesson& lesson);

        /**
         * @brief Edits an existing lesson in the database.
         * @param lesson The Lesson object containing the updated lesson details.
         * @return The ID of the edited lesson.
         */
        int editLesson(const Lesson& lesson);

        /**
         * @brief Edits multiple lessons in the database.
         * @param lessons A vector of Lesson objects containing the updated lesson details.
         */
        void editLessons(const std::vector<Lesson>& lessons);

        /**
         * @brief Adds a word to a specific lesson in the database.
         * @param lessonId The ID of the lesson to which the word should be added.
         * @param word The Word object containing the word details to add.
         */
        void addWordToLesson(int lessonId, const Word& word);

        /**
         * @brief Adds multiple lessons to the database.
         * @param lessons A vector of Lesson objects to add to the database.
         */
        void addLessons(const std::vector<Lesson>& lessons);

        /**
         * @brief Renames multiple lessons in the database.
         * @param lessons A vector of Lesson objects containing the updated names.
         */
        void renameLessons(const std::vector<Lesson>& lessons);

        /**
         * @brief Removes multiple lessons from the database.
         * @param lessons A vector of Lesson objects to remove from the database.
         */
        void removeLessons(const std::vector<Lesson>& lessons);

        /**
         * @brief Retrieves all lessons from the database.
         * @return A vector containing all lessons stored in the database.
         */
        std::vector<Lesson> getAllLessons() const;

        /**
         * @brief Retrieves the names of all lessons from the database.
         * @return A vector containing the names of all lessons.
         */
        std::vector<std::string> getLessonNames() const;

        /**
         * @brief Retrieves all words in a specific lesson from the database.
         * @param lessonId The ID of the lesson.
         * @return A vector containing all words associated with the specified lesson.
         */
        std::vector<Word> getWordsInLesson(int lessonId) const;

    private:

        /**
         * @brief Adds a new lesson to the database with specified names.
         * @param mainName The main name of the lesson.
         * @param subName The sub name of the lesson.
         * @param groupName The group name of the lesson.
         * @return The ID of the added lesson.
         */
        int addLesson(const std::string& mainName, const std::string& subName, const std::string& groupName);

        /**
         * @brief Renames an existing lesson in the database.
         * @param lessonId The ID of the lesson to rename.
         * @param newGroupName The new group name for the lesson.
         * @param newMainName The new main name for the lesson.
         * @param newSubName The new sub name for the lesson.
         */
        void renameLesson(int lessonId, const std::string& newGroupName, const std::string& newMainName, const std::string& newSubName);

        Database& m_database; /**< Reference to the database object for managing lesson data. */
    };
}
