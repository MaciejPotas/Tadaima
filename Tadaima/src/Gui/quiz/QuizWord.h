/**
 * @file QuizWord.h
 * @brief Defines the QuizWord struct used in quizzes.
 */

#pragma once

#include <string>

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            /**
             * @struct QuizWord
             * @brief Represents a word used in a quiz.
             *
             * The QuizWord struct holds the ID and the word itself used in quizzes.
             */
            struct QuizWord
            {
                int wordId; /**< The unique identifier for the word. */
                std::string word; /**< The word used in the quiz. */

                /**
                 * @brief Constructs a new QuizWord object.
                 *
                 * @param id The unique identifier for the word.
                 * @param word The word itself.
                 */
                QuizWord(int id, const std::string& word) : wordId(id), word(word) {}
            };
        }
    }
}
