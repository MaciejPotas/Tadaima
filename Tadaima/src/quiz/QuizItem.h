/**
 * @file QuizWord.h
 * @brief Defines the QuizWord struct used in quizzes.
 */

#pragma once

#include <string>

namespace tadaima
{
    namespace quiz
    {
        /**
         * @class QuizItem
         * @brief Abstract base class for a generic quiz item.
         *
         * This class defines the interface for quiz items, such as providing a key and an answer.
         */
        class QuizItem
        {
        public:
            virtual ~QuizItem() = default;

            /**
             * @brief Gets the unique key of the quiz item.
             *
             * @return The key of the quiz item.
             */
            virtual std::string getKey() const = 0;

            /**
             * @brief Gets the expected answer for the quiz item.
             *
             * @return The correct answer for the quiz item.
             */
            virtual std::string getAnswer() const = 0;
        };
    }
}
