/**
 * @file ApplicationSettingsWidget.h
 * @brief Defines the ApplicationSettingsWidget class for managing application settings in the GUI.
 */

#pragma once

#include "PackageType.h"
#include "Tools/DataPackage.h"
#include <cstring>
#include <string>
#include "lessons/Lesson.h"
#include "dictionary/Conjugations.h"

namespace tools { class Logger; }

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @brief Represents a package containing lesson data.
             */
            class LessonDataPackage : public tools::DataPackage
            {
            public:

                LessonDataPackage(const std::vector<Lesson>& lessons) : DataPackage(PackageType::Lessons), m_lessons(lessons)
                {

                }

                LessonDataPackage(const Lesson& lesson) : DataPackage(PackageType::Lessons)
                {
                    m_lessons.push_back(lesson);
                }

                std::vector<Lesson> m_lessons;
            };
        }
    }
}
