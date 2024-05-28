#include "gui/quiz/VocabularyQuiz.h"
#include "gui/quiz/Flashcard.h"
#include <gtest/gtest.h>
#include "Tools/Logger.h"

using namespace tadaima;
using namespace tadaima::gui::quiz;

class MockLogger : public tools::Logger
{
public:
    void log(const std::string& message, tools::LogLevel level) override
    {
    }
};
TEST(VocabularyQuizTest, InitializeQuiz)
{
    // Create example words and lessons
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };
    Lesson lesson1{ 1, "Lesson 1", "Sub 1", {word1, word2} };

    // Create flashcards
    std::vector<Flashcard> flashcards;
    flashcards.emplace_back(word1, lesson1.id, 0);
    flashcards.emplace_back(word2, lesson1.id, 0);

    // Initialize the quiz with required correct answers set to 3
    VocabularyQuiz quiz(flashcards, AnswerType::Romaji, 3, false);

    // Check initial state
    EXPECT_FALSE(quiz.isQuizComplete());
}

TEST(VocabularyQuizTest, AdvanceQuiz)
{
    // Create example words and lessons
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };
    Lesson lesson1{ 1, "Lesson 1", "Sub 1", {word1, word2} };

    // Create flashcards
    std::vector<Flashcard> flashcards;
    flashcards.emplace_back(word1, lesson1.id, 0);
    flashcards.emplace_back(word2, lesson1.id, 0);

    // Initialize the quiz with required correct answers set to 3 and shuffling disabled
    VocabularyQuiz quiz(flashcards, AnswerType::Romaji, 3, false);

    // Advance the quiz with correct and incorrect answers
    quiz.advance("wrong"); // Incorrect answer for word1
    EXPECT_EQ(flashcards[0].badAttempts, 1);
    EXPECT_EQ(flashcards[0].learned, false);

    quiz.advance("wrong"); // Incorrect answer for word2
    EXPECT_EQ(flashcards[1].badAttempts, 1);
    EXPECT_EQ(flashcards[1].learned, false);

    quiz.advance("a"); // Correct answer for word1
    EXPECT_EQ(flashcards[0].badAttempts, 1);
    EXPECT_EQ(flashcards[0].learned, false);

    quiz.advance("i"); // Correct answer for word2
    EXPECT_EQ(flashcards[1].badAttempts, 1);
    EXPECT_EQ(flashcards[1].learned, false);

    quiz.advance("a"); // Correct answer for word1
    EXPECT_EQ(flashcards[0].badAttempts, 1);
    EXPECT_EQ(flashcards[0].learned, false);

    quiz.advance("wrong"); // Incorrect answer for word2
    EXPECT_EQ(flashcards[1].badAttempts, 2);
    EXPECT_EQ(flashcards[1].learned, false);

    quiz.advance("a"); // Correct answer for word1
    EXPECT_EQ(flashcards[0].badAttempts, 1);
    EXPECT_EQ(flashcards[0].learned, true);

    quiz.advance("i"); // Correct answer for word2
    EXPECT_EQ(flashcards[1].badAttempts, 2);
    EXPECT_EQ(flashcards[1].learned, false);

    quiz.advance("i"); // Correct answer for word2
    EXPECT_EQ(flashcards[1].badAttempts, 2);
    EXPECT_EQ(flashcards[1].learned, false);

    quiz.advance("i"); // Correct answer for word2
    EXPECT_EQ(flashcards[1].badAttempts, 2);
    EXPECT_EQ(flashcards[1].learned, true);
}
TEST(VocabularyQuizTest, QuizCompletion)
{
    // Create example words and lessons
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };
    Lesson lesson1{ 1, "Lesson 1", "Sub 1", {word1, word2} };

    // Create flashcards
    std::vector<Flashcard> flashcards;
    flashcards.emplace_back(word1, lesson1.id, 0);
    flashcards.emplace_back(word2, lesson1.id, 0);

    // Initialize the quiz with required correct answers set to 3 and shuffling disabled
    VocabularyQuiz quiz(flashcards, AnswerType::Romaji, 3, false);

    // Simulate correct answers to complete the quiz
    for( int i = 0; i < 3; ++i )
    {
        quiz.advance("a"); // Correct answer for word1
        quiz.advance("i"); // Correct answer for word2
    }

    EXPECT_TRUE(quiz.isQuizComplete());
}

TEST(VocabularyQuizTest, FlashcardsWithMistakes)
{
    // Create example words and lessons
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };
    Lesson lesson1{ 1, "Lesson 1", "Sub 1", {word1, word2} };

    // Create flashcards
    std::vector<Flashcard> flashcards;
    flashcards.emplace_back(word1, lesson1.id, 0);
    flashcards.emplace_back(word2, lesson1.id, 0);

    // Initialize the quiz with required correct answers set to 3 and shuffling disabled
    VocabularyQuiz quiz(flashcards, AnswerType::Romaji, 3, false);

    // Introduce mistakes
    quiz.advance("wrong"); // Incorrect answer for word1
    quiz.advance("wrong"); // Incorrect answer for word1
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("wrong"); // Incorrect answer for word2

    auto flashcardsWithMistakes = quiz.getFlashcardsWithMistakes(1);
    EXPECT_EQ(flashcardsWithMistakes.size(), 2);
    EXPECT_EQ(flashcardsWithMistakes[0].word.translation, "a");
    EXPECT_EQ(flashcardsWithMistakes[1].word.translation, "i");
}

TEST(VocabularyQuizTest, DecrementOnIncorrectAnswer)
{
    // Create example words and lessons
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };
    Lesson lesson1{ 1, "Lesson 1", "Sub 1", {word1, word2} };

    // Create flashcards
    std::vector<Flashcard> flashcards;
    flashcards.emplace_back(word1, lesson1.id, 0);
    flashcards.emplace_back(word2, lesson1.id, 0);

    // Initialize the quiz with required correct answers set to 3 and shuffling disabled
    VocabularyQuiz quiz(flashcards, AnswerType::Romaji, 3, false);

    // Answer correctly twice
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("i"); // Correct answer for word2
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("i"); // Correct answer for word2

    // Answer incorrectly and check the decrement
    quiz.advance("wrong"); // Incorrect answer for word1
    EXPECT_EQ(flashcards[0].badAttempts, 1);
    EXPECT_EQ(flashcards[0].learned, false);
    quiz.advance("wrong"); // Incorrect answer for word2
    EXPECT_EQ(flashcards[1].badAttempts, 1);
    EXPECT_EQ(flashcards[1].learned, false);

    // Answer correctly again
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("i"); // Correct answer for word2
    EXPECT_EQ(flashcards[0].badAttempts, 1);
    EXPECT_EQ(flashcards[0].learned, false);
    EXPECT_EQ(flashcards[1].badAttempts, 1);
    EXPECT_EQ(flashcards[1].learned, false);
}

TEST(VocabularyQuizTest, NoDecrementBelowZero)
{
    // Create example words and lessons
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };
    Lesson lesson1{ 1, "Lesson 1", "Sub 1", {word1, word2} };

    // Create flashcards
    std::vector<Flashcard> flashcards;
    flashcards.emplace_back(word1, lesson1.id, 0);
    flashcards.emplace_back(word2, lesson1.id, 0);

    // Initialize the quiz with required correct answers set to 3 and shuffling disabled
    VocabularyQuiz quiz(flashcards, AnswerType::Romaji, 3, false);

    // Answer incorrectly multiple times
    for( int i = 0; i < 5; ++i )
    {
        quiz.advance("wrong"); // Incorrect answer for word1
        quiz.advance("wrong"); // Incorrect answer for word1
    }

    // Check that the correct answers count does not go below zero
    EXPECT_EQ(flashcards[0].badAttempts, 5);
    EXPECT_EQ(flashcards[0].learned, false);
    quiz.advance("a"); // Correct answer for word1
    EXPECT_EQ(flashcards[0].badAttempts, 5);
    EXPECT_EQ(flashcards[0].learned, false);
}


TEST(VocabularyQuizTest, CompletionWithMixedAnswers)
{
    // Create example words and lessons
    Word word1{ 1, "ka", "a", "a", "Example sentence 1", {"tag1"} };
    Word word2{ 2, "ki", "i", "i", "Example sentence 2", {"tag2"} };
    Lesson lesson1{ 1, "Lesson 1", "Sub 1", {word1, word2} };

    // Create flashcards
    std::vector<Flashcard> flashcards;
    flashcards.emplace_back(word1, lesson1.id, 0);
    flashcards.emplace_back(word2, lesson1.id, 0);

    // Initialize the quiz with required correct answers set to 3 and shuffling disabled
    VocabularyQuiz quiz(flashcards, AnswerType::Romaji, 3, false);

    // Answer with a mix of correct and incorrect answers
    quiz.advance("wrong"); // Incorrect answer for word1
    quiz.advance("i"); // Correct answer for word2
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("wrong"); // Incorrect answer for word2
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("i"); // Correct answer for word2
    quiz.advance("a"); // Correct answer for word1
    quiz.advance("i"); // Correct answer for word2
    quiz.advance("i"); // Correct answer for word2

    EXPECT_TRUE(quiz.isQuizComplete());
    EXPECT_EQ(flashcards[0].badAttempts, 1);
    EXPECT_EQ(flashcards[0].learned, true);
    EXPECT_EQ(flashcards[1].badAttempts, 1);
    EXPECT_EQ(flashcards[1].learned, true);
}
