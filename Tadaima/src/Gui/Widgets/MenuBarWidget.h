/**
 * @file MenuBarWidget.h
 * @brief Defines the MenuBarWidget class, representing the menu bar widget.
 */
#pragma once

#include "Widget.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @brief A widget representing the menu bar.
             */
            class MenuBarWidget : public Widget
            {
            public:
                /**
                 * @brief Initializes the menu bar widget.
                 *
                 * @param r_package The data package for initialization.
                 */
                void initialize(const tools::DataPackage& r_package) override;

                /**
                 * @brief Draws the menu bar widget.
                 *
                 * @param p_open Pointer to a boolean indicating whether the widget is open.
                 */
                void draw(bool* p_open) override;

            private:
                /**
                 * @brief Displays the about window.
                 *
                 * @param p_open Pointer to a boolean indicating whether the window is open.
                 */
                void showAboutWindow(bool* p_open);
            };
        }
    }
}
