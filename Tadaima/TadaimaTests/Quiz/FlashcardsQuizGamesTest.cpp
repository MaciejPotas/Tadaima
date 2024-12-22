#include "quiz/Quiz.h"
#include "quiz/QuizItem.h"
#include "quiz/ConjugationItem.h"
#include "quiz/VocabularyItem.h"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace tadaima::quiz;
using namespace tadaima;

class QuizTestSuite : public ::testing::Test
{
protected:
    std::vector<std::unique_ptr<QuizItem>> flashcards;

    void SetUp() override
    {
        // Common setup for tests if required.
    }

    void TearDown() override
    {
        // Common teardown for tests if required.
    }
};

std::unique_ptr<QuizItem> createVocabularyItem(int id, const std::string& word)
{
    return std::make_unique<VocabularyItem>(id, word);
}

std::unique_ptr<QuizItem> createConjugationItem(int id, ConjugationType type, const std::string& answer)
{
    return std::make_unique<ConjugationItem>(id, type, answer);
}

TEST_F(QuizTestSuite, VocabularyQuizInitialization)
{
    flashcards.push_back(createVocabularyItem(1, "apple"));
    flashcards.push_back(createVocabularyItem(2, "banana"));

    Quiz quiz(flashcards, 2, false);

    EXPECT_EQ(quiz.getNumberOfItems(), 2);
    EXPECT_EQ(quiz.getLearntItems(), 0);
    EXPECT_FALSE(quiz.isQuizComplete());
}

TEST_F(QuizTestSuite, ConjugationQuizInitialization)
{
    flashcards.push_back(createConjugationItem(1, ConjugationType::PAST, "ate"));
    flashcards.push_back(createConjugationItem(2, ConjugationType::PAST, "ran"));

    Quiz quiz(flashcards, 2, false);

    EXPECT_EQ(quiz.getNumberOfItems(), 2);
    EXPECT_EQ(quiz.getLearntItems(), 0);
    EXPECT_FALSE(quiz.isQuizComplete());
}

TEST_F(QuizTestSuite, VocabularyQuizAdvanceCorrect)
{
    flashcards.push_back(createVocabularyItem(1, "apple"));
    flashcards.push_back(createVocabularyItem(2, "banana"));

    Quiz quiz(flashcards, 2, false);

    EXPECT_TRUE(quiz.advance("apple"));
    EXPECT_FALSE(quiz.advance("grape"));
    EXPECT_TRUE(quiz.advance("apple"));

    EXPECT_EQ(quiz.getLearntItems(), 1);
}

TEST_F(QuizTestSuite, ConjugationQuizAdvanceCorrect)
{
    flashcards.push_back(createConjugationItem(1, ConjugationType::PAST, "ate"));
    flashcards.push_back(createConjugationItem(2, ConjugationType::PAST, "ran"));

    Quiz quiz(flashcards, 2, false);

    EXPECT_TRUE(quiz.advance("ate"));
    EXPECT_FALSE(quiz.advance("walked"));
    EXPECT_TRUE(quiz.advance("ate"));

    EXPECT_EQ(quiz.getLearntItems(), 1);
}

TEST_F(QuizTestSuite, QuizCompletion)
{
    flashcards.push_back(createVocabularyItem(1, "apple"));
    flashcards.push_back(createVocabularyItem(2, "banana"));

    Quiz quiz(flashcards, 1, false);

    EXPECT_TRUE(quiz.advance("apple"));
    EXPECT_TRUE(quiz.advance("banana"));
    EXPECT_TRUE(quiz.isQuizComplete());
}

TEST_F(QuizTestSuite, ShuffleEnabled)
{
    flashcards.push_back(createVocabularyItem(1, "apple"));
    flashcards.push_back(createVocabularyItem(2, "banana"));
    flashcards.push_back(createVocabularyItem(3, "cherry"));

    Quiz quiz(flashcards, 1, true);

    EXPECT_EQ(quiz.getNumberOfItems(), 3);
    EXPECT_FALSE(quiz.isQuizComplete());
}

TEST_F(QuizTestSuite, NoAdvanceOnIncorrect)
{
    flashcards.push_back(createVocabularyItem(1, "apple"));

    Quiz quiz(flashcards, 1, false);

    EXPECT_FALSE(quiz.advance("banana"));
    EXPECT_EQ(quiz.getLearntItems(), 0);
    EXPECT_FALSE(quiz.isQuizComplete());
}

TEST_F(QuizTestSuite, StatisticsUpdate)
{
    flashcards.push_back(createVocabularyItem(1, "apple"));

    Quiz quiz(flashcards, 2, false);

    EXPECT_FALSE(quiz.advance("banana")); // Incorrect attempt
    EXPECT_TRUE(quiz.advance("apple"));   // Correct attempt
    EXPECT_FALSE(quiz.advance("grape")); // Incorrect attempt

    const auto& stats = quiz.getStatistics();
    auto it = stats.find("1");
    ASSERT_NE(it, stats.end());
    EXPECT_EQ(it->second.goodAttempts, 1);
    EXPECT_EQ(it->second.badAttempts, 2);
    EXPECT_FALSE(it->second.learnt);
}

TEST_F(QuizTestSuite, QuizReset)
{
    flashcards.push_back(createVocabularyItem(1, "apple"));
    flashcards.push_back(createVocabularyItem(2, "banana"));

    Quiz quiz(flashcards, 1, false);

    EXPECT_TRUE(quiz.advance("apple"));
    EXPECT_TRUE(quiz.advance("banana"));

    EXPECT_TRUE(quiz.isQuizComplete());

    // Reset and reinitialize the quiz
    quiz = Quiz(flashcards, 1, false);

    EXPECT_EQ(quiz.getLearntItems(), 0);
    EXPECT_FALSE(quiz.isQuizComplete());
}

TEST_F(QuizTestSuite, EdgeCaseInvalidAnswer)
{
    flashcards.push_back(createVocabularyItem(1, "apple"));

    Quiz quiz(flashcards, 1, false);

    EXPECT_FALSE(quiz.advance("")); // Empty answer
    EXPECT_FALSE(quiz.advance("12345")); // Invalid answer
}
