/**
 * @file VocabularyQuizWidget.h
 * @brief Declaration of the VocabularyQuizWidget class for displaying vocabulary quizzes.
 *
 * This file contains the declaration of the VocabularyQuizWidget class, which is responsible
 * for rendering the vocabulary quiz interface and handling user interactions.
 */

#pragma once

#include "imgui.h"
#include "quiz/VocabularyQuiz.h"
#include "Tools/Logger.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "quiz/Flashcard.h"
#include <unordered_set>
#include "Widget.h"

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

                std::vector<quiz::Flashcard> m_flashcards; ///< Vector of flashcards for the quiz.
                std::unique_ptr<quiz::VocabularyQuiz> m_quiz; ///< Unique pointer to the VocabularyQuiz instance.
                char m_userInput[50] = { 0 }; ///< User input buffer.
                std::unordered_set<int> revealedHints; ///< Set of indices of revealed hints.
                std::string currentHint; ///< Current hint for the user.
                bool showCorrectAnswer = false; ///< Boolean indicating whether to show the correct answer.
                std::string correctAnswer; ///< The correct answer for the current flashcard.
                std::string correctAnswerMessage; ///< Message to display when the user provides the correct answer.
            };
        }
    }
}