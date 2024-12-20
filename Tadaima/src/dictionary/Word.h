#pragma once

#include "Conjugations.h"
#include <string>
#include <array>
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
        std::string kanji; /**< The kanji representation of the word. */
        std::string translation; /**< The translation of the word. */
        std::string romaji; /**< The romaji representation of the word. */
        std::string exampleSentence; /**< An example sentence using the word. */
        std::vector<std::string> tags; /**< Tags associated with the word. */
        std::array<std::string, CONJUGATION_COUNT> conjugations;

        // Default constructor
        Word() : id(-1) {}

        // Parameterized constructor
        Word(int id, const std::string& kana, const std::string& kanji, const std::string& translation, const std::string& romaji, const std::string& exampleSentence, const std::vector<std::string>& tags)
            : id(id), kana(kana), kanji(kanji), translation(translation), romaji(romaji), exampleSentence(exampleSentence), tags(tags)
        {
        }

        // Copy constructor
        Word(const Word& other)
            : id(other.id), kana(other.kana), kanji(other.kanji), translation(other.translation), romaji(other.romaji), exampleSentence(other.exampleSentence), tags(other.tags), conjugations(other.conjugations)
        {
        }

        // Comparison operators
        bool operator==(const Word& other) const
        {
            return kana == other.kana &&
                kanji == other.kanji &&
                translation == other.translation &&
                romaji == other.romaji &&
                exampleSentence == other.exampleSentence &&
                tags == other.tags &&
                conjugations == other.conjugations;
        }

        bool operator!=(const Word& other) const
        {
            return !(*this == other);
        }

        void clear()
        {
            id = -1;
            kana.clear();
            kanji.clear();
            translation.clear();
            romaji.clear();
            exampleSentence.clear();
            tags.clear();
            conjugations.fill("");
        }
    };
}