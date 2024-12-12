#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gui/quiz/MultipleChoiceQuiz.h"
#include "Tools/Logger.h"
#include "gui/quiz/QuizType.h"

using namespace tadaima;
using namespace tadaima::gui::quiz;
using namespace tadaima::gui;

using ::testing::_;
using ::testing::Return;

class QuizGameTest : public ::testing::Test
{
protected:
    tools::Logger logger;
    std::vector<Lesson> lessons;

    void SetUp() override
    {
        Word word1{ 1, "kana1", "kanji1", "translation1", "romaji1", "example1", {"tag1"} };
        Word word2{ 2, "kana2", "kanji2", "translation2", "romaji2", "example2", {"tag2"} };
        Lesson lesson1{ 1, "GroupName1", "Main Name 1", "Sub Name 1", {word1} };
        Lesson lesson2{ 2, "GroupName2", "Main Name 2", "Sub Name 2", {word2} };
        lessons = { lesson1, lesson2 };
    }
};

TEST_F(QuizGameTest, InitializeQuizGame)
{
    MultipleChoiceQuiz quizGame(quiz::WordType::BaseWord, quiz::WordType::Romaji, lessons, logger);
    quizGame.start();

    // Check that quiz initializes with the correct number of options.
    EXPECT_EQ(quizGame.getCurrentOptions().size(), 4);
    EXPECT_FALSE(quizGame.isFinished());
}

TEST_F(QuizGameTest, StartQuiz)
{
    MultipleChoiceQuiz quizGame(quiz::WordType::BaseWord, quiz::WordType::Romaji, lessons, logger);
    quizGame.start();

    // Ensure quiz starts and options are populated.
    EXPECT_FALSE(quizGame.isFinished());
    EXPECT_EQ(quizGame.getCurrentOptions().size(), 4);
}

TEST_F(QuizGameTest, AdvanceQuiz)
{
    MultipleChoiceQuiz quizGame(quiz::WordType::BaseWord, quiz::WordType::Romaji, lessons, logger);
    quizGame.start();

    // Simulate advancing the quiz by providing an answer.
    quizGame.advance('a'); // Assuming 'a' corresponds to a valid answer option.
    EXPECT_FALSE(quizGame.isFinished());
    EXPECT_EQ(quizGame.getCurrentOptions().size(), 4); // Options should remain consistent for the next question.
}

TEST_F(QuizGameTest, QuizFinished)
{
    MultipleChoiceQuiz quizGame(quiz::WordType::BaseWord, quiz::WordType::Romaji, lessons, logger);
    quizGame.start();

    // Advance the quiz through all questions.
    for( size_t i = 0; i < lessons.size(); ++i )
    {
        quizGame.advance('a'); // Assuming 'a' is a valid option.
    }

    // Check that the quiz is marked as finished after all questions are answered.
    EXPECT_TRUE(quizGame.isFinished());
}
