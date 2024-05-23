#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "MockApplication.h"
#include "MockGui.h"
#include "bridge/EventBridge.h"
#include "gui/Widgets/WidgetTypes.h"

using namespace tadaima;
using ::testing::_;
using ::testing::Return;
using ::testing::SaveArg;

class MockWidget : public gui::widget::Widget
{
public:
    MockWidget() : Widget(gui::widget::Type::LessonTreeView) {}

    MOCK_METHOD(void, draw, (bool* p_open), (override));
};

class EventBridgeTest : public ::testing::Test
{
protected:
    tadaima::application::Application mockApplication;
    tadaima::gui::Gui mockGui;
    EventBridge eventBridge;
    MockWidget mockWidget;

    void SetUp() override
    {
        eventBridge.initialize(mockApplication, mockGui);
    }
};

TEST_F(EventBridgeTest, InitializeAddsListener)
{
    EXPECT_CALL(mockGui, addListener(gui::widget::Type::LessonTreeView, _)).Times(1);
    eventBridge.initialize(mockApplication, mockGui);
}

TEST_F(EventBridgeTest, InitializeGui)
{
    std::vector<Lesson> lessons = {
        {1, "Main Name 1", "Sub Name 1", {}},
        {2, "Main Name 2", "Sub Name 2", {}}
    };

    EXPECT_CALL(mockGui, initializeWidget(gui::widget::Type::LessonTreeView, _)).Times(1);
    eventBridge.initializeGui(lessons);
}

TEST_F(EventBridgeTest, HandleEventLessonCreated)
{
    gui::widget::LessonTreeViewWidget::LessonDataPackage dataPackage(1);
    gui::widget::WidgetEvent event(mockWidget, gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnLessonCreated, &dataPackage);

    EXPECT_CALL(mockApplication, setEvent(application::ApplicationEvent::OnLessonCreated, _)).Times(1);
    eventBridge.handleEvent(&event);
}

TEST_F(EventBridgeTest, HandleEventLessonRename)
{
    gui::widget::LessonTreeViewWidget::LessonDataPackage dataPackage(1);
    gui::widget::WidgetEvent event(mockWidget, gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnLessonRename, &dataPackage);

    EXPECT_CALL(mockApplication, setEvent(application::ApplicationEvent::OnLessonUpdate, _)).Times(1);
    eventBridge.handleEvent(&event);
}

TEST_F(EventBridgeTest, HandleInvalidEvent)
{
    gui::widget::WidgetEvent event(mockWidget, static_cast<gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent>(999), nullptr);

    EXPECT_THROW(eventBridge.handleEvent(nullptr), std::invalid_argument);
    EXPECT_THROW(eventBridge.handleEvent(&event), std::invalid_argument);
}
