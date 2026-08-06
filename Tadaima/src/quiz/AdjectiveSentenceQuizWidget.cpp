#include "AdjectiveSentenceQuizWidget.h"

#include "tools/pugixml.hpp"
#include "tools/SystemTools.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <format>
#include <limits>
#include <stdexcept>
#include <tuple>
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
                        result = static_cast<uint32_t>(attribute.as_uint(defaultValue));
                    }

                    return result;
                }
            }

            AdjectiveSentenceQuizWidget::AdjectiveSentenceQuizWidget(tools::Logger& logger)
                : m_logger(logger)
                , m_rng(static_cast<uint32_t>(
                    std::chrono::steady_clock::now().time_since_epoch().count()))
            {
            }

            void AdjectiveSentenceQuizWidget::initialize(const tools::DataPackage& r_package)
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
                            package->get<std::string>(SettingsPackageKey::TriesForQuiz);

                        copyToBuffer(m_projectPath, sizeof(m_projectPath), projectPath);
                        m_numberOfTries = std::stoi(numberOfTries);

                        m_logger.log(
                            "AdjectiveSentenceQuizWidget: Initialized with quiz project path.",
                            tools::LogLevel::INFO);
                    }
                }
                catch( const std::exception& exception )
                {
                    m_logger.log(
                        "AdjectiveSentenceQuizWidget::initialize exception: " +
                        std::string(exception.what()),
                        tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log(
                        "AdjectiveSentenceQuizWidget::initialize unknown exception.",
                        tools::LogLevel::PROBLEM);
                }
            }

            void AdjectiveSentenceQuizWidget::draw(bool* p_open)
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
                        "AdjectiveSentenceQuizWidget::draw exception: " +
                        std::string(exception.what()),
                        tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log(
                        "AdjectiveSentenceQuizWidget::draw unknown exception.",
                        tools::LogLevel::PROBLEM);
                }
            }

            void AdjectiveSentenceQuizWidget::drawSettingsWindow(bool* p_open)
            {
                ImGui::SetNextWindowSize(ImVec2(700, 570), ImGuiCond_FirstUseEver);
                ImGui::PushStyleColor(
                    ImGuiCol_WindowBg,
                    ImVec4(0.98f, 0.92f, 0.84f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));

                bool windowOpen = true;

                if( ImGui::Begin(
                    "Adjective Sentence Quiz Settings",
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
                        "Adjective Sentence Quiz");
                    ImGui::TextWrapped(
                        "This quiz loads quiz_project.xml and builds adjective sentences "
                        "from the adjective dictionary, reusable subjects, and valid "
                        "adjective sentence patterns.");
                    ImGui::Separator();

                    ImGui::SeparatorText("Quiz mode");

                    const char* quizModes[] =
                    {
                        "Write Japanese sentence from source sentence",
                        "Transform Japanese sentence"
                    };

                    if( ImGui::Combo(
                        "Mode",
                        &m_quizModeIndex,
                        quizModes,
                        IM_ARRAYSIZE(quizModes)) )
                    {
                        m_quizMode = static_cast<QuizMode>(m_quizModeIndex);
                        m_showStatistics = false;
                    }

                    ImGui::Separator();
                    ImGui::InputText(
                        "Word filter",
                        m_wordFilter,
                        IM_ARRAYSIZE(m_wordFilter));
                    ImGui::TextDisabled(
                        "Examples: type=i_adjective | type=na_adjective | "
                        "tag=color | id=atarashii");

                    if( m_quizMode == QuizMode::Transformation )
                    {
                        ImGui::Spacing();
                        ImGui::SeparatorText("Transformation direction");
                        ImGui::Checkbox(
                            "Polite to plain",
                            &m_enablePoliteToPlain);
                        ImGui::SameLine();
                        ImGui::Checkbox(
                            "Plain to polite",
                            &m_enablePlainToPolite);
                    }
                    else
                    {
                        ImGui::Spacing();
                        ImGui::SeparatorText("Japanese answer style");
                        ImGui::Checkbox("Polite", &m_enablePoliteOutput);
                        ImGui::SameLine();
                        ImGui::Checkbox(
                            "Plain / informal",
                            &m_enablePlainOutput);
                    }

                    ImGui::Spacing();
                    ImGui::SeparatorText("Sentence shape");
                    ImGui::Checkbox(
                        "After topic noun",
                        &m_enablePredicateSentences);
                    ImGui::SameLine();
                    ImGui::TextDisabled("このテストはむずかしいです");

                    ImGui::Checkbox(
                        "Simple これは... sentence",
                        &m_enableSimpleSentences);
                    ImGui::SameLine();
                    ImGui::TextDisabled("これはむずかしいです");

                    ImGui::Checkbox(
                        "Adjectives before noun",
                        &m_enableAttributiveSentences);
                    ImGui::SameLine();
                    ImGui::TextDisabled("これはむずかしいテストです");

                    ImGui::Spacing();
                    ImGui::SeparatorText("Adjective types");
                    ImGui::Checkbox("i-adjectives", &m_enableIAdjectives);
                    ImGui::SameLine();
                    ImGui::Checkbox("na-adjectives", &m_enableNaAdjectives);

                    ImGui::Spacing();
                    ImGui::SeparatorText("Adjective combination");
                    ImGui::SliderInt(
                        "Adjectives per sentence",
                        &m_numberOfAdjectives,
                        1,
                        3);
                    ImGui::TextDisabled(
                        "Earlier adjectives use their XML connective form; "
                        "the final adjective receives the selected form.");

                    ImGui::Spacing();
                    ImGui::SeparatorText("Forms");

                    if( ImGui::BeginTable(
                        "AdjectiveQuizFormsTable",
                        4,
                        ImGuiTableFlags_SizingStretchSame) )
                    {
                        ImGui::TableNextColumn();
                        ImGui::Checkbox(
                            "Non-past affirmative",
                            &m_enableNonPast);
                        ImGui::TableNextColumn();
                        ImGui::Checkbox(
                            "Non-past negative",
                            &m_enableNegative);
                        ImGui::TableNextColumn();
                        ImGui::Checkbox(
                            "Past affirmative",
                            &m_enablePast);
                        ImGui::TableNextColumn();
                        ImGui::Checkbox(
                            "Past negative",
                            &m_enablePastNegative);
                        ImGui::EndTable();
                    }

                    ImGui::TextDisabled(
                        "Predicate patterns use a complete adjective form. "
                        "Before a noun, the XML copula follows the noun.");

                    ImGui::Spacing();
                    ImGui::SeparatorText("Quiz size");
                    ImGui::SliderInt(
                        "Number of questions",
                        &m_numberOfQuestions,
                        1,
                        200);

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
                        ImGui::Text(
                            "Adjectives: %u",
                            m_statistics.adjectives);
                        ImGui::Text(
                            "Valid adjective patterns: %u",
                            m_statistics.sentencePatterns);
                        ImGui::Text(
                            "Reusable subjects: %u",
                            m_statistics.entities);
                        ImGui::Text(
                            "Tag profiles: %u",
                            m_statistics.tagProfiles);
                        ImGui::Text(
                            "Adjectives per sentence: %d",
                            m_numberOfAdjectives);
                        ImGui::Text(
                            "Eligible adjective words: %llu",
                            static_cast<unsigned long long>(
                                m_statistics.eligibleFinalChoices));
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

            void AdjectiveSentenceQuizWidget::drawQuizWindow(bool* p_open)
            {
                ImGui::SetNextWindowSize(ImVec2(720, 540), ImGuiCond_FirstUseEver);
                ImGui::PushStyleColor(
                    ImGuiCol_WindowBg,
                    ImVec4(0.98f, 0.92f, 0.84f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));

                bool windowOpen = true;

                if( ImGui::Begin(
                    "Adjective Sentence Quiz",
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
                        ImGui::Text(
                            "Correct answers: %u",
                            m_correctAnswers);
                        ImGui::Text(
                            "Wrong answers: %u",
                            m_wrongAnswers);
                        ImGui::Text(
                            "Accepted manually: %u",
                            m_acceptedAnswers);
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

                        if( m_quizMode == QuizMode::Transformation )
                        {
                            ImGui::Text("Change the sentence to:");
                        }
                        else
                        {
                            ImGui::Text("Write the Japanese sentence:");
                        }

                        ImGui::SameLine();
                        ImGui::TextColored(
                            ImVec4(0.8f, 0.2f, 0.2f, 1.0f),
                            "%s",
                            question.targetLabel.c_str());
                        ImGui::Spacing();
                        ImGui::Text("Source sentence:");
                        ImGui::PushTextWrapPos(0.0f);
                        ImGui::TextColored(
                            ImVec4(0.15f, 0.15f, 0.15f, 1.0f),
                            "%s",
                            question.sourceSentence.c_str());
                        ImGui::PopTextWrapPos();

                        if( !question.questionText.empty() &&
                            (m_quizMode == QuizMode::Transformation) )
                        {
                            ImGui::Spacing();
                            ImGui::TextDisabled(
                                "Context: %s",
                                question.questionText.c_str());
                        }

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

                        if( ImGui::Button(
                            "Accept / Skip",
                            ImVec2(130, 0)) )
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
                            ImGui::TextWrapped(
                                "%s",
                                m_feedbackMessage.c_str());

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
                                "Adjective meaning: %s",
                                question.wordMeaning.c_str());
                            ImGui::TextDisabled(
                                "Adjective kana: %s",
                                question.wordKana.c_str());
                            ImGui::TextDisabled(
                                "Adjective kanji: %s",
                                question.wordKanji.c_str());
                            ImGui::TextDisabled(
                                "Adjective count: %u",
                                question.adjectiveCount);
                            ImGui::TextDisabled(
                                "Pattern: %s",
                                question.patternId.c_str());
                            ImGui::TextDisabled(
                                "Source form: %s",
                                question.sourceConjugation.c_str());
                            ImGui::TextDisabled(
                                "Target form: %s",
                                question.targetConjugation.c_str());
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

            bool AdjectiveSentenceQuizWidget::loadFiles()
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

                    const auto adjectivesIterator =
                        m_dictionaries.find("adjectives");
                    result = result &&
                        (adjectivesIterator != m_dictionaries.end()) &&
                        !adjectivesIterator->second.empty() &&
                        (countValidPatterns() > 0U);

                    if( result )
                    {
                        m_filesLoaded = true;
                        m_statusMessage = std::format(
                            "Quiz project loaded: {} adjectives, {} valid patterns, "
                            "{} reusable subjects.",
                            adjectivesIterator->second.size(),
                            countValidPatterns(),
                            m_entities.size());
                    }
                    else if( m_statusMessage.empty() )
                    {
                        m_statusMessage =
                            "Could not load an adjective dictionary and valid "
                            "adjective sentence patterns from the quiz project.";
                    }
                }
                catch( const std::exception& exception )
                {
                    m_logger.log(
                        "AdjectiveSentenceQuizWidget::loadFiles exception: " +
                        std::string(exception.what()),
                        tools::LogLevel::PROBLEM);
                    m_statusMessage = exception.what();
                }
                catch( ... )
                {
                    m_logger.log(
                        "AdjectiveSentenceQuizWidget::loadFiles unknown exception.",
                        tools::LogLevel::PROBLEM);
                    m_statusMessage =
                        "Unknown error while loading the quiz project.";
                }

                return result;
            }

            bool AdjectiveSentenceQuizWidget::loadProject(const std::string& path)
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
                        "Could not load adjective quiz project XML from path: " +
                        m_projectFullPath +
                        ". Parser status: " +
                        parseResult.description(),
                        tools::LogLevel::PROBLEM);
                }

                return result;
            }

            bool AdjectiveSentenceQuizWidget::loadDictionary(
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
                        word.translation =
                            GetAttribute(wordNode, "translation");
                        word.kana = GetAttribute(wordNode, "kana");
                        word.kanji = GetAttribute(wordNode, "kanji");
                        word.type = GetAttribute(wordNode, "type");
                        word.ending = GetAttribute(wordNode, "ending");
                        word.attributes["dictionary"] = dictionaryId;

                        const pugi::xml_node tagsNode =
                            wordNode.child("tags");

                        for( pugi::xml_node tagNode =
                            tagsNode.child("tag");
                            tagNode;
                            tagNode = tagNode.next_sibling("tag") )
                        {
                            const std::string tag =
                                trimText(GetText(tagNode));

                            if( !tag.empty() )
                            {
                                appendUnique(word.tags, tag);
                            }
                        }

                        const pugi::xml_node formsNode =
                            wordNode.child("forms");

                        for( pugi::xml_node formNode =
                            formsNode.child("form");
                            formNode;
                            formNode = formNode.next_sibling("form") )
                        {
                            FormData form;
                            form.name = GetAttribute(formNode, "name");
                            form.style = GetAttribute(formNode, "style");
                            form.context = GetAttribute(formNode, "context");
                            form.prompt = GetAttribute(formNode, "prompt");
                            form.promptMasculine =
                                GetAttribute(formNode, "prompt_m");
                            form.promptFeminine =
                                GetAttribute(formNode, "prompt_f");
                            form.promptNeuter =
                                GetAttribute(formNode, "prompt_n");
                            form.promptPlural =
                                GetAttribute(formNode, "prompt_p");
                            form.value = trimText(GetText(formNode));

                            if( form.context.empty() )
                            {
                                form.context = "none";
                            }

                            if( !form.name.empty() && !form.value.empty() )
                            {
                                word.forms[form.name] = form;
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
                        "Could not load dictionary '" +
                        dictionaryId +
                        "' from path: " +
                        fullPath +
                        ". Parser status: " +
                        parseResult.description(),
                        tools::LogLevel::PROBLEM);
                }

                return result;
            }

            bool AdjectiveSentenceQuizWidget::loadVocabulary(
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
                            includedNode =
                            includedNode.next_sibling("profile") )
                        {
                            const std::string includedId =
                                GetAttribute(includedNode, "ref");

                            if( !includedId.empty() )
                            {
                                appendUnique(profile.includes, includedId);
                            }
                        }

                        const pugi::xml_node tagsNode =
                            profileNode.child("tags");

                        for( pugi::xml_node tagNode =
                            tagsNode.child("tag");
                            tagNode;
                            tagNode = tagNode.next_sibling("tag") )
                        {
                            const std::string tag =
                                trimText(GetText(tagNode));

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

                    const pugi::xml_node entitiesNode =
                        rootNode.child("entities");

                    for( pugi::xml_node entityNode =
                        entitiesNode.child("entity");
                        entityNode;
                        entityNode = entityNode.next_sibling("entity") )
                    {
                        EntityData entity;
                        entity.id = GetAttribute(entityNode, "id");
                        entity.kana = GetAttribute(entityNode, "kana");
                        entity.kanji = GetAttribute(entityNode, "kanji");
                        const pugi::xml_node polishNode =
                            entityNode.child("polish");
                        entity.gender =
                            GetAttribute(polishNode, "gender");

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

                        const pugi::xml_node entityProfilesNode =
                            entityNode.child("profiles");

                        for( pugi::xml_node profileNode =
                            entityProfilesNode.child("profile");
                            profileNode;
                            profileNode =
                            profileNode.next_sibling("profile") )
                        {
                            const std::string profileId =
                                GetAttribute(profileNode, "ref");

                            if( !profileId.empty() )
                            {
                                appendUnique(entity.profiles, profileId);
                            }
                        }

                        const pugi::xml_node tagsNode =
                            entityNode.child("tags");

                        for( pugi::xml_node tagNode =
                            tagsNode.child("tag");
                            tagNode;
                            tagNode = tagNode.next_sibling("tag") )
                        {
                            const std::string tag =
                                trimText(GetText(tagNode));

                            if( !tag.empty() )
                            {
                                appendUnique(entity.tags, tag);
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

            bool AdjectiveSentenceQuizWidget::loadContexts(
                const std::string& fullPath)
            {
                bool result = false;
                pugi::xml_document document;
                const pugi::xml_parse_result parseResult =
                    document.load_file(fullPath.c_str());

                if( parseResult )
                {
                    const pugi::xml_node rootNode =
                        document.child("contexts");

                    for( pugi::xml_node contextNode =
                        rootNode.child("context");
                        contextNode;
                        contextNode = contextNode.next_sibling("context") )
                    {
                        ContextPoolData pool;
                        pool.id = GetAttribute(contextNode, "id");
                        pool.label = GetAttribute(contextNode, "label");

                        for( pugi::xml_node optionNode =
                            contextNode.child("option");
                            optionNode;
                            optionNode =
                            optionNode.next_sibling("option") )
                        {
                            ContextOptionData option;
                            option.id = GetAttribute(optionNode, "id");
                            option.polishPrefix =
                                GetAttribute(optionNode, "pl_prefix");
                            option.japanesePrefix =
                                GetAttribute(optionNode, "jp_prefix");
                            option.weight = std::max(
                                1U,
                                GetUnsignedAttribute(
                                    optionNode,
                                    "weight",
                                    1U));

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

            bool AdjectiveSentenceQuizWidget::loadSentencePatterns(
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

            bool AdjectiveSentenceQuizWidget::buildEffectiveEntityTags()
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
                                "AdjectiveSentenceQuizWidget profile error: " +
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

            bool AdjectiveSentenceQuizWidget::appendProfileTags(
                const std::string& profileId,
                std::vector<std::string>& recursionPath,
                std::vector<std::string>& tags,
                std::string& error) const
            {
                bool result = false;
                const auto profileIterator =
                    m_tagProfiles.find(profileId);

                if( profileIterator == m_tagProfiles.end() )
                {
                    error = "Unknown tag profile: " + profileId;
                }
                else if( containsValue(recursionPath, profileId) )
                {
                    recursionPath.push_back(profileId);
                    error =
                        "Tag profile inheritance cycle: " +
                        joinText(recursionPath, " -> ");
                    recursionPath.pop_back();
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

            AdjectiveSentenceQuizWidget::PatternValidation
                AdjectiveSentenceQuizWidget::validatePattern(
                    const SentencePatternData& pattern) const
            {
                PatternValidation validation;

                if( pattern.id == "adjective_predicate" )
                {
                    validation.kind = PatternKind::Predicate;
                }
                else if( pattern.id == "adjective_simple" )
                {
                    validation.kind = PatternKind::Simple;
                }
                else if( pattern.id == "adjective_attributive" )
                {
                    validation.kind = PatternKind::Attributive;
                }
                else
                {
                    return validation;
                }

                const std::vector<std::string> tokens =
                    extractTokens(
                        pattern.questionTemplate +
                        "\n" +
                        pattern.answerTemplate);
                bool hasSubject = false;

                for( const std::string& token : tokens )
                {
                    std::string root;
                    std::string field;
                    splitToken(token, root, field);
                    hasSubject = hasSubject || (root == "subject");

                    if( (root != "adjective") &&
                        (root != "subject") &&
                        (root != "context") &&
                        (root != "copula") )
                    {
                        validation.error =
                            "Pattern '" +
                            pattern.id +
                            "' contains unsupported slot '" +
                            root +
                            "'.";
                    }
                }

                const bool hasPrompt =
                    containsValue(tokens, "adjective.prompt");
                const bool hasAdjectiveForm =
                    containsValue(tokens, "adjective.form");
                const bool hasAttributive =
                    containsValue(tokens, "adjective.attributive");
                const bool hasCopulaForm =
                    containsValue(tokens, "copula.form");

                if( validation.error.empty() && !hasPrompt )
                {
                    validation.error =
                        "Pattern '" +
                        pattern.id +
                        "' has no {adjective.prompt} token.";
                }
                else if( validation.error.empty() &&
                    (validation.kind == PatternKind::Predicate) &&
                    !hasSubject )
                {
                    validation.error = "Predicate pattern has no subject token.";
                }
                else if( validation.error.empty() &&
                    (validation.kind == PatternKind::Predicate) &&
                    !hasAdjectiveForm )
                {
                    validation.error =
                        "Predicate pattern has no {adjective.form} token.";
                }
                else if( validation.error.empty() &&
                    (validation.kind == PatternKind::Simple) &&
                    hasSubject )
                {
                    validation.error =
                        "Simple pattern must not contain a subject token.";
                }
                else if( validation.error.empty() &&
                    (validation.kind == PatternKind::Simple) &&
                    !hasAdjectiveForm )
                {
                    validation.error =
                        "Simple pattern has no {adjective.form} token.";
                }
                else if( validation.error.empty() &&
                    (validation.kind == PatternKind::Attributive) &&
                    !hasSubject )
                {
                    validation.error =
                        "Attributive pattern has no subject token.";
                }
                else if( validation.error.empty() &&
                    (validation.kind == PatternKind::Attributive) &&
                    !hasAttributive )
                {
                    validation.error =
                        "Attributive pattern has no "
                        "{adjective.attributive} token.";
                }
                else if( validation.error.empty() &&
                    (validation.kind == PatternKind::Attributive) &&
                    !hasCopulaForm )
                {
                    validation.error =
                        "Attributive pattern has no {copula.form} token.";
                }

                validation.valid = validation.error.empty();
                return validation;
            }

            const AdjectiveSentenceQuizWidget::SentencePatternData*
                AdjectiveSentenceQuizWidget::findValidPattern(
                    const std::string& id) const
            {
                const SentencePatternData* result = nullptr;

                for( const SentencePatternData& pattern : m_sentencePatterns )
                {
                    if( (pattern.id == id) &&
                        validatePattern(pattern).valid )
                    {
                        result = &pattern;
                        break;
                    }
                }

                return result;
            }

            uint32_t AdjectiveSentenceQuizWidget::countValidPatterns() const
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

            std::vector<AdjectiveSentenceQuizWidget::GenerationRule>
                AdjectiveSentenceQuizWidget::buildEnabledTransformationRules() const
            {
                std::vector<GenerationRule> rules;

                const auto appendDirection =
                    [this, &rules](
                        bool enabled,
                        bool sourcePolite,
                        bool targetPolite)
                    {
                        if( enabled )
                        {
                            if( m_enableNonPast )
                            {
                                rules.push_back(
                                    {
                                        FormCategory::NonPast,
                                        true,
                                        sourcePolite,
                                        targetPolite,
                                        targetPolite ?
                                            "polite non-past adjective predicate" :
                                            "plain non-past adjective predicate"
                                    });
                            }

                            if( m_enableNegative )
                            {
                                rules.push_back(
                                    {
                                        FormCategory::Negative,
                                        true,
                                        sourcePolite,
                                        targetPolite,
                                        targetPolite ?
                                            "polite non-past negative adjective predicate" :
                                            "plain non-past negative adjective predicate"
                                    });
                            }

                            if( m_enablePast )
                            {
                                rules.push_back(
                                    {
                                        FormCategory::Past,
                                        true,
                                        sourcePolite,
                                        targetPolite,
                                        targetPolite ?
                                            "polite past adjective predicate" :
                                            "plain past adjective predicate"
                                    });
                            }

                            if( m_enablePastNegative )
                            {
                                rules.push_back(
                                    {
                                        FormCategory::PastNegative,
                                        true,
                                        sourcePolite,
                                        targetPolite,
                                        targetPolite ?
                                            "polite past negative adjective predicate" :
                                            "plain past negative adjective predicate"
                                    });
                            }
                        }
                    };

                appendDirection(
                    m_enablePoliteToPlain,
                    true,
                    false);
                appendDirection(
                    m_enablePlainToPolite,
                    false,
                    true);

                return rules;
            }

            std::vector<AdjectiveSentenceQuizWidget::GenerationRule>
                AdjectiveSentenceQuizWidget::buildEnabledMeaningToJapaneseRules() const
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
                                        FormCategory::NonPast,
                                        false,
                                        false,
                                        polite,
                                        polite ?
                                            "polite non-past adjective predicate" :
                                            "plain non-past adjective predicate"
                                    });
                            }

                            if( m_enableNegative )
                            {
                                rules.push_back(
                                    {
                                        FormCategory::Negative,
                                        false,
                                        false,
                                        polite,
                                        polite ?
                                            "polite non-past negative adjective predicate" :
                                            "plain non-past negative adjective predicate"
                                    });
                            }

                            if( m_enablePast )
                            {
                                rules.push_back(
                                    {
                                        FormCategory::Past,
                                        false,
                                        false,
                                        polite,
                                        polite ?
                                            "polite past adjective predicate" :
                                            "plain past adjective predicate"
                                    });
                            }

                            if( m_enablePastNegative )
                            {
                                rules.push_back(
                                    {
                                        FormCategory::PastNegative,
                                        false,
                                        false,
                                        polite,
                                        polite ?
                                            "polite past negative adjective predicate" :
                                            "plain past negative adjective predicate"
                                    });
                            }
                        }
                    };

                appendStyle(m_enablePlainOutput, false);
                appendStyle(m_enablePoliteOutput, true);
                return rules;
            }

            std::vector<AdjectiveSentenceQuizWidget::QuestionSpace>
                AdjectiveSentenceQuizWidget::buildQuestionSpaces(
                    bool transformationMode) const
            {
                std::vector<QuestionSpace> spaces;

                if( m_enablePredicateSentences )
                {
                    const std::vector<GenerationRule> rules =
                        transformationMode ?
                        buildEnabledTransformationRules() :
                        buildEnabledMeaningToJapaneseRules();
                    appendPredicateSpaces(spaces, rules);
                }

                if( m_enableSimpleSentences )
                {
                    appendSimpleSpaces(spaces, transformationMode);
                }

                if( m_enableAttributiveSentences )
                {
                    appendAttributiveSpaces(spaces, transformationMode);
                }

                return spaces;
            }

            void AdjectiveSentenceQuizWidget::appendPredicateSpaces(
                std::vector<QuestionSpace>& spaces,
                const std::vector<GenerationRule>& rules) const
            {
                const SentencePatternData* pattern =
                    findValidPattern("adjective_predicate");

                if( pattern != nullptr )
                {
                    for( const GenerationRule& rule : rules )
                    {
                        const std::string targetForm =
                            getFiniteFormName(
                                rule.category,
                                rule.targetPolite);
                        const std::string sourceForm =
                            rule.hasSourceForm ?
                            getFiniteFormName(
                                rule.category,
                                rule.sourcePolite) :
                            targetForm;

                        appendSpacesForPatternPair(
                            spaces,
                            *pattern,
                            *pattern,
                            rule,
                            sourceForm,
                            targetForm,
                            nullptr,
                            std::string(),
                            std::string());
                    }
                }
            }

            void AdjectiveSentenceQuizWidget::appendSimpleSpaces(
                std::vector<QuestionSpace>& spaces,
                bool transformationMode) const
            {
                const SentencePatternData* pattern =
                    findValidPattern("adjective_simple");
                const auto adjectivesIterator =
                    m_dictionaries.find("adjectives");

                if( (pattern == nullptr) ||
                    (adjectivesIterator == m_dictionaries.end()) )
                {
                    return;
                }

                const std::vector<GenerationRule> rules =
                    transformationMode ?
                    buildEnabledTransformationRules() :
                    buildEnabledMeaningToJapaneseRules();
                const std::vector<WordFilterRule> filters =
                    parseWordFilter(std::string(m_wordFilter));
                const size_t prefixCount =
                    static_cast<size_t>(
                        std::clamp(m_numberOfAdjectives, 1, 3) - 1);
                std::vector<const DictionaryWord*> prefixBase;

                for( const DictionaryWord& adjective :
                    adjectivesIterator->second )
                {
                    if( isAdjectiveTypeEnabled(adjective) &&
                        wordMatchesFilter(adjective, filters) &&
                        (getForm(adjective, "connective") != nullptr) )
                    {
                        prefixBase.push_back(&adjective);
                    }
                }

                for( const GenerationRule& rule : rules )
                {
                    const std::string targetForm =
                        getFiniteFormName(
                            rule.category,
                            rule.targetPolite);
                    const std::string sourceForm =
                        rule.hasSourceForm ?
                        getFiniteFormName(
                            rule.category,
                            rule.sourcePolite) :
                        targetForm;

                    for( const DictionaryWord& finalAdjective :
                        adjectivesIterator->second )
                    {
                        if( !isAdjectiveTypeEnabled(finalAdjective) ||
                            !wordMatchesFilter(finalAdjective, filters) )
                        {
                            continue;
                        }

                        const FormData* source =
                            getForm(finalAdjective, sourceForm);
                        const FormData* target =
                            getForm(finalAdjective, targetForm);

                        if( (source == nullptr) || (target == nullptr) )
                        {
                            continue;
                        }

                        const std::string sourceContext =
                            source->context.empty() ?
                            "none" :
                            source->context;
                        const std::string targetContext =
                            target->context.empty() ?
                            "none" :
                            target->context;
                        const ContextPoolData* contextPool =
                            getContextPool(targetContext);

                        if( (sourceContext != targetContext) ||
                            (contextPool == nullptr) )
                        {
                            continue;
                        }

                        QuestionSpace space;
                        space.sourcePattern = pattern;
                        space.targetPattern = pattern;
                        space.finalAdjective = &finalAdjective;
                        space.contextPool = contextPool;
                        space.rule = rule;
                        space.sourceForm = sourceForm;
                        space.targetForm = targetForm;

                        for( const DictionaryWord* candidate : prefixBase )
                        {
                            if( candidate != &finalAdjective )
                            {
                                space.prefixCandidates.push_back(candidate);
                            }
                        }

                        space.permutationCount = permutationCount(
                            space.prefixCandidates.size(),
                            prefixCount);

                        if( space.permutationCount > 0U )
                        {
                            const uint64_t contextCount =
                                static_cast<uint64_t>(
                                    contextPool->options.size());

                            if( space.permutationCount <=
                                (std::numeric_limits<uint64_t>::max() /
                                    contextCount) )
                            {
                                space.questionCount =
                                    space.permutationCount *
                                    contextCount;
                                spaces.push_back(std::move(space));
                            }
                        }
                    }
                }
            }

            void AdjectiveSentenceQuizWidget::appendAttributiveSpaces(
                std::vector<QuestionSpace>& spaces,
                bool transformationMode) const
            {
                const SentencePatternData* pattern =
                    findValidPattern("adjective_attributive");
                const auto copulasIterator =
                    m_dictionaries.find("copulas");

                if( (pattern == nullptr) ||
                    (copulasIterator == m_dictionaries.end()) )
                {
                    return;
                }

                std::vector<GenerationRule> rules =
                    transformationMode ?
                    buildEnabledTransformationRules() :
                    buildEnabledMeaningToJapaneseRules();

                for( GenerationRule& rule : rules )
                {
                    rule.label = replaceAll(
                        rule.label,
                        "adjective predicate",
                        "adjective before noun");
                    const std::string targetForm =
                        "attributive_nonpast";
                    const std::string sourceForm =
                        "attributive_nonpast";
                    const std::string targetCopulaForm =
                        getCopulaFormName(
                            rule.category,
                            rule.targetPolite);
                    const std::string sourceCopulaForm =
                        rule.hasSourceForm ?
                        getCopulaFormName(
                            rule.category,
                            rule.sourcePolite) :
                        targetCopulaForm;

                    for( const DictionaryWord& copula :
                        copulasIterator->second )
                    {
                        if( (getForm(copula, sourceCopulaForm) != nullptr) &&
                            (getForm(copula, targetCopulaForm) != nullptr) )
                        {
                            appendSpacesForPatternPair(
                                spaces,
                                *pattern,
                                *pattern,
                                rule,
                                sourceForm,
                                targetForm,
                                &copula,
                                sourceCopulaForm,
                                targetCopulaForm);
                        }
                    }
                }
            }

            void AdjectiveSentenceQuizWidget::appendSpacesForPatternPair(
                std::vector<QuestionSpace>& spaces,
                const SentencePatternData& sourcePattern,
                const SentencePatternData& targetPattern,
                const GenerationRule& rule,
                const std::string& sourceForm,
                const std::string& targetForm,
                const DictionaryWord* copula,
                const std::string& sourceCopulaForm,
                const std::string& targetCopulaForm) const
            {
                const auto adjectivesIterator =
                    m_dictionaries.find("adjectives");

                if( adjectivesIterator == m_dictionaries.end() )
                {
                    return;
                }

                const std::vector<WordFilterRule> filters =
                    parseWordFilter(std::string(m_wordFilter));
                const size_t prefixCount =
                    static_cast<size_t>(
                        std::clamp(m_numberOfAdjectives, 1, 3) - 1);

                for( const auto& entityEntry : m_entities )
                {
                    const EntityData& entity = entityEntry.second;
                    const std::vector<const DictionaryWord*> prefixBase =
                        getCompatibleAdjectives(
                            entity,
                            true,
                            filters);

                    for( const DictionaryWord& finalAdjective :
                        adjectivesIterator->second )
                    {
                        if( isAdjectiveTypeEnabled(finalAdjective) &&
                            wordMatchesFilter(finalAdjective, filters) &&
                            entityMatchesAdjective(
                                entity,
                                finalAdjective) )
                        {
                            const FormData* source =
                                getForm(finalAdjective, sourceForm);
                            const FormData* target =
                                getForm(finalAdjective, targetForm);
                            const bool requiresAttributive =
                                (copula != nullptr);

                            if( (source != nullptr) &&
                                (target != nullptr) &&
                                (!requiresAttributive ||
                                    (getForm(
                                        finalAdjective,
                                        "attributive_nonpast") != nullptr)) )
                            {
                                std::string sourceContext =
                                    source->context.empty() ?
                                    "none" :
                                    source->context;
                                std::string targetContext =
                                    target->context.empty() ?
                                    "none" :
                                    target->context;
                                bool contextsCompatible = true;
                                const FormData* sourceCopula =
                                    (copula != nullptr) ?
                                    getForm(*copula, sourceCopulaForm) :
                                    nullptr;
                                const FormData* targetCopula =
                                    (copula != nullptr) ?
                                    getForm(*copula, targetCopulaForm) :
                                    nullptr;

                                const auto mergeCopulaContext =
                                    [&contextsCompatible](
                                        std::string& context,
                                        const FormData* copulaForm)
                                    {
                                        if( copulaForm != nullptr )
                                        {
                                            const std::string copulaContext =
                                                copulaForm->context.empty() ?
                                                "none" :
                                                copulaForm->context;

                                            if( context == "none" )
                                            {
                                                context = copulaContext;
                                            }
                                            else if( (copulaContext != "none") &&
                                                (copulaContext != context) )
                                            {
                                                contextsCompatible = false;
                                            }
                                        }
                                    };

                                mergeCopulaContext(
                                    sourceContext,
                                    sourceCopula);
                                mergeCopulaContext(
                                    targetContext,
                                    targetCopula);

                                const ContextPoolData* contextPool =
                                    getContextPool(targetContext);

                                if( contextsCompatible &&
                                    (sourceContext == targetContext) &&
                                    (contextPool != nullptr) )
                                {
                                    QuestionSpace space;
                                    space.sourcePattern = &sourcePattern;
                                    space.targetPattern = &targetPattern;
                                    space.subject = &entity;
                                    space.finalAdjective = &finalAdjective;
                                    space.copula = copula;
                                    space.contextPool = contextPool;
                                    space.rule = rule;
                                    space.sourceForm = sourceForm;
                                    space.targetForm = targetForm;
                                    space.sourceCopulaForm =
                                        sourceCopulaForm;
                                    space.targetCopulaForm =
                                        targetCopulaForm;

                                    for( const DictionaryWord* candidate :
                                        prefixBase )
                                    {
                                        if( candidate != &finalAdjective )
                                        {
                                            space.prefixCandidates.push_back(
                                                candidate);
                                        }
                                    }

                                    space.permutationCount = permutationCount(
                                        space.prefixCandidates.size(),
                                        prefixCount);

                                    if( space.permutationCount > 0U )
                                    {
                                        const uint64_t contextCount =
                                            static_cast<uint64_t>(
                                                contextPool->options.size());

                                        if( space.permutationCount <=
                                            (std::numeric_limits<uint64_t>::max() /
                                                contextCount) )
                                        {
                                            space.questionCount =
                                                space.permutationCount *
                                                contextCount;
                                            spaces.push_back(std::move(space));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            std::vector<AdjectiveSentenceQuizWidget::QuizQuestion>
                AdjectiveSentenceQuizWidget::buildQuestionsForMode(
                    bool transformationMode,
                    size_t maximumQuestions) const
            {
                std::vector<QuizQuestion> questions;
                const std::vector<QuestionSpace> spaces =
                    buildQuestionSpaces(transformationMode);
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
                        renderQuestionSeed(
                            seed,
                            transformationMode,
                            question) )
                    {
                        questions.push_back(std::move(question));
                    }
                }

                return questions;
            }

            bool AdjectiveSentenceQuizWidget::seedFromGlobalIndex(
                const std::vector<QuestionSpace>& spaces,
                uint64_t globalIndex,
                QuestionSeed& seed) const
            {
                bool result = false;

                for( const QuestionSpace& space : spaces )
                {
                    if( globalIndex < space.questionCount )
                    {
                        const uint64_t contextCount =
                            static_cast<uint64_t>(
                                space.contextPool->options.size());
                        const uint64_t permutationRank =
                            globalIndex / contextCount;
                        const size_t contextIndex =
                            static_cast<size_t>(
                                globalIndex % contextCount);
                        const size_t prefixCount =
                            static_cast<size_t>(
                                std::clamp(
                                    m_numberOfAdjectives,
                                    1,
                                    3) -
                                1);

                        seed.space = &space;
                        seed.adjectiveChain =
                            unrankPrefixPermutation(
                                space.prefixCandidates,
                                prefixCount,
                                permutationRank);

                        if( seed.adjectiveChain.size() == prefixCount )
                        {
                            seed.adjectiveChain.push_back(
                                space.finalAdjective);
                            seed.contextOption =
                                &space.contextPool->options[contextIndex];
                            result = true;
                        }

                        break;
                    }

                    globalIndex -= space.questionCount;
                }

                return result;
            }

            std::vector<const AdjectiveSentenceQuizWidget::DictionaryWord*>
                AdjectiveSentenceQuizWidget::unrankPrefixPermutation(
                    const std::vector<const DictionaryWord*>& candidates,
                    size_t prefixCount,
                    uint64_t rank) const
            {
                std::vector<const DictionaryWord*> result;
                std::vector<const DictionaryWord*> remaining = candidates;

                for( size_t position = 0U;
                    position < prefixCount;
                    ++position )
                {
                    const size_t positionsAfter =
                        prefixCount - position - 1U;
                    const uint64_t blockSize =
                        permutationCount(
                            remaining.size() - 1U,
                            positionsAfter);
                    const uint64_t selectedIndex =
                        (blockSize > 0U) ?
                        (rank / blockSize) :
                        0U;

                    if( selectedIndex >= remaining.size() )
                    {
                        result.clear();
                        break;
                    }

                    result.push_back(
                        remaining[static_cast<size_t>(selectedIndex)]);
                    remaining.erase(
                        remaining.begin() +
                        static_cast<std::ptrdiff_t>(selectedIndex));

                    if( blockSize > 0U )
                    {
                        rank %= blockSize;
                    }
                }

                return result;
            }

            bool AdjectiveSentenceQuizWidget::renderQuestionSeed(
                const QuestionSeed& seed,
                bool transformationMode,
                QuizQuestion& question) const
            {
                bool result = false;

                if( (seed.space != nullptr) &&
                    (seed.space->sourcePattern != nullptr) &&
                    (seed.space->targetPattern != nullptr) &&
                    (seed.space->contextPool != nullptr) &&
                    (seed.contextOption != nullptr) &&
                    !seed.adjectiveChain.empty() )
                {
                    std::string targetQuestion;
                    std::string targetAnswer;
                    std::string sourceQuestion;
                    std::string sourceAnswer;
                    std::string error;

                    const bool targetRendered = renderPattern(
                        *seed.space->targetPattern,
                        seed.space->subject,
                        seed.adjectiveChain,
                        seed.space->targetForm,
                        seed.space->copula,
                        seed.space->targetCopulaForm,
                        *seed.space->contextPool,
                        *seed.contextOption,
                        targetQuestion,
                        targetAnswer,
                        error);
                    bool sourceRendered = targetRendered;

                    if( transformationMode && sourceRendered )
                    {
                        sourceRendered = renderPattern(
                            *seed.space->sourcePattern,
                            seed.space->subject,
                            seed.adjectiveChain,
                            seed.space->sourceForm,
                            seed.space->copula,
                            seed.space->sourceCopulaForm,
                            *seed.space->contextPool,
                            *seed.contextOption,
                            sourceQuestion,
                            sourceAnswer,
                            error);
                    }

                    const bool distinct =
                        !transformationMode ||
                        (normalizeAnswer(sourceAnswer) !=
                            normalizeAnswer(targetAnswer));

                    if( targetRendered &&
                        sourceRendered &&
                        distinct &&
                        !targetAnswer.empty() &&
                        !(transformationMode ?
                            sourceAnswer :
                            targetQuestion).empty() )
                    {
                        question.key = buildQuestionKey(
                            seed,
                            seed.space->sourceForm,
                            seed.space->targetForm);
                        question.sourceSentence =
                            transformationMode ?
                            sourceAnswer :
                            targetQuestion;
                        question.targetSentence = targetAnswer;
                        question.questionText =
                            transformationMode ?
                            targetQuestion :
                            std::string();
                        question.targetLabel =
                            seed.space->rule.label;
                        question.sourceConjugation =
                            transformationMode ?
                            (seed.space->sourceCopulaForm.empty() ?
                                seed.space->sourceForm :
                                "attributive_nonpast + " +
                                seed.space->sourceCopulaForm) :
                            "source_language";
                        question.targetConjugation =
                            seed.space->targetCopulaForm.empty() ?
                            seed.space->targetForm :
                            "attributive_nonpast + " +
                            seed.space->targetCopulaForm;
                        question.wordMeaning =
                            buildChainInfo(
                                seed.adjectiveChain,
                                "translation");
                        question.wordKana =
                            buildChainInfo(
                                seed.adjectiveChain,
                                "kana");
                        question.wordKanji =
                            buildChainInfo(
                                seed.adjectiveChain,
                                "kanji");
                        question.patternId =
                            (seed.space->sourcePattern->id ==
                                seed.space->targetPattern->id) ?
                            seed.space->targetPattern->id :
                            seed.space->sourcePattern->id +
                            " -> " +
                            seed.space->targetPattern->id;
                        question.bindings =
                            ((seed.space->subject != nullptr) ?
                                "subject=" +
                                seed.space->subject->id +
                                "; " :
                                std::string()) +
                            "adjectives=" +
                            buildChainInfo(
                                seed.adjectiveChain,
                                "id") +
                            ((seed.space->copula != nullptr) ?
                                "; copula=" +
                                seed.space->copula->id :
                                std::string());
                        question.adjectiveCount =
                            static_cast<uint32_t>(
                                seed.adjectiveChain.size());
                        result = true;
                    }
                    else if( !error.empty() )
                    {
                        m_logger.log(
                            "Adjective question could not be rendered: " +
                            error,
                            tools::LogLevel::PROBLEM);
                    }
                }

                return result;
            }

            uint64_t AdjectiveSentenceQuizWidget::countPossibleQuestions(
                bool transformationMode) const
            {
                uint64_t result = 0U;

                for( const QuestionSpace& space :
                    buildQuestionSpaces(transformationMode) )
                {
                    if( result >
                        (std::numeric_limits<uint64_t>::max() -
                            space.questionCount) )
                    {
                        result = std::numeric_limits<uint64_t>::max();
                        break;
                    }

                    result += space.questionCount;
                }

                return result;
            }

            uint64_t AdjectiveSentenceQuizWidget::permutationCount(
                size_t candidateCount,
                size_t selectedCount) const
            {
                uint64_t result = 1U;

                if( selectedCount > candidateCount )
                {
                    return 0U;
                }

                for( size_t index = 0U;
                    index < selectedCount;
                    ++index )
                {
                    const uint64_t factor =
                        static_cast<uint64_t>(candidateCount - index);

                    if( result >
                        (std::numeric_limits<uint64_t>::max() / factor) )
                    {
                        return std::numeric_limits<uint64_t>::max();
                    }

                    result *= factor;
                }

                return result;
            }

            std::vector<uint64_t>
                AdjectiveSentenceQuizWidget::chooseQuestionIndices(
                    uint64_t total,
                    size_t maximumQuestions) const
            {
                std::vector<uint64_t> result;

                if( total == 0U )
                {
                    return result;
                }

                uint64_t requested =
                    (maximumQuestions == 0U) ?
                    total :
                    std::min(
                        total,
                        static_cast<uint64_t>(maximumQuestions));

                if( requested >
                    static_cast<uint64_t>(
                        std::numeric_limits<size_t>::max()) )
                {
                    requested =
                        static_cast<uint64_t>(
                            std::numeric_limits<size_t>::max());
                }

                result.reserve(static_cast<size_t>(requested));

                if( !m_randomOrder )
                {
                    for( uint64_t index = 0U;
                        index < requested;
                        ++index )
                    {
                        result.push_back(index);
                    }
                }
                else
                {
                    std::unordered_set<uint64_t> selected;
                    selected.reserve(
                        static_cast<size_t>(requested * 2U + 1U));
                    std::uniform_int_distribution<uint64_t> distribution(
                        0U,
                        total - 1U);

                    while( selected.size() <
                        static_cast<size_t>(requested) )
                    {
                        selected.insert(distribution(m_rng));
                    }

                    result.assign(selected.begin(), selected.end());
                    std::shuffle(result.begin(), result.end(), m_rng);
                }

                return result;
            }

            bool AdjectiveSentenceQuizWidget::renderPattern(
                const SentencePatternData& pattern,
                const EntityData* subject,
                const std::vector<const DictionaryWord*>& adjectiveChain,
                const std::string& finalForm,
                const DictionaryWord* copula,
                const std::string& copulaForm,
                const ContextPoolData& contextPool,
                const ContextOptionData& contextOption,
                std::string& question,
                std::string& answer,
                std::string& error) const
            {
                const bool questionRendered = renderTemplate(
                    pattern.questionTemplate,
                    subject,
                    adjectiveChain,
                    finalForm,
                    copula,
                    copulaForm,
                    contextPool,
                    contextOption,
                    question,
                    error);
                const bool answerRendered =
                    questionRendered &&
                    renderTemplate(
                        pattern.answerTemplate,
                        subject,
                        adjectiveChain,
                        finalForm,
                        copula,
                        copulaForm,
                        contextPool,
                        contextOption,
                        answer,
                        error);

                return answerRendered;
            }

            bool AdjectiveSentenceQuizWidget::renderTemplate(
                const std::string& text,
                const EntityData* subject,
                const std::vector<const DictionaryWord*>& adjectiveChain,
                const std::string& finalForm,
                const DictionaryWord* copula,
                const std::string& copulaForm,
                const ContextPoolData& contextPool,
                const ContextOptionData& contextOption,
                std::string& rendered,
                std::string& error) const
            {
                bool result = true;
                rendered = text;

                for( const std::string& expression :
                    extractTokens(text) )
                {
                    std::string value;

                    if( resolveToken(
                        expression,
                        subject,
                        adjectiveChain,
                        finalForm,
                        copula,
                        copulaForm,
                        contextPool,
                        contextOption,
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
                        break;
                    }
                }

                if( result )
                {
                    rendered = replaceAll(rendered, "\r", "");
                    rendered = replaceAll(rendered, "\n", " ");
                    rendered = replaceAll(rendered, "\t", " ");

                    while( rendered.find("  ") != std::string::npos )
                    {
                        rendered = replaceAll(rendered, "  ", " ");
                    }

                    rendered = trimText(rendered);
                }

                return result;
            }

            bool AdjectiveSentenceQuizWidget::resolveToken(
                const std::string& expression,
                const EntityData* subject,
                const std::vector<const DictionaryWord*>& adjectiveChain,
                const std::string& finalForm,
                const DictionaryWord* copula,
                const std::string& copulaForm,
                const ContextPoolData& contextPool,
                const ContextOptionData& contextOption,
                std::string& value,
                std::string& error) const
            {
                std::string root;
                std::string field;
                splitToken(expression, root, field);

                if( root == "context" )
                {
                    if( field == "id" )
                    {
                        value = contextPool.id;
                        return true;
                    }

                    if( field == "option" )
                    {
                        value = contextOption.id;
                        return true;
                    }

                    if( field == "pl_prefix" )
                    {
                        value = contextOption.polishPrefix;
                        return true;
                    }

                    if( field == "jp_prefix" )
                    {
                        value = contextOption.japanesePrefix;
                        return true;
                    }

                    error = "Unknown context field '" + field + "'.";
                    return false;
                }

                if( root == "subject" )
                {
                    if( subject == nullptr )
                    {
                        error =
                            "Pattern requests a subject, but this sentence "
                            "shape has no subject binding.";
                        return false;
                    }

                    return resolveSubjectToken(
                        *subject,
                        field,
                        value,
                        error);
                }

                if( root == "adjective" )
                {
                    return resolveAdjectiveToken(
                        subject,
                        adjectiveChain,
                        field,
                        finalForm,
                        value,
                        error);
                }

                if( root == "copula" )
                {
                    return resolveCopulaToken(
                        copula,
                        subject,
                        field,
                        copulaForm,
                        value,
                        error);
                }

                error = "Unknown adjective quiz slot '" + root + "'.";
                return false;
            }

            bool AdjectiveSentenceQuizWidget::resolveCopulaToken(
                const DictionaryWord* copula,
                const EntityData* subject,
                const std::string& field,
                const std::string& copulaForm,
                std::string& value,
                std::string& error) const
            {
                if( copula == nullptr )
                {
                    error =
                        "Pattern requests a copula, but this sentence shape "
                        "has no copula binding.";
                    return false;
                }

                const FormData* form =
                    getForm(*copula, copulaForm);

                if( form == nullptr )
                {
                    error =
                        "Copula '" +
                        copula->id +
                        "' has no form '" +
                        copulaForm +
                        "'.";
                    return false;
                }

                if( field.empty() || (field == "form") )
                {
                    value = form->value;
                }
                else if( field == "prompt" )
                {
                    value = getFormPrompt(
                        *form,
                        (subject != nullptr) ?
                        subject->gender :
                        std::string("neuter"));
                }
                else if( field == "style" )
                {
                    value = form->style;
                }
                else if( field == "form_name" )
                {
                    value = form->name;
                }
                else if( field == "id" )
                {
                    value = copula->id;
                }
                else if( field == "kana" )
                {
                    value = copula->kana;
                }
                else if( field == "translation" )
                {
                    value = copula->translation;
                }
                else
                {
                    error =
                        "Unknown copula field '" +
                        field +
                        "'.";
                    return false;
                }

                return !value.empty();
            }

            bool AdjectiveSentenceQuizWidget::resolveSubjectToken(
                const EntityData& subject,
                const std::string& field,
                std::string& value,
                std::string& error) const
            {
                bool result = true;

                if( field.empty() || (field == "kana") )
                {
                    value = subject.kana;
                }
                else if( field == "id" )
                {
                    value = subject.id;
                }
                else if( field == "kanji" )
                {
                    value = subject.kanji;
                }
                else if( field == "gender" )
                {
                    value = subject.gender;
                }
                else if( field == "profiles" )
                {
                    value = joinText(subject.profiles, ", ");
                }
                else if( (field == "categories") || (field == "tags") )
                {
                    const auto tagsIterator =
                        m_effectiveEntityTags.find(subject.id);

                    if( tagsIterator != m_effectiveEntityTags.end() )
                    {
                        value = joinText(tagsIterator->second, ", ");
                    }
                    else
                    {
                        result = false;
                    }
                }
                else if( field.rfind("pl_", 0U) == 0U )
                {
                    const auto polishIterator =
                        subject.polish.find(field.substr(3U));

                    if( polishIterator != subject.polish.end() )
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
                        "Unknown or unavailable subject field '" +
                        field +
                        "'.";
                }

                return result;
            }

            bool AdjectiveSentenceQuizWidget::resolveAdjectiveToken(
                const EntityData* subject,
                const std::vector<const DictionaryWord*>& adjectiveChain,
                const std::string& field,
                const std::string& finalForm,
                std::string& value,
                std::string& error) const
            {
                bool result = !adjectiveChain.empty();

                if( result )
                {
                    const DictionaryWord& finalAdjective =
                        *adjectiveChain.back();

                    if( field.empty() || (field == "form") )
                    {
                        value = buildCombinedJapaneseForm(
                            adjectiveChain,
                            finalForm);
                    }
                    else if( field == "attributive" )
                    {
                        value = buildCombinedJapaneseForm(
                            adjectiveChain,
                            "attributive_nonpast");
                    }
                    else if( field == "prompt" )
                    {
                        value = buildCombinedPrompt(
                            subject,
                            adjectiveChain,
                            finalForm);
                    }
                    else if( field == "id" )
                    {
                        value = finalAdjective.id;
                    }
                    else if( field == "kana" )
                    {
                        value = finalAdjective.kana;
                    }
                    else if( field == "kanji" )
                    {
                        value = finalAdjective.kanji;
                    }
                    else if( field == "translation" )
                    {
                        value = finalAdjective.translation;
                    }
                    else if( field == "type" )
                    {
                        value = finalAdjective.type;
                    }
                    else if( (field == "categories") ||
                        (field == "tags") )
                    {
                        value = joinText(finalAdjective.tags, ", ");
                    }
                    else
                    {
                        result = false;
                    }
                }

                if( !result )
                {
                    error =
                        "Unknown or unavailable adjective field '" +
                        field +
                        "'.";
                }

                return result && !value.empty();
            }

            std::string
                AdjectiveSentenceQuizWidget::buildCombinedJapaneseForm(
                    const std::vector<const DictionaryWord*>& adjectiveChain,
                    const std::string& finalForm) const
            {
                std::string result;

                for( size_t index = 0U;
                    index < adjectiveChain.size();
                    ++index )
                {
                    const DictionaryWord* adjective =
                        adjectiveChain[index];
                    const std::string formName =
                        (index + 1U < adjectiveChain.size()) ?
                        "connective" :
                        finalForm;
                    const FormData* form =
                        (adjective != nullptr) ?
                        getForm(*adjective, formName) :
                        nullptr;

                    if( form == nullptr )
                    {
                        return std::string();
                    }

                    result += form->value;
                }

                return result;
            }

            std::string AdjectiveSentenceQuizWidget::buildCombinedPrompt(
                const EntityData* subject,
                const std::vector<const DictionaryWord*>& adjectiveChain,
                const std::string& finalForm) const
            {
                if( adjectiveChain.empty() )
                {
                    return std::string();
                }

                const DictionaryWord* finalAdjective =
                    adjectiveChain.back();
                const FormData* finalFormData =
                    (finalAdjective != nullptr) ?
                    getForm(*finalAdjective, finalForm) :
                    nullptr;

                if( finalFormData == nullptr )
                {
                    return std::string();
                }

                std::string finalPrompt =
                    getFormPrompt(
                        *finalFormData,
                        (subject != nullptr) ?
                        subject->gender :
                        std::string("neuter"));

                if( adjectiveChain.size() == 1U )
                {
                    return finalPrompt;
                }

                std::vector<std::string> parts;

                for( size_t index = 0U;
                    index + 1U < adjectiveChain.size();
                    ++index )
                {
                    const FormData* connective =
                        getForm(
                            *adjectiveChain[index],
                            "connective");

                    if( connective != nullptr )
                    {
                        parts.push_back(
                            getFormPrompt(
                                *connective,
                                (subject != nullptr) ?
                                subject->gender :
                                std::string("neuter")));
                    }
                }

                std::string promptPrefix;

                /*
                 * The dictionary prompts are Polish. Keeping the finite phrase
                 * at the front produces natural prompts such as
                 * "jest nowy i czerwony". Japanese morphology itself remains
                 * entirely data-driven by the XML forms.
                 */
                static const std::vector<std::string> finitePrefixes =
                {
                    "nie były ",
                    "nie byli ",
                    "nie była ",
                    "nie było ",
                    "nie był ",
                    "nie są ",
                    "nie jest ",
                    "były ",
                    "byli ",
                    "była ",
                    "było ",
                    "był ",
                    "są ",
                    "jest "
                };

                for( const std::string& prefix : finitePrefixes )
                {
                    if( finalPrompt.rfind(prefix, 0U) == 0U )
                    {
                        promptPrefix = prefix;
                        finalPrompt =
                            finalPrompt.substr(prefix.size());
                        break;
                    }
                }

                parts.push_back(finalPrompt);
                return promptPrefix + joinText(parts, " i ");
            }

            std::string AdjectiveSentenceQuizWidget::buildChainInfo(
                const std::vector<const DictionaryWord*>& adjectiveChain,
                const std::string& field) const
            {
                std::vector<std::string> values;

                for( const DictionaryWord* adjective : adjectiveChain )
                {
                    if( adjective != nullptr )
                    {
                        if( field == "id" )
                        {
                            values.push_back(adjective->id);
                        }
                        else if( field == "translation" )
                        {
                            values.push_back(adjective->translation);
                        }
                        else if( field == "kana" )
                        {
                            values.push_back(adjective->kana);
                        }
                        else if( field == "kanji" )
                        {
                            values.push_back(adjective->kanji);
                        }
                        else if( field == "type" )
                        {
                            values.push_back(adjective->type);
                        }
                    }
                }

                return joinText(values, " + ");
            }

            std::string AdjectiveSentenceQuizWidget::buildQuestionKey(
                const QuestionSeed& seed,
                const std::string& sourceForm,
                const std::string& targetForm) const
            {
                if( (seed.space == nullptr) ||
                    (seed.contextOption == nullptr) )
                {
                    return std::string();
                }

                return
                    seed.space->sourcePattern->id +
                    "->" +
                    seed.space->targetPattern->id +
                    ":" +
                    ((seed.space->subject != nullptr) ?
                        seed.space->subject->id :
                        std::string("no_subject")) +
                    ":" +
                    buildChainInfo(seed.adjectiveChain, "id") +
                    ":" +
                    seed.contextOption->id +
                    ":" +
                    sourceForm +
                    "->" +
                    targetForm +
                    ":" +
                    ((seed.space->copula != nullptr) ?
                        seed.space->copula->id :
                        std::string("no_copula")) +
                    ":" +
                    seed.space->sourceCopulaForm +
                    "->" +
                    seed.space->targetCopulaForm;
            }

            std::string AdjectiveSentenceQuizWidget::getFiniteFormName(
                FormCategory category,
                bool polite) const
            {
                switch( category )
                {
                    case FormCategory::NonPast:
                    {
                        return polite ?
                            "predicate_polite_nonpast" :
                            "predicate_plain_nonpast";
                    }

                    case FormCategory::Negative:
                    {
                        return polite ?
                            "predicate_polite_negative" :
                            "predicate_plain_negative";
                    }

                    case FormCategory::Past:
                    {
                        return polite ?
                            "predicate_polite_past" :
                            "predicate_plain_past";
                    }

                    case FormCategory::PastNegative:
                    {
                        return polite ?
                            "predicate_polite_past_negative" :
                            "predicate_plain_past_negative";
                    }

                    default:
                    {
                        return std::string();
                    }
                }
            }

            std::string AdjectiveSentenceQuizWidget::getCopulaFormName(
                FormCategory category,
                bool polite) const
            {
                switch( category )
                {
                    case FormCategory::NonPast:
                    {
                        return polite ?
                            "polite_nonpast" :
                            "dictionary";
                    }

                    case FormCategory::Negative:
                    {
                        return polite ?
                            "polite_negative" :
                            "plain_negative";
                    }

                    case FormCategory::Past:
                    {
                        return polite ?
                            "past_polite" :
                            "past_plain";
                    }

                    case FormCategory::PastNegative:
                    {
                        return polite ?
                            "past_negative_polite" :
                            "past_negative_plain";
                    }

                    default:
                    {
                        return std::string();
                    }
                }
            }

            const AdjectiveSentenceQuizWidget::FormData*
                AdjectiveSentenceQuizWidget::getForm(
                    const DictionaryWord& word,
                    const std::string& formName) const
            {
                const auto iterator = word.forms.find(formName);
                return (iterator != word.forms.end()) ?
                    &iterator->second :
                    nullptr;
            }

            std::string AdjectiveSentenceQuizWidget::getFormPrompt(
                const FormData& form,
                const std::string& gender) const
            {
                std::string result = form.prompt;

                if( (gender == "masculine") &&
                    !form.promptMasculine.empty() )
                {
                    result = form.promptMasculine;
                }
                else if( (gender == "feminine") &&
                    !form.promptFeminine.empty() )
                {
                    result = form.promptFeminine;
                }
                else if( (gender == "neuter") &&
                    !form.promptNeuter.empty() )
                {
                    result = form.promptNeuter;
                }
                else if( (gender == "plural") &&
                    !form.promptPlural.empty() )
                {
                    result = form.promptPlural;
                }

                if( result.empty() )
                {
                    result = form.value;
                }

                return result;
            }

            const AdjectiveSentenceQuizWidget::ContextPoolData*
                AdjectiveSentenceQuizWidget::getContextPool(
                    const std::string& contextId) const
            {
                const std::string resolvedId =
                    contextId.empty() ? "none" : contextId;
                const auto iterator = m_contexts.find(resolvedId);
                return (iterator != m_contexts.end()) ?
                    &iterator->second :
                    nullptr;
            }

            std::vector<const AdjectiveSentenceQuizWidget::DictionaryWord*>
                AdjectiveSentenceQuizWidget::getCompatibleAdjectives(
                    const EntityData& entity,
                    bool requireConnective,
                    const std::vector<WordFilterRule>& filters) const
            {
                std::vector<const DictionaryWord*> result;
                const auto adjectivesIterator =
                    m_dictionaries.find("adjectives");

                if( adjectivesIterator != m_dictionaries.end() )
                {
                    for( const DictionaryWord& adjective :
                        adjectivesIterator->second )
                    {
                        if( isAdjectiveTypeEnabled(adjective) &&
                            wordMatchesFilter(adjective, filters) &&
                            entityMatchesAdjective(entity, adjective) &&
                            (!requireConnective ||
                                (getForm(adjective, "connective") != nullptr)) )
                        {
                            result.push_back(&adjective);
                        }
                    }
                }

                std::sort(
                    result.begin(),
                    result.end(),
                    [](const DictionaryWord* first,
                        const DictionaryWord* second)
                    {
                        return std::tie(first->kana, first->id) <
                            std::tie(second->kana, second->id);
                    });
                return result;
            }

            bool AdjectiveSentenceQuizWidget::isAdjectiveTypeEnabled(
                const DictionaryWord& adjective) const
            {
                if( adjective.type == "i_adjective" )
                {
                    return m_enableIAdjectives;
                }

                if( adjective.type == "na_adjective" )
                {
                    return m_enableNaAdjectives;
                }

                return false;
            }

            bool AdjectiveSentenceQuizWidget::entityMatchesAdjective(
                const EntityData& entity,
                const DictionaryWord& adjective) const
            {
                const auto tagsIterator =
                    m_effectiveEntityTags.find(entity.id);
                return
                    (tagsIterator != m_effectiveEntityTags.end()) &&
                    tagsIntersect(
                        tagsIterator->second,
                        adjective.tags);
            }

            void AdjectiveSentenceQuizWidget::startQuiz()
            {
                const bool loaded = loadFiles();

                if( loaded )
                {
                    const size_t requestedSize =
                        (m_numberOfQuestions > 0) ?
                        static_cast<size_t>(m_numberOfQuestions) :
                        0U;
                    const bool transformationMode =
                        (m_quizMode == QuizMode::Transformation);
                    m_questions = buildQuestionsForMode(
                        transformationMode,
                        requestedSize);

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
                            "project, filters, adjective count, and form settings.";
                    }
                }
            }

            void AdjectiveSentenceQuizWidget::submitAnswer()
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

            void AdjectiveSentenceQuizWidget::scheduleAutoAdvanceAfterCorrectAnswer()
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

            void AdjectiveSentenceQuizWidget::processAutoAdvance()
            {
                if( m_waitingForNextQuestion &&
                    (m_autoAdvanceDueTime > 0.0) &&
                    (ImGui::GetTime() >= m_autoAdvanceDueTime) )
                {
                    m_autoAdvanceDueTime = 0.0;
                    advanceQuestion();
                }
            }

            void AdjectiveSentenceQuizWidget::acceptCurrentAnswerAsCorrect()
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

            void AdjectiveSentenceQuizWidget::skipQuestion()
            {
                if( m_currentQuestionIndex < m_questions.size() )
                {
                    ++m_correctAnswers;
                    ++m_acceptedAnswers;
                    advanceQuestion();
                }
            }

            void AdjectiveSentenceQuizWidget::advanceQuestion()
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

            void AdjectiveSentenceQuizWidget::resetQuizState()
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

            void AdjectiveSentenceQuizWidget::updateStatistics()
            {
                const bool loaded = loadFiles();

                if( loaded )
                {
                    const auto adjectivesIterator =
                        m_dictionaries.find("adjectives");
                    m_statistics.adjectives =
                        (adjectivesIterator != m_dictionaries.end()) ?
                        static_cast<uint32_t>(
                            adjectivesIterator->second.size()) :
                        0U;
                    m_statistics.sentencePatterns =
                        countValidPatterns();
                    m_statistics.entities =
                        static_cast<uint32_t>(m_entities.size());
                    m_statistics.tagProfiles =
                        static_cast<uint32_t>(m_tagProfiles.size());
                    const bool transformationMode =
                        (m_quizMode == QuizMode::Transformation);
                    const std::vector<QuestionSpace> spaces =
                        buildQuestionSpaces(transformationMode);
                    std::unordered_set<const DictionaryWord*>
                        eligibleFinalChoices;

                    for( const QuestionSpace& space : spaces )
                    {
                        if( space.finalAdjective != nullptr )
                        {
                            eligibleFinalChoices.insert(
                                space.finalAdjective);
                        }
                    }

                    m_statistics.eligibleFinalChoices =
                        static_cast<uint64_t>(
                            eligibleFinalChoices.size());
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
                            break;
                        }

                        m_statistics.generatedQuestions +=
                            space.questionCount;
                    }

                    m_statusMessage =
                        "Statistics refreshed from quiz_project.xml.";
                }
            }

            std::vector<AdjectiveSentenceQuizWidget::WordFilterRule>
                AdjectiveSentenceQuizWidget::parseWordFilter(
                    const std::string& filterText) const
            {
                std::vector<WordFilterRule> rules;
                const std::string trimmedFilter =
                    trimText(filterText);

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

                            for( const std::string& value :
                                splitText(
                                    trimText(
                                        trimmedExpression.substr(
                                            equalPosition + 1U)),
                                    ',') )
                            {
                                const std::string cleaned =
                                    removeOptionalQuotes(
                                        trimText(value));

                                if( !cleaned.empty() )
                                {
                                    rule.values.push_back(cleaned);
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

            bool AdjectiveSentenceQuizWidget::wordMatchesFilter(
                const DictionaryWord& word,
                const std::vector<WordFilterRule>& rules) const
            {
                for( const WordFilterRule& rule : rules )
                {
                    bool ruleMatches = false;

                    for( const std::string& value : rule.values )
                    {
                        if( wordFieldMatches(
                            word,
                            rule.field,
                            value) )
                        {
                            ruleMatches = true;
                        }
                    }

                    if( !ruleMatches )
                    {
                        return false;
                    }
                }

                return true;
            }

            bool AdjectiveSentenceQuizWidget::wordFieldMatches(
                const DictionaryWord& word,
                const std::string& field,
                const std::string& value) const
            {
                if( field == "tag" )
                {
                    return wordHasTag(word, value);
                }

                if( field == "dictionary" )
                {
                    return word.dictionaryId == value;
                }

                const auto iterator = word.attributes.find(field);
                return
                    (iterator != word.attributes.end()) &&
                    (iterator->second == value);
            }

            bool AdjectiveSentenceQuizWidget::wordHasTag(
                const DictionaryWord& word,
                const std::string& tag) const
            {
                return containsValue(word.tags, tag);
            }

            bool AdjectiveSentenceQuizWidget::tagsIntersect(
                const std::vector<std::string>& first,
                const std::vector<std::string>& second) const
            {
                for( const std::string& value : first )
                {
                    if( containsValue(second, value) )
                    {
                        return true;
                    }
                }

                return false;
            }

            bool AdjectiveSentenceQuizWidget::containsValue(
                const std::vector<std::string>& values,
                const std::string& value) const
            {
                return
                    std::find(values.begin(), values.end(), value) !=
                    values.end();
            }

            void AdjectiveSentenceQuizWidget::appendUnique(
                std::vector<std::string>& values,
                const std::string& value) const
            {
                if( !containsValue(values, value) )
                {
                    values.push_back(value);
                }
            }

            std::vector<std::string>
                AdjectiveSentenceQuizWidget::extractTokens(
                    const std::string& text) const
            {
                std::vector<std::string> tokens;
                size_t position = 0U;

                while( position < text.size() )
                {
                    const size_t open = text.find('{', position);

                    if( open == std::string::npos )
                    {
                        break;
                    }

                    const size_t close = text.find('}', open + 1U);

                    if( close == std::string::npos )
                    {
                        break;
                    }

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

                return tokens;
            }

            void AdjectiveSentenceQuizWidget::splitToken(
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

            std::vector<std::string>
                AdjectiveSentenceQuizWidget::splitText(
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

            std::string AdjectiveSentenceQuizWidget::trimText(
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

            std::string AdjectiveSentenceQuizWidget::removeOptionalQuotes(
                const std::string& text) const
            {
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
                        return text.substr(1U, text.size() - 2U);
                    }
                }

                return text;
            }

            std::string AdjectiveSentenceQuizWidget::joinText(
                const std::vector<std::string>& values,
                const std::string& separator) const
            {
                std::string result;

                for( const std::string& value : values )
                {
                    if( !value.empty() )
                    {
                        if( !result.empty() )
                        {
                            result += separator;
                        }

                        result += value;
                    }
                }

                return result;
            }

            std::string AdjectiveSentenceQuizWidget::replaceAll(
                std::string text,
                const std::string& token,
                const std::string& value) const
            {
                if( token.empty() )
                {
                    return text;
                }

                size_t position = 0U;

                while( (position = text.find(token, position)) !=
                    std::string::npos )
                {
                    text.replace(position, token.size(), value);
                    position += value.size();
                }

                return text;
            }

            std::string AdjectiveSentenceQuizWidget::normalizeAnswer(
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

            std::string AdjectiveSentenceQuizWidget::buildConfiguredPath(
                const std::string& path) const
            {
                const std::filesystem::path configured(path);

                if( configured.is_absolute() )
                {
                    return configured.lexically_normal().string();
                }

                return
                    (std::filesystem::path(getexepath()) / configured)
                    .lexically_normal()
                    .string();
            }

            std::string AdjectiveSentenceQuizWidget::resolveProjectFile(
                const std::string& projectDirectory,
                const std::string& path) const
            {
                const std::filesystem::path configured(path);

                if( configured.is_absolute() )
                {
                    return configured.lexically_normal().string();
                }

                return
                    (std::filesystem::path(projectDirectory) / configured)
                    .lexically_normal()
                    .string();
            }

            void AdjectiveSentenceQuizWidget::copyToBuffer(
                char* destination,
                size_t destinationSize,
                const std::string& source)
            {
                if( (destination != nullptr) &&
                    (destinationSize > 0U) )
                {
                    std::memset(destination, 0, destinationSize);
                    std::memcpy(
                        destination,
                        source.c_str(),
                        std::min(
                            source.size(),
                            destinationSize - 1U));
                }
            }
        }
    }
}
