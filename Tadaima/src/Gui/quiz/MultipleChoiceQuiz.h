/**
 * @file QuizGame.h
 * @brief Declares the QuizGame class for managing the quiz logic.
 */

#pragma once

#include "lessons/Lesson.h"
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include "QuizType.h"

namespace tools { class Logger; }

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            /**
             * @class QuizGame
             * @brief Manages the logic of the quiz game, including question generation, answer evaluation, and results.
             */
            class MultipleChoiceQuiz
            {
            public:

                /**
                 * @brief Constructs a new QuizGame object.
                 *
                 * @param logger A reference to a Logger instance for logging.
                 * @param lessons A vector containing Lesson objects to initialize the game.
                 */
                MultipleChoiceQuiz(quiz::WordType base, quiz::WordType desired, const std::vector<Lesson>& lessons, tools::Logger& logger);

                /**
                 * @brief Starts the quiz game.
                 *
                 * Initializes the game and prepares the first question.
                 */
                void start();

                /**
                 * @brief Advances the game by one step.
                 *
                 * Evaluates the user's answer and proceeds to the next question.
                 *
                 * @param answer The user's answer (a/b/c/d).
                 */
                void advance(char answer);

                /**
                 * @brief Checks if the quiz is finished.
                 *
                 * Determines if all questions have been answered.
                 *
                 * @return True if the quiz is finished, otherwise false.
                 */
                bool isFinished() const;

                /**
                 * @brief Gets the current question prompt.
                 *
                 * Retrieves the prompt for the current question.
                 *
                 * @return The current question prompt.
                 */
                std::string getCurrentQuestion() const;

                /**
                 * @brief Gets the current options.
                 *
                 * Retrieves the multiple-choice options for the current question.
                 *
                 * @return A vector of the current options.
                 */
                std::vector<std::string> getCurrentOptions() const;

                /**
                 * @brief Gets the quiz results.
                 *
                 * Provides a summary of the user's performance in the quiz.
                 *
                 * @return The quiz results.
                 */
                std::string getResults() const;

                /**
                 * @brief Gets the index of the current question.
                 *
                 * Retrieves the index of the current question in the quiz.
                 *
                 * @return The index of the current question.
                 */
                int getCurrentQuestionIndex() const;

                /**
                 * @brief Gets the total number of questions in the quiz.
                 *
                 * Retrieves the total number of questions.
                 *
                 * @return The total number of questions.
                 */
                int getTotalQuestions() const;

                /**
                 * @brief Gets the index of the correct answer for the current question.
                 *
                 * Retrieves the index of the correct answer within the current options.
                 *
                 * @return The index of the correct answer.
                 */
                int getCorrectAnswerIndex() const;

            private:

                /**
                 * @brief Initializes the quiz game by extracting words from lessons.
                 *
                 * Prepares the list of words to be used in the quiz.
                 *
                 * @param lessons The vector of lessons.
                 */
                void initialize(const std::vector<Lesson>& lessons);

                /**
                 * @brief Generates multiple-choice options.
                 *
                 * Creates a set of options including the correct word and random incorrect options.
                 *
                 * @param correctWord The correct word.
                 * @return A vector of multiple-choice options including the correct word.
                 */
                std::vector<std::string> generateOptions(const Word& correctWord);

                /**
                 * @brief Shuffles the words vector to randomize the quiz order.
                 *
                 * Ensures that questions are presented in a random order.
                 */
                void shuffleWords();

                quiz::WordType m_baseWord; ///< The mother language type.
                quiz::WordType m_inputWord; ///< The learning language type.
                tools::Logger& m_logger; /**< Reference to the Logger instance for logging. */
                std::vector<Word> m_quizWords; /**< Vector of words extracted from lessons for the quiz. */
                std::mt19937 rng; /**< Random number generator for shuffling words and generating options. */
                size_t currentWordIndex; /**< Index of the current word in the quiz. */
                int correctCount; /**< Count of correctly answered questions. */
                std::vector<std::string> currentOptions; /**< Vector of multiple-choice options for the current question. */
                int correctAnswerIndex; /**< Index of the correct answer within the current options. */
            };
        }
    }
}
