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
                 * @brief Draws the top buttons for creating and importing lessons.
                 */
                void drawTopButtons();

                /**
                 * @brief Handles the tree view for marking words.
                 * @param markedWords Set of marked word IDs.
                 */
                void handleTreeView(std::unordered_set<int>& markedWords);

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
                 * @return A vector of parsed lessons.
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
                 * @param renamePopupOpen Boolean reference indicating if the rename popup is open.
                 */
                void ShowRenamePopup(bool& renamePopupOpen);

                /**
                 * @brief Handles moving marked words to a new lesson.
                 * @param createNewLessonPopupOpen Boolean reference indicating if the create new lesson popup should be open.
                 * @param markedWords Set of marked word IDs.
                 */
                void handleWordsMove(bool& createNewLessonPopupOpen, std::unordered_set<int>& markedWords);

                /**
                 * @brief Handles the deletion of a lesson.
                 * @param deleteLesson Boolean reference indicating if a lesson is to be deleted.
                 */
                void handleLessonDelete(bool& deleteLesson);

                /**
                 * @brief Draws the edit lesson widget.
                 * @param open_edit_lesson Boolean reference indicating if the edit lesson widget is open.
                 * @param originalLesson Reference to the original lesson.
                 * @param selectedLesson Reference to the selected lesson.
                 */
                void handleLessonEdit(bool& open_edit_lesson, Lesson& originalLesson, Lesson& selectedLesson);

                /**
                 * @brief Handles the export of lessons.
                 * @param lessonsToExport Set of lesson IDs to be exported.
                 */
                void handleExportLessons(std::unordered_set<int> lessonsToExport);

                /**
                 * @brief Draws the lessons tree.
                 * @param markedWords Set of marked word IDs.
                 * @param lessonsToExport Set of lesson IDs to be exported.
                 * @param open_edit_lesson Boolean reference indicating if the edit lesson widget is open.
                 * @param selectedLesson Reference to the selected lesson.
                 * @param originalLesson Reference to the original lesson.
                 * @param renamePopupOpen Boolean reference indicating if the rename popup is open.
                 * @param deleteLesson Boolean reference indicating if a lesson is to be deleted.
                 * @param createNewLessonPopupOpen Boolean reference indicating if the create new lesson popup should be open.
                 */
                void drawLessonsTree(std::unordered_set<int>& markedWords, std::unordered_set<int>& lessonsToExport, bool& open_edit_lesson, Lesson& selectedLesson, Lesson& originalLesson, bool& renamePopupOpen, bool& deleteLesson, bool& createNewLessonPopupOpen);

                /**
                 * @brief Parses and exports lessons to a file.
                 * @param filePath The path to the file to export lessons to.
                 * @param lessonsToExport Set of lesson IDs to be exported.
                 */
                void parseAndExportLessons(const std::string& filePath, const std::unordered_set<int>& lessonsToExport);

                /**
                 * @brief Handles the deletion of a lesson.
                 */
                void handleDeleteLesson();

                /**
                 * @brief Saves the renamed lesson.
                 */
                void saveRenamedLesson();

                /**
                 * @brief Finds a lesson by its ID.
                 * @param id The ID of the lesson to find.
                 * @return The found lesson, or an empty lesson if not found.
                 */
                Lesson findLessonWithId(int id);

                std::deque<LessonGroup> m_cashedLessons; /**< Deque containing lesson groups. */
                LessonPackageType m_type; /**< The type of package. */
                LessonSettingsWidget m_lessonSettingsWidget; /**< The lesson settings widget. */
                tools::Logger& m_logger; /**< Reference to the logger. */

                std::unordered_set<int> m_selectedLessons; /**< Set storing indices of selected lessons. */
                int m_selectedLessonIndex = -1; /**< Index of the selected lesson. */
                int m_changedLessonGroupIndex = -1; /**< Index of the changed lesson group. */
                int m_changedLessonIndex = -1; /**< Index of the changed lesson. */
                char renameMainNameBuffer[256] = ""; /**< Buffer for renaming the main name. */
                char renameSubNameBuffer[256] = ""; /**< Buffer for renaming the sub-name. */
                char newLessonMainNameBuffer[256] = ""; /**< Buffer for the new lesson's main name. */
                char newLessonSubNameBuffer[256] = ""; /**< Buffer for the new lesson's sub name. */
            };
        }
    }
}
