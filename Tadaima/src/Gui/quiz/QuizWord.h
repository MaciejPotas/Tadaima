#pragma once
#include <string>

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            struct QuizWord
            {
                int wordId;
                std::string word;

                QuizWord(int id, const std::string& word) : wordId(id), word(word) {}
            };
        }
    }
}