#include "LessonTreeViewWidget.h"
#include "resources/IconsFontAwesome4.h"
#include "imgui.h"
#include <map>

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

                    auto lessonPackages = package->get<std::vector<LessonPackage>>(PackageKey::LessonsPackage);
                    for( const auto& lessonPackage : lessonPackages )
                    {
                        Lesson lesson;
                        lesson.id = lessonPackage.get<int>(LessonDataKey::id);
                        lesson.mainName = lessonPackage.get<std::string>(LessonDataKey::MainName);
                        lesson.subName = lessonPackage.get<std::string>(LessonDataKey::SubName);

                        auto wordPackages = lessonPackage.get<std::vector<WordDataPackage>>(LessonDataKey::Words);
                        for( const auto& wordPackage : wordPackages )
                        {
                            Word word;
                            word.id = wordPackage.get<int>(WordDataKey::id);
                            word.kana = wordPackage.get<std::string>(WordDataKey::Kana);
                            word.translation = wordPackage.get<std::string>(WordDataKey::Translation);
                            word.romaji = wordPackage.get<std::string>(WordDataKey::Romaji);
                            word.exampleSentence = wordPackage.get<std::string>(WordDataKey::ExampleSentence);
                            word.tags = wordPackage.get<std::vector<std::string>>(WordDataKey::Tags);

                            lesson.words.push_back(word);
                        }

                        lessonMap[lesson.mainName].mainName = lesson.mainName;
                        lessonMap[lesson.mainName].subLessons.push_back(lesson);
                    }

                    for( const auto& pair : lessonMap )
                    {
                        m_cashedLessons.push_back(pair.second);
                    }
                }
            }

            LessonTreeViewWidget::LessonDataPackage createLessonDataPackageFromLesson(const Lesson& lesson)
            {
                // Create a LessonDataPackage with an identifier
                LessonTreeViewWidget::LessonDataPackage lessonDataPackage(1);

                // Create a vector to hold LessonPackage objects
                std::vector<LessonTreeViewWidget::LessonPackage> lessonPackages;

                // Create a new LessonPackage using the existing Lesson object
                LessonTreeViewWidget::LessonPackage lessonPackage(lesson.id);

                // Set the lesson details from the existing Lesson object
                lessonPackage.set(LessonTreeViewWidget::LessonDataKey::id, lesson.id);
                lessonPackage.set(LessonTreeViewWidget::LessonDataKey::MainName, lesson.mainName);
                lessonPackage.set(LessonTreeViewWidget::LessonDataKey::SubName, lesson.subName);

                // Create a vector to hold WordDataPackage objects
                std::vector<LessonTreeViewWidget::WordDataPackage> wordPackages;

                // Iterate over the words in the existing Lesson object
                for( const auto& word : lesson.words )
                {
                    // Create a new WordDataPackage for each word
                    LessonTreeViewWidget::WordDataPackage wordPackage(word.id);

                    // Set the word details from the existing Word object
                    wordPackage.set(LessonTreeViewWidget::WordDataKey::Kana, word.kana);
                    wordPackage.set(LessonTreeViewWidget::WordDataKey::Translation, word.translation);
                    wordPackage.set(LessonTreeViewWidget::WordDataKey::Romaji, word.romaji);
                    wordPackage.set(LessonTreeViewWidget::WordDataKey::ExampleSentence, word.exampleSentence);
                    wordPackage.set(LessonTreeViewWidget::WordDataKey::Tags, word.tags);

                    // Add the WordDataPackage to the vector of word packages
                    wordPackages.push_back(wordPackage);
                }

                // Set the words for the lesson
                lessonPackage.set(LessonTreeViewWidget::LessonDataKey::Words, wordPackages);

                // Add the LessonPackage to the vector of lesson packages
                lessonPackages.push_back(lessonPackage);

                // Set the lessons package in the LessonDataPackage
                lessonDataPackage.set(LessonTreeViewWidget::PackageKey::LessonsPackage, lessonPackages);

                return lessonDataPackage;
            }

            void LessonTreeViewWidget::draw(bool* p_open)
            {
                static bool open_add_new_lesson = false;
                static bool open_edit_lesson = false;
                //static Lesson newLesson; // Store the new lesson
                static Lesson selectedLesson; // Store the selected lesson for editing
                static Lesson originalLesson; // Store the original lesson for comparison
                static bool renamePopupOpen = false;
                static bool deleteLesson = false;

                if( !ImGui::Begin("Lessons Overview", p_open, ImGuiWindowFlags_NoDecoration) )
                {
                    ImGui::End();
                    return;
                }

                if( ImGui::Button(ICON_FA_PLUS " Add New Lesson") )
                {
                    // Create a new lesson object
                    selectedLesson = Lesson();
                    m_type = PackageType::LessonCreated;
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

                            if( !lesson.subName.empty() )
                            {
                                if( ImGui::TreeNode(lesson.subName.c_str()) )
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
                            }
                            else
                            {
                                // If there is no subName, we directly show the words
                                for( size_t wordIndex = 0; wordIndex < lesson.words.size(); wordIndex++ )
                                {
                                    const auto& word = lesson.words[wordIndex];
                                    ImGui::PushID(static_cast<int>(wordIndex)); // Ensure unique IDs for each word
                                    ImGui::Text(" %s - %s", word.translation.c_str(), word.kana.c_str());
                                    ImGui::PopID();
                                }
                            }

                            if( ImGui::IsItemClicked() )
                            {
                                selectedLesson = lesson;
                                m_selectedLessonIndex = static_cast<int>(lessonIndex);
                            }

                            if( ImGui::BeginPopupContextItem(("context_" + std::to_string(groupIndex) + "_" + std::to_string(lessonIndex)).c_str()) )
                            {
                                if( ImGui::MenuItem("Edit") )
                                {
                                    m_changedLessonGroupIndex = static_cast<int>(groupIndex);
                                    m_changedLessonIndex = static_cast<int>(lessonIndex);
                                    originalLesson = lesson; // Store the original lesson for comparison
                                    selectedLesson = lesson;
                                    open_edit_lesson = true;
                                    m_lessonSettingsWidget.setLesson(selectedLesson);
                                    ImGui::CloseCurrentPopup();
                                }
                                if( ImGui::MenuItem("Rename") )
                                {
                                    m_changedLessonGroupIndex = static_cast<int>(groupIndex);
                                    m_changedLessonIndex = static_cast<int>(lessonIndex);
                                    strncpy(renameMainNameBuffer, lessonGroup.mainName.c_str(), sizeof(renameMainNameBuffer));
                                    strncpy(renameSubNameBuffer, lesson.subName.c_str(), sizeof(renameSubNameBuffer));
                                    renamePopupOpen = true;
                                    ImGui::CloseCurrentPopup();
                                }
                                if( ImGui::MenuItem("Delete") )
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

                    deleteLesson = false;
                }

                ShowRenamePopup();

                ImGui::End();
            }

            void LessonTreeViewWidget::ShowRenamePopup()
            {
                if( ImGui::BeginPopupModal("Rename Lesson", nullptr, ImGuiWindowFlags_AlwaysAutoResize) )
                {
                    ImGui::InputText("Main Name", renameMainNameBuffer, sizeof(renameMainNameBuffer));
                    ImGui::InputText("Sub Name", renameSubNameBuffer, sizeof(renameSubNameBuffer));

                    if( ImGui::Button("Save") )
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
                    if( ImGui::Button("Cancel") )
                    {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
            }
        }
    }
}
