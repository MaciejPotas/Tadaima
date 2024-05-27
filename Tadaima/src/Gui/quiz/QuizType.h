#pragma once

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            enum class QuizType : uint8_t
            {
                MultipleChoiceQuiz,
                VocabularyQuiz
            };

            enum class AnswerType : uint8_t
            {
                Romaji,
                Kana,
                Translation
            };
      
        }
    }
}