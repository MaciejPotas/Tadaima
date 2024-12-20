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
                        lessonMap[lesson.groupName].groupName = lesson.groupName;
                        lessonMap[lesson.groupName].subLessons[lesson.mainName].push_back(lesson);
                    }

                    for( const auto& pair : lessonMap )
                    {
                        m_cashedLessons.push_back(pair.second);
                    }
                }

                m_lessonSettingsWidget.initialize(r_package);
                m_logger.log("LessonTreeViewWidget initialized.");
            }

            Lesson LessonTreeViewWidget::copyWordsToNewLesson(const std::unordered_set<int>& wordIds)
            {
                m_logger.log("Copying words to a new lesson.");

                Lesson newLesson;
                newLesson.groupName = "Mixed vocabulary";
                newLesson.mainName = "Mixed vocabulary";
                newLesson.subName = "Mixed vocabulary";

                std::unordered_set<int> wordIdSet(wordIds.begin(), wordIds.end());


                for( const auto& lessonGroup : m_cashedLessons )
                {
                    for( const auto& subLessons : lessonGroup.subLessons )
                    {
                        for( const auto& lesson : subLessons.second )
                        {
                            for( const auto& word : lesson.words )
                            {
                                if( wordIdSet.find(word.id) != wordIdSet.end() )
                                {
                                    newLesson.words.push_back(word);
                                }
                            }
                        }
                    }
                }

                return newLesson;
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
                    lessonPackage.set(LessonDataKey::GroupName, lesson.groupName);
                    lessonPackage.set(LessonDataKey::MainName, lesson.mainName);
                    lessonPackage.set(LessonDataKey::SubName, lesson.subName);
                    std::vector<WordDataPackage> wordPackages;

                    for( const auto& word : lesson.words )
                    {
                        WordDataPackage wordPackage(word.id);
                        wordPackage.set(LessonWordDataKey::id, word.id);
                        wordPackage.set(LessonWordDataKey::Kana, word.kana);
                        wordPackage.set(LessonWordDataKey::Kanji, word.kanji);
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
                lessonPackage.set(LessonDataKey::GroupName, lesson.groupName);
                lessonPackage.set(LessonDataKey::MainName, lesson.mainName);
                lessonPackage.set(LessonDataKey::SubName, lesson.subName);
                std::vector<WordDataPackage> wordPackages;

                for( const auto& word : lesson.words )
                {
                    WordDataPackage wordPackage(word.id);
                    wordPackage.set(LessonWordDataKey::id, word.id);
                    wordPackage.set(LessonWordDataKey::Kana, word.kana);
                    wordPackage.set(LessonWordDataKey::Kanji, word.kanji);
                    wordPackage.set(LessonWordDataKey::Translation, word.translation);
                    wordPackage.set(LessonWordDataKey::Romaji, word.romaji);
                    wordPackage.set(LessonWordDataKey::ExampleSentence, word.exampleSentence);
                    wordPackage.set(LessonWordDataKey::Tags, word.tags);
                    wordPackage.set(LessonWordDataKey::Conjugation, word.conjugations);
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
                    for( const auto& subLessons : lessonGroup.subLessons )
                    {
                        for( const auto& lesson : subLessons.second )
                        {
                            lessonMap[lesson.id] = lesson;
                        }
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

                // Style for buttons
                ImVec4 buttonColor = ImVec4(0.95f, 0.77f, 0.06f, 1.0f);  // Yellow
                ImVec4 buttonHoveredColor = ImVec4(0.85f, 0.67f, 0.06f, 1.0f);  // Darker Yellow
                ImVec4 buttonActiveColor = ImVec4(0.75f, 0.57f, 0.06f, 1.0f);  // Even Darker Yellow

                // Push style for buttons
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.87f, 0.93f, 0.97f, 1.0f));  // Light blue
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.53f, 0.81f, 0.92f, 0.6f));  // Light blue on hover
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.95f, 0.77f, 0.06f, 1.0f));  // Yellow on active
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 5));


                // Create button
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
                // Import button
                if( ImGui::Button(ICON_FA_UPLOAD " Import") )
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

                // Pop style for buttons
                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();
            }

            void LessonTreeViewWidget::drawLessonsTree(std::unordered_set<int>& markedWords, std::unordered_set<int>& lessonsToExport, bool& open_edit_lesson, Lesson& selectedLesson, Lesson& originalLesson, bool& renamePopupOpen, bool& deleteLesson, bool& createNewLessonPopupOpen)
            {
                const bool ctrlPressed = ImGui::GetIO().KeyCtrl;
                const bool shiftPressed = ImGui::GetIO().KeyShift; // Track shift key state

                static int rightClickedLessonGroupId = -1; // Track last right-clicked lesson group ID
                static int rightClickedLessonSubGroupId = -1; // Track last right-clicked lesson group ID

                bool popupShown = false;
                static bool showDeleteGroupConfirmation = false;

                static int lastSelectedWordId = -1; // Track last selected word ID

                for( size_t groupIndex = 0; groupIndex < m_cashedLessons.size(); groupIndex++ )
                {
                    auto& lessonGroup = m_cashedLessons[groupIndex];
                    ImGui::PushID(static_cast<int>(groupIndex));

                    if( ImGui::TreeNode(lessonGroup.groupName.c_str()) )
                    {
                        int mainGroupIndex = 0;
                        for( const auto& [mainName, lessons] : lessonGroup.subLessons )
                        {
                            ImGui::PushID(mainName.c_str());

                            // Draw the mainName node
                            if( ImGui::TreeNode(mainName.c_str()) )
                            {
                                int lessonIndex = 0;
                                for( const auto& lesson : lessons )
                                {
                                    ImGui::PushID(lesson.id);
                                    //auto& lesson = lessonGroup.subLessons[lessonIndex];
                                    //ImGui::PushID(static_cast<int>(lessonIndex));
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
                                                memcpy(newLessonGroupNameBuffer, lesson.groupName.c_str(), lesson.groupName.size());
                                                memcpy(newLessonMainNameBuffer, lesson.mainName.c_str(), lesson.mainName.size());
                                                memcpy(newLessonSubNameBuffer, lesson.subName.c_str(), lesson.subName.size());

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
                                                if( ImGui::BeginMenu(ICON_FA_PLAY "PlayMixedVocabulary") )
                                                {
                                                    auto package = createLessonDataPackageFromLesson(copyWordsToNewLesson(markedWords));
                                                    if( ImGui::MenuItem("vocabulary quiz") )
                                                    {
                                                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnPlayVocabularyQuiz, &package));
                                                    }

                                                    if( ImGui::MenuItem("multiple choice quiz") )
                                                    {
                                                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnPlayMultipleChoiceQuiz, &package));
                                                    }

                                                    if( ImGui::MenuItem("Conjuation quiz") )
                                                    {
                                                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnConjuactionQuiz, &package));
                                                    }

                                                    ImGui::EndMenu();
                                                }

                                                if( ImGui::MenuItem("Move Marked Words to New Lesson") )
                                                {
                                                    createNewLessonPopupOpen = true;
                                                    ImGui::CloseCurrentPopup();
                                                }

                                                if( ImGui::MenuItem("Delete Marked Words") )
                                                {
                                                    std::unordered_set<int> impactedLessonIDs; // Updated variable name for affected lessons

                                                    // Iterate through all lesson groups
                                                    for( auto& currentLessonGroup : m_cashedLessons )
                                                    {
                                                        for( auto& [groupMainName, groupedLessons] : currentLessonGroup.subLessons )
                                                        {
                                                            for( auto& currentLesson : groupedLessons )
                                                            {
                                                                // Remove marked words from the current lesson
                                                                auto wordIt = std::remove_if(
                                                                    currentLesson.words.begin(),
                                                                    currentLesson.words.end(),
                                                                    [&markedWords](const Word& word)
                                                                    {
                                                                        return markedWords.count(word.id) > 0; // Check if the word is marked
                                                                    });

                                                                // Check if any words were removed
                                                                if( wordIt != currentLesson.words.end() )
                                                                {
                                                                    currentLesson.words.erase(wordIt, currentLesson.words.end());
                                                                    impactedLessonIDs.insert(currentLesson.id); // Mark the lesson as impacted
                                                                }
                                                            }
                                                        }
                                                    }

                                                    // Emit an event for the modified lessons
                                                    if( !impactedLessonIDs.empty() )
                                                    {
                                                        auto updatedPackage = createLessonDataPackageFromSelectedNodes(impactedLessonIDs); // Updated variable name for package
                                                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonEdited, &updatedPackage));
                                                    }

                                                    // Clear marked words and close the popup
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
                                        popupShown = true;
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

                                            if( ImGui::MenuItem("conjuaction quiz") )
                                            {
                                                auto package = (m_selectedLessons.size() > 0) ?
                                                    createLessonDataPackageFromSelectedNodes(m_selectedLessons) :
                                                    createLessonDataPackageFromLesson(lesson);
                                                emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnConjuactionQuiz, &package));
                                            }

                                            ImGui::EndMenu();
                                        }

                                        if( ImGui::MenuItem(ICON_FA_PENCIL " Edit") )
                                        {
                                            m_logger.log("Edit lesson selected.");
                                            m_changedLessonGroupIndex = static_cast<int>(groupIndex);
                                            m_changedLessonIndex = static_cast<int>(lessonIndex);
                                            m_changedLessonSubGroupIndex = static_cast<int>(mainGroupIndex);
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
                                            m_changedLessonSubGroupIndex = static_cast<int>(mainGroupIndex);

                                            strncpy(renameGroupNameBuffer, lesson.groupName.c_str(), sizeof(renameGroupNameBuffer));
                                            strncpy(renameMainNameBuffer, lesson.mainName.c_str(), sizeof(renameMainNameBuffer));
                                            strncpy(renameSubNameBuffer, lesson.subName.c_str(), sizeof(renameSubNameBuffer));
                                            renamePopupOpen = true;
                                            ImGui::CloseCurrentPopup();
                                        }

                                        if( ImGui::MenuItem(ICON_FA_TRASH " Delete") )
                                        {
                                            m_logger.log("Delete lesson selected.");
                                            m_changedLessonGroupIndex = static_cast<int>(groupIndex);
                                            m_changedLessonIndex = static_cast<int>(lessonIndex);
                                            m_changedLessonSubGroupIndex = static_cast<int>(mainGroupIndex);
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
                                        }
                                    }

                                    ImGui::PopID();
                                    ++lessonIndex;
                                }
                                ImGui::TreePop();
                            }

                            if( ImGui::IsItemClicked(1) && !popupShown )
                            {
                                rightClickedLessonGroupId = static_cast<int>(groupIndex);
                                rightClickedLessonSubGroupId = static_cast<int>(mainGroupIndex);

                                ImGui::OpenPopup("LessonGroupContextMenu");
                            }

                            if( rightClickedLessonGroupId == (int)groupIndex && ImGui::BeginPopup("LessonGroupContextMenu") )
                            {
                                if( ImGui::MenuItem(ICON_FA_TRASH " Delete Group") )
                                {
                                    // Show the delete group confirmation popup instead of deleting immediately
                                    showDeleteGroupConfirmation = true;
                                    ImGui::CloseCurrentPopup();
                                }

                                ImGui::EndPopup();
                            }

                            ImGui::PopID();
                            ++mainGroupIndex;
                        }

                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }

                if( ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && !ImGui::IsMouseReleased(1) )
                {
                    m_selectedLessons.clear();
                }

                // Show the delete group confirmation popup
                if( showDeleteGroupConfirmation )
                {
                    ImGui::OpenPopup("Delete SubGroup Confirmation");
                    showDeleteGroupConfirmation = false;
                }

                // Confirmation popup
                if( ImGui::BeginPopupModal("Delete SubGroup Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize) )
                {
                    ImGui::Text("Are you sure you want to delete this subgroup?");
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    if( rightClickedLessonGroupId >= 0 && rightClickedLessonGroupId < static_cast<int>(m_cashedLessons.size()) &&
                        rightClickedLessonSubGroupId >= 0 )
                    {
                        if( ImGui::Button("Yes", ImVec2(120, 0)) )
                        {
                            // Clear selected lessons
                            m_selectedLessons.clear();

                            // Collect lesson IDs from the selected subgroup
                            auto& lessonGroup = m_cashedLessons[rightClickedLessonGroupId];
                            auto subGroupIt = std::next(lessonGroup.subLessons.begin(), rightClickedLessonSubGroupId);

                            if( subGroupIt != lessonGroup.subLessons.end() )
                            {
                                for( const auto& lesson : subGroupIt->second )
                                {
                                    m_selectedLessons.insert(lesson.id);
                                }

                                // Log and mark the subgroup for deletion
                                m_logger.log("Delete lesson subgroup confirmed: " + subGroupIt->first);
                                // lessonGroup.subLessons.erase(subGroupIt); // Remove the subgroup
                            }

                            rightClickedLessonGroupId = -1; // Reset the group ID
                            rightClickedLessonSubGroupId = -1; // Reset the subgroup ID
                            deleteLesson = true;

                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::SameLine();

                        if( ImGui::Button("No", ImVec2(120, 0)) )
                        {
                            // Close the popup without performing any action
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    else
                    {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: Invalid subgroup selected.");
                        if( ImGui::Button("Close", ImVec2(120, 0)) )
                        {
                            ImGui::CloseCurrentPopup();
                        }
                    }

                    ImGui::EndPopup();
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
                    if( !m_selectedLessons.empty() )
                    {
                        // Handle deletion of multiple selected lessons
                        auto package = createLessonDataPackageFromSelectedNodes(m_selectedLessons);
                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonDelete, &package));

                        for( auto lessonId : m_selectedLessons )
                        {
                            // Remove the lessons from the cached data
                            for( auto& lessonGroup : m_cashedLessons )
                            {
                                for( auto& [mainName, lessons] : lessonGroup.subLessons )
                                {
                                    auto it = std::remove_if(
                                        lessons.begin(),
                                        lessons.end(),
                                        [lessonId](const Lesson& lesson) { return lesson.id == lessonId; });

                                    if( it != lessons.end() )
                                    {
                                        lessons.erase(it, lessons.end());
                                        break; // Found and removed; exit loop
                                    }
                                }
                            }
                        }

                        m_selectedLessons.clear();
                        m_logger.log("Selected lessons deleted.");
                    }
                    else
                    {
                        // Handle single lesson deletion based on indices
                        if( m_changedLessonGroupIndex >= 0 && m_changedLessonGroupIndex < static_cast<int>(m_cashedLessons.size()) )
                        {
                            auto& lessonGroupToDelete = m_cashedLessons[m_changedLessonGroupIndex];
                            if( m_changedLessonIndex >= 0 && m_changedLessonIndex < static_cast<int>(lessonGroupToDelete.subLessons.size()) )
                            {
                                // Find the lesson using the m_changedLessonIndex
                                auto subLessonIt = std::next(lessonGroupToDelete.subLessons.begin(), m_changedLessonSubGroupIndex);

                                if( subLessonIt != lessonGroupToDelete.subLessons.end() )
                                {
                                    auto& lessons = subLessonIt->second;

                                    if( m_changedLessonIndex >= 0 && m_changedLessonIndex < static_cast<int>(lessons.size()) )
                                    {
                                        const auto& lessonToDelete = lessons[m_changedLessonIndex];

                                        // Prepare the data package for the single lesson deletion
                                        LessonDataPackage package = createLessonDataPackageFromLesson(lessonToDelete);
                                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonDelete, &package));
                                        m_logger.log("Lesson deleted: " + lessonToDelete.mainName + " - " + lessonToDelete.subName);

                                        // Remove the specific lesson
                                        lessons.erase(lessons.begin() + m_changedLessonIndex);

                                        // Clean up empty subgroups if needed
                                        if( lessons.empty() )
                                        {
                                            lessonGroupToDelete.subLessons.erase(subLessonIt);
                                        }
                                    }
                                }
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
                    ImGui::InputText("Group Name", newLessonGroupNameBuffer, sizeof(newLessonGroupNameBuffer));
                    ImGui::InputText("Main Name", newLessonMainNameBuffer, sizeof(newLessonMainNameBuffer));
                    ImGui::InputText("Sub Name", newLessonSubNameBuffer, sizeof(newLessonSubNameBuffer));

                    if( ImGui::Button("Create") )
                    {
                        std::string groupName(newLessonGroupNameBuffer);
                        std::string mainName(newLessonMainNameBuffer);
                        std::string subName(newLessonSubNameBuffer);
                        Lesson* existingLesson = nullptr;

                        std::vector<Lesson> lessonsToEdit;

                        // Find or create the group and lesson
                        for( auto& lessonGroup : m_cashedLessons )
                        {
                            if( lessonGroup.groupName == groupName )
                            {
                                for( auto& [existingMainName, lessons] : lessonGroup.subLessons )
                                {
                                    if( existingMainName == mainName )
                                    {
                                        auto it = std::find_if(lessons.begin(), lessons.end(), [&subName](const Lesson& lesson)
                                            {
                                                return lesson.subName == subName;
                                            });

                                        if( it != lessons.end() )
                                        {
                                            existingLesson = &(*it);
                                        }
                                        break;
                                    }
                                }
                            }
                            if( existingLesson ) break;
                        }

                        if( !existingLesson )
                        {
                            // Create a new lesson if it doesn't exist
                            Lesson newLesson;
                            newLesson.groupName = groupName;
                            newLesson.mainName = mainName;
                            newLesson.subName = subName;

                            for( auto& lessonGroup : m_cashedLessons )
                            {
                                for( auto& [existingMainName, lessons] : lessonGroup.subLessons )
                                {
                                    for( auto& lesson : lessons )
                                    {
                                        for( int wordID : markedWords )
                                        {
                                            auto it = std::find_if(lesson.words.begin(), lesson.words.end(), [wordID](const Word& word)
                                                {
                                                    return word.id == wordID;
                                                });

                                            if( it != lesson.words.end() )
                                            {
                                                // Avoid duplicates in the new lesson
                                                auto existingWordIt = std::find_if(newLesson.words.begin(), newLesson.words.end(),
                                                    [&it](const Word& word) { return *it == word; });

                                                if( existingWordIt == newLesson.words.end() )
                                                {
                                                    newLesson.words.push_back(*it);
                                                    newLesson.words.back().id = 0; // Reset ID for the new lesson
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            lessonsToEdit.push_back(newLesson);
                        }
                        else
                        {
                            // Add or overwrite words in the existing lesson
                            for( auto& lessonGroup : m_cashedLessons )
                            {
                                for( auto& [existingMainName, lessons] : lessonGroup.subLessons )
                                {
                                    for( auto& lesson : lessons )
                                    {
                                        for( int wordID : markedWords )
                                        {
                                            auto it = std::find_if(lesson.words.begin(), lesson.words.end(), [wordID](const Word& word)
                                                {
                                                    return word.id == wordID;
                                                });

                                            if( it != lesson.words.end() )
                                            {
                                                // Avoid duplicates in the existing lesson
                                                auto existingWordIt = std::find_if(existingLesson->words.begin(), existingLesson->words.end(),
                                                    [&it](const Word& word) { return *it == word; });

                                                if( existingWordIt != existingLesson->words.end() )
                                                {
                                                    *existingWordIt = *it; // Overwrite existing word
                                                }
                                                else
                                                {
                                                    existingLesson->words.push_back(*it); // Add new word
                                                    existingLesson->words.back().id = 0; // Reset ID
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            lessonsToEdit.push_back(*existingLesson);
                        }

                        std::unordered_set<int> affectedLessonIDs;

                        // Remove marked words from all lessons
                        for( auto& lessonGroup : m_cashedLessons )
                        {
                            for( auto& [existingMainName, lessons] : lessonGroup.subLessons )
                            {
                                for( auto& lesson : lessons )
                                {
                                    auto it = std::remove_if(lesson.words.begin(), lesson.words.end(), [&markedWords](const Word& word)
                                        {
                                            return markedWords.find(word.id) != markedWords.end();
                                        });

                                    if( it != lesson.words.end() )
                                    {
                                        lesson.words.erase(it, lesson.words.end());
                                        affectedLessonIDs.insert(lesson.id);
                                    }
                                }
                            }
                        }

                        for( int id : affectedLessonIDs )
                        {
                            lessonsToEdit.push_back(findLessonWithId(id));
                        }

                        // Create the data package and emit the event
                        auto package = createLessonDataPackageFromLessons(lessonsToEdit);
                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonEdited, &package));

                        // Reset buffers and close popup
                        memset(newLessonGroupNameBuffer, 0, sizeof(newLessonGroupNameBuffer));
                        memset(newLessonMainNameBuffer, 0, sizeof(newLessonMainNameBuffer));
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
                //
                                /*
                                handle lesson export.
                            */
                            // handleExportLessons(lessonsToExport);

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

                // Map to hold all lessons by their IDs for quick lookup
                std::unordered_map<int, Lesson> lessonMap;

                for( const auto& lessonGroup : m_cashedLessons )
                {
                    for( const auto& [mainName, groupedLessons] : lessonGroup.subLessons )
                    {
                        for( const auto& lesson : groupedLessons )
                        {
                            lessonMap[lesson.id] = lesson;
                        }
                    }
                }

                // Iterate through the lessons to be exported
                for( int id : lessonsToExport )
                {
                    auto it = lessonMap.find(id);
                    if( it != lessonMap.end() )
                    {
                        const Lesson& lesson = it->second;

                        // Create an XML node for the lesson
                        pugi::xml_node lessonNode = root.append_child("lesson");
                        lessonNode.append_attribute("groupName") = lesson.groupName.c_str();
                        lessonNode.append_attribute("mainName") = lesson.mainName.c_str();
                        lessonNode.append_attribute("subName") = lesson.subName.c_str();

                        // Add words to the lesson node
                        for( const auto& word : lesson.words )
                        {
                            pugi::xml_node wordNode = lessonNode.append_child("word");
                            wordNode.append_attribute("translation") = word.translation.c_str();
                            wordNode.append_attribute("romaji") = word.romaji.c_str();
                            wordNode.append_attribute("kana") = word.kana.c_str();
                            wordNode.append_attribute("kanji") = word.kanji.c_str(); // Include kanji if available
                            wordNode.append_attribute("example") = word.exampleSentence.c_str();

                            // Add tags as child nodes of the word
                            for( const auto& tag : word.tags )
                            {
                                pugi::xml_node tagNode = wordNode.append_child("tag");
                                tagNode.append_attribute("name") = tag.c_str();
                            }
                        }
                    }
                }

                // Save the XML document to the specified file
                if( !doc.save_file(filePath.c_str()) )
                {
                    m_logger.log("Error: Could not save XML file!", tools::LogLevel::PROBLEM);
                }
                else
                {
                    m_logger.log("Lessons successfully exported to file: " + filePath, tools::LogLevel::INFO);
                }
            }

            LessonDataPackage LessonTreeViewWidget::createLessonDataPackageFromSelectedNodes(const std::unordered_set<int>& selectedNodeIds)
            {
                m_logger.log("Creating LessonDataPackage from selected nodes.");

                // Map to hold all lessons by their IDs for quick lookup
                std::unordered_map<int, Lesson> allLessonsById;

                // Populate the map from cached lessons
                for( const auto& cachedLessonGroup : m_cashedLessons )
                {
                    for( const auto& [mainName, groupedLessons] : cachedLessonGroup.subLessons )
                    {
                        for( const auto& groupedLesson : groupedLessons )
                        {
                            allLessonsById[groupedLesson.id] = groupedLesson;
                        }
                    }
                }

                // Collect lessons matching the selected node IDs
                std::vector<Lesson> selectedLessons;
                selectedLessons.reserve(selectedNodeIds.size());

                for( int nodeId : selectedNodeIds )
                {
                    auto lessonIt = allLessonsById.find(nodeId);
                    if( lessonIt != allLessonsById.end() )
                    {
                        selectedLessons.push_back(lessonIt->second);
                    }
                }

                // Create and return the data package from the selected lessons
                return createLessonDataPackageFromLessons(selectedLessons);
            }

            void LessonTreeViewWidget::ShowRenamePopup(bool& isRenamePopupOpen)
            {
                if( isRenamePopupOpen )
                {
                    ImGui::OpenPopup("Rename Lesson");
                    isRenamePopupOpen = false;
                }

                if( ImGui::BeginPopupModal("Rename Lesson", nullptr, ImGuiWindowFlags_AlwaysAutoResize) )
                {
                    ImGui::TextWrapped("Please enter the new names for the lesson:");

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Input for the group name
                    ImGui::Text("Group Name:");
                    ImGui::PushItemWidth(300);
                    ImGui::InputText("##GroupNameInput", renameGroupNameBuffer, sizeof(renameGroupNameBuffer));
                    ImGui::PopItemWidth();

                    ImGui::Spacing();

                    // Input for the main name
                    ImGui::Text("Main Name:");
                    ImGui::PushItemWidth(300);
                    ImGui::InputText("##MainNameInput", renameMainNameBuffer, sizeof(renameMainNameBuffer));
                    ImGui::PopItemWidth();

                    ImGui::Spacing();

                    // Input for the sub name
                    ImGui::Text("Sub Name:");
                    ImGui::PushItemWidth(300);
                    ImGui::InputText("##SubNameInput", renameSubNameBuffer, sizeof(renameSubNameBuffer));
                    ImGui::PopItemWidth();

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Save button
                    if( ImGui::Button("Save", ImVec2(120, 0)) )
                    {
                        if( m_changedLessonGroupIndex >= 0 && m_changedLessonGroupIndex < static_cast<int>(m_cashedLessons.size()) )
                        {
                            auto& targetLessonGroup = m_cashedLessons[m_changedLessonGroupIndex];
                            if( m_changedLessonIndex >= 0 && m_changedLessonIndex < static_cast<int>(targetLessonGroup.subLessons.size()) )
                            {
                                // Find the correct sub-lesson list based on the current `m_changedLessonIndex`
                                auto& subLessonPair = *std::next(targetLessonGroup.subLessons.begin(), m_changedLessonIndex);
                                auto& targetLesson = subLessonPair.second.front(); // Assuming a single lesson per subName

                                bool groupNameChanged = targetLesson.groupName != renameGroupNameBuffer;
                                bool mainNameChanged = targetLesson.mainName != renameMainNameBuffer;
                                bool subNameChanged = targetLesson.subName != renameSubNameBuffer;

                                if( groupNameChanged || mainNameChanged || subNameChanged )
                                {
                                    // Update the names
                                    if( groupNameChanged )
                                    {
                                        targetLesson.groupName = renameGroupNameBuffer;
                                    }

                                    if( mainNameChanged )
                                    {
                                        targetLesson.mainName = renameMainNameBuffer;
                                    }

                                    if( subNameChanged )
                                    {
                                        targetLesson.subName = renameSubNameBuffer;
                                    }

                                    // Create and emit data package
                                    LessonDataPackage updatedPackage = createLessonDataPackageFromLesson(targetLesson);
                                    emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonRename, &updatedPackage));

                                    // Log the rename operation
                                    m_logger.log("Lesson renamed: " + std::string(renameGroupNameBuffer) + " - " + std::string(renameMainNameBuffer) + " - " + std::string(renameSubNameBuffer));
                                }
                            }
                        }
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    // Cancel button
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
