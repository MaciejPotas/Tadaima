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
                    m_cashedLessons.clear();

                    std::unordered_map<std::string, LessonGroup> lessonMap;

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
                static Lesson newLesson;

                if( !ImGui::Begin("Lessons Overview", p_open, ImGuiWindowFlags_NoDecoration) )
                {
                    ImGui::End();
                    return;
                }

                if( ImGui::Button(ICON_FA_PLUS " Add New Lesson") )
                {
                    // Create a new lesson object
                    newLesson = Lesson();
                    m_type = PackageType::LessonCreated;
                    open_add_new_lesson = true;
                    m_lessonSettingsWidget.setLesson(newLesson);
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
                    if( !open_add_new_lesson )
                    {
                        LessonDataPackage package = createLessonDataPackageFromLesson(newLesson);
                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonCreated, &package));
                    }
                }

                static int selected = -1;

                for( const auto& lessonGroup : m_cashedLessons )
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
                                        ImGui::Text(" %s - %s", word.translation.c_str(), word.kana.c_str());
                                    }
                                    ImGui::TreePop();
                                }
                            }
                            else
                            {
                                // If there is no subName, we directly show the words
                                for( const auto& word : lesson.words )
                                {
                                    ImGui::Text(" %s - %s", word.translation.c_str(), word.kana.c_str());
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
