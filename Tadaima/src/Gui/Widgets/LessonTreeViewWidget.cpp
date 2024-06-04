#include "LessonTreeViewWidget.h"
#include "resources/IconsFontAwesome4.h"
#include "imgui.h"
#include <map>
#include <unordered_set>

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            void LessonTreeViewWidget::initialize(const tools::DataPackage& r_package)
            {
                const LessonDataPackage* package = dynamic_cast<const LessonDataPackage*>(&r_package);
                if( package )
                {
                    m_cashedLessons.clear();
                    std::map<std::string, LessonGroup> lessonMap;
                    auto allLessons = package->decode();

                    for( const auto& lesson : allLessons )
                    {
                        lessonMap[lesson.mainName].mainName = lesson.mainName;
                        lessonMap[lesson.mainName].subLessons.push_back(lesson);
                    }

                    for( const auto& pair : lessonMap )
                    {
                        m_cashedLessons.push_back(pair.second);
                    }
                }
            }

            LessonDataPackage LessonTreeViewWidget::createLessonDataPackageFromLessons(const std::vector<Lesson>& lessons)
            {
                // Create a LessonDataPackage with an identifier
                LessonDataPackage lessonDataPackage;

                // Create a vector to hold LessonPackage objects
                std::vector<LessonPackage> lessonPackages;

                for( const auto& lesson : lessons )
                {
                    // Create a new LessonPackage using the existing Lesson object
                    LessonPackage lessonPackage(lesson.id);

                    // Set the lesson details from the existing Lesson object
                    lessonPackage.set(LessonDataKey::id, lesson.id);
                    lessonPackage.set(LessonDataKey::MainName, lesson.mainName);
                    lessonPackage.set(LessonDataKey::SubName, lesson.subName);

                    // Create a vector to hold WordDataPackage objects
                    std::vector<WordDataPackage> wordPackages;

                    // Iterate over the words in the existing Lesson object
                    for( const auto& word : lesson.words )
                    {
                        // Create a new WordDataPackage for each word
                        WordDataPackage wordPackage(word.id);

                        // Set the word details from the existing Word object
                        wordPackage.set(LessonWordDataKey::id, word.id);
                        wordPackage.set(LessonWordDataKey::Kana, word.kana);
                        wordPackage.set(LessonWordDataKey::Translation, word.translation);
                        wordPackage.set(LessonWordDataKey::Romaji, word.romaji);
                        wordPackage.set(LessonWordDataKey::ExampleSentence, word.exampleSentence);
                        wordPackage.set(LessonWordDataKey::Tags, word.tags);

                        // Add the WordDataPackage to the vector of word packages
                        wordPackages.push_back(wordPackage);
                    }

                    // Set the words for the lesson
                    lessonPackage.set(LessonDataKey::Words, wordPackages);

                    // Add the LessonPackage to the vector of lesson packages
                    lessonPackages.push_back(lessonPackage);
                }

                // Set the lessons package in the LessonDataPackage
                lessonDataPackage.set(LessonPackageKey::LessonsPackage, lessonPackages);

                return lessonDataPackage;
            }

            LessonDataPackage LessonTreeViewWidget::createLessonDataPackageFromLesson(const Lesson& lesson)
            {
                // Create a LessonDataPackage with an identifier
                LessonDataPackage lessonDataPackage;

                // Create a vector to hold LessonPackage objects
                std::vector<LessonPackage> lessonPackages;

                // Create a new LessonPackage using the existing Lesson object
                LessonPackage lessonPackage(lesson.id);

                // Set the lesson details from the existing Lesson object
                lessonPackage.set(LessonDataKey::id, lesson.id);
                lessonPackage.set(LessonDataKey::MainName, lesson.mainName);
                lessonPackage.set(LessonDataKey::SubName, lesson.subName);

                // Create a vector to hold WordDataPackage objects
                std::vector<WordDataPackage> wordPackages;

                // Iterate over the words in the existing Lesson object
                for( const auto& word : lesson.words )
                {
                    // Create a new WordDataPackage for each word
                    WordDataPackage wordPackage(word.id);

                    // Set the word details from the existing Word object
                    wordPackage.set(LessonWordDataKey::id, word.id);
                    wordPackage.set(LessonWordDataKey::Kana, word.kana);
                    wordPackage.set(LessonWordDataKey::Translation, word.translation);
                    wordPackage.set(LessonWordDataKey::Romaji, word.romaji);
                    wordPackage.set(LessonWordDataKey::ExampleSentence, word.exampleSentence);
                    wordPackage.set(LessonWordDataKey::Tags, word.tags);

                    // Add the WordDataPackage to the vector of word packages
                    wordPackages.push_back(wordPackage);
                }

                // Set the words for the lesson
                lessonPackage.set(LessonDataKey::Words, wordPackages);

                // Add the LessonPackage to the vector of lesson packages
                lessonPackages.push_back(lessonPackage);

                // Set the lessons package in the LessonDataPackage
                lessonDataPackage.set(LessonPackageKey::LessonsPackage, lessonPackages);

                return lessonDataPackage;
            }

            void LessonTreeViewWidget::draw(bool* p_open)
            {
                static bool open_add_new_lesson = false;
                static bool open_edit_lesson = false;
                static Lesson selectedLesson; // Store the selected lesson for editing
                static Lesson originalLesson; // Store the original lesson for comparison
                static bool renamePopupOpen = false;
                static bool deleteLesson = false;

                bool ctrlPressed = ImGui::GetIO().KeyCtrl; // Check if the Ctrl key is pressed

                if( !ImGui::Begin("Lessons Overview", p_open, ImGuiWindowFlags_NoDecoration) )
                {
                    ImGui::End();
                    return;
                }

                if( ImGui::Button(ICON_FA_PLUS " Add New Lesson") )
                {
                    // Create a new lesson object
                    selectedLesson = Lesson();
                    m_type = LessonPackageType::LessonCreated;
                    open_add_new_lesson = true;
                    m_lessonSettingsWidget.setLesson(selectedLesson);
                }

                if( open_add_new_lesson )
                {
                    m_lessonSettingsWidget.draw(&open_add_new_lesson);
                    if( !open_add_new_lesson )
                    {
                        if( !selectedLesson.isEmpty() )
                        {
                            LessonDataPackage package = createLessonDataPackageFromLesson(selectedLesson);
                            emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonCreated, &package));
                        }
                    }
                }

                bool clickedOutside = ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && !ImGui::IsMouseReleased(1);

                for( size_t groupIndex = 0; groupIndex < m_cashedLessons.size(); groupIndex++ )
                {
                    auto& lessonGroup = m_cashedLessons[groupIndex];
                    ImGui::PushID(static_cast<int>(groupIndex)); // Ensure unique IDs for each group

                    if( ImGui::TreeNode(lessonGroup.mainName.c_str()) )
                    {
                        for( size_t lessonIndex = 0; lessonIndex < lessonGroup.subLessons.size(); lessonIndex++ )
                        {
                            auto& lesson = lessonGroup.subLessons[lessonIndex];
                            ImGui::PushID(static_cast<int>(lessonIndex)); // Ensure unique IDs for each lesson

                            // Determine if the lesson is selected
                            bool isSelected = m_selectedLessons.find(lesson.id) != m_selectedLessons.end();

                            // Change color if selected
                            if( isSelected )
                            {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green color for selected items
                            }

                            // Show lesson item with selection support
                            if( ImGui::TreeNodeEx(lesson.subName.empty() ? lesson.mainName.c_str() : lesson.subName.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | (isSelected ? ImGuiTreeNodeFlags_Selected : 0)) )
                            {
                                for( size_t wordIndex = 0; wordIndex < lesson.words.size(); wordIndex++ )
                                {
                                    const auto& word = lesson.words[wordIndex];
                                    ImGui::PushID(static_cast<int>(wordIndex)); // Ensure unique IDs for each word
                                    ImGui::Text(" %s - %s", word.translation.c_str(), word.kana.c_str());
                                    ImGui::PopID();
                                }
                                ImGui::TreePop();
                            }

                            // Revert color change if applied
                            if( isSelected )
                            {
                                ImGui::PopStyleColor();
                            }

                            if( ImGui::IsItemClicked(0) )
                            {
                                if( ctrlPressed )
                                {
                                    if( isSelected )
                                        m_selectedLessons.erase(lesson.id);
                                    else
                                        m_selectedLessons.insert(lesson.id);
                                }
                                else
                                {
                                    m_selectedLessons.clear();
                                }
                            }

                            if( ImGui::BeginPopupContextItem(("context_" + std::to_string(groupIndex) + "_" + std::to_string(lessonIndex)).c_str()) )
                            {
                                if( ImGui::BeginMenu(ICON_FA_PLAY"Play") )
                                {
                                    if( ImGui::MenuItem("vocabulary quiz") )
                                    {
                                        if( m_selectedLessons.size() > 0 )
                                        {
                                            auto package = createLessonDataPackageFromSelectedNodes(m_selectedLessons);
                                            emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnPlayVocabularyQuiz, &package));
                                        }
                                        else
                                        {
                                            LessonDataPackage package = createLessonDataPackageFromLesson(lesson);
                                            emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnPlayVocabularyQuiz, &package));
                                        }
                                    }

                                    if( ImGui::MenuItem("multiple choice quiz") )
                                    {
                                        if( m_selectedLessons.size() > 0 )
                                        {
                                            auto package = createLessonDataPackageFromSelectedNodes(m_selectedLessons);
                                            emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnPlayMultipleChoiceQuiz, &package));
                                        }
                                        else
                                        {
                                            LessonDataPackage package = createLessonDataPackageFromLesson(lesson);
                                            emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnPlayMultipleChoiceQuiz, &package));
                                        }
                                    }

                                    ImGui::EndMenu();
                                }

                                if( ImGui::MenuItem(ICON_FA_PENCIL "Edit") )
                                {
                                    m_changedLessonGroupIndex = static_cast<int>(groupIndex);
                                    m_changedLessonIndex = static_cast<int>(lessonIndex);
                                    originalLesson = lesson; // Store the original lesson for comparison
                                    selectedLesson = lesson;
                                    open_edit_lesson = true;
                                    m_lessonSettingsWidget.setLesson(selectedLesson);
                                    ImGui::CloseCurrentPopup();
                                }

                                if( ImGui::MenuItem(ICON_FA_PENCIL "Rename") )
                                {
                                    m_changedLessonGroupIndex = static_cast<int>(groupIndex);
                                    m_changedLessonIndex = static_cast<int>(lessonIndex);
                                    strncpy(renameMainNameBuffer, lessonGroup.mainName.c_str(), sizeof(renameMainNameBuffer));
                                    strncpy(renameSubNameBuffer, lesson.subName.c_str(), sizeof(renameSubNameBuffer));
                                    renamePopupOpen = true;
                                    ImGui::CloseCurrentPopup();
                                }
                                if( ImGui::MenuItem(ICON_FA_TRASH " Delete") )
                                {
                                    m_changedLessonGroupIndex = static_cast<int>(groupIndex);
                                    m_changedLessonIndex = static_cast<int>(lessonIndex);
                                    deleteLesson = true;
                                    ImGui::CloseCurrentPopup();
                                }
                                ImGui::EndPopup();
                            }

                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }

                if( clickedOutside )
                {
                    m_selectedLessons.clear();
                }

                if( open_edit_lesson )
                {
                    m_lessonSettingsWidget.draw(&open_edit_lesson);
                    if( !open_edit_lesson )
                    {
                        if( originalLesson != selectedLesson )
                        {
                            LessonDataPackage package = createLessonDataPackageFromLesson(selectedLesson);
                            emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonEdited, &package));
                        }
                    }
                }

                if( renamePopupOpen )
                {
                    ImGui::OpenPopup("Rename Lesson");
                    renamePopupOpen = false;
                }

                if( deleteLesson )
                {
                    if( m_selectedLessons.size() > 0 )
                    {
                        auto package = createLessonDataPackageFromSelectedNodes(m_selectedLessons);
                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonDelete, &package));
                        m_selectedLessons.clear();
                    }
                    else
                    {
                        if( m_changedLessonGroupIndex >= 0 && m_changedLessonGroupIndex < static_cast<int>(m_cashedLessons.size()) )
                        {
                            auto& lessonGroup = m_cashedLessons[m_changedLessonGroupIndex];
                            if( m_changedLessonIndex >= 0 && m_changedLessonIndex < static_cast<int>(lessonGroup.subLessons.size()) )
                            {
                                auto& lesson = lessonGroup.subLessons[m_changedLessonIndex];

                                LessonDataPackage package = createLessonDataPackageFromLesson(lesson);
                                emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonDelete, &package));
                            }
                        }
                    }

                    deleteLesson = false;
                }

                ShowRenamePopup();

                ImGui::End();
            }

           LessonDataPackage LessonTreeViewWidget::createLessonDataPackageFromSelectedNodes(const std::unordered_set<int>& nodes)
           {
               std::unordered_map<int, Lesson> lessonMap;

               for( const auto& lessonGroup : m_cashedLessons )
               {
                   for( const auto& lesson : lessonGroup.subLessons )
                   {
                       lessonMap[lesson.id] = lesson;
                   }
               }

               // Filter out the lessons that match the given node IDs
               std::vector<Lesson> lessons;
               lessons.reserve(nodes.size());  // Reserve space for optimization

               for( int id : nodes )
               {
                   auto it = lessonMap.find(id);
                   if( it != lessonMap.end() )
                   {
                       lessons.push_back(it->second);
                   }
               }

               return createLessonDataPackageFromLessons(lessons);

            }

            void LessonTreeViewWidget::ShowRenamePopup()
            {
                if( ImGui::BeginPopupModal("Rename Lesson", nullptr, ImGuiWindowFlags_AlwaysAutoResize) )
                {
                    ImGui::TextWrapped("Please enter the new names for the lesson:");

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::Text("Main Name:");
                    ImGui::PushItemWidth(300);
                    ImGui::InputText("##MainName", renameMainNameBuffer, sizeof(renameMainNameBuffer));
                    ImGui::PopItemWidth();

                    ImGui::Spacing();

                    ImGui::Text("Sub Name:");
                    ImGui::PushItemWidth(300);
                    ImGui::InputText("##SubName", renameSubNameBuffer, sizeof(renameSubNameBuffer));
                    ImGui::PopItemWidth();

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    if( ImGui::Button("Save", ImVec2(120, 0)) )
                    {
                        if( m_changedLessonGroupIndex >= 0 && m_changedLessonGroupIndex < static_cast<int>(m_cashedLessons.size()) )
                        {
                            auto& lessonGroup = m_cashedLessons[m_changedLessonGroupIndex];
                            if( m_changedLessonIndex >= 0 && m_changedLessonIndex < static_cast<int>(lessonGroup.subLessons.size()) )
                            {
                                auto& lesson = lessonGroup.subLessons[m_changedLessonIndex];
                                if( lesson.mainName != renameMainNameBuffer || lesson.subName != renameSubNameBuffer )
                                {
                                    lessonGroup.mainName = renameMainNameBuffer;
                                    lesson.mainName = renameMainNameBuffer;
                                    lesson.subName = renameSubNameBuffer;

                                    LessonDataPackage package = createLessonDataPackageFromLesson(lesson);
                                    emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonRename, &package));
                                }
                            }
                        }
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Cancel", ImVec2(120, 0)) )
                    {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
            }

        }
    }
}
