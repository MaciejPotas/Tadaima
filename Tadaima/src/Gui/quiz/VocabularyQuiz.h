/**
 * @file VocabularyQuiz.h
 * @brief Declaration of the VocabularyQuiz class for managing vocabulary quizzes.
 *
 * This file contains the declaration of the VocabularyQuiz class, which is used
 * to manage the state and progress of a vocabulary quiz. The quiz uses flashcards
 * to test the user's knowledge and keeps track of correct answers and mistakes.
 */

#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <random>
#include "Flashcard.h"
#include "QuizType.h"

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            /**
             * @class VocabularyQuiz
             * @brief A class to manage a vocabulary quiz using flashcards.
             *
             * The VocabularyQuiz class provides functionality to conduct a vocabulary
             * quiz using a set of flashcards. It tracks the user's progress, manages
             * correct answers, and provides information about flashcards with mistakes.
             */
            class VocabularyQuiz
            {
            public:
                /**
                 * @brief Constructor for the VocabularyQuiz class.
                 *
                 * Initializes the quiz with a set of flashcards, the required number of correct
                 * answers for each flashcard, and an option to shuffle the flashcards.
                 *
                 * @param flashcards Reference to a vector of flashcards to be used in the quiz.
                 * @param answerType The type of answer expected (e.g., Romaji, Kanji).
                 * @param requiredCorrectAnswers The number of correct answers required for each flashcard.
                 * @param enableShuffle Boolean indicating whether to shuffle the flashcards.
                 */
                VocabularyQuiz(std::vector<Flashcard>& flashcards, WordType answerType, int requiredCorrectAnswers, bool enableShuffle = true);

                /**
                 * @brief Advances the quiz to the next flashcard based on the user's answer.
                 *
                 * This function checks the user's answer against the current flashcard and updates
                 * the progress of the quiz accordingly.
                 *
                 * @param userAnswer The answer provided by the user.
                 * @return True if the user's answer was correct, false otherwise.
                 */
                bool advance(const std::string& userAnswer);

                /**
                 * @brief Checks if the user's answer is correct.
                 *
                 * @param userAnswer The answer provided by the user.
                 * @return True if the user's answer was correct, false otherwise.
                 */
                bool isCorrect(const std::string& userAnswer);

                /**
                 * @brief Checks if the quiz is complete.
                 *
                 * This function returns true if all flashcards have been answered correctly the
                 * required number of times.
                 *
                 * @return True if the quiz is complete, false otherwise.
                 */
                bool isQuizComplete() const;

                /**
                 * @brief Retrieves the current flashcard.
                 *
                 * This function returns a reference to the current flashcard.
                 *
                 * @return A reference to the current flashcard.
                 */
                const Flashcard& getCurrentFlashCard() const;

                /**
                 * @brief Retrieves flashcards with a minimum number of mistakes.
                 *
                 * This function returns a vector of flashcards that have been answered incorrectly
                 * at least the specified number of times.
                 *
                 * @param minMistakes The minimum number of mistakes a flashcard must have.
                 * @return A vector of flashcards with at least minMistakes mistakes.
                 */
                std::vector<Flashcard> getFlashcardsWithMistakes(int minMistakes) const;

                /**
                 * @brief Retrieves the flashcards that have been learned.
                 *
                 * This function returns a vector of flashcards that have been answered correctly
                 * the required number of times.
                 *
                 * @return A vector of learned flashcards.
                 */
                std::vector<Flashcard> getLearntWords() const;

            private:
                /**
                 * @brief Moves to the next flashcard in the quiz.
                 *
                 * This function advances the quiz to the next flashcard, updating the current index
                 * and selecting the next flashcard.
                 */
                void moveToNextFlashcard();

                /**
                 * @brief Checks if all flashcards have been learned.
                 *
                 * This function returns true if all flashcards have been answered correctly the
                 * required number of times.
                 *
                 * @return True if all flashcards have been learned, false otherwise.
                 */
                bool allFlashcardsLearned() const;

                std::vector<Flashcard>& m_flashcards; ///< The vector of flashcards used in the quiz.
                std::unordered_map<int, int> m_correctAnswers; ///< Map of word IDs to correct attempts.
                Flashcard* m_currentFlashcard = nullptr; ///< Pointer to the current flashcard.
                WordType m_answerType; ///< The type of answer expected (e.g., Romaji, Kanji).
                int m_currentIndex = 0; ///< Index of the current flashcard.
                int m_requiredCorrectAnswers; ///< The number of correct answers required for each flashcard.
                bool m_shuffleEnabled; ///< Boolean indicating whether shuffling is enabled.
            };
        }
    }
}
