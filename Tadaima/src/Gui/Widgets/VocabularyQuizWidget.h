/**
 * @file VocabularyQuizWidget.h
 * @brief Declaration of the VocabularyQuizWidget class for displaying vocabulary quizzes.
 *
 * This file contains the declaration of the VocabularyQuizWidget class, which is responsible
 * for rendering the vocabulary quiz interface and handling user interactions.
 */

#pragma once

#include "Widget.h"
#include "quiz/Flashcard.h"
#include "quiz/VocabularyQuiz.h"
#include "Tools/Logger.h"
#include <unordered_set>
#include <vector>
#include <string>
#include <memory>

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @class VocabularyQuizWidget
             * @brief A class for rendering and managing a vocabulary quiz widget.
             *
             * The VocabularyQuizWidget class provides an interface for displaying a vocabulary quiz,
             * handling user input, and managing the quiz state.
             */
            class VocabularyQuizWidget : public Widget
            {
            public:
                /**
                 * @brief Constructor for the VocabularyQuizWidget class.
                 *
                 * Initializes the widget with a logger and a set of lessons to generate flashcards from.
                 *
                 * @param logger Reference to a logger for logging purposes.
                 * @param lessons Vector of lessons to generate flashcards from.
                 */
                VocabularyQuizWidget(tools::Logger& logger, const std::vector<Lesson>& lessons);

                /**
                 * @brief Draws the quiz widget.
                 *
                 * Renders the quiz widget and handles user interactions.
                 *
                 * @param p_open Pointer to a boolean indicating whether the widget window is open.
                 */
                void draw(bool* p_open) override;

            private:

                /**
                 * @brief Initializes flashcards from a set of lessons.
                 *
                 * Generates flashcards based on the provided lessons.
                 *
                 * @param lessons Vector of lessons to generate flashcards from.
                 */
                void initializeFlashcards(const std::vector<Lesson>& lessons);

                /**
                 * @brief Gets a hint for the current flashcard.
                 *
                 * Returns a hint for the current flashcard to assist the user.
                 *
                 * @return A string containing the hint for the current flashcard.
                 */
                std::string getHint();

                /**
                 * @brief Calculates the progress for the progress bar.
                 *
                 * @return A float value representing the current progress.
                 */
                float calculateProgress();

                /**
                 * @brief Gets the correct answer for the specified flashcard.
                 *
                 * @param type The type of the word to get the correct answer for.
                 * @param flashcard The flashcard to get the correct answer from.
                 * @return A string containing the correct answer.
                 */
                std::string getCorrectAnswer(quiz::WordType type, const quiz::Flashcard& flashcard);

                tools::Logger& m_logger; ///< Reference to the logger for logging purposes.
                quiz::WordType m_motherType; ///< The mother language type.
                quiz::WordType m_learnType; ///< The learning language type.

                std::vector<quiz::Flashcard> m_flashcards; ///< Vector of flashcards for the quiz.
                std::unique_ptr<quiz::VocabularyQuiz> m_quiz; ///< Unique pointer to the VocabularyQuiz instance.

                char m_userInput[30] = { 0 }; ///< User input buffer.
                std::unordered_set<int> m_revealedHints; ///< Set of indices of revealed hints.

                std::string m_currentHint; ///< Current hint for the user.
                std::string m_correctAnswer; ///< The correct answer for the current flashcard.
                std::string m_correctAnswerMessage; ///< Message to display when the user provides the correct answer.
                std::string m_translation; ///< Translation of the current word.
                std::string m_kana; ///< Kana representation of the current word.
                std::string m_romaji; ///< Romaji representation of the current word.
                std::string m_example; ///< Example sentence using the current word.

                bool m_showCorrectAnswer = false; ///< Boolean indicating whether to show the correct answer.
                bool m_overrideAnswer = false; ///< Boolean indicating whether to override the incorrect answer as correct.
            };
        }
    }
}
