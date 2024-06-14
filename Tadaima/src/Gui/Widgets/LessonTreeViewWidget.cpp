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

            Lesson LessonTreeViewWidget::findLessonWithId(int id)
            {
                m_logger.log("Finding lesson with ID.");
                std::unordered_map<int, Lesson> lessonMap;

                for( const auto& lessonGroup : m_cashedLessons )
                {
                    for( const auto& lesson : lessonGroup.subLessons )
                    {
                        lessonMap[lesson.id] = lesson;
                    }
                }

                auto it = lessonMap.find(id);
                if( it != lessonMap.end() )
                {
                    return it->second;
                }

                return Lesson();
            }


            void LessonTreeViewWidget::drawTopButtons()
            {
                static bool open_add_new_lesson = false;
                static Lesson selectedLesson;

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
            }

            void LessonTreeViewWidget::drawLessonsTree(std::unordered_set<int>& markedWords, std::unordered_set<int>& lessonsToExport, bool& open_edit_lesson, Lesson& selectedLesson, Lesson& originalLesson, bool& renamePopupOpen, bool& deleteLesson, bool& createNewLessonPopupOpen)
            {
                const bool ctrlPressed = ImGui::GetIO().KeyCtrl;
                const bool shiftPressed = ImGui::GetIO().KeyShift; // Track shift key state

                static int lastSelectedWordId = -1; // Track last selected word ID

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

                                    // Check if the word is marked
                                    bool isWordMarked = markedWords.find(word.id) != markedWords.end();
                                    if( isWordMarked )
                                    {
                                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Marked words in red
                                    }

                                    ImGui::Text(" %s - %s", word.translation.c_str(), word.kana.c_str());

                                    // Mark the word if control or shift is pressed and clicked
                                    if( ImGui::IsItemClicked(0) )
                                    {
                                        memcpy(newLessonMainNameBuffer, lessonGroup.mainName.c_str(), lessonGroup.mainName.size());
                                        if( ctrlPressed )
                                        {
                                            if( isWordMarked )
                                            {
                                                markedWords.erase(word.id);
                                            }
                                            else
                                            {
                                                markedWords.insert(word.id);
                                            }
                                            lastSelectedWordId = word.id;
                                        }
                                        else if( shiftPressed )
                                        {
                                            if( lastSelectedWordId != -1 && lastSelectedWordId != word.id )
                                            {
                                                int startId = std::min(lastSelectedWordId, word.id);
                                                int endId = std::max(lastSelectedWordId, word.id);
                                                bool marking = false;
                                                for( size_t markWordIndex = 0; markWordIndex < lesson.words.size(); markWordIndex++ )
                                                {
                                                    const auto& markWord = lesson.words[markWordIndex];
                                                    if( markWord.id == startId || markWord.id == endId )
                                                    {
                                                        marking = !marking;
                                                        markedWords.insert(markWord.id);
                                                    }
                                                    if( marking || markWord.id == startId || markWord.id == endId )
                                                    {
                                                        markedWords.insert(markWord.id);
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                markedWords.insert(word.id);
                                            }
                                            lastSelectedWordId = word.id;
                                        }
                                        else
                                        {
                                            markedWords.clear();
                                            markedWords.insert(word.id);
                                            lastSelectedWordId = word.id;
                                        }
                                    }

                                    // Open context menu on right-click
                                    if( ImGui::IsItemClicked(1) )
                                    {
                                        ImGui::OpenPopup("WordContextMenu");
                                    }

                                    if( isWordMarked )
                                    {
                                        ImGui::PopStyleColor();
                                    }

                                    if( ImGui::BeginPopup("WordContextMenu") )
                                    {
                                        if( ImGui::MenuItem("Move Marked Words to New Lesson") )
                                        {
                                            createNewLessonPopupOpen = true;
                                            ImGui::CloseCurrentPopup();
                                        }

                                        if( ImGui::MenuItem("Delete Marked Words") )
                                        {
                                            std::unordered_set<int> affectedLessonIDs;
                                            // Remove marked words from all lessons
                                            for( auto& lessonGroupToDelete : m_cashedLessons )
                                            {
                                                for( auto& lessonToDelete : lessonGroupToDelete.subLessons )
                                                {
                                                    for( int wordID : markedWords )
                                                    {
                                                        auto it = std::remove_if(lessonToDelete.words.begin(), lessonToDelete.words.end(), [wordID](const Word& word)
                                                            {
                                                                return word.id == wordID;
                                                            });
                                                        if( it != lessonToDelete.words.end() )
                                                        {
                                                            affectedLessonIDs.insert(lessonToDelete.id);
                                                            lessonToDelete.words.erase(it, lessonToDelete.words.end());
                                                        }
                                                    }
                                                }
                                            }

                                            auto package = createLessonDataPackageFromSelectedNodes(affectedLessonIDs);
                                            emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonEdited, &package));

                                            markedWords.clear();
                                            ImGui::CloseCurrentPopup();
                                        }

                                        ImGui::EndPopup();
                                    }

                                    ImGui::PopID();
                                }
                                ImGui::TreePop();
                            }

                            if( isSelected )
                            {
                                ImGui::PopStyleColor();
                            }

                            // Context menu for lesson node
                            if( ImGui::IsItemClicked(1) )
                            {
                                ImGui::OpenPopup("LessonContextMenu");
                            }

                            if( ImGui::BeginPopup("LessonContextMenu") )
                            {
                                if( ImGui::BeginMenu(ICON_FA_PLAY "Play") )
                                {
                                    if( ImGui::MenuItem("vocabulary quiz") )
                                    {
                                        auto package = (m_selectedLessons.size() > 0) ?
                                            createLessonDataPackageFromSelectedNodes(m_selectedLessons) :
                                            createLessonDataPackageFromLesson(lesson);
                                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnPlayVocabularyQuiz, &package));
                                    }

                                    if( ImGui::MenuItem("multiple choice quiz") )
                                    {
                                        auto package = (m_selectedLessons.size() > 0) ?
                                            createLessonDataPackageFromSelectedNodes(m_selectedLessons) :
                                            createLessonDataPackageFromLesson(lesson);
                                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnPlayMultipleChoiceQuiz, &package));
                                    }

                                    ImGui::EndMenu();
                                }

                                if( ImGui::MenuItem(ICON_FA_PENCIL " Edit") )
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

                                if( ImGui::MenuItem(ICON_FA_PENCIL " Rename") )
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
                                    //  m_selectedLessons.insert(lesson.id);
                                }
                            }

                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }

                if( ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && !ImGui::IsMouseReleased(1) )
                {
                    m_selectedLessons.clear();
                }
            }

            void LessonTreeViewWidget::handleExportLessons(std::unordered_set<int> lessonsToExport)
            {
                if( ImGuiFileDialog::Instance()->Display("SaveFileDlgKey") )
                {
                    if( ImGuiFileDialog::Instance()->IsOk() )
                    {
                        std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                        m_logger.log("Save file selected: " + filePath);
                        parseAndExportLessons(filePath, lessonsToExport);
                    }
                    ImGuiFileDialog::Instance()->Close();
                }
            }


            void LessonTreeViewWidget::handleLessonEdit(bool& open_edit_lesson, Lesson& originalLesson, Lesson& selectedLesson)
            {
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
                            open_edit_lesson = false;
                        }
                    }
                }
            }


            void LessonTreeViewWidget::handleLessonDelete(bool& deleteLesson)
            {
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
                            auto& lessonGroupToDelete = m_cashedLessons[m_changedLessonGroupIndex];
                            if( m_changedLessonIndex >= 0 && m_changedLessonIndex < static_cast<int>(lessonGroupToDelete.subLessons.size()) )
                            {
                                auto& lessonToDelete = lessonGroupToDelete.subLessons[m_changedLessonIndex];
                                LessonDataPackage package = createLessonDataPackageFromLesson(lessonToDelete);
                                emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonDelete, &package));
                                m_logger.log("Lesson deleted: " + lessonToDelete.mainName + " - " + lessonToDelete.subName);
                            }
                        }
                    }

                    deleteLesson = false;
                }
            }

            void LessonTreeViewWidget::handleWordsMove(bool& createNewLessonPopupOpen, std::unordered_set<int>& markedWords)
            {

                if( createNewLessonPopupOpen )
                {
                    ImGui::OpenPopup("Create New Lesson");
                    createNewLessonPopupOpen = false;
                }

                if( ImGui::BeginPopupModal("Create New Lesson", NULL, ImGuiWindowFlags_AlwaysAutoResize) )
                {
                    ImGui::InputText("Main Name", newLessonMainNameBuffer, sizeof(newLessonMainNameBuffer));
                    ImGui::InputText("Sub Name", newLessonSubNameBuffer, sizeof(newLessonSubNameBuffer));

                    if( ImGui::Button("Create") )
                    {
                        std::string mainName(newLessonMainNameBuffer);
                        std::string subName(newLessonSubNameBuffer);
                        Lesson* existingLesson = nullptr;

                        std::vector<Lesson> toEdit;

                        // Check if a lesson with the same name and subname already exists
                        for( auto& lessonGroup : m_cashedLessons )
                        {
                            for( auto& lesson : lessonGroup.subLessons )
                            {
                                if( lesson.mainName == mainName && lesson.subName == subName )
                                {
                                    existingLesson = &lesson;
                                    break;
                                }
                            }
                            if( existingLesson )
                                break;
                        }

                        if( !existingLesson )
                        {
                            // Create a new lesson if it doesn't exist
                            Lesson newLesson;
                            newLesson.mainName = mainName;
                            newLesson.subName = subName;

                            for( auto& lessonGroup : m_cashedLessons )
                            {
                                for( auto& lesson : lessonGroup.subLessons )
                                {
                                    // Find marked words in the current lesson
                                    for( int wordID : markedWords )
                                    {
                                        auto it = std::find_if(lesson.words.begin(), lesson.words.end(), [wordID](const Word& word)
                                            {
                                                return word.id == wordID;
                                            });
                                        if( it != lesson.words.end() )
                                        {
                                            // Check if the word already exists in the new lesson based on content
                                            auto existingWordIt = std::find_if(newLesson.words.begin(), newLesson.words.end(), [it](const Word& word)
                                                {
                                                    return *it == word; // Using operator== to compare content
                                                });
                                            if( existingWordIt == newLesson.words.end() )
                                            {
                                                newLesson.words.push_back(*it);
                                                newLesson.words.back().id = 0;
                                            }
                                        }
                                    }
                                }
                            }

                            toEdit.push_back(newLesson);

                        }
                        else
                        {
                            // Add or overwrite words in the existing lesson
                            for( auto& lessonGroup : m_cashedLessons )
                            {
                                for( auto& lesson : lessonGroup.subLessons )
                                {
                                    // Find marked words in the current lesson
                                    for( int wordID : markedWords )
                                    {
                                        auto it = std::find_if(lesson.words.begin(), lesson.words.end(), [wordID](const Word& word)
                                            {
                                                return word.id == wordID;
                                            });
                                        if( it != lesson.words.end() )
                                        {
                                            // Check if word already exists in the existing lesson based on content
                                            auto existingWordIt = std::find_if(existingLesson->words.begin(), existingLesson->words.end(), [it](const Word& word)
                                                {
                                                    return *it == word; // Using operator== to compare content
                                                });
                                            if( existingWordIt != existingLesson->words.end() )
                                            {
                                                *existingWordIt = *it; // Overwrite the word
                                            }
                                            else
                                            {
                                                existingLesson->words.push_back(*it); // Add new word
                                                existingLesson->words.back().id = 0;
                                            }
                                        }
                                    }
                                }
                            }

                            toEdit.push_back(*existingLesson);
                        }

                        std::unordered_set<int> affectedLessonIDs;

                        // Remove marked words from all lessons
                        for( auto& lessonGroupToDelete : m_cashedLessons )
                        {
                            for( auto& lessonToDelete : lessonGroupToDelete.subLessons )
                            {
                                for( int wordID : markedWords )
                                {
                                    auto it = std::remove_if(lessonToDelete.words.begin(), lessonToDelete.words.end(), [wordID](const Word& word)
                                        {
                                            return word.id == wordID;
                                        });
                                    if( it != lessonToDelete.words.end() )
                                    {
                                        affectedLessonIDs.insert(lessonToDelete.id);
                                        lessonToDelete.words.erase(it, lessonToDelete.words.end());
                                    }
                                }
                            }
                        }

                        for( auto id : affectedLessonIDs )
                        {
                            toEdit.push_back(findLessonWithId(id));
                        }

                        auto package = createLessonDataPackageFromLessons(toEdit);
                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonEdited, &package));

                        memset(newLessonSubNameBuffer, 0, sizeof(newLessonSubNameBuffer));
                        ImGui::CloseCurrentPopup();

                        markedWords.clear();
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Cancel") )
                    {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
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
                static std::unordered_set<int> markedWords; // Store marked word IDs
                static bool createNewLessonPopupOpen = false; // Track if the create new lesson popup should be open

                if( !ImGui::Begin("Lessons Overview", p_open, ImGuiWindowFlags_NoDecoration) )
                {
                    ImGui::End();
                    return;
                }

                /*
                    Handle top layer of the tree view. Buttons.
                */
                drawTopButtons();

                /*
                    Draw lessons tree and mark all events that can occur on it.
                */
                drawLessonsTree(markedWords, lessonsToExport, open_edit_lesson, selectedLesson, originalLesson, renamePopupOpen, deleteLesson, createNewLessonPopupOpen);



                /*
                *
                * Handle creataion of new lesson.
                */
                handleLessonEdit(open_edit_lesson, originalLesson, selectedLesson);




                handleLessonDelete(deleteLesson);



                handleWordsMove(createNewLessonPopupOpen, markedWords);

                /*
                handle lesson export.
            */
                handleExportLessons(lessonsToExport);

                ShowRenamePopup(renamePopupOpen);




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
                    return parsedLessons;
                }

                std::unordered_set<std::string> uniqueLessons; // To track unique lessons

                for( pugi::xml_node lessonNode : doc.child("lessons").children("lesson") )
                {
                    std::string lessonName = lessonNode.attribute("name").as_string();

                    for( pugi::xml_node subnameNode : lessonNode.children("subname") )
                    {
                        Lesson lesson;
                        lesson.mainName = lessonName;
                        lesson.subName = subnameNode.attribute("name").as_string();
                        std::string uniqueKey = lesson.mainName + ":" + lesson.subName;

                        if( uniqueLessons.find(uniqueKey) == uniqueLessons.end() )
                        {
                            for( pugi::xml_node wordNode : subnameNode.children("word") )
                            {
                                Word word;
                                word.translation = wordNode.attribute("translation").as_string();
                                word.romaji = wordNode.attribute("romaji").as_string();
                                word.kana = wordNode.attribute("kana").as_string();
                                lesson.words.push_back(word);
                            }

                            parsedLessons.push_back(lesson);
                            uniqueLessons.insert(uniqueKey);
                        }
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

            void LessonTreeViewWidget::ShowRenamePopup(bool& renamePopupOpen)
            {
                if( renamePopupOpen )
                {
                    ImGui::OpenPopup("Rename Lesson");
                    renamePopupOpen = false;
                }

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

        } // namespace widget
    } // namespace gui
} // namespace tadaima
