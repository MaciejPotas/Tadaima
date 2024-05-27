/**
 * @file QuizManager.h
 * @brief Declares the QuizManager class for managing quiz widgets and their interactions.
 */

#pragma once

#include "lessons/Lesson.h"
#include <memory>
#include <vector>
#include "Widgets/QuizWidget.h"
#include "QuizType.h"

namespace tools { class Logger; }


namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            /**
             * @class QuizManager
             * @brief Manages the lifecycle and display of quiz widgets.
             */
            class QuizManager
            {
            public:
                /**
                 * @brief Constructs a new QuizManager object.
                 *
                 * @param logger A reference to a Logger instance for logging.
                 */
                QuizManager(tools::Logger& logger);

                /**
                 * @brief Starts a quiz for the given lessons.
                 *
                 * Initializes a new QuizWidget for the provided lessons.
                 *
                 * @param lessons A vector containing Lesson objects to create a quiz for.
                 */
                void startQuiz(QuizType type, const std::vector<Lesson>& lessons);

                /**
                 * @brief Draws the current quiz widget.
                 *
                 * Renders the quiz widget on the screen if it is open.
                 */
                void draw();

            private:

                QuizType m_quizType;
                tools::Logger& m_logger; /**< Reference to the Logger instance for logging. */
                std::unique_ptr<widget::Widget> m_quiz; /**< Unique pointer to the QuizWidget. */
                bool quizWidgetOpen; /**< Boolean flag to track if the quiz widget is open. */
            };
        }
    }
}