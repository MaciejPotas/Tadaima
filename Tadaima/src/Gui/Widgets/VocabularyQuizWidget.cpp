#include "VocabularyQuizWidget.h"
#include <algorithm>
#include <random>
#include <numeric>
#include <cstring> // For memset

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            VocabularyQuizWidget::VocabularyQuizWidget(tools::Logger& logger, const std::vector<Lesson>& lessons)
            {
                initializeFlashcards(lessons);
                m_quiz = std::make_unique<quiz::VocabularyQuiz>(m_flashcards, quiz::AnswerType::Romaji, 2, true);
            }

            void VocabularyQuizWidget::initializeFlashcards(const std::vector<Lesson>& lessons)
            {
                for( const auto& lesson : lessons )
                {
                    for( const auto& word : lesson.words )
                    {
                        m_flashcards.emplace_back(word, lesson.id, 0);
                    }
                }
            }

            std::string VocabularyQuizWidget::getHint()
            {
                const auto& romaji = m_quiz->getCurrentFlashCard().word.romaji;

                if( revealedHints.size() >= romaji.size() ) return romaji; // All hints revealed

                // Get indices of characters not yet revealed
                std::vector<int> indices;
                for( int i = 0; i < romaji.size(); ++i )
                {
                    if( revealedHints.find(i) == revealedHints.end() )
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

                    revealedHints.insert(index);
                    currentHint[index] = romaji[index];
                }

                return currentHint;
            }

            void VocabularyQuizWidget::draw(bool* p_open)
            {
                ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

                if( ImGui::Begin("Quiz Game", p_open, ImGuiWindowFlags_NoCollapse) )
                {
                    if( m_quiz->isQuizComplete() )
                    {
                        ImGui::Text("Quiz Complete!");
                        ImGui::Separator();
                        ImGui::Text("Quiz Statistics:");
                        for( const auto& flashcard : m_flashcards )
                        {
                            ImGui::Text("Word: %s", flashcard.word.translation.c_str());
                            ImGui::SameLine();
                            ImGui::Text("Attempts: %d", flashcard.badAttempts); // Assuming badAttempts and goodAttempts exist
                        }
                    }
                    else
                    {
                        const auto& flashcard = m_quiz->getCurrentFlashCard();

                        ImGui::Text("Word: %s", flashcard.word.translation.c_str());
                        ImGui::Spacing();

                        ImGui::PushItemWidth(-1);
                        if( ImGui::InputText("Translation", m_userInput, sizeof(m_userInput), ImGuiInputTextFlags_EnterReturnsTrue) )
                        {
                            showCorrectAnswer = true;
                            correctAnswer = flashcard.word.romaji;
                            bool isCorrect = m_quiz->advance(m_userInput);

                            memset(m_userInput, 0, sizeof(m_userInput));
                            revealedHints.clear();
                            currentHint.clear();

                            if( isCorrect )
                            {
                                correctAnswerMessage = "Your answer is correct!";
                            }
                            else
                            {
                                correctAnswerMessage = "Your answer is incorrect.";
                            }

                            // Set focus back to input field
                            ImGui::SetKeyboardFocusHere(-1);
                        }
                        ImGui::PopItemWidth();

                        ImGui::Spacing();
                        if( ImGui::Button("Hint") )
                        {
                            if( currentHint.empty() )
                            {
                                currentHint = std::string(flashcard.word.romaji.size(), '*'); // Reset currentHint to asterisks
                            }
                            currentHint = getHint();
                        }
                        ImGui::SameLine();
                        if( !currentHint.empty() )
                        {
                            ImGui::Text("Hint: %s", currentHint.c_str());
                        }

                        if( showCorrectAnswer )
                        {
                            ImGui::Spacing();

                            if( correctAnswerMessage == "Your answer is correct!" )
                            {
                                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", correctAnswerMessage.c_str());
                            }
                            else
                            {
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", correctAnswerMessage.c_str());
                            }

                            ImGui::Text("Correct Answer:");
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", correctAnswer.c_str());
                        }

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Text("Words to learn: %d", m_flashcards.size() - m_quiz->getLearntWords().size());
                    }
                }

                ImGui::End();
            }
        }
    }
}
