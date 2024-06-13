/**
 * @file ApplicationSettingsWidget.h
 * @brief Defines the ApplicationSettingsWidget class for managing application settings in the GUI.
 */

#pragma once

#include "Widget.h"
#include <string>
#include <quiz/QuizType.h>

namespace tools { class Logger; }

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @brief The ApplicationSettingsWidget class provides a widget for managing application settings.
             */
            class ApplicationSettingsWidget : public Widget
            {
            public:

                /**
                 * @brief Enum for application settings widget events.
                 */
                enum ApplicationSettingsWidgetEvent : uint8_t
                {
                    OnSettingsChanged /**< Event triggered when settings are changed. */
                };

                /**
                 * @brief Constructs an ApplicationSettingsWidget object.
                 * @param logger Reference to a Logger instance for logging.
                 */
                ApplicationSettingsWidget(tools::Logger& logger);

                /**
                 * @brief Draws the settings widget.
                 * @param p_open Pointer to a boolean indicating if the widget should remain open.
                 */
                virtual void draw(bool* p_open) override;

                /**
                 * @brief Opens the settings widget.
                 */
                void Open();

                /**
                 * @brief Initializes the settings widget with the given data package.
                 * @param r_package The data package for initialization.
                 */
                void initialize(const tools::DataPackage& r_package) override;

            private:

                /**
                 * @brief Applies the current settings.
                 */
                void ApplySettings();

                /**
                 * @brief Converts a string to a WordType.
                 * @param str The string to convert.
                 * @return The corresponding WordType.
                 */
                quiz::WordType stringToWordType(const std::string& str);

                /**
                 * @brief Converts a WordType to a string.
                 * @param type The WordType to convert.
                 * @return The corresponding string.
                 */
                std::string wordTypeToString(quiz::WordType type);

                /**
                 * @brief Shows a help marker with the given description.
                 * @param desc The description to display in the help marker.
                 */
                void ShowHelpMarker(const char* desc);

                /**
                 * @brief Shows field help with the given description.
                 * @param desc The description to display in the field help.
                 */
                void ShowFieldHelp(const char* desc);

                tools::Logger& m_logger; /**< Reference to the Logger instance for logging. */
                char m_dictionaryPath[50] = ""; /**< Path to the dictionary. */
                char m_scriptPaths[50] = ""; /**< Path to the dictionary. */
                char m_username[30] = ""; /**< Username for the application. */
                int m_inputOption = quiz::WordType::BaseWord; /**< Input option for word type. */
                int m_translationOption = quiz::WordType::Romaji; /**< Translation option for word type. */
            };
        }
    }
}
