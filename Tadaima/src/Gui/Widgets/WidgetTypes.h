// ----------------------------------------------------------------------------
// File:        WidgetType.h
// Description: Contains all possible widgets.
// ----------------------------------------------------------------------------
#pragma once

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            enum Type : uint8_t
            {
                Button,
                FileSystem
            };

            enum ButtonAction : uint8_t
            {
                OnClick
            };
        }
    }
}