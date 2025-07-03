#include "LessonTreeViewWidget.h"
#include "resources/IconsFontAwesome4.h"
#include "imgui.h"
#include <map>
#include <unordered_set>
#include "ImGuiFileDialog.h"
#include "Tools/pugixml.hpp"
#include "Tools/Logger.h"
#include "LessonTreeViewWidget/LessonUtils.h"
#include "LessonTreeViewWidget/LessonFileIO.h"

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

                    for( const auto& lesson : package->m_lessons )
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
                LessonDataPackage lessonDataPackage(lessons);

                return lessonDataPackage;
            }

            LessonDataPackage LessonTreeViewWidget::createLessonDataPackageFromLesson(const Lesson& lesson)
            {
                m_logger.log("Creating LessonDataPackage from a single lesson.");
                LessonDataPackage lessonDataPackage(lesson);

                return lessonDataPackage;
            }

            Lesson LessonTreeViewWidget::findLessonWithId(int id) const
            {
                for( const auto& group : m_cashedLessons )
                    for( const auto& [_, lessons] : group.subLessons )
                        for( const auto& lesson : lessons )
                            if( lesson.id == id )
                                return lesson;
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
                    //m_type = LessonPackageType::LessonCreated;
                    open_add_new_lesson = true;
                    m_lessonSettingsWidget.clear();
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
                        auto lessons = LessonFileIO::importLessons(filePath, m_logger);
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

            void LessonTreeViewWidget::drawWordRow(const Word& word, const Lesson& lesson)
            {
                bool isSelected = m_selectedWords.count(word.id) > 0;
                if( isSelected )
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.2f, 0.2f, 1));

                if( ImGui::Selectable((word.translation + " - " + word.kana).c_str(), isSelected) )
                {
                    const bool ctrl = ImGui::GetIO().KeyCtrl;
                    const bool shift = ImGui::GetIO().KeyShift;

                    if( ctrl )
                    {
                        if( isSelected )
                            m_selectedWords.erase(word.id);
                        else
                            m_selectedWords.insert(word.id);
                        m_lastSelectedWordId = word.id;
                    }
                    else if( shift && m_lastSelectedWordId != -1 )
                    {
                        auto it1 = std::find_if(lesson.words.begin(), lesson.words.end(), [&](const Word& w) { return w.id == m_lastSelectedWordId; });
                        auto it2 = std::find_if(lesson.words.begin(), lesson.words.end(), [&](const Word& w) { return w.id == word.id; });
                        if( it1 != lesson.words.end() && it2 != lesson.words.end() )
                        {
                            if( it1 > it2 ) std::swap(it1, it2);
                            for( auto it = it1; it <= it2; ++it )
                                m_selectedWords.insert(it->id);
                        }
                        m_lastSelectedWordId = word.id;
                    }
                    else
                    {
                        m_selectedWords.clear();
                        m_selectedWords.insert(word.id);
                        m_lastSelectedWordId = word.id;
                    }
                }

                if( isSelected )
                    ImGui::PopStyleColor();


                // Right-click: context menu for this word (and all selected words)
                if( ImGui::BeginPopupContextItem(("WordContextMenu" + std::to_string(word.id)).c_str()) )
                {
                    showSelectedWordsContextMenu(lesson);
                }
            }

            void LessonTreeViewWidget::setLessonWordsSelection(const Lesson& lesson, bool select)
            {
                for( const auto& w : lesson.words )
                {
                    if( select )
                        m_selectedWords.insert(w.id);
                    else
                        m_selectedWords.erase(w.id);
                }
            }

            void LessonTreeViewWidget::setLessonRangeSelection(const std::vector<Lesson>& lessonsInSubgroup, int fromIdx, int toIdx, bool select)
            {
                int from = std::min(fromIdx, toIdx);
                int to = std::max(fromIdx, toIdx);
                for( int i = from; i <= to; ++i )
                {
                    int lid = lessonsInSubgroup[i].id;
                    if( select )
                        m_selectedLessons.insert(lid);
                    else
                        m_selectedLessons.erase(lid);
                    setLessonWordsSelection(lessonsInSubgroup[i], select);
                }
            }

            void LessonTreeViewWidget::showSelectedWordsContextMenu(const Lesson& lesson)
            {
                // 1. Play menu
                if( ImGui::BeginMenu("Play") )
                {
                    struct QuizOption
                    {
                        const char* label;
                        LessonTreeViewWidgetEvent event;
                        const char* tooltip;
                    } quizOptions[] = {
                        { ICON_FA_QUESTION " Vocabulary Quiz",      LessonTreeViewWidgetEvent::OnPlayVocabularyQuiz, "Test your vocab on selected words." },
                        { ICON_FA_LIST     " Multiple Choice Quiz", LessonTreeViewWidgetEvent::OnPlayMultipleChoiceQuiz, "Multiple choice from these words." },
                        { ICON_FA_MAGIC    " Conjugation Quiz",     LessonTreeViewWidgetEvent::OnConjuactionQuiz, "Practice conjugation of selected words." }
                    };
                    for( const auto& opt : quizOptions )
                    {
                        if( ImGui::MenuItem(opt.label) )
                        {
                            // Package only selected words
                            auto lessonPackage = LessonUtils::copyWordsToNewLesson(m_selectedWords, m_cashedLessons);
                            auto package = createLessonDataPackageFromLesson(lessonPackage);
                            emitEvent(WidgetEvent(*this, opt.event, &package));
                        }
                        if( ImGui::IsItemHovered() )
                            ImGui::SetTooltip("%s", opt.tooltip);
                    }
                    ImGui::EndMenu();
                }

                ImGui::Separator();

                // 2. Move to new lesson
                if( ImGui::MenuItem(ICON_FA_PLUS " Move to New Lesson...") )
                {
                    m_pendingAction.type = LessonActionState::Type::MoveWordsToLesson;
                    m_pendingAction.editable = lesson;
                    // Optionally store which words, etc.
                   // ImGui::CloseCurrentPopup();
                }
                if( ImGui::IsItemHovered() )
                    ImGui::SetTooltip("Move selected word(s) to a new lesson.");

                // 3. Delete words
                if( ImGui::MenuItem(ICON_FA_TRASH " Delete") )
                {
                    // m_pendingAction.type = LessonActionState::Type::DeleteWords;
                    ImGui::CloseCurrentPopup();
                }
                if( ImGui::IsItemHovered() )
                    ImGui::SetTooltip("Remove selected word(s) from all lessons.");

                ImGui::Separator();


                if( ImGui::IsItemHovered() )
                    ImGui::SetTooltip("Export selected word(s) to file.");

                ImGui::EndPopup();
            }

            void LessonTreeViewWidget::showLessonContextMenu(const Lesson& lesson)
            {
                ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1), ICON_FA_BOOK "  %s", lesson.subName.c_str());
                ImGui::Separator();

                if( ImGui::BeginMenu("Play") )
                {
                    ImGui::TextDisabled("Choose a quiz mode:");
                    ImGui::Separator();

                    struct QuizOption
                    {
                        const char* label;
                        LessonTreeViewWidgetEvent event;
                    } quizOptions[] = {
                        { ICON_FA_QUESTION " Vocabulary Quiz", LessonTreeViewWidgetEvent::OnPlayVocabularyQuiz },
                        { ICON_FA_LIST     " Multiple Choice Quiz", LessonTreeViewWidgetEvent::OnPlayMultipleChoiceQuiz },
                        { ICON_FA_MAGIC    " Conjugation Quiz", LessonTreeViewWidgetEvent::OnConjuactionQuiz }
                    };
                    for( const auto& opt : quizOptions )
                    {
                        if( ImGui::MenuItem(opt.label) )
                        {
                            auto package = (m_selectedLessons.size() > 0)
                                ? createLessonDataPackageFromSelectedNodes(m_selectedLessons)
                                : createLessonDataPackageFromLesson(lesson);
                            emitEvent(WidgetEvent(*this, opt.event, &package));
                        }
                    }
                    ImGui::EndMenu();
                }

                ImGui::Separator();

                struct ActionOption
                {
                    const char* label;
                    const char* icon;
                    LessonActionState::Type type;
                    bool needsOriginal;
                } actionOptions[] = {
                    { "Edit",   ICON_FA_PENCIL,       LessonActionState::Type::Edit,   true },
                    { "Rename", ICON_FA_PENCIL_SQUARE,LessonActionState::Type::Rename, true },
                    { "Delete", ICON_FA_TRASH,        LessonActionState::Type::Delete, false },
                    { "Export", ICON_FA_DOWNLOAD,     LessonActionState::Type::Export, true }
                };
                for( const auto& opt : actionOptions )
                {
                    if( ImGui::MenuItem((std::string(opt.icon) + " " + opt.label).c_str()) )
                    {
                        m_pendingAction.type = opt.type;
                        m_pendingAction.lessonId = lesson.id;
                        if( opt.needsOriginal )
                        {
                            m_pendingAction.original = lesson;
                        }
                        m_pendingAction.editable = lesson;
                    }
                }
            }

            void LessonTreeViewWidget::drawLessonRow(const Lesson& lesson, const std::vector<Lesson>& lessonsInSubgroup, int lessonIdx)
            {
                bool isNodeOpen = ImGui::TreeNodeEx(
                    (void*)(intptr_t)lesson.id,
                    ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth, ""
                );
                ImGui::SameLine();

                bool isLessonSelected = m_selectedLessons.count(lesson.id) > 0;

                if( ImGui::Selectable(lesson.subName.c_str(), isLessonSelected, ImGuiSelectableFlags_SpanAllColumns) )
                {
                    const bool ctrl = ImGui::GetIO().KeyCtrl;
                    const bool shift = ImGui::GetIO().KeyShift;

                    if( ctrl )
                    {
                        // Toggle this lesson and its words
                        if( isLessonSelected )
                        {
                            m_selectedLessons.erase(lesson.id);
                            setLessonWordsSelection(lesson, false);
                        }
                        else
                        {
                            m_selectedLessons.insert(lesson.id);
                            setLessonWordsSelection(lesson, true);
                        }
                        m_lastSelectedLessonId = lesson.id;
                    }
                    else if( shift && m_lastSelectedLessonId != -1 )
                    {
                        int lastIdx = -1;
                        for( size_t i = 0; i < lessonsInSubgroup.size(); ++i )
                            if( lessonsInSubgroup[i].id == m_lastSelectedLessonId )
                                lastIdx = (int)i;
                        if( lastIdx != -1 )
                            setLessonRangeSelection(lessonsInSubgroup, lastIdx, lessonIdx, true);

                        m_lastSelectedLessonId = lesson.id;
                    }
                    else
                    {
                        // Clear all selection
                        m_selectedLessons.clear();
                        m_selectedWords.clear();
                    }
                }

                if( ImGui::BeginPopupContextItem(("LessonContextMenu" + std::to_string(lesson.id)).c_str()) )
                {
                    showLessonContextMenu(lesson);
                    ImGui::EndPopup();
                }

                // Draw words if expanded
                if( isNodeOpen )
                {
                    for( size_t wi = 0; wi < lesson.words.size(); ++wi )
                        drawWordRow(lesson.words[wi], lesson);
                    ImGui::TreePop();
                }
            }

            void LessonTreeViewWidget::drawLessonsTree()
            {
                for( size_t g = 0; g < m_cashedLessons.size(); ++g )
                {
                    const auto& group = m_cashedLessons[g];
                    ImGui::PushID((int)g);

                    if( ImGui::TreeNode(group.groupName.c_str()) )
                    {
                        for( const auto& [mainName, lessons] : group.subLessons )
                        {
                            ImGui::PushID(mainName.c_str());
                            if( ImGui::TreeNode(mainName.c_str()) )
                            {
                                for( size_t lessonIndex = 0; lessonIndex < lessons.size(); ++lessonIndex )
                                {
                                    drawLessonRow(lessons[lessonIndex], lessons, (int)lessonIndex);
                                }

                                ImGui::TreePop();
                            }
                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
            }

            void LessonTreeViewWidget::handleLessonEdit()
            {
                static bool initialized = false;

                if( m_pendingAction.type == LessonActionState::Type::Edit )
                {
                    bool open = true;
                    if( !initialized )
                    {
                        m_lessonSettingsWidget.setLesson(m_pendingAction.editable); // Pass editable reference if needed
                        initialized = true;
                    }

                    m_lessonSettingsWidget.draw(&open);

                    if( !open )
                    {
                        // If user accepted (check via a bool from the widget, or compare original/editable)
                        if( m_pendingAction.editable != m_pendingAction.original )
                        {
                            LessonDataPackage pkg = createLessonDataPackageFromLesson(m_pendingAction.editable);
                            emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonEdited, &pkg));
                            m_logger.log("Lesson edited.");
                        }
                        initialized = false;
                        m_pendingAction.type = LessonActionState::Type::None;
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
            void LessonTreeViewWidget::showMoveWordsToLessonPopup()
            {
                if( m_pendingAction.type != LessonActionState::Type::MoveWordsToLesson )
                    return;

                // Only open once

                static bool popupJustOpened = false;
                if( !popupJustOpened )
                {
                    ImGui::OpenPopup("Move Words to Lesson");
                    std::strncpy(GroupNameBuf, m_pendingAction.editable.groupName.c_str(), sizeof(GroupNameBuf));
                    std::strncpy(MainNameBuf, m_pendingAction.editable.mainName.c_str(), sizeof(MainNameBuf));
                    std::strncpy(SubNameBuf, m_pendingAction.editable.subName.c_str(), sizeof(SubNameBuf));

                    popupJustOpened = true;
                }

                if( ImGui::BeginPopupModal("Move Words to Lesson", nullptr, ImGuiWindowFlags_AlwaysAutoResize) )
                {

                    ImGui::Text("Move %zu words to which lesson?", m_selectedWords.size());
                    ImGui::InputText("Group Name", GroupNameBuf, sizeof(GroupNameBuf));
                    ImGui::InputText("Main Name", MainNameBuf, sizeof(MainNameBuf));
                    ImGui::InputText("Sub Name", SubNameBuf, sizeof(SubNameBuf));

                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                    if( ImGui::Button("Move") )
                    {
                        std::string groupName(GroupNameBuf);
                        std::string mainName(MainNameBuf);
                        std::string subName(SubNameBuf);

                        std::unordered_set<int> movedWordIds(m_selectedWords.begin(), m_selectedWords.end());
                        std::vector<Word> wordsToMove;

                        // 1. Collect the Word objects to move
                        for( const auto& group : m_cashedLessons )
                            for( const auto& [main, lessons] : group.subLessons )
                                for( const auto& lesson : lessons )
                                    for( const auto& w : lesson.words )
                                        if( movedWordIds.count(w.id) )
                                            wordsToMove.push_back(w);

                        // 2. Remove words from their original lessons (track changed lessons)
                        std::vector<Lesson> updatedLessons;
                        Lesson* destinationLesson = nullptr;

                        for( auto& group : m_cashedLessons )
                        {
                            for( auto& [main, lessons] : group.subLessons )
                            {
                                for( auto& lesson : lessons )
                                {
                                    size_t oldSz = lesson.words.size();
                                    lesson.words.erase(
                                        std::remove_if(lesson.words.begin(), lesson.words.end(),
                                            [&](const Word& w) { return movedWordIds.count(w.id); }),
                                        lesson.words.end());

                                    if( lesson.words.size() != oldSz )
                                        updatedLessons.push_back(lesson);

                                    // Look for destination lesson pointer
                                    if( !destinationLesson
                                        && lesson.groupName == groupName
                                        && lesson.mainName == mainName
                                        && lesson.subName == subName )
                                    {
                                        destinationLesson = &lesson;
                                    }
                                }
                            }
                        }

                        // 3. If destination lesson exists, add words (avoid dups); else create new
                        if( destinationLesson )
                        {
                            for( const auto& w : wordsToMove )
                            {
                                auto already = std::find_if(destinationLesson->words.begin(), destinationLesson->words.end(),
                                    [&](const Word& ww) { return ww == w; });
                                if( already == destinationLesson->words.end() )
                                {
                                    Word wCopy = w;
                                    wCopy.id = 0; // Reset ID for new context, if needed
                                    destinationLesson->words.push_back(wCopy);
                                }
                            }
                            // If not already in updatedLessons, add
                            if( std::none_of(updatedLessons.begin(), updatedLessons.end(), [&](const Lesson& l) { return l.id == destinationLesson->id; }) )
                                updatedLessons.push_back(*destinationLesson);
                        }
                        else
                        {
                            Lesson newLesson;
                            newLesson.groupName = groupName;
                            newLesson.mainName = mainName;
                            newLesson.subName = subName;
                            for( const auto& w : wordsToMove )
                            {
                                Word wCopy = w;
                                wCopy.id = 0;
                                newLesson.words.push_back(wCopy);
                            }
                            updatedLessons.push_back(newLesson);
                            // You may want to add newLesson to m_cashedLessons as well, if you manage cache here
                        }

                        // 4. Emit and cleanup
                        auto package = createLessonDataPackageFromLessons(updatedLessons);
                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonEdited, &package));

                        memset(GroupNameBuf, 0, sizeof(GroupNameBuf));
                        memset(MainNameBuf, 0, sizeof(MainNameBuf));
                        memset(SubNameBuf, 0, sizeof(SubNameBuf));
                        m_pendingAction = {}; // Reset to None
                        popupJustOpened = false;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();
                    if( ImGui::Button("Cancel") )
                    {
                        m_pendingAction = {};
                        popupJustOpened = false;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
                else
                {
                    popupJustOpened = false;
                }
            }


            void LessonTreeViewWidget::draw(bool* p_open)
            {

                if( !ImGui::Begin("Lessons Overview", p_open, ImGuiWindowFlags_NoDecoration) )
                {
                    ImGui::End();
                    return;
                }

                /*
                    Handle top layer of the tree view. Buttons.
                */
                drawTopButtons();

                drawLessonsTree();

                handleLessonEdit();

                ShowRenamePopup();

                ShowDeletePopup();

                showMoveWordsToLessonPopup();

                handleExportLessons();

                ImGui::End();
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

            void LessonTreeViewWidget::ShowRenamePopup()
            {
                if( m_pendingAction.type != LessonActionState::Type::Rename )
                    return;

                // Fill buffers from pendingAction on popup open
                static bool initialized = false;
                if( !ImGui::IsPopupOpen("Rename Lesson") )
                {
                    std::strncpy(GroupNameBuf, m_pendingAction.editable.groupName.c_str(), sizeof(GroupNameBuf));
                    std::strncpy(MainNameBuf, m_pendingAction.editable.mainName.c_str(), sizeof(MainNameBuf));
                    std::strncpy(SubNameBuf, m_pendingAction.editable.subName.c_str(), sizeof(SubNameBuf));
                    GroupNameBuf[sizeof(GroupNameBuf) - 1] = '\0';
                    MainNameBuf[sizeof(MainNameBuf) - 1] = '\0';
                    SubNameBuf[sizeof(SubNameBuf) - 1] = '\0';
                    ImGui::OpenPopup("Rename Lesson");
                    initialized = true;
                }

                if( ImGui::BeginPopupModal("Rename Lesson", nullptr, ImGuiWindowFlags_AlwaysAutoResize) )
                {
                    ImGui::TextWrapped("Please enter the new names for the lesson:");
                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                    ImGui::Text("Group Name:");
                    ImGui::PushItemWidth(300);
                    ImGui::InputText("##GroupNameInput", GroupNameBuf, sizeof(GroupNameBuf));
                    ImGui::PopItemWidth();

                    ImGui::Spacing();

                    ImGui::Text("Main Name:");
                    ImGui::PushItemWidth(300);
                    ImGui::InputText("##MainNameInput", MainNameBuf, sizeof(MainNameBuf));
                    ImGui::PopItemWidth();

                    ImGui::Spacing();

                    ImGui::Text("Sub Name:");
                    ImGui::PushItemWidth(300);
                    ImGui::InputText("##SubNameInput", SubNameBuf, sizeof(SubNameBuf));
                    ImGui::PopItemWidth();

                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                    if( ImGui::Button("Save", ImVec2(120, 0)) )
                    {
                        // Update names
                        m_pendingAction.editable.groupName = GroupNameBuf;
                        m_pendingAction.editable.mainName = MainNameBuf;
                        m_pendingAction.editable.subName = SubNameBuf;

                        // Only emit if changed
                        if( m_pendingAction.editable != m_pendingAction.original )
                        {
                            LessonDataPackage updatedPackage = createLessonDataPackageFromLesson(m_pendingAction.editable);
                            emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonRename, &updatedPackage));
                            m_logger.log("Lesson renamed: " + std::string(GroupNameBuf) + " - " + std::string(MainNameBuf) + " - " + std::string(SubNameBuf));
                        }
                        ImGui::CloseCurrentPopup();
                        m_pendingAction.type = LessonActionState::Type::None;
                        initialized = false;
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Cancel", ImVec2(120, 0)) )
                    {
                        ImGui::CloseCurrentPopup();
                        m_pendingAction.type = LessonActionState::Type::None;
                        initialized = false;
                    }
                    ImGui::EndPopup();
                }

            }

            void LessonTreeViewWidget::ShowDeletePopup()
            {
                if( m_pendingAction.type != LessonActionState::Type::Delete )
                    return;

                // Open the popup if not already open
                if( !ImGui::IsPopupOpen("Delete Lesson") )
                {
                    ImGui::OpenPopup("Delete Lesson");
                }

                if( ImGui::BeginPopupModal("Delete Lesson", nullptr, ImGuiWindowFlags_AlwaysAutoResize) )
                {
                    ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1), "Are you sure you want to delete this lesson?");
                    ImGui::Spacing();
                    ImGui::Text("%s / %s / %s",
                        m_pendingAction.editable.groupName.c_str(),
                        m_pendingAction.editable.mainName.c_str(),
                        m_pendingAction.editable.subName.c_str()
                    );
                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                    if( ImGui::Button("Delete", ImVec2(120, 0)) )
                    {
                        // Do the deletion logic!
                        LessonDataPackage package = createLessonDataPackageFromLesson(m_pendingAction.editable);
                        emitEvent(WidgetEvent(*this, LessonTreeViewWidgetEvent::OnLessonDelete, &package));
                        m_logger.log("Lesson deleted: " +
                            m_pendingAction.editable.groupName + " / " +
                            m_pendingAction.editable.mainName + " / " +
                            m_pendingAction.editable.subName
                        );
                        ImGui::CloseCurrentPopup();
                        m_pendingAction.type = LessonActionState::Type::None;
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Cancel", ImVec2(120, 0)) )
                    {
                        ImGui::CloseCurrentPopup();
                        m_pendingAction.type = LessonActionState::Type::None;
                    }

                    ImGui::EndPopup();
                }
            }

            void LessonTreeViewWidget::handleExportLessons()
            {
                if( m_pendingAction.type != LessonActionState::Type::Export )
                    return;

                static bool initialize = false;

                if( !initialize )
                {
                    IGFD::FileDialogConfig config;
                    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Always);
                    ImGuiFileDialog::Instance()->OpenDialog("SaveLessonDlgKey", "Export Lessons", ".xml", config);
                    initialize = true;
                }

                if( ImGuiFileDialog::Instance()->Display("SaveLessonDlgKey") )
                {
                    if( ImGuiFileDialog::Instance()->IsOk() )
                    {
                        std::vector<Lesson> selected;
                        for( int id : m_selectedLessons )
                            selected.push_back(findLessonWithId(id));

                        std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                        LessonFileIO::exportLessons(filePath, selected, m_logger);
                        m_logger.log("Lessons exported to: " + filePath);
                    }

                    initialize = false;
                    m_pendingAction.type = LessonActionState::Type::None;
                    ImGuiFileDialog::Instance()->Close();
                }
            }

        } // namespace widget
    } // namespace gui
} // namespace tadaima
