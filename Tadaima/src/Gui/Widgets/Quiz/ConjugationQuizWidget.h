#pragma once

#include "../Widget.h"
#include "tools/Logger.h"
#include "Lessons/Lesson.h"
#include <vector>
#include <string>
#include <memory>
#include "imgui.h"

namespace tadaima
{
    namespace quiz { class Quiz; }
    namespace gui
    {
        namespace widget
        {
            /**
             * @class ConjugationQuizWidget
             * @brief A widget for conducting conjugation quizzes in the GUI.
             *
             * This widget provides an interface for selecting conjugation types, conducting a quiz,
             * and displaying feedback on user performance. It supports features such as progress tracking,
             * displaying correct answers, and navigation between quiz states.
             */
            class ConjugationQuizWidget : public Widget
            {
            public:
                /**
                 * @brief Constructs a ConjugationQuizWidget.
                 *
                 * Initializes the widget with the specified lessons, number of tries, and selected conjugations.
                 *
                 * @param selectedConjugations A bitmask indicating selected conjugation types.
                 * @param numberOfTries The number of attempts allowed per flashcard.
                 * @param lessons A vector of lessons containing words and conjugations.
                 * @param logger A reference to a Logger instance for logging activity.
                 */
                ConjugationQuizWidget(uint16_t selectedConjugations, uint8_t numberOfTries, const std::vector<Lesson>& lessons, tools::Logger& logger);

                /**
                 * @brief Renders the widget to the GUI.
                 *
                 * Dynamically determines whether to render the selection window or the quiz window
                 * based on the current state of the widget.
                 *
                 * @param p_open A pointer to a boolean indicating whether the widget is open.
                 */
                void draw(bool* p_open) override;

            private:

                float calculateProgress();
                /**
                 * @brief Initializes quiz flashcards based on the selected conjugation types.
                 *
                 * @param selectedTypes A vector of conjugation types selected by the user.
                 */
                void initializeFlashcards(const std::vector<ConjugationType>& selectedTypes);

                /**
                 * @brief Retrieves a word by its ID from the lessons.
                 *
                 * @param id The unique ID of the word to retrieve.
                 * @return The Word object associated with the given ID.
                 * @throws std::runtime_error If the word ID is not found.
                 */
                Word getWordById(int id);

                /**
                 * @brief Draws the selection window for conjugation types.
                 *
                 * Displays a grid of checkboxes for selecting conjugation types and options
                 * to start or cancel the quiz. The selection is determined by `m_selectedConjugations`.
                 *
                 * @param p_open A pointer to a boolean indicating whether the widget is open.
                 */
                void drawSelectionWindow(bool* p_open);

                /**
                 * @brief Draws the quiz interface.
                 *
                 * Displays the current quiz question, user input, progress, and feedback.
                 *
                 * @param p_open A pointer to a boolean indicating whether the widget is open.
                 */
                void drawQuizWindow(bool* p_open);

                /**
                 * @brief Processes the user's input and determines if the answer is correct.
                 *
                 * Updates the feedback message, sets the appropriate buttons to be displayed,
                 * and prepares for the next input.
                 *
                 * @param correctAnswer The correct answer for the current flashcard.
                 */
                void processUserInput(const std::string& correctAnswer);

                /**
                 * @brief Renders the "Correct!" button for correct answers.
                 *
                 * Handles user interaction with the "Correct!" button, including keyboard focus
                 * and styling.
                 */
                void drawCorrectButton();

                /**
                 * @brief Renders the "Accept" and "Wrong" buttons for incorrect answers.
                 *
                 * Handles user interaction with decision buttons, including keyboard navigation
                 * and styling.
                 */
                void drawDecisionButtons();

                /**
                 * @brief Renders the quiz completion screen.
                 *
                 * Displays a summary of quiz statistics, including attempts and performance.
                 */
                void drawCompletionScreen();

                /**
                 * @brief Resets the widget state after a user action.
                 *
                 * Clears user input, resets feedback messages, and re-focuses the input field.
                 */
                void resetAfterInput();

                std::string ConjugationTypeToFullQuiestion(ConjugationType type, const std::string& romaji, const std::string& translation, bool showHint = false);

                uint16_t m_selectedConjugations;          /**< Bitmask representing selected conjugation types. */
                uint8_t m_numberOfTries;                  /**< Number of tries allowed for each flashcard. */
                std::vector<Lesson> m_lessons;            /**< A vector of lessons containing flashcard data. */
                tools::Logger& m_logger;                 /**< A logger instance for logging widget activity. */
                std::unique_ptr<tadaima::quiz::Quiz> m_quiz; /**< Pointer to the quiz logic instance. */

                char m_userInput[100] = { 0 };           /**< Buffer for storing user input during the quiz. */
                std::string m_correctAnswer;             /**< The correct answer for the current flashcard. */
                std::string m_correctAnswerMessage;      /**< Feedback message for the user's answer. */
                ImVec4 m_correctAnswerColor;             /**< Color for displaying the correct answer. */

                bool m_showButtons = false;              /**< Whether to show decision buttons (Accept/Wrong). */
                bool m_showCorrectButton = false;        /**< Whether to show the "Correct!" button. */
                bool m_focusOnCorrect = false;           /**< Whether the "Correct!" button has focus. */
                bool m_focusOnWrong = false;             /**< Whether the "Wrong" button has focus. */
                bool m_focusOnAccept = false;            /**< Whether the "Accept" button has focus. */
                bool m_setFocusOnInputField = true;      /**< Whether to focus on the input field. */

                bool showSelectionWindow = true;         /**< Whether to show the selection window. */
            };
        }
    }
}
