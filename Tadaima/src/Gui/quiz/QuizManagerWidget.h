/**
 * @file QuizManager.h
 * @brief Declares the QuizManagerWidget class for managing quiz widgets and their interactions.
 */

#pragma once

#include "QuizType.h"
#include "widgets/Widget.h"
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
             * @class QuizManagerWidget
             * @brief Manages the lifecycle and display of quiz widgets.
             */
            class QuizManagerWidget : public widget::Widget
            {
            public:

                /**
                 * @brief Constructs a new QuizManagerWidget object.
                 *
                 * Initializes the QuizManagerWidget with a logger for event tracking.
                 *
                 * @param logger A reference to a Logger instance for logging activities.
                 */
                QuizManagerWidget(tools::Logger& logger);

                /**
                 * @brief Starts a quiz for the given lessons.
                 *
                 * Creates and initializes a new QuizWidget for the provided lessons,
                 * based on the specified quiz type.
                 *
                 * @param type The type of quiz to start.
                 * @param lessons A vector containing Lesson objects to be used in the quiz.
                 */
                void startQuiz(QuizType type, const std::vector<Lesson>& lessons);

                /**
                 * @brief Draws the current quiz widget.
                 *
                 * Renders the active quiz widget on the screen if it is open.
                 *
                 * @param p_open Pointer to a boolean indicating if the widget should remain open.
                 *               If nullptr, the widget remains open by default.
                 */
                void draw(bool* p_open = nullptr) override;

                /**
                 * @brief Initializes the QuizManagerWidget with the provided data package.
                 *
                 * Configures the QuizManagerWidget with the settings and data
                 * supplied in the data package.
                 *
                 * @param r_package The data package containing initialization data.
                 */
                void initialize(const tools::DataPackage& r_package) override;

            private:

                quiz::WordType m_answerWordType = quiz::WordType::BaseWord; /**< Word type for the quiz answers. */
                quiz::WordType m_askedWordType = quiz::WordType::Romaji; /**< Word type for the quiz questions. */
                uint8_t m_triesForAWord = 1; /**< The maximum number of attempts allowed for each word in the quiz. */
                uint16_t m_conjugationMask = 0; /**< The conjugation mask for the conjugation quiz settings. */

                QuizType m_quizType; /**< The current quiz type being managed. */
                tools::Logger& m_logger; /**< Reference to the Logger instance for logging activities. */
                std::unique_ptr<widget::Widget> m_quiz; /**< Unique pointer to the active QuizWidget. */
                bool quizWidgetOpen = false; /**< Boolean flag indicating if the quiz widget is currently open. */
            };
        }
    }
}
