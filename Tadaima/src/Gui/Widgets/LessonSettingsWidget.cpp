#pragma once

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
                m_logger.log("Initializing LessonSettingsWidget", tools::LogLevel::INFO);
                std::memset(m_mainNameBuffer, 0, sizeof(m_mainNameBuffer));
                std::memset(m_subNameBuffer, 0, sizeof(m_subNameBuffer));
                std::memset(m_groupNameBuffer, 0, sizeof(m_groupNameBuffer)); // Added groupName buffer initialization
                std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                std::memset(m_kanjiBuffer, 0, sizeof(m_kanjiBuffer));
                clearConjugationBuffers(m_conjugationBuffers);
            }

            void LessonSettingsWidget::draw(bool* p_open)
            {
                if( *p_open )
                {
                    ImGui::OpenPopup(m_isEditing ? "Edit Lesson Modal" : "Add New Lesson Modal");
                }

                ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_Always);  // Adjust size as needed
                ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.98f, 0.92f, 0.84f, 1.0f)); // Light peach background
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10)); // Add padding

                // Open the modal window
                if( ImGui::BeginPopupModal(m_isEditing ? "Edit Lesson Modal" : "Add New Lesson Modal", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize) )
                {
                    // Header section
                    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), m_isEditing ? "Edit Lesson" : "Add New Lesson");
                    ImGui::Separator();
                    ImGui::Text("Use this form to add or edit lessons. Fill out the fields below and manage the words in the lesson.");
                    ImGui::Spacing();

                    // Input fields for lesson group name
                    ImGui::InputText("##GroupName", m_groupNameBuffer, sizeof(m_groupNameBuffer));
                    ImGui::SameLine();
                    ImGui::Text("Group Name");

                    // Input fields for lesson name
                    ImGui::InputText("##MainName", m_mainNameBuffer, sizeof(m_mainNameBuffer));
                    ImGui::SameLine();
                    ImGui::Text("Main Name");

                    ImGui::InputText("##SubName", m_subNameBuffer, sizeof(m_subNameBuffer));
                    ImGui::SameLine();
                    ImGui::Text("Sub Name");

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::Columns(2);

                    // Left Column: Word input fields
                    ImGui::TextColored(ImVec4(0.2f, 0.4f, 0.8f, 1.0f), "Add/Edit Word");
                    ImGui::Spacing();

                    ImGui::InputText("##Translation", m_translationBuffer, sizeof(m_translationBuffer));
                    ImGui::SameLine();
                    ImGui::Text("Translation");

                    ImGui::Spacing();

                    // Button to translate using the dictionary
                    if( ImGui::Button("Translate") )
                    {
                        m_logger.log("Translating word: " + std::string(m_translationBuffer), tools::LogLevel::INFO);
                        try
                        {
                            Word translatedWord = m_dictionary.getTranslation(m_translationBuffer);
                            std::strncpy(m_translationBuffer, translatedWord.translation.c_str(), sizeof(m_translationBuffer));
                            std::strncpy(m_romajiBuffer, translatedWord.romaji.c_str(), sizeof(m_romajiBuffer));
                            std::strncpy(m_kanaBuffer, translatedWord.kana.c_str(), sizeof(m_kanaBuffer));
                            std::strncpy(m_kanjiBuffer, translatedWord.kanji.c_str(), sizeof(m_kanjiBuffer)); // Set the kanji
                            std::strncpy(m_exampleSentenceBuffer, translatedWord.exampleSentence.c_str(), sizeof(m_exampleSentenceBuffer)); // Set the kanji
                            m_logger.log("Translated: ");
                            m_logger.log(" ->: " + translatedWord.translation);
                            m_logger.log(" ->: " + translatedWord.romaji);
                            m_logger.log(" ->: " + translatedWord.kana);
                            m_logger.log(" ->: " + translatedWord.kanji);
                            m_logger.log(" ->: " + translatedWord.exampleSentence);
                        }
                        catch( const std::exception& e )
                        {
                            m_logger.log(std::string("Translation error: ") + e.what(), tools::LogLevel::PROBLEM);
                        }
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Conjugations") )
                    {
                        ImGui::OpenPopup("Conjugations Modal");
                    }

                    drawConjugationsModal();

                    ImGui::Spacing();

                    ImGui::InputText("##Romaji", m_romajiBuffer, sizeof(m_romajiBuffer));
                    ImGui::SameLine();
                    ImGui::Text("Romaji");

                    ImGui::InputText("##Kana", m_kanaBuffer, sizeof(m_kanaBuffer));
                    ImGui::SameLine();
                    ImGui::Text("Kana");

                    // Add Kanji Input Field
                    ImGui::InputText("##Kanji", m_kanjiBuffer, sizeof(m_kanjiBuffer));
                    ImGui::SameLine();
                    ImGui::Text("Kanji");

                    // Example Sentence with Tooltip
                    ImGui::InputText("##ExampleSentence", m_exampleSentenceBuffer, sizeof(m_exampleSentenceBuffer));
                    ImGui::SameLine();
                    ImGui::Text("Example Sentence");
                    if( ImGui::IsItemHovered() )
                    {
                        ImGui::SetTooltip("%s", m_exampleSentenceBuffer);
                    }

                    ImGui::InputText("##Tags", m_tagBuffer, sizeof(m_tagBuffer));
                    ImGui::SameLine();
                    ImGui::Text("Tags (comma-separated)");
                    if( ImGui::IsItemHovered() )
                    {
                        ImGui::SetTooltip("Enter tags separated by commas, e.g., tag1,tag2,tag3");
                    }

                    ImGui::Spacing();
                    ImGui::Spacing();

                    if( ImGui::Button("Add Word") )
                    {
                        m_logger.log("Adding word", tools::LogLevel::INFO);
                        // Ensure required fields are set
                        if( std::strlen(m_translationBuffer) > 0 && std::strlen(m_kanaBuffer) > 0 )
                        {
                            m_newWord.kana = std::string(m_kanaBuffer);
                            m_newWord.kanji = std::string(m_kanjiBuffer);
                            m_newWord.translation = std::string(m_translationBuffer);
                            m_newWord.romaji = std::string(m_romajiBuffer);
                            m_newWord.exampleSentence = std::string(m_exampleSentenceBuffer);
                            //m_newWord.conjugations = m_conjugation;

                            // Split tags by comma
                            std::string tagsStr(m_tagBuffer);
                            std::istringstream iss(tagsStr);
                            std::string tag;
                            while( std::getline(iss, tag, ',') )
                            {
                                m_newWord.tags.push_back(tag);
                            }

                            for( int i = 0; i < CONJUGATION_COUNT; ++i )
                            {
                                m_newWord.conjugations[i] = std::string(m_conjugationBuffers[i].data());
                            }

                            m_newLesson.groupName = std::string(m_groupNameBuffer);
                            m_newLesson.mainName = std::string(m_mainNameBuffer);
                            m_newLesson.subName = std::string(m_subNameBuffer);

                            m_newLesson.words.push_back(m_newWord);

                            // Clear the word input fields for the next word
                            std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                            std::memset(m_kanjiBuffer, 0, sizeof(m_kanjiBuffer));
                            std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                            std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                            std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                            std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                            clearConjugationBuffers(m_conjugationBuffers);
                            m_newWord.clear();
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
                                updatedWord.kanji = std::string(m_kanjiBuffer);
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

                                for( int i = 0; i < CONJUGATION_COUNT; ++i )
                                {
                                    updatedWord.conjugations[i] = std::string(m_conjugationBuffers[i].data());
                                }

                                m_newLesson.words[m_selectedWordIndex] = updatedWord;
                                m_selectedWordIndex = -1;  // Reset selection after update

                                // Clear the word input fields
                                std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                                std::memset(m_kanjiBuffer, 0, sizeof(m_kanjiBuffer));
                                std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                                std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                                std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                                std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                                clearConjugationBuffers(m_conjugationBuffers);
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
                            std::memset(m_kanjiBuffer, 0, sizeof(m_kanjiBuffer));
                            std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                            std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                            std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                            std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                            clearConjugationBuffers(m_conjugationBuffers);
                        }
                    }

                    ImGui::NextColumn();

                    // Right Column: List of words
                    ImGui::TextColored(ImVec4(0.2f, 0.4f, 0.8f, 1.0f), "Words in this Lesson");
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::BeginChild("WordsList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 40));
                    for( size_t index = 0; index < m_newLesson.words.size(); ++index )
                    {
                        const auto& word = m_newLesson.words[index];
                        ImGui::PushID(static_cast<int>(index));

                        if( ImGui::Selectable(word.translation.c_str(), index == static_cast<size_t>(m_selectedWordIndex)) )
                        {
                            m_logger.log("Selected word at index: " + std::to_string(index), tools::LogLevel::DEBUG);
                            // Fill the input fields with the selected word's data
                            m_selectedWordIndex = static_cast<int>(index);
                            std::strncpy(m_kanaBuffer, word.kana.c_str(), sizeof(m_kanaBuffer));
                            std::strncpy(m_kanjiBuffer, word.kanji.c_str(), sizeof(m_kanjiBuffer));
                            std::strncpy(m_translationBuffer, word.translation.c_str(), sizeof(m_translationBuffer));
                            std::strncpy(m_romajiBuffer, word.romaji.c_str(), sizeof(m_romajiBuffer));
                            std::strncpy(m_exampleSentenceBuffer, word.exampleSentence.c_str(), sizeof(m_exampleSentenceBuffer));

                            for( size_t i = 0; i < CONJUGATION_COUNT; ++i )
                            {
                                std::strncpy(m_conjugationBuffers[i].data(), word.conjugations[i].c_str(), m_conjugationBuffers[i].size() - 1);
                                m_conjugationBuffers[i][m_conjugationBuffers[i].size() - 1] = '\0'; 
                            }

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

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    if( ImGui::Button("Save Lesson", ImVec2(120, 0)) )
                    {
                        m_logger.log("Saving lesson", tools::LogLevel::INFO);
                        if( strlen(m_groupNameBuffer) > 0 && strlen(m_mainNameBuffer) > 0 && strlen(m_subNameBuffer) > 0 && !m_newLesson.words.empty() )
                        {
                            m_lesson->groupName = std::string(m_groupNameBuffer);
                            m_lesson->mainName = std::string(m_mainNameBuffer);
                            m_lesson->subName = std::string(m_subNameBuffer);
                            m_lesson->words = m_newLesson.words;
                            *p_open = false;
                            ImGui::CloseCurrentPopup(); // Close the modal
                        }
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Cancel", ImVec2(120, 0)) )
                    {
                        m_logger.log("Canceling lesson settings", tools::LogLevel::INFO);
                        // Clear the buffers and reset the new lesson
                        std::memset(m_groupNameBuffer, 0, sizeof(m_groupNameBuffer));
                        std::memset(m_mainNameBuffer, 0, sizeof(m_mainNameBuffer));
                        std::memset(m_subNameBuffer, 0, sizeof(m_subNameBuffer));
                        std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                        std::memset(m_kanjiBuffer, 0, sizeof(m_kanjiBuffer));
                        std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                        std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                        std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                        std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                        clearConjugationBuffers(m_conjugationBuffers);
                        m_newLesson = Lesson(); // Reset new lesson
                        m_selectedWordIndex = -1; // Reset selection
                        m_isEditing = false; // Exit edit mode
                        *p_open = false;
                        ImGui::CloseCurrentPopup(); // Close the modal
                    }

                    ImGui::EndPopup();
                }

                ImGui::PopStyleColor();  // Restore previous style
                ImGui::PopStyleVar();  // Restore previous padding

                // Check for clicks outside the selection to reset the selected word index
                if( ImGui::IsMouseDoubleClicked(0) && !ImGui::IsAnyItemHovered() && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) )
                {
                    m_logger.log("Clicked outside selection, resetting selected word index", tools::LogLevel::DEBUG);
                    m_selectedWordIndex = -1;
                    // Clear the word input fields
                    std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                    std::memset(m_kanjiBuffer, 0, sizeof(m_kanjiBuffer));
                    std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                    std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                    std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                    std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                    clearConjugationBuffers(m_conjugationBuffers);
                }

                if( !ImGui::IsPopupOpen("Add New Lesson Modal") && !ImGui::IsPopupOpen("Edit Lesson Modal") )
                {
                    m_logger.log("Closing lesson settings widget", tools::LogLevel::DEBUG);
                    std::memset(m_groupNameBuffer, 0, sizeof(m_groupNameBuffer));
                    std::memset(m_mainNameBuffer, 0, sizeof(m_mainNameBuffer));
                    std::memset(m_subNameBuffer, 0, sizeof(m_subNameBuffer));
                    std::memset(m_kanaBuffer, 0, sizeof(m_kanaBuffer));
                    std::memset(m_kanjiBuffer, 0, sizeof(m_kanjiBuffer));
                    std::memset(m_translationBuffer, 0, sizeof(m_translationBuffer));
                    std::memset(m_romajiBuffer, 0, sizeof(m_romajiBuffer));
                    std::memset(m_exampleSentenceBuffer, 0, sizeof(m_exampleSentenceBuffer));
                    std::memset(m_tagBuffer, 0, sizeof(m_tagBuffer));
                    clearConjugationBuffers(m_conjugationBuffers);
                    m_newLesson = Lesson(); // Reset new lesson
                    m_selectedWordIndex = -1; // Reset selection
                }
            }

            void LessonSettingsWidget::setLesson(Lesson& lesson)
            {
                m_logger.log("Setting lesson for editing", tools::LogLevel::INFO);
                m_lesson = &lesson;
                // Populate buffers with the current lesson data for editing
                std::strncpy(m_groupNameBuffer, lesson.groupName.c_str(), sizeof(m_groupNameBuffer));
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
                    const std::string conjugationPath = package->get<std::string>(SettingsPackageKey::ConjugationPath);
                    m_dictionary.setPathForTranslator(dictionaryPath);
                    m_dictionary.setPathForConjugationTranslator(conjugationPath);
                    m_logger.log("Dictionary path set to: " + dictionaryPath, tools::LogLevel::INFO);
                    m_logger.log("Conjugation path set to: " + conjugationPath, tools::LogLevel::INFO);
                }
            }

            void LessonSettingsWidget::drawConjugationsModal()
            {
                ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));

                // Disable scrolling for the popup window
                if( ImGui::BeginPopupModal("Conjugations Modal", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse) )
                {
                    std::string title = "Conjugations for ";
                    title += m_romajiBuffer;
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.4f, 1.0f), title.c_str());
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Autofill button
                    if( ImGui::Button("Autofill Conjugations") )
                    {
                        m_logger.log("Autofilling conjugations for word: " + std::string(m_romajiBuffer), tools::LogLevel::INFO);

                        try
                        {
                            auto conjugations = m_dictionary.getConjugations(std::string(m_romajiBuffer));

                            // Update buffers with fetched conjugations
                            for( int i = 0; i < CONJUGATION_COUNT; ++i )
                            {
                                std::strncpy(m_conjugationBuffers[i].data(), conjugations[i].c_str(), m_conjugationBuffers[i].size() - 1);
                                m_conjugationBuffers[i][m_conjugationBuffers[i].size() - 1] = '\0';
                            }

                            m_logger.log("Conjugations autofilled successfully.", tools::LogLevel::INFO);
                        }
                        catch( const std::exception& e )
                        {
                            m_logger.log(std::string("Error while autofilling conjugations: ") + e.what(), tools::LogLevel::PROBLEM);
                        }
                    }

                    ImGui::Spacing();

                    // Begin a child region for the scrollable table
                    ImGui::BeginChild("ConjugationsTableRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 10), true, ImGuiWindowFlags_HorizontalScrollbar);

                    if( ImGui::BeginTable("ConjugationsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable) )
                    {
                        ImGui::TableSetupColumn("Conjugation Type", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn("Conjugated Form", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableHeadersRow();

                        for( int i = 0; i < CONJUGATION_COUNT; ++i )
                        {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            std::string name = ConjugationTypeToString((ConjugationType)i);
                            ImGui::Text(name.c_str());

                            ImGui::TableNextColumn();
                            ImGui::PushID(i);
                            ImGui::InputText("##ConjugationInput", m_conjugationBuffers[i].data(), sizeof(m_conjugationBuffers[i]));
                            ImGui::PopID();
                        }

                        ImGui::EndTable();
                    }

                    ImGui::EndChild(); // End the scrollable child region

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Keep the Save and Cancel buttons at the bottom
                    if( ImGui::Button("Save", ImVec2(100, 0)) )
                    {
                        m_logger.log("Saving conjugations for current word", tools::LogLevel::INFO);
                        for( int i = 0; i < CONJUGATION_COUNT; ++i )
                        {
                            m_newWord.conjugations[i] = std::string(m_conjugationBuffers[i].data());
                        }
                        ImGui::CloseCurrentPopup();
                        m_isConjugating = false;
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Cancel", ImVec2(100, 0)) )
                    {
                        m_logger.log("Cancelling conjugations modal", tools::LogLevel::INFO);
                        ImGui::CloseCurrentPopup();
                        m_isConjugating = false;
                    }

                    ImGui::EndPopup();
                }

                ImGui::PopStyleVar();
            }

            void LessonSettingsWidget::clearConjugationBuffers(std::array<std::array<char, 128>, CONJUGATION_COUNT>& buffers)
            {
                for( auto& innerArray : buffers )
                {
                    for( char& c : innerArray )
                    {
                        c = '\0'; // Reset each character to null terminator
                    }
                }
            }

        }
    }
}
