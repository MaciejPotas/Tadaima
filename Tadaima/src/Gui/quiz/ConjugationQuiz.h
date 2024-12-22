/**
 * @file ConjugationQuiz.h
 * @brief Declaration of the ConjugationQuiz class for managing conjugation quizzes.
 *
 * This file contains the declaration of the ConjugationQuiz class, which is used
 * to manage the state and progress of a conjugation quiz. The quiz uses flashcards
 * to test the user's knowledge of conjugations and keeps track of correct answers and mistakes.
 */

#pragma once

#include "QuizWord.h"
#include "Dictionary/Conjugations.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <array>

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            /**
             * @class ConjugationQuiz
             * @brief A class to manage a conjugation quiz using flashcards.
             *
             * The ConjugationQuiz class provides functionality to conduct a conjugation
             * quiz using a set of flashcards. It tracks the user's progress, manages
             * correct answers, and provides information about flashcards with mistakes.
             */
            class ConjugationQuiz
            {
            public:
                /**
                 * @struct ConjugationFlashCard
                 * @brief Represents a flashcard for a conjugation quiz.
                 *
                 * Each flashcard contains a word ID, a conjugation type, and the expected conjugated form.
                 */
                struct ConjugationFlashCard
                {
                    int wordId;                  ///< The unique identifier for the word.
                    ConjugationType type;        ///< The conjugation type being tested.
                    std::string answer;          ///< The correct conjugated form.

                    ConjugationFlashCard(int id, ConjugationType type, const std::string& ans)
                        : wordId(id), type(type), answer(ans)
                    {
                    }
                };

                /**
                 * @struct WordStatistics
                 * @brief A structure to hold statistics for each word in the quiz.
                 *
                 * This structure keeps track of the number of good and bad attempts
                 * for each conjugation and whether the conjugation has been learnt.
                 */
                struct WordStatistics
                {
                    int goodAttempts; ///< Number of good attempts.
                    int badAttempts; ///< Number of bad attempts.
                    bool learnt; ///< Whether the conjugation has been learnt.

                    WordStatistics() : goodAttempts(0), badAttempts(0), learnt(false) {}
                };


                struct pair_hash
                {
                    std::size_t operator()(const std::pair<int, ConjugationType>& p) const
                    {
                        // For example, hash the first int,
                        // then XOR with a shift of hashing the enum
                        auto h1 = std::hash<int>()(p.first);
                        // If ConjugationType is an enum, cast to int
                        auto h2 = std::hash<int>()(static_cast<int>(p.second));
                        return h1 ^ (h2 << 1);
                    }
                };

                /**
                 * @brief Constructor for the ConjugationQuiz class.
                 *
                 * Initializes the quiz with a set of conjugation flashcards, the required number of correct
                 * answers for each flashcard, and an option to shuffle the flashcards.
                 *
                 * @param flashcards Reference to a vector of flashcards to be used in the quiz.
                 * @param requiredCorrectAnswers The number of correct answers required for each flashcard.
                 * @param enableShuffle Boolean indicating whether to shuffle the flashcards.
                 */
                ConjugationQuiz(std::vector<ConjugationFlashCard>& flashcards, int requiredCorrectAnswers, bool enableShuffle = true);

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
                uint32_t getNumberOfFlashcards() const;

                /**
                 * @brief Retrieves the number of learnt conjugations in the quiz.
                 *
                 * @return The number of learnt conjugations.
                 */
                uint32_t getLearntConjugations() const;

                /**
                 * @brief Retrieves the statistics for each conjugation in the quiz.
                 *
                 * @return A const reference to an unordered map of conjugation statistics.
                 */
                const std::unordered_map<std::pair<int, ConjugationType>,
                    WordStatistics,
                    pair_hash>&
                    getStatistics() const;

                /**
                 * @brief Retrieves the current flashcard.
                 *
                 * This function returns a reference to the current flashcard.
                 *
                 * @return A reference to the current flashcard.
                 */
                const ConjugationFlashCard& getCurrentFlashCard() const;

            private:

                /**
                 * @brief Moves to the next flashcard in the quiz.
                 *
                 * This function advances the quiz to the next flashcard, updating the current index
                 * and selecting the next flashcard.
                 */
                void moveToNextFlashcard();

                std::vector<ConjugationFlashCard> m_flashcards; ///< The vector of flashcards used in the quiz.
                std::unordered_map<std::pair<int, ConjugationType>,
                    WordStatistics,
                    pair_hash> m_statistics;

                ///< Map of word IDs to their statistics.

                ConjugationFlashCard* m_currentFlashcard = nullptr; ///< Pointer to the current flashcard.


                int m_currentIndex = 0; ///< Index of the current flashcard.
                int m_requiredCorrectAnswers; ///< The number of correct answers required for each flashcard.
                bool m_shuffleEnabled; ///< Boolean indicating whether shuffling is enabled.
            };
        }
    }
}
