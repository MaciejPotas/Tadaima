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
        Word word1{ 1, "kana1", "translation1", "romaji1", "example1", {"tag1"} };
        Word word2{ 2, "kana2", "translation2", "romaji2", "example2", {"tag2"} };
        Lesson lesson1{ 1, "Main Name 1", "Sub Name 1", {word1} };
        Lesson lesson2{ 2, "Main Name 2", "Sub Name 2", {word2} };
        lessons = { lesson1, lesson2 };
    }
};

TEST_F(QuizGameTest, InitializeQuizGame)
{

    MultipleChoiceQuiz quizGame(quiz::WordType::BaseWord, quiz::WordType::Romaji, lessons, logger);
    quizGame.start();
    EXPECT_EQ(quizGame.getCurrentOptions().size(), 4);
}

TEST_F(QuizGameTest, StartQuiz)
{
    MultipleChoiceQuiz quizGame(quiz::WordType::BaseWord, quiz::WordType::Romaji, lessons, logger);
    quizGame.start();
    EXPECT_FALSE(quizGame.isFinished());
    EXPECT_EQ(quizGame.getCurrentOptions().size(), 4);
}

TEST_F(QuizGameTest, AdvanceQuiz)
{
    MultipleChoiceQuiz quizGame(quiz::WordType::BaseWord, quiz::WordType::Romaji, lessons, logger);
    quizGame.start();
    quizGame.advance('a');
    EXPECT_EQ(quizGame.getCurrentOptions().size(), 4);
}

TEST_F(QuizGameTest, QuizFinished)
{
    MultipleChoiceQuiz quizGame(quiz::WordType::BaseWord, quiz::WordType::Romaji, lessons, logger);
    quizGame.start();
    for( size_t i = 0; i < lessons.size(); ++i )
    {
        quizGame.advance('a');
    }
    EXPECT_TRUE(quizGame.isFinished());
}
