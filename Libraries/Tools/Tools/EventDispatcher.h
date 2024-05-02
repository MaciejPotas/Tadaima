// ----------------------------------------------------------------------------
// File:        EventDispatcher.h
// Description: Header file for the EventDispatcher and EventListener classes.
//              Provides a mechanism for event handling and dispatching.
// ----------------------------------------------------------------------------
#pragma  once

#include <vector>
#include <functional>

namespace tools
{
    template<typename Entity, typename EventData>
    class EventDispatcher
    {
    public:

        using EventHandler = std::function<void(EventData data)>;

        void addListener(const Entity& entity, const EventHandler& handler)
        {
            listeners[entity].push_back(handler);
        }

        void emit(const Entity& entity, const EventData data)
        {
            auto entityListeners = listeners.find(entity);
            if( entityListeners != listeners.end() )
            {
                for( auto& eventHandler : entityListeners->second )
                {
                    eventHandler(data);
                }
            }
        }

    private:
        std::unordered_map<Entity, std::vector<EventHandler>> listeners;
    };
}
