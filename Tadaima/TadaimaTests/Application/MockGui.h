#pragma once

#include "gui/Widgets/WidgetTypes.h"
#include "gmock/gmock.h"
#include "Gui/Widgets/Widget.h"

namespace tadaima
{
    namespace gui
    {
        class Gui
        {
        public:
            MOCK_METHOD(void, addListener, (gui::widget::Type, std::function<void(const gui::widget::WidgetEvent*)>), ());
            MOCK_METHOD(void, initializeWidget, (gui::widget::Type, const tools::DataPackage&), ());
        };
    }
}
