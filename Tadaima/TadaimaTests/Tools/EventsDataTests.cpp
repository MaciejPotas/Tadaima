#include <gtest/gtest.h>
#include "Tools/EventsData.h"

// Define some mock types to use for testing
struct EventPackage
{
    int value;
    bool operator==(const EventPackage& other) const
    {
        return value == other.value;
    }
};

struct AdditionalParam
{
    std::string text;
    bool operator==(const AdditionalParam& other) const
    {
        return text == other.text;
    }
};

using TestEventType = uint8_t;
constexpr TestEventType EVENT_TYPE_1 = 1;
constexpr TestEventType EVENT_TYPE_2 = 2;

using TestEventsData = tools::EventsData<EventPackage, AdditionalParam>;

// Test setting and getting event data for EventPackage type
TEST(EventsDataTest, SetAndGetEventData_EventPackage)
{
    TestEventsData eventsData;
    EventPackage eventPackage = { 42 };

    eventsData.setEvent(EVENT_TYPE_1, eventPackage);

    ASSERT_TRUE(eventsData.isEventOccurred(EVENT_TYPE_1));
    EXPECT_EQ(eventsData.getEventData<EventPackage>(EVENT_TYPE_1), eventPackage);
}

// Test setting and getting event data for AdditionalParam type
TEST(EventsDataTest, SetAndGetEventData_AdditionalParam)
{
    TestEventsData eventsData;
    AdditionalParam additionalParam = { "test" };

    eventsData.setEvent(EVENT_TYPE_2, additionalParam);

    ASSERT_TRUE(eventsData.isEventOccurred(EVENT_TYPE_2));
    EXPECT_EQ(eventsData.getEventData<AdditionalParam>(EVENT_TYPE_2), additionalParam);
}

// Test setting event without data
TEST(EventsDataTest, SetEventWithoutData)
{
    TestEventsData eventsData;

    eventsData.setEvent(EVENT_TYPE_1);

    ASSERT_TRUE(eventsData.isEventOccurred(EVENT_TYPE_1));
}

// Test clearing an event
TEST(EventsDataTest, ClearEvent)
{
    TestEventsData eventsData;
    EventPackage eventPackage = { 42 };

    eventsData.setEvent(EVENT_TYPE_1, eventPackage);
    eventsData.clearEvent(EVENT_TYPE_1);

    EXPECT_FALSE(eventsData.isEventOccurred(EVENT_TYPE_1));
}

// Test checking if any event has changed
TEST(EventsDataTest, AnyEventChanged)
{
    TestEventsData eventsData;
    EventPackage eventPackage = { 42 };

    EXPECT_FALSE(eventsData.anyEventChanged());

    eventsData.setEvent(EVENT_TYPE_1, eventPackage);

    EXPECT_TRUE(eventsData.anyEventChanged());
}

// Test exception when getting data for non-existent event
TEST(EventsDataTest, GetNonExistentEventData)
{
    TestEventsData eventsData;

    EXPECT_THROW(eventsData.getEventData<EventPackage>(EVENT_TYPE_1), std::runtime_error);
}

// Test exception when getting data of wrong type
TEST(EventsDataTest, GetWrongTypeEventData)
{
    TestEventsData eventsData;
    EventPackage eventPackage = { 42 };

    eventsData.setEvent(EVENT_TYPE_1, eventPackage);

    EXPECT_THROW(eventsData.getEventData<AdditionalParam>(EVENT_TYPE_1), std::runtime_error);
}

// Test setting event with EventDataType
TEST(EventsDataTest, SetEventWithEventDataType)
{
    TestEventsData eventsData;
    TestEventsData::EventDataType eventData;
    EventPackage eventPackage = { 42 };
    eventData.set(EVENT_TYPE_1, eventPackage);

    eventsData.setEvent(EVENT_TYPE_1, eventData);

    ASSERT_TRUE(eventsData.isEventOccurred(EVENT_TYPE_1));
    EXPECT_EQ(eventsData.getEventData<EventPackage>(EVENT_TYPE_1), eventPackage);
}
