#include <gtest/gtest.h>
#include "gui/quiz/ConjugationQuiz.h"
#include "Dictionary/Conjugations.h"

using namespace tadaima;
using namespace tadaima::gui::quiz;

class ConjugationQuizTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Example flashcards for testing
        flashcards.emplace_back(1, ConjugationType::PAST, "ate");
        flashcards.emplace_back(2, ConjugationType::NEGATIVE, "don’t eat");
        flashcards.emplace_back(3, ConjugationType::TE_FORM, "eating");

        requiredCorrectAnswers = 2;
        quiz = std::make_unique<ConjugationQuiz>(flashcards, requiredCorrectAnswers, false);
    }

    std::vector<ConjugationQuiz::ConjugationFlashCard> flashcards;
    int requiredCorrectAnswers;
    std::unique_ptr<ConjugationQuiz> quiz;
};

TEST_F(ConjugationQuizTest, Initialization)
{
    EXPECT_EQ(quiz->getNumberOfFlashcards(), flashcards.size());
    EXPECT_EQ(quiz->getLearntConjugations(), 0);
}
TEST_F(ConjugationQuizTest, CorrectAnswer)
{
    const auto& currentCard = quiz->getCurrentFlashCard();
    bool result = quiz->advance(currentCard.answer);
    EXPECT_TRUE(result);

    // Check statistics update
    auto stats = quiz->getStatistics();
    auto key = std::make_pair(currentCard.wordId, currentCard.type);
    EXPECT_EQ(stats[key].goodAttempts, 1);
    EXPECT_EQ(stats[key].badAttempts, 0);
}
TEST_F(ConjugationQuizTest, IncorrectAnswer)
{
    const auto& currentCard = quiz->getCurrentFlashCard();
    std::string incorrectAnswer = currentCard.answer + "_wrong";
    bool result = quiz->advance(incorrectAnswer);
    EXPECT_FALSE(result);

    // Check statistics update
    auto stats = quiz->getStatistics();
    auto key = std::make_pair(currentCard.wordId, currentCard.type);
    EXPECT_EQ(stats[key].goodAttempts, 0);
    EXPECT_EQ(stats[key].badAttempts, 1);
}
TEST_F(ConjugationQuizTest, MoveToNextFlashcard)
{
    const auto& firstCard = quiz->getCurrentFlashCard();
    quiz->advance(firstCard.answer);

    // Ensure the current flashcard changes
    const auto& nextCard = quiz->getCurrentFlashCard();
    EXPECT_NE(&firstCard, &nextCard);
}
TEST_F(ConjugationQuizTest, QuizCompletion)
{
    // Simulate correct answers for all flashcards
    for( int i = 0; i < requiredCorrectAnswers; ++i )
    {
        for( auto& card : flashcards )
        {
            quiz->advance(card.answer);
        }
    }

    EXPECT_TRUE(quiz->isQuizComplete());
    EXPECT_EQ(quiz->getLearntConjugations(), flashcards.size());
}
TEST_F(ConjugationQuizTest, QuizNotComplete)
{
    // Answer correctly only once for each flashcard
    for( auto& card : flashcards )
    {
        quiz->advance(card.answer);
    }

    EXPECT_FALSE(quiz->isQuizComplete());
    EXPECT_LT(quiz->getLearntConjugations(), flashcards.size());
}
TEST_F(ConjugationQuizTest, ShuffleFlashcards)
{
    // Create a quiz with shuffle enabled
    std::vector<ConjugationQuiz::ConjugationFlashCard> originalFlashcards = flashcards;
    ConjugationQuiz shuffledQuiz(originalFlashcards, requiredCorrectAnswers, true);

    bool isShuffled = false;
    for( size_t i = 0; i < flashcards.size(); ++i )
    {
        if( shuffledQuiz.getCurrentFlashCard().wordId != originalFlashcards[i].wordId )
        {
            isShuffled = true;
            break;
        }
        shuffledQuiz.advance(shuffledQuiz.getCurrentFlashCard().answer);
    }

    EXPECT_TRUE(isShuffled);
}

TEST_F(ConjugationQuizTest, NoShuffleFlashcards)
{
    // Create a quiz with shuffle disabled
    ConjugationQuiz orderedQuiz(flashcards, requiredCorrectAnswers, false);

    for( size_t i = 0; i < flashcards.size(); ++i )
    {
        EXPECT_EQ(orderedQuiz.getCurrentFlashCard().wordId, flashcards[i].wordId);
        orderedQuiz.advance(orderedQuiz.getCurrentFlashCard().answer);
    }
}
TEST_F(ConjugationQuizTest, GetStatistics)
{
    // Answer some flashcards
    quiz->advance(flashcards[0].answer);
    quiz->advance("wrong answer");
    quiz->advance(flashcards[2].answer);

    auto stats = quiz->getStatistics();

    auto key1 = std::make_pair(flashcards[0].wordId, flashcards[0].type);
    auto key2 = std::make_pair(flashcards[1].wordId, flashcards[1].type);
    auto key3 = std::make_pair(flashcards[2].wordId, flashcards[2].type);

    EXPECT_EQ(stats[key1].goodAttempts, 1);
    EXPECT_EQ(stats[key1].badAttempts, 0);

    EXPECT_EQ(stats[key2].goodAttempts, 0);
    EXPECT_EQ(stats[key2].badAttempts, 1);

    EXPECT_EQ(stats[key3].goodAttempts, 1);
    EXPECT_EQ(stats[key3].badAttempts, 0);
}
TEST_F(ConjugationQuizTest, GetCurrentFlashcardException)
{
    // Simulate finishing the quiz
    for( int i = 0; i < requiredCorrectAnswers; ++i )
    {
        for( auto& card : flashcards )
        {
            quiz->advance(card.answer);
        }
    }

    EXPECT_THROW(quiz->getCurrentFlashCard(), std::invalid_argument);
}