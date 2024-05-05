/**
 * @file VocabularySettingsWidget.h
 * @brief Defines the VocabularySettingsWidget class, representing the vocabulary settings widget.
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
             * @brief The VocabularySettingsWidget class represents the vocabulary settings widget.
             */
            class VocabularySettingsWidget : public Widget
            {
            public:
                /**
                 * @brief Initializes the vocabulary settings widget.
                 *
                 * @param r_package The data package for initialization.
                 */
                void initialize(const tools::DataPackage& r_package) override;

                /**
                 * @brief Draws the vocabulary settings widget.
                 *
                 * @param p_open Pointer to a boolean indicating whether the widget is open.
                 */
                void draw(bool* p_open) override;
            };
        }
    }
}
