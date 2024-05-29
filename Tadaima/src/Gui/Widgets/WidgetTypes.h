/**
 * @file WidgetEnums.h
 * @brief Defines enums related to GUI widgets.
 */

#pragma once
#include <cstdint>

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @brief Enum representing types of widgets.
             */
            enum Type : uint8_t
            {
                MenuBar = 0,                ///< ID for the menu bar widget.
                LessonTreeView = 1,         ///< ID for the lesson tree view widget.
                Dashboard = 2,              ///< ID for the main dashboard widget.
                VocabularySettings = 3,     ///< ID for the vocabulary settings widget.
                ApplicationSettings = 4,    ///< ID for the application settings widget.
                Npc = 5
            };

            /**
             * @brief Enum representing actions for buttons.
             */
            enum ButtonAction : uint8_t
            {
                OnClick /**< Action triggered when a button is clicked. */
            };
        }
    }
}
