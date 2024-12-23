/**
 * @file ConjugationItem.h
 * @brief Defines the ConjugationItem struct used in quizzes.
 */

#pragma once

#include <quiz/QuizItem.h>
#include "dictionary/Conjugations.h"

namespace tadaima
{
    namespace quiz
    {
        /**
         * @class ConjugationItem
         * @brief A concrete implementation of QuizItem for conjugation quizzes.
         */
        class ConjugationItem : public QuizItem
        {
        public:
            ConjugationItem(int id, ConjugationType type, const std::string& answer)
                : m_id(id), m_type(type), m_answer(answer)
            {
            }

            std::string getKey() const override
            {
                return std::to_string(m_id) + "_" + std::to_string(static_cast<int>(m_type));
            }

            std::string getAnswer() const override
            {
                return m_answer;
            }

            ConjugationType getType() const
            {
                return m_type;
            }

        private:
            int m_id;
            ConjugationType m_type;
            std::string m_answer;
        };
    }
}
