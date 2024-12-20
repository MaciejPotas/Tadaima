/**
 * @file VocabularyQuizWidget.h
 * @brief Declaration of the VocabularyQuizWidget class for displaying vocabulary quizzes.
 *
 * This file contains the declaration of the VocabularyQuizWidget class, which is responsible
 * for rendering the vocabulary quiz interface and handling user interactions.
 */

#pragma once

#include "Widget.h"
#include "quiz/VocabularyQuiz.h"
#include "tools/Logger.h"
#include <unordered_set>
#include <vector>
#include <string>
#include <memory>
#include "lessons/Lesson.h"
#include "quiz/QuizType.h"

namespace tadaima { struct Word; }

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
                 * @brief Constructs a VocabularyQuizWidget object.
                 *
                 * Initializes the quiz widget with the specified word types, number of tries, lessons, and logger.
                 *
                 * @param base The base word type for the quiz (mother language).
                 * @param desired The desired word type for the quiz (learning language).
                 * @param numberOfTries The maximum number of tries allowed for each flashcard.
                 * @param lessons A vector containing lessons to initialize the quiz with.
                 * @param logger Reference to a Logger instance for logging.
                 */
                VocabularyQuizWidget(quiz::WordType base, quiz::WordType desired, uint8_t numberOfTries, const std::vector<Lesson>& lessons, tools::Logger& logger);

                /**
                 * @brief Draws the quiz widget.
                 *
                 * Renders the quiz widget and handles user interactions. Displays progress, flashcards, and user input fields.
                 *
                 * @param p_open Pointer to a boolean indicating whether the widget window is open.
                 */
                void draw(bool* p_open) override;

            private:

                /**
                 * @brief Retrieves a word by its ID.
                 *
                 * Searches the lessons to find a word matching the provided ID.
                 *
                 * @param id The ID of the word to retrieve.
                 * @return The Word object corresponding to the given ID.
                 */
                Word getWordById(int id);

                /**
                 * @brief Gets the translation of a word.
                 *
                 * Translates the given word to the specified word type.
                 *
                 * @param word The word to translate.
                 * @param type The word type for translation (e.g., Kana, Romaji, BaseWord).
                 * @return A string containing the translation.
                 */
                std::string getTranslation(const Word& word, quiz::WordType type) const;

                /**
                 * @brief Gets a hint for the current flashcard.
                 *
                 * Generates a hint to assist the user in answering the current flashcard.
                 *
                 * @return A string containing the hint for the current flashcard.
                 */
                std::string getHint();

                /**
                 * @brief Calculates the progress for the progress bar.
                 *
                 * Computes the percentage of the quiz completed.
                 *
                 * @return A float value representing the current progress (0.0 to 1.0).
                 */
                float calculateProgress();

                tools::Logger& m_logger; /**< Reference to the logger for logging purposes. */
                quiz::WordType m_baseWord; /**< The mother language word type. */
                quiz::WordType m_inputWord; /**< The learning language word type. */
                std::vector<Lesson> m_lessons; /**< A vector of lessons used to generate flashcards. */

                std::unique_ptr<quiz::VocabularyQuiz> m_quiz; /**< Unique pointer to the VocabularyQuiz instance. */
                char m_userInput[50] = { 0 }; /**< User input buffer for answering flashcards. */
                std::unordered_set<int> m_revealedHints; /**< Set of indices of revealed hints. */

                std::string m_currentHint; /**< The current hint displayed to the user. */
                std::string m_correctAnswer; /**< The correct answer for the current flashcard. */
                std::string m_correctAnswerMessage; /**< Message displayed upon a correct answer. */
                std::string m_translation; /**< Translation of the current word. */
                std::string m_kana; /**< Kana representation of the current word. */
                std::string m_romaji; /**< Romaji representation of the current word. */
                std::string m_example; /**< Example sentence using the current word. */

                bool m_showCorrectAnswer = false; /**< Boolean indicating whether to show the correct answer. */
                bool m_overrideAnswer = false; /**< Boolean indicating whether to override the incorrect answer as correct. */
                uint8_t m_numberOfTries; /**< The maximum number of tries allowed for each word. */
            };
        }
    }
}
