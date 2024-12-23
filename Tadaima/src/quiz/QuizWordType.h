/**
 * @file QuizItem.h
 * @brief Defines the QuizItem struct used in quizzes.
 */

#pragma once

#include <cstdint>

namespace tadaima
{
    namespace quiz
    {
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
