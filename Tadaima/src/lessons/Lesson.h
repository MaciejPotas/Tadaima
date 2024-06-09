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

        // Default constructor
        Word() : id(-1) {}

        // Parameterized constructor
        Word(int id, const std::string& kana, const std::string& translation, const std::string& romaji, const std::string& exampleSentence, const std::vector<std::string>& tags)
            : id(id), kana(kana), translation(translation), romaji(romaji), exampleSentence(exampleSentence), tags(tags)
        {
        }

        // Copy constructor
        Word(const Word& other)
            : id(other.id), kana(other.kana), translation(other.translation), romaji(other.romaji), exampleSentence(other.exampleSentence), tags(other.tags)
        {
        }


        // Comparison operators
        bool operator==(const Word& other) const
        {
            return kana == other.kana &&
                translation == other.translation &&
                romaji == other.romaji &&
                exampleSentence == other.exampleSentence &&
                tags == other.tags;
        }

        bool operator!=(const Word& other) const
        {
            return !(*this == other);
        }

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

        // Comparison operators
        bool operator==(const Lesson& other) const
        {
            return id == other.id &&
                mainName == other.mainName &&
                subName == other.subName &&
                words == other.words;
        }

        bool operator!=(const Lesson& other) const
        {
            return !(*this == other);
        }

        bool isEmpty() const
        {
            return mainName.empty() && subName.empty() && words.empty();
        }
    };
}
