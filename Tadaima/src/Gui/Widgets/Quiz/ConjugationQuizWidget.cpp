#include "ConjugationQuizWidget.h"
#include "imgui.h"
#include <format>
#include <random>
#include "quiz/ConjugationItem.h"
#include "quiz/Quiz.h"

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

            float ConjugationQuizWidget::calculateProgress()
            {
                try
                {
                    int totalWords = m_quiz->getNumberOfItems();
                    int maxProgress = totalWords * m_numberOfTries; // Maximum progress is number of words * 2

                    // Calculate net progress based on correct and incorrect attempts
                    int correctAttempts = 0;
                    int incorrectAttempts = 0;

                    for( const auto& entry : m_quiz->getStatistics() )
                    {
                        correctAttempts += entry.second.goodAttempts;
                        incorrectAttempts += entry.second.badAttempts;
                    }

                    // Net progress is the total correct attempts minus the total incorrect attempts
                    int netProgress = correctAttempts - incorrectAttempts;

                    // Clamp the net progress between 0 and maxProgress
                    netProgress = std::max(0, std::min(netProgress, maxProgress));

                    // Calculate the progress as a float between 0 and 1
                    return static_cast<float>(netProgress) / maxProgress;
                }
                catch( const std::exception& e )
                {
                    m_logger.log(std::format("Error calculating progress: {}", e.what()), tools::LogLevel::PROBLEM);
                    return 0.0f;
                }
            }

            void ConjugationQuizWidget::initializeFlashcards(const std::vector<ConjugationType>& selectedTypes)
            {
                try
                {
                    std::vector<std::unique_ptr<tadaima::quiz::QuizItem>> flashcards;

                    for( const auto& lesson : m_lessons )
                    {
                        for( const auto& word : lesson.words )
                        {
                            for( const auto& type : selectedTypes )
                            {
                                const std::string& conjugation = word.conjugations[type];
                                if( !conjugation.empty() )
                                {
                                    flashcards.push_back(std::make_unique<tadaima::quiz::ConjugationItem>(word.id, type, conjugation));
                                }
                            }
                        }
                    }

                    if( flashcards.empty() )
                    {
                        throw std::runtime_error("No valid conjugation flashcards could be created.");
                    }

                    m_quiz = std::make_unique<quiz::Quiz>(flashcards, m_numberOfTries, true);
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
                constexpr int padding = 15;

                ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.98f, 0.92f, 0.84f, 1.0f)); // Light peach background

                if( ImGui::Begin("Select Conjugations", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar) )
                {
                    // Fancy Header
                    ImGui::TextColored(ImVec4(0.6f, 0.2f, 0.8f, 1.0f), "Welcome to the Conjugation Quiz!");
                    ImGui::Text("Select the conjugations you want to practice.");
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::BeginChild("CheckboxGrid", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - padding), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                    ImGui::Columns(numColumns, nullptr, false);

                    // Render checkboxes with the bitfield for selected conjugations
                    for( int i = 0; i < CONJUGATION_COUNT; ++i )
                    {
                        // Determine if the checkbox should be checked based on the bitfield
                        bool isChecked = (m_selectedConjugations & (1 << i)) != 0;

                        // Use colors for dynamic checkbox text
                        ImVec4 textColor = isChecked ? ImVec4(0.0f, 0.8f, 0.0f, 1.0f) : ImVec4(0.8f, 0.0f, 0.0f, 1.0f);

                        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
                        if( ImGui::Checkbox(ConjugationTypeToString(static_cast<ConjugationType>(i)).c_str(), &isChecked) )
                        {
                            // Update the bitfield based on the checkbox state
                            if( isChecked )
                                m_selectedConjugations |= (1 << i); // Set the bit
                            else
                                m_selectedConjugations &= ~(1 << i); // Clear the bit
                        }
                        ImGui::PopStyleColor();

                        ImGui::NextColumn();
                    }

                    ImGui::Columns(1);
                    ImGui::EndChild();

                    ImGui::Spacing();
                    ImGui::Separator();

                    // Navigation Buttons
                    if( ImGui::Button("Start Quiz", ImVec2(150, 40)) )
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
                    if( ImGui::Button("Cancel", ImVec2(150, 40)) )
                    {
                        *p_open = false;
                    }

                    ImGui::End();
                }

                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            }

            void ConjugationQuizWidget::drawQuizWindow(bool* p_open)
            {
                ImGui::SetNextWindowSize(ImVec2(700, 400), ImGuiCond_FirstUseEver);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.95f, 0.95f, 0.98f, 1.0f)); // Light bluish background

                if( ImGui::Begin("Conjugation Quiz", p_open, ImGuiWindowFlags_NoCollapse) )
                {
                    if( !m_quiz->isQuizComplete() )
                    {
                        // Title with Font Awesome icon
                        ImGui::TextColored(ImVec4(0.3f, 0.5f, 0.8f, 1.0f), "\uf121 Conjugation Quiz");
                        ImGui::Separator();
                        ImGui::Spacing();

                        // Progress Section
                        float progress = calculateProgress();
                        ImGui::Text("Conjugation to learn: %d", m_quiz->getNumberOfItems() - m_quiz->getLearntItems());

                        ImGui::ProgressBar(progress, ImVec2(-1, 0), std::format("{:.0f}%%", progress * 100).c_str());
                        ImGui::Spacing();
                        ImGui::Separator();

                        // Styled Question Card
                        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.98f, 0.92f, 1.0f)); // Soft yellow background
                        ImGui::BeginChild("QuizCard", ImVec2(-1, 80), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

                        const auto item = static_cast<const tadaima::quiz::ConjugationItem*>(m_quiz->getCurrentItem());
                        auto word = getWordById(std::stoi(item->getKey()));

                        static bool f1KeyWasPressed = false; // Tracks the key's previous state
                        static bool showHint = false;        // Tracks whether the hint should be shown

                        if( ImGui::IsKeyPressed(ImGuiKey_F1) )
                        {
                            if( !f1KeyWasPressed )
                            { // Only toggle on the first press
                                showHint = !showHint;
                                f1KeyWasPressed = true;
                            }
                        }
                        else
                        {
                            f1KeyWasPressed = false; // Reset the state when the key is released
                        }

                        // Highlighted Conjugation Type
                        ImGui::TextColored(ImVec4(0.8f, 0.3f, 0.3f, 1.0f), "\uf059 %s ", ConjugationTypeToFullQuiestion(item->getType(), word.romaji, word.translation, showHint).c_str());

                        // Display Word with emphasis
                        ImGui::TextWrapped("\uf0f6 \"%s\"", word.translation.c_str());

                        ImGui::EndChild();
                        ImGui::PopStyleColor();

                        ImGui::Spacing();
                        ImGui::Separator();

                        // Input Section
                        ImGui::Text("\uf044 Your Answer:");
                        if( m_setFocusOnInputField )
                        {
                            ImGui::SetKeyboardFocusHere();
                            m_setFocusOnInputField = false;
                        }

                        if( ImGui::InputText("##InputField", m_userInput, sizeof(m_userInput), ImGuiInputTextFlags_EnterReturnsTrue) )
                        {
                            m_correctAnswer = item->getAnswer();
                            if( m_quiz->isCorrect(m_userInput) )
                            {
                                m_correctAnswerMessage = "\uf058 Correct!";
                                m_correctAnswerColor = ImVec4(0.0f, 0.8f, 0.0f, 1.0f); // Green
                                m_showCorrectButton = true;
                                m_focusOnCorrect = true; // Focus on "Correct!" button
                                m_showButtons = false;
                            }
                            else
                            {
                                m_correctAnswerMessage = std::format("\uf057 Incorrect! Correct Answer: \"{}\"", m_correctAnswer);
                                m_correctAnswerColor = ImVec4(0.8f, 0.0f, 0.0f, 1.0f); // Red
                                m_showButtons = true;
                                m_focusOnWrong = true;
                                m_showCorrectButton = false;
                            }
                        }

                        // Feedback Message with Font Awesome icon
                        ImGui::TextColored(m_correctAnswerColor, "%s", m_correctAnswerMessage.c_str());
                        ImGui::Spacing();

                        // Buttons
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
                        const auto& word = getWordById(std::stoi(entry.first));
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

            std::string ConjugationQuizWidget::ConjugationTypeToFullQuiestion(ConjugationType type, const std::string& romaji, const std::string& translation, bool showHint)
            {
                bool isVerb = !romaji.empty() && romaji.back() == 'u'; // Check if the word ends with 'u'

                std::string word = translation;
                if( showHint )
                {
                    word += std::format(" ( {} )", romaji);
                }

                switch( type )
                {
                    case PLAIN:
                        return isVerb ? "What is the form of: \"" + word + "\"?" : "What is the form of: \"is " + word + "\"?";
                    case POLITE:
                        return isVerb ? "What is the POLITE form of: \"" + word + "\"?" : "What is the POLITE form of: \"is " + word + "\"?";
                    case NEGATIVE:
                        return isVerb ? "What is the form of: \"not " + word + "\"?" : "What is the form of: \"is not " + word + "\"?";
                    case POLITE_NEGATIVE:
                        return isVerb ? "What is the POLITE form of: \"not " + word + "\"?" : "What is the POLITE form of: \"is not " + word + "\"?";
                    case PAST:
                        return isVerb ? "What is the form of: \"did " + word + "\"?" : "What is the form of: \"was " + word + "\"?";
                    case POLITE_PAST:
                        return isVerb ? "What is the POLITE form of: \"did " + word + "\"?" : "What is the POLITE form of: \"was " + word + "\"?";
                    case PAST_NEGATIVE:
                        return isVerb ? "What is the form of: \"did not " + word + "\"?" : "What is the form of: \"was not " + word + "\"?";
                    case POLITE_PAST_NEGATIVE:
                        return isVerb ? "What is the POLITE form of: \"did not " + word + "\"?" : "What is the POLITE form of: \"was not " + word + "\"?";
                    case TE_FORM:
                        return isVerb ? "What is the te-form of: \"" + word + "\"?" : "What is the te-form of: \"is " + word + "\"?";
                    case POTENTIAL:
                        return isVerb ? "What is the form of: \"can " + word + "\"?" : "What is the form of: \"can be " + word + "\"?";
                    case PASSIVE:
                        return isVerb ? "What is the form of: \"is being " + word + "\"?" : "What is the form of: \"is considered as " + word + "\"?";
                    case CAUSATIVE:
                        return isVerb ? "What is the form of: \"make/let someone " + word + "\"?" : "What is the form of: \"is made to be " + word + "\"?";
                    case CONDITIONAL:
                        return isVerb ? "What is the form of: \"if " + word + "\"?" : "What is the form of: \"if it is " + word + "\"?";
                    case VOLITIONAL:
                        return isVerb ? "What is the form of: \"let's " + word + "\"?" : "What is the form of: \"let it be " + word + "\"?";
                    case IMPERATIVE:
                        return isVerb ? "What is the form of: \"do " + word + "\"?" : "What is the form of: \"be " + word + "\"?";
                    default:
                        return "Unknown conjugation type.";
                }
            }
        }
    }
}