/**
 * @file ApplicationSettingsWidget.h
 * @brief Defines the ApplicationSettingsWidget class for managing application settings in the GUI.
 */

#pragma once

#include "Widget.h"
#include <string>
#include "quiz/QuizWordType.h"

namespace tools { class Logger; }

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @class ApplicationSettingsWidget
             * @brief Provides a widget for managing application settings.
             */
            class ApplicationSettingsWidget : public Widget
            {
            public:

                /**
                 * @enum ApplicationSettingsWidgetEvent
                 * @brief Enum for application settings widget events.
                 */
                enum ApplicationSettingsWidgetEvent : uint8_t
                {
                    OnSettingsChanged /**< Event triggered when settings are changed. */
                };

                /**
                 * @brief Constructs an ApplicationSettingsWidget object.
                 *
                 * Initializes the widget and prepares it to handle application settings.
                 *
                 * @param logger Reference to a Logger instance for logging.
                 */
                ApplicationSettingsWidget(tools::Logger& logger);

                /**
                 * @brief Draws the settings widget.
                 *
                 * Renders the application settings widget on the GUI.
                 *
                 * @param p_open Pointer to a boolean indicating if the widget should remain open.
                 */
                virtual void draw(bool* p_open) override;

                /**
                 * @brief Opens the settings widget.
                 *
                 * Opens the modal window for the settings widget.
                 */
                void Open();

                /**
                 * @brief Initializes the settings widget with the given data package.
                 *
                 * Populates the widget's fields with data from the provided package.
                 *
                 * @param r_package The data package for initialization.
                 */
                void initialize(const tools::DataPackage& r_package) override;

            private:

                /**
                 * @brief Applies the current settings.
                 *
                 * Saves the settings entered in the widget and triggers the necessary events.
                 */
                void ApplySettings();

                /**
                 * @brief Converts a string to a WordType.
                 *
                 * Maps a string representation to the corresponding `WordType` enum value.
                 *
                 * @param str The string to convert.
                 * @return The corresponding WordType.
                 * @throws std::invalid_argument If the string is invalid.
                 */
                tadaima::quiz::WordType stringToWordType(const std::string& str);

                /**
                 * @brief Converts a WordType to a string.
                 *
                 * Maps a `WordType` enum value to its string representation.
                 *
                 * @param type The WordType to convert.
                 * @return The corresponding string.
                 * @throws std::invalid_argument If the WordType is invalid.
                 */
                std::string wordTypeToString(tadaima::quiz::WordType type);

                /**
                 * @brief Shows a help marker with the given description.
                 *
                 * Displays a small question mark icon that shows the description when hovered.
                 *
                 * @param desc The description to display in the help marker.
                 */
                //void ShowHelpMarker(const char* desc);

                /**
                 * @brief Shows field help with the given description.
                 *
                 * Displays a tooltip with detailed information when the field is hovered.
                 *
                 * @param desc The description to display in the field help.
                 */
                void ShowFieldHelp(const char* desc);

                tools::Logger& m_logger; /**< Reference to the Logger instance for logging. */
                char m_dictionaryPath[50] = ""; /**< Path to the dictionary used by the application. */
                char m_conjugationPath[50] = ""; /**< Path to the conjugation used by the application. */
                char m_scriptPaths[50] = ""; /**< Path to the directory containing quiz scripts. */
                char m_username[30] = ""; /**< Username for the application. */
                int m_inputOption = tadaima::quiz::WordType::BaseWord; /**< The input word type for quizzes. */
                int m_translationOption = tadaima::quiz::WordType::Romaji; /**< The translation word type for quizzes. */
                int m_numberOfTries = 1; /**< The number of tries allowed for answering a quiz question. */
                bool m_showlogs = false; /**< Flag indicating whether logs should be displayed. */
                uint16_t m_conjugationBits = 0; // 16 bits for up to 16 conjugation types
            };
        }
    }
}
