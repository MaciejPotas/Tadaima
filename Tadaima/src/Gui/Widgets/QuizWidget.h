/**
 * @file QuizWidget.h
 * @brief Declares the QuizWidget class for the GUI quiz application.
 */

#pragma once

#include "gui/quiz/MultipleChoiceQuiz.h"
#include <chrono>
#include <vector>
#include <string>
#include "Widget.h"
#include "quiz/QuizType.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @class QuizWidget
             * @brief Represents a widget for displaying and interacting with a quiz in the GUI.
             */
            class QuizWidget : public Widget
            {
            public:

                /**
                 * @brief Constructs a VocabularyQuizWidget object.
                 * @param base The base word type for the quiz.
                 * @param desired The desired word type for the quiz.
                 * @param lessons Vector of lessons to initialize the quiz with.
                 * @param logger Reference to a Logger instance for logging.
                 */
                QuizWidget(quiz::WordType base, quiz::WordType desired, const std::vector<Lesson>& lessons, tools::Logger& logger);

                /**
                 * @brief Draws the quiz widget on the screen.
                 *
                 * This function is responsible for rendering the quiz interface using ImGui.
                 */
                void draw(bool* p_open = nullptr) override;

            private:
                /**
                 * @brief Highlights the correct answer and advances to the next question.
                 *
                 * This function handles the logic for highlighting the correct answer and transitioning to the next question.
                 */
                void highlightAndAdvance();

                tools::Logger& m_logger; /**< Reference to the Logger instance for logging. */
                quiz::MultipleChoiceQuiz quizGame; /**< Instance of QuizGame to manage quiz logic. */

                quiz::WordType m_baseWord; ///< The mother language type.
                quiz::WordType m_inputWord; ///< The learning language type.
               // bool isQuizWindowOpen = true; /**< Boolean flag to track if the quiz window is open. */
                bool highlightCorrectAnswer = false; /**< Boolean flag to indicate if the correct answer should be highlighted. */
                std::chrono::steady_clock::time_point highlightStartTime; /**< Time point for when the highlight started. */
                int correctAnswerIndex = -1; /**< Index of the correct answer within the current options. */
                char selectedOption = '\0'; /**< The option selected by the user. */
                std::string bufferedQuestion; /**< The current question to be displayed. */
                std::vector<std::string> bufferedOptions; /**< The current options to be displayed. */
            };
        }
    }
}
