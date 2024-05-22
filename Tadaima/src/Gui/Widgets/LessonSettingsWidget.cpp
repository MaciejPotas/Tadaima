#include "LessonSettingsWidget.h"
#include "imgui.h"
#include <cstring>
#include <iosfwd>
#include <sstream> // Include this header for std::istringstream

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            void LessonSettingsWidget::draw(bool* p_open)
            {
                static Lesson newLesson; // New lesson to be added
                static char mainNameBuffer[256] = "";  // Buffer for the lesson main name
                static char subNameBuffer[256] = "";   // Buffer for the lesson sub name
                static char kanaBuffer[256] = "";      // Buffer for word kana
                static char translationBuffer[256] = ""; // Buffer for word translation
                static char romajiBuffer[256] = "";    // Buffer for word romaji
                static char exampleSentenceBuffer[256] = ""; // Buffer for example sentence
                static char tagBuffer[256] = "";       // Buffer for word tags

                if( *p_open )
                {
                    ImGui::OpenPopup("Add New Lesson Modal");
                }

                ImGui::SetNextWindowSize(ImVec2(700, 400), ImGuiCond_Always);  // Adjust size as needed
                // Open the modal window
                if( ImGui::BeginPopupModal("Add New Lesson Modal", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize) )
                {
                    ImGui::InputText("Main Name", mainNameBuffer, sizeof(mainNameBuffer));
                    ImGui::InputText("Sub Name", subNameBuffer, sizeof(subNameBuffer));

                    ImGui::Columns(2);

                    // Left Column: Word input fields
                    ImGui::Text("Add/Edit Word");
                    ImGui::InputText("Kana", kanaBuffer, sizeof(kanaBuffer));
                    ImGui::InputText("Translation", translationBuffer, sizeof(translationBuffer));
                    ImGui::InputText("Romaji", romajiBuffer, sizeof(romajiBuffer));
                    ImGui::InputText("Example Sentence", exampleSentenceBuffer, sizeof(exampleSentenceBuffer));
                    ImGui::InputText("Tags (comma-separated)", tagBuffer, sizeof(tagBuffer));

                    if( ImGui::Button("Add Word") )
                    {
                        // Ensure required fields are set
                        if( std::strlen(translationBuffer) > 0 && std::strlen(kanaBuffer) > 0 )
                        {
                            Word newWord;
                            newWord.kana = std::string(kanaBuffer);
                            newWord.translation = std::string(translationBuffer);
                            newWord.romaji = std::string(romajiBuffer);
                            newWord.exampleSentence = std::string(exampleSentenceBuffer);

                            // Split tags by comma
                            std::string tagsStr(tagBuffer);
                            std::istringstream iss(tagsStr);
                            std::string tag;
                            while( std::getline(iss, tag, ',') )
                            {
                                newWord.tags.push_back(tag);
                            }

                            newLesson.mainName = std::string(mainNameBuffer);
                            newLesson.subName = std::string(subNameBuffer);
                            newLesson.words.push_back(newWord);

                            // Clear the word input fields for the next word
                            std::memset(kanaBuffer, 0, sizeof(kanaBuffer));
                            std::memset(translationBuffer, 0, sizeof(translationBuffer));
                            std::memset(romajiBuffer, 0, sizeof(romajiBuffer));
                            std::memset(exampleSentenceBuffer, 0, sizeof(exampleSentenceBuffer));
                            std::memset(tagBuffer, 0, sizeof(tagBuffer));
                        }
                    }

                    ImGui::NextColumn();

                    // Right Column: List of words
                    ImGui::BeginChild("WordsList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
                    ImGui::Text("Words in this Lesson:");
                    ImGui::Separator();

                    for( size_t index = 0; index < newLesson.words.size(); ++index )
                    {
                        const auto& word = newLesson.words[index];
                        if( ImGui::Selectable(word.kana.c_str()) )
                        {
                            // Handle word selection if needed
                        }

                        if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) )
                        {
                            // Fill the input fields with the selected word's data
                            std::strncpy(kanaBuffer, word.kana.c_str(), sizeof(kanaBuffer));
                            std::strncpy(translationBuffer, word.translation.c_str(), sizeof(translationBuffer));
                            std::strncpy(romajiBuffer, word.romaji.c_str(), sizeof(romajiBuffer));
                            std::strncpy(exampleSentenceBuffer, word.exampleSentence.c_str(), sizeof(exampleSentenceBuffer));

                            // Convert tags to a comma-separated string
                            std::string tagsStr;
                            for( const auto& tag : word.tags )
                            {
                                if( !tagsStr.empty() )
                                    tagsStr += ",";
                                tagsStr += tag;
                            }
                            std::strncpy(tagBuffer, tagsStr.c_str(), sizeof(tagBuffer));
                        }
                    }

                    ImGui::EndChild();

                    ImGui::Columns(1);

                    if( ImGui::Button("Save Lesson") )
                    {
                        if( strlen(mainNameBuffer) > 0 && strlen(subNameBuffer) > 0 && !newLesson.words.empty() )
                        {
                            // Pass the new lesson to the LessonManager
                            // Assuming you have a reference to LessonManager instance
                            // lessonManager.addLesson(newLesson.mainName, newLesson.subName);
                            // for (const auto& word : newLesson.words)
                            // {
                            //     lessonManager.addWordToLesson(newLesson.id, word);
                            // }

                            // Clear the buffers and reset the new lesson
                            newLesson = Lesson(); // Reset new lesson
                            *p_open = false;
                            ImGui::CloseCurrentPopup(); // Close the modal
                        }
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Cancel") )
                    {
                        // Clear the buffers and reset the new lesson
                        std::memset(mainNameBuffer, 0, sizeof(mainNameBuffer));
                        std::memset(subNameBuffer, 0, sizeof(subNameBuffer));
                        std::memset(kanaBuffer, 0, sizeof(kanaBuffer));
                        std::memset(translationBuffer, 0, sizeof(translationBuffer));
                        std::memset(romajiBuffer, 0, sizeof(romajiBuffer));
                        std::memset(exampleSentenceBuffer, 0, sizeof(exampleSentenceBuffer));
                        std::memset(tagBuffer, 0, sizeof(tagBuffer));
                        newLesson = Lesson(); // Reset new lesson
                        *p_open = false;
                        ImGui::CloseCurrentPopup(); // Close the modal
                    }

                    ImGui::EndPopup();
                }

                if( !ImGui::IsPopupOpen("Add New Lesson Modal") )
                {
                    std::memset(mainNameBuffer, 0, sizeof(mainNameBuffer));
                    std::memset(subNameBuffer, 0, sizeof(subNameBuffer));
                    std::memset(kanaBuffer, 0, sizeof(kanaBuffer));
                    std::memset(translationBuffer, 0, sizeof(translationBuffer));
                    std::memset(romajiBuffer, 0, sizeof(romajiBuffer));
                    std::memset(exampleSentenceBuffer, 0, sizeof(exampleSentenceBuffer));
                    std::memset(tagBuffer, 0, sizeof(tagBuffer));
                    newLesson = Lesson(); // Reset new lesson
                }
            }

            void LessonSettingsWidget::setLesson(Lesson& lesson)
            {
                m_lesson = &lesson;
            }
        }
    }
}
