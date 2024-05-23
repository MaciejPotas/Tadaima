#pragma once

#include "tools/Database.h"
#include "gmock/gmock.h"

class MockDatabase : public tadaima::Database
{
public:
    MOCK_METHOD(int, addLesson, (const std::string& mainName, const std::string& subName), (override));
    MOCK_METHOD(int, addWord, (int lessonId, const tadaima::Word& word), (override));
    MOCK_METHOD(void, addTag, (int wordId, const std::string& tag), (override));
    MOCK_METHOD(void, updateLesson, (int lessonId, const std::string& newMainName, const std::string& newSubName), (override));
    MOCK_METHOD(void, updateWord, (int wordId, const tadaima::Word& updatedWord), (override));
    MOCK_METHOD(void, deleteLesson, (int lessonId), (override));
    MOCK_METHOD(void, deleteWord, (int wordId), (override));
    MOCK_METHOD(std::vector<std::string>, getLessonNames, (), (const, override));
    MOCK_METHOD(std::vector<tadaima::Word>, getWordsInLesson, (int lessonId), (const, override));
    MOCK_METHOD(std::vector<tadaima::Lesson>, getAllLessons, (), (const, override));
};
