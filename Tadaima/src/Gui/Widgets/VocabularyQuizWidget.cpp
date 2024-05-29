#include "VocabularyQuizWidget.h"
#include <algorithm>
#include "imgui.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            VocabularyQuizWidget::VocabularyQuizWidget(tools::Logger& logger, const std::vector<Lesson>& lessons) : m_logger(logger)
            {
                initializeFlashcards(lessons);

                m_motherType = quiz::WordType::Translation;
                m_learnType = quiz::WordType::Romaji;

                m_quiz = std::make_unique<quiz::VocabularyQuiz>(m_flashcards, m_learnType, 2, true);
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

                if( m_revealedHints.size() >= romaji.size() ) return romaji; // All hints revealed

                // Get indices of characters not yet revealed
                std::vector<int> indices;
                for( int i = 0; i < static_cast<int>(romaji.size()); ++i )
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
                    m_currentHint[index] = romaji[index];
                }

                return m_currentHint;
            }

            float VocabularyQuizWidget::calculateProgress()
            {
                int totalWords = m_flashcards.size();
                int maxProgress = totalWords * 2; // Maximum progress is number of words * 2

                // Calculate net progress based on correct and incorrect attempts
                int correctAttempts = 0;
                int incorrectAttempts = 0;

                for( const auto& flashcard : m_flashcards )
                {
                    correctAttempts += flashcard.goodAttemps;
                    incorrectAttempts += flashcard.badAttempts;
                }

                // Net progress is the total correct attempts minus the total incorrect attempts
                int netProgress = correctAttempts - incorrectAttempts;

                // Clamp the net progress between 0 and maxProgress
                netProgress = std::max(0, std::min(netProgress, maxProgress));

                // Calculate the progress as a float between 0 and 1
                return static_cast<float>(netProgress) / maxProgress;
            }

            std::string VocabularyQuizWidget::getCorrectAnswer(quiz::WordType type, const quiz::Flashcard& flashcard)
            {
                std::string correctAnswer;

                switch( type )
                {
                    case quiz::WordType::Translation:
                        correctAnswer = flashcard.word.translation;
                        break;

                    case quiz::WordType::Kana:
                        correctAnswer = flashcard.word.kana;
                        break;

                    case quiz::WordType::Romaji:
                        correctAnswer = flashcard.word.romaji;
                        break;
                    default:
                        break;
                }

                return correctAnswer;
            }

            void VocabularyQuizWidget::draw(bool* p_open)
            {
                ImGui::SetNextWindowSize(ImVec2(600, 220), ImGuiCond_FirstUseEver);

                if( ImGui::Begin("Quiz Game", p_open, ImGuiWindowFlags_NoCollapse) )
                {
                    ImGui::Columns(2, NULL, true); // Create two columns

                    // Left Column
                    ImGui::SetColumnWidth(0, 300);
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);

                    // Use the calculateProgress function to get the progress value
                    float progress = calculateProgress();
                    ImGui::ProgressBar(progress, ImVec2(-1, 0), "Progress");

                    ImGui::Spacing();
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);
                    ImGui::Text("Words to learn: %d", m_flashcards.size() - m_quiz->getLearntWords().size());

                    if( m_quiz->isQuizComplete() )
                    {
                        m_logger.log("Vocabulary quiz complete", tools::LogLevel::INFO);
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

                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);
                        ImGui::Text("Word: %s", flashcard.word.translation.c_str());
                        ImGui::Spacing();

                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);
                        ImGui::PushItemWidth(-1);
                        if( ImGui::InputText("Translation", m_userInput, sizeof(m_userInput), ImGuiInputTextFlags_EnterReturnsTrue) )
                        {
                            m_translation = flashcard.word.translation;
                            m_kana = flashcard.word.kana;
                            m_romaji = flashcard.word.romaji;
                            m_example = flashcard.word.exampleSentence;

                            m_showCorrectAnswer = true;
                            m_correctAnswer = flashcard.word.romaji;
                            const bool isCorrect = m_quiz->isCorrect(m_userInput);

                            if( isCorrect )
                            {
                                m_quiz->advance(m_userInput);
                                memset(m_userInput, 0, sizeof(m_userInput));
                                m_correctAnswerMessage = "Your answer is correct!";
                            }
                            else
                            {
                                m_correctAnswerMessage = "Your answer is incorrect.";
                            }

                            m_overrideAnswer = false;

                            // Set focus back to input field
                            ImGui::SetKeyboardFocusHere(-1);
                        }
                        ImGui::PopItemWidth();

                        ImGui::Spacing();
                        if( ImGui::Button("Hint") )
                        {
                            if( m_currentHint.empty() )
                            {
                                m_currentHint = std::string(flashcard.word.romaji.size(), '*'); // Reset currentHint to asterisks
                            }
                            m_currentHint = getHint();
                        }

                        if( !m_overrideAnswer && m_correctAnswerMessage == "Your answer is incorrect." )
                        {
                            ImGui::SameLine();
                            if( ImGui::Button("Accept it!") )
                            {
                                m_quiz->advance(getCorrectAnswer(m_learnType, flashcard));
                                memset(m_userInput, 0, sizeof(m_userInput));
                                m_overrideAnswer = true;
                                m_correctAnswerMessage = "Your answer has been marked as correct!";
                            }

                            ImGui::SameLine();
                            if( ImGui::Button("Wrong!") )
                            {
                                m_quiz->advance(m_userInput);
                                memset(m_userInput, 0, sizeof(m_userInput));
                                m_correctAnswerMessage = "Your answer has been marked as wrong!";
                            }
                        }

                        if( !m_currentHint.empty() )
                        {
                            ImGui::Text("Hint: %s", m_currentHint.c_str());
                        }

                        ImGui::NextColumn();

                        // Right Column
                        if( m_showCorrectAnswer )
                        {
                            ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "%s", m_correctAnswerMessage.c_str());

                            ImGui::Text("Previous flashcard:");

                            ImGui::Text("Translation:");
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "%s", m_translation.c_str());

                            ImGui::Text("Kana:");
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "%s", m_kana.c_str());

                            ImGui::Text("Romaji:");
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "%s", m_romaji.c_str());

                            ImGui::Text("Example:");
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "%s", m_example.c_str());
                        }
                        else
                        {
                            ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), "%s", "Your answer.");

                            ImGui::Text("Previous flashcard:");
                            ImGui::Text("Translation:");
                            ImGui::Text("Kana:");
                            ImGui::Text("Romaji:");
                            ImGui::Text("Example:");
                        }

                        ImGui::Spacing();
                    }

                    ImGui::Columns(1);
                }

                ImGui::End();
            }
        }
    }
}
