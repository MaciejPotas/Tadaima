// ----------------------------------------------------------------------------
// File:        widget.h
// Description: Header file for the classes related to the widgets and widget
//              events.
// ----------------------------------------------------------------------------

#pragma once

#include "Tools/DataPackage.h"
#include "WidgetTypes.h"
#include <string>
#include <functional>

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            class Widget;

            /**
             * @brief Represents a widget event.
             *
             * This class represents a widget event.
             */
            class WidgetEvent
            {
            public:
                using EventType = uint8_t;

                /**
                 * @brief Constructor.
                 *
                 * Initializes the widget event with the given emitter, event type, and event data.
                 *
                 * @param emitter The emitter of the event.
                 * @param eventType The type of the event.
                 * @param eventData The event data.
                 */
                WidgetEvent(const Widget& emitter, const EventType& eventType, tools::DataPackage* eventData)
                    : m_emitter(emitter), m_eventType(eventType), m_eventData(eventData)
                {
                }

                /**
                 * @brief Get the emitter of the event.
                 *
                 * @return The object emitting the event.
                 */
                const Widget& getWidget() const
                {
                    return m_emitter;
                }

                /**
                 * @brief Get the type of the event.
                 *
                 * @return The type of the event.
                 */
                EventType getEventType() const
                {
                    return m_eventType;
                }

                /**
                 * @brief Get the event data.
                 *
                 * @return The data associated with the event.
                 */
                tools::DataPackage* getEventData() const
                {
                    return m_eventData;
                }

            private:
                const Widget& m_emitter; ///< The object emitting the event.
                EventType m_eventType; ///< The type of the event.
                tools::DataPackage* m_eventData = nullptr; ///< The data associated with the event.
            };

            /**
             * @brief Base class for widgets.
             *
             * This class represents a base class for widgets.
             */
            class Widget
            {
            public:
                using Listener = std::function<void(const WidgetEvent& data)>;

                virtual ~Widget() = default;

                /**
                 * @brief Initializes the widget with data.
                 *
                 * This method initializes the widget with the provided data package.
                 *
                 * @param r_package The data package to initialize the widget with.
                 */
                virtual void initialize([[maybe_unused]] const tools::DataPackage& r_package) {}

                /**
                 * @brief Draws the widget.
                 *
                 * This method draws the widget.
                 */
                virtual void draw(bool* p_open = nullptr) = 0;

                /**
                 * @brief Sets the observer for the widget.
                 *
                 * @param observer The observer function to set.
                 */
                void setObserver(Listener observer)
                {
                    m_observer = observer;
                }

                /**
                 * @brief Gets the ID of the widget.
                 *
                 * @return The ID of the widget.
                 */
                std::string getId() const
                {
                    return m_id;
                }

                /**
                 * @brief Gets the type of the widget.
                 *
                 * @return The type of the widget.
                 */
                Type getType() const
                {
                    return m_type;
                }

            protected:

                /**
                 * @brief Emits an event.
                 *
                 * This method emits an event with the provided data.
                 *
                 * @param data The event data to emit.
                 */
                void emitEvent(const WidgetEvent& data)
                {
                    m_observer(data);
                }

            private:
                std::string m_id; ///< The ID of the widget.
                Type m_type; ///< The type of the widget.
                Listener m_observer; ///< The observer function for the widget.
            };
        }
    }
}