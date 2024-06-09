/**
 * @file LessonTreeViewWidget.h
 * @brief Defines the LessonTreeViewWidget class, representing a lesson tree view widget.
 */

#pragma once

#include "Widget.h"
#include "lessons/Lesson.h"
#include "LessonSettingsWidget.h"
#include "packages/LessonDataPackage.h"
#include <unordered_set>
#include <deque>
#include <map>

namespace tools { class Logger; }

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
                 * @param logger A reference to a Logger object.
                 */
                LessonTreeViewWidget(tools::Logger& logger);

                /**
                 * @brief Enum for lesson tree view widget events.
                 */
                enum LessonTreeViewWidgetEvent : uint8_t
                {
                    OnLessonCreated, /**< Event triggered when a lesson is created. */
                    OnLessonRename, /**< Event triggered when a lesson is renamed. */
                    OnLessonDelete, /**< Event triggered when a lesson is deleted. */
                    OnLessonEdited, /**< Event triggered when a lesson is edited. */
                    OnPlayMultipleChoiceQuiz, /**< Event triggered to play a multiple choice quiz. */
                    OnPlayVocabularyQuiz /**< Event triggered to play a vocabulary quiz. */
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

                /**
                 * @brief Parses and imports lessons from a file.
                 * @param filePath The path to the file containing lessons.
                 */
                std::vector<Lesson> parseLessons(const std::string& filePath);

                /**
                 * @brief Adds lessons with an overwrite check.
                 * @param newLessons A map containing the new lessons to be added.
                 */
                void addLessonsWithOverwriteCheck(const std::map<std::string, LessonGroup>& newLessons);

                /**
                 * @brief Draws the overwrite popup.
                 */
                void drawOverwritePopup();

                /**
                 * @brief Creates a lesson data package from selected nodes.
                 * @param nodes A set of selected nodes.
                 * @return A LessonDataPackage object.
                 */
                LessonDataPackage createLessonDataPackageFromSelectedNodes(const std::unordered_set<int>& nodes);

                /**
                 * @brief Creates a lesson data package from a list of lessons.
                 * @param lessons A vector of lessons.
                 * @return A LessonDataPackage object.
                 */
                LessonDataPackage createLessonDataPackageFromLessons(const std::vector<Lesson>& lessons);

                /**
                 * @brief Creates a lesson data package from a single lesson.
                 * @param lesson A Lesson object.
                 * @return A LessonDataPackage object.
                 */
                LessonDataPackage createLessonDataPackageFromLesson(const Lesson& lesson);

                /**
                 * @brief Creates a lesson package from a single lesson.
                 * @param lesson A Lesson object.
                 * @return A LessonPackage object.
                 */
                LessonPackage createLessonPackage(const Lesson& lesson);

                /**
                 * @brief Shows the rename popup.
                 */
                void ShowRenamePopup();

                /**
                 * @brief Handles the add new lesson button.
                 * @param open_add_new_lesson Reference to a boolean indicating if the add new lesson widget is open.
                 * @param selectedLesson Reference to the selected lesson.
                 */
                void handleAddNewLessonButton(bool& open_add_new_lesson, Lesson& selectedLesson);

                /**
                 * @brief Draws the add new lesson widget.
                 * @param open_add_new_lesson Reference to a boolean indicating if the add new lesson widget is open.
                 * @param selectedLesson Reference to the selected lesson.
                 */
                void drawAddNewLessonWidget(bool& open_add_new_lesson, Lesson& selectedLesson);

                /**
                 * @brief Draws the lessons tree.
                 * @param clickedOutside Indicates if a click was made outside the widget.
                 * @param open_edit_lesson Reference to a boolean indicating if the edit lesson widget is open.
                 * @param selectedLesson Reference to the selected lesson.
                 * @param originalLesson Reference to the original lesson.
                 * @param renamePopupOpen Reference to a boolean indicating if the rename popup is open.
                 * @param deleteLesson Reference to a boolean indicating if a lesson is to be deleted.
                 * @param ctrlPressed Indicates if the control key is pressed.
                 */
                void drawLessonsTree(bool clickedOutside, bool& open_edit_lesson, Lesson& selectedLesson, Lesson& originalLesson, bool& renamePopupOpen, bool& deleteLesson, bool ctrlPressed);

                /**
                 * @brief Draws the edit lesson widget.
                 * @param open_edit_lesson Reference to a boolean indicating if the edit lesson widget is open.
                 * @param selectedLesson Reference to the selected lesson.
                 * @param originalLesson Reference to the original lesson.
                 */
                void drawEditLessonWidget(bool& open_edit_lesson, Lesson& selectedLesson, Lesson& originalLesson);


                void parseAndExportLessons(const std::string& filePath, const std::unordered_set<int>& lessonsToExport);
                /**
                 * @brief Handles the delete lesson action.
                 */
                void handleDeleteLesson();

                /**
                 * @brief Saves the renamed lesson.
                 */
                void saveRenamedLesson();
                Lesson findLessonWithId(int id);

                std::deque<LessonGroup> m_cashedLessons; /**< Deque containing lesson groups. */
                LessonPackageType m_type; /**< The type of package. */
                LessonSettingsWidget m_lessonSettingsWidget; /**< The lesson settings widget. */
                tools::Logger& m_logger;

                std::unordered_set<int> m_selectedLessons; /**< Set storing indices of selected lessons. */
                int m_selectedLessonIndex = -1; /**< Index of the selected lesson. */
                int m_changedLessonGroupIndex = -1; /**< Index of the changed lesson group. */
                int m_changedLessonIndex = -1; /**< Index of the changed lesson. */
                char renameMainNameBuffer[256] = ""; /**< Buffer for renaming the main name. */
                char renameSubNameBuffer[256] = ""; /**< Buffer for renaming the sub-name. */
                char newLessonMainNameBuffer[256] = ""; // Buffer for the new lesson's main name
                char newLessonSubNameBuffer[256] = ""; // Buffer for the new lesson's sub name
            };
        }
    }
}
