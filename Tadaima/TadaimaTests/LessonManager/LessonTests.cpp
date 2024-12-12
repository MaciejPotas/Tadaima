#include "gtest/gtest.h"
#include "lessons/Lesson.h"

using namespace tadaima;

// Unit tests for the Word struct
TEST(WordTest, EqualityOperator)
{
    Word word1{ 1, "kana1", "kanji1", "translation1", "romaji1", "example1", {"tag1", "tag2"} };
    Word word2{ 1, "kana1", "kanji1", "translation1", "romaji1", "example1", {"tag1", "tag2"} };
    Word word3{ 2, "kana2", "kanji2", "translation2", "romaji2", "example2", {"tag3", "tag4"} };

    EXPECT_TRUE(word1 == word2);
    EXPECT_FALSE(word1 == word3);
}

TEST(WordTest, InequalityOperator)
{
    Word word1{ 1, "kana1", "kanji1", "translation1", "romaji1", "example1", {"tag1", "tag2"} };
    Word word2{ 1, "kana1", "kanji1", "translation1", "romaji1", "example1", {"tag1", "tag2"} };
    Word word3{ 2, "kana2", "kanji2", "translation2", "romaji2", "example2", {"tag3", "tag4"} };

    EXPECT_FALSE(word1 != word2);
    EXPECT_TRUE(word1 != word3);
}

// Unit tests for the Lesson struct
TEST(LessonTest, EqualityOperator)
{
    Word word1{ 1, "kana1", "kanji1", "translation1", "romaji1", "example1", {"tag1", "tag2"} };
    Word word2{ 2, "kana2", "kanji2", "translation2", "romaji2", "example2", {"tag3", "tag4"} };

    Lesson lesson1{ 1, "GroupName", "mainName1", "subName1", {word1, word2} };
    Lesson lesson2{ 1, "GroupName", "mainName1", "subName1", {word1, word2} };
    Lesson lesson3{ 2, "AnotherGroup", "mainName2", "subName2", {word2} };

    EXPECT_TRUE(lesson1 == lesson2);
    EXPECT_FALSE(lesson1 == lesson3);
}

TEST(LessonTest, InequalityOperator)
{
    Word word1{ 1, "kana1", "kanji1", "translation1", "romaji1", "example1", {"tag1", "tag2"} };
    Word word2{ 2, "kana2", "kanji2", "translation2", "romaji2", "example2", {"tag3", "tag4"} };

    Lesson lesson1{ 1, "GroupName", "mainName1", "subName1", {word1, word2} };
    Lesson lesson2{ 1, "GroupName", "mainName1", "subName1", {word1, word2} };
    Lesson lesson3{ 2, "AnotherGroup", "mainName2", "subName2", {word2} };

    EXPECT_FALSE(lesson1 != lesson2);
    EXPECT_TRUE(lesson1 != lesson3);
}

TEST(LessonTest, IsEmpty)
{
    Lesson emptyLesson{ 0, "", "", "", {} };
    Lesson nonEmptyLesson{ 1, "GroupName", "mainName1", "subName1", {Word{1, "kana1", "kanji1", "translation1", "romaji1", "example1", {"tag1"}}} };

    EXPECT_TRUE(emptyLesson.isEmpty());
    EXPECT_FALSE(nonEmptyLesson.isEmpty());
}
