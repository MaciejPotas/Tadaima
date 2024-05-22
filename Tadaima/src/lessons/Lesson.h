/**
 * @file LessonStructs.h
 * @brief Defines the Word and Lesson structs representing word and lesson entities.
 */

#pragma once

#include <string>
#include <vector>

namespace tadaima
{
    /**
     * @brief Struct representing a word.
     */
    struct Word
    {
        int id; /**< The ID of the word. */
        std::string kana; /**< The kana representation of the word. */
        std::string translation; /**< The translation of the word. */
        std::string romaji; /**< The romaji representation of the word. */
        std::string exampleSentence; /**< An example sentence using the word. */
        std::vector<std::string> tags; /**< Tags associated with the word. */
    };

    /**
     * @brief Struct representing a lesson.
     */
    struct Lesson
    {
        int id; /**< The ID of the lesson. */
        std::string mainName; /**< The main name of the lesson. */
        std::string subName; /**< The sub name of the lesson. */
        std::vector<Word> words; /**< Words associated with the lesson. */
    };
}
