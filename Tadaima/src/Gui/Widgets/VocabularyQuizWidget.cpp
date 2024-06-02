#include "VocabularyQuizWidget.h"
#include <algorithm>
#include "imgui.h"
#include "packages/SettingsDataPackage.h"
#include "Lessons/Lesson.h"
#include <stdexcept>
#include <format>
#include <algorithm>
#include <random>
namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            VocabularyQuizWidget::VocabularyQuizWidget(quiz::WordType base, quiz::WordType desired, const std::vector<Lesson>& lessons, tools::Logger& logger)
                : m_baseWord(base), m_inputWord(desired), m_lessons(lessons), m_logger(logger)
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
                    ImGui::SetNextWindowSize(ImVec2(600, 220), ImGuiCond_FirstUseEver);
                    if( ImGui::Begin("Quiz Game", p_open, ImGuiWindowFlags_NoCollapse) )
                    {
                        static bool setFocusOnInputField = true;

                        ImGui::Columns(2, NULL, true); // Create two columns

                        // Left Column
                        ImGui::SetColumnWidth(0, 300);
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);

                        // Use the calculateProgress function to get the progress value
                        float progress = calculateProgress();
                        ImGui::ProgressBar(progress, ImVec2(-1, 0), "Progress");

                        ImGui::Spacing();
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);
                        ImGui::Text("Words to learn: %d", m_quiz->getNumberOflashcards() - m_quiz->getLearntWords());

                        if( m_quiz->isQuizComplete() )
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
                        else
                        {
                            const auto& flashcard = m_quiz->getCurrentFlashCard();
                            auto word = getWordById(flashcard.wordId);
                            auto translate = getTranslation(word, m_baseWord);

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);
                            ImGui::Text("Word: %s", translate.c_str());
                            ImGui::Spacing();

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);
                            ImGui::PushItemWidth(-1);

                            if( setFocusOnInputField )
                            {
                                ImGui::SetKeyboardFocusHere();
                                setFocusOnInputField = false;
                            }

                            if( ImGui::InputText("Translation", m_userInput, sizeof(m_userInput), ImGuiInputTextFlags_EnterReturnsTrue) )
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
                                    m_quiz->advance(m_userInput);
                                    memset(m_userInput, 0, sizeof(m_userInput));
                                    m_correctAnswerMessage = "Your answer is correct!";
                                    m_revealedHints.clear();
                                    m_currentHint.clear();
                                }
                                else
                                {
                                    m_correctAnswerMessage = "Your answer is incorrect.";
                                }

                                m_overrideAnswer = false;

                                // Set focus back to input field
                                setFocusOnInputField = true;
                            }

                            ImGui::PopItemWidth();

                            ImGui::Spacing();
                            if( ImGui::Button("Hint") )
                            {
                                if( m_currentHint.empty() )
                                {
                                    m_currentHint = std::string(flashcard.word.size(), '*'); // Reset currentHint to asterisks
                                }
                                m_currentHint = getHint();
                            }

                            if( !m_overrideAnswer && m_correctAnswerMessage == "Your answer is incorrect." )
                            {
                                ImGui::SameLine();
                                if( ImGui::Button("Accept it!") )
                                {
                                    m_quiz->advance(flashcard.word);
                                    memset(m_userInput, 0, sizeof(m_userInput));
                                    m_overrideAnswer = true;
                                    m_correctAnswerMessage = "Your answer has been marked as correct!";
                                    m_revealedHints.clear();
                                    m_currentHint.clear();
                                    setFocusOnInputField = true;
                                }

                                ImGui::SameLine();
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
                catch( const std::exception& e )
                {
                    m_logger.log(std::format("Error in draw method: {}", e.what()), tools::LogLevel::PROBLEM);
                }
            }
        }
    }
}
