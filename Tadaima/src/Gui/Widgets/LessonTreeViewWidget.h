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
                 * @brief Constructs a LessonTreeViewWidget object.
                 */
                LessonTreeViewWidget() : Widget(Type::LessonTreeView) {}

                /**
                 * @brief Enum for lesson tree view widget events.
                 */
                enum LessonTreeViewWidgetEvent : uint8_t
                {
                    OnLessonCreated,
                    OnLessonRename
                };

                /**
                 * @brief Enum for package types.
                 */
                enum class PackageType : uint32_t
                {
                    LessonCreated,
                    LessonModified,
                    LessonDeleted
                };

                /**
                 * @brief Enum for package keys.
                 */
                enum class PackageKey : uint32_t
                {
                    Type,
                    LessonsPackage
                };

                /**
                 * @brief Enum for lesson data keys.
                 */
                enum class LessonDataKey : uint32_t
                {
                    id,
                    MainName,
                    SubName,
                    Words
                };

                /**
                 * @brief Enum for word data keys.
                 */
                enum class WordDataKey : uint32_t
                {
                    id,
                    Kana,
                    Translation,
                    Romaji,
                    ExampleSentence,
                    Tags
                };

                /**
                 * @brief Alias for word data package.
                 */
                using WordDataPackage = tools::ComplexDataPackage<WordDataKey, int, std::string, std::vector<std::string>>;

                /**
                 * @brief Alias for lesson package.
                 */
                using LessonPackage = tools::ComplexDataPackage<LessonDataKey, int, std::string, std::vector<WordDataPackage>>;

                /**
                 * @brief Represents a package containing lesson data.
                 */
                class LessonDataPackage : public tools::ComplexDataPackage<PackageKey, PackageType, std::vector<LessonPackage>>
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

                void ShowRenamePopup();


                /**
                 * @brief Represents a group of lessons.
                 */
                struct LessonGroup
                {
                    std::string mainName; /**< The main name of the lesson group. */
                    std::vector<Lesson> subLessons; /**< The sub-lessons within the group. */
                };

                std::vector<LessonGroup> m_cashedLessons; /**< Vector containing lesson groups. */
                PackageType m_type; /**< The type of package. */
                LessonSettingsWidget m_lessonSettingsWidget; /**< The lesson settings widget. */

                int renameLessonGroupIndex = -1;
                int renameLessonIndex = -1;
                char renameMainNameBuffer[256] = "";
                char renameSubNameBuffer[256] = "";
            };
        }
    }
}
