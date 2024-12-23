/**
 * @file ConjugationSettingsWidget.h
 * @brief Declaration of the ConjugationSettingsWidget class for managing conjugation settings in the GUI.
 *
 * This file contains the declaration of the ConjugationSettingsWidget class, which provides a GUI widget
 * for editing and managing conjugation settings for words. It interacts with the dictionary module
 * and logs the user operations for debugging and tracking.
 */

#pragma once

#include <string>
#include <array>
#include "dictionary/Conjugations.h"
#include "Widget.h"
#include "dictionary/Dictionary.h"

namespace tools { class Logger; }

namespace tadaima
{
    struct Word;

    namespace gui
    {
        namespace widget
        {
            /**
             * @class ConjugationSettingsWidget
             * @brief A GUI widget for managing conjugation settings of words.
             *
             * The ConjugationSettingsWidget class provides functionality for editing
             * conjugation forms of words within the GUI. It allows initialization with
             * a specific word, drawing the widget, and retrieving conjugation forms.
             * It also includes utilities for clearing buffers and interacting with
             * the dictionary and logger.
             */
            class ConjugationSettingsWidget
            {
            public:

                /**
                 * @brief Constructor for ConjugationSettingsWidget.
                 *
                 * Initializes the widget with a reference to the dictionary and logger.
                 *
                 * @param r_Dictionary Reference to the dictionary.
                 * @param logger Reference to the logger for tracking operations.
                 */
                ConjugationSettingsWidget(tadaima::Dictionary& r_Dictionary, tools::Logger& logger);

                /**
                 * @brief Starts the conjugation settings widget.
                 *
                 * This function is called to begin the widget's operation and prepare it
                 * for user interaction.
                 */
                void start();

                /**
                 * @brief Initializes the widget with a specific word for editing.
                 *
                 * This function sets up the widget to edit the conjugations of the provided word.
                 *
                 * @param r_WordToEdit Reference to the word to be edited.
                 */
                void initialize(tadaima::Word& r_WordToEdit);

                /**
                 * @brief Draws the conjugation settings widget.
                 *
                 * This function renders the widget in the GUI, allowing the user to view
                 * and edit conjugation forms.
                 */
                void draw();

                /**
                 * @brief Clears the widget's buffers and resets its state.
                 *
                 * This function clears all internal buffers and resets the widget
                 * to its initial state.
                 */
                void clear();

            private:

                /**
                 * @brief Clears the internal conjugation buffers.
                 *
                 * This utility function resets the conjugation buffers to their default state.
                 *
                 * @param buffers Reference to the array of conjugation buffers to clear.
                 */
                void clearConjugationBuffers(std::array<std::array<char, 128>, CONJUGATION_COUNT>& buffers);

                std::array<std::array<char, 128>, CONJUGATION_COUNT> m_conjugationBuffers; ///< Buffers for storing conjugation forms.
                char m_conjugationWord[50] = ""; ///< Buffer for the word in romaji format.
                tools::Logger& m_logger; ///< Reference to the logger for tracking operations.
                tadaima::Dictionary& mr_Dictionary; ///< Reference to the dictionary for managing words.
                tadaima::Word* mp_Word; ///< Pointer to the word being edited.
            };
        }
    }
}
