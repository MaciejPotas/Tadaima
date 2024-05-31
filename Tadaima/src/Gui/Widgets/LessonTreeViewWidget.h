/**
 * @file LessonTreeViewWidget.h
 * @brief Defines the LessonTreeViewWidget class, representing a lesson tree view widget.
 */

#pragma once

#include "Widget.h"
#include "lessons/Lesson.h"
#include "LessonSettingsWidget.h"
#include "packages/LessonDataPackage.h"

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
                    OnLessonRename,
                    OnLessonDelete,
                    OnLessonEdited,
                    OnPlayMultipleChoiceQuiz,
                    OnPlayVocabularyQuiz
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
                LessonPackageType m_type; /**< The type of package. */
                LessonSettingsWidget m_lessonSettingsWidget; /**< The lesson settings widget. */

                int m_selectedLessonIndex = -1;
                int m_changedLessonGroupIndex = -1;
                int m_changedLessonIndex = -1;
                char renameMainNameBuffer[256] = "";
                char renameSubNameBuffer[256] = "";
            };
        }
    }
}
