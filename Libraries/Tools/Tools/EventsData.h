#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <algorithm>
#include <format>
#include "DataPackage.h"

namespace tools
{
    /**
     * @brief Represents a flexible event data handling system.
     *
     * The EventData class manages different types of events identified by EventType.
     * Each event can hold custom data defined by EventDataType.
     */
    template<typename EventPackage>
    class EventsData
    {
    public:

        /**
         * @brief Alias for storing complex data packages.
         *
         * This alias defines the type of data package associated with events.
         * It's templated to allow flexibility in the type of data stored with events.
         */
        using DataId = uint8_t;
        using EventType = uint8_t;
        using EventDataType = tools::ComplexDataPackage<EventType, EventPackage>;

        /**
         * @brief Sets or updates an event with given type and data.
         *
         * @param type The type of the event.
         * @param data The data associated with the event.
         */
        template<typename DataType>
        void setEvent(EventType type, const DataType& data)
        {
            events[type] = { true };
            events[type].data.set(type, data);
        }

        void setEvent(EventType type, const EventDataType& data)
        {
            events[type] = { true };
            events[type].data = data;
        }

        /**
         * @brief Sets an event without associated data.
         *
         * @param id Unique identifier for the event.
         * @param type The type of the event.
         */
        void setEvent(EventType type)
        {
            events[type] = { true };
        }

        template<typename DataType>
        DataType getEventData(EventType type) const
        {
            auto it = events.find(type);
            if( it != events.end() && it->second.occurred )
            {
                return it->second.data.get<DataType>(type);
            }

            throw std::invalid_argument(std::format("No data for type {}!", type));
        }

        /**
         * @brief Checks if an event of a specified type has occurred.
         *
         * @param type The type of the event to check.
         * @return true If the event has occurred.
         * @return false If the event has not occurred.
         */
        bool isEventOccurred(EventType type) const
        {
            auto it = events.find(type);
            return it != events.end() && it->second.occurred;
        }

        /**
         * @brief Determines if any event has changed its state to occurred.
         *
         * @return true If any event is marked as occurred.
         * @return false If no events are marked as occurred.
         */
        bool anyEventChanged() const
        {
            return std::any_of(events.begin(), events.end(),
                [](const auto& pair)
                {
                    return pair.second.occurred;
                });
        }

        /**
         * @brief Retrieves the unique identifier associated with an event type.
         *
         * @param type The type of the event.
         * @return The unique identifier of the event.
         */
        std::string getEventId(EventType type) const
        {
            auto it = events.find(type);
            if( it != events.end() && it->second.occurred )
            {
                return it->second.id;
            }
            return "";
        }

        /**
         * @brief Clears the occurred status of an event of a specified type.
         *
         * @param type The type of the event to clear.
         */
        void clearEvent(EventType type)
        {
            auto it = events.find(type);
            if( it != events.end() )
            {
                it->second.occurred = false;
            }
        }

    private:

        /**
         * @brief Struct to hold information about events.
         *
         * This struct contains metadata and data associated with each event.
         */
        struct EventInfo
        {
            bool occurred = false;    ///< Indicates whether the event has occurred.
            EventDataType data;       ///< Data associated with the event.
        };

        std::map<EventType, EventInfo> events; ///< Maps event types to their corresponding EventInfo.
    };
}
