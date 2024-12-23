/**
 * @file QuizItem.h
 * @brief Defines the QuizItem struct used in quizzes.
 */

#pragma once

#include <quiz/QuizItem.h>

namespace tadaima
{
    namespace quiz
    {
        /**
         * @class VocabularyItem
         * @brief A concrete implementation of QuizItem for vocabulary quizzes.
         */
        class VocabularyItem : public QuizItem
        {
        public:
            VocabularyItem(int id, const std::string& word)
                : m_id(id), m_word(word)
            {
            }

            std::string getKey() const override
            {
                return std::to_string(m_id);
            }

            std::string getAnswer() const override
            {
                return m_word;
            }

        private:
            int m_id;
            std::string m_word;
        };
    }
}
