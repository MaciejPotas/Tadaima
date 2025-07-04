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
                 * @struct LessonActionState
                 * @brief Represents the current pending action for a lesson.
                 */
                struct LessonActionState
                {
                    /**
                     * @enum Type
                     * @brief The type of pending action.
                     */
                    enum class Type
                    {
                        None, Edit, Rename, Delete, DeleteGroup, Export, MoveWordsToLesson
                    } type = Type::None;
                    int lessonId = -1;        /**< The lesson ID related to the action. */
                    Lesson original;           /**< Original lesson before changes. */
                    Lesson editable;           /**< Editable copy for modification. */
                };
                LessonActionState m_pendingAction;

                /**
                 * @enum LessonTreeViewWidgetEvent
                 * @brief Enum for lesson tree view widget events.
                 */
                enum LessonTreeViewWidgetEvent : uint8_t
                {
                    OnLessonCreated,             /**< Triggered when a lesson is created. */
                    OnLessonRename,              /**< Triggered when a lesson is renamed. */
                    OnLessonDelete,              /**< Triggered when a lesson is deleted. */
                    OnLessonEdited,              /**< Triggered when a lesson is edited. */
                    OnPlayMultipleChoiceQuiz,    /**< Triggered for multiple-choice quiz. */
                    OnPlayVocabularyQuiz,        /**< Triggered for vocabulary quiz. */
                    OnConjuactionQuiz,           /**< Triggered for conjugation quiz. */
                    OnQuizSelect                 /**< Triggered when a quiz is selected. */
                };

                /**
                 * @brief Constructor.
                 * @param logger Reference to a Logger instance.
                 */
                LessonTreeViewWidget(tools::Logger& logger);

                /**
                 * @brief Initializes the widget with a data package.
                 * @param r_package The data package to initialize with.
                 */
                void initialize(const tools::DataPackage& r_package) override;

                /**
                 * @brief Draws the main widget window.
                 * @param p_open Pointer to a boolean indicating whether the window is open.
                 */
                void draw(bool* p_open) override;

                /**
                 * @brief Draws the top row of buttons (Create, Import, etc.).
                 */
                void drawTopButtons();

                /**
                 * @brief Draws the lesson tree hierarchy.
                 */
                void drawLessonsTree();

                /**
                 * @brief Draws a single lesson row and handles its selection.
                 * @param lesson The lesson to draw.
                 * @param lessonsInSubgroup Reference to the vector of lessons in the current subgroup.
                 * @param lessonIdx Index of this lesson within lessonsInSubgroup.
                 */
                void drawLessonRow(const Lesson& lesson, const std::vector<Lesson>& lessonsInSubgroup, int lessonIdx);

                /**
                 * @brief Draws a word row for a given lesson.
                 * @param word The word to draw.
                 * @param lesson The lesson containing the word.
                 */
                void drawWordRow(const Word& word, const Lesson& lesson);

                /**
                 * @brief Shows the context menu for a lesson.
                 * @param lesson The lesson for which to show the menu.
                 */
                void showLessonContextMenu(const Lesson& lesson);

                /**
                 * @brief Shows the context menu for selected words in a lesson.
                 * @param lesson The lesson containing the selected words.
                 */
                void showSelectedWordsContextMenu(const Lesson& lesson);

                /**
                 * @brief Shows the rename lesson popup dialog.
                 */
                void ShowRenamePopup();

                /**
                 * @brief Shows the delete lesson popup dialog.
                 */
                void ShowDeletePopup();

                /**
                 * @brief Shows the move words to lesson popup dialog.
                 */
                void showMoveWordsToLessonPopup();

                /**
                 * @brief Sets all words in the lesson as selected or unselected.
                 * @param lesson The lesson whose words to update.
                 * @param select True to select, false to unselect.
                 */
                void setLessonWordsSelection(const Lesson& lesson, bool select);

                /**
                 * @brief Selects or unselects a range of lessons in the current subgroup.
                 * @param lessonsInSubgroup The vector of lessons.
                 * @param fromIdx Index to start selection (inclusive).
                 * @param toIdx Index to end selection (inclusive).
                 * @param select True to select, false to unselect.
                 */
                void setLessonRangeSelection(const std::vector<Lesson>& lessonsInSubgroup, int fromIdx, int toIdx, bool select);

                /**
                 * @brief Handles editing a lesson.
                 */
                void handleLessonEdit();

                /**
                 * @brief Handles exporting all currently marked lessons.
                 */
                void handleExportLessons();

                /**
                 * @brief Creates a new lesson from a set of word IDs.
                 * @param wordIds The set of word IDs to copy.
                 * @return The newly created Lesson.
                 */
                Lesson copyWordsToNewLesson(const std::unordered_set<int>& wordIds);

                /**
                 * @brief Packages lessons for data transfer/export from a set of selected node IDs.
                 * @param nodes Set of lesson IDs.
                 * @return A LessonDataPackage containing the selected lessons.
                 */
                LessonDataPackage createLessonDataPackageFromSelectedNodes(const std::unordered_set<int>& nodes);

                /**
                 * @brief Packages a list of lessons into a LessonDataPackage.
                 * @param lessons Vector of lessons.
                 * @return A LessonDataPackage containing the given lessons.
                 */
                LessonDataPackage createLessonDataPackageFromLessons(const std::vector<Lesson>& lessons);

                /**
                 * @brief Packages a single lesson into a LessonDataPackage.
                 * @param lesson The lesson to package.
                 * @return A LessonDataPackage containing the given lesson.
                 */
                LessonDataPackage createLessonDataPackageFromLesson(const Lesson& lesson);

                /**
                 * @brief Finds a lesson by ID.
                 * @param id The lesson ID to find.
                 * @return The lesson with the given ID, or an empty lesson if not found.
                 */
                Lesson findLessonWithId(int id) const;

                /**
                 * @struct LessonGroup
                 * @brief Represents a group of lessons organized by group name.
                 */
                struct LessonGroup
                {
                    std::string groupName;  /**< Name of the lesson group. */
                    std::map<std::string, std::vector<Lesson>> subLessons; /**< Map: mainName -> list of lessons. */
                };

                std::deque<LessonGroup> m_cashedLessons;     /**< Cached groups of lessons. */
                LessonSettingsWidget m_lessonSettingsWidget; /**< Widget for lesson editing. */
                tools::Logger& m_logger;                     /**< Logger reference. */

                int m_lastSelectedWordId = -1;               /**< Last selected word ID (for range selection). */
                int m_lastSelectedLessonId = -1;             /**< Last selected lesson ID (for range selection). */
                int m_changedLessonGroupIndex = -1;          /**< Index of changed lesson group. */
                int m_changedLessonSubGroupIndex = -1;       /**< Index of changed lesson subgroup. */
                int m_changedLessonIndex = -1;               /**< Index of changed lesson. */

                std::unordered_set<int> m_lessonsToExport;   /**< Set of lessons marked for export. */
                std::unordered_set<int> m_selectedWords;     /**< Currently selected word IDs. */
                std::unordered_set<int> m_selectedLessons;   /**< Currently selected lesson IDs. */

                char m_GroupNameBuf[128] = {};                 /**< Buffer for editing group name. */
                char m_MainNameBuf[128] = {};                  /**< Buffer for editing main name. */
                char m_SubNameBuf[128] = {};                   /**< Buffer for editing sub name. */
            };
        }
    }
}
