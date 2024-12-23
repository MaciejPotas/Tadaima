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
             * @class LessonTreeViewWidget
             * @brief A widget representing a tree view for managing lessons and their associated words.
             */
            class LessonTreeViewWidget : public Widget
            {
            public:
                /**
                 * @brief Constructs a LessonTreeViewWidget object.
                 *
                 * Initializes the widget and sets up internal data structures.
                 *
                 * @param logger A reference to a Logger object.
                 */
                LessonTreeViewWidget(tools::Logger& logger);

                /**
                 * @enum LessonTreeViewWidgetEvent
                 * @brief Enum for lesson tree view widget events.
                 */
                enum LessonTreeViewWidgetEvent : uint8_t
                {
                    OnLessonCreated, /**< Event triggered when a lesson is created. */
                    OnLessonRename, /**< Event triggered when a lesson is renamed. */
                    OnLessonDelete, /**< Event triggered when a lesson is deleted. */
                    OnLessonEdited, /**< Event triggered when a lesson is edited. */
                    OnPlayMultipleChoiceQuiz, /**< Event triggered to play a multiple-choice quiz. */
                    OnPlayVocabularyQuiz, /**< Event triggered to play a vocabulary quiz. */
                    OnConjuactionQuiz,
                    OnQuizSelect /**< Event triggered when a quiz is selected. */
                };

                /**
                 * @brief Initializes the lesson tree view widget with data.
                 *
                 * Populates the tree view with lessons and associated data from the provided package.
                 *
                 * @param r_package The data package for initialization.
                 */
                void initialize(const tools::DataPackage& r_package) override;

                /**
                 * @brief Draws the top buttons for creating, importing, and managing lessons.
                 */
                void drawTopButtons();

                /**
                 * @brief Handles the tree view for marking words.
                 *
                 * Updates the internal state based on user interactions for marking words.
                 *
                 * @param markedWords A set of marked word IDs.
                 */
                void handleTreeView(std::unordered_set<int>& markedWords);

                /**
                 * @brief Draws the lesson tree view widget.
                 *
                 * Renders the entire tree view widget on the GUI.
                 *
                 * @param p_open Pointer to a boolean indicating whether the widget is open.
                 */
                void draw(bool* p_open) override;

            private:
                /**
                 * @struct LessonGroup
                 * @brief Represents a group of lessons organized by a common group name.
                 */
                struct LessonGroup
                {
                    std::string groupName; /**< The name of the lesson group. */
                    std::map<std::string, std::vector<Lesson>> subLessons; /**< The sub-lessons within the group. */
                };

                /**
                 * @brief Parses lessons from a file for import.
                 *
                 * Reads lessons from the specified file and prepares them for addition to the tree view.
                 *
                 * @param filePath The path to the file containing lessons.
                 * @return A vector of parsed lessons.
                 */
                std::vector<Lesson> parseLessons(const std::string& filePath);

                /**
                 * @brief Adds lessons to the tree view with an overwrite check.
                 *
                 * Ensures that existing lessons are not overwritten without user confirmation.
                 *
                 * @param newLessons A map containing the new lessons to be added.
                 */
                void addLessonsWithOverwriteCheck(const std::map<std::string, LessonGroup>& newLessons);

                /**
                 * @brief Draws the overwrite popup for handling conflicting lesson imports.
                 */
                void drawOverwritePopup();

                /**
                 * @brief Creates a LessonDataPackage from selected nodes.
                 *
                 * Collects lessons matching the selected node IDs and packages them into a data object.
                 *
                 * @param nodes A set of selected node IDs.
                 * @return A LessonDataPackage object.
                 */
                LessonDataPackage createLessonDataPackageFromSelectedNodes(const std::unordered_set<int>& nodes);

                /**
                 * @brief Creates a LessonDataPackage from a list of lessons.
                 *
                 * Bundles the provided lessons into a data package for export or other operations.
                 *
                 * @param lessons A vector of lessons.
                 * @return A LessonDataPackage object.
                 */
                LessonDataPackage createLessonDataPackageFromLessons(const std::vector<Lesson>& lessons);

                /**
                 * @brief Creates a LessonDataPackage from a single lesson.
                 *
                 * Converts a single lesson into a data package for further use.
                 *
                 * @param lesson A Lesson object.
                 * @return A LessonDataPackage object.
                 */
                LessonDataPackage createLessonDataPackageFromLesson(const Lesson& lesson);

                /**
                 * @brief Creates a LessonPackage from a single lesson.
                 *
                 * Converts a lesson into a lightweight package structure.
                 *
                 * @param lesson A Lesson object.
                 * @return A LessonPackage object.
                 */
                LessonPackage createLessonPackage(const Lesson& lesson);

                /**
                 * @brief Shows the rename popup for editing lesson names.
                 *
                 * Displays a modal window for renaming a lesson or its components.
                 *
                 * @param renamePopupOpen Reference to a boolean indicating if the rename popup is open.
                 */
                void ShowRenamePopup(bool& renamePopupOpen);

                /**
                 * @brief Handles moving marked words to a new lesson.
                 *
                 * Transfers words from the current lesson to a newly created lesson.
                 *
                 * @param createNewLessonPopupOpen Reference to a boolean indicating if the create new lesson popup is open.
                 * @param markedWords A set of marked word IDs.
                 */
                void handleWordsMove(bool& createNewLessonPopupOpen, std::unordered_set<int>& markedWords);

                /**
                 * @brief Handles the deletion of a lesson or lessons.
                 *
                 * Removes lessons or marked items based on user selection.
                 *
                 * @param deleteLesson Reference to a boolean indicating if a lesson is to be deleted.
                 */
                void handleLessonDelete(bool& deleteLesson);

                /**
                 * @brief Handles editing a lesson.
                 *
                 * Allows modifications to the selected lesson and its components.
                 *
                 * @param open_edit_lesson Reference to a boolean indicating if the edit lesson widget is open.
                 * @param originalLesson Reference to the original lesson.
                 * @param selectedLesson Reference to the selected lesson.
                 */
                void handleLessonEdit(bool& open_edit_lesson, Lesson& originalLesson, Lesson& selectedLesson);

                /**
                 * @brief Handles exporting selected lessons to a file.
                 *
                 * Saves lessons in a structured format to the specified file.
                 *
                 * @param lessonsToExport A set of lesson IDs to be exported.
                 */
                void handleExportLessons(std::unordered_set<int> lessonsToExport);

                /**
                 * @brief Copies words to a new lesson.
                 *
                 * Creates a new lesson containing the selected words from existing lessons.
                 *
                 * @param wordIds A set of word IDs to copy.
                 * @return The new Lesson object.
                 */
                Lesson copyWordsToNewLesson(const std::unordered_set<int>& wordIds);

                /**
                 * @brief Draws the hierarchical tree structure of lessons and words.
                 *
                 * Renders the tree view and handles user interactions within the widget.
                 *
                 * @param markedWords A set of marked word IDs.
                 * @param lessonsToExport A set of lesson IDs to be exported.
                 * @param open_edit_lesson Reference to a boolean indicating if the edit lesson widget is open.
                 * @param selectedLesson Reference to the selected lesson.
                 * @param originalLesson Reference to the original lesson.
                 * @param renamePopupOpen Reference to a boolean indicating if the rename popup is open.
                 * @param deleteLesson Reference to a boolean indicating if a lesson is to be deleted.
                 * @param createNewLessonPopupOpen Reference to a boolean indicating if the create new lesson popup should be open.
                 */
                void drawLessonsTree(std::unordered_set<int>& markedWords, std::unordered_set<int>& lessonsToExport, bool& open_edit_lesson, Lesson& selectedLesson, Lesson& originalLesson, bool& renamePopupOpen, bool& deleteLesson, bool& createNewLessonPopupOpen);

                /**
                 * @brief Parses lessons and exports them to a file.
                 *
                 * Serializes lessons into a file format for external use.
                 *
                 * @param filePath The path to the file to export lessons to.
                 * @param lessonsToExport A set of lesson IDs to be exported.
                 */
                void parseAndExportLessons(const std::string& filePath, const std::unordered_set<int>& lessonsToExport);

                /**
                 * @brief Finds a lesson by its ID.
                 *
                 * Searches the cached lessons for a match by ID.
                 *
                 * @param id The ID of the lesson to find.
                 * @return The found lesson, or an empty lesson if not found.
                 */
                Lesson findLessonWithId(int id);

                std::deque<LessonGroup> m_cashedLessons; /**< Deque containing grouped lesson data. */
                LessonPackageType m_type; /**< The type of package being handled. */
                LessonSettingsWidget m_lessonSettingsWidget; /**< The widget for managing lesson settings. */
                tools::Logger& m_logger; /**< Reference to the logger for debug and info messages. */

                std::unordered_set<int> m_selectedLessons; /**< Set storing IDs of selected lessons. */
                int m_selectedLessonIndex = -1; /**< Index of the currently selected lesson. */
                int m_changedLessonGroupIndex = -1; /**< Index of the changed lesson group. */
                int m_changedLessonSubGroupIndex = -1; /**< Index of the changed lesson subgroup. */
                int m_changedLessonIndex = -1; /**< Index of the changed lesson. */
                char renameGroupNameBuffer[100] = ""; /**< Buffer for renaming a lesson group. */
                char renameMainNameBuffer[100] = ""; /**< Buffer for renaming a lesson's main name. */
                char renameSubNameBuffer[100] = ""; /**< Buffer for renaming a lesson's sub name. */
                char newLessonGroupNameBuffer[100] = ""; /**< Buffer for creating a new lesson group. */
                char newLessonMainNameBuffer[100] = ""; /**< Buffer for creating a new lesson's main name. */
                char newLessonSubNameBuffer[100] = ""; /**< Buffer for creating a new lesson's sub name. */
            };
        }
    }
}
