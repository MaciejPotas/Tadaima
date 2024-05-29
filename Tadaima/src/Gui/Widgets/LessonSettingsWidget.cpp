#include "LessonSettingsWidget.h"
#include "Lessons/LessonManager.h"
#include "imgui.h"
#include <cstring>
#include <iosfwd>
#include <sstream>

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            void LessonSettingsWidget::draw(bool* p_open)
            {
                if( *p_open )
                {
                    ImGui::OpenPopup(m_isEditing ? "Edit Lesson Modal" : "Add New Lesson Modal");
                }

                ImGui::SetNextWindowSize(ImVec2(700, 400), ImGuiCond_Always);  // Adjust size as needed
                ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

                // Open the modal window
                if( ImGui::BeginPopupModal(m_isEditing ? "Edit Lesson Modal" : "Add New Lesson Modal", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize) )
                {
                    ImGui::Columns(2);
                    ImGui::InputText("Main Name", m_mainNameBuffer, sizeof(m_mainNameBuffer));
                    ImGui::InputText("Sub Name", m_subNameBuffer, sizeof(m_subNameBuffer));

                    // Left Column: Word input fields
                    ImGui::Text("Add/Edit Word");
                    ImGui::InputText("Kana", m_kanaBuffer, sizeof(m_kanaBuffer));
                    ImGui::InputText("Translation", m_translationBuffer, sizeof(m_translationBuffer));
                    ImGui::InputText("Romaji", m_romajiBuffer, sizeof(m_romajiBuffer));
                    ImGui::InputText("Example Sentence", m_exampleSentenceBuffer, sizeof(m_exampleSentenceBuffer));
                    ImGui::InputText("Tags (comma-separated)", m_tagBuffer, sizeof(m_tagBuffer));
                    if( ImGui::IsItemHovered() )
                    {
                        ImGui::SetTooltip("Enter tags separated by commas, e.g., tag1,tag2,tag3");
                    }

                    if( ImGui::Button("Add Word") )
                    {
                        // Ensure required fields are set
                        if( std::strlen(m_translationBuffer) > 0 && std::strlen(m_kanaBuffer) > 0 )
                        {
                            Word newWord;
                            newWord.kana = std::string(m_kanaBuffer);
                            newWord.translation = std::string(m_translationBuffer);
                            newWord.romaji = std::string(m_romajiBuffer);
                            newWord.exampleSentence = std::string(m_exampleSentenceBuffer);

                            // Split tags by comma
                            std::string tagsStr(m_tagBuffer);
                            std::istringstream iss(tagsStr);
                            std::string tag;
                            while( std::getline(iss, tag, ',') )
                            {
                                newWord.tags.push_back(tag);
                            }

                            m_newLesson.mainName = std::string(m_mainNameBuffer);
                            m_newLesson.subName = std::string(m_subNameBuffer);
                            m_newLesson.words.push_back(newWord);

                            // Clear the word input fields for the next word
                            std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                            std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                            std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                            std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                            std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                        }
                    }

                    ImGui::NextColumn();

                    // Right Column: List of words
                    ImGui::BeginChild("WordsList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
                    ImGui::Text("Words in this Lesson:");
                    ImGui::Separator();

                    for( size_t index = 0; index < m_newLesson.words.size(); ++index )
                    {
                        const auto& word = m_newLesson.words[index];
                        if( ImGui::Selectable(word.kana.c_str()) )
                        {
                            // Handle word selection if needed
                        }

                        if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) )
                        {
                            // Fill the input fields with the selected word's data
                            std::strncpy(m_kanaBuffer, word.kana.c_str(), sizeof(m_kanaBuffer));
                            std::strncpy(m_translationBuffer, word.translation.c_str(), sizeof(m_translationBuffer));
                            std::strncpy(m_romajiBuffer, word.romaji.c_str(), sizeof(m_romajiBuffer));
                            std::strncpy(m_exampleSentenceBuffer, word.exampleSentence.c_str(), sizeof(m_exampleSentenceBuffer));

                            // Convert tags to a comma-separated string
                            std::string tagsStr;
                            for( const auto& tag : word.tags )
                            {
                                if( !tagsStr.empty() )
                                    tagsStr += ",";
                                tagsStr += tag;
                            }
                            std::strncpy(m_tagBuffer, tagsStr.c_str(), sizeof(m_tagBuffer));
                        }
                    }

                    ImGui::EndChild();

                    ImGui::Columns(1);

                    if( ImGui::Button("Save Lesson") )
                    {
                        if( strlen(m_mainNameBuffer) > 0 && strlen(m_subNameBuffer) > 0 && !m_newLesson.words.empty() )
                        {
                            // Set the lesson's main name and sub name from the buffers
                            m_lesson->mainName = std::string(m_mainNameBuffer);
                            m_lesson->subName = std::string(m_subNameBuffer);
                            m_lesson->words = m_newLesson.words;
                            *p_open = false;
                            ImGui::CloseCurrentPopup(); // Close the modal
                        }
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Cancel") )
                    {
                        // Clear the buffers and reset the new lesson
                        std::memset(m_mainNameBuffer, 0, sizeof(m_mainNameBuffer));
                        std::memset(m_subNameBuffer, 0, sizeof(m_subNameBuffer));
                        std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                        std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                        std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                        std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                        std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                        m_newLesson = Lesson(); // Reset new lesson
                        m_isEditing = false; // Exit edit mode
                        *p_open = false;
                        ImGui::CloseCurrentPopup(); // Close the modal
                    }

                    ImGui::EndPopup();
                }

                ImGui::PopStyleColor();  // Restore previous style

                if( !ImGui::IsPopupOpen("Add New Lesson Modal") && !ImGui::IsPopupOpen("Edit Lesson Modal") )
                {
                    std::memset(m_mainNameBuffer, 0, sizeof(m_mainNameBuffer));
                    std::memset(m_subNameBuffer, 0, sizeof(m_subNameBuffer));
                    std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                    std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                    std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                    std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                    std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                    m_newLesson = Lesson(); // Reset new lesson
                }
            }

            void LessonSettingsWidget::setLesson(Lesson& lesson)
            {
                m_lesson = &lesson;
                // Populate buffers with the current lesson data for editing
                std::strncpy(m_mainNameBuffer, lesson.mainName.c_str(), sizeof(m_mainNameBuffer));
                std::strncpy(m_subNameBuffer, lesson.subName.c_str(), sizeof(m_subNameBuffer));
                m_newLesson = lesson; // Copy the lesson to the newLesson object for editing
                m_isEditing = true; // Enable edit mode
            }
        }
    }
}
