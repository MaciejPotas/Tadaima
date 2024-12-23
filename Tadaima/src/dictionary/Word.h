#pragma once

#include "Conjugations.h"
#include <string>
#include <array>
#include <vector>

namespace tadaima
{
    /**
     * @struct Word
     * @brief Represents a word with its properties, including conjugations and metadata.
     *
     * The `Word` struct contains information about a word, such as its ID, kana, kanji,
     * translation, romaji, example sentence, tags, and conjugations. It provides constructors,
     * comparison operators, and a utility method for clearing its data.
     */
    struct Word
    {
        int id; /**< The unique ID of the word. */
        std::string kana; /**< The kana representation of the word. */
        std::string kanji; /**< The kanji representation of the word. */
        std::string translation; /**< The translation of the word into another language. */
        std::string romaji; /**< The romaji (romanized) representation of the word. */
        std::string exampleSentence; /**< An example sentence showcasing the use of the word. */
        std::vector<std::string> tags; /**< Tags or labels associated with the word. */
        std::array<std::string, CONJUGATION_COUNT> conjugations; /**< An array of conjugations for the word. */

        /**
         * @brief Default constructor.
         *
         * Initializes the word with a default ID of -1 and clears all fields.
         */
        Word() : id(-1) {}

        /**
         * @brief Parameterized constructor.
         *
         * Initializes the word with the given properties.
         *
         * @param id The ID of the word.
         * @param kana The kana representation of the word.
         * @param kanji The kanji representation of the word.
         * @param translation The translation of the word.
         * @param romaji The romaji representation of the word.
         * @param exampleSentence An example sentence using the word.
         * @param tags A vector of tags associated with the word.
         */
        Word(int id, const std::string& kana, const std::string& kanji, const std::string& translation,
            const std::string& romaji, const std::string& exampleSentence, const std::vector<std::string>& tags)
            : id(id), kana(kana), kanji(kanji), translation(translation), romaji(romaji),
            exampleSentence(exampleSentence), tags(tags)
        {
        }

        /**
         * @brief Copy constructor.
         *
         * Copies the properties of another `Word` instance.
         *
         * @param other The `Word` instance to copy from.
         */
        Word(const Word& other)
            : id(other.id), kana(other.kana), kanji(other.kanji), translation(other.translation),
            romaji(other.romaji), exampleSentence(other.exampleSentence), tags(other.tags),
            conjugations(other.conjugations)
        {
        }

        /**
         * @brief Equality operator.
         *
         * Compares two `Word` instances for equality.
         *
         * @param other The `Word` instance to compare with.
         * @return `true` if all fields are equal, `false` otherwise.
         */
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

        /**
         * @brief Inequality operator.
         *
         * Compares two `Word` instances for inequality.
         *
         * @param other The `Word` instance to compare with.
         * @return `true` if any field is different, `false` otherwise.
         */
        bool operator!=(const Word& other) const
        {
            return !(*this == other);
        }

        /**
         * @brief Clears the fields of the word.
         *
         * Resets the ID to -1 and clears all other fields.
         */
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
