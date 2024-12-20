#include "ConjugationQuizWidget.h"
#include "imgui.h"
#include <format>
#include <random>

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            ConjugationQuizWidget::ConjugationQuizWidget(uint16_t selectedConjugations, uint8_t numberOfTries, const std::vector<Lesson>& lessons, tools::Logger& logger)
                : m_selectedConjugations(selectedConjugations), m_numberOfTries(numberOfTries), m_lessons(lessons), m_logger(logger), m_showButtons(false), m_showCorrectButton(false), m_focusOnCorrect(false), m_focusOnWrong(false), m_focusOnAccept(false), m_correctAnswerMessage("Your answer is ..."), m_correctAnswerColor(ImVec4(0.0f, 0.0f, 0.0f, 1.0f))
            {
                m_logger.log("Initializing ConjugationQuizWidget...", tools::LogLevel::INFO);
            }

            void ConjugationQuizWidget::initializeFlashcards(const std::vector<ConjugationType>& selectedTypes)
            {
                try
                {
                    std::vector<quiz::ConjugationQuiz::ConjugationFlashCard> flashcards;

                    for( const auto& lesson : m_lessons )
                    {
                        for( const auto& word : lesson.words )
                        {
                            for( const auto& type : selectedTypes )
                            {
                                const std::string& conjugation = word.conjugations[type];
                                if( !conjugation.empty() )
                                {
                                    flashcards.emplace_back(word.id, type, conjugation);
                                }
                            }
                        }
                    }

                    if( flashcards.empty() )
                    {
                        throw std::runtime_error("No valid conjugation flashcards could be created.");
                    }

                    m_quiz = std::make_unique<quiz::ConjugationQuiz>(flashcards, m_numberOfTries, true);
                }
                catch( const std::exception& e )
                {
                    m_logger.log(std::format("Error during initialization: {}", e.what()), tools::LogLevel::PROBLEM);
                }
            }

            Word ConjugationQuizWidget::getWordById(int id)
            {
                for( const auto& lesson : m_lessons )
                {
                    for( const auto& word : lesson.words )
                    {
                        if( word.id == id )
                        {
                            return word;
                        }
                    }
                }
                throw std::runtime_error(std::format("Word with ID {} not found.", id));
            }

            void ConjugationQuizWidget::draw(bool* p_open)
            {
                if( showSelectionWindow )
                {
                    drawSelectionWindow(p_open);
                }
                else
                {
                    drawQuizWindow(p_open);
                }
            }

            void ConjugationQuizWidget::drawSelectionWindow(bool* p_open)
            {
                constexpr int numColumns = 3;
                constexpr int padding = 10;

                ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_Always);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));

                if( ImGui::Begin("Select Conjugations", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar) )
                {
                    ImGui::Text("Select conjugations to test:");
                    ImGui::Separator();

                    ImGui::BeginChild("CheckboxGrid", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - padding), false);
                    ImGui::Columns(numColumns, nullptr, false);

                    for( int i = 0; i < CONJUGATION_COUNT; ++i )
                    {
                        // Determine if the checkbox should be checked based on the bitfield
                        bool isChecked = (m_selectedConjugations & (1 << i)) != 0;

                        if( ImGui::Checkbox(ConjugationTypeToString(static_cast<ConjugationType>(i)).c_str(), &isChecked) )
                        {
                            // Update the bitfield based on the checkbox state
                            if( isChecked )
                                m_selectedConjugations |= (1 << i);  // Set the bit
                            else
                                m_selectedConjugations &= ~(1 << i); // Clear the bit
                        }

                        ImGui::NextColumn();
                    }

                    ImGui::Columns(1);
                    ImGui::EndChild();

                    ImGui::Separator();

                    if( ImGui::Button("Start Quiz", ImVec2(100, 0)) )
                    {
                        std::vector<ConjugationType> selectedTypes;
                        for( int i = 0; i < CONJUGATION_COUNT; ++i )
                        {
                            if( m_selectedConjugations & (1 << i) ) // Check if bit is set
                            {
                                selectedTypes.push_back(static_cast<ConjugationType>(i));
                            }
                        }

                        if( !selectedTypes.empty() )
                        {
                            initializeFlashcards(selectedTypes);
                            showSelectionWindow = false;
                        }
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Cancel", ImVec2(100, 0)) )
                    {
                        *p_open = false;
                    }

                    ImGui::End();
                }

                ImGui::PopStyleVar();
            }


            void ConjugationQuizWidget::drawQuizWindow(bool* p_open)
            {
                ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.98f, 0.92f, 0.84f, 1.0f));

                if( ImGui::Begin("Conjugation Quiz", p_open, ImGuiWindowFlags_NoCollapse) )
                {
                    if( !m_quiz->isQuizComplete() )
                    {
                        // Progress Section
                        ImGui::Text("Progress");
                        float progress = static_cast<float>(m_quiz->getLearntConjugations()) / m_quiz->getNumberOfFlashcards();
                        ImGui::ProgressBar(progress, ImVec2(-1, 0), std::format("{:.0f}%", progress * 100).c_str());
                        ImGui::Text("Conjugations to learn: %d", m_quiz->getNumberOfFlashcards() - m_quiz->getLearntConjugations());
                        ImGui::Separator();

                        // "Your answer is ..." message
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", m_correctAnswerMessage.c_str());

                        const auto& flashcard = m_quiz->getCurrentFlashCard();
                        const auto& word = getWordById(flashcard.wordId);

                        // Display Correct Answer
                        ImGui::TextColored(m_correctAnswerColor, "Correct Answer: %s", m_correctAnswer.c_str());
                        ImGui::Separator();

                        ImGui::Text("Conjugation Type: %s", ConjugationTypeToString(flashcard.type).c_str());
                        ImGui::Text("Word: %s", word.translation.c_str());

                        if( m_setFocusOnInputField )
                        {
                            ImGui::SetKeyboardFocusHere();
                            m_setFocusOnInputField = false;
                        }

                        if( ImGui::InputText("Your Answer", m_userInput, sizeof(m_userInput), ImGuiInputTextFlags_EnterReturnsTrue) )
                        {
                            m_correctAnswer = flashcard.answer;
                            if( m_quiz->isCorrect(m_userInput) )
                            {
                                m_correctAnswerMessage = "Your answer is correct!";
                                m_correctAnswerColor = ImVec4(0.0f, 0.8f, 0.0f, 1.0f); // Green
                                m_showCorrectButton = true;
                                m_focusOnCorrect = true; // Focus on "Correct!" button
                                m_showButtons = false;
                            }
                            else
                            {
                                m_correctAnswerMessage = "Your answer is incorrect.";
                                m_correctAnswerColor = ImVec4(0.8f, 0.0f, 0.0f, 1.0f); // Red
                                m_showButtons = true;
                                m_focusOnWrong = true;
                                m_showCorrectButton = false;
                            }
                        }

                        // Display Correct Button if correct
                        if( m_showCorrectButton )
                        {
                            drawCorrectButton();
                        }
                        else if( m_showButtons )
                        {
                            drawDecisionButtons();
                        }
                    }
                    else
                    {
                        drawCompletionScreen();
                    }

                    ImGui::End();
                }

                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
            }

            void ConjugationQuizWidget::drawCorrectButton()
            {
                static bool enterHandled = false;

                // Apply styling for the Correct! button when focused
                if( m_focusOnCorrect )
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
                    ImGui::SetKeyboardFocusHere();
                }

                // Draw the Correct! button
                if( ImGui::Button("Correct!") )
                {
                    m_quiz->advance(m_correctAnswer);
                    resetAfterInput();
                    enterHandled = false; // Reset the flag when button is clicked
                    ImGui::PopStyleColor(2);
                }

                // Handle Enter key for the Correct! button
                if( ImGui::IsKeyPressed(ImGuiKey_Enter) )
                {
                    if( !enterHandled )
                    {
                        enterHandled = true; // Set the flag after the first Enter is detected
                    }
                    else
                    {
                        // Handle the button action only after the first Enter is processed
                        m_quiz->advance(m_correctAnswer);
                        resetAfterInput();
                        enterHandled = false; // Reset the flag for subsequent inputs
                        ImGui::PopStyleColor(2);
                    }
                }

                // Pop styling if it was applied
                if( m_focusOnCorrect )
                {
                    ImGui::PopStyleColor(2);
                }
            }

            void ConjugationQuizWidget::drawDecisionButtons()
            {
                static bool enterHandled = false; // Flag to manage Enter key handling for decision buttons

                // Accept Button
                if( m_focusOnAccept )
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
                    ImGui::SetKeyboardFocusHere(); // Set focus on the Accept button
                }
                if( ImGui::Button("Accept") )
                {
                    m_correctAnswerMessage = "Your answer has been marked as correct.";
                    m_quiz->advance(m_correctAnswer);
                    resetAfterInput();
                    enterHandled = false; // Reset Enter handling for subsequent input
                    ImGui::PopStyleColor(2); // Restore styles
                }
                if( ImGui::IsKeyPressed(ImGuiKey_Enter) && m_focusOnAccept )
                {
                    if( !enterHandled )
                    {
                        enterHandled = true; // Mark Enter as handled for the current frame
                    }
                    else
                    {
                        m_correctAnswerMessage = "Your answer has been marked as correct.";
                        m_quiz->advance(m_correctAnswer);
                        resetAfterInput();
                        enterHandled = false; // Reset Enter handling
                        ImGui::PopStyleColor(2); // Restore styles
                    }
                }
                if( m_focusOnAccept )
                {
                    ImGui::PopStyleColor(2); // Restore styles
                }

                ImGui::SameLine();

                // Wrong Button
                if( m_focusOnWrong )
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                    ImGui::SetKeyboardFocusHere(); // Set focus on the Wrong button
                }
                if( ImGui::Button("Wrong") )
                {
                    m_quiz->advance(""); // Mark as wrong
                    resetAfterInput();
                    enterHandled = false; // Reset Enter handling for subsequent input
                    ImGui::PopStyleColor(2); // Restore styles
                }

                if( ImGui::IsKeyPressed(ImGuiKey_Enter) && m_focusOnWrong )
                {
                    if( !enterHandled )
                    {
                        enterHandled = true; // Mark Enter as handled for the current frame
                    }
                    else
                    {
                        m_quiz->advance(""); // Mark as wrong
                        resetAfterInput();
                        enterHandled = false; // Reset Enter handling
                        ImGui::PopStyleColor(2); // Restore styles
                    }
                }
                if( m_focusOnWrong )
                {
                    ImGui::PopStyleColor(2); // Restore styles
                }

                // Navigate Between Buttons
                if( ImGui::IsKeyPressed(ImGuiKey_LeftArrow) )
                {
                    m_focusOnAccept = true;
                    m_focusOnWrong = false;
                }
                if( ImGui::IsKeyPressed(ImGuiKey_RightArrow) )
                {
                    m_focusOnAccept = false;
                    m_focusOnWrong = true;
                }
            }

            void ConjugationQuizWidget::drawCompletionScreen()
            {
                // Title
                ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "Quiz Complete!");
                ImGui::Separator();

                // Summary Title
                ImGui::Text("Quiz Summary:");
                ImGui::Separator();

                // Display statistics in a styled table
                if( ImGui::BeginTable("QuizStats", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg) )
                {
                    // Header Row
                    ImGui::TableSetupColumn("Word", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Attempts", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Success Rate", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableHeadersRow();

                    const auto& statistics = m_quiz->getStatistics();

                    for( const auto& entry : statistics )
                    {
                        const auto& word = getWordById(entry.first);
                        int totalAttempts = entry.second.goodAttempts + entry.second.badAttempts;

                        // Populate Table Rows
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", word.translation.c_str());

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", totalAttempts);

                        ImGui::TableSetColumnIndex(2);
                        if( totalAttempts > 0 )
                        {
                            float successRate = static_cast<float>(entry.second.goodAttempts) / totalAttempts * 100.0f;
                            ImGui::Text("%.1f%%", successRate);
                        }
                        else
                        {
                            ImGui::Text("N/A");
                        }
                    }
                    ImGui::EndTable();
                }

                ImGui::SameLine();

                if( ImGui::Button("Close", ImVec2(150, 0)) )
                {
                    // Option to close the quiz or window
                    m_quiz.reset();  // Cleanup quiz data
                }
            }

            void ConjugationQuizWidget::resetAfterInput()
            {
                m_correctAnswerMessage = "Your answer is ...";
                m_correctAnswerColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Reset to black
                m_showButtons = false;
                m_showCorrectButton = false;
                m_focusOnWrong = false;
                m_focusOnAccept = false;
                m_focusOnCorrect = false;
                m_setFocusOnInputField = true;
                memset(m_userInput, 0, sizeof(m_userInput));
                m_correctAnswer.clear();
            }
        }
    }
}
