#include "../Mocks/MockApplication.h"
#include "../Mocks/MockGui.h"
#include "Application/ApplicationSettings.h"
#include "gui/widgets/packages/LessonDataPackage.h"
#include "Gui/Widgets/LessonTreeViewWidget.h"
#include "Tools/Logger.h"

class EventBridgeTest : public ::testing::Test
{
protected:

    class EventBridgeMock : public EventBridge
    {
    public:
        using EventBridge::wordTypeToString;
        using EventBridge::stringToWordType;
    };

    class LoggerMock : public tools::Logger
    {

    public:
        virtual void log(const std::string& message, tools::LogLevel level) override
        {
        }
    };

    EventBridgeTest() : mockApp(loggerMock, eventBridge), mockGui(loggerMock, config)
    {

    }

    Gui::config config;
    LoggerMock loggerMock;
    EventBridgeMock eventBridge;
    Application mockApp;
    MockGui mockGui;

    void SetUp() override
    {
        eventBridge.initialize(mockApp, mockGui);
    }
};

TEST_F(EventBridgeTest, Initialize)
{
    EXPECT_CALL(mockGui, addListener(widget::Type::LessonTreeView, _)).Times(1);
    EXPECT_CALL(mockGui, addListener(widget::Type::ApplicationSettings, _)).Times(1);

    eventBridge.initialize(mockApp, mockGui);
}

TEST_F(EventBridgeTest, InitializeGui)
{
    std::vector<Lesson> lessons = {
        {1, "Group1", "Main1", "Sub1", {}},
        {2, "Group2", "Main2", "Sub2", {}}
    };

    EXPECT_CALL(mockGui, initializeWidget(_)).Times(1);

    eventBridge.initializeGui(lessons);
}

TEST_F(EventBridgeTest, InitializeSettings)
{
    tadaima::application::ApplicationSettings settings{
        "username",
        "path/to/dictionary",
        "path/to/scripts",
        true,
        "Kana",
        "Romaji",
        "3"
    };

    EXPECT_CALL(mockGui, initializeWidget(_)).Times(1);

    eventBridge.initializeSettings(settings);
}

TEST_F(EventBridgeTest, HandleEvent_NullData)
{
    EXPECT_THROW(eventBridge.handleEvent(nullptr), std::invalid_argument);
}

TEST_F(EventBridgeTest, StringToWordType_ValidString)
{
    EXPECT_EQ(eventBridge.stringToWordType("BaseWord"), quiz::WordType::BaseWord);
    EXPECT_EQ(eventBridge.stringToWordType("Kana"), quiz::WordType::Kana);
    EXPECT_EQ(eventBridge.stringToWordType("Romaji"), quiz::WordType::Romaji);
}

TEST_F(EventBridgeTest, StringToWordType_InvalidString)
{
    EXPECT_THROW(eventBridge.stringToWordType("InvalidType"), std::invalid_argument);
}

TEST_F(EventBridgeTest, WordTypeToString_ValidEnum)
{
    EXPECT_EQ(eventBridge.wordTypeToString(quiz::WordType::BaseWord), "BaseWord");
    EXPECT_EQ(eventBridge.wordTypeToString(quiz::WordType::Kana), "Kana");
    EXPECT_EQ(eventBridge.wordTypeToString(quiz::WordType::Romaji), "Romaji");
}

TEST_F(EventBridgeTest, WordTypeToString_InvalidEnum)
{
    EXPECT_THROW(eventBridge.wordTypeToString(static_cast<quiz::WordType>(-1)), std::invalid_argument);
}
