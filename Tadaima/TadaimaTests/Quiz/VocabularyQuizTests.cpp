#include "gui/quiz/VocabularyQuiz.h"
#include <gtest/gtest.h>
#include "Tools/Logger.h"
#include "Lessons/Lesson.h"

using namespace tadaima;
using namespace tadaima::gui::quiz;

class MockLogger : public tools::Logger
{
public:
    void log(const std::string& message, tools::LogLevel level) override {}
};

std::vector<QuizWord> convertWordsToQuizWords(const std::vector<Word>& words)
{
    std::vector<QuizWord> quizWords;
    for( const auto& word : words )
    {
        quizWords.emplace_back(word.id, word.romaji);
    }
    return quizWords;
}

TEST(VocabularyQuizTest, InitializeQuiz)
{
    Word word1{ 1, "ka", "kanji1", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "kanji2", "i", "i", "Example sentence 2", {"tag2"} };

    std::vector<Word> words = { word1, word2 };
    std::vector<QuizWord> flashcards = convertWordsToQuizWords(words);

    VocabularyQuiz quiz(flashcards, 3, false);
    EXPECT_FALSE(quiz.isQuizComplete());
}

TEST(VocabularyQuizTest, AdvanceQuiz)
{
    Word word1{ 1, "ka", "kanji1", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "kanji2", "i", "i", "Example sentence 2", {"tag2"} };

    std::vector<Word> words = { word1, word2 };
    std::vector<QuizWord> flashcards = convertWordsToQuizWords(words);

    VocabularyQuiz quiz(flashcards, 2, false);

    quiz.advance("wrong"); // Incorrect answer for word1
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 1);
    EXPECT_FALSE(quiz.getStatistics().at(1).learnt);

    quiz.advance("i"); // Correct answer for word2
    EXPECT_EQ(quiz.getStatistics().at(2).badAttempts, 0);
    EXPECT_FALSE(quiz.getStatistics().at(2).learnt);

    quiz.advance("a"); // Correct answer for word1
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 1);
    EXPECT_FALSE(quiz.getStatistics().at(1).learnt);

    quiz.advance("i"); // Correct answer for word2
    EXPECT_EQ(quiz.getStatistics().at(2).badAttempts, 0);
    EXPECT_TRUE(quiz.getStatistics().at(2).learnt);

    quiz.advance("a"); // Correct answer for word1
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 1);
    EXPECT_FALSE(quiz.getStatistics().at(1).learnt);

    quiz.advance("a"); // Correct answer for word1
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 1);
    EXPECT_TRUE(quiz.getStatistics().at(1).learnt);
}

// ... (Other tests updated similarly)
