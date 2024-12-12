#pragma once

#include "gui/Widgets/WidgetTypes.h"
#include "gmock/gmock.h"
#include "Gui/Widgets/Widget.h"
#include "Gui/Gui.h"

using namespace tadaima;
using namespace tadaima::application;
using namespace tadaima::gui;
using namespace tadaima::gui::widget;
using namespace ::testing;

class MockGui : public Gui
{
public:
    MockGui(tools::Logger& logger, const config& r_config) : Gui(logger, r_config)
    {

    }

    MOCK_METHOD(void, addListener, (widget::Type type, std::function<void(const WidgetEvent*)> callback), (override));
    MOCK_METHOD(void, initializeWidget, (const tools::DataPackage& data), (override));
};