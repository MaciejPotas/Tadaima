/**
 * @file LessonSettingsWidget.h
 * @brief Defines the LessonSettingsWidget class, providing functionality to display and edit settings for a lesson.
 */

#pragma once

#include "Widget.h"
#include "ConjugationSettingsWidget.h"
#include "Lessons/Lesson.h"
#include "dictionary/Conjugations.h"
#include "dictionary/Dictionary.h"

namespace tools { class Logger; }

namespace tadaima
{
    struct Lesson;

    namespace gui
    {
        namespace widget
        {
            /**
             * @class LessonSettingsWidget
             * @brief Provides functionality to display and edit settings for a lesson.
             *
             * The LessonSettingsWidget class is responsible for rendering the lesson settings GUI widget,
             * allowing the user to create, edit, and manage lesson settings. It includes functionality for
             * handling word fields, conjugations, and associated metadata.
             */
            class LessonSettingsWidget : public Widget
            {
            public:

                /**
                 * @brief Constructor for the LessonSettingsWidget.
                 *
                 * Initializes the widget with a reference to the logger.
                 *
                 * @param logger Reference to the logger for tracking operations.
                 */
                LessonSettingsWidget(tools::Logger& logger);

                /**
                 * @brief Draws the lesson settings widget.
                 *
                 * This function renders the widget in the GUI, providing the user interface
                 * to edit lesson settings.
                 *
                 * @param p_open Pointer to a boolean indicating whether the widget is open.
                 */
                void draw(bool* p_open) override;

                /**
                 * @brief Sets the lesson for the settings widget.
                 *
                 * This function assigns the provided lesson to the widget for editing.
                 *
                 * @param lesson The lesson to be set.
                 */
                void setLesson(Lesson& lesson);

                /**
                 * @brief Initializes the widget with data from a data package.
                 *
                 * This function uses a data package to populate the widget's fields.
                 *
                 * @param r_package The data package used for initialization.
                 */
                void initialize(const tools::DataPackage& r_package) override;

                /**
                 * @brief Clears the widget's fields and resets its state.
                 *
                 * This function clears all input buffers and resets the widget's internal flags.
                 */
                void clear();

            private:

                /**
                 * @brief Creates a new word using the current input fields.
                 *
                 * This utility function constructs a new word object from the input buffers.
                 */
                void createAWordFromFields();

                bool m_conjugationJustSaved = false;
                bool m_isEditing = false; ///< Flag indicating whether the widget is in edit mode.
                char m_mainNameBuffer[50] = ""; ///< Buffer for the lesson main name.
                char m_subNameBuffer[50] = ""; ///< Buffer for the lesson sub name.
                char m_groupNameBuffer[50] = ""; ///< Buffer for the lesson group name.
                char m_kanaBuffer[50] = ""; ///< Buffer for word kana.
                char m_kanjiBuffer[50] = ""; ///< Buffer for Kanji.
                char m_translationBuffer[50] = ""; ///< Buffer for word translation.
                char m_romajiBuffer[50] = ""; ///< Buffer for word romaji.
                char m_exampleSentenceBuffer[300] = ""; ///< Buffer for example sentence.
                char m_tagBuffer[100] = ""; ///< Buffer for word tags.

                Word m_newWord; ///< Temporary storage for a newly created word.
                int m_selectedWordIndex = -1; ///< Index of the selected word in the list.
                Lesson m_newLesson; ///< New lesson to be added or edited.
                Dictionary m_dictionary; ///< Dictionary object for managing words.
                tools::Logger& m_logger; ///< Reference to the logger for operation tracking.
                Lesson* m_lesson; ///< Pointer to the current lesson being edited.
                ConjugationSettingsWidget m_ConjugationSettingsWidget; ///< Widget for conjugation settings.
            };
        }
    }
}
