#include "SentenceTransformationQuizWidget.h"

#include "tools/pugixml.hpp"
#include "tools/SystemTools.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <format>
#include <limits>
#include <stdexcept>
#include <tuple>
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

                uint32_t GetUnsignedAttribute(const pugi::xml_node& node, const char* name, uint32_t defaultValue)
                {
                    uint32_t result = defaultValue;
                    const pugi::xml_attribute attribute = node.attribute(name);

                    if( attribute )
                    {
                        const unsigned int parsedValue = attribute.as_uint(defaultValue);
                        result = static_cast<uint32_t>(parsedValue);
                    }

                    return result;
                }
            }

            SentenceTransformationQuizWidget::SentenceTransformationQuizWidget(tools::Logger& logger)
                : m_logger(logger)
                , m_rng(static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count()))
            {
            }

            void SentenceTransformationQuizWidget::initialize(const tools::DataPackage& r_package)
            {
                try
                {
                    const SettingsDataPackage* package = dynamic_cast<const SettingsDataPackage*>(&r_package);

                    if( package != nullptr )
                    {
                        const std::string projectPath = package->get<std::string>(SettingsPackageKey::SentenceQuizSentenceMapPath);
                        const std::string numberOfTries = package->get<std::string>(SettingsPackageKey::TriesForQuiz);

                        copyToBuffer(m_projectPath, sizeof(m_projectPath), projectPath);
                        m_numberOfTries = std::stoi(numberOfTries);

                        m_logger.log(
                            "SentenceTransformationQuizWidget: Initialized with quiz project path.",
                            tools::LogLevel::INFO);
                    }
                }
                catch( const std::exception& exception )
                {
                    m_logger.log(
                        "SentenceTransformationQuizWidget::initialize exception: " + std::string(exception.what()),
                        tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log(
                        "SentenceTransformationQuizWidget::initialize unknown exception.",
                        tools::LogLevel::PROBLEM);
                }
            }

            void SentenceTransformationQuizWidget::draw(bool* p_open)
            {
                try
                {
                    if( p_open != nullptr )
                    {
                        if( *p_open )
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
                }
                catch( const std::exception& exception )
                {
                    m_logger.log(
                        "SentenceTransformationQuizWidget::draw exception: " + std::string(exception.what()),
                        tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log(
                        "SentenceTransformationQuizWidget::draw unknown exception.",
                        tools::LogLevel::PROBLEM);
                }
            }

            void SentenceTransformationQuizWidget::drawSettingsWindow(bool* p_open)
            {
                ImGui::SetNextWindowSize(ImVec2(680, 600), ImGuiCond_FirstUseEver);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.98f, 0.92f, 0.84f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));

                bool windowOpen = true;

                if( ImGui::Begin("Sentence Transformation Quiz Settings", &windowOpen, ImGuiWindowFlags_NoCollapse) )
                {
                    if( ImGui::IsKeyPressed(ImGuiKey_F1) )
                    {
                        if( !m_showStatistics )
                        {
                            updateStatistics();
                        }

                        m_showStatistics = !m_showStatistics;
                    }

                    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Sentence Transformation Quiz");
                    ImGui::TextWrapped(
                        "This quiz loads quiz_project.xml and builds compatible sentences from its dictionaries, "
                        "reusable vocabulary, contexts, and sentence patterns.");
                    ImGui::Separator();

                    ImGui::InputText("Word filter", m_wordFilter, IM_ARRAYSIZE(m_wordFilter));
                    ImGui::TextDisabled(
                        "Examples: ending=su | ending=su,ku,kuru | type=godan;ending=su,ku | "
                        "tag=reporting | id=yomu | dictionary=verbs");

                    ImGui::Spacing();
                    ImGui::SeparatorText("Quiz mode");

                    const char* quizModes[] =
                    {
                        "Transform Japanese sentence",
                        "Write Japanese sentence from source sentence"
                    };

                    if( ImGui::Combo("Mode", &m_quizModeIndex, quizModes, IM_ARRAYSIZE(quizModes)) )
                    {
                        m_quizMode = static_cast<QuizMode>(m_quizModeIndex);
                        m_showStatistics = false;
                    }

                    if( m_quizMode == QuizMode::Transformation )
                    {
                        ImGui::Spacing();
                        ImGui::SeparatorText("Transformation direction");

                        ImGui::Checkbox("Polite to plain", &m_enablePoliteToPlain);
                        ImGui::SameLine();
                        ImGui::Checkbox("Plain to polite", &m_enablePlainToPolite);
                    }
                    else
                    {
                        ImGui::Spacing();
                        ImGui::SeparatorText("Japanese answer style");

                        ImGui::Checkbox("Polite", &m_enablePoliteOutput);
                        ImGui::SameLine();
                        ImGui::Checkbox("Plain / informal", &m_enablePlainOutput);

                        ImGui::TextDisabled("This mode shows the generated source sentence and expects Japanese.");
                    }

                    ImGui::Spacing();
                    ImGui::SeparatorText("Forms");

                    if( ImGui::BeginTable("SentenceQuizFormsTable", 4, ImGuiTableFlags_SizingStretchSame) )
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

                    ImGui::Spacing();
                    ImGui::SeparatorText("Quiz size");

                    ImGui::SliderInt("Number of questions", &m_numberOfQuestions, 1, 200);
                    ImGui::Checkbox("Random order", &m_randomOrder);

                    ImGui::Spacing();
                    ImGui::Separator();

                    if( ImGui::Button("Load files / refresh statistics", ImVec2(240, 0)) )
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

                    ImGui::TextDisabled("Press F1 in this window to show or hide statistics.");

                    if( !m_statusMessage.empty() )
                    {
                        ImGui::Spacing();
                        ImGui::TextWrapped("%s", m_statusMessage.c_str());
                    }

                    if( m_showStatistics )
                    {
                        ImGui::Spacing();
                        ImGui::SeparatorText("Statistics");

                        ImGui::Text("Words: %u", m_statistics.words);
                        ImGui::Text("Enabled sentence patterns: %u", m_statistics.sentencePatterns);
                        ImGui::Text("Reusable entities: %u", m_statistics.entities);
                        ImGui::Text("Tag profiles: %u", m_statistics.tagProfiles);
                        ImGui::Text("Possible questions with current settings: %u", m_statistics.generatedQuestions);
                    }
                }

                ImGui::End();

                if( !windowOpen )
                {
                    m_showSettingsWindow = false;
                }

                ImGui::PopStyleColor();
                ImGui::PopStyleVar();

                if( p_open != nullptr )
                {
                    if( !m_showSettingsWindow && !m_showQuizWindow )
                    {
                        *p_open = false;
                    }
                }
            }

            void SentenceTransformationQuizWidget::drawQuizWindow(bool* p_open)
            {
                ImGui::SetNextWindowSize(ImVec2(660, 500), ImGuiCond_FirstUseEver);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.98f, 0.92f, 0.84f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));

                bool windowOpen = true;

                if( ImGui::Begin("Sentence Transformation Quiz", &windowOpen, ImGuiWindowFlags_NoCollapse) )
                {
                    processAutoAdvance();

                    if( ImGui::IsKeyPressed(ImGuiKey_F1) )
                    {
                        m_showFullAnswer = !m_showFullAnswer;
                    }

                    if( m_questions.empty() || (m_currentQuestionIndex >= m_questions.size()) )
                    {
                        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Quiz complete!");
                        ImGui::Separator();

                        ImGui::Text("Correct answers: %u", m_correctAnswers);
                        ImGui::Text("Wrong answers: %u", m_wrongAnswers);
                        ImGui::Text("Accepted manually: %u", m_acceptedAnswers);

                        ImGui::Spacing();

                        if( ImGui::Button("Back to settings", ImVec2(160, 0)) )
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
                        const QuizQuestion& question = m_questions[m_currentQuestionIndex];
                        const float progress = static_cast<float>(m_currentQuestionIndex) /
                            static_cast<float>(m_questions.size());

                        ImGui::Text("Question %u / %zu", m_currentQuestionIndex + 1U, m_questions.size());
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

                        if( !question.questionText.empty() && (m_quizMode == QuizMode::Transformation) )
                        {
                            ImGui::Spacing();
                            ImGui::TextDisabled("Context: %s", question.questionText.c_str());
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

                        const bool submittedByButton = ImGui::Button("Check", ImVec2(120, 0));
                        ImGui::SameLine();

                        if( ImGui::Button("Accept / Skip", ImVec2(130, 0)) )
                        {
                            skipQuestion();
                        }

                        ImGui::SameLine();

                        if( ImGui::Button("Back to settings", ImVec2(160, 0)) )
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

                        ImGui::Spacing();

                        if( !m_feedbackMessage.empty() )
                        {
                            ImGui::Separator();
                            ImGui::TextWrapped("%s", m_feedbackMessage.c_str());

                            if( m_waitingForNextQuestion )
                            {
                                if( ImGui::Button("Accept as correct", ImVec2(160, 0)) )
                                {
                                    acceptCurrentAnswerAsCorrect();
                                }
                            }
                        }

                        if( m_showFullAnswer )
                        {
                            ImGui::Spacing();
                            ImGui::SeparatorText("F1 full answer");
                            ImGui::TextWrapped("Expected sentence: %s", question.targetSentence.c_str());
                            ImGui::TextDisabled("Verb meaning: %s", question.wordMeaning.c_str());
                            ImGui::TextDisabled("Verb kana: %s", question.wordKana.c_str());
                            ImGui::TextDisabled("Verb kanji: %s", question.wordKanji.c_str());
                            ImGui::TextDisabled("Pattern: %s", question.patternId.c_str());
                            ImGui::TextDisabled("Target form: %s", question.targetConjugation.c_str());

                            if( !question.bindings.empty() )
                            {
                                ImGui::TextWrapped("Bindings: %s", question.bindings.c_str());
                            }
                        }

                        ImGui::TextDisabled("Press F1 to show or hide the full expected answer.");
                    }
                }

                ImGui::End();

                if( !windowOpen )
                {
                    m_showQuizWindow = false;
                }

                ImGui::PopStyleColor();
                ImGui::PopStyleVar();

                if( p_open != nullptr )
                {
                    if( !m_showSettingsWindow && !m_showQuizWindow )
                    {
                        *p_open = false;
                    }
                }
            }

            bool SentenceTransformationQuizWidget::loadFiles()
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

                    if( result )
                    {
                        m_filesLoaded = true;
                        m_statusMessage = std::format(
                            "Quiz project loaded: {} dictionaries, {} patterns, {} entities.",
                            m_dictionaries.size(),
                            m_sentencePatterns.size(),
                            m_entities.size());
                    }
                    else if( m_statusMessage.empty() )
                    {
                        m_statusMessage = "Could not load the quiz project.";
                    }
                }
                catch( const std::exception& exception )
                {
                    m_logger.log(
                        "SentenceTransformationQuizWidget::loadFiles exception: " + std::string(exception.what()),
                        tools::LogLevel::PROBLEM);
                    m_statusMessage = exception.what();
                }
                catch( ... )
                {
                    m_logger.log(
                        "SentenceTransformationQuizWidget::loadFiles unknown exception.",
                        tools::LogLevel::PROBLEM);
                    m_statusMessage = "Unknown error while loading the quiz project.";
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::loadProject(const std::string& path)
            {
                bool result = false;
                m_projectFullPath = buildConfiguredPath(path);

                pugi::xml_document document;
                const pugi::xml_parse_result parseResult = document.load_file(m_projectFullPath.c_str());

                if( parseResult )
                {
                    const pugi::xml_node rootNode = document.child("quiz_project");

                    if( rootNode )
                    {
                        m_projectDirectory = std::filesystem::path(m_projectFullPath).parent_path().string();
                        bool allFilesLoaded = true;
                        uint32_t loadedDictionaries = 0U;

                        const pugi::xml_node dictionariesNode = rootNode.child("dictionaries");

                        for( pugi::xml_node dictionaryNode = dictionariesNode.child("dictionary");
                            dictionaryNode;
                            dictionaryNode = dictionaryNode.next_sibling("dictionary") )
                        {
                            const std::string dictionaryId = GetAttribute(dictionaryNode, "id");
                            const std::string file = GetAttribute(dictionaryNode, "file");

                            if( !dictionaryId.empty() && !file.empty() )
                            {
                                const std::string fullPath = resolveProjectFile(m_projectDirectory, file);
                                const bool dictionaryLoaded = loadDictionary(dictionaryId, fullPath);
                                allFilesLoaded = allFilesLoaded && dictionaryLoaded;

                                if( dictionaryLoaded )
                                {
                                    ++loadedDictionaries;
                                }
                            }
                            else
                            {
                                allFilesLoaded = false;
                            }
                        }

                        const std::string vocabularyFile = GetAttribute(rootNode.child("vocabulary"), "file");
                        const std::string contextsFile = GetAttribute(rootNode.child("contexts"), "file");
                        const std::string sentenceMapsFile = GetAttribute(rootNode.child("sentence_maps"), "file");

                        const bool vocabularyLoaded = !vocabularyFile.empty() &&
                            loadVocabulary(resolveProjectFile(m_projectDirectory, vocabularyFile));
                        const bool contextsLoaded = !contextsFile.empty() &&
                            loadContexts(resolveProjectFile(m_projectDirectory, contextsFile));
                        const bool patternsLoaded = !sentenceMapsFile.empty() &&
                            loadSentencePatterns(resolveProjectFile(m_projectDirectory, sentenceMapsFile));

                        allFilesLoaded = allFilesLoaded &&
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
                        "Could not load sentence quiz project XML from path: " +
                        m_projectFullPath +
                        ". Parser status: " +
                        parseResult.description(),
                        tools::LogLevel::PROBLEM);
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::loadDictionary(
                const std::string& dictionaryId,
                const std::string& fullPath)
            {
                bool result = false;
                pugi::xml_document document;
                const pugi::xml_parse_result parseResult = document.load_file(fullPath.c_str());

                if( parseResult )
                {
                    const pugi::xml_node rootNode = document.child("dictionary");
                    const pugi::xml_node wordsNode = rootNode.child("words");
                    std::vector<DictionaryWord> words;

                    for( pugi::xml_node wordNode = wordsNode.child("word");
                        wordNode;
                        wordNode = wordNode.next_sibling("word") )
                    {
                        DictionaryWord word;
                        word.dictionaryId = dictionaryId;

                        for( pugi::xml_attribute attribute = wordNode.first_attribute();
                            attribute;
                            attribute = attribute.next_attribute() )
                        {
                            word.attributes[attribute.name()] = attribute.value();
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

                        const pugi::xml_node relationsNode = wordNode.child("relations");

                        for( pugi::xml_node relationNode = relationsNode.child("relation");
                            relationNode;
                            relationNode = relationNode.next_sibling("relation") )
                        {
                            const std::string role = GetAttribute(relationNode, "role");

                            if( !role.empty() )
                            {
                                std::vector<std::string>& relationTags = word.relations[role];

                                for( pugi::xml_node tagNode = relationNode.child("tag");
                                    tagNode;
                                    tagNode = tagNode.next_sibling("tag") )
                                {
                                    const std::string tag = trimText(GetText(tagNode));

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
                                word.forms[form.name] = form;
                            }
                        }

                        if( !word.id.empty() && !word.kana.empty() && !word.forms.empty() )
                        {
                            words.push_back(word);
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
                        "Could not load dictionary '" + dictionaryId + "' from path: " +
                        fullPath +
                        ". Parser status: " +
                        parseResult.description(),
                        tools::LogLevel::PROBLEM);
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::loadVocabulary(const std::string& fullPath)
            {
                bool result = false;
                pugi::xml_document document;
                const pugi::xml_parse_result parseResult = document.load_file(fullPath.c_str());

                if( parseResult )
                {
                    const pugi::xml_node rootNode = document.child("sentence_vocabulary");
                    const pugi::xml_node profilesNode = rootNode.child("tag_profiles");

                    for( pugi::xml_node profileNode = profilesNode.child("profile");
                        profileNode;
                        profileNode = profileNode.next_sibling("profile") )
                    {
                        TagProfile profile;
                        profile.id = GetAttribute(profileNode, "id");

                        const pugi::xml_node includesNode = profileNode.child("includes");

                        for( pugi::xml_node includedNode = includesNode.child("profile");
                            includedNode;
                            includedNode = includedNode.next_sibling("profile") )
                        {
                            const std::string includedId = GetAttribute(includedNode, "ref");

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
                            m_tagProfiles[profile.id] = profile;
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

                        const pugi::xml_node polishNode = entityNode.child("polish");
                        entity.gender = GetAttribute(polishNode, "gender");

                        for( pugi::xml_attribute attribute = polishNode.first_attribute();
                            attribute;
                            attribute = attribute.next_attribute() )
                        {
                            if( std::string(attribute.name()) != "gender" )
                            {
                                entity.polish[attribute.name()] = attribute.value();
                            }
                        }

                        const pugi::xml_node entityProfilesNode = entityNode.child("profiles");

                        for( pugi::xml_node profileNode = entityProfilesNode.child("profile");
                            profileNode;
                            profileNode = profileNode.next_sibling("profile") )
                        {
                            const std::string profileId = GetAttribute(profileNode, "ref");

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

                        if( !entity.id.empty() && !entity.kana.empty() )
                        {
                            m_entities[entity.id] = entity;
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

            bool SentenceTransformationQuizWidget::loadContexts(const std::string& fullPath)
            {
                bool result = false;
                pugi::xml_document document;
                const pugi::xml_parse_result parseResult = document.load_file(fullPath.c_str());

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

                        for( pugi::xml_node optionNode = contextNode.child("option");
                            optionNode;
                            optionNode = optionNode.next_sibling("option") )
                        {
                            ContextOptionData option;
                            option.id = GetAttribute(optionNode, "id");
                            option.polishPrefix = GetAttribute(optionNode, "pl_prefix");
                            option.japanesePrefix = GetAttribute(optionNode, "jp_prefix");
                            option.weight = std::max(1U, GetUnsignedAttribute(optionNode, "weight", 1U));

                            if( !option.id.empty() )
                            {
                                pool.options.push_back(option);
                            }
                        }

                        if( !pool.id.empty() && !pool.options.empty() )
                        {
                            m_contexts[pool.id] = pool;
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

            bool SentenceTransformationQuizWidget::loadSentencePatterns(const std::string& fullPath)
            {
                bool result = false;
                pugi::xml_document document;
                const pugi::xml_parse_result parseResult = document.load_file(fullPath.c_str());

                if( parseResult )
                {
                    const pugi::xml_node rootNode = document.child("sentence_maps");
                    const pugi::xml_node patternsNode = rootNode.child("sentence_patterns");

                    for( pugi::xml_node patternNode = patternsNode.child("sentence_pattern");
                        patternNode;
                        patternNode = patternNode.next_sibling("sentence_pattern") )
                    {
                        SentencePatternData pattern;
                        pattern.id = GetAttribute(patternNode, "id");
                        pattern.label = GetAttribute(patternNode, "label");
                        pattern.questionTemplate = trimText(GetText(patternNode.child("question")));
                        pattern.answerTemplate = trimText(GetText(patternNode.child("answer")));

                        if( !pattern.id.empty() &&
                            !pattern.questionTemplate.empty() &&
                            !pattern.answerTemplate.empty() )
                        {
                            m_sentencePatterns.push_back(pattern);
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

            bool SentenceTransformationQuizWidget::buildEffectiveEntityTags()
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

                        if( !appendProfileTags(profileId, recursionPath, effectiveTags, error) )
                        {
                            result = false;
                            m_statusMessage = error;
                            m_logger.log(
                                "SentenceTransformationQuizWidget profile error: " + error,
                                tools::LogLevel::PROBLEM);
                        }
                    }

                    for( const std::string& tag : entity.tags )
                    {
                        appendUnique(effectiveTags, tag);
                    }

                    if( result )
                    {
                        m_effectiveEntityTags[entity.id] = effectiveTags;
                    }
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::appendProfileTags(
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
                    recursionPath.push_back(profileId);
                    error = "Tag profile inheritance cycle: " + joinText(recursionPath, " -> ");
                    recursionPath.pop_back();
                }
                else
                {
                    result = true;
                    recursionPath.push_back(profileId);

                    for( const std::string& includedProfile : profileIterator->second.includes )
                    {
                        if( !appendProfileTags(includedProfile, recursionPath, tags, error) )
                        {
                            result = false;
                        }
                    }

                    if( result )
                    {
                        for( const std::string& tag : profileIterator->second.tags )
                        {
                            appendUnique(tags, tag);
                        }
                    }

                    recursionPath.pop_back();
                }

                return result;
            }

            SentenceTransformationQuizWidget::SlotKind
                SentenceTransformationQuizWidget::getSlotKind(const std::string& slotId) const
            {
                SlotKind result = SlotKind::Entity;

                if( (slotId == "adjective") ||
                    (slotId == "predicate") ||
                    (slotId == "i_adjective") ||
                    (slotId == "na_adjective") )
                {
                    result = SlotKind::Adjective;
                }
                else if( (slotId == "verb") ||
                    (slotId.rfind("verb_", 0U) == 0U) ||
                    ((slotId.size() >= 5U) && (slotId.substr(slotId.size() - 5U) == "_verb")) ||
                    ((slotId.size() >= 10U) && (slotId.substr(slotId.size() - 10U) == "_auxiliary")) )
                {
                    result = SlotKind::Verb;
                }

                return result;
            }

            std::string SentenceTransformationQuizWidget::getDefaultFormSpec(const std::string& slotId) const
            {
                std::string result = "form";

                if( (getSlotKind(slotId) == SlotKind::Adjective) && (slotId != "predicate") )
                {
                    result = "attributive";
                }

                return result;
            }

            std::string SentenceTransformationQuizWidget::getWordSlotFilterTag(const std::string& slotId) const
            {
                std::string result;

                if( (slotId.size() >= 5U) &&
                    (slotId.substr(slotId.size() - 5U) == "_verb") &&
                    (slotId.rfind("verb_", 0U) != 0U) )
                {
                    result = slotId.substr(0U, slotId.size() - 5U);
                }
                else if( (slotId.size() >= 10U) &&
                    (slotId.substr(slotId.size() - 10U) == "_auxiliary") )
                {
                    result = slotId;
                }

                return result;
            }

            std::string SentenceTransformationQuizWidget::getDictionaryForSlot(const std::string& slotId) const
            {
                std::string result = "verbs";

                if( getSlotKind(slotId) == SlotKind::Adjective )
                {
                    result = "adjectives";
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::isFormSelector(
                const std::string& slotId,
                const std::string& field) const
            {
                bool result = false;

                if( getSlotKind(slotId) != SlotKind::Entity )
                {
                    if( field.empty() )
                    {
                        result = true;
                    }
                    else
                    {
                        static const std::vector<std::string> metadataFields =
                        {
                            "id",
                            "kana",
                            "kanji",
                            "translation",
                            "type",
                            "ending",
                            "categories",
                            "tags",
                            "prompt",
                            "style",
                            "form_name"
                        };

                        result = !containsValue(metadataFields, field);
                    }
                }

                return result;
            }

            SentenceTransformationQuizWidget::PatternAnalysis
                SentenceTransformationQuizWidget::analyzePattern(const SentencePatternData& pattern) const
            {
                PatternAnalysis analysis;
                std::unordered_map<std::string, std::vector<std::string>> specsBySlot;
                const std::vector<std::string> expressions =
                    extractTokens(pattern.answerTemplate + "\n" + pattern.questionTemplate);

                for( const std::string& expression : expressions )
                {
                    std::string root;
                    std::string field;
                    splitToken(expression, root, field);

                    if( root != "context" )
                    {
                        const SlotKind kind = getSlotKind(root);

                        if( kind == SlotKind::Entity )
                        {
                            appendUnique(analysis.entitySlots, root);
                        }
                        else
                        {
                            appendUnique(analysis.wordSlots, root);

                            if( isFormSelector(root, field) )
                            {
                                specsBySlot[root].push_back(field.empty() ? getDefaultFormSpec(root) : field);
                            }
                        }
                    }
                }

                std::vector<std::string> dynamicSlots;

                for( const std::string& slotId : analysis.wordSlots )
                {
                    std::string primarySpec = getDefaultFormSpec(slotId);
                    const auto specsIterator = specsBySlot.find(slotId);

                    if( specsIterator != specsBySlot.end() )
                    {
                        const std::vector<std::string>& specs = specsIterator->second;

                        if( containsValue(specs, "form") )
                        {
                            primarySpec = "form";
                        }
                        else if( !specs.empty() )
                        {
                            primarySpec = specs.front();
                        }
                    }

                    analysis.primarySpecs[slotId] = primarySpec;

                    if( primarySpec == "form" )
                    {
                        dynamicSlots.push_back(slotId);
                    }
                }

                if( dynamicSlots.size() > 1U )
                {
                    analysis.error = "Pattern '" + pattern.id +
                        "' has more than one dynamic .form target: " +
                        joinText(dynamicSlots, ", ");
                }
                else
                {
                    if( dynamicSlots.size() == 1U )
                    {
                        analysis.dynamicSlot = dynamicSlots.front();
                    }

                    for( const std::string& entitySlot : analysis.entitySlots )
                    {
                        std::vector<std::string> relationVerbSlots;

                        for( const std::string& wordSlot : analysis.wordSlots )
                        {
                            if( getSlotKind(wordSlot) == SlotKind::Verb )
                            {
                                const std::string filterTag = getWordSlotFilterTag(wordSlot);
                                const auto verbsIterator = m_dictionaries.find("verbs");

                                if( verbsIterator != m_dictionaries.end() )
                                {
                                    bool relationFound = false;

                                    for( const DictionaryWord& word : verbsIterator->second )
                                    {
                                        const bool filterMatches = filterTag.empty() || wordHasTag(word, filterTag);
                                        const auto relationIterator = word.relations.find(entitySlot);

                                        if( filterMatches &&
                                            (relationIterator != word.relations.end()) &&
                                            !relationIterator->second.empty() )
                                        {
                                            relationFound = true;
                                        }
                                    }

                                    if( relationFound )
                                    {
                                        relationVerbSlots.push_back(wordSlot);
                                    }
                                }
                            }
                        }

                        if( !analysis.dynamicSlot.empty() &&
                            (getSlotKind(analysis.dynamicSlot) == SlotKind::Verb) &&
                            containsValue(relationVerbSlots, analysis.dynamicSlot) )
                        {
                            analysis.driverForEntity[entitySlot] = analysis.dynamicSlot;
                        }
                        else if( relationVerbSlots.size() == 1U )
                        {
                            analysis.driverForEntity[entitySlot] = relationVerbSlots.front();
                        }
                        else if( relationVerbSlots.size() > 1U )
                        {
                            analysis.error = "Entity role '" + entitySlot +
                                "' has more than one possible verb driver in pattern '" +
                                pattern.id +
                                "': " +
                                joinText(relationVerbSlots, ", ");
                        }
                    }

                    for( const std::string& wordSlot : analysis.wordSlots )
                    {
                        if( (getSlotKind(wordSlot) == SlotKind::Adjective) &&
                            !analysis.entitySlots.empty() )
                        {
                            if( analysis.entitySlots.size() == 1U )
                            {
                                analysis.entityForAdjective[wordSlot] = analysis.entitySlots.front();
                            }
                            else
                            {
                                analysis.error = "Adjective slot '" +
                                    wordSlot +
                                    "' has more than one possible entity in pattern '" +
                                    pattern.id +
                                    "'.";
                            }
                        }
                    }
                }

                analysis.valid = analysis.error.empty();
                return analysis;
            }

            bool SentenceTransformationQuizWidget::isUsableVerbPattern(const PatternAnalysis& analysis) const
            {
                const bool result = analysis.valid &&
                    !analysis.dynamicSlot.empty() &&
                    (getSlotKind(analysis.dynamicSlot) == SlotKind::Verb);

                return result;
            }

            bool SentenceTransformationQuizWidget::isEnabledForThisQuiz(
                const SentencePatternData& pattern) const
            {
                static const std::vector<std::string> enabledPatternIds =
                {
                    "simple",
                    "transitive_object",
                    "destination",
                    "companion"
                };
                const bool result = containsValue(enabledPatternIds, pattern.id);

                return result;
            }

            std::vector<SentenceTransformationQuizWidget::GenerationRule>
                SentenceTransformationQuizWidget::buildEnabledRules() const
            {
                std::vector<GenerationRule> rules;

                if( m_enablePoliteToPlain )
                {
                    if( m_enableNonPast )
                    {
                        rules.push_back(
                            { FormCategory::NonPast, true, true, false, "plain non-past affirmative" });
                    }

                    if( m_enableNegative )
                    {
                        rules.push_back(
                            { FormCategory::Negative, true, true, false, "plain non-past negative" });
                    }

                    if( m_enablePast )
                    {
                        rules.push_back(
                            { FormCategory::Past, true, true, false, "plain past affirmative" });
                    }

                    if( m_enablePastNegative )
                    {
                        rules.push_back(
                            { FormCategory::PastNegative, true, true, false, "plain past negative" });
                    }
                }

                if( m_enablePlainToPolite )
                {
                    if( m_enableNonPast )
                    {
                        rules.push_back(
                            { FormCategory::NonPast, true, false, true, "polite non-past affirmative" });
                    }

                    if( m_enableNegative )
                    {
                        rules.push_back(
                            { FormCategory::Negative, true, false, true, "polite non-past negative" });
                    }

                    if( m_enablePast )
                    {
                        rules.push_back(
                            { FormCategory::Past, true, false, true, "polite past affirmative" });
                    }

                    if( m_enablePastNegative )
                    {
                        rules.push_back(
                            { FormCategory::PastNegative, true, false, true, "polite past negative" });
                    }
                }

                return rules;
            }

            std::vector<SentenceTransformationQuizWidget::GenerationRule>
                SentenceTransformationQuizWidget::buildEnabledMeaningToJapaneseRules() const
            {
                std::vector<GenerationRule> rules;

                if( m_enablePlainOutput )
                {
                    if( m_enableNonPast )
                    {
                        rules.push_back(
                            { FormCategory::NonPast, false, false, false, "plain non-past affirmative" });
                    }

                    if( m_enableNegative )
                    {
                        rules.push_back(
                            { FormCategory::Negative, false, false, false, "plain non-past negative" });
                    }

                    if( m_enablePast )
                    {
                        rules.push_back(
                            { FormCategory::Past, false, false, false, "plain past affirmative" });
                    }

                    if( m_enablePastNegative )
                    {
                        rules.push_back(
                            { FormCategory::PastNegative, false, false, false, "plain past negative" });
                    }
                }

                if( m_enablePoliteOutput )
                {
                    if( m_enableNonPast )
                    {
                        rules.push_back(
                            { FormCategory::NonPast, false, false, true, "polite non-past affirmative" });
                    }

                    if( m_enableNegative )
                    {
                        rules.push_back(
                            { FormCategory::Negative, false, false, true, "polite non-past negative" });
                    }

                    if( m_enablePast )
                    {
                        rules.push_back(
                            { FormCategory::Past, false, false, true, "polite past affirmative" });
                    }

                    if( m_enablePastNegative )
                    {
                        rules.push_back(
                            { FormCategory::PastNegative, false, false, true, "polite past negative" });
                    }
                }

                return rules;
            }

            std::string SentenceTransformationQuizWidget::getFiniteFormName(
                const DictionaryWord& word,
                FormCategory category,
                bool polite) const
            {
                std::string result;
                const bool adjective = (word.dictionaryId == "adjectives");

                switch( category )
                {
                    case FormCategory::NonPast:
                    {
                        if( adjective )
                        {
                            result = polite ? "predicate_polite_nonpast" : "predicate_plain_nonpast";
                        }
                        else
                        {
                            result = polite ? "polite_nonpast" : "dictionary";
                        }
                        break;
                    }

                    case FormCategory::Negative:
                    {
                        if( adjective )
                        {
                            result = polite ? "predicate_polite_negative" : "predicate_plain_negative";
                        }
                        else
                        {
                            result = polite ? "polite_negative" : "plain_negative";
                        }
                        break;
                    }

                    case FormCategory::Past:
                    {
                        if( adjective )
                        {
                            result = polite ? "predicate_polite_past" : "predicate_plain_past";
                        }
                        else
                        {
                            result = polite ? "past_polite" : "past_plain";
                        }
                        break;
                    }

                    case FormCategory::PastNegative:
                    {
                        if( adjective )
                        {
                            result = polite ?
                                "predicate_polite_past_negative" :
                                "predicate_plain_past_negative";
                        }
                        else
                        {
                            result = polite ? "past_negative_polite" : "past_negative_plain";
                        }
                        break;
                    }

                    default:
                    {
                        break;
                    }
                }

                return result;
            }

            std::string SentenceTransformationQuizWidget::getCanonicalFormName(
                const DictionaryWord& word,
                const std::string& formSpec,
                const std::string& dynamicForm) const
            {
                std::string result = formSpec;

                if( formSpec == "form" )
                {
                    result = dynamicForm;
                }
                else if( word.forms.find(formSpec) != word.forms.end() )
                {
                    result = formSpec;
                }
                else if( formSpec == "te" )
                {
                    result = "te_form";
                }
                else if( formSpec == "attributive" )
                {
                    result = "attributive_nonpast";
                }
                else if( word.dictionaryId == "adjectives" )
                {
                    static const std::unordered_map<std::string, std::string> adjectiveAliases =
                    {
                        { "plain_nonpast", "predicate_plain_nonpast" },
                        { "polite_nonpast", "predicate_polite_nonpast" },
                        { "plain_negative", "predicate_plain_negative" },
                        { "polite_negative", "predicate_polite_negative" },
                        { "plain_past", "predicate_plain_past" },
                        { "past_plain", "predicate_plain_past" },
                        { "polite_past", "predicate_polite_past" },
                        { "past_polite", "predicate_polite_past" },
                        { "plain_past_negative", "predicate_plain_past_negative" },
                        { "past_negative_plain", "predicate_plain_past_negative" },
                        { "polite_past_negative", "predicate_polite_past_negative" },
                        { "past_negative_polite", "predicate_polite_past_negative" }
                    };
                    const auto iterator = adjectiveAliases.find(formSpec);

                    if( iterator != adjectiveAliases.end() )
                    {
                        result = iterator->second;
                    }
                }
                else
                {
                    static const std::unordered_map<std::string, std::string> verbAliases =
                    {
                        { "plain_nonpast", "dictionary" },
                        { "plain_past", "past_plain" },
                        { "polite_past", "past_polite" },
                        { "plain_past_negative", "past_negative_plain" },
                        { "polite_past_negative", "past_negative_polite" }
                    };
                    const auto iterator = verbAliases.find(formSpec);

                    if( iterator != verbAliases.end() )
                    {
                        result = iterator->second;
                    }
                }

                return result;
            }

            const SentenceTransformationQuizWidget::FormData*
                SentenceTransformationQuizWidget::getForm(
                    const DictionaryWord& word,
                    const std::string& formSpec,
                    const std::string& dynamicForm) const
            {
                const FormData* result = nullptr;
                const std::string formName = getCanonicalFormName(word, formSpec, dynamicForm);
                const auto formIterator = word.forms.find(formName);

                if( formIterator != word.forms.end() )
                {
                    result = &formIterator->second;
                }

                return result;
            }

            std::string SentenceTransformationQuizWidget::getFormPrompt(
                const FormData& form,
                const std::string& gender) const
            {
                std::string result = form.prompt;

                if( (gender == "masculine") && !form.promptMasculine.empty() )
                {
                    result = form.promptMasculine;
                }
                else if( (gender == "feminine") && !form.promptFeminine.empty() )
                {
                    result = form.promptFeminine;
                }
                else if( (gender == "neuter") && !form.promptNeuter.empty() )
                {
                    result = form.promptNeuter;
                }
                else if( (gender == "plural") && !form.promptPlural.empty() )
                {
                    result = form.promptPlural;
                }

                if( result.empty() )
                {
                    result = form.value;
                }

                return result;
            }

            const SentenceTransformationQuizWidget::ContextPoolData*
                SentenceTransformationQuizWidget::getContextPool(const std::string& contextId) const
            {
                const ContextPoolData* result = nullptr;
                const std::string resolvedId = contextId.empty() ? "none" : contextId;
                const auto contextIterator = m_contexts.find(resolvedId);

                if( contextIterator != m_contexts.end() )
                {
                    result = &contextIterator->second;
                }

                return result;
            }

            std::vector<const SentenceTransformationQuizWidget::DictionaryWord*>
                SentenceTransformationQuizWidget::getVerbCandidates(
                    const std::string& slotId,
                    const PatternAnalysis& analysis,
                    const GenerationRule& rule) const
            {
                std::vector<const DictionaryWord*> candidates;
                const auto dictionaryIterator = m_dictionaries.find("verbs");

                if( dictionaryIterator != m_dictionaries.end() )
                {
                    const std::vector<WordFilterRule> filters =
                        parseWordFilter(std::string(m_wordFilter));
                    const std::string slotFilterTag = getWordSlotFilterTag(slotId);
                    const bool dynamicSlot = (slotId == analysis.dynamicSlot);

                    for( const DictionaryWord& word : dictionaryIterator->second )
                    {
                        bool eligible = slotFilterTag.empty() || wordHasTag(word, slotFilterTag);

                        if( eligible && dynamicSlot )
                        {
                            const std::string targetForm =
                                getFiniteFormName(word, rule.category, rule.targetPolite);
                            eligible = (word.forms.find(targetForm) != word.forms.end()) &&
                                wordMatchesFilter(word, filters);

                            if( eligible && rule.hasSourceForm )
                            {
                                const std::string sourceForm =
                                    getFiniteFormName(word, rule.category, rule.sourcePolite);
                                eligible = (word.forms.find(sourceForm) != word.forms.end());
                            }
                        }
                        else if( eligible )
                        {
                            const auto primaryIterator = analysis.primarySpecs.find(slotId);

                            if( primaryIterator != analysis.primarySpecs.end() )
                            {
                                eligible =
                                    (getForm(word, primaryIterator->second, std::string()) != nullptr);
                            }
                        }

                        if( eligible )
                        {
                            for( const auto& driverEntry : analysis.driverForEntity )
                            {
                                if( driverEntry.second == slotId )
                                {
                                    const std::string& role = driverEntry.first;
                                    const auto relationIterator = word.relations.find(role);
                                    bool matchingEntityExists = false;

                                    if( relationIterator != word.relations.end() )
                                    {
                                        for( const auto& entityEntry : m_entities )
                                        {
                                            if( wordCanUseEntity(word, role, entityEntry.second) )
                                            {
                                                matchingEntityExists = true;
                                            }
                                        }
                                    }

                                    eligible = eligible &&
                                        (relationIterator != word.relations.end()) &&
                                        matchingEntityExists;
                                }
                            }
                        }

                        if( eligible )
                        {
                            candidates.push_back(&word);
                        }
                    }
                }

                std::sort(
                    candidates.begin(),
                    candidates.end(),
                    [](const DictionaryWord* first, const DictionaryWord* second)
                    {
                        return std::tie(first->kana, first->id) < std::tie(second->kana, second->id);
                    });

                return candidates;
            }

            std::vector<const SentenceTransformationQuizWidget::EntityData*>
                SentenceTransformationQuizWidget::getEntityCandidates(
                    const std::string& slotId,
                    const PatternAnalysis& analysis,
                    const SentenceBinding& binding) const
            {
                std::vector<const EntityData*> candidates;
                const auto driverIterator = analysis.driverForEntity.find(slotId);
                bool adjectiveEntity = false;

                for( const auto& adjectiveEntry : analysis.entityForAdjective )
                {
                    if( adjectiveEntry.second == slotId )
                    {
                        adjectiveEntity = true;
                    }
                }

                for( const auto& entityEntry : m_entities )
                {
                    const EntityData& entity = entityEntry.second;
                    bool eligible = true;

                    if( driverIterator != analysis.driverForEntity.end() )
                    {
                        const auto wordIterator = binding.words.find(driverIterator->second);
                        eligible = (wordIterator != binding.words.end()) &&
                            wordCanUseEntity(*wordIterator->second, slotId, entity);
                    }
                    else if( adjectiveEntity )
                    {
                        eligible = false;
                        const auto adjectivesIterator = m_dictionaries.find("adjectives");

                        if( adjectivesIterator != m_dictionaries.end() )
                        {
                            for( const DictionaryWord& adjective : adjectivesIterator->second )
                            {
                                if( entityMatchesAdjective(entity, adjective) )
                                {
                                    eligible = true;
                                }
                            }
                        }
                    }

                    if( eligible )
                    {
                        candidates.push_back(&entity);
                    }
                }

                std::sort(
                    candidates.begin(),
                    candidates.end(),
                    [](const EntityData* first, const EntityData* second)
                    {
                        return std::tie(first->kana, first->id) < std::tie(second->kana, second->id);
                    });

                return candidates;
            }

            std::vector<const SentenceTransformationQuizWidget::DictionaryWord*>
                SentenceTransformationQuizWidget::getAdjectiveCandidates(
                    const std::string& slotId,
                    const PatternAnalysis& analysis,
                    const SentenceBinding& binding,
                    const GenerationRule& rule) const
            {
                std::vector<const DictionaryWord*> candidates;
                const auto dictionaryIterator = m_dictionaries.find("adjectives");

                if( dictionaryIterator != m_dictionaries.end() )
                {
                    const bool dynamicSlot = (slotId == analysis.dynamicSlot);
                    const std::vector<WordFilterRule> filters =
                        parseWordFilter(std::string(m_wordFilter));

                    for( const DictionaryWord& adjective : dictionaryIterator->second )
                    {
                        bool eligible = true;

                        if( slotId == "i_adjective" )
                        {
                            eligible = (adjective.type == "i_adjective");
                        }
                        else if( slotId == "na_adjective" )
                        {
                            eligible = (adjective.type == "na_adjective");
                        }

                        const auto entitySlotIterator = analysis.entityForAdjective.find(slotId);

                        if( eligible && (entitySlotIterator != analysis.entityForAdjective.end()) )
                        {
                            const auto entityIterator = binding.entities.find(entitySlotIterator->second);
                            eligible = (entityIterator != binding.entities.end()) &&
                                entityMatchesAdjective(*entityIterator->second, adjective);
                        }

                        if( eligible && dynamicSlot )
                        {
                            const std::string targetForm =
                                getFiniteFormName(adjective, rule.category, rule.targetPolite);
                            eligible = (adjective.forms.find(targetForm) != adjective.forms.end()) &&
                                wordMatchesFilter(adjective, filters);

                            if( eligible && rule.hasSourceForm )
                            {
                                const std::string sourceForm =
                                    getFiniteFormName(adjective, rule.category, rule.sourcePolite);
                                eligible = (adjective.forms.find(sourceForm) != adjective.forms.end());
                            }
                        }
                        else if( eligible )
                        {
                            const auto primaryIterator = analysis.primarySpecs.find(slotId);

                            if( primaryIterator != analysis.primarySpecs.end() )
                            {
                                eligible =
                                    (getForm(adjective, primaryIterator->second, std::string()) != nullptr);
                            }
                        }

                        if( eligible )
                        {
                            candidates.push_back(&adjective);
                        }
                    }
                }

                std::sort(
                    candidates.begin(),
                    candidates.end(),
                    [](const DictionaryWord* first, const DictionaryWord* second)
                    {
                        return std::tie(first->kana, first->id) < std::tie(second->kana, second->id);
                    });

                return candidates;
            }

            std::vector<SentenceTransformationQuizWidget::SentenceBinding>
                SentenceTransformationQuizWidget::buildBindings(
                    const PatternAnalysis& analysis,
                    const GenerationRule& rule) const
            {
                std::vector<SentenceBinding> bindings(1U);

                for( const std::string& slotId : analysis.wordSlots )
                {
                    if( getSlotKind(slotId) == SlotKind::Verb )
                    {
                        const std::vector<const DictionaryWord*> candidates =
                            getVerbCandidates(slotId, analysis, rule);
                        std::vector<SentenceBinding> expanded;

                        for( const SentenceBinding& binding : bindings )
                        {
                            for( const DictionaryWord* candidate : candidates )
                            {
                                SentenceBinding value = binding;
                                value.words[slotId] = candidate;
                                expanded.push_back(value);
                            }
                        }

                        bindings = std::move(expanded);
                    }
                }

                for( const std::string& slotId : analysis.entitySlots )
                {
                    std::vector<SentenceBinding> expanded;

                    for( const SentenceBinding& binding : bindings )
                    {
                        const std::vector<const EntityData*> candidates =
                            getEntityCandidates(slotId, analysis, binding);

                        for( const EntityData* candidate : candidates )
                        {
                            SentenceBinding value = binding;
                            value.entities[slotId] = candidate;
                            expanded.push_back(value);
                        }
                    }

                    bindings = std::move(expanded);
                }

                for( const std::string& slotId : analysis.wordSlots )
                {
                    if( getSlotKind(slotId) == SlotKind::Adjective )
                    {
                        std::vector<SentenceBinding> expanded;

                        for( const SentenceBinding& binding : bindings )
                        {
                            const std::vector<const DictionaryWord*> candidates =
                                getAdjectiveCandidates(slotId, analysis, binding, rule);

                            for( const DictionaryWord* candidate : candidates )
                            {
                                SentenceBinding value = binding;
                                value.words[slotId] = candidate;
                                expanded.push_back(value);
                            }
                        }

                        bindings = std::move(expanded);
                    }
                }

                return bindings;
            }

            std::vector<SentenceTransformationQuizWidget::QuizQuestion>
                SentenceTransformationQuizWidget::buildQuestionBank(size_t maximumQuestions) const
            {
                return buildQuestionsForMode(true, maximumQuestions);
            }

            std::vector<SentenceTransformationQuizWidget::QuizQuestion>
                SentenceTransformationQuizWidget::buildMeaningToJapaneseQuestionBank(
                    size_t maximumQuestions) const
            {
                return buildQuestionsForMode(false, maximumQuestions);
            }

            std::vector<SentenceTransformationQuizWidget::QuizQuestion>
                SentenceTransformationQuizWidget::buildQuestionsForMode(
                    bool transformationMode,
                    size_t maximumQuestions) const
            {
                std::vector<QuizQuestion> questions;
                std::vector<QuestionSeed> seeds = buildQuestionSeeds(transformationMode);

                if( m_randomOrder )
                {
                    applyWeightedRandomOrder(seeds);
                }

                const size_t reserveSize = (maximumQuestions > 0U) ?
                    std::min(maximumQuestions, seeds.size()) :
                    seeds.size();
                questions.reserve(reserveSize);

                for( const QuestionSeed& seed : seeds )
                {
                    if( (maximumQuestions == 0U) || (questions.size() < maximumQuestions) )
                    {
                        QuizQuestion question;

                        if( renderQuestionSeed(seed, transformationMode, question) )
                        {
                            questions.push_back(question);
                        }
                    }
                }

                return questions;
            }

            std::vector<SentenceTransformationQuizWidget::QuestionSeed>
                SentenceTransformationQuizWidget::buildQuestionSeeds(bool transformationMode) const
            {
                std::vector<QuestionSeed> seeds;
                const std::vector<GenerationRule> rules = transformationMode ?
                    buildEnabledRules() :
                    buildEnabledMeaningToJapaneseRules();

                for( const SentencePatternData& pattern : m_sentencePatterns )
                {
                    const PatternAnalysis analysis = analyzePattern(pattern);

                    if( isEnabledForThisQuiz(pattern) && isUsableVerbPattern(analysis) )
                    {
                        for( const GenerationRule& rule : rules )
                        {
                            const std::vector<SentenceBinding> bindings = buildBindings(analysis, rule);

                            for( const SentenceBinding& binding : bindings )
                            {
                                const auto dynamicWordIterator = binding.words.find(analysis.dynamicSlot);

                                if( dynamicWordIterator != binding.words.end() )
                                {
                                    const DictionaryWord& dynamicWord = *dynamicWordIterator->second;
                                    const std::string targetFormName =
                                        getFiniteFormName(dynamicWord, rule.category, rule.targetPolite);
                                    const std::string sourceFormName = rule.hasSourceForm ?
                                        getFiniteFormName(dynamicWord, rule.category, rule.sourcePolite) :
                                        targetFormName;
                                    const FormData* targetForm =
                                        getForm(dynamicWord, "form", targetFormName);
                                    const FormData* sourceForm =
                                        getForm(dynamicWord, "form", sourceFormName);

                                    if( (targetForm != nullptr) && (sourceForm != nullptr) )
                                    {
                                        const std::string targetContext =
                                            targetForm->context.empty() ? "none" : targetForm->context;
                                        const std::string sourceContext =
                                            sourceForm->context.empty() ? "none" : sourceForm->context;
                                        const ContextPoolData* contextPool = getContextPool(targetContext);

                                        if( (contextPool != nullptr) && (sourceContext == targetContext) )
                                        {
                                            for( const ContextOptionData& contextOption : contextPool->options )
                                            {
                                                QuestionSeed seed;
                                                seed.pattern = &pattern;
                                                seed.analysis = analysis;
                                                seed.rule = rule;
                                                seed.binding = binding;
                                                seed.contextPool = contextPool;
                                                seed.contextOption = &contextOption;
                                                seed.sourceForm = sourceFormName;
                                                seed.targetForm = targetFormName;
                                                seed.selectionWeight =
                                                    std::max(1U, contextOption.weight);
                                                seeds.push_back(std::move(seed));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if( !analysis.valid )
                    {
                        m_logger.log(
                            "Sentence pattern '" + pattern.id + "' is invalid: " + analysis.error,
                            tools::LogLevel::PROBLEM);
                    }
                }

                return seeds;
            }

            bool SentenceTransformationQuizWidget::renderQuestionSeed(
                const QuestionSeed& seed,
                bool transformationMode,
                QuizQuestion& question) const
            {
                bool result = false;

                if( (seed.pattern != nullptr) &&
                    (seed.contextPool != nullptr) &&
                    (seed.contextOption != nullptr) )
                {
                    std::string targetQuestion;
                    std::string targetAnswer;
                    std::string sourceAnswer;
                    std::string error;

                    const bool targetQuestionRendered = renderTemplate(
                        seed.pattern->questionTemplate,
                        seed.analysis,
                        seed.binding,
                        seed.targetForm,
                        *seed.contextPool,
                        *seed.contextOption,
                        targetQuestion,
                        error);
                    const bool targetAnswerRendered = targetQuestionRendered &&
                        renderTemplate(
                            seed.pattern->answerTemplate,
                            seed.analysis,
                            seed.binding,
                            seed.targetForm,
                            *seed.contextPool,
                            *seed.contextOption,
                            targetAnswer,
                            error);
                    bool sourceAnswerRendered = targetAnswerRendered;

                    if( transformationMode && sourceAnswerRendered )
                    {
                        sourceAnswerRendered = renderTemplate(
                            seed.pattern->answerTemplate,
                            seed.analysis,
                            seed.binding,
                            seed.sourceForm,
                            *seed.contextPool,
                            *seed.contextOption,
                            sourceAnswer,
                            error);
                    }

                    if( targetAnswerRendered && sourceAnswerRendered )
                    {
                        const auto dynamicWordIterator =
                            seed.binding.words.find(seed.analysis.dynamicSlot);

                        if( dynamicWordIterator != seed.binding.words.end() )
                        {
                            const DictionaryWord& dynamicWord =
                                *dynamicWordIterator->second;
                            const bool distinctTransformation =
                                !transformationMode ||
                                (normalizeAnswer(sourceAnswer) != normalizeAnswer(targetAnswer));

                            if( !targetAnswer.empty() &&
                                !(transformationMode ? sourceAnswer : targetQuestion).empty() &&
                                distinctTransformation )
                            {
                                question.key = buildQuestionKey(
                                    *seed.pattern,
                                    seed.analysis,
                                    seed.binding,
                                    *seed.contextOption,
                                    seed.sourceForm,
                                    seed.targetForm);
                                question.sourceSentence = transformationMode ?
                                    sourceAnswer :
                                    targetQuestion;
                                question.targetSentence = targetAnswer;
                                question.questionText = transformationMode ?
                                    targetQuestion :
                                    std::string();
                                question.targetLabel = seed.rule.label;
                                question.sourceConjugation = transformationMode ?
                                    seed.sourceForm :
                                    "source_language";
                                question.targetConjugation = seed.targetForm;
                                question.wordMeaning = dynamicWord.translation;
                                question.wordKana = dynamicWord.kana;
                                question.wordKanji = dynamicWord.kanji;
                                question.patternId = seed.pattern->id;
                                question.bindings =
                                    buildBindingDescription(seed.analysis, seed.binding);
                                question.selectionWeight = seed.selectionWeight;
                                result = true;
                            }
                        }
                    }
                    else if( !error.empty() )
                    {
                        m_logger.log(
                            "Sentence pattern '" +
                            seed.pattern->id +
                            "' could not be rendered: " +
                            error,
                            tools::LogLevel::PROBLEM);
                    }
                }

                return result;
            }

            uint64_t SentenceTransformationQuizWidget::countPossibleQuestions(
                bool transformationMode) const
            {
                const uint64_t result =
                    static_cast<uint64_t>(buildQuestionSeeds(transformationMode).size());

                return result;
            }

            void SentenceTransformationQuizWidget::applyWeightedRandomOrder(
                std::vector<QuestionSeed>& seeds) const
            {
                struct WeightedSeed
                {
                    double priority = 0.0;
                    QuestionSeed seed;
                };

                std::vector<WeightedSeed> weighted;
                weighted.reserve(seeds.size());
                std::uniform_real_distribution<double> distribution(
                    std::numeric_limits<double>::min(),
                    1.0);

                for( QuestionSeed& seed : seeds )
                {
                    const double randomValue = distribution(m_rng);
                    const double weight =
                        static_cast<double>(std::max(1U, seed.selectionWeight));
                    WeightedSeed value;
                    value.priority = -std::log(randomValue) / weight;
                    value.seed = std::move(seed);
                    weighted.push_back(std::move(value));
                }

                std::sort(
                    weighted.begin(),
                    weighted.end(),
                    [](const WeightedSeed& first, const WeightedSeed& second)
                    {
                        return first.priority < second.priority;
                    });

                seeds.clear();
                seeds.reserve(weighted.size());

                for( WeightedSeed& value : weighted )
                {
                    seeds.push_back(std::move(value.seed));
                }
            }

            bool SentenceTransformationQuizWidget::renderTemplate(
                const std::string& text,
                const PatternAnalysis& analysis,
                const SentenceBinding& binding,
                const std::string& dynamicForm,
                const ContextPoolData& contextPool,
                const ContextOptionData& contextOption,
                std::string& rendered,
                std::string& error) const
            {
                bool result = true;
                rendered.clear();
                size_t cursor = 0U;

                while( result && (cursor < text.size()) )
                {
                    const size_t opening = text.find('{', cursor);

                    if( opening == std::string::npos )
                    {
                        rendered.append(text.substr(cursor));
                        cursor = text.size();
                    }
                    else
                    {
                        rendered.append(text.substr(cursor, opening - cursor));
                        const size_t closing = text.find('}', opening + 1U);

                        if( closing == std::string::npos )
                        {
                            result = false;
                            error = "Unclosed token in template.";
                        }
                        else
                        {
                            const std::string expression =
                                trimText(text.substr(opening + 1U, closing - opening - 1U));
                            std::string value;
                            result = resolveToken(
                                expression,
                                analysis,
                                binding,
                                dynamicForm,
                                contextPool,
                                contextOption,
                                value,
                                error);

                            if( result )
                            {
                                rendered.append(value);
                                cursor = closing + 1U;
                            }
                        }
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

            bool SentenceTransformationQuizWidget::resolveToken(
                const std::string& expression,
                const PatternAnalysis& analysis,
                const SentenceBinding& binding,
                const std::string& dynamicForm,
                const ContextPoolData& contextPool,
                const ContextOptionData& contextOption,
                std::string& value,
                std::string& error) const
            {
                bool result = false;
                std::string root;
                std::string field;
                splitToken(expression, root, field);

                if( root == "context" )
                {
                    if( field == "id" )
                    {
                        value = contextPool.id;
                        result = true;
                    }
                    else if( field == "option" )
                    {
                        value = contextOption.id;
                        result = true;
                    }
                    else if( field == "pl_prefix" )
                    {
                        value = contextOption.polishPrefix;
                        result = true;
                    }
                    else if( field == "jp_prefix" )
                    {
                        value = contextOption.japanesePrefix;
                        result = true;
                    }
                    else
                    {
                        error = "Unknown context field '" + field + "'.";
                    }
                }
                else if( getSlotKind(root) == SlotKind::Entity )
                {
                    result = resolveEntityToken(root, field, binding, value, error);
                }
                else
                {
                    result = resolveWordToken(
                        root,
                        field,
                        analysis,
                        binding,
                        dynamicForm,
                        value,
                        error);
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::resolveWordToken(
                const std::string& slotId,
                const std::string& field,
                const PatternAnalysis& analysis,
                const SentenceBinding& binding,
                const std::string& dynamicForm,
                std::string& value,
                std::string& error) const
            {
                bool result = false;
                const auto wordIterator = binding.words.find(slotId);

                if( wordIterator == binding.words.end() )
                {
                    error = "No word selected for slot '" + slotId + "'.";
                }
                else
                {
                    const DictionaryWord& word = *wordIterator->second;
                    std::string resolvedField = field;

                    if( resolvedField.empty() )
                    {
                        resolvedField = getDefaultFormSpec(slotId);
                    }

                    if( isFormSelector(slotId, resolvedField) )
                    {
                        const FormData* form = getForm(word, resolvedField, dynamicForm);

                        if( form != nullptr )
                        {
                            value = form->value;
                            result = true;
                        }
                        else
                        {
                            error = "Word '" +
                                word.id +
                                "' has no form required by token '{" +
                                slotId +
                                "." +
                                resolvedField +
                                "}'.";
                        }
                    }
                    else
                    {
                        const auto primaryIterator = analysis.primarySpecs.find(slotId);
                        const std::string primarySpec = (primaryIterator != analysis.primarySpecs.end()) ?
                            primaryIterator->second :
                            getDefaultFormSpec(slotId);
                        const FormData* primaryForm = getForm(word, primarySpec, dynamicForm);
                        std::string gender;
                        const auto entitySlotIterator = analysis.entityForAdjective.find(slotId);

                        if( entitySlotIterator != analysis.entityForAdjective.end() )
                        {
                            const auto entityIterator = binding.entities.find(entitySlotIterator->second);

                            if( entityIterator != binding.entities.end() )
                            {
                                gender = entityIterator->second->gender;
                            }
                        }

                        if( resolvedField == "id" )
                        {
                            value = word.id;
                            result = true;
                        }
                        else if( resolvedField == "kana" )
                        {
                            value = word.kana;
                            result = true;
                        }
                        else if( resolvedField == "kanji" )
                        {
                            value = word.kanji;
                            result = true;
                        }
                        else if( resolvedField == "translation" )
                        {
                            value = word.translation;
                            result = true;
                        }
                        else if( resolvedField == "type" )
                        {
                            value = word.type;
                            result = true;
                        }
                        else if( resolvedField == "ending" )
                        {
                            value = word.ending;
                            result = true;
                        }
                        else if( (resolvedField == "categories") || (resolvedField == "tags") )
                        {
                            value = joinText(word.tags, ", ");
                            result = true;
                        }
                        else if( (resolvedField == "prompt") && (primaryForm != nullptr) )
                        {
                            value = getFormPrompt(*primaryForm, gender);
                            result = true;
                        }
                        else if( (resolvedField == "style") && (primaryForm != nullptr) )
                        {
                            value = primaryForm->style;
                            result = true;
                        }
                        else if( (resolvedField == "form_name") && (primaryForm != nullptr) )
                        {
                            value = primaryForm->name;
                            result = true;
                        }
                        else
                        {
                            error = "Unknown or unavailable word field '" +
                                resolvedField +
                                "' for slot '" +
                                slotId +
                                "'.";
                        }
                    }
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::resolveEntityToken(
                const std::string& slotId,
                const std::string& field,
                const SentenceBinding& binding,
                std::string& value,
                std::string& error) const
            {
                bool result = false;
                const auto entityIterator = binding.entities.find(slotId);

                if( entityIterator == binding.entities.end() )
                {
                    error = "No entity selected for slot '" + slotId + "'.";
                }
                else
                {
                    const EntityData& entity = *entityIterator->second;

                    if( field.empty() || (field == "kana") )
                    {
                        value = entity.kana;
                        result = true;
                    }
                    else if( field == "id" )
                    {
                        value = entity.id;
                        result = true;
                    }
                    else if( field == "kanji" )
                    {
                        value = entity.kanji;
                        result = true;
                    }
                    else if( field == "jp_demonstrative" )
                    {
                        value = "この" + entity.kana;
                        result = true;
                    }
                    else if( field == "gender" )
                    {
                        value = entity.gender;
                        result = true;
                    }
                    else if( field == "profiles" )
                    {
                        value = joinText(entity.profiles, ", ");
                        result = true;
                    }
                    else if( (field == "categories") || (field == "tags") )
                    {
                        const auto tagsIterator = m_effectiveEntityTags.find(entity.id);

                        if( tagsIterator != m_effectiveEntityTags.end() )
                        {
                            value = joinText(tagsIterator->second, ", ");
                            result = true;
                        }
                    }
                    else if( field.rfind("pl_", 0U) == 0U )
                    {
                        const std::string polishField = field.substr(3U);
                        const auto polishIterator = entity.polish.find(polishField);

                        if( polishIterator != entity.polish.end() )
                        {
                            value = polishIterator->second;
                            result = true;
                        }
                    }

                    if( !result )
                    {
                        error = "Unknown or unavailable entity field '" +
                            field +
                            "' for slot '" +
                            slotId +
                            "'.";
                    }
                }

                return result;
            }

            void SentenceTransformationQuizWidget::startQuiz()
            {
                const bool loaded = loadFiles();

                if( loaded )
                {
                    const size_t requestedSize = (m_numberOfQuestions > 0) ?
                        static_cast<size_t>(m_numberOfQuestions) :
                        0U;

                    if( m_quizMode == QuizMode::Transformation )
                    {
                        m_questions = buildQuestionBank(requestedSize);
                    }
                    else
                    {
                        m_questions = buildMeaningToJapaneseQuestionBank(requestedSize);
                    }

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
                            "No questions could be generated for the current project, filters, and form settings.";
                    }
                }
            }

            void SentenceTransformationQuizWidget::submitAnswer()
            {
                if( !m_waitingForNextQuestion && (m_currentQuestionIndex < m_questions.size()) )
                {
                    const QuizQuestion& question = m_questions[m_currentQuestionIndex];
                    const std::string userAnswer = normalizeAnswer(std::string(m_userAnswer));
                    const std::string expectedAnswer = normalizeAnswer(question.targetSentence);

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

            void SentenceTransformationQuizWidget::scheduleAutoAdvanceAfterCorrectAnswer()
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

            void SentenceTransformationQuizWidget::processAutoAdvance()
            {
                if( m_waitingForNextQuestion && (m_autoAdvanceDueTime > 0.0) )
                {
                    if( ImGui::GetTime() >= m_autoAdvanceDueTime )
                    {
                        m_autoAdvanceDueTime = 0.0;
                        advanceQuestion();
                    }
                }
            }

            void SentenceTransformationQuizWidget::acceptCurrentAnswerAsCorrect()
            {
                if( m_waitingForNextQuestion && (m_currentQuestionIndex < m_questions.size()) )
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

            void SentenceTransformationQuizWidget::skipQuestion()
            {
                if( m_currentQuestionIndex < m_questions.size() )
                {
                    ++m_correctAnswers;
                    ++m_acceptedAnswers;
                    advanceQuestion();
                }
            }

            void SentenceTransformationQuizWidget::advanceQuestion()
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

            void SentenceTransformationQuizWidget::resetQuizState()
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

            void SentenceTransformationQuizWidget::updateStatistics()
            {
                const bool loaded = loadFiles();

                if( loaded )
                {
                    uint32_t wordCount = 0U;

                    for( const auto& dictionaryEntry : m_dictionaries )
                    {
                        wordCount += static_cast<uint32_t>(dictionaryEntry.second.size());
                    }

                    m_statistics.words = wordCount;
                    uint32_t enabledPatterns = 0U;

                    for( const SentencePatternData& pattern : m_sentencePatterns )
                    {
                        const PatternAnalysis analysis = analyzePattern(pattern);

                        if( isEnabledForThisQuiz(pattern) && isUsableVerbPattern(analysis) )
                        {
                            ++enabledPatterns;
                        }
                    }

                    m_statistics.sentencePatterns = enabledPatterns;
                    m_statistics.entities =
                        static_cast<uint32_t>(m_entities.size());
                    m_statistics.tagProfiles =
                        static_cast<uint32_t>(m_tagProfiles.size());

                    if( m_quizMode == QuizMode::Transformation )
                    {
                        m_statistics.generatedQuestions =
                            static_cast<uint32_t>(countPossibleQuestions(true));
                    }
                    else
                    {
                        m_statistics.generatedQuestions =
                            static_cast<uint32_t>(countPossibleQuestions(false));
                    }

                    m_statusMessage = "Statistics refreshed from quiz_project.xml.";
                }
            }

            std::vector<SentenceTransformationQuizWidget::WordFilterRule>
                SentenceTransformationQuizWidget::parseWordFilter(const std::string& filterText) const
            {
                std::vector<WordFilterRule> rules;
                const std::string trimmedFilter = trimText(filterText);

                if( !trimmedFilter.empty() )
                {
                    const std::vector<std::string> expressions = splitText(trimmedFilter, ';');

                    for( const std::string& expression : expressions )
                    {
                        const std::string trimmedExpression = trimText(expression);
                        const size_t equalPosition = trimmedExpression.find('=');

                        if( equalPosition != std::string::npos )
                        {
                            WordFilterRule rule;
                            rule.field = trimText(trimmedExpression.substr(0U, equalPosition));

                            const std::string valuesText =
                                trimText(trimmedExpression.substr(equalPosition + 1U));
                            const std::vector<std::string> values = splitText(valuesText, ',');

                            for( const std::string& value : values )
                            {
                                const std::string cleanedValue =
                                    removeOptionalQuotes(trimText(value));

                                if( !cleanedValue.empty() )
                                {
                                    rule.values.push_back(cleanedValue);
                                }
                            }

                            if( !rule.field.empty() && !rule.values.empty() )
                            {
                                rules.push_back(rule);
                            }
                        }
                    }
                }

                return rules;
            }

            bool SentenceTransformationQuizWidget::wordMatchesFilter(
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

                    if( !ruleMatches )
                    {
                        result = false;
                    }
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::wordFieldMatches(
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
                    const auto attributeIterator = word.attributes.find(field);

                    if( attributeIterator != word.attributes.end() )
                    {
                        result = (attributeIterator->second == value);
                    }
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::wordHasTag(
                const DictionaryWord& word,
                const std::string& tag) const
            {
                return containsValue(word.tags, tag);
            }

            bool SentenceTransformationQuizWidget::wordCanUseEntity(
                const DictionaryWord& word,
                const std::string& role,
                const EntityData& entity) const
            {
                bool result = false;
                const auto relationIterator = word.relations.find(role);
                const auto entityTagsIterator = m_effectiveEntityTags.find(entity.id);

                if( (relationIterator != word.relations.end()) &&
                    (entityTagsIterator != m_effectiveEntityTags.end()) )
                {
                    result = tagsIntersect(relationIterator->second, entityTagsIterator->second);
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::entityMatchesAdjective(
                const EntityData& entity,
                const DictionaryWord& adjective) const
            {
                bool result = false;
                const auto tagsIterator = m_effectiveEntityTags.find(entity.id);

                if( tagsIterator != m_effectiveEntityTags.end() )
                {
                    result = tagsIntersect(tagsIterator->second, adjective.tags);
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::tagsIntersect(
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

            bool SentenceTransformationQuizWidget::containsValue(
                const std::vector<std::string>& values,
                const std::string& value) const
            {
                const bool result =
                    (std::find(values.begin(), values.end(), value) != values.end());

                return result;
            }

            void SentenceTransformationQuizWidget::appendUnique(
                std::vector<std::string>& values,
                const std::string& value) const
            {
                if( !containsValue(values, value) )
                {
                    values.push_back(value);
                }
            }

            std::vector<std::string> SentenceTransformationQuizWidget::extractTokens(
                const std::string& text) const
            {
                std::vector<std::string> tokens;
                size_t cursor = 0U;

                while( cursor < text.size() )
                {
                    const size_t opening = text.find('{', cursor);

                    if( opening == std::string::npos )
                    {
                        cursor = text.size();
                    }
                    else
                    {
                        const size_t closing = text.find('}', opening + 1U);

                        if( closing == std::string::npos )
                        {
                            cursor = text.size();
                        }
                        else
                        {
                            const std::string expression =
                                trimText(text.substr(opening + 1U, closing - opening - 1U));

                            if( !expression.empty() )
                            {
                                appendUnique(tokens, expression);
                            }

                            cursor = closing + 1U;
                        }
                    }
                }

                return tokens;
            }

            void SentenceTransformationQuizWidget::splitToken(
                const std::string& expression,
                std::string& root,
                std::string& field) const
            {
                const size_t dotPosition = expression.find('.');
                root = trimText(expression);
                field.clear();

                if( dotPosition != std::string::npos )
                {
                    root = trimText(expression.substr(0U, dotPosition));
                    field = trimText(expression.substr(dotPosition + 1U));
                }
            }

            std::vector<std::string> SentenceTransformationQuizWidget::splitText(
                const std::string& text,
                char separator) const
            {
                std::vector<std::string> parts;
                std::string currentPart;

                for( const char character : text )
                {
                    if( character == separator )
                    {
                        parts.push_back(currentPart);
                        currentPart.clear();
                    }
                    else
                    {
                        currentPart.push_back(character);
                    }
                }

                parts.push_back(currentPart);
                return parts;
            }

            std::string SentenceTransformationQuizWidget::trimText(const std::string& text) const
            {
                std::string result = text;

                while( !result.empty() &&
                    ((result.front() == ' ') ||
                        (result.front() == '\t') ||
                        (result.front() == '\r') ||
                        (result.front() == '\n')) )
                {
                    result.erase(result.begin());
                }

                while( !result.empty() &&
                    ((result.back() == ' ') ||
                        (result.back() == '\t') ||
                        (result.back() == '\r') ||
                        (result.back() == '\n')) )
                {
                    result.pop_back();
                }

                return result;
            }

            std::string SentenceTransformationQuizWidget::removeOptionalQuotes(
                const std::string& text) const
            {
                std::string result = text;

                if( result.size() >= 2U )
                {
                    const bool doubleQuotes =
                        (result.front() == '"') && (result.back() == '"');
                    const bool singleQuotes =
                        (result.front() == '\'') && (result.back() == '\'');

                    if( doubleQuotes || singleQuotes )
                    {
                        result = result.substr(1U, result.size() - 2U);
                    }
                }

                return result;
            }

            std::string SentenceTransformationQuizWidget::joinText(
                const std::vector<std::string>& values,
                const std::string& separator) const
            {
                std::string result;

                for( size_t index = 0U; index < values.size(); ++index )
                {
                    if( index > 0U )
                    {
                        result.append(separator);
                    }

                    result.append(values[index]);
                }

                return result;
            }

            std::string SentenceTransformationQuizWidget::replaceAll(
                std::string text,
                const std::string& token,
                const std::string& value) const
            {
                size_t position = 0U;

                while( (position = text.find(token, position)) != std::string::npos )
                {
                    text.replace(position, token.length(), value);
                    position += value.length();
                }

                return text;
            }

            std::string SentenceTransformationQuizWidget::normalizeAnswer(std::string text) const
            {
                text = replaceAll(text, " ", "");
                text = replaceAll(text, "\t", "");
                text = replaceAll(text, "\r", "");
                text = replaceAll(text, "\n", "");
                text = replaceAll(text, ".", "");
                text = replaceAll(text, ",", "");
                text = replaceAll(text, "?", "");
                text = replaceAll(text, "!", "");
                text = replaceAll(text, "。", "");
                text = replaceAll(text, "、", "");
                text = replaceAll(text, "？", "");
                text = replaceAll(text, "！", "");

                return text;
            }

            std::string SentenceTransformationQuizWidget::buildConfiguredPath(
                const std::string& path) const
            {
                std::filesystem::path configuredPath(path);
                std::filesystem::path result;

                if( configuredPath.is_absolute() )
                {
                    result = configuredPath;
                }
                else
                {
                    const std::filesystem::path executablePath(getexepath());
                    result = executablePath / configuredPath;
                }

                return result.lexically_normal().string();
            }

            std::string SentenceTransformationQuizWidget::resolveProjectFile(
                const std::string& projectDirectory,
                const std::string& path) const
            {
                std::filesystem::path referencedPath(path);
                std::filesystem::path result;

                if( referencedPath.is_absolute() )
                {
                    result = referencedPath;
                }
                else
                {
                    result = std::filesystem::path(projectDirectory) / referencedPath;
                }

                return result.lexically_normal().string();
            }

            std::string SentenceTransformationQuizWidget::buildBindingDescription(
                const PatternAnalysis& analysis,
                const SentenceBinding& binding) const
            {
                std::vector<std::string> values;

                for( const std::string& slotId : analysis.wordSlots )
                {
                    const auto iterator = binding.words.find(slotId);

                    if( iterator != binding.words.end() )
                    {
                        values.push_back(slotId + "=" + iterator->second->id);
                    }
                }

                for( const std::string& slotId : analysis.entitySlots )
                {
                    const auto iterator = binding.entities.find(slotId);

                    if( iterator != binding.entities.end() )
                    {
                        values.push_back(slotId + "=" + iterator->second->id);
                    }
                }

                return joinText(values, ", ");
            }

            std::string SentenceTransformationQuizWidget::buildQuestionKey(
                const SentencePatternData& pattern,
                const PatternAnalysis& analysis,
                const SentenceBinding& binding,
                const ContextOptionData& contextOption,
                const std::string& sourceForm,
                const std::string& targetForm) const
            {
                std::string result = pattern.id;
                result.append(":");
                result.append(sourceForm);
                result.append(":");
                result.append(targetForm);
                result.append(":");
                result.append(contextOption.id);

                for( const std::string& slotId : analysis.wordSlots )
                {
                    const auto iterator = binding.words.find(slotId);

                    if( iterator != binding.words.end() )
                    {
                        result.append(":");
                        result.append(slotId);
                        result.append("=");
                        result.append(iterator->second->id);
                    }
                }

                for( const std::string& slotId : analysis.entitySlots )
                {
                    const auto iterator = binding.entities.find(slotId);

                    if( iterator != binding.entities.end() )
                    {
                        result.append(":");
                        result.append(slotId);
                        result.append("=");
                        result.append(iterator->second->id);
                    }
                }

                return result;
            }

            void SentenceTransformationQuizWidget::copyToBuffer(
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
