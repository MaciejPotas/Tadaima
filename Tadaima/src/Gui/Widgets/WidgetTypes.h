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
            enum class Type : uint8_t
            {
                Button, /**< Button widget type. */
                FileSystem /**< File system widget type. */
            };

            /**
             * @brief Enum representing actions for buttons.
             */
            enum class ButtonAction : uint8_t
            {
                OnClick /**< Action triggered when a button is clicked. */
            };
        }
    }
}
