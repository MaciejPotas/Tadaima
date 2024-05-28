#pragma once

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
                VocabularyQuiz      ///< A quiz focusing on vocabulary.
            };

            /**
             * @brief Enum class representing different types of answers.
             */
            enum class AnswerType : uint8_t
            {
                Romaji,     ///< Answer in Romaji (Latin script representation of Japanese).
                Kana,       ///< Answer in Kana (Japanese syllabaries including Hiragana and Katakana).
                Translation ///< Answer in the translated language.
            };
      
        }
    }
}