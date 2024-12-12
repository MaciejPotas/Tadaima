#pragma once

#include <cstdint>

namespace tadaima
{
    namespace application
    {
        enum ApplicationEvent : uint8_t
        {
            OnLessonCreated,
            OnLessonUpdate,
            OnLessonDelete,
            OnLessonEdited,
            OnSettingsChanged
        };
    }
}