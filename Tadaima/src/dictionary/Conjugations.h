#pragma once

#include <string>

namespace tadaima
{
    /**
     * @enum ConjugationType
     * @brief Represents different types of verb conjugations in the Japanese language.
     */
    enum ConjugationType
    {
        PLAIN,                  /**< Plain (dictionary) form of the verb. */
        POLITE,                 /**< Polite form (e.g., desu, masu). */
        NEGATIVE,               /**< Negative form (e.g., nai). */
        POLITE_NEGATIVE,        /**< Polite negative form (e.g., masen). */
        PAST,                   /**< Past tense form (e.g., ta). */
        POLITE_PAST,            /**< Polite past tense form (e.g., mashita). */
        PAST_NEGATIVE,          /**< Past negative form (e.g., nakatta). */
        POLITE_PAST_NEGATIVE,   /**< Polite past negative form (e.g., masen deshita). */
        TE_FORM,                /**< Te-form of the verb, used for linking sentences or requests. */
        POTENTIAL,              /**< Potential form (e.g., rareru, dekiru). */
        PASSIVE,                /**< Passive form (e.g., rareru, sareru). */
        CAUSATIVE,              /**< Causative form (e.g., saseru, seru). */
        CONDITIONAL,            /**< Conditional form (e.g., ba, tara). */
        VOLITIONAL,             /**< Volitional form (e.g., ou, you, shiyou). */
        IMPERATIVE,             /**< Imperative form (commands). */
        CONJUGATION_COUNT       /**< Total number of conjugation types. Used for iteration or validation. */
    };

    /**
     * @brief Converts a ConjugationType enum value to its string representation.
     *
     * This function provides a human-readable string representation of a given
     * ConjugationType enum value, useful for debugging, logging, or displaying in a GUI.
     *
     * @param type The ConjugationType value to convert.
     * @return A string representation of the specified ConjugationType.
     */
    std::string ConjugationTypeToString(ConjugationType type);

    /**
     * @brief Converts a ConjugationType enum value to its string representation.
     *
     * This function provides a human-readable string representation of a given
     * ConjugationType enum value, useful for debugging, logging, or displaying in a GUI.
     *
     * @param type The ConjugationType value to convert.
     * @param word The word to include in the question.
     * @return A string representation of the specified ConjugationType.
     */
    std::string ConjugationTypeToFullQuiestion(ConjugationType type, const std::string& romaji, const std::string& translation);
}
