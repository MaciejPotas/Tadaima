#pragma once

#include "Application/Application.h"
#include "Application/ApplicationEventList.h"
#include "Lessons/Lesson.h"
#include "gmock/gmock.h"

using namespace tadaima;
using namespace tadaima::application;
using namespace ::testing;

class MockApplication : public Application
{
public:
   // MOCK_METHOD(void, setEvent, (ApplicationEvent event, const std::any& data), (override));
};