/**
 * @file EventsData.h
 * @brief Header file for the EventsData template class.
 *
 * This file contains the declaration of the EventsData template class which is responsible
 * for managing events and their associated data. It includes methods for setting, retrieving,
 * and clearing events, as well as checking if any events have occurred.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <algorithm>
#include <format>
#include "DataPackage.h"

namespace tools
{
    /**
     * @brief The EventsData class template manages events and their associated data.
     *
     * @tparam EventPackage The main type of data package for the events.
     * @tparam AdditionalParams Additional types that can be included in the data package.
     */
    template<typename EventPackage, typename... AdditionalParams>
    class EventsData
    {
    public:
        using DataId = uint8_t; /**< Type alias for the data ID. */
        using EventType = uint8_t; /**< Type alias for the event type. */
        using EventDataType = ComplexDataPackage<EventType, EventPackage, AdditionalParams...>; /**< Type alias for the event data type. */

        /**
         * @brief Sets an event with the given data.
         *
         * @tparam DataType The type of data to set for the event.
         * @param type The type of the event.
         * @param data The data to associate with the event.
         */
        template<typename DataType>
        void setEvent(EventType type, const DataType& data)
        {
            static_assert((std::is_same_v<DataType, EventPackage> || (std::is_same_v<DataType, AdditionalParams> || ...)),
                "DataType is not allowed in EventDataType");
            events[type] = { true, EventDataType{} };
            events[type].data.set(type, data);
        }

        /**
         * @brief Sets an event with the given event data type.
         *
         * @param type The type of the event.
         * @param data The event data to associate with the event.
         */
        void setEvent(EventType type, const EventDataType& data)
        {
            events[type] = { true, data };
        }

        /**
         * @brief Sets an event without data.
         *
         * @param type The type of the event.
         */
        void setEvent(EventType type)
        {
            events[type] = { true, EventDataType{} };
        }

        /**
         * @brief Retrieves the data associated with the specified event type.
         *
         * @tparam DataType The type of data to retrieve.
         * @param type The type of the event.
         * @return The data associated with the event.
         * @throws std::runtime_error if the event data is not found or cannot be retrieved.
         */
        template<typename DataType>
        DataType getEventData(EventType type) const
        {
            static_assert((std::is_same_v<DataType, EventPackage> || (std::is_same_v<DataType, AdditionalParams> || ...)),
                "DataType is not allowed in EventDataType");
            auto it = events.find(type);
            if( it != events.end() && it->second.occurred )
            {
                try
                {
                    return it->second.data.template get<DataType>(type);
                }
                catch( const std::exception& e )
                {
                    throw std::runtime_error("Failed to get event data: " + std::string(e.what()));
                }
            }
            throw std::runtime_error("Event data not found for the given type.");
        }

        /**
         * @brief Checks if the specified event has occurred.
         *
         * @param type The type of the event.
         * @return True if the event has occurred, false otherwise.
         */
        bool isEventOccurred(EventType type) const
        {
            auto it = events.find(type);
            return it != events.end() && it->second.occurred;
        }

        /**
         * @brief Checks if any event has changed.
         *
         * @return True if any event has occurred, false otherwise.
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
         * @brief Retrieves the ID of the specified event as a string.
         *
         * @param type The type of the event.
         * @return The ID of the event as a string, or an empty string if the event has not occurred.
         */
        std::string getEventId(EventType type) const
        {
            auto it = events.find(type);
            if( it != events.end() && it->second.occurred )
            {
                return std::to_string(it->second.data.id()); // Use std::to_string to convert the ID to a string
            }
            return "";
        }

        /**
         * @brief Clears the specified event.
         *
         * @param type The type of the event.
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
         * @brief Struct to hold information about an event.
         */
        struct EventInfo
        {
            bool occurred = false; /**< Flag indicating whether the event has occurred. */
            EventDataType data; /**< The data associated with the event. */
        };

        std::unordered_map<EventType, EventInfo> events; /**< Map to store events and their information. */
    };
}
