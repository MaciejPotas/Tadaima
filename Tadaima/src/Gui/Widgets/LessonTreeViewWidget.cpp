#include "LessonTreeViewWidget.h"
#include "resources/IconsFontAwesome4.h"
#include "imgui.h"
#include <map>
#include <unordered_set>
#include "ImGuiFileDialog.h"
#include "Tools/pugixml.hpp"
#include "Tools/Logger.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            LessonTreeViewWidget::LessonTreeViewWidget(tools::Logger& logger)
                : Widget(Type::LessonTreeView), m_lessonSettingsWidget(logger), m_logger(logger)
            {
                m_logger.log("LessonTreeViewWidget created.");
            }

            void LessonTreeViewWidget::initialize(const tools::DataPackage& r_package)
            {
                m_logger.log("Initializing LessonTreeViewWidget.");
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

                m_lessonSettingsWidget.initialize(r_package);
                m_logger.log("LessonTreeViewWidget initialized.");
            }

            LessonDataPackage LessonTreeViewWidget::createLessonDataPackageFromLessons(const std::vector<Lesson>& lessons)
            {
                m_logger.log("Creating LessonDataPackage from lessons.");
                LessonDataPackage lessonDataPackage;
                std::vector<LessonPackage> lessonPackages;

                for( const auto& lesson : lessons )
                {
                    LessonPackage lessonPackage(lesson.id);
                    lessonPackage.set(LessonDataKey::id, lesson.id);
                    lessonPackage.set(LessonDataKey::MainName, lesson.mainName);
                    lessonPackage.set(LessonDataKey::SubName, lesson.subName);
                    std::vector<WordDataPackage> wordPackages;

                    for( const auto& word : lesson.words )
                    {
                        WordDataPackage wordPackage(word.id);
                        wordPackage.set(LessonWordDataKey::id, word.id);
                        wordPackage.set(LessonWordDataKey::Kana, word.kana);
                        wordPackage.set(LessonWordDataKey::Translation, word.translation);
                        wordPackage.set(LessonWordDataKey::Romaji, word.romaji);
                        wordPackage.set(LessonWordDataKey::ExampleSentence, word.exampleSentence);
                        wordPackage.set(LessonWordDataKey::Tags, word.tags);
                        wordPackages.push_back(wordPackage);
                    }

                    lessonPackage.set(LessonDataKey::Words, wordPackages);
                    lessonPackages.push_back(lessonPackage);
                }

                lessonDataPackage.set(LessonPackageKey::LessonsPackage, lessonPackages);
                m_logger.log("LessonDataPackage created from lessons.");
                return lessonDataPackage;
            }

            LessonDataPackage LessonTreeViewWidget::createLessonDataPackageFromLesson(const Lesson& lesson)
            {
                m_logger.log("Creating LessonDataPackage from a single lesson.");
                LessonDataPackage lessonDataPackage;
                std::vector<LessonPackage> lessonPackages;
                LessonPackage lessonPackage(lesson.id);
                lessonPackage.set(LessonDataKey::id, lesson.id);
                lessonPackage.set(LessonDataKey::MainName, lesson.mainName);
                lessonPackage.set(LessonDataKey::SubName, lesson.subName);
                std::vector<WordDataPackage> wordPackages;

                for( const auto& word : lesson.words )
                {
                    WordDataPackage wordPackage(word.id);
                    wordPackage.set(LessonWordDataKey::id, word.id);
                    wordPackage.set(LessonWordDataKey::Kana, word.kana);
                    wordPackage.set(LessonWordDataKey::Translation, word.translation);
                    wordPackage.set(LessonWordDataKey::Romaji, word.romaji);
                    wordPackage.set(LessonWordDataKey::ExampleSentence, word.exampleSentence);
                    wordPackage.set(LessonWordDataKey::Tags, word.tags);
                    wordPackages.push_back(wordPackage);
                }

                lessonPackage.set(LessonDataKey::Words, wordPackages);
                lessonPackages.push_back(lessonPackage);
                lessonDataPackage.set(LessonPackageKey::LessonsPackage, lessonPackages);
                m_logger.log("LessonDataPackage created from a single lesson.");
                return lessonDataPackage;
            }

            void LessonTreeViewWidget::draw(bool* p_open)
            {
                static bool open_add_new_lesson = false;
                static bool open_edit_lesson = false;
                static Lesson selectedLesson;
                static Lesson originalLesson;
                static bool renamePopupOpen = false;
                static bool deleteLesson = false;
                static std::unordered_set<int> lessonsToExport;
                bool ctrlPressed = ImGui::GetIO().KeyCtrl;

                if( !ImGui::Begin("Lessons Overview", p_open, ImGuiWindowFlags_NoDecoration) )
                {
                    ImGui::End();
                    return;
                }

                if( ImGui::Button(ICON_FA_PLUS " Create") )
                {
                    m_logger.log("Create button clicked.");
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
                            m_logger.log("New lesson created.");
                        }
                    }
                }

                ImGui::SameLine();
                if( ImGui::Button(ICON_FA_PLUS " Import") )
                {
                    m_logger.log("Import button clicked.");
                    IGFD::FileDialogConfig config;
                    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Always);
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".xml", config);
                }

                if( ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey") )
                {
                    if( ImGuiFileDialog::Instance()->IsOk() )
                    {
                        std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                        m_logger.log("File selected: " + filePath);
                        auto lessons = parseLessons(filePath);
                        auto package = createLessonDataPackageFromLessons(lessons);
                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonCreated, &package));
                        m_logger.log("New lesson imported.");
                    }
                    ImGuiFileDialog::Instance()->Close();
                }

                bool clickedOutside = ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && !ImGui::IsMouseReleased(1);

                for( size_t groupIndex = 0; groupIndex < m_cashedLessons.size(); groupIndex++ )
                {
                    auto& lessonGroup = m_cashedLessons[groupIndex];
                    ImGui::PushID(static_cast<int>(groupIndex));

                    if( ImGui::TreeNode(lessonGroup.mainName.c_str()) )
                    {
                        for( size_t lessonIndex = 0; lessonIndex < lessonGroup.subLessons.size(); lessonIndex++ )
                        {
                            auto& lesson = lessonGroup.subLessons[lessonIndex];
                            ImGui::PushID(static_cast<int>(lessonIndex));
                            bool isSelected = m_selectedLessons.find(lesson.id) != m_selectedLessons.end();

                            if( isSelected )
                            {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                            }

                            if( ImGui::TreeNodeEx(lesson.subName.empty() ? lesson.mainName.c_str() : lesson.subName.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | (isSelected ? ImGuiTreeNodeFlags_Selected : 0)) )
                            {
                                for( size_t wordIndex = 0; wordIndex < lesson.words.size(); wordIndex++ )
                                {
                                    const auto& word = lesson.words[wordIndex];
                                    ImGui::PushID(static_cast<int>(wordIndex));
                                    ImGui::Text(" %s - %s", word.translation.c_str(), word.kana.c_str());
                                    ImGui::PopID();
                                }
                                ImGui::TreePop();
                            }

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
                                if( ImGui::BeginMenu(ICON_FA_PLAY "Play") )
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
                                    m_logger.log("Edit lesson selected.");
                                    m_changedLessonGroupIndex = static_cast<int>(groupIndex);
                                    m_changedLessonIndex = static_cast<int>(lessonIndex);
                                    originalLesson = lesson;
                                    selectedLesson = lesson;
                                    open_edit_lesson = true;
                                    m_lessonSettingsWidget.setLesson(selectedLesson);
                                    ImGui::CloseCurrentPopup();
                                }

                                if( ImGui::MenuItem(ICON_FA_PENCIL "Rename") )
                                {
                                    m_logger.log("Rename lesson selected.");
                                    m_changedLessonGroupIndex = static_cast<int>(groupIndex);
                                    m_changedLessonIndex = static_cast<int>(lessonIndex);
                                    strncpy(renameMainNameBuffer, lessonGroup.mainName.c_str(), sizeof(renameMainNameBuffer));
                                    strncpy(renameSubNameBuffer, lesson.subName.c_str(), sizeof(renameSubNameBuffer));
                                    renamePopupOpen = true;
                                    ImGui::CloseCurrentPopup();
                                }
                                if( ImGui::MenuItem(ICON_FA_TRASH " Delete") )
                                {
                                    m_logger.log("Delete lesson selected.");
                                    m_changedLessonGroupIndex = static_cast<int>(groupIndex);
                                    m_changedLessonIndex = static_cast<int>(lessonIndex);
                                    deleteLesson = true;
                                    ImGui::CloseCurrentPopup();
                                }

                                if( ImGui::MenuItem(ICON_FA_ARROW_RIGHT " Export") )
                                {
                                    m_logger.log("Export lesson selected.");
                                    lessonsToExport.clear();
                                    lessonsToExport.insert(lesson.id);
                                    ImGui::CloseCurrentPopup();
                                    IGFD::FileDialogConfig config;
                                    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Always);
                                    ImGuiFileDialog::Instance()->OpenDialog("SaveFileDlgKey", "Save File", ".xml", config);
                                }

                                ImGui::EndPopup();
                            }

                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }

                if( ImGuiFileDialog::Instance()->Display("SaveFileDlgKey") )
                {
                    if( ImGuiFileDialog::Instance()->IsOk() )
                    {
                        std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                        m_logger.log("Save file selected: " + filePath);
                        parseAndExportLessons(filePath, m_selectedLessons);
                    }
                    ImGuiFileDialog::Instance()->Close();
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
                            m_logger.log("Lesson edited.");
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
                        m_logger.log("Selected lessons deleted.");
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
                                m_logger.log("Lesson deleted: " + lesson.mainName + " - " + lesson.subName);
                            }
                        }
                    }

                    deleteLesson = false;
                }

                ShowRenamePopup();
                ImGui::End();
            }

            std::vector<Lesson> LessonTreeViewWidget::parseLessons(const std::string& filePath)
            {
                m_logger.log("Parsing and importing lessons from file: " + filePath);
                pugi::xml_document doc;
                pugi::xml_parse_result result = doc.load_file(filePath.c_str());
                std::vector<Lesson> parsedLessons;

                if( !result )
                {
                    m_logger.log("Error: Could not load XML file!");
                    parsedLessons;
                }

                std::map<std::string, LessonGroup> lessonMap;

                for( pugi::xml_node lessonNode : doc.child("lessons").children("lesson") )
                {
                    Lesson lesson;
                    std::string lessonName = lessonNode.attribute("name").as_string();
                    LessonGroup lessonGroup;
                    lessonGroup.mainName = lessonName;

                    for( pugi::xml_node subnameNode : lessonNode.children("subname") )
                    {

                        lesson.mainName = lessonName;
                        lesson.subName = subnameNode.attribute("name").as_string();

                        bool lessonExists = false;
                        for( auto& existingGroup : m_cashedLessons )
                        {
                            if( existingGroup.mainName == lesson.mainName )
                            {
                                for( const auto& existingLesson : existingGroup.subLessons )
                                {
                                    if( existingLesson.subName == lesson.subName )
                                    {
                                        lessonExists = true;
                                        break;
                                    }
                                }
                                if( !lessonExists )
                                {
                                    for( pugi::xml_node wordNode : subnameNode.children("word") )
                                    {
                                        Word word;
                                        word.translation = wordNode.attribute("translation").as_string();
                                        word.romaji = wordNode.attribute("romaji").as_string();
                                        word.kana = wordNode.attribute("kana").as_string();
                                        lesson.words.push_back(word);
                                    }
                                    existingGroup.subLessons.push_back(lesson);
                                }

                                parsedLessons.push_back(lesson);
                                lessonExists = true; // To ensure no duplicate groups are added
                                break;
                            }
                        }

                        if( !lessonExists )
                        {
                            for( pugi::xml_node wordNode : subnameNode.children("word") )
                            {
                                Word word;
                                word.translation = wordNode.attribute("translation").as_string();
                                word.romaji = wordNode.attribute("romaji").as_string();
                                word.kana = wordNode.attribute("kana").as_string();
                                lesson.words.push_back(word);
                            }

                            lessonGroup.subLessons.push_back(lesson);
                        }
                    }

                    if( !lessonGroup.subLessons.empty() )
                    {
                        lessonMap[lessonName] = lessonGroup;
                    }
                }

                for( const auto& pair : lessonMap )
                {
                    bool groupExists = false;
                    for( auto& existingGroup : m_cashedLessons )
                    {
                        if( existingGroup.mainName == pair.first )
                        {
                            existingGroup.subLessons.insert(existingGroup.subLessons.end(), pair.second.subLessons.begin(), pair.second.subLessons.end());
                            groupExists = true;
                            break;
                        }
                    }

                    if( !groupExists )
                    {
                        Lesson lesson;
                        lesson.mainName = pair.second.mainName;
                        lesson.subName = pair.second.subLessons[0].subName;
                        lesson.words = pair.second.subLessons[0].words;

                        parsedLessons.push_back(lesson);
                    }
                }

                m_logger.log("Lessons imported from file.");

                return parsedLessons;
            }

            void LessonTreeViewWidget::parseAndExportLessons(const std::string& filePath, const std::unordered_set<int>& lessonsToExport)
            {
                m_logger.log("Parsing and exporting lessons to file: " + filePath);

                pugi::xml_document doc;
                pugi::xml_node root = doc.append_child("lessons");

                std::unordered_map<int, Lesson> lessonMap;

                for( const auto& lessonGroup : m_cashedLessons )
                {
                    for( const auto& lesson : lessonGroup.subLessons )
                    {
                        lessonMap[lesson.id] = lesson;
                    }
                }

                for( int id : lessonsToExport )
                {
                    auto it = lessonMap.find(id);
                    if( it != lessonMap.end() )
                    {
                        const Lesson& lesson = it->second;
                        pugi::xml_node lessonNode = root.append_child("lesson");
                        lessonNode.append_attribute("name") = lesson.mainName.c_str();

                        pugi::xml_node subnameNode = lessonNode.append_child("subname");
                        subnameNode.append_attribute("name") = lesson.subName.c_str();

                        for( const auto& word : lesson.words )
                        {
                            pugi::xml_node wordNode = subnameNode.append_child("word");
                            wordNode.append_attribute("translation") = word.translation.c_str();
                            wordNode.append_attribute("romaji") = word.romaji.c_str();
                            wordNode.append_attribute("kana") = word.kana.c_str();
                        }
                    }
                }

                if( !doc.save_file(filePath.c_str()) )
                {
                    m_logger.log("Error: Could not save XML file!");
                }
                else
                {
                    m_logger.log("Lessons exported to file: " + filePath);
                }
            }

            LessonDataPackage LessonTreeViewWidget::createLessonDataPackageFromSelectedNodes(const std::unordered_set<int>& nodes)
            {
                m_logger.log("Creating LessonDataPackage from selected nodes.");
                std::unordered_map<int, Lesson> lessonMap;

                for( const auto& lessonGroup : m_cashedLessons )
                {
                    for( const auto& lesson : lessonGroup.subLessons )
                    {
                        lessonMap[lesson.id] = lesson;
                    }
                }

                std::vector<Lesson> lessons;
                lessons.reserve(nodes.size());

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
                                    m_logger.log("Lesson renamed: " + std::string(renameMainNameBuffer) + " - " + std::string(renameSubNameBuffer));
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
