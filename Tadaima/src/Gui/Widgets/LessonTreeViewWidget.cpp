#include "LessonTreeViewWidget.h"
#include "gui.h"
#include "resources/IconsFontAwesome4.h"
#include "imgui.h"
#include <sstream>
#include <unordered_map>

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
                    lessons.clear();

                    std::unordered_map<std::string, LessonGroup> lessonMap;

                    auto lessonPackages = package->get<std::vector<LessonPackage>>(PackageKey::LessonsPackage);
                    for( const auto& lessonPackage : lessonPackages )
                    {
                        Lesson lesson;
                        lesson.mainName = lessonPackage.get<std::string>(LessonDataKey::MainName);
                        lesson.subName = lessonPackage.get<std::string>(LessonDataKey::SubName);

                        auto wordPackages = lessonPackage.get<std::vector<WordDataPackage>>(LessonDataKey::Words);
                        for( const auto& wordPackage : wordPackages )
                        {
                            Word word;
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
                        lessons.push_back(pair.second);
                    }
                }
            }

            void LessonTreeViewWidget::draw(bool* p_open)
            {
                static bool open_add_new_lesson = false;
                if( !ImGui::Begin("Lessons Overview", p_open, ImGuiWindowFlags_NoDecoration) )
                {
                    ImGui::End();
                    return;
                }

                ImGui::Separator();
                if( ImGui::Button(ICON_FA_PLUS " Add New Lesson") )
                {
                    // Logic to add new lesson
                    open_add_new_lesson = true;
                }
                ImGui::SameLine();
                if( ImGui::Button(ICON_FA_PLUS " Edit Lesson") )
                {
                    // Logic to edit a lesson
                }
                ImGui::SameLine();
                if( ImGui::Button(ICON_FA_TRASH " Delete Lesson") )
                {
                    // Logic to delete a lesson
                }

                if( open_add_new_lesson )
                {
                    m_lessonSettingsWidget.draw(&open_add_new_lesson);
                }

                static int selected = -1;

                for( const auto& lessonGroup : lessons )
                {
                    if( ImGui::TreeNode(lessonGroup.mainName.c_str()) )
                    {
                        for( const auto& lesson : lessonGroup.subLessons )
                        {
                            if( !lesson.subName.empty() )
                            {
                                if( ImGui::TreeNode(lesson.subName.c_str()) )
                                {
                                    for( const auto& word : lesson.words )
                                    {
                                        ImGui::Text("  Word: %s (%s) - %s", word.kana.c_str(), word.romaji.c_str(), word.translation.c_str());
                                        ImGui::Text("  Example: %s", word.exampleSentence.c_str());
                                        ImGui::Text("  Tags: ");
                                        for( const auto& tag : word.tags )
                                        {
                                            ImGui::SameLine();
                                            ImGui::Text("%s", tag.c_str());
                                        }
                                    }
                                    ImGui::TreePop();
                                }
                            }
                            else
                            {
                                // If there is no subName, we directly show the words
                                for( const auto& word : lesson.words )
                                {
                                    ImGui::Text("  Word: %s (%s) - %s", word.kana.c_str(), word.romaji.c_str(), word.translation.c_str());
                                    ImGui::Text("  Example: %s", word.exampleSentence.c_str());
                                    ImGui::Text("  Tags: ");
                                    for( const auto& tag : word.tags )
                                    {
                                        ImGui::SameLine();
                                        ImGui::Text("%s", tag.c_str());
                                    }
                                }
                            }
                        }
                        ImGui::TreePop();
                    }
                }

                ImGui::End();
            }
        }
    }
}
