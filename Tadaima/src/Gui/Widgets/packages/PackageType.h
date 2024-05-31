/**
 * @file PackageType.h
 * @brief Defines enums related to widget's package type.
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
            enum PackageType : uint8_t
            {
     /*           MenuBar = 0,                ///< ID for the menu bar widget.
                LessonTreeView = 1,         ///< ID for the lesson tree view widget.
                Dashboard = 2,              ///< ID for the main dashboard widget.
                VocabularySettings = 3,     ///< ID for the vocabulary settings widget.*/
                Lessons = 0,
                Settings = 1,    ///< ID for the application settings widget.
                None = 2
            };

        }
    }
}
