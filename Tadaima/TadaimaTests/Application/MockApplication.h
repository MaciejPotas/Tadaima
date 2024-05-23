#pragma once

#include "application/ApplicationEventList.h"
#include "Lessons/Lesson.h"
#include "gmock/gmock.h"

namespace tadaima
{
    namespace application
    {
        class Application
        {
        public:
            MOCK_METHOD(void, setEvent, (ApplicationEvent, const std::vector<Lesson>&), ());
        };
    }
}
