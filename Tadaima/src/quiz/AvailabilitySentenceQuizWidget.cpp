#include "AvailabilitySentenceQuizWidget.h"

#include "tools/pugixml.hpp"
#include "tools/SystemTools.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <limits>
#include <unordered_set>
#include <utility>

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            namespace
            {
                std::string GetAttribute(const pugi::xml_node& node, const char* name)
                {
                    std::string result;
                    const pugi::xml_attribute attribute = node.attribute(name);

                    if( attribute )
                    {
                        result = attribute.as_string();
                    }

                    return result;
                }

                std::string GetText(const pugi::xml_node& node)
                {
                    std::string result;

                    if( node )
                    {
                        result = node.text().as_string();
                    }

                    return result;
                }

                uint32_t GetUnsignedAttribute(
                    const pugi::xml_node& node,
                    const char* name,
                    uint32_t defaultValue)
                {
                    uint32_t result = defaultValue;
                    const pugi::xml_attribute attribute = node.attribute(name);

                    if( attribute )
                    {
                        result = static_cast<uint32_t>(
                            attribute.as_uint(defaultValue));
                    }

                    return result;
                }

                uint64_t SafeMultiply(uint64_t first, uint64_t second)
                {
                    uint64_t result = 0U;

                    if( (first != 0U) && (second != 0U) )
                    {
                        if( first >
                            (std::numeric_limits<uint64_t>::max() / second) )
                        {
                            result = std::numeric_limits<uint64_t>::max();
                        }
                        else
                        {
                            result = first * second;
                        }
                    }

                    return result;
                }
            }

            AvailabilitySentenceQuizWidget::AvailabilitySentenceQuizWidget(
                tools::Logger& logger)
                : m_logger(logger)
                , m_rng(static_cast<uint32_t>(
                    std::chrono::steady_clock::now().time_since_epoch().count()))
            {
            }

            void AvailabilitySentenceQuizWidget::initialize(
                const tools::DataPackage& r_package)
            {
                try
                {
                    const SettingsDataPackage* package =
                        dynamic_cast<const SettingsDataPackage*>(&r_package);

                    if( package != nullptr )
                    {
                        const std::string projectPath =
                            package->get<std::string>(
                                SettingsPackageKey::SentenceQuizSentenceMapPath);
                        const std::string numberOfTries =
                            package->get<std::string>(
                                SettingsPackageKey::TriesForQuiz);

                        copyToBuffer(
                            m_projectPath,
                            sizeof(m_projectPath),
                            projectPath);
                        m_numberOfTries = std::stoi(numberOfTries);

                        m_logger.log(
                            "AvailabilitySentenceQuizWidget: Initialized with quiz project path.",
                            tools::LogLevel::INFO);
                    }
                }
                catch( const std::exception& exception )
                {
                    m_logger.log(
                        "AvailabilitySentenceQuizWidget::initialize exception: " +
                        std::string(exception.what()),
                        tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log(
                        "AvailabilitySentenceQuizWidget::initialize unknown exception.",
                        tools::LogLevel::PROBLEM);
                }
            }

            void AvailabilitySentenceQuizWidget::draw(bool* p_open)
            {
                try
                {
                    if( (p_open != nullptr) && *p_open )
                    {
                        if( !m_showSettingsWindow && !m_showQuizWindow )
                        {
                            m_showSettingsWindow = true;
                        }

                        if( m_showSettingsWindow )
                        {
                            drawSettingsWindow(p_open);
                        }

                        if( m_showQuizWindow )
                        {
                            drawQuizWindow(p_open);
                        }

                        if( !m_showSettingsWindow && !m_showQuizWindow )
                        {
                            *p_open = false;
                        }
                    }
                }
                catch( const std::exception& exception )
                {
                    m_logger.log(
                        "AvailabilitySentenceQuizWidget::draw exception: " +
                        std::string(exception.what()),
                        tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log(
                        "AvailabilitySentenceQuizWidget::draw unknown exception.",
                        tools::LogLevel::PROBLEM);
                }
            }

            void AvailabilitySentenceQuizWidget::drawSettingsWindow(bool* p_open)
            {
                ImGui::SetNextWindowSize(ImVec2(720, 620), ImGuiCond_FirstUseEver);
                ImGui::PushStyleColor(
                    ImGuiCol_WindowBg,
                    ImVec4(0.98f, 0.92f, 0.84f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));

                bool windowOpen = true;

                if( ImGui::Begin(
                    "Availability Grammar Quiz Settings",
                    &windowOpen,
                    ImGuiWindowFlags_NoCollapse) )
                {
                    if( ImGui::IsKeyPressed(ImGuiKey_F1) )
                    {
                        if( !m_showStatistics )
                        {
                            updateStatistics();
                        }

                        m_showStatistics = !m_showStatistics;
                    }

                    ImGui::TextColored(
                        ImVec4(0.8f, 0.2f, 0.2f, 1.0f),
                        "Verb + noun + ga + aru Quiz");
                    ImGui::TextWrapped(
                        "Tests a dictionary-form verb followed by an availability noun "
                        "and a finite form of aru. Verbs, relations, nouns, contexts and "
                        "sentence patterns are read from quiz_project.xml.");
                    ImGui::Separator();

                    ImGui::InputText(
                        "Main verb filter",
                        m_wordFilter,
                        IM_ARRAYSIZE(m_wordFilter));
                    ImGui::TextDisabled(
                        "Examples: id=yomu | type=godan | tag=reporting");

                    ImGui::Spacing();
                    ImGui::SeparatorText("Sentence shape");
                    ImGui::Checkbox("Verb only", &m_enableSimple);
                    ImGui::SameLine();
                    ImGui::TextDisabled("はたらくじかんがない");
                    ImGui::Checkbox("Object を", &m_enableObject);
                    ImGui::SameLine();
                    ImGui::TextDisabled("ほんをよむじかんがない");
                    ImGui::Checkbox("Destination へ", &m_enableDestination);
                    ImGui::SameLine();
                    ImGui::TextDisabled("がっこうへいくじかんがない");
                    ImGui::Checkbox("Companion と", &m_enableCompanion);
                    ImGui::SameLine();
                    ImGui::TextDisabled("ともだちとはなすじかんがない");
                    ImGui::Checkbox("Place で", &m_enablePlace);
                    ImGui::SameLine();
                    ImGui::TextDisabled("こうえんでするけいかくがない");

                    ImGui::Spacing();
                    ImGui::SeparatorText("Japanese answer style");
                    ImGui::Checkbox("Plain / informal", &m_enablePlainOutput);
                    ImGui::SameLine();
                    ImGui::Checkbox("Polite", &m_enablePoliteOutput);

                    ImGui::Spacing();
                    ImGui::SeparatorText("ある form");

                    if( ImGui::BeginTable(
                        "AvailabilityQuizFormsTable",
                        4,
                        ImGuiTableFlags_SizingStretchSame) )
                    {
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("Non-past affirmative", &m_enableNonPast);
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("Non-past negative", &m_enableNegative);
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("Past affirmative", &m_enablePast);
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("Past negative", &m_enablePastNegative);
                        ImGui::EndTable();
                    }

                    ImGui::TextDisabled(
                        "The main verb always uses its XML dictionary form. "
                        "Only the existence verb changes form.");

                    ImGui::Spacing();
                    ImGui::SeparatorText("Quiz size");
                    ImGui::SliderInt(
                        "Number of questions",
                        &m_numberOfQuestions,
                        1,
                        200);
                    ImGui::Checkbox("Random order", &m_randomOrder);

                    ImGui::Spacing();
                    ImGui::Separator();

                    if( ImGui::Button(
                        "Load files / refresh statistics",
                        ImVec2(240, 0)) )
                    {
                        updateStatistics();
                        m_showStatistics = true;
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Start quiz", ImVec2(120, 0)) )
                    {
                        startQuiz();
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Cancel", ImVec2(120, 0)) )
                    {
                        m_showSettingsWindow = false;
                    }

                    ImGui::TextDisabled(
                        "Press F1 in this window to show or hide statistics.");

                    if( !m_statusMessage.empty() )
                    {
                        ImGui::Spacing();
                        ImGui::TextWrapped("%s", m_statusMessage.c_str());
                    }

                    if( m_showStatistics )
                    {
                        ImGui::Spacing();
                        ImGui::SeparatorText("Statistics");
                        ImGui::Text("Verbs: %u", m_statistics.verbs);
                        ImGui::Text(
                            "Availability nouns: %u",
                            m_statistics.availabilityNouns);
                        ImGui::Text(
                            "Valid grammar patterns: %u",
                            m_statistics.sentencePatterns);
                        ImGui::Text("Reusable entities: %u", m_statistics.entities);
                        ImGui::Text("Tag profiles: %u", m_statistics.tagProfiles);
                        ImGui::Text(
                            "Compatible generation blocks: %llu",
                            static_cast<unsigned long long>(
                                m_statistics.combinationSpaces));
                        ImGui::Text(
                            "Possible questions with current settings: %llu",
                            static_cast<unsigned long long>(
                                m_statistics.generatedQuestions));
                    }
                }

                ImGui::End();

                if( !windowOpen )
                {
                    m_showSettingsWindow = false;
                }

                ImGui::PopStyleColor();
                ImGui::PopStyleVar();

                if( (p_open != nullptr) &&
                    !m_showSettingsWindow &&
                    !m_showQuizWindow )
                {
                    *p_open = false;
                }
            }

            void AvailabilitySentenceQuizWidget::drawQuizWindow(bool* p_open)
            {
                ImGui::SetNextWindowSize(ImVec2(720, 540), ImGuiCond_FirstUseEver);
                ImGui::PushStyleColor(
                    ImGuiCol_WindowBg,
                    ImVec4(0.98f, 0.92f, 0.84f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));

                bool windowOpen = true;

                if( ImGui::Begin(
                    "Availability Grammar Quiz",
                    &windowOpen,
                    ImGuiWindowFlags_NoCollapse) )
                {
                    processAutoAdvance();

                    if( ImGui::IsKeyPressed(ImGuiKey_F1) )
                    {
                        m_showFullAnswer = !m_showFullAnswer;
                    }

                    if( m_questions.empty() ||
                        (m_currentQuestionIndex >= m_questions.size()) )
                    {
                        ImGui::TextColored(
                            ImVec4(0.8f, 0.2f, 0.2f, 1.0f),
                            "Quiz complete!");
                        ImGui::Separator();
                        ImGui::Text("Correct answers: %u", m_correctAnswers);
                        ImGui::Text("Wrong answers: %u", m_wrongAnswers);
                        ImGui::Text("Accepted manually: %u", m_acceptedAnswers);
                        ImGui::Spacing();

                        if( ImGui::Button(
                            "Back to settings",
                            ImVec2(160, 0)) )
                        {
                            m_showQuizWindow = false;
                            m_showSettingsWindow = true;
                        }

                        ImGui::SameLine();

                        if( ImGui::Button("Close", ImVec2(120, 0)) )
                        {
                            m_showQuizWindow = false;
                        }
                    }
                    else
                    {
                        const QuizQuestion& question =
                            m_questions[m_currentQuestionIndex];
                        const float progress =
                            static_cast<float>(m_currentQuestionIndex) /
                            static_cast<float>(m_questions.size());

                        ImGui::Text(
                            "Question %u / %zu",
                            m_currentQuestionIndex + 1U,
                            m_questions.size());
                        ImGui::ProgressBar(progress, ImVec2(-1, 0));
                        ImGui::Text(
                            "Correct: %u    Wrong: %u    Accepted: %u",
                            m_correctAnswers,
                            m_wrongAnswers,
                            m_acceptedAnswers);
                        ImGui::Spacing();
                        ImGui::Separator();

                        ImGui::Text("Write the Japanese sentence:");
                        ImGui::SameLine();
                        ImGui::TextColored(
                            ImVec4(0.8f, 0.2f, 0.2f, 1.0f),
                            "%s",
                            question.targetLabel.c_str());
                        ImGui::Spacing();
                        ImGui::Text("Source:");
                        ImGui::PushTextWrapPos(0.0f);
                        ImGui::TextColored(
                            ImVec4(0.15f, 0.15f, 0.15f, 1.0f),
                            "%s",
                            question.sourceSentence.c_str());
                        ImGui::PopTextWrapPos();
                        ImGui::Spacing();

                        if( m_focusAnswerInput )
                        {
                            ImGui::SetKeyboardFocusHere();
                            m_focusAnswerInput = false;
                        }

                        const bool submittedByEnter = ImGui::InputText(
                            "Your answer",
                            m_userAnswer,
                            IM_ARRAYSIZE(m_userAnswer),
                            ImGuiInputTextFlags_EnterReturnsTrue);
                        ImGui::Spacing();
                        const bool submittedByButton =
                            ImGui::Button("Check", ImVec2(120, 0));
                        ImGui::SameLine();

                        if( ImGui::Button("Accept / Skip", ImVec2(130, 0)) )
                        {
                            skipQuestion();
                        }

                        ImGui::SameLine();

                        if( ImGui::Button(
                            "Back to settings",
                            ImVec2(160, 0)) )
                        {
                            m_showQuizWindow = false;
                            m_showSettingsWindow = true;
                        }

                        if( submittedByEnter || submittedByButton )
                        {
                            submitAnswer();
                        }

                        if( m_waitingForNextQuestion )
                        {
                            ImGui::SameLine();

                            if( ImGui::Button("Next", ImVec2(120, 0)) )
                            {
                                advanceQuestion();
                            }
                        }

                        if( !m_feedbackMessage.empty() )
                        {
                            ImGui::Spacing();
                            ImGui::Separator();
                            ImGui::TextWrapped("%s", m_feedbackMessage.c_str());

                            if( m_waitingForNextQuestion &&
                                ImGui::Button(
                                    "Accept as correct",
                                    ImVec2(160, 0)) )
                            {
                                acceptCurrentAnswerAsCorrect();
                            }
                        }

                        if( m_showFullAnswer )
                        {
                            ImGui::Spacing();
                            ImGui::SeparatorText("F1 full answer");
                            ImGui::TextWrapped(
                                "Expected Japanese sentence: %s",
                                question.targetSentence.c_str());
                            ImGui::TextDisabled(
                                "Main verb: %s (%s)",
                                question.mainVerbMeaning.c_str(),
                                question.mainVerbKana.c_str());
                            ImGui::TextDisabled(
                                "Availability noun: %s / %s",
                                question.availabilityNounKana.c_str(),
                                question.availabilityNounKanji.c_str());
                            ImGui::TextDisabled(
                                "Existence form: %s",
                                question.existenceForm.c_str());
                            ImGui::TextDisabled(
                                "Pattern: %s",
                                question.patternId.c_str());
                            ImGui::TextDisabled(
                                "Bindings: %s",
                                question.bindings.c_str());
                        }

                        ImGui::TextDisabled(
                            "Press F1 to show or hide the full expected answer.");
                    }
                }

                ImGui::End();

                if( !windowOpen )
                {
                    m_showQuizWindow = false;
                }

                ImGui::PopStyleColor();
                ImGui::PopStyleVar();

                if( (p_open != nullptr) &&
                    !m_showSettingsWindow &&
                    !m_showQuizWindow )
                {
                    *p_open = false;
                }
            }

            bool AvailabilitySentenceQuizWidget::loadFiles()
            {
                bool result = false;

                try
                {
                    m_filesLoaded = false;
                    m_dictionaries.clear();
                    m_tagProfiles.clear();
                    m_entities.clear();
                    m_effectiveEntityTags.clear();
                    m_contexts.clear();
                    m_sentencePatterns.clear();

                    result = loadProject(std::string(m_projectPath));

                    const auto verbsIterator = m_dictionaries.find("verbs");
                    result = result &&
                        (verbsIterator != m_dictionaries.end()) &&
                        !verbsIterator->second.empty() &&
                        !getExistenceVerbs().empty() &&
                        !getAvailabilityNouns().empty() &&
                        (countValidPatterns() > 0U);

                    if( result )
                    {
                        m_filesLoaded = true;
                        m_statusMessage = std::format(
                            "Quiz project loaded: {} verbs, {} availability nouns, "
                            "{} valid grammar patterns.",
                            verbsIterator->second.size(),
                            getAvailabilityNouns().size(),
                            countValidPatterns());
                    }
                    else if( m_statusMessage.empty() )
                    {
                        m_statusMessage =
                            "Could not load verbs, an existence verb tagged "
                            "'existence', availability nouns, and valid grammar patterns.";
                    }
                }
                catch( const std::exception& exception )
                {
                    m_logger.log(
                        "AvailabilitySentenceQuizWidget::loadFiles exception: " +
                        std::string(exception.what()),
                        tools::LogLevel::PROBLEM);
                    m_statusMessage = exception.what();
                }
                catch( ... )
                {
                    m_logger.log(
                        "AvailabilitySentenceQuizWidget::loadFiles unknown exception.",
                        tools::LogLevel::PROBLEM);
                    m_statusMessage =
                        "Unknown error while loading the quiz project.";
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::loadProject(
                const std::string& path)
            {
                bool result = false;
                m_projectFullPath = buildConfiguredPath(path);

                pugi::xml_document document;
                const pugi::xml_parse_result parseResult =
                    document.load_file(m_projectFullPath.c_str());

                if( parseResult )
                {
                    const pugi::xml_node rootNode =
                        document.child("quiz_project");

                    if( rootNode )
                    {
                        m_projectDirectory =
                            std::filesystem::path(m_projectFullPath)
                            .parent_path()
                            .string();
                        bool allFilesLoaded = true;
                        uint32_t loadedDictionaries = 0U;
                        const pugi::xml_node dictionariesNode =
                            rootNode.child("dictionaries");

                        for( pugi::xml_node dictionaryNode =
                            dictionariesNode.child("dictionary");
                            dictionaryNode;
                            dictionaryNode =
                            dictionaryNode.next_sibling("dictionary") )
                        {
                            const std::string dictionaryId =
                                GetAttribute(dictionaryNode, "id");
                            const std::string file =
                                GetAttribute(dictionaryNode, "file");

                            if( !dictionaryId.empty() && !file.empty() )
                            {
                                const bool loaded = loadDictionary(
                                    dictionaryId,
                                    resolveProjectFile(
                                        m_projectDirectory,
                                        file));
                                allFilesLoaded = allFilesLoaded && loaded;

                                if( loaded )
                                {
                                    ++loadedDictionaries;
                                }
                            }
                            else
                            {
                                allFilesLoaded = false;
                            }
                        }

                        const std::string vocabularyFile =
                            GetAttribute(rootNode.child("vocabulary"), "file");
                        const std::string contextsFile =
                            GetAttribute(rootNode.child("contexts"), "file");
                        const std::string sentenceMapsFile =
                            GetAttribute(rootNode.child("sentence_maps"), "file");

                        const bool vocabularyLoaded =
                            !vocabularyFile.empty() &&
                            loadVocabulary(
                                resolveProjectFile(
                                    m_projectDirectory,
                                    vocabularyFile));
                        const bool contextsLoaded =
                            !contextsFile.empty() &&
                            loadContexts(
                                resolveProjectFile(
                                    m_projectDirectory,
                                    contextsFile));
                        const bool patternsLoaded =
                            !sentenceMapsFile.empty() &&
                            loadSentencePatterns(
                                resolveProjectFile(
                                    m_projectDirectory,
                                    sentenceMapsFile));

                        allFilesLoaded =
                            allFilesLoaded &&
                            vocabularyLoaded &&
                            contextsLoaded &&
                            patternsLoaded &&
                            (loadedDictionaries > 0U);

                        if( allFilesLoaded )
                        {
                            result = buildEffectiveEntityTags();
                        }
                    }
                }

                if( !result )
                {
                    m_logger.log(
                        "Could not load availability quiz project XML from path: " +
                        m_projectFullPath +
                        ". Parser status: " +
                        parseResult.description(),
                        tools::LogLevel::PROBLEM);
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::loadDictionary(
                const std::string& dictionaryId,
                const std::string& fullPath)
            {
                bool result = false;
                pugi::xml_document document;
                const pugi::xml_parse_result parseResult =
                    document.load_file(fullPath.c_str());

                if( parseResult )
                {
                    const pugi::xml_node wordsNode =
                        document.child("dictionary").child("words");
                    std::vector<DictionaryWord> words;

                    for( pugi::xml_node wordNode = wordsNode.child("word");
                        wordNode;
                        wordNode = wordNode.next_sibling("word") )
                    {
                        DictionaryWord word;
                        word.dictionaryId = dictionaryId;

                        for( pugi::xml_attribute attribute =
                            wordNode.first_attribute();
                            attribute;
                            attribute = attribute.next_attribute() )
                        {
                            word.attributes[attribute.name()] =
                                attribute.value();
                        }

                        word.id = GetAttribute(wordNode, "id");
                        word.translation = GetAttribute(wordNode, "translation");
                        word.kana = GetAttribute(wordNode, "kana");
                        word.kanji = GetAttribute(wordNode, "kanji");
                        word.type = GetAttribute(wordNode, "type");
                        word.ending = GetAttribute(wordNode, "ending");
                        word.attributes["dictionary"] = dictionaryId;

                        const pugi::xml_node tagsNode = wordNode.child("tags");

                        for( pugi::xml_node tagNode = tagsNode.child("tag");
                            tagNode;
                            tagNode = tagNode.next_sibling("tag") )
                        {
                            const std::string tag = trimText(GetText(tagNode));

                            if( !tag.empty() )
                            {
                                appendUnique(word.tags, tag);
                            }
                        }

                        const pugi::xml_node categoriesNode =
                            wordNode.child("categories");

                        for( pugi::xml_node categoryNode =
                            categoriesNode.child("category");
                            categoryNode;
                            categoryNode =
                            categoryNode.next_sibling("category") )
                        {
                            const std::string category =
                                trimText(GetText(categoryNode));

                            if( !category.empty() )
                            {
                                appendUnique(word.tags, category);
                            }
                        }

                        const pugi::xml_node relationsNode =
                            wordNode.child("relations");

                        for( pugi::xml_node relationNode =
                            relationsNode.child("relation");
                            relationNode;
                            relationNode =
                            relationNode.next_sibling("relation") )
                        {
                            const std::string role =
                                GetAttribute(relationNode, "role");

                            if( !role.empty() )
                            {
                                std::vector<std::string>& relationTags =
                                    word.relations[role];

                                for( pugi::xml_node tagNode =
                                    relationNode.child("tag");
                                    tagNode;
                                    tagNode = tagNode.next_sibling("tag") )
                                {
                                    const std::string tag =
                                        trimText(GetText(tagNode));

                                    if( !tag.empty() )
                                    {
                                        appendUnique(relationTags, tag);
                                    }
                                }

                                const std::string tagsAttribute =
                                    GetAttribute(relationNode, "tags");

                                for( const std::string& rawTag :
                                    splitText(tagsAttribute, ',') )
                                {
                                    const std::string tag = trimText(rawTag);

                                    if( !tag.empty() )
                                    {
                                        appendUnique(relationTags, tag);
                                    }
                                }
                            }
                        }

                        for( pugi::xml_node relationNode =
                            wordNode.child("accepts");
                            relationNode;
                            relationNode =
                            relationNode.next_sibling("accepts") )
                        {
                            const std::string role =
                                GetAttribute(relationNode, "role");

                            if( !role.empty() )
                            {
                                std::vector<std::string>& relationTags =
                                    word.relations[role];

                                for( pugi::xml_node tagNode =
                                    relationNode.child("tag");
                                    tagNode;
                                    tagNode = tagNode.next_sibling("tag") )
                                {
                                    const std::string tag =
                                        trimText(GetText(tagNode));

                                    if( !tag.empty() )
                                    {
                                        appendUnique(relationTags, tag);
                                    }
                                }
                            }
                        }

                        const pugi::xml_node formsNode = wordNode.child("forms");

                        for( pugi::xml_node formNode = formsNode.child("form");
                            formNode;
                            formNode = formNode.next_sibling("form") )
                        {
                            FormData form;
                            form.name = GetAttribute(formNode, "name");
                            form.style = GetAttribute(formNode, "style");
                            form.context = GetAttribute(formNode, "context");
                            form.prompt = GetAttribute(formNode, "prompt");
                            form.promptMasculine = GetAttribute(formNode, "prompt_m");
                            form.promptFeminine = GetAttribute(formNode, "prompt_f");
                            form.promptNeuter = GetAttribute(formNode, "prompt_n");
                            form.promptPlural = GetAttribute(formNode, "prompt_p");
                            form.value = trimText(GetText(formNode));

                            if( form.context.empty() )
                            {
                                form.context = "none";
                            }

                            if( !form.name.empty() && !form.value.empty() )
                            {
                                word.forms[form.name] = std::move(form);
                            }
                        }

                        if( !word.id.empty() &&
                            !word.kana.empty() &&
                            !word.forms.empty() )
                        {
                            words.push_back(std::move(word));
                        }
                    }

                    if( !words.empty() )
                    {
                        m_dictionaries[dictionaryId] = std::move(words);
                        result = true;
                    }
                }

                if( !result )
                {
                    m_logger.log(
                        "Could not load dictionary '" + dictionaryId +
                        "' from path: " + fullPath +
                        ". Parser status: " + parseResult.description(),
                        tools::LogLevel::PROBLEM);
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::loadVocabulary(
                const std::string& fullPath)
            {
                bool result = false;
                pugi::xml_document document;
                const pugi::xml_parse_result parseResult =
                    document.load_file(fullPath.c_str());

                if( parseResult )
                {
                    const pugi::xml_node rootNode =
                        document.child("sentence_vocabulary");
                    const pugi::xml_node profilesNode =
                        rootNode.child("tag_profiles");

                    for( pugi::xml_node profileNode =
                        profilesNode.child("profile");
                        profileNode;
                        profileNode = profileNode.next_sibling("profile") )
                    {
                        TagProfile profile;
                        profile.id = GetAttribute(profileNode, "id");
                        const pugi::xml_node includesNode =
                            profileNode.child("includes");

                        for( pugi::xml_node includedNode =
                            includesNode.child("profile");
                            includedNode;
                            includedNode = includedNode.next_sibling("profile") )
                        {
                            const std::string includedId =
                                GetAttribute(includedNode, "ref");

                            if( !includedId.empty() )
                            {
                                appendUnique(profile.includes, includedId);
                            }
                        }

                        const pugi::xml_node tagsNode = profileNode.child("tags");

                        for( pugi::xml_node tagNode = tagsNode.child("tag");
                            tagNode;
                            tagNode = tagNode.next_sibling("tag") )
                        {
                            const std::string tag = trimText(GetText(tagNode));

                            if( !tag.empty() )
                            {
                                appendUnique(profile.tags, tag);
                            }
                        }

                        if( !profile.id.empty() )
                        {
                            m_tagProfiles[profile.id] = std::move(profile);
                        }
                    }

                    const pugi::xml_node entitiesNode = rootNode.child("entities");

                    for( pugi::xml_node entityNode = entitiesNode.child("entity");
                        entityNode;
                        entityNode = entityNode.next_sibling("entity") )
                    {
                        EntityData entity;
                        entity.id = GetAttribute(entityNode, "id");
                        entity.kana = GetAttribute(entityNode, "kana");
                        entity.kanji = GetAttribute(entityNode, "kanji");
                        const pugi::xml_node polishNode =
                            entityNode.child("polish");
                        entity.gender = GetAttribute(polishNode, "gender");

                        for( pugi::xml_attribute attribute =
                            polishNode.first_attribute();
                            attribute;
                            attribute = attribute.next_attribute() )
                        {
                            if( std::string(attribute.name()) != "gender" )
                            {
                                entity.polish[attribute.name()] =
                                    attribute.value();
                            }
                        }

                        const pugi::xml_node profilesNodeForEntity =
                            entityNode.child("profiles");

                        for( pugi::xml_node profileNode =
                            profilesNodeForEntity.child("profile");
                            profileNode;
                            profileNode = profileNode.next_sibling("profile") )
                        {
                            const std::string profileId =
                                GetAttribute(profileNode, "ref");

                            if( !profileId.empty() )
                            {
                                appendUnique(entity.profiles, profileId);
                            }
                        }

                        const pugi::xml_node tagsNode = entityNode.child("tags");

                        for( pugi::xml_node tagNode = tagsNode.child("tag");
                            tagNode;
                            tagNode = tagNode.next_sibling("tag") )
                        {
                            const std::string tag = trimText(GetText(tagNode));

                            if( !tag.empty() )
                            {
                                appendUnique(entity.tags, tag);
                            }
                        }

                        const pugi::xml_node categoriesNode =
                            entityNode.child("categories");

                        for( pugi::xml_node categoryNode =
                            categoriesNode.child("category");
                            categoryNode;
                            categoryNode = categoryNode.next_sibling("category") )
                        {
                            const std::string category =
                                trimText(GetText(categoryNode));

                            if( !category.empty() )
                            {
                                appendUnique(entity.tags, category);
                            }
                        }

                        if( !entity.id.empty() && !entity.kana.empty() )
                        {
                            m_entities[entity.id] = std::move(entity);
                        }
                    }

                    result = !m_entities.empty();
                }

                if( !result )
                {
                    m_logger.log(
                        "Could not load sentence vocabulary from path: " +
                        fullPath +
                        ". Parser status: " +
                        parseResult.description(),
                        tools::LogLevel::PROBLEM);
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::loadContexts(
                const std::string& fullPath)
            {
                bool result = false;
                pugi::xml_document document;
                const pugi::xml_parse_result parseResult =
                    document.load_file(fullPath.c_str());

                if( parseResult )
                {
                    const pugi::xml_node rootNode = document.child("contexts");

                    for( pugi::xml_node contextNode = rootNode.child("context");
                        contextNode;
                        contextNode = contextNode.next_sibling("context") )
                    {
                        ContextPoolData pool;
                        pool.id = GetAttribute(contextNode, "id");
                        pool.label = GetAttribute(contextNode, "label");

                        for( pugi::xml_node optionNode =
                            contextNode.child("option");
                            optionNode;
                            optionNode = optionNode.next_sibling("option") )
                        {
                            ContextOptionData option;
                            option.id = GetAttribute(optionNode, "id");
                            option.polishPrefix =
                                GetAttribute(optionNode, "pl_prefix");
                            option.japanesePrefix =
                                GetAttribute(optionNode, "jp_prefix");
                            option.weight = std::max(
                                1U,
                                GetUnsignedAttribute(optionNode, "weight", 1U));

                            if( !option.id.empty() )
                            {
                                pool.options.push_back(std::move(option));
                            }
                        }

                        if( !pool.id.empty() && !pool.options.empty() )
                        {
                            m_contexts[pool.id] = std::move(pool);
                        }
                    }

                    result = (m_contexts.find("none") != m_contexts.end());
                }

                if( !result )
                {
                    m_logger.log(
                        "Could not load quiz contexts from path: " +
                        fullPath +
                        ". Parser status: " +
                        parseResult.description(),
                        tools::LogLevel::PROBLEM);
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::loadSentencePatterns(
                const std::string& fullPath)
            {
                bool result = false;
                pugi::xml_document document;
                const pugi::xml_parse_result parseResult =
                    document.load_file(fullPath.c_str());

                if( parseResult )
                {
                    const pugi::xml_node patternsNode =
                        document.child("sentence_maps")
                        .child("sentence_patterns");

                    for( pugi::xml_node patternNode =
                        patternsNode.child("sentence_pattern");
                        patternNode;
                        patternNode =
                        patternNode.next_sibling("sentence_pattern") )
                    {
                        SentencePatternData pattern;
                        pattern.id = GetAttribute(patternNode, "id");
                        pattern.label = GetAttribute(patternNode, "label");
                        pattern.questionTemplate =
                            trimText(GetText(patternNode.child("question")));
                        pattern.answerTemplate =
                            trimText(GetText(patternNode.child("answer")));

                        if( !pattern.id.empty() &&
                            !pattern.questionTemplate.empty() &&
                            !pattern.answerTemplate.empty() )
                        {
                            m_sentencePatterns.push_back(std::move(pattern));
                        }
                    }

                    result = !m_sentencePatterns.empty();
                }

                if( !result )
                {
                    m_logger.log(
                        "Could not load sentence patterns from path: " +
                        fullPath +
                        ". Parser status: " +
                        parseResult.description(),
                        tools::LogLevel::PROBLEM);
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::buildEffectiveEntityTags()
            {
                bool result = true;

                for( const auto& entityEntry : m_entities )
                {
                    const EntityData& entity = entityEntry.second;
                    std::vector<std::string> effectiveTags;

                    for( const std::string& profileId : entity.profiles )
                    {
                        std::vector<std::string> recursionPath;
                        std::string error;

                        if( !appendProfileTags(
                            profileId,
                            recursionPath,
                            effectiveTags,
                            error) )
                        {
                            result = false;
                            m_statusMessage = error;
                            m_logger.log(
                                "AvailabilitySentenceQuizWidget profile error: " +
                                error,
                                tools::LogLevel::PROBLEM);
                        }
                    }

                    for( const std::string& tag : entity.tags )
                    {
                        appendUnique(effectiveTags, tag);
                    }

                    if( result )
                    {
                        m_effectiveEntityTags[entity.id] =
                            std::move(effectiveTags);
                    }
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::appendProfileTags(
                const std::string& profileId,
                std::vector<std::string>& recursionPath,
                std::vector<std::string>& tags,
                std::string& error) const
            {
                bool result = false;
                const auto profileIterator = m_tagProfiles.find(profileId);

                if( profileIterator == m_tagProfiles.end() )
                {
                    error = "Unknown tag profile: " + profileId;
                }
                else if( containsValue(recursionPath, profileId) )
                {
                    error = "Tag profile inheritance cycle at: " + profileId;
                }
                else
                {
                    result = true;
                    recursionPath.push_back(profileId);

                    for( const std::string& includedProfile :
                        profileIterator->second.includes )
                    {
                        if( !appendProfileTags(
                            includedProfile,
                            recursionPath,
                            tags,
                            error) )
                        {
                            result = false;
                        }
                    }

                    if( result )
                    {
                        for( const std::string& tag :
                            profileIterator->second.tags )
                        {
                            appendUnique(tags, tag);
                        }
                    }

                    recursionPath.pop_back();
                }

                return result;
            }

            AvailabilitySentenceQuizWidget::PatternValidation
                AvailabilitySentenceQuizWidget::validatePattern(
                    const SentencePatternData& pattern) const
            {
                PatternValidation validation;

                if( pattern.id == "verb_noun_ga_aru_simple" )
                {
                    validation.kind = PatternKind::Simple;
                }
                else if( pattern.id == "verb_noun_ga_aru_object" )
                {
                    validation.kind = PatternKind::Object;
                }
                else if( pattern.id == "verb_noun_ga_aru_destination" )
                {
                    validation.kind = PatternKind::Destination;
                }
                else if( pattern.id == "verb_noun_ga_aru_companion" )
                {
                    validation.kind = PatternKind::Companion;
                }
                else if( pattern.id == "verb_noun_ga_aru_place" )
                {
                    validation.kind = PatternKind::Place;
                }

                if( validation.kind != PatternKind::Invalid )
                {
                    const std::vector<std::string> tokens =
                        extractTokens(
                            pattern.questionTemplate +
                            "\n" +
                            pattern.answerTemplate);
                    const std::string role = getRoleName(validation.kind);
                    bool hasMainDictionary = false;
                    bool hasAvailabilityNoun = false;
                    bool hasExistenceForm = false;
                    bool hasRole = role.empty();

                    for( const std::string& token : tokens )
                    {
                        std::string root;
                        std::string field;
                        splitToken(token, root, field);

                        hasMainDictionary = hasMainDictionary ||
                            ((root == "verb_main") &&
                                (field == "dictionary"));
                        hasAvailabilityNoun = hasAvailabilityNoun ||
                            (root == "availability_noun");
                        hasExistenceForm = hasExistenceForm ||
                            ((root == "existence_verb") &&
                                (field == "form"));
                        hasRole = hasRole || (root == role);

                        const bool commonRoot =
                            (root == "context") ||
                            (root == "verb_main") ||
                            (root == "availability_noun") ||
                            (root == "existence_verb");
                        const bool roleRoot =
                            !role.empty() && (root == role);

                        if( !commonRoot && !roleRoot )
                        {
                            validation.error =
                                "Pattern '" + pattern.id +
                                "' contains unsupported slot '" +
                                root + "'.";
                        }
                    }

                    if( validation.error.empty() && !hasMainDictionary )
                    {
                        validation.error =
                            "Pattern '" + pattern.id +
                            "' has no {verb_main.dictionary} token.";
                    }
                    else if( validation.error.empty() && !hasAvailabilityNoun )
                    {
                        validation.error =
                            "Pattern '" + pattern.id +
                            "' has no availability_noun token.";
                    }
                    else if( validation.error.empty() && !hasExistenceForm )
                    {
                        validation.error =
                            "Pattern '" + pattern.id +
                            "' has no {existence_verb.form} token.";
                    }
                    else if( validation.error.empty() && !hasRole )
                    {
                        validation.error =
                            "Pattern '" + pattern.id +
                            "' has no required {" + role + "} token.";
                    }

                    validation.valid = validation.error.empty();
                }

                return validation;
            }

            const AvailabilitySentenceQuizWidget::SentencePatternData*
                AvailabilitySentenceQuizWidget::findValidPattern(
                    const std::string& id) const
            {
                const SentencePatternData* result = nullptr;

                for( const SentencePatternData& pattern : m_sentencePatterns )
                {
                    if( (pattern.id == id) &&
                        validatePattern(pattern).valid )
                    {
                        result = &pattern;
                    }
                }

                return result;
            }

            uint32_t AvailabilitySentenceQuizWidget::countValidPatterns() const
            {
                uint32_t result = 0U;

                for( const SentencePatternData& pattern : m_sentencePatterns )
                {
                    if( validatePattern(pattern).valid )
                    {
                        ++result;
                    }
                }

                return result;
            }

            std::string AvailabilitySentenceQuizWidget::getPatternId(
                PatternKind kind) const
            {
                std::string result;

                if( kind == PatternKind::Simple )
                {
                    result = "verb_noun_ga_aru_simple";
                }
                else if( kind == PatternKind::Object )
                {
                    result = "verb_noun_ga_aru_object";
                }
                else if( kind == PatternKind::Destination )
                {
                    result = "verb_noun_ga_aru_destination";
                }
                else if( kind == PatternKind::Companion )
                {
                    result = "verb_noun_ga_aru_companion";
                }
                else if( kind == PatternKind::Place )
                {
                    result = "verb_noun_ga_aru_place";
                }

                return result;
            }

            std::string AvailabilitySentenceQuizWidget::getRoleName(
                PatternKind kind) const
            {
                std::string result;

                if( kind == PatternKind::Object )
                {
                    result = "object";
                }
                else if( kind == PatternKind::Destination )
                {
                    result = "destination";
                }
                else if( kind == PatternKind::Companion )
                {
                    result = "companion";
                }
                else if( kind == PatternKind::Place )
                {
                    result = "place";
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::isPatternEnabled(
                PatternKind kind) const
            {
                bool result = false;

                if( kind == PatternKind::Simple )
                {
                    result = m_enableSimple;
                }
                else if( kind == PatternKind::Object )
                {
                    result = m_enableObject;
                }
                else if( kind == PatternKind::Destination )
                {
                    result = m_enableDestination;
                }
                else if( kind == PatternKind::Companion )
                {
                    result = m_enableCompanion;
                }
                else if( kind == PatternKind::Place )
                {
                    result = m_enablePlace;
                }

                return result;
            }

            std::vector<AvailabilitySentenceQuizWidget::GenerationRule>
                AvailabilitySentenceQuizWidget::buildEnabledRules() const
            {
                std::vector<GenerationRule> rules;

                const auto appendStyle =
                    [this, &rules](bool enabled, bool polite)
                    {
                        if( enabled )
                        {
                            if( m_enableNonPast )
                            {
                                rules.push_back(
                                    {
                                        polite ? "polite_nonpast" : "dictionary",
                                        polite ?
                                            "polite non-past ある" :
                                            "plain non-past ある"
                                    });
                            }

                            if( m_enableNegative )
                            {
                                rules.push_back(
                                    {
                                        polite ? "polite_negative" : "plain_negative",
                                        polite ?
                                            "polite non-past negative ある" :
                                            "plain non-past negative ある"
                                    });
                            }

                            if( m_enablePast )
                            {
                                rules.push_back(
                                    {
                                        polite ? "past_polite" : "past_plain",
                                        polite ?
                                            "polite past ある" :
                                            "plain past ある"
                                    });
                            }

                            if( m_enablePastNegative )
                            {
                                rules.push_back(
                                    {
                                        polite ?
                                            "past_negative_polite" :
                                            "past_negative_plain",
                                        polite ?
                                            "polite past negative ある" :
                                            "plain past negative ある"
                                    });
                            }
                        }
                    };

                appendStyle(m_enablePlainOutput, false);
                appendStyle(m_enablePoliteOutput, true);
                return rules;
            }

            std::vector<const AvailabilitySentenceQuizWidget::DictionaryWord*>
                AvailabilitySentenceQuizWidget::getMainVerbCandidates(
                    PatternKind kind,
                    const std::vector<WordFilterRule>& filters) const
            {
                std::vector<const DictionaryWord*> result;
                const auto verbsIterator = m_dictionaries.find("verbs");
                const std::string role = getRoleName(kind);

                if( verbsIterator != m_dictionaries.end() )
                {
                    for( const DictionaryWord& word : verbsIterator->second )
                    {
                        bool compatible =
                            (getForm(word, "dictionary") != nullptr) &&
                            wordMatchesFilter(word, filters);

                        if( compatible && !role.empty() )
                        {
                            compatible =
                                !getCompatibleRoleEntities(word, role).empty();
                        }

                        if( compatible )
                        {
                            result.push_back(&word);
                        }
                    }
                }

                return result;
            }

            std::vector<const AvailabilitySentenceQuizWidget::DictionaryWord*>
                AvailabilitySentenceQuizWidget::getExistenceVerbs() const
            {
                std::vector<const DictionaryWord*> result;
                const auto verbsIterator = m_dictionaries.find("verbs");

                if( verbsIterator != m_dictionaries.end() )
                {
                    for( const DictionaryWord& word : verbsIterator->second )
                    {
                        if( wordHasTag(word, "existence") )
                        {
                            result.push_back(&word);
                        }
                    }
                }

                return result;
            }

            std::vector<const AvailabilitySentenceQuizWidget::EntityData*>
                AvailabilitySentenceQuizWidget::getAvailabilityNouns() const
            {
                std::vector<const EntityData*> result;

                for( const auto& entityEntry : m_entities )
                {
                    const auto tagsIterator =
                        m_effectiveEntityTags.find(entityEntry.first);

                    if( (tagsIterator != m_effectiveEntityTags.end()) &&
                        containsValue(
                            tagsIterator->second,
                            "availability_noun") )
                    {
                        result.push_back(&entityEntry.second);
                    }
                }

                std::sort(
                    result.begin(),
                    result.end(),
                    [](const EntityData* first, const EntityData* second)
                    {
                        return first->kana < second->kana;
                    });
                return result;
            }

            std::vector<const AvailabilitySentenceQuizWidget::EntityData*>
                AvailabilitySentenceQuizWidget::getCompatibleRoleEntities(
                    const DictionaryWord& word,
                    const std::string& role) const
            {
                std::vector<const EntityData*> result;

                for( const auto& entityEntry : m_entities )
                {
                    if( entityMatchesRelation(
                        entityEntry.second,
                        word,
                        role) )
                    {
                        result.push_back(&entityEntry.second);
                    }
                }

                std::sort(
                    result.begin(),
                    result.end(),
                    [](const EntityData* first, const EntityData* second)
                    {
                        return first->kana < second->kana;
                    });
                return result;
            }

            bool AvailabilitySentenceQuizWidget::entityMatchesRelation(
                const EntityData& entity,
                const DictionaryWord& word,
                const std::string& role) const
            {
                bool result = false;
                const auto relationIterator = word.relations.find(role);
                const auto tagsIterator = m_effectiveEntityTags.find(entity.id);

                if( (relationIterator != word.relations.end()) &&
                    (tagsIterator != m_effectiveEntityTags.end()) )
                {
                    result = tagsIntersect(
                        relationIterator->second,
                        tagsIterator->second);
                }

                return result;
            }

            std::vector<AvailabilitySentenceQuizWidget::QuestionSpace>
                AvailabilitySentenceQuizWidget::buildQuestionSpaces() const
            {
                std::vector<QuestionSpace> spaces;
                const std::vector<GenerationRule> rules = buildEnabledRules();
                const std::vector<const DictionaryWord*> existenceVerbs =
                    getExistenceVerbs();
                const std::vector<const EntityData*> availabilityNouns =
                    getAvailabilityNouns();
                const std::vector<WordFilterRule> filters =
                    parseWordFilter(std::string(m_wordFilter));
                const PatternKind kinds[] =
                {
                    PatternKind::Simple,
                    PatternKind::Object,
                    PatternKind::Destination,
                    PatternKind::Companion,
                    PatternKind::Place
                };

                if( !rules.empty() &&
                    !existenceVerbs.empty() &&
                    !availabilityNouns.empty() )
                {
                    for( const PatternKind kind : kinds )
                    {
                        const SentencePatternData* pattern = nullptr;

                        if( isPatternEnabled(kind) )
                        {
                            pattern = findValidPattern(getPatternId(kind));
                        }

                        if( pattern != nullptr )
                        {
                            const std::vector<const DictionaryWord*> mainVerbs =
                                getMainVerbCandidates(kind, filters);
                            const std::string role = getRoleName(kind);

                            for( const DictionaryWord* mainVerb : mainVerbs )
                            {
                                std::vector<const EntityData*> roleEntities;

                                if( !role.empty() )
                                {
                                    roleEntities = getCompatibleRoleEntities(
                                        *mainVerb,
                                        role);
                                }

                                const uint64_t roleCount = role.empty() ?
                                    1U :
                                    static_cast<uint64_t>(roleEntities.size());

                                if( roleCount > 0U )
                                {
                                    for( const DictionaryWord* existenceVerb :
                                        existenceVerbs )
                                    {
                                        for( const GenerationRule& rule : rules )
                                        {
                                            const FormData* form =
                                                getForm(*existenceVerb, rule.formName);

                                            if( form != nullptr )
                                            {
                                                const ContextPoolData* contextPool =
                                                    getContextPool(form->context);

                                                if( contextPool != nullptr )
                                                {
                                                    QuestionSpace space;
                                                    space.pattern = pattern;
                                                    space.kind = kind;
                                                    space.mainVerb = mainVerb;
                                                    space.existenceVerb = existenceVerb;
                                                    space.contextPool = contextPool;
                                                    space.rule = rule;
                                                    space.availabilityNouns =
                                                        availabilityNouns;
                                                    space.roleEntities =
                                                        roleEntities;
                                                    space.questionCount = SafeMultiply(
                                                        static_cast<uint64_t>(
                                                            availabilityNouns.size()),
                                                        roleCount);
                                                    space.questionCount = SafeMultiply(
                                                        space.questionCount,
                                                        static_cast<uint64_t>(
                                                            contextPool->options.size()));

                                                    if( space.questionCount > 0U )
                                                    {
                                                        spaces.push_back(
                                                            std::move(space));
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                return spaces;
            }

            std::vector<AvailabilitySentenceQuizWidget::QuizQuestion>
                AvailabilitySentenceQuizWidget::buildQuestions(
                    size_t maximumQuestions) const
            {
                std::vector<QuizQuestion> questions;
                const std::vector<QuestionSpace> spaces = buildQuestionSpaces();
                uint64_t total = 0U;

                for( const QuestionSpace& space : spaces )
                {
                    if( total >
                        (std::numeric_limits<uint64_t>::max() -
                            space.questionCount) )
                    {
                        total = std::numeric_limits<uint64_t>::max();
                    }
                    else
                    {
                        total += space.questionCount;
                    }
                }

                const std::vector<uint64_t> indices =
                    chooseQuestionIndices(total, maximumQuestions);
                questions.reserve(indices.size());

                for( const uint64_t index : indices )
                {
                    QuestionSeed seed;
                    QuizQuestion question;

                    if( seedFromGlobalIndex(spaces, index, seed) &&
                        renderQuestionSeed(seed, question) )
                    {
                        questions.push_back(std::move(question));
                    }
                }

                return questions;
            }

            bool AvailabilitySentenceQuizWidget::seedFromGlobalIndex(
                const std::vector<QuestionSpace>& spaces,
                uint64_t globalIndex,
                QuestionSeed& seed) const
            {
                bool result = false;

                for( const QuestionSpace& space : spaces )
                {
                    if( !result && (globalIndex < space.questionCount) )
                    {
                        const uint64_t contextCount =
                            static_cast<uint64_t>(
                                space.contextPool->options.size());
                        const uint64_t nounCount =
                            static_cast<uint64_t>(
                                space.availabilityNouns.size());
                        const uint64_t roleCount =
                            space.roleEntities.empty() ?
                            1U :
                            static_cast<uint64_t>(space.roleEntities.size());

                        if( (contextCount > 0U) &&
                            (nounCount > 0U) &&
                            (roleCount > 0U) )
                        {
                            uint64_t localIndex = globalIndex;
                            const size_t contextIndex =
                                static_cast<size_t>(localIndex % contextCount);
                            localIndex /= contextCount;
                            const size_t nounIndex =
                                static_cast<size_t>(localIndex % nounCount);
                            localIndex /= nounCount;
                            const size_t roleIndex =
                                static_cast<size_t>(localIndex % roleCount);

                            seed.space = &space;
                            seed.availabilityNoun =
                                space.availabilityNouns[nounIndex];
                            seed.roleEntity =
                                space.roleEntities.empty() ?
                                nullptr :
                                space.roleEntities[roleIndex];
                            seed.contextOption =
                                &space.contextPool->options[contextIndex];
                            result = true;
                        }
                    }
                    else if( !result )
                    {
                        globalIndex -= space.questionCount;
                    }
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::renderQuestionSeed(
                const QuestionSeed& seed,
                QuizQuestion& question) const
            {
                bool result = false;

                if( (seed.space != nullptr) &&
                    (seed.availabilityNoun != nullptr) &&
                    (seed.contextOption != nullptr) )
                {
                    std::string source;
                    std::string answer;
                    std::string error;

                    if( renderPattern(seed, source, answer, error) )
                    {
                        const std::string role =
                            getRoleName(seed.space->kind);
                        const std::string roleBinding =
                            (seed.roleEntity != nullptr) ?
                            std::format(
                                ", {}={}",
                                role,
                                seed.roleEntity->id) :
                            std::string();

                        question.key = std::format(
                            "{}|{}|{}|{}|{}|{}",
                            seed.space->pattern->id,
                            seed.space->mainVerb->id,
                            seed.availabilityNoun->id,
                            seed.space->existenceVerb->id,
                            seed.space->rule.formName,
                            seed.contextOption->id) +
                            roleBinding;
                        question.sourceSentence = source;
                        question.targetSentence = answer;
                        question.targetLabel = seed.space->rule.label;
                        question.patternId = seed.space->pattern->id;
                        question.mainVerbMeaning =
                            seed.space->mainVerb->translation;
                        question.mainVerbKana = seed.space->mainVerb->kana;
                        question.availabilityNounKana =
                            seed.availabilityNoun->kana;
                        question.availabilityNounKanji =
                            seed.availabilityNoun->kanji;
                        question.existenceForm = seed.space->rule.formName;
                        question.bindings = std::format(
                            "verb_main={}, availability_noun={}, existence_verb={}{}",
                            seed.space->mainVerb->id,
                            seed.availabilityNoun->id,
                            seed.space->existenceVerb->id,
                            roleBinding);
                        result = true;
                    }
                    else
                    {
                        m_logger.log(
                            "AvailabilitySentenceQuizWidget render error: " +
                            error,
                            tools::LogLevel::PROBLEM);
                    }
                }

                return result;
            }

            uint64_t AvailabilitySentenceQuizWidget::countPossibleQuestions() const
            {
                uint64_t result = 0U;
                const std::vector<QuestionSpace> spaces = buildQuestionSpaces();

                for( const QuestionSpace& space : spaces )
                {
                    if( result >
                        (std::numeric_limits<uint64_t>::max() -
                            space.questionCount) )
                    {
                        result = std::numeric_limits<uint64_t>::max();
                    }
                    else
                    {
                        result += space.questionCount;
                    }
                }

                return result;
            }

            std::vector<uint64_t>
                AvailabilitySentenceQuizWidget::chooseQuestionIndices(
                    uint64_t total,
                    size_t maximumQuestions) const
            {
                std::vector<uint64_t> result;

                if( (total > 0U) && (maximumQuestions > 0U) )
                {
                    const uint64_t wanted = std::min(
                        total,
                        static_cast<uint64_t>(maximumQuestions));
                    result.reserve(static_cast<size_t>(wanted));

                    if( !m_randomOrder )
                    {
                        for( uint64_t index = 0U; index < wanted; ++index )
                        {
                            result.push_back(index);
                        }
                    }
                    else if( wanted == total )
                    {
                        for( uint64_t index = 0U; index < total; ++index )
                        {
                            result.push_back(index);
                        }

                        std::shuffle(result.begin(), result.end(), m_rng);
                    }
                    else
                    {
                        std::unordered_set<uint64_t> selected;
                        std::uniform_int_distribution<uint64_t> distribution(
                            0U,
                            total - 1U);

                        while( result.size() < static_cast<size_t>(wanted) )
                        {
                            const uint64_t index = distribution(m_rng);

                            if( selected.insert(index).second )
                            {
                                result.push_back(index);
                            }
                        }
                    }
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::renderPattern(
                const QuestionSeed& seed,
                std::string& question,
                std::string& answer,
                std::string& error) const
            {
                bool result = false;

                if( (seed.space != nullptr) &&
                    (seed.space->pattern != nullptr) )
                {
                    const bool questionRendered = renderTemplate(
                        seed.space->pattern->questionTemplate,
                        seed,
                        question,
                        error);
                    bool answerRendered = false;

                    if( questionRendered )
                    {
                        answerRendered = renderTemplate(
                            seed.space->pattern->answerTemplate,
                            seed,
                            answer,
                            error);
                    }

                    result = questionRendered && answerRendered;
                }
                else
                {
                    error = "Question seed has no sentence pattern.";
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::renderTemplate(
                const std::string& text,
                const QuestionSeed& seed,
                std::string& rendered,
                std::string& error) const
            {
                bool result = true;
                rendered = text;
                const std::vector<std::string> tokens = extractTokens(text);

                for( const std::string& expression : tokens )
                {
                    std::string value;

                    if( result && resolveToken(
                        expression,
                        seed,
                        value,
                        error) )
                    {
                        rendered = replaceAll(
                            rendered,
                            "{" + expression + "}",
                            value);
                    }
                    else
                    {
                        result = false;
                    }
                }

                if( result &&
                    ((rendered.find('{') != std::string::npos) ||
                        (rendered.find('}') != std::string::npos)) )
                {
                    result = false;
                    error = "Rendered template still contains unresolved tokens.";
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::resolveToken(
                const std::string& expression,
                const QuestionSeed& seed,
                std::string& value,
                std::string& error) const
            {
                bool result = false;
                std::string root;
                std::string field;
                splitToken(expression, root, field);

                if( (seed.space == nullptr) ||
                    (seed.contextOption == nullptr) ||
                    (seed.availabilityNoun == nullptr) )
                {
                    error = "Incomplete question seed while resolving token.";
                }
                else if( root == "context" )
                {
                    if( field == "id" )
                    {
                        value = seed.space->contextPool->id;
                        result = true;
                    }
                    else if( field == "option" )
                    {
                        value = seed.contextOption->id;
                        result = true;
                    }
                    else if( field == "pl_prefix" )
                    {
                        value = seed.contextOption->polishPrefix;
                        result = true;
                    }
                    else if( field == "jp_prefix" )
                    {
                        value = seed.contextOption->japanesePrefix;
                        result = true;
                    }
                    else
                    {
                        error = "Unknown context field: " + field;
                    }
                }
                else if( root == "verb_main" )
                {
                    result = resolveWordToken(
                        *seed.space->mainVerb,
                        field,
                        "dictionary",
                        false,
                        value,
                        error);
                }
                else if( root == "existence_verb" )
                {
                    result = resolveWordToken(
                        *seed.space->existenceVerb,
                        field,
                        seed.space->rule.formName,
                        true,
                        value,
                        error);
                }
                else if( root == "availability_noun" )
                {
                    result = resolveEntityToken(
                        *seed.availabilityNoun,
                        field,
                        value,
                        error);
                }
                else if( root == getRoleName(seed.space->kind) )
                {
                    if( seed.roleEntity != nullptr )
                    {
                        result = resolveEntityToken(
                            *seed.roleEntity,
                            field,
                            value,
                            error);
                    }
                    else
                    {
                        error = "Pattern role has no selected entity: " + root;
                    }
                }
                else
                {
                    error = "Unsupported token root: " + root;
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::resolveWordToken(
                const DictionaryWord& word,
                const std::string& field,
                const std::string& selectedForm,
                bool dynamicForm,
                std::string& value,
                std::string& error) const
            {
                bool result = true;
                const FormData* selected = getForm(word, selectedForm);

                if( field.empty() )
                {
                    value = word.kana;
                }
                else if( field == "id" )
                {
                    value = word.id;
                }
                else if( field == "kana" )
                {
                    value = word.kana;
                }
                else if( field == "kanji" )
                {
                    value = word.kanji;
                }
                else if( field == "translation" )
                {
                    value = word.translation;
                }
                else if( field == "type" )
                {
                    value = word.type;
                }
                else if( field == "ending" )
                {
                    value = word.ending;
                }
                else if( field == "form" )
                {
                    if( dynamicForm && (selected != nullptr) )
                    {
                        value = selected->value;
                    }
                    else
                    {
                        result = false;
                    }
                }
                else if( field == "prompt" )
                {
                    if( selected != nullptr )
                    {
                        value = selected->prompt;
                    }
                    else
                    {
                        result = false;
                    }
                }
                else if( field == "style" )
                {
                    if( selected != nullptr )
                    {
                        value = selected->style;
                    }
                    else
                    {
                        result = false;
                    }
                }
                else if( field == "form_name" )
                {
                    if( selected != nullptr )
                    {
                        value = selected->name;
                    }
                    else
                    {
                        result = false;
                    }
                }
                else
                {
                    const FormData* explicitForm = getForm(word, field);

                    if( explicitForm != nullptr )
                    {
                        value = explicitForm->value;
                    }
                    else
                    {
                        result = false;
                    }
                }

                if( !result )
                {
                    error =
                        "Unsupported or unavailable word field '" +
                        field +
                        "' for word '" +
                        word.id +
                        "'.";
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::resolveEntityToken(
                const EntityData& entity,
                const std::string& field,
                std::string& value,
                std::string& error) const
            {
                bool result = true;

                if( field.empty() )
                {
                    value = entity.kana;
                }
                else if( field == "id" )
                {
                    value = entity.id;
                }
                else if( field == "kana" )
                {
                    value = entity.kana;
                }
                else if( field == "kanji" )
                {
                    value = entity.kanji;
                }
                else if( field == "gender" )
                {
                    value = entity.gender;
                }
                else if( field.rfind("pl_", 0U) == 0U )
                {
                    const auto polishIterator =
                        entity.polish.find(field.substr(3U));

                    if( polishIterator != entity.polish.end() )
                    {
                        value = polishIterator->second;
                    }
                    else
                    {
                        result = false;
                    }
                }
                else
                {
                    result = false;
                }

                if( !result )
                {
                    error =
                        "Unsupported or unavailable entity field '" +
                        field +
                        "' for entity '" +
                        entity.id +
                        "'.";
                }

                return result;
            }

            const AvailabilitySentenceQuizWidget::FormData*
                AvailabilitySentenceQuizWidget::getForm(
                    const DictionaryWord& word,
                    const std::string& formName) const
            {
                const FormData* result = nullptr;
                const auto iterator = word.forms.find(formName);

                if( iterator != word.forms.end() )
                {
                    result = &iterator->second;
                }

                return result;
            }

            const AvailabilitySentenceQuizWidget::ContextPoolData*
                AvailabilitySentenceQuizWidget::getContextPool(
                    const std::string& contextId) const
            {
                const ContextPoolData* result = nullptr;
                const auto iterator = m_contexts.find(contextId);

                if( iterator != m_contexts.end() )
                {
                    result = &iterator->second;
                }

                return result;
            }

            void AvailabilitySentenceQuizWidget::startQuiz()
            {
                const bool loaded = loadFiles();

                if( loaded )
                {
                    const size_t requestedSize =
                        (m_numberOfQuestions > 0) ?
                        static_cast<size_t>(m_numberOfQuestions) :
                        0U;
                    m_questions = buildQuestions(requestedSize);

                    if( !m_questions.empty() )
                    {
                        resetQuizState();
                        m_showSettingsWindow = false;
                        m_showQuizWindow = true;
                        m_statusMessage = std::format(
                            "Quiz started with {} questions.",
                            m_questions.size());
                    }
                    else
                    {
                        m_statusMessage =
                            "No questions could be generated for the current "
                            "project, sentence shapes, verb filter and form settings.";
                    }
                }
            }

            void AvailabilitySentenceQuizWidget::submitAnswer()
            {
                if( !m_waitingForNextQuestion &&
                    (m_currentQuestionIndex < m_questions.size()) )
                {
                    const QuizQuestion& question =
                        m_questions[m_currentQuestionIndex];
                    const std::string userAnswer =
                        normalizeAnswer(std::string(m_userAnswer));
                    const std::string expectedAnswer =
                        normalizeAnswer(question.targetSentence);

                    if( userAnswer == expectedAnswer )
                    {
                        ++m_correctAnswers;
                        m_feedbackMessage = std::format(
                            "Correct! Expected answer: {}",
                            question.targetSentence);
                        m_waitingForNextQuestion = true;
                        scheduleAutoAdvanceAfterCorrectAnswer();
                    }
                    else
                    {
                        ++m_currentTry;

                        if( m_currentTry >= m_numberOfTries )
                        {
                            ++m_wrongAnswers;
                            m_feedbackMessage = std::format(
                                "Incorrect. Expected answer: {}",
                                question.targetSentence);
                            m_waitingForNextQuestion = true;
                        }
                        else
                        {
                            m_feedbackMessage = std::format(
                                "Incorrect. Try again. Attempt {}/{}.",
                                m_currentTry,
                                m_numberOfTries);
                            m_focusAnswerInput = true;
                        }
                    }
                }
            }

            void AvailabilitySentenceQuizWidget::scheduleAutoAdvanceAfterCorrectAnswer()
            {
                if( m_autoAdvanceDelayMs <= 0 )
                {
                    advanceQuestion();
                }
                else
                {
                    m_autoAdvanceDueTime =
                        ImGui::GetTime() +
                        (static_cast<double>(m_autoAdvanceDelayMs) / 1000.0);
                }
            }

            void AvailabilitySentenceQuizWidget::processAutoAdvance()
            {
                if( m_waitingForNextQuestion &&
                    (m_autoAdvanceDueTime > 0.0) &&
                    (ImGui::GetTime() >= m_autoAdvanceDueTime) )
                {
                    m_autoAdvanceDueTime = 0.0;
                    advanceQuestion();
                }
            }

            void AvailabilitySentenceQuizWidget::acceptCurrentAnswerAsCorrect()
            {
                if( m_waitingForNextQuestion &&
                    (m_currentQuestionIndex < m_questions.size()) )
                {
                    if( m_wrongAnswers > 0U )
                    {
                        --m_wrongAnswers;
                    }

                    ++m_correctAnswers;
                    ++m_acceptedAnswers;
                    advanceQuestion();
                }
            }

            void AvailabilitySentenceQuizWidget::skipQuestion()
            {
                if( m_currentQuestionIndex < m_questions.size() )
                {
                    ++m_correctAnswers;
                    ++m_acceptedAnswers;
                    advanceQuestion();
                }
            }

            void AvailabilitySentenceQuizWidget::advanceQuestion()
            {
                ++m_currentQuestionIndex;
                m_currentTry = 0;
                m_waitingForNextQuestion = false;
                m_showFullAnswer = false;
                m_focusAnswerInput = true;
                m_autoAdvanceDueTime = 0.0;
                m_feedbackMessage.clear();
                std::memset(m_userAnswer, 0, sizeof(m_userAnswer));
            }

            void AvailabilitySentenceQuizWidget::resetQuizState()
            {
                m_currentQuestionIndex = 0U;
                m_correctAnswers = 0U;
                m_wrongAnswers = 0U;
                m_acceptedAnswers = 0U;
                m_currentTry = 0;
                m_waitingForNextQuestion = false;
                m_showFullAnswer = false;
                m_focusAnswerInput = true;
                m_autoAdvanceDueTime = 0.0;
                m_feedbackMessage.clear();
                std::memset(m_userAnswer, 0, sizeof(m_userAnswer));
            }

            void AvailabilitySentenceQuizWidget::updateStatistics()
            {
                const bool loaded = loadFiles();

                if( loaded )
                {
                    const auto verbsIterator = m_dictionaries.find("verbs");
                    m_statistics.verbs =
                        (verbsIterator != m_dictionaries.end()) ?
                        static_cast<uint32_t>(verbsIterator->second.size()) :
                        0U;
                    m_statistics.availabilityNouns =
                        static_cast<uint32_t>(getAvailabilityNouns().size());
                    m_statistics.sentencePatterns = countValidPatterns();
                    m_statistics.entities =
                        static_cast<uint32_t>(m_entities.size());
                    m_statistics.tagProfiles =
                        static_cast<uint32_t>(m_tagProfiles.size());
                    const std::vector<QuestionSpace> spaces =
                        buildQuestionSpaces();
                    m_statistics.combinationSpaces =
                        static_cast<uint64_t>(spaces.size());
                    m_statistics.generatedQuestions = 0U;

                    for( const QuestionSpace& space : spaces )
                    {
                        if( m_statistics.generatedQuestions >
                            (std::numeric_limits<uint64_t>::max() -
                                space.questionCount) )
                        {
                            m_statistics.generatedQuestions =
                                std::numeric_limits<uint64_t>::max();
                        }
                        else
                        {
                            m_statistics.generatedQuestions +=
                                space.questionCount;
                        }
                    }

                    m_statusMessage =
                        "Statistics refreshed from quiz_project.xml.";
                }
            }

            std::vector<AvailabilitySentenceQuizWidget::WordFilterRule>
                AvailabilitySentenceQuizWidget::parseWordFilter(
                    const std::string& filterText) const
            {
                std::vector<WordFilterRule> rules;
                const std::string trimmedFilter = trimText(filterText);

                if( !trimmedFilter.empty() )
                {
                    for( const std::string& expression :
                        splitText(trimmedFilter, ';') )
                    {
                        const std::string trimmedExpression =
                            trimText(expression);
                        const size_t equalPosition =
                            trimmedExpression.find('=');

                        if( equalPosition != std::string::npos )
                        {
                            WordFilterRule rule;
                            rule.field = trimText(
                                trimmedExpression.substr(
                                    0U,
                                    equalPosition));

                            for( const std::string& rawValue :
                                splitText(
                                    trimText(
                                        trimmedExpression.substr(
                                            equalPosition + 1U)),
                                    ',') )
                            {
                                const std::string value =
                                    removeOptionalQuotes(trimText(rawValue));

                                if( !value.empty() )
                                {
                                    rule.values.push_back(value);
                                }
                            }

                            if( !rule.field.empty() &&
                                !rule.values.empty() )
                            {
                                rules.push_back(std::move(rule));
                            }
                        }
                    }
                }

                return rules;
            }

            bool AvailabilitySentenceQuizWidget::wordMatchesFilter(
                const DictionaryWord& word,
                const std::vector<WordFilterRule>& rules) const
            {
                bool result = true;

                for( const WordFilterRule& rule : rules )
                {
                    bool ruleMatches = false;

                    for( const std::string& value : rule.values )
                    {
                        if( wordFieldMatches(word, rule.field, value) )
                        {
                            ruleMatches = true;
                        }
                    }

                    result = result && ruleMatches;
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::wordFieldMatches(
                const DictionaryWord& word,
                const std::string& field,
                const std::string& value) const
            {
                bool result = false;

                if( field == "tag" )
                {
                    result = wordHasTag(word, value);
                }
                else if( field == "dictionary" )
                {
                    result = (word.dictionaryId == value);
                }
                else
                {
                    const auto iterator = word.attributes.find(field);
                    result =
                        (iterator != word.attributes.end()) &&
                        (iterator->second == value);
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::wordHasTag(
                const DictionaryWord& word,
                const std::string& tag) const
            {
                return containsValue(word.tags, tag);
            }

            bool AvailabilitySentenceQuizWidget::tagsIntersect(
                const std::vector<std::string>& first,
                const std::vector<std::string>& second) const
            {
                bool result = false;

                for( const std::string& value : first )
                {
                    if( containsValue(second, value) )
                    {
                        result = true;
                    }
                }

                return result;
            }

            bool AvailabilitySentenceQuizWidget::containsValue(
                const std::vector<std::string>& values,
                const std::string& value) const
            {
                return
                    std::find(values.begin(), values.end(), value) !=
                    values.end();
            }

            void AvailabilitySentenceQuizWidget::appendUnique(
                std::vector<std::string>& values,
                const std::string& value) const
            {
                if( !containsValue(values, value) )
                {
                    values.push_back(value);
                }
            }

            std::vector<std::string>
                AvailabilitySentenceQuizWidget::extractTokens(
                    const std::string& text) const
            {
                std::vector<std::string> tokens;
                size_t position = 0U;

                while( position < text.size() )
                {
                    const size_t open = text.find('{', position);

                    if( open == std::string::npos )
                    {
                        position = text.size();
                    }
                    else
                    {
                        const size_t close = text.find('}', open + 1U);

                        if( close == std::string::npos )
                        {
                            position = text.size();
                        }
                        else
                        {
                            const std::string token =
                                trimText(
                                    text.substr(
                                        open + 1U,
                                        close - open - 1U));

                            if( !token.empty() )
                            {
                                appendUnique(tokens, token);
                            }

                            position = close + 1U;
                        }
                    }
                }

                return tokens;
            }

            void AvailabilitySentenceQuizWidget::splitToken(
                const std::string& expression,
                std::string& root,
                std::string& field) const
            {
                const size_t dot = expression.find('.');

                if( dot == std::string::npos )
                {
                    root = trimText(expression);
                    field.clear();
                }
                else
                {
                    root = trimText(expression.substr(0U, dot));
                    field = trimText(expression.substr(dot + 1U));
                }
            }

            std::vector<std::string> AvailabilitySentenceQuizWidget::splitText(
                const std::string& text,
                char separator) const
            {
                std::vector<std::string> parts;
                std::string current;

                for( const char character : text )
                {
                    if( character == separator )
                    {
                        parts.push_back(current);
                        current.clear();
                    }
                    else
                    {
                        current.push_back(character);
                    }
                }

                parts.push_back(current);
                return parts;
            }

            std::string AvailabilitySentenceQuizWidget::trimText(
                const std::string& text) const
            {
                size_t first = 0U;

                while( (first < text.size()) &&
                    ((text[first] == ' ') ||
                        (text[first] == '\t') ||
                        (text[first] == '\r') ||
                        (text[first] == '\n')) )
                {
                    ++first;
                }

                size_t last = text.size();

                while( (last > first) &&
                    ((text[last - 1U] == ' ') ||
                        (text[last - 1U] == '\t') ||
                        (text[last - 1U] == '\r') ||
                        (text[last - 1U] == '\n')) )
                {
                    --last;
                }

                return text.substr(first, last - first);
            }

            std::string AvailabilitySentenceQuizWidget::removeOptionalQuotes(
                const std::string& text) const
            {
                std::string result = text;

                if( text.size() >= 2U )
                {
                    const bool doubleQuoted =
                        (text.front() == '"') &&
                        (text.back() == '"');
                    const bool singleQuoted =
                        (text.front() == '\'') &&
                        (text.back() == '\'');

                    if( doubleQuoted || singleQuoted )
                    {
                        result = text.substr(1U, text.size() - 2U);
                    }
                }

                return result;
            }

            std::string AvailabilitySentenceQuizWidget::replaceAll(
                std::string text,
                const std::string& token,
                const std::string& value) const
            {
                if( !token.empty() )
                {
                    size_t position = 0U;

                    while( (position = text.find(token, position)) !=
                        std::string::npos )
                    {
                        text.replace(position, token.size(), value);
                        position += value.size();
                    }
                }

                return text;
            }

            std::string AvailabilitySentenceQuizWidget::normalizeAnswer(
                std::string text) const
            {
                static const std::vector<std::string> ignored =
                {
                    " ",
                    "\t",
                    "\r",
                    "\n",
                    ".",
                    ",",
                    "?",
                    "!",
                    "。",
                    "、",
                    "？",
                    "！"
                };

                for( const std::string& token : ignored )
                {
                    text = replaceAll(text, token, "");
                }

                return text;
            }

            std::string AvailabilitySentenceQuizWidget::buildConfiguredPath(
                const std::string& path) const
            {
                std::string result;
                const std::filesystem::path configured(path);

                if( configured.is_absolute() )
                {
                    result = configured.lexically_normal().string();
                }
                else
                {
                    result =
                        (std::filesystem::path(getexepath()) / configured)
                        .lexically_normal()
                        .string();
                }

                return result;
            }

            std::string AvailabilitySentenceQuizWidget::resolveProjectFile(
                const std::string& projectDirectory,
                const std::string& path) const
            {
                std::string result;
                const std::filesystem::path configured(path);

                if( configured.is_absolute() )
                {
                    result = configured.lexically_normal().string();
                }
                else
                {
                    result =
                        (std::filesystem::path(projectDirectory) / configured)
                        .lexically_normal()
                        .string();
                }

                return result;
            }

            void AvailabilitySentenceQuizWidget::copyToBuffer(
                char* destination,
                size_t destinationSize,
                const std::string& source)
            {
                if( (destination != nullptr) && (destinationSize > 0U) )
                {
                    std::memset(destination, 0, destinationSize);
                    std::memcpy(
                        destination,
                        source.c_str(),
                        std::min(source.size(), destinationSize - 1U));
                }
            }
        }
    }
}
