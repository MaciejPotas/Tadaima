/**
 * @file LessonTreeViewWidget.h
 * @brief Defines the LessonTreeViewWidget class, representing a lesson tree view widget.
 */
#pragma once

#include "Widget.h"
#include "lessons/Lesson.h"
#include "LessonSettingsWidget.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @brief A widget representing a lesson tree view.
             */
            class LessonTreeViewWidget : public Widget
            {
            public:
                /**
                 * @brief Enum for package keys.
                 */
                enum class PackageKey : uint32_t
                {
                    LessonsPackage
                };

                /**
                 * @brief Enum for lesson data keys.
                 */
                enum class LessonDataKey : uint32_t
                {
                    MainName,
                    SubName,
                    Words
                };

                /**
                 * @brief Enum for word data keys.
                 */
                enum class WordDataKey : uint32_t
                {
                    Kana,
                    Translation,
                    Romaji,
                    ExampleSentence,
                    Tags
                };

                /**
                 * @brief Alias for word data package.
                 */
                using WordDataPackage = tools::ComplexDataPackage<WordDataKey, std::string, std::vector<std::string>>;

                /**
                 * @brief Alias for lesson package.
                 */
                using LessonPackage = tools::ComplexDataPackage<LessonDataKey, std::string, std::vector<WordDataPackage>>;

                /**
                 * @brief Represents a package containing lesson data.
                 */
                class LessonDataPackage : public tools::ComplexDataPackage<PackageKey, std::vector<LessonPackage>>
                {
                public:
                    /**
                     * @brief Constructs a LessonDataPackage object with the given identifier.
                     * @param id The identifier of the package.
                     */
                    LessonDataPackage(tools::DataPackage::Identifier id) : ComplexDataPackage(id) {}
                };

                /**
                 * @brief Initializes the lesson tree view widget.
                 * @param r_package The data package for initialization.
                 */
                void initialize(const tools::DataPackage& r_package) override;

                /**
                 * @brief Draws the lesson tree view widget.
                 * @param p_open Pointer to a boolean indicating whether the widget is open.
                 */
                void draw(bool* p_open) override;

            private:
                /**
                 * @brief Represents a group of lessons.
                 */
                struct LessonGroup
                {
                    std::string mainName; /**< The main name of the lesson group. */
                    std::vector<Lesson> subLessons; /**< The sub-lessons within the group. */
                };

                std::vector<LessonGroup> lessons; /**< Vector containing lesson groups. */

                /**
                 * @brief Adds a new lesson.
                 */
                void AddNewLesson();

                LessonSettingsWidget m_lessonSettingsWidget; /**< The lesson settings widget. */
            };
        }
    }
}
