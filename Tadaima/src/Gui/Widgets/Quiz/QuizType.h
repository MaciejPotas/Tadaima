#pragma once

#include <cstdint>

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            /**
             * @brief Enum class representing different types of quizzes.
             */
            enum class QuizType : uint8_t
            {
                MultipleChoiceQuiz, ///< A quiz with multiple choice questions.
                VocabularyQuiz,     ///< A quiz focusing on vocabulary.
                ConjuactionQuiz     ///< A quiz focusing on testing the conjuaction.
            };
      
        }
    }
}