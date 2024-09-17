/**
 * @file MainDashboardWidget.h
 * @brief Defines the MainDashboardWidget class, representing the main dashboard widget.
 */
#pragma once

#include "Widget.h"
#include <string>
#include <array>

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

            private:
                std::string m_username = "Gakusei-dono";
                float m_progress = 0.0f;
                std::string m_wordOfTheDay;
                std::string m_wordMeaning;
                std::array<float, 10> m_vocabPerformance = { 0.7f, 0.8f, 0.6f, 0.9f, 0.5f, 0.6f, 0.7f, 0.8f, 0.6f, 0.9f };
                std::array<float, 10> m_pronunciationPerformance = { 0.6f, 0.7f, 0.8f, 0.7f, 0.6f, 0.7f, 0.8f, 0.6f, 0.7f, 0.8f };
            };
        }
    }
}
