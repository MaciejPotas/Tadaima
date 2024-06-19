#include "VocabularyQuizWidget.h"
#include <algorithm>
#include "imgui.h"
#include "packages/SettingsDataPackage.h"
#include "Lessons/Lesson.h"
#include <stdexcept>
#include <format>
#include <random>

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            VocabularyQuizWidget::VocabularyQuizWidget(quiz::WordType base, quiz::WordType desired, const std::vector<Lesson>& lessons, tools::Logger& logger)
                : m_baseWord(base), m_inputWord(desired), m_lessons(lessons), m_logger(logger), m_correctAnswerMessage("You're answer is ...")
            {
                try
                {
                    m_logger.log("Initializing VocabularyQuizWidget...", tools::LogLevel::INFO);
                    std::vector<quiz::QuizWord> flashcards;
                    for( const auto& lesson : lessons )
                    {
                        for( const auto& word : lesson.words )
                        {
                            std::string string = getTranslation(word, desired);
                            if( !string.empty() )
                            {
                                flashcards.push_back({ word.id , string });
                            }
                        }
                    }

                    if( flashcards.empty() )
                    {
                        throw std::runtime_error("No valid flashcards could be created.");
                    }

                    m_quiz = std::make_unique<quiz::VocabularyQuiz>(flashcards, 2, true);
                }
                catch( const std::exception& e )
                {
                    m_logger.log(std::format("Error during initialization: {}", e.what()), tools::LogLevel::PROBLEM);
                }
            }

            tadaima::Word VocabularyQuizWidget::getWordById(int id)
            {
                try
                {
                    for( const auto& lesson : m_lessons )
                    {
                        for( const auto& word : lesson.words )
                        {
                            if( id == word.id )
                            {
                                return word;
                            }
                        }
                    }
                    throw std::invalid_argument(std::format("Wrong Id for the word: {}", id));
                }
                catch( const std::invalid_argument& e )
                {
                    m_logger.log(e.what(), tools::LogLevel::PROBLEM);
                    throw; // Re-throw to let the caller handle it if necessary
                }
            }

            std::string VocabularyQuizWidget::getTranslation(const Word& word, quiz::WordType type) const
            {
                std::string correctAnswer;

                switch( type )
                {
                    case quiz::WordType::BaseWord:
                        correctAnswer = word.translation;
                        break;

                    case quiz::WordType::Kana:
                        correctAnswer = word.kana;
                        break;

                    case quiz::WordType::Romaji:
                        correctAnswer = word.romaji;
                        break;

                    default:
                        m_logger.log(std::format("Wrong type for translation. Value: {}", static_cast<int>(type)), tools::LogLevel::PROBLEM);
                        break;
                }

                return correctAnswer;
            }

            std::string VocabularyQuizWidget::getHint()
            {
                try
                {
                    std::string translation = m_quiz->getCurrentFlashCard().word;

                    if( m_revealedHints.size() >= translation.size() )
                    {
                        return translation; // All hints revealed
                    }

                    // Get indices of characters not yet revealed
                    std::vector<int> indices;
                    for( int i = 0; i < static_cast<int>(translation.size()); ++i )
                    {
                        if( m_revealedHints.find(i) == m_revealedHints.end() )
                        {
                            indices.push_back(i);
                        }
                    }

                    // Select a random index from the unrevealed characters
                    if( !indices.empty() )
                    {
                        std::random_device rd;
                        std::mt19937 g(rd());
                        std::shuffle(indices.begin(), indices.end(), g);
                        int index = indices.front();

                        m_revealedHints.insert(index);
                        m_currentHint[index] = translation[index];
                    }

                    return m_currentHint;
                }
                catch( const std::exception& e )
                {
                    m_logger.log(std::format("Error getting hint: {}", e.what()), tools::LogLevel::PROBLEM);
                    return "";
                }
            }

            float VocabularyQuizWidget::calculateProgress()
            {
                try
                {
                    int totalWords = m_quiz->getNumberOflashcards();
                    int maxProgress = totalWords * 2; // Maximum progress is number of words * 2

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

            void VocabularyQuizWidget::draw(bool* p_open)
            {
                try
                {
                    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
                    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.98f, 0.92f, 0.84f, 1.0f)); // Light peach background
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10)); // Add padding

                    if( ImGui::Begin("Quiz Game", p_open, ImGuiWindowFlags_NoCollapse) )
                    {
                        static bool setFocusOnInputField = true;
                        static bool focusOnWrongButton = false;  // Variable to control focus on the "Wrong!" button
                        static bool focusOnAcceptButton = false;  // Variable to control focus on the "Accept it!" button
                        static bool enterPressed = false;  // Flag to detect Enter key press

                        // Detect Enter key press
                        if( ImGui::IsKeyPressed(ImGuiKey_Enter) )
                        {
                            enterPressed = true;
                        }

                        // Detect Left Arrow key press
                        bool leftArrowPressed = ImGui::IsKeyPressed(ImGuiKey_LeftArrow);
                        // Detect Right Arrow key press
                        bool rightArrowPressed = ImGui::IsKeyPressed(ImGuiKey_RightArrow);
                        // Detect H key press for hint
                        bool f1KeyPressed = ImGui::IsKeyPressed(ImGuiKey_F1);

                        // Progress Section
                        ImGui::Text("Progress");
                        float progress = calculateProgress();
                        ImGui::ProgressBar(progress, ImVec2(-1, 0));
                        ImGui::Text("Words to learn: %d", m_quiz->getNumberOflashcards() - m_quiz->getLearntWords());

                        ImGui::Separator();

                        // Quiz Section
                        if( !m_quiz->isQuizComplete() )
                        {
                            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", m_correctAnswerMessage.c_str());
                            ImGui::Text("Previous flashcard:");

                            if( m_inputWord == quiz::WordType::BaseWord && !m_translation.empty() )
                            {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                                ImGui::BulletText("Translation: %s", m_translation.c_str());
                                ImGui::PopStyleColor();
                            }
                            else
                            {
                                ImGui::BulletText("Translation: %s", m_translation.c_str());
                            }

                            if( m_inputWord == quiz::WordType::Kana && !m_kana.empty() )
                            {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                                ImGui::BulletText("Kana: %s", m_kana.c_str());
                                ImGui::PopStyleColor();
                            }
                            else
                            {
                                ImGui::BulletText("Kana: %s", m_kana.c_str());
                            }

                            if( m_inputWord == quiz::WordType::Romaji && !m_romaji.empty() )
                            {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                                ImGui::BulletText("Romaji: %s", m_romaji.c_str());
                                ImGui::PopStyleColor();
                            }
                            else
                            {
                                ImGui::BulletText("Romaji: %s", m_romaji.c_str());
                            }

                            ImGui::BulletText("Example: %s", m_example.c_str());

                            ImGui::Spacing();

                            const auto& flashcard = m_quiz->getCurrentFlashCard();
                            auto word = getWordById(flashcard.wordId);
                            auto translate = getTranslation(word, m_baseWord);

                            ImGui::Text("Word:");
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "%s", translate.c_str());
                            if( setFocusOnInputField )
                            {
                                ImGui::SetKeyboardFocusHere();
                                setFocusOnInputField = false;
                            }

                            if( ImGui::InputText(" ", m_userInput, sizeof(m_userInput), ImGuiInputTextFlags_EnterReturnsTrue) )
                            {
                                m_translation = word.translation;
                                m_kana = word.kana;
                                m_romaji = word.romaji;
                                m_example = word.exampleSentence;

                                m_showCorrectAnswer = true;
                                m_correctAnswer = flashcard.word;

                                const bool isCorrect = m_quiz->isCorrect(m_userInput);

                                if( isCorrect )
                                {
                                    m_correctAnswerMessage = "Your answer is correct!";
                                    m_revealedHints.clear();
                                    m_currentHint.clear();
                                    focusOnWrongButton = false;
                                    focusOnAcceptButton = true;
                                    enterPressed = false;  // Clear Enter key press flag
                                }
                                else
                                {
                                    m_correctAnswerMessage = "Your answer is incorrect.";
                                    focusOnWrongButton = true;
                                    focusOnAcceptButton = false;
                                }

                                m_overrideAnswer = false;
                                setFocusOnInputField = true;

                                // Clear Enter key press flag after handling input
                                enterPressed = false;
                            }

                            ImGui::Spacing();

                            // Controls Section
                            ImGui::Separator();
                            if( ImGui::Button("Hint") || f1KeyPressed )
                            {
                                if( m_currentHint.empty() )
                                {
                                    m_currentHint = std::string(flashcard.word.size(), '*'); // Reset currentHint to asterisks
                                }
                                m_currentHint = getHint();
                            }

                            ImGui::SameLine();

                            if( !m_overrideAnswer && m_correctAnswerMessage == "Your answer is correct!" )
                            {
                                // Accept it! button
                                if( focusOnAcceptButton )
                                {
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));  // Change button color to green
                                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));  // Change hover color
                                    ImGui::SetKeyboardFocusHere();
                                }
                                if( ImGui::Button("Correct!") || (focusOnAcceptButton && enterPressed) )
                                {
                                    m_quiz->advance(flashcard.word);
                                    memset(m_userInput, 0, sizeof(m_userInput));
                                    m_overrideAnswer = true;
                                    m_correctAnswerMessage = "Your answer has been marked as correct!";
                                    m_revealedHints.clear();
                                    m_currentHint.clear();
                                    setFocusOnInputField = true;
                                    enterPressed = false;  // Clear Enter key press flag
                                    focusOnAcceptButton = false;
                                    ImGui::PopStyleColor(2);  // Revert to the original button color

                                    m_translation.clear();
                                    m_kana.clear();
                                    m_romaji.clear();
                                    m_example.clear();
                                }
                                if( focusOnAcceptButton )
                                {
                                    ImGui::PopStyleColor(2);  // Revert to the original button color
                                }
                            }

                            if( !m_overrideAnswer && m_correctAnswerMessage == "Your answer is incorrect." )
                            {
                                // Handle left arrow navigation to switch focus
                                if( leftArrowPressed )
                                {
                                    if( focusOnWrongButton )
                                    {
                                        focusOnAcceptButton = true;
                                        focusOnWrongButton = false;
                                    }
                                }
                                // Handle right arrow navigation to switch focus
                                if( rightArrowPressed )
                                {
                                    if( focusOnAcceptButton )
                                    {
                                        focusOnAcceptButton = false;
                                        focusOnWrongButton = true;
                                    }
                                }

                                // Accept it! button
                                if( focusOnAcceptButton )
                                {
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));  // Change button color to green
                                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));  // Change hover color
                                    ImGui::SetKeyboardFocusHere();
                                }
                                if( ImGui::Button("Accept it!") || (focusOnAcceptButton && enterPressed) )
                                {
                                    m_quiz->advance(flashcard.word);
                                    memset(m_userInput, 0, sizeof(m_userInput));
                                    m_overrideAnswer = true;
                                    m_correctAnswerMessage = "Your answer has been marked as correct!";
                                    m_revealedHints.clear();
                                    m_currentHint.clear();
                                    setFocusOnInputField = true;
                                    enterPressed = false;  // Clear Enter key press flag
                                    focusOnAcceptButton = false;
                                    ImGui::PopStyleColor(2);  // Revert to the original button color
                                    m_translation.clear();
                                    m_kana.clear();
                                    m_romaji.clear();
                                    m_example.clear();
                                }
                                if( focusOnAcceptButton )
                                {
                                    ImGui::PopStyleColor(2);  // Revert to the original button color
                                }

                                ImGui::SameLine();
                                // Wrong! button
                                if( focusOnWrongButton )
                                {
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));  // Change button color to red
                                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));  // Change hover color
                                    ImGui::SetKeyboardFocusHere();
                                    if( ImGui::Button("Wrong!") || (focusOnWrongButton && enterPressed) )
                                    {
                                        m_quiz->advance(m_userInput);
                                        memset(m_userInput, 0, sizeof(m_userInput));
                                        m_correctAnswerMessage = "Your answer has been marked as wrong!";
                                        m_revealedHints.clear();
                                        m_currentHint.clear();
                                        setFocusOnInputField = true;
                                        focusOnWrongButton = false;
                                        enterPressed = false;  // Clear Enter key press flag
                                        m_translation.clear();
                                        m_kana.clear();
                                        m_romaji.clear();
                                        m_example.clear();
                                    }
                                    ImGui::PopStyleColor(2);  // Revert to the original button color
                                }
                                else
                                {
                                    if( ImGui::Button("Wrong!") )
                                    {
                                        m_quiz->advance(m_userInput);
                                        memset(m_userInput, 0, sizeof(m_userInput));
                                        m_correctAnswerMessage = "Your answer has been marked as wrong!";
                                        m_revealedHints.clear();
                                        m_currentHint.clear();
                                        setFocusOnInputField = true;
                                    }
                                }
                            }

                            if( !m_currentHint.empty() )
                            {
                                ImGui::Text("Hint: %s", m_currentHint.c_str());
                            }
                        }
                        else
                        {
                            ImGui::Text("Quiz Complete!");
                            ImGui::Separator();
                            ImGui::Text("Quiz Statistics:");
                            const auto& statistics = m_quiz->getStatistics();
                            for( const auto& entry : statistics )
                            {
                                auto word = getWordById(entry.first);
                                auto translate = getTranslation(word, m_baseWord);
                                ImGui::Text("Word: %s", translate.c_str());
                                ImGui::SameLine();
                                ImGui::Text("Attempts: %d", entry.second.badAttempts); // Assuming badAttempts and goodAttempts exist
                            }
                        }
                    }
                    ImGui::End();

                    ImGui::PopStyleColor();  // Restore previous style
                    ImGui::PopStyleVar();  // Restore previous padding
                }
                catch( const std::exception& e )
                {
                    m_logger.log(std::format("Error in draw method: {}", e.what()), tools::LogLevel::PROBLEM);
                }
            }

        }
    }
}
