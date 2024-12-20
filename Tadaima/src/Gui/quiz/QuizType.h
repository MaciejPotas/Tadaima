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

            /**
             * @brief Enum class representing different types of answers.
             */
            enum WordType : uint8_t
            {
                BaseWord,   ///< Answer in the translated language.
                Kana,       ///< Answer in Kana (Japanese syllabaries including Hiragana and Katakana).
                Romaji,     ///< Answer in Romaji (Latin script representation of Japanese).
            };
      
        }
    }
}