/**
 * @file QuizWidget.h
 * @brief Declares the QuizWidget class for the GUI quiz application.
 */

#pragma once

#include "quiz/QuizGame.h"
#include "imgui.h"

namespace tools { class Logger; }

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
            class QuizWidget
            {
            public:
                /**
                 * @brief Constructs a new QuizWidget object.
                 *
                 * @param logger A reference to a Logger instance for logging.
                 * @param lessons A vector containing Lesson objects to initialize the quiz.
                 */
                QuizWidget(tools::Logger& logger, const std::vector<Lesson>& lessons);

                /**
                 * @brief Draws the quiz widget on the screen.
                 *
                 * This function is responsible for rendering the quiz interface using ImGui.
                 */
                void draw();

            private:
                tools::Logger& m_logger; /**< Reference to the Logger instance for logging. */
                QuizGame quizGame; /**< Instance of QuizGame to manage quiz logic. */
                bool isQuizWindowOpen = true; /**< Boolean flag to track if the quiz window is open. */
            };
        }
    }
}
