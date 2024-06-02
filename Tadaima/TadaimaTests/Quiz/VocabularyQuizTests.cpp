#include "gui/quiz/VocabularyQuiz.h"
#include "gui/quiz/Flashcard.h"
#include <gtest/gtest.h>
#include "Tools/Logger.h"
#include "Lessons/Lesson.h"

using namespace tadaima;
using namespace tadaima::gui::quiz;

class MockLogger : public tools::Logger
{
public:
    void log(const std::string& message, tools::LogLevel level) override
    {
    }
};

std::vector<QuizWord> convertWordsToQuizWords(const std::vector<Word>& words)
{
    std::vector<QuizWord> quizWords;
    for( const auto& word : words )
    {
        quizWords.push_back(QuizWord(word.id, word.romaji));
    }
    return quizWords;
}

TEST(VocabularyQuizTest, InitializeQuiz)
{
    // Create example words
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };

    // Convert to QuizWords
    std::vector<Word> words = { word1, word2 };
    std::vector<QuizWord> flashcards = convertWordsToQuizWords(words);

    // Initialize the quiz with required correct answers set to 3
    VocabularyQuiz quiz(flashcards, 3, false);

    // Check initial state
    EXPECT_FALSE(quiz.isQuizComplete());
}

TEST(VocabularyQuizTest, AdvanceQuiz)
{
    // Create example words
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };

    // Convert to QuizWords
    std::vector<Word> words = { word1, word2 };
    std::vector<QuizWord> flashcards = convertWordsToQuizWords(words);

    // Initialize the quiz with required correct answers set to 3 and shuffling disabled
    VocabularyQuiz quiz(flashcards, 2, false);

    // Advance the quiz with correct and incorrect answers
    quiz.advance("wrong"); // Incorrect answer for word1
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 1);
    EXPECT_EQ(quiz.getStatistics().at(1).learnt, false);

    quiz.advance("wrong"); // Incorrect answer for word2
    EXPECT_EQ(quiz.getStatistics().at(2).badAttempts, 1);
    EXPECT_EQ(quiz.getStatistics().at(2).learnt, false);

    quiz.advance("a"); // Correct answer for word1
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 1);
    EXPECT_EQ(quiz.getStatistics().at(1).learnt, false);

    quiz.advance("i"); // Correct answer for word2
    EXPECT_EQ(quiz.getStatistics().at(2).badAttempts, 1);
    EXPECT_EQ(quiz.getStatistics().at(2).learnt, false);

    quiz.advance("a"); // Correct answer for word1
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 1);
    EXPECT_EQ(quiz.getStatistics().at(1).learnt, false);

    quiz.advance("wrong"); // Incorrect answer for word2
    EXPECT_EQ(quiz.getStatistics().at(2).badAttempts, 2);
    EXPECT_EQ(quiz.getStatistics().at(2).learnt, false);

    quiz.advance("a"); // Correct answer for word1
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 1);
    EXPECT_EQ(quiz.getStatistics().at(1).learnt, true);

    quiz.advance("i"); // Correct answer for word2
    EXPECT_EQ(quiz.getStatistics().at(2).badAttempts, 2);
    EXPECT_EQ(quiz.getStatistics().at(2).learnt, false);

    quiz.advance("i"); // Correct answer for word2
    EXPECT_EQ(quiz.getStatistics().at(2).badAttempts, 2);
    EXPECT_EQ(quiz.getStatistics().at(2).learnt, false);

    quiz.advance("i"); // Correct answer for word2
    EXPECT_EQ(quiz.getStatistics().at(2).badAttempts, 2);
    EXPECT_EQ(quiz.getStatistics().at(2).learnt, true);
}

TEST(VocabularyQuizTest, QuizCompletion)
{
    // Create example words
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };

    // Convert to QuizWords
    std::vector<Word> words = { word1, word2 };
    std::vector<QuizWord> flashcards = convertWordsToQuizWords(words);

    // Initialize the quiz with required correct answers set to 3 and shuffling disabled
    VocabularyQuiz quiz(flashcards, 3, false);

    // Simulate correct answers to complete the quiz
    for( int i = 0; i < 3; ++i )
    {
        quiz.advance("a"); // Correct answer for word1
        quiz.advance("i"); // Correct answer for word2
    }

    EXPECT_TRUE(quiz.isQuizComplete());
}

TEST(VocabularyQuizTest, DecrementOnIncorrectAnswer)
{
    // Create example words
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };

    // Convert to QuizWords
    std::vector<Word> words = { word1, word2 };
    std::vector<QuizWord> flashcards = convertWordsToQuizWords(words);

    // Initialize the quiz with required correct answers set to 3 and shuffling disabled
    VocabularyQuiz quiz(flashcards, 3, false);

    // Answer correctly twice
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("i"); // Correct answer for word2
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("i"); // Correct answer for word2

    // Answer incorrectly and check the decrement
    quiz.advance("wrong"); // Incorrect answer for word1
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 1);
    EXPECT_EQ(quiz.getStatistics().at(1).learnt, false);
    quiz.advance("wrong"); // Incorrect answer for word2
    EXPECT_EQ(quiz.getStatistics().at(2).badAttempts, 1);
    EXPECT_EQ(quiz.getStatistics().at(2).learnt, false);

    // Answer correctly again
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("i"); // Correct answer for word2
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 1);
    EXPECT_EQ(quiz.getStatistics().at(1).learnt, false);
    EXPECT_EQ(quiz.getStatistics().at(2).badAttempts, 1);
    EXPECT_EQ(quiz.getStatistics().at(2).learnt, false);
}

TEST(VocabularyQuizTest, NoDecrementBelowZero)
{
    // Create example words
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };

    // Convert to QuizWords
    std::vector<Word> words = { word1, word2 };
    std::vector<QuizWord> flashcards = convertWordsToQuizWords(words);

    // Initialize the quiz with required correct answers set to 3 and shuffling disabled
    VocabularyQuiz quiz(flashcards, 3, false);

    // Answer incorrectly multiple times
    for( int i = 0; i < 5; ++i )
    {
        quiz.advance("wrong"); // Incorrect answer for word1
        quiz.advance("wrong"); // Incorrect answer for word2
    }

    // Check that the correct answers count does not go below zero
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 5);
    EXPECT_EQ(quiz.getStatistics().at(1).learnt, false);
    quiz.advance("a"); // Correct answer for word1
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 5);
    EXPECT_EQ(quiz.getStatistics().at(1).learnt, false);
}

TEST(VocabularyQuizTest, CompletionWithMixedAnswers)
{
    // Create example words
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };

    // Convert to QuizWords
    std::vector<Word> words = { word1, word2 };
    std::vector<QuizWord> flashcards = convertWordsToQuizWords(words);

    // Initialize the quiz with required correct answers set to 3 and shuffling disabled
    VocabularyQuiz quiz(flashcards, 2, false);

    // Answer with a mix of correct and incorrect answers
    quiz.advance("wrong"); // Incorrect answer for word1
    quiz.advance("i"); // Correct answer for word2
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("wrong"); // Incorrect answer for word2
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("i"); // Correct answer for word2
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("i"); // Correct answer for word2
    quiz.advance("a"); // Correct answer for word2

    EXPECT_TRUE(quiz.isQuizComplete());
    EXPECT_EQ(quiz.getStatistics().at(1).badAttempts, 1);
    EXPECT_EQ(quiz.getStatistics().at(1).learnt, true);
    EXPECT_EQ(quiz.getStatistics().at(2).badAttempts, 1);
    EXPECT_EQ(quiz.getStatistics().at(2).learnt, true);
}
