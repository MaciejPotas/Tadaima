#include "SentenceTransformationQuizWidget.h"

#include "tools/pugixml.hpp"
#include "tools/SystemTools.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <format>
#include <stdexcept>

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
                        const std::string dictionaryPath = package->get<std::string>(SettingsPackageKey::SentenceQuizDictionaryPath);
                        const std::string sentenceMapPath = package->get<std::string>(SettingsPackageKey::SentenceQuizSentenceMapPath);
                        const std::string numberOfTries = package->get<std::string>(SettingsPackageKey::TriesForQuiz);

                        copyToBuffer(m_dictionaryPath, sizeof(m_dictionaryPath), dictionaryPath);
                        copyToBuffer(m_sentenceMapPath, sizeof(m_sentenceMapPath), sentenceMapPath);

                        m_numberOfTries = std::stoi(numberOfTries);

                        m_logger.log("SentenceTransformationQuizWidget: Initialized.", tools::LogLevel::INFO);
                    }
                }
                catch( const std::exception& exception )
                {
                    m_logger.log("SentenceTransformationQuizWidget::initialize exception: " + std::string(exception.what()), tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log("SentenceTransformationQuizWidget::initialize unknown exception.", tools::LogLevel::PROBLEM);
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
                    m_logger.log("SentenceTransformationQuizWidget::draw exception: " + std::string(exception.what()), tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log("SentenceTransformationQuizWidget::draw unknown exception.", tools::LogLevel::PROBLEM);
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
                    ImGui::TextWrapped("This quiz creates Japanese sentence questions from a dictionary XML file and a sentence map XML file.");
                    ImGui::Separator();

                    ImGui::InputText("Word filter", m_wordFilter, IM_ARRAYSIZE(m_wordFilter));
                    ImGui::TextDisabled("Examples: ending=su | ending=su,ku,kuru | type=godan;ending=su,ku | tag=eat_food");

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

                        ImGui::TextDisabled("This mode shows the source sentence and expects a Japanese sentence.");
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

                    ImGui::TextDisabled("Press F1 in this window to show or hide statistics from the selected files.");

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
                        ImGui::Text("Sentence maps: %u", m_statistics.sentenceMaps);
                        ImGui::Text("Slot lists: %u", m_statistics.slotLists);
                        ImGui::Text("Slot items: %u", m_statistics.slotItems);
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
                ImGui::SetNextWindowSize(ImVec2(660, 480), ImGuiCond_FirstUseEver);
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
                        const float progress = static_cast<float>(m_currentQuestionIndex) / static_cast<float>(m_questions.size());

                        ImGui::Text("Question %u / %zu", m_currentQuestionIndex + 1U, m_questions.size());
                        ImGui::ProgressBar(progress, ImVec2(-1, 0));
                        ImGui::Text("Correct: %u    Wrong: %u    Accepted: %u", m_correctAnswers, m_wrongAnswers, m_acceptedAnswers);

                        ImGui::Spacing();
                        ImGui::Separator();

                        if( m_quizMode == QuizMode::Transformation )
                        {
                            ImGui::Text("Change the sentence to:");
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "%s", question.targetLabel.c_str());
                            ImGui::Spacing();
                            ImGui::Text("Source sentence:");
                        }
                        else
                        {
                            ImGui::Text("Write the Japanese sentence:");
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "%s", question.targetLabel.c_str());
                            ImGui::Spacing();
                            ImGui::Text("Source sentence:");
                        }

                        ImGui::PushTextWrapPos(0.0f);
                        ImGui::TextColored(ImVec4(0.15f, 0.15f, 0.15f, 1.0f), "%s", question.sourceSentence.c_str());
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

                        const bool submittedByEnter = ImGui::InputText("Your answer", m_userAnswer, IM_ARRAYSIZE(m_userAnswer), ImGuiInputTextFlags_EnterReturnsTrue);

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

                            if( m_quizMode == QuizMode::Transformation )
                            {
                                ImGui::TextWrapped("Expected sentence: %s", question.targetSentence.c_str());
                            }
                            else
                            {
                                ImGui::TextWrapped("Expected Japanese sentence: %s", question.targetSentence.c_str());
                            }

                            ImGui::TextDisabled("Verb meaning: %s", question.wordMeaning.c_str());
                            ImGui::TextDisabled("Verb kana: %s", question.wordKana.c_str());
                            ImGui::TextDisabled("Verb kanji: %s", question.wordKanji.c_str());
                            ImGui::TextDisabled("Map: %s", question.mapId.c_str());
                            ImGui::TextDisabled("Target form: %s", question.targetConjugation.c_str());
                        }

                        ImGui::TextDisabled("Press F1 in this window to show or hide the full expected answer.");
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
                    m_words.clear();
                    m_sentenceMaps.clear();
                    m_slotLists.clear();

                    const bool dictionaryLoaded = loadDictionary(std::string(m_dictionaryPath));
                    const bool sentenceMapsLoaded = loadSentenceMaps(std::string(m_sentenceMapPath));

                    if( dictionaryLoaded && sentenceMapsLoaded )
                    {
                        m_filesLoaded = true;
                        result = true;
                        m_statusMessage = "Files loaded correctly.";
                    }
                    else
                    {
                        m_statusMessage = "Could not load dictionary or sentence map file.";
                    }
                }
                catch( const std::exception& exception )
                {
                    m_logger.log("SentenceTransformationQuizWidget::loadFiles exception: " + std::string(exception.what()), tools::LogLevel::PROBLEM);
                    m_statusMessage = exception.what();
                }
                catch( ... )
                {
                    m_logger.log("SentenceTransformationQuizWidget::loadFiles unknown exception.", tools::LogLevel::PROBLEM);
                    m_statusMessage = "Unknown error while loading files.";
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::loadDictionary(const std::string& path)
            {
                bool result = false;
                const std::string fullPath = buildFullPath(path);

                pugi::xml_document document;
                const pugi::xml_parse_result parseResult = document.load_file(fullPath.c_str());

                if( parseResult )
                {
                    const pugi::xml_node rootNode = document.child("dictionary");
                    const pugi::xml_node wordsNode = rootNode.child("words");

                    for( pugi::xml_node wordNode = wordsNode.child("word"); wordNode; wordNode = wordNode.next_sibling("word") )
                    {
                        DictionaryWord word;

                        for( pugi::xml_attribute attribute = wordNode.first_attribute(); attribute; attribute = attribute.next_attribute() )
                        {
                            word.attributes[attribute.name()] = attribute.value();
                        }

                        word.translation = GetAttribute(wordNode, "translation");
                        word.kana = GetAttribute(wordNode, "kana");
                        word.kanji = GetAttribute(wordNode, "kanji");
                        word.type = GetAttribute(wordNode, "type");
                        word.ending = GetAttribute(wordNode, "ending");

                        const pugi::xml_node tagsNode = wordNode.child("tags");

                        for( pugi::xml_node tagNode = tagsNode.child("tag"); tagNode; tagNode = tagNode.next_sibling("tag") )
                        {
                            const std::string tag = GetText(tagNode);

                            if( !tag.empty() )
                            {
                                word.tags.push_back(tag);
                            }
                        }

                        const pugi::xml_node conjugationsNode = wordNode.child("conjugations");

                        for( pugi::xml_node conjugationNode = conjugationsNode.child("conjugation"); conjugationNode; conjugationNode = conjugationNode.next_sibling("conjugation") )
                        {
                            ConjugationData conjugation;
                            conjugation.name = GetAttribute(conjugationNode, "name");
                            conjugation.style = GetAttribute(conjugationNode, "style");
                            conjugation.sentencePrompt = GetAttribute(conjugationNode, "sentence_prompt");
                            conjugation.value = GetText(conjugationNode);

                            if( !conjugation.name.empty() && !conjugation.value.empty() )
                            {
                                word.conjugations[conjugation.name] = conjugation;
                            }
                        }

                        if( !word.kana.empty() && !word.conjugations.empty() )
                        {
                            m_words.push_back(word);
                        }
                    }

                    result = !m_words.empty();
                }

                if( !result )
                {
                    m_logger.log("Could not load sentence quiz dictionary XML from path: " + fullPath + ". Parser status: " + parseResult.description(), tools::LogLevel::PROBLEM);
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::loadSentenceMaps(const std::string& path)
            {
                bool result = false;
                const std::string fullPath = buildFullPath(path);

                pugi::xml_document document;
                const pugi::xml_parse_result parseResult = document.load_file(fullPath.c_str());

                if( parseResult )
                {
                    const pugi::xml_node rootNode = document.child("sentence_maps_data");
                    const pugi::xml_node slotListsNode = rootNode.child("slot_lists");

                    for( pugi::xml_node slotListNode = slotListsNode.child("slot_list"); slotListNode; slotListNode = slotListNode.next_sibling("slot_list") )
                    {
                        const std::string slotListId = GetAttribute(slotListNode, "id");
                        std::vector<SlotItem> slots;

                        for( pugi::xml_node itemNode = slotListNode.child("item"); itemNode; itemNode = itemNode.next_sibling("item") )
                        {
                            SlotItem slot;
                            slot.text = GetAttribute(itemNode, "text");
                            slot.jp = GetAttribute(itemNode, "jp");

                            if( !slot.jp.empty() )
                            {
                                slots.push_back(slot);
                            }
                        }

                        if( !slotListId.empty() )
                        {
                            m_slotLists[slotListId] = slots;
                        }
                    }

                    const pugi::xml_node sentenceMapsNode = rootNode.child("sentence_maps");

                    for( pugi::xml_node mapNode = sentenceMapsNode.child("sentence_map"); mapNode; mapNode = mapNode.next_sibling("sentence_map") )
                    {
                        SentenceMap sentenceMap;
                        sentenceMap.id = GetAttribute(mapNode, "id");
                        sentenceMap.label = GetAttribute(mapNode, "label");
                        sentenceMap.requiredTag = GetAttribute(mapNode, "requires_tag");
                        sentenceMap.conjugation = GetAttribute(mapNode, "conjugation");
                        sentenceMap.slotList = GetAttribute(mapNode, "slot_list");
                        sentenceMap.questionTemplate = GetText(mapNode.child("question"));
                        sentenceMap.answerTemplate = GetText(mapNode.child("answer"));

                        if( !sentenceMap.id.empty() &&
                            !sentenceMap.requiredTag.empty() &&
                            !sentenceMap.conjugation.empty() &&
                            !sentenceMap.answerTemplate.empty() )
                        {
                            m_sentenceMaps.push_back(sentenceMap);
                        }
                    }

                    result = !m_sentenceMaps.empty();
                }

                if( !result )
                {
                    m_logger.log("Could not load sentence maps XML from path: " + fullPath + ". Parser status: " + parseResult.description(), tools::LogLevel::PROBLEM);
                }

                return result;
            }

            std::vector<SentenceTransformationQuizWidget::TransformationRule> SentenceTransformationQuizWidget::buildEnabledRules() const
            {
                std::vector<TransformationRule> rules;

                if( m_enablePoliteToPlain )
                {
                    if( m_enableNonPast )
                    {
                        rules.push_back({ "polite_nonpast", "dictionary", "dictionary", "plain non-past affirmative", true });
                    }

                    if( m_enableNegative )
                    {
                        rules.push_back({ "polite_negative", "plain_negative", "plain_negative", "plain non-past negative", true });
                    }

                    if( m_enablePast )
                    {
                        rules.push_back({ "past_polite", "past_plain", "past_plain", "plain past affirmative", true });
                    }

                    if( m_enablePastNegative )
                    {
                        rules.push_back({ "past_negative_polite", "past_negative_plain", "past_plain", "plain past negative", true });
                    }
                }

                if( m_enablePlainToPolite )
                {
                    if( m_enableNonPast )
                    {
                        rules.push_back({ "dictionary", "polite_nonpast", "dictionary", "polite non-past affirmative", true });
                    }

                    if( m_enableNegative )
                    {
                        rules.push_back({ "plain_negative", "polite_negative", "plain_negative", "polite non-past negative", true });
                    }

                    if( m_enablePast )
                    {
                        rules.push_back({ "past_plain", "past_polite", "past_plain", "polite past affirmative", true });
                    }

                    if( m_enablePastNegative )
                    {
                        rules.push_back({ "past_negative_plain", "past_negative_polite", "past_plain", "polite past negative", true });
                    }
                }

                return rules;
            }

            std::vector<SentenceTransformationQuizWidget::MeaningToJapaneseRule> SentenceTransformationQuizWidget::buildEnabledMeaningToJapaneseRules() const
            {
                std::vector<MeaningToJapaneseRule> rules;

                if( m_enablePlainOutput )
                {
                    if( m_enableNonPast )
                    {
                        rules.push_back({ "dictionary", "dictionary", "plain non-past affirmative", true });
                    }

                    if( m_enableNegative )
                    {
                        rules.push_back({ "plain_negative", "plain_negative", "plain non-past negative", true });
                    }

                    if( m_enablePast )
                    {
                        rules.push_back({ "past_plain", "past_plain", "plain past affirmative", true });
                    }

                    if( m_enablePastNegative )
                    {
                        rules.push_back({ "past_negative_plain", "past_plain", "plain past negative", true });
                    }
                }

                if( m_enablePoliteOutput )
                {
                    if( m_enableNonPast )
                    {
                        rules.push_back({ "polite_nonpast", "dictionary", "polite non-past affirmative", true });
                    }

                    if( m_enableNegative )
                    {
                        rules.push_back({ "polite_negative", "plain_negative", "polite non-past negative", true });
                    }

                    if( m_enablePast )
                    {
                        rules.push_back({ "past_polite", "past_plain", "polite past affirmative", true });
                    }

                    if( m_enablePastNegative )
                    {
                        rules.push_back({ "past_negative_polite", "past_plain", "polite past negative", true });
                    }
                }

                return rules;
            }

            std::vector<SentenceTransformationQuizWidget::QuizQuestion> SentenceTransformationQuizWidget::buildQuestionBank() const
            {
                std::vector<QuizQuestion> questions;
                const std::vector<TransformationRule> rules = buildEnabledRules();
                const std::vector<WordFilterRule> wordFilterRules = parseWordFilter(std::string(m_wordFilter));

                for( const DictionaryWord& word : m_words )
                {
                    if( wordMatchesFilter(word, wordFilterRules) )
                    {
                        for( const SentenceMap& sentenceMap : m_sentenceMaps )
                        {
                            if( hasTag(word, sentenceMap.requiredTag) )
                            {
                                for( const TransformationRule& rule : rules )
                                {
                                    if( rule.enabled &&
                                        (sentenceMap.conjugation == rule.templateConjugation) &&
                                        hasConjugation(word, rule.sourceConjugation) &&
                                        hasConjugation(word, rule.targetConjugation) )
                                    {
                                        std::vector<SlotItem> slots;

                                        if( sentenceMap.slotList.empty() )
                                        {
                                            slots.push_back(SlotItem{});
                                        }
                                        else
                                        {
                                            const auto slotListIterator = m_slotLists.find(sentenceMap.slotList);

                                            if( slotListIterator != m_slotLists.end() )
                                            {
                                                slots = slotListIterator->second;
                                            }
                                        }

                                        for( size_t slotIndex = 0U; slotIndex < slots.size(); ++slotIndex )
                                        {
                                            const SlotItem& slot = slots[slotIndex];
                                            const std::string sourceConjugation = getConjugationValue(word, rule.sourceConjugation);
                                            const std::string targetConjugation = getConjugationValue(word, rule.targetConjugation);
                                            const std::string prompt = getSentencePrompt(word, rule.templateConjugation);

                                            QuizQuestion question;
                                            question.key = std::format("{}:{}:{}:{}:{}", word.kana, sentenceMap.id, slotIndex, rule.sourceConjugation, rule.targetConjugation);
                                            question.sourceSentence = renderTemplate(sentenceMap.answerTemplate, slot, sourceConjugation, prompt);
                                            question.targetSentence = renderTemplate(sentenceMap.answerTemplate, slot, targetConjugation, prompt);
                                            question.questionText = renderTemplate(sentenceMap.questionTemplate, slot, targetConjugation, prompt);
                                            question.targetLabel = rule.label;
                                            question.sourceConjugation = rule.sourceConjugation;
                                            question.targetConjugation = rule.targetConjugation;
                                            question.templateConjugation = rule.templateConjugation;
                                            question.wordMeaning = word.translation;
                                            question.wordKana = word.kana;
                                            question.wordKanji = word.kanji;
                                            question.mapId = sentenceMap.id;
                                            question.slotText = slot.text;
                                            question.slotJp = slot.jp;

                                            if( !question.sourceSentence.empty() &&
                                                !question.targetSentence.empty() &&
                                                (question.sourceSentence != question.targetSentence) )
                                            {
                                                questions.push_back(question);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                return questions;
            }

            std::vector<SentenceTransformationQuizWidget::QuizQuestion> SentenceTransformationQuizWidget::buildMeaningToJapaneseQuestionBank() const
            {
                std::vector<QuizQuestion> questions;
                const std::vector<MeaningToJapaneseRule> rules = buildEnabledMeaningToJapaneseRules();
                const std::vector<WordFilterRule> wordFilterRules = parseWordFilter(std::string(m_wordFilter));

                for( const DictionaryWord& word : m_words )
                {
                    if( wordMatchesFilter(word, wordFilterRules) )
                    {
                        for( const SentenceMap& sentenceMap : m_sentenceMaps )
                        {
                            if( hasTag(word, sentenceMap.requiredTag) )
                            {
                                for( const MeaningToJapaneseRule& rule : rules )
                                {
                                    if( rule.enabled &&
                                        (sentenceMap.conjugation == rule.templateConjugation) &&
                                        hasConjugation(word, rule.targetConjugation) &&
                                        !sentenceMap.questionTemplate.empty() )
                                    {
                                        std::vector<SlotItem> slots;

                                        if( sentenceMap.slotList.empty() )
                                        {
                                            slots.push_back(SlotItem{});
                                        }
                                        else
                                        {
                                            const auto slotListIterator = m_slotLists.find(sentenceMap.slotList);

                                            if( slotListIterator != m_slotLists.end() )
                                            {
                                                slots = slotListIterator->second;
                                            }
                                        }

                                        for( size_t slotIndex = 0U; slotIndex < slots.size(); ++slotIndex )
                                        {
                                            const SlotItem& slot = slots[slotIndex];
                                            const std::string targetConjugation = getConjugationValue(word, rule.targetConjugation);
                                            const std::string prompt = getSentencePrompt(word, rule.targetConjugation);

                                            QuizQuestion question;
                                            question.key = std::format("{}:{}:{}:language_to_japanese:{}", word.kana, sentenceMap.id, slotIndex, rule.targetConjugation);
                                            question.sourceSentence = renderTemplate(sentenceMap.questionTemplate, slot, targetConjugation, prompt);
                                            question.targetSentence = renderTemplate(sentenceMap.answerTemplate, slot, targetConjugation, prompt);
                                            question.questionText.clear();
                                            question.targetLabel = rule.label;
                                            question.sourceConjugation = "source_language";
                                            question.targetConjugation = rule.targetConjugation;
                                            question.templateConjugation = rule.templateConjugation;
                                            question.wordMeaning = word.translation;
                                            question.wordKana = word.kana;
                                            question.wordKanji = word.kanji;
                                            question.mapId = sentenceMap.id;
                                            question.slotText = slot.text;
                                            question.slotJp = slot.jp;

                                            if( !question.sourceSentence.empty() && !question.targetSentence.empty() )
                                            {
                                                questions.push_back(question);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                return questions;
            }

            void SentenceTransformationQuizWidget::startQuiz()
            {
                const bool loaded = loadFiles();

                if( loaded )
                {
                    if( m_quizMode == QuizMode::Transformation )
                    {
                        m_questions = buildQuestionBank();
                    }
                    else
                    {
                        m_questions = buildMeaningToJapaneseQuestionBank();
                    }

                    if( m_randomOrder )
                    {
                        std::shuffle(m_questions.begin(), m_questions.end(), m_rng);
                    }

                    if( m_numberOfQuestions > 0 )
                    {
                        const size_t requestedSize = static_cast<size_t>(m_numberOfQuestions);

                        if( m_questions.size() > requestedSize )
                        {
                            m_questions.resize(requestedSize);
                        }
                    }

                    if( !m_questions.empty() )
                    {
                        resetQuizState();
                        m_showSettingsWindow = false;
                        m_showQuizWindow = true;
                        m_statusMessage = std::format("Quiz started with {} questions.", m_questions.size());
                    }
                    else
                    {
                        m_statusMessage = "No questions could be generated for current settings.";
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
                        m_feedbackMessage = std::format("Correct! Expected answer: {}", question.targetSentence);
                        m_waitingForNextQuestion = true;
                        scheduleAutoAdvanceAfterCorrectAnswer();
                    }
                    else
                    {
                        ++m_currentTry;

                        if( m_currentTry >= m_numberOfTries )
                        {
                            ++m_wrongAnswers;
                            m_feedbackMessage = std::format("Incorrect. Expected answer: {}", question.targetSentence);
                            m_waitingForNextQuestion = true;
                        }
                        else
                        {
                            m_feedbackMessage = std::format("Incorrect. Try again. Attempt {}/{}.", m_currentTry, m_numberOfTries);
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
                    m_autoAdvanceDueTime = ImGui::GetTime() + (static_cast<double>(m_autoAdvanceDelayMs) / 1000.0);
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
                    m_statistics.words = static_cast<uint32_t>(m_words.size());
                    m_statistics.sentenceMaps = static_cast<uint32_t>(m_sentenceMaps.size());
                    m_statistics.slotLists = static_cast<uint32_t>(m_slotLists.size());

                    uint32_t slotItems = 0U;

                    for( const auto& slotList : m_slotLists )
                    {
                        slotItems += static_cast<uint32_t>(slotList.second.size());
                    }

                    m_statistics.slotItems = slotItems;

                    if( m_quizMode == QuizMode::Transformation )
                    {
                        m_statistics.generatedQuestions = static_cast<uint32_t>(buildQuestionBank().size());
                    }
                    else
                    {
                        m_statistics.generatedQuestions = static_cast<uint32_t>(buildMeaningToJapaneseQuestionBank().size());
                    }

                    m_statusMessage = "Statistics refreshed.";
                }
            }

            std::string SentenceTransformationQuizWidget::buildFullPath(const std::string& path) const
            {
                std::filesystem::path exePath(getexepath());
                std::filesystem::path script(path);
                auto fullPath = ((exePath) / script).string();

                return fullPath;
            }

            std::string SentenceTransformationQuizWidget::renderTemplate(const std::string& text, const SlotItem& slot, const std::string& conjugation, const std::string& verbPrompt) const
            {
                std::string result = text;

                result = replaceAll(result, "{slot}", slot.text);
                result = replaceAll(result, "{slot_jp}", slot.jp);
                result = replaceAll(result, "{conjugation}", conjugation);
                result = replaceAll(result, "{verb_prompt}", verbPrompt);

                return result;
            }

            std::string SentenceTransformationQuizWidget::replaceAll(std::string text, const std::string& token, const std::string& value) const
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

                return text;
            }

            bool SentenceTransformationQuizWidget::hasTag(const DictionaryWord& word, const std::string& tag) const
            {
                bool result = false;

                for( const std::string& wordTag : word.tags )
                {
                    if( wordTag == tag )
                    {
                        result = true;
                    }
                }

                return result;
            }

            bool SentenceTransformationQuizWidget::hasConjugation(const DictionaryWord& word, const std::string& conjugationName) const
            {
                bool result = false;
                const auto iterator = word.conjugations.find(conjugationName);

                if( iterator != word.conjugations.end() )
                {
                    result = !iterator->second.value.empty();
                }

                return result;
            }

            std::string SentenceTransformationQuizWidget::getConjugationValue(const DictionaryWord& word, const std::string& conjugationName) const
            {
                std::string result;
                const auto iterator = word.conjugations.find(conjugationName);

                if( iterator != word.conjugations.end() )
                {
                    result = iterator->second.value;
                }

                return result;
            }

            std::string SentenceTransformationQuizWidget::getSentencePrompt(const DictionaryWord& word, const std::string& conjugationName) const
            {
                std::string result;
                const auto iterator = word.conjugations.find(conjugationName);

                if( iterator != word.conjugations.end() )
                {
                    result = iterator->second.sentencePrompt;
                }

                if( result.empty() )
                {
                    result = word.kana;
                }

                return result;
            }

            std::vector<SentenceTransformationQuizWidget::WordFilterRule> SentenceTransformationQuizWidget::parseWordFilter(const std::string& filterText) const
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

                            const std::string valuesText = trimText(trimmedExpression.substr(equalPosition + 1U));
                            const std::vector<std::string> values = splitText(valuesText, ',');

                            for( const std::string& value : values )
                            {
                                const std::string cleanedValue = removeOptionalQuotes(trimText(value));

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

            bool SentenceTransformationQuizWidget::wordMatchesFilter(const DictionaryWord& word, const std::vector<WordFilterRule>& rules) const
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

            bool SentenceTransformationQuizWidget::wordFieldMatches(const DictionaryWord& word, const std::string& field, const std::string& value) const
            {
                bool result = false;

                if( field == "tag" )
                {
                    for( const std::string& tag : word.tags )
                    {
                        if( tag == value )
                        {
                            result = true;
                        }
                    }
                }
                else
                {
                    const auto iterator = word.attributes.find(field);

                    if( iterator != word.attributes.end() )
                    {
                        result = (iterator->second == value);
                    }
                }

                return result;
            }

            std::vector<std::string> SentenceTransformationQuizWidget::splitText(const std::string& text, char separator) const
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

                while( !result.empty() && ((result.front() == ' ') || (result.front() == '\t')) )
                {
                    result.erase(result.begin());
                }

                while( !result.empty() && ((result.back() == ' ') || (result.back() == '\t')) )
                {
                    result.pop_back();
                }

                return result;
            }

            std::string SentenceTransformationQuizWidget::removeOptionalQuotes(const std::string& text) const
            {
                std::string result = text;

                if( result.size() >= 2U )
                {
                    const bool hasDoubleQuotes = (result.front() == '"') && (result.back() == '"');
                    const bool hasSingleQuotes = (result.front() == '\'') && (result.back() == '\'');

                    if( hasDoubleQuotes || hasSingleQuotes )
                    {
                        result = result.substr(1U, result.size() - 2U);
                    }
                }

                return result;
            }

            void SentenceTransformationQuizWidget::copyToBuffer(char* destination, size_t destinationSize, const std::string& source)
            {
                if( (destination != nullptr) && (destinationSize > 0U) )
                {
                    std::memset(destination, 0, destinationSize);
                    std::memcpy(destination, source.c_str(), std::min(source.size(), destinationSize - 1U));
                }
            }
        }
    }
}