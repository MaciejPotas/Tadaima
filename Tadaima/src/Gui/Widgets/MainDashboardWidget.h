/**
 * @file MainDashboardWidget.h
 * @brief Defines the MainDashboardWidget class, representing the main dashboard widget.
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
             * @brief A widget representing the main dashboard.
             */
            class MainDashboardWidget : public Widget
            {
            public:
                /**
                 * @brief Initializes the main dashboard widget.
                 *
                 * @param r_package The data package for initialization.
                 */
                void initialize(const tools::DataPackage& r_package) override;

                /**
                 * @brief Draws the main dashboard widget.
                 *
                 * @param p_open Pointer to a boolean indicating whether the widget is open.
                 */
                void draw(bool* p_open) override;
            };
        }
    }
}
