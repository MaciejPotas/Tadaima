/**
 * @file VocabularyQuiz.h
 * @brief Declaration of the VocabularyQuiz class for managing vocabulary quizzes.
 *
 * This file contains the declaration of the VocabularyQuiz class, which is used
 * to manage the state and progress of a vocabulary quiz. The quiz uses flashcards
 * to test the user's knowledge and keeps track of correct answers and mistakes.
 */

#pragma once

#include "QuizWord.h"
#include <vector>
#include <unordered_map>
#include <string>



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
                 * @struct WordStatistics
                 * @brief A structure to hold statistics for each word in the quiz.
                 *
                 * This structure keeps track of the number of good and bad attempts
                 * for each word and whether the word has been learnt.
                 */
                struct WordStatistics
                {
                    int goodAttempts; ///< Number of good attempts.
                    int badAttempts; ///< Number of bad attempts.
                    bool learnt; ///< Whether the word has been learnt.

                    /**
                     * @brief Constructor for WordStatistics.
                     *
                     * Initializes the statistics with zero attempts and not learnt status.
                     */
                    WordStatistics() : goodAttempts(0), badAttempts(0), learnt(false) {}
                };

                /**
                 * @brief Constructor for the VocabularyQuiz class.
                 *
                 * Initializes the quiz with a set of flashcards, the required number of correct
                 * answers for each flashcard, and an option to shuffle the flashcards.
                 *
                 * @param flashcards Reference to a vector of flashcards to be used in the quiz.
                 * @param requiredCorrectAnswers The number of correct answers required for each flashcard.
                 * @param enableShuffle Boolean indicating whether to shuffle the flashcards.
                 */
                VocabularyQuiz(std::vector<QuizWord>& flashcards, int requiredCorrectAnswers, bool enableShuffle = true);

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
                 * @brief Retrieves the number of flashcards in the quiz.
                 *
                 * @return The number of flashcards.
                 */
                uint32_t getNumberOflashcards() const;

                /**
                 * @brief Retrieves the number of learnt words in the quiz.
                 *
                 * @return The number of learnt words.
                 */
                uint32_t getLearntWords() const;

                /**
                 * @brief Retrieves the statistics for each word in the quiz.
                 *
                 * @return A const reference to an unordered map of word statistics.
                 */
                const std::unordered_map<int, WordStatistics>& getStatistics() const;

                /**
                 * @brief Retrieves the current flashcard.
                 *
                 * This function returns a reference to the current flashcard.
                 *
                 * @return A reference to the current flashcard.
                 */
                const QuizWord& getCurrentFlashCard() const;

            private:

                /**
                 * @brief Moves to the next flashcard in the quiz.
                 *
                 * This function advances the quiz to the next flashcard, updating the current index
                 * and selecting the next flashcard.
                 */
                void moveToNextFlashcard();

                std::vector<QuizWord> m_flashcards; ///< The vector of flashcards used in the quiz.
                std::unordered_map<int, WordStatistics> m_statistics; ///< Map of word IDs to their statistics.
                std::unordered_map<int, bool> learntStatus; ///< Map of word IDs to their learnt status.

                QuizWord* m_currentFlashcard = nullptr; ///< Pointer to the current flashcard.

                int m_currentIndex = 0; ///< Index of the current flashcard.
                int m_requiredCorrectAnswers; ///< The number of correct answers required for each flashcard.
                bool m_shuffleEnabled; ///< Boolean indicating whether shuffling is enabled.
            };
        }
    }
}
