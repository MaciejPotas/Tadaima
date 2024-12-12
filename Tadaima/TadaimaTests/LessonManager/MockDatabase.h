#pragma once

#include "tools/Database.h"
#include "gmock/gmock.h"
#include "Application/ApplicationSettings.h"

/**
 * @class MockDatabase
 * @brief Mock implementation of the Database class for unit testing.
 */
class MockDatabase : public tadaima::Database
{
public:
    /**
     * @brief Mock method to add a new lesson to the database.
     * @param mainName The main name of the lesson.
     * @param subName The sub name of the lesson.
     * @param groupName The group name of the lesson.
     * @return The ID of the added lesson.
     */
    MOCK_METHOD(int, addLesson, (const std::string& mainName, const std::string& subName, const std::string& groupName), (override));

    /**
     * @brief Mock method to edit an existing lesson in the database.
     * @param lesson The Lesson object containing updated details.
     * @return True if the lesson was successfully edited, false otherwise.
     */
    MOCK_METHOD(bool, editLesson, (const tadaima::Lesson& lesson), (override));

    /**
     * @brief Mock method to add a word to a specific lesson in the database.
     * @param lessonId The ID of the lesson.
     * @param word The Word object to add.
     * @return The ID of the added word.
     */
    MOCK_METHOD(int, addWord, (int lessonId, const tadaima::Word& word), (override));

    /**
     * @brief Mock method to add a tag to a word in the database.
     * @param wordId The ID of the word.
     * @param tag The tag to associate with the word.
     */
    MOCK_METHOD(void, addTag, (int wordId, const std::string& tag), (override));

    /**
     * @brief Mock method to update an existing lesson in the database.
     * @param lessonId The ID of the lesson to update.
     * @param newGroupName The new group name for the lesson.
     * @param newMainName The new main name for the lesson.
     * @param newSubName The new sub name for the lesson.
     */
    MOCK_METHOD(void, updateLesson, (int lessonId, const std::string& newGroupName, const std::string& newMainName, const std::string& newSubName), (override));

    /**
     * @brief Mock method to update an existing word in the database.
     * @param wordId The ID of the word to update.
     * @param updatedWord The updated Word object.
     */
    MOCK_METHOD(void, updateWord, (int wordId, const tadaima::Word& updatedWord), (override));

    /**
     * @brief Mock method to delete a lesson from the database.
     * @param lessonId The ID of the lesson to delete.
     */
    MOCK_METHOD(void, deleteLesson, (int lessonId), (override));

    /**
     * @brief Mock method to delete a word from the database.
     * @param wordId The ID of the word to delete.
     */
    MOCK_METHOD(void, deleteWord, (int wordId), (override));

    /**
     * @brief Mock method to retrieve the names of all lessons in the database.
     * @return A vector of strings containing the names of all lessons.
     */
    MOCK_METHOD(std::vector<std::string>, getLessonNames, (), (const, override));

    /**
     * @brief Mock method to retrieve all words in a specific lesson from the database.
     * @param lessonId The ID of the lesson.
     * @return A vector of Word objects in the specified lesson.
     */
    MOCK_METHOD(std::vector<tadaima::Word>, getWordsInLesson, (int lessonId), (const, override));

    /**
     * @brief Mock method to retrieve all lessons from the database.
     * @return A vector of Lesson objects representing all lessons.
     */
    MOCK_METHOD(std::vector<tadaima::Lesson>, getAllLessons, (), (const, override));

    /**
     * @brief Mock method to save application settings to the database.
     * @param settings The ApplicationSettings object to save.
     */
    MOCK_METHOD(void, saveSettings, (const tadaima::application::ApplicationSettings& settings), (override));

    /**
     * @brief Mock method to load application settings from the database.
     * @return An ApplicationSettings object containing the loaded settings.
     */
    MOCK_METHOD(tadaima::application::ApplicationSettings, loadSettings, (), (override));
};
