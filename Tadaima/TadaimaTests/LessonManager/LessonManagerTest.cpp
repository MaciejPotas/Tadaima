#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "lessons/LessonManager.h"
#include "MockDatabase.h"

using namespace tadaima;
using ::testing::_;
using ::testing::Return;

class LessonManagerTest : public ::testing::Test
{
protected:
    MockDatabase mockDatabase;
    LessonManager lessonManager{ mockDatabase };
};

TEST_F(LessonManagerTest, AddLesson)
{
    Word word1{ 1, "kana1", "kanji1", "translation1", "romaji1", "example1", {"tag1"} };
    Word word2{ 2, "kana2", "kanji2", "translation2", "romaji2", "example2", {"tag2"} };

    Lesson lesson;
    lesson.groupName = "Group Name";
    lesson.mainName = "Main Name";
    lesson.subName = "Sub Name";
    lesson.words = { word1, word2 };

    EXPECT_CALL(mockDatabase, addLesson("Main Name", "Sub Name", "Group Name")).WillOnce(Return(1));
    EXPECT_CALL(mockDatabase, addWord(1, word1)).WillOnce(Return(2));
    EXPECT_CALL(mockDatabase, addTag(2, "tag1"));
    EXPECT_CALL(mockDatabase, addWord(1, word2)).WillOnce(Return(3));
    EXPECT_CALL(mockDatabase, addTag(3, "tag2"));

    int lessonId = lessonManager.addLesson(lesson);

    EXPECT_EQ(lessonId, 1);
}

TEST_F(LessonManagerTest, AddLessons)
{
    Word word1{ 1, "kana1", "kanji1", "translation1", "romaji1", "example1", {"tag1"} };
    Word word2{ 2, "kana2", "kanji2", "translation2", "romaji2", "example2", {"tag2"} };

    Lesson lesson1{ 1, "Group 1", "Main Name 1", "Sub Name 1", {word1} };
    Lesson lesson2{ 2, "Group 2", "Main Name 2", "Sub Name 2", {word2} };

    std::vector<Lesson> lessons = { lesson1, lesson2 };

    EXPECT_CALL(mockDatabase, addLesson("Main Name 1", "Sub Name 1", "Group 1")).WillOnce(Return(1));
    EXPECT_CALL(mockDatabase, addWord(1, word1)).WillOnce(Return(2));
    EXPECT_CALL(mockDatabase, addTag(2, "tag1"));

    EXPECT_CALL(mockDatabase, addLesson("Main Name 2", "Sub Name 2", "Group 2")).WillOnce(Return(3));
    EXPECT_CALL(mockDatabase, addWord(3, word2)).WillOnce(Return(4));
    EXPECT_CALL(mockDatabase, addTag(4, "tag2"));

    lessonManager.addLessons(lessons);
}

TEST_F(LessonManagerTest, RenameLessons)
{
    Lesson lesson1{ 1, "Group 1", "Main Name 1", "Sub Name 1", {} };
    Lesson lesson2{ 2, "Group 2", "Main Name 2", "Sub Name 2", {} };

    std::vector<Lesson> lessons = { lesson1, lesson2 };

    EXPECT_CALL(mockDatabase, updateLesson(1, "Group 1", "Main Name 1", "Sub Name 1"));
    EXPECT_CALL(mockDatabase, updateLesson(2, "Group 2", "Main Name 2", "Sub Name 2"));

    lessonManager.renameLessons(lessons);
}

TEST_F(LessonManagerTest, GetAllLessons)
{
    std::vector<Lesson> lessons = {
        {1, "Group 1", "Main Name 1", "Sub Name 1", {}},
        {2, "Group 2", "Main Name 2", "Sub Name 2", {}}
    };

    EXPECT_CALL(mockDatabase, getAllLessons()).WillOnce(Return(lessons));

    auto result = lessonManager.getAllLessons();

    EXPECT_EQ(result, lessons);
}

TEST_F(LessonManagerTest, AddWordToLesson)
{
    Word word{ 0, "kana", "kanji", "translation", "romaji", "example", {"tag1", "tag2"} };

    EXPECT_CALL(mockDatabase, addWord(1, word)).WillOnce(Return(2));
    EXPECT_CALL(mockDatabase, addTag(2, "tag1"));
    EXPECT_CALL(mockDatabase, addTag(2, "tag2"));

    lessonManager.addWordToLesson(1, word);
}

TEST_F(LessonManagerTest, GetLessonNames)
{
    std::vector<std::string> lessonNames = { "Lesson 1", "Lesson 2" };

    EXPECT_CALL(mockDatabase, getLessonNames()).WillOnce(Return(lessonNames));

    auto result = lessonManager.getLessonNames();

    EXPECT_EQ(result, lessonNames);
}

TEST_F(LessonManagerTest, GetWordsInLesson)
{
    std::vector<Word> words = {
        {1, "kana1", "kanji1", "translation1", "romaji1", "example1", {"tag1"}},
        {2, "kana2", "kanji2", "translation2", "romaji2", "example2", {"tag2"}}
    };

    EXPECT_CALL(mockDatabase, getWordsInLesson(1)).WillOnce(Return(words));

    auto result = lessonManager.getWordsInLesson(1);

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].kana, "kana1");
    EXPECT_EQ(result[1].translation, "translation2");
}

TEST_F(LessonManagerTest, EditLessons)
{
    Lesson lesson1{ 1, "Group 1", "Main Name 1", "Sub Name 1", {} };
    Lesson lesson2{ 2, "Group 2", "Main Name 2", "Sub Name 2", {} };

    std::vector<Lesson> lessons = { lesson1, lesson2 };

    EXPECT_CALL(mockDatabase, editLesson(lesson1));
    EXPECT_CALL(mockDatabase, editLesson(lesson2));

    lessonManager.editLessons(lessons);
}
