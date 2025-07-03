#pragma once
#include <vector>
#include "lessons/Lesson.h"
#include "Tools/Logger.h"

namespace tadaima::gui::widget
{
    struct LessonFileIO
    {
        // Import lessons from XML file
        static std::vector<Lesson> importLessons(const std::string& filePath, tools::Logger& logger);

        // Export lessons to XML file
        static void exportLessons(
            const std::string& filePath,
            const std::vector<Lesson>& lessons,
            tools::Logger& logger
        );
    };
}
