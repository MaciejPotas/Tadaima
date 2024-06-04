/**
 * @file LessonSettingsWidget.h
 * @brief Defines the LessonSettingsWidget class, providing functionality to display and edit settings for a lesson.
 */

#pragma once

#include "Widget.h"
#include "Lessons/Lesson.h"

namespace tadaima
{
    struct Lesson;

    namespace gui
    {
        namespace widget
        {
            /**
             * @brief The LessonSettingsWidget class provides functionality to display and edit
             * settings for a lesson.
             */
            class LessonSettingsWidget : public Widget
            {
            public:
                /**
                 * @brief Draws the lesson settings widget.
                 * @param p_open Pointer to a boolean indicating whether the widget is open.
                 */
                void draw(bool* p_open) override;

                /**
                 * @brief Sets the lesson for the settings widget.
                 * @param lesson The lesson to be set.
                 */
                void setLesson(Lesson& lesson);

            private:
                bool m_isEditing = false; ///< Flag indicating whether the widget is in edit mode.
                char m_mainNameBuffer[50] = ""; ///< Buffer for the lesson main name.
                char m_subNameBuffer[50] = "";  ///< Buffer for the lesson sub name.
                char m_kanaBuffer[50] = ""; ///< Buffer for word kana.
                char m_translationBuffer[50] = ""; ///< Buffer for word translation.
                char m_romajiBuffer[50] = ""; ///< Buffer for word romaji.
                char m_exampleSentenceBuffer[100] = ""; ///< Buffer for example sentence.
                char m_tagBuffer[100] = ""; ///< Buffer for word tags.
                int m_selectedWordIndex = -1; ///< Index of the selected word in the list.
                Lesson m_newLesson; ///< New lesson to be added or edited.

                /**
                 * @brief Pointer to the lesson object.
                 */
                Lesson* m_lesson;
            };
        }
    }
}
