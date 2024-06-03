/**
 * @file QuizManager.h
 * @brief Declares the QuizManager class for managing quiz widgets and their interactions.
 */

#pragma once

#include "QuizType.h"
#include "widgets/Widget.h"
#include "widgets/QuizWidget.h"
#include "lessons/Lesson.h"
#include <memory>
#include <vector>

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
            class QuizManagerWidget : public widget::Widget
            {
            public:

                /**
                 * @brief Constructs a new QuizManagerWidget object.
                 *
                 * @param logger A reference to a Logger instance for logging.
                 */
                QuizManagerWidget(tools::Logger& logger);

                /**
                 * @brief Starts a quiz for the given lessons.
                 *
                 * Initializes a new QuizWidget for the provided lessons.
                 *
                 * @param type The type of quiz to start.
                 * @param lessons A vector containing Lesson objects to create a quiz for.
                 */
                void startQuiz(QuizType type, const std::vector<Lesson>& lessons);

                /**
                 * @brief Draws the current quiz widget.
                 *
                 * Renders the quiz widget on the screen if it is open.
                 *
                 * @param p_open Pointer to a boolean indicating if the widget should remain open.
                 */
                void draw(bool* p_open = nullptr) override;

                /**
                 * @brief Initializes the QuizManagerWidget with the given data package.
                 *
                 * @param r_package The data package for initialization.
                 */
                void initialize(const tools::DataPackage& r_package) override;

            private:

                quiz::WordType m_answerWordType = quiz::WordType::BaseWord; /**< Input option for word type. */
                quiz::WordType m_askedWordType = quiz::WordType::Romaji; /**< Translation option for word type. */

                QuizType m_quizType; /**< The current quiz type. */
                tools::Logger& m_logger; /**< Reference to the Logger instance for logging. */
                std::unique_ptr<widget::Widget> m_quiz; /**< Unique pointer to the QuizWidget. */
                bool quizWidgetOpen = false; /**< Boolean flag to track if the quiz widget is open. */
            };
        }
    }
}
