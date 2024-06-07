#include "LessonSettingsWidget.h"
#include "imgui.h"
#include <cstring>
#include <iosfwd>
#include <sstream>
#include <stdexcept>
#include "packages/SettingsDataPackage.h"
#include "Tools/Logger.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {

            LessonSettingsWidget::LessonSettingsWidget(tools::Logger& logger)
                : m_logger(logger), m_selectedWordIndex(-1), m_isEditing(false)
            {
                m_logger.log("Initializing LessonSettingsWidget", tools::LogLevel::DEBUG);
                std::memset(m_mainNameBuffer, 0, sizeof(m_mainNameBuffer));
                std::memset(m_subNameBuffer, 0, sizeof(m_subNameBuffer));
                std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
            }

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
                    ImGui::InputText("Translation", m_translationBuffer, sizeof(m_translationBuffer));

                    // Button to translate using the dictionary
                    if( ImGui::Button("Translate") )
                    {
                        m_logger.log("Translating word: " + std::string(m_translationBuffer), tools::LogLevel::INFO);
                        try
                        {
                            Word translatedWord = m_dictionary.getTranslation(m_translationBuffer);
                            std::strncpy(m_romajiBuffer, translatedWord.romaji.c_str(), sizeof(m_romajiBuffer));
                            std::strncpy(m_kanaBuffer, translatedWord.kana.c_str(), sizeof(m_kanaBuffer));
                            m_logger.log("Translated: ");
                            m_logger.log(" ->: " + translatedWord.translation);
                            m_logger.log(" ->: " + translatedWord.romaji);
                            m_logger.log(" ->: " + translatedWord.kana);
                        }
                        catch( const std::exception& e )
                        {
                            m_logger.log(std::string("Translation error: ") + e.what(), tools::LogLevel::PROBLEM);
                        }
                    }

                    ImGui::InputText("Romaji", m_romajiBuffer, sizeof(m_romajiBuffer));
                    ImGui::InputText("Kana", m_kanaBuffer, sizeof(m_kanaBuffer));
                    ImGui::InputText("Example Sentence", m_exampleSentenceBuffer, sizeof(m_exampleSentenceBuffer));
                    ImGui::InputText("Tags (comma-separated)", m_tagBuffer, sizeof(m_tagBuffer));
                    if( ImGui::IsItemHovered() )
                    {
                        ImGui::SetTooltip("Enter tags separated by commas, e.g., tag1,tag2,tag3");
                    }

                    if( ImGui::Button("Add Word") )
                    {
                        m_logger.log("Adding word", tools::LogLevel::INFO);
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

                    if( m_selectedWordIndex >= 0 )
                    {
                        ImGui::SameLine();
                        if( ImGui::Button("Update Word") )
                        {
                            m_logger.log("Updating word at index: " + std::to_string(m_selectedWordIndex), tools::LogLevel::INFO);
                            if( std::strlen(m_translationBuffer) > 0 && std::strlen(m_kanaBuffer) > 0 )
                            {
                                Word updatedWord;
                                updatedWord.kana = std::string(m_kanaBuffer);
                                updatedWord.translation = std::string(m_translationBuffer);
                                updatedWord.romaji = std::string(m_romajiBuffer);
                                updatedWord.exampleSentence = std::string(m_exampleSentenceBuffer);

                                // Split tags by comma
                                std::string tagsStr(m_tagBuffer);
                                std::istringstream iss(tagsStr);
                                std::string tag;
                                while( std::getline(iss, tag, ',') )
                                {
                                    updatedWord.tags.push_back(tag);
                                }

                                m_newLesson.words[m_selectedWordIndex] = updatedWord;
                                m_selectedWordIndex = -1;  // Reset selection after update

                                // Clear the word input fields
                                std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                                std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                                std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                                std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                                std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                            }
                        }

                        ImGui::SameLine();
                        if( ImGui::Button("Remove Word") )
                        {
                            m_logger.log("Removing word at index: " + std::to_string(m_selectedWordIndex), tools::LogLevel::INFO);
                            m_newLesson.words.erase(m_newLesson.words.begin() + m_selectedWordIndex);
                            m_selectedWordIndex = -1; // Reset selection after removal
                            // Clear the word input fields
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
                        ImGui::PushID(static_cast<int>(index));

                        if( ImGui::Selectable(word.kana.c_str(), index == static_cast<size_t>(m_selectedWordIndex)) )
                        {
                            m_logger.log("Selected word at index: " + std::to_string(index), tools::LogLevel::DEBUG);
                            // Fill the input fields with the selected word's data
                            m_selectedWordIndex = static_cast<int>(index);
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

                        ImGui::PopID();
                    }

                    ImGui::EndChild();

                    ImGui::Columns(1);

                    if( ImGui::Button("Save Lesson") )
                    {
                        m_logger.log("Saving lesson", tools::LogLevel::INFO);
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
                        m_logger.log("Canceling lesson settings", tools::LogLevel::INFO);
                        // Clear the buffers and reset the new lesson
                        std::memset(m_mainNameBuffer, 0, sizeof(m_mainNameBuffer));
                        std::memset(m_subNameBuffer, 0, sizeof(m_subNameBuffer));
                        std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                        std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                        std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                        std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                        std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                        m_newLesson = Lesson(); // Reset new lesson
                        m_selectedWordIndex = -1; // Reset selection
                        m_isEditing = false; // Exit edit mode
                        *p_open = false;
                        ImGui::CloseCurrentPopup(); // Close the modal
                    }

                    ImGui::EndPopup();
                }

                ImGui::PopStyleColor();  // Restore previous style

                // Check for clicks outside the selection to reset the selected word index
                if( ImGui::IsMouseDoubleClicked(0) && !ImGui::IsAnyItemHovered() && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) )
                {
                    m_logger.log("Clicked outside selection, resetting selected word index", tools::LogLevel::DEBUG);
                    m_selectedWordIndex = -1;
                    // Clear the word input fields
                    std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                    std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                    std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                    std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                    std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                }

                if( !ImGui::IsPopupOpen("Add New Lesson Modal") && !ImGui::IsPopupOpen("Edit Lesson Modal") )
                {
                    m_logger.log("Closing lesson settings widget", tools::LogLevel::DEBUG);
                    std::memset(m_mainNameBuffer, 0, sizeof(m_mainNameBuffer));
                    std::memset(m_subNameBuffer, 0, sizeof(m_subNameBuffer));
                    std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                    std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                    std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                    std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                    std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                    m_newLesson = Lesson(); // Reset new lesson
                    m_selectedWordIndex = -1; // Reset selection
                }
            }

            void LessonSettingsWidget::setLesson(Lesson& lesson)
            {
                m_logger.log("Setting lesson for editing", tools::LogLevel::INFO);
                m_lesson = &lesson;
                // Populate buffers with the current lesson data for editing
                std::strncpy(m_mainNameBuffer, lesson.mainName.c_str(), sizeof(m_mainNameBuffer));
                std::strncpy(m_subNameBuffer, lesson.subName.c_str(), sizeof(m_subNameBuffer));
                m_newLesson = lesson; // Copy the lesson to the newLesson object for editing
                m_isEditing = true; // Enable edit mode
                m_selectedWordIndex = -1; // Reset selection
            }

            void LessonSettingsWidget::initialize(const tools::DataPackage& r_package)
            {
                m_logger.log("Initializing LessonSettingsWidget with data package", tools::LogLevel::INFO);
                const SettingsDataPackage* package = dynamic_cast<const SettingsDataPackage*>(&r_package);
                if( package )
                {
                    const std::string dictionaryPath = package->get<std::string>(SettingsPackageKey::DictionaryPath);
                    m_dictionary.setPathForTranslator(dictionaryPath);
                    m_logger.log("Dictionary path set to: " + dictionaryPath, tools::LogLevel::DEBUG);
                }
            }

        }
    }
}
