#include "AdjectiveSentenceQuizWidget.h"

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

            AdjectiveSentenceQuizWidget::AdjectiveSentenceQuizWidget(tools::Logger& logger)
                : m_logger(logger)
                , m_rng(static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count()))
            {
            }

            void AdjectiveSentenceQuizWidget::initialize(const tools::DataPackage& r_package)
            {
                try
                {
                    const SettingsDataPackage* package = dynamic_cast<const SettingsDataPackage*>(&r_package);

                    if( package != nullptr )
                    {
                        const std::string dictionaryPath = package->get<std::string>(SettingsPackageKey::AdjectiveSentenceQuizDictionaryPath);
                        const std::string sentenceMapPath = package->get<std::string>(SettingsPackageKey::AdjectiveSentenceQuizSentenceMapPath);
                        const std::string numberOfTries = package->get<std::string>(SettingsPackageKey::TriesForQuiz);

                        copyToBuffer(m_dictionaryPath, sizeof(m_dictionaryPath), dictionaryPath);
                        copyToBuffer(m_sentenceMapPath, sizeof(m_sentenceMapPath), sentenceMapPath);

                        m_numberOfTries = std::stoi(numberOfTries);

                        m_logger.log("AdjectiveSentenceQuizWidget: Initialized.", tools::LogLevel::INFO);
                    }
                }
                catch( const std::exception& exception )
                {
                    m_logger.log("AdjectiveSentenceQuizWidget::initialize exception: " + std::string(exception.what()), tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log("AdjectiveSentenceQuizWidget::initialize unknown exception.", tools::LogLevel::PROBLEM);
                }
            }

            void AdjectiveSentenceQuizWidget::draw(bool* p_open)
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
                    m_logger.log("AdjectiveSentenceQuizWidget::draw exception: " + std::string(exception.what()), tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log("AdjectiveSentenceQuizWidget::draw unknown exception.", tools::LogLevel::PROBLEM);
                }
            }

            void AdjectiveSentenceQuizWidget::drawSettingsWindow(bool* p_open)
            {
                ImGui::SetNextWindowSize(ImVec2(700, 540), ImGuiCond_FirstUseEver);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.98f, 0.92f, 0.84f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));

                bool windowOpen = true;

                if( ImGui::Begin("Adjective Sentence Quiz Settings", &windowOpen, ImGuiWindowFlags_NoCollapse) )
                {
                    if( ImGui::IsKeyPressed(ImGuiKey_F1) )
                    {
                        if( !m_showStatistics )
                        {
                            updateStatistics();
                        }

                        m_showStatistics = !m_showStatistics;
                    }

                    ImGui::TextWrapped("This quiz creates Japanese adjective sentence questions from a dictionary XML file and a sentence map XML file.");
                    ImGui::Separator();

                    ImGui::SeparatorText("Quiz mode");

                    const char* quizModes[] =
                    {
                        "Write Japanese sentence from source sentence",
                        "Transform Japanese sentence"
                    };

                    if( ImGui::Combo("Mode", &m_quizModeIndex, quizModes, IM_ARRAYSIZE(quizModes)) )
                    {
                        m_quizMode = static_cast<QuizMode>(m_quizModeIndex);
                        m_showStatistics = false;
                    }

                    ImGui::Separator();

                    ImGui::InputText("Word filter", m_wordFilter, IM_ARRAYSIZE(m_wordFilter));
                    ImGui::TextDisabled("Examples: type=i_adjective | type=na_adjective | tag=test_description | type=i_adjective;tag=test_description");

                    ImGui::Spacing();

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
                    ImGui::SeparatorText("Adjective placement");

                    ImGui::Checkbox("After topic noun", &m_enablePredicateSentences);
                    ImGui::SameLine();
                    ImGui::TextDisabled("このテストはむずかしいです");
                    ImGui::SameLine();
                    ImGui::Checkbox("Before noun", &m_enableAttributiveSentences);
                    ImGui::SameLine();
                    ImGui::TextDisabled("これはむずかしいテストです");

                    ImGui::Spacing();
                    ImGui::SeparatorText("Adjective types");

                    ImGui::Checkbox("i-adjectives", &m_enableIAdjectives);
                    ImGui::SameLine();
                    ImGui::Checkbox("na-adjectives", &m_enableNaAdjectives);

                    ImGui::Spacing();
                    ImGui::SeparatorText("Adjective combination");

                    ImGui::SliderInt("Adjectives per sentence", &m_numberOfAdjectives, 1, 4);
                    ImGui::TextDisabled("1 = no combination. Values above 1 require the connective form in XML.");

                    ImGui::Spacing();
                    ImGui::SeparatorText("Forms");

                    if( ImGui::BeginTable("AdjectiveQuizFormsTable", 4, ImGuiTableFlags_SizingStretchSame) )
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
                    //      ImGui::Checkbox("Random order", &m_randomOrder);
                         // ImGui::SliderInt("Auto advance after correct answer [ms]", &m_autoAdvanceDelayMs, 0, 10000);

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

                        ImGui::Text("Adjectives: %u", m_statistics.words);
                        ImGui::Text("Sentence maps: %u", m_statistics.sentenceMaps);
                        ImGui::Text("Slot lists: %u", m_statistics.slotLists);
                        ImGui::Text("Slot items: %u", m_statistics.slotItems);
                        ImGui::Text("Adjectives per sentence: %d", m_numberOfAdjectives);
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

            void AdjectiveSentenceQuizWidget::drawQuizWindow(bool* p_open)
            {
                ImGui::SetNextWindowSize(ImVec2(720, 540), ImGuiCond_FirstUseEver);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.98f, 0.92f, 0.84f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));

                bool windowOpen = true;

                if( ImGui::Begin("Adjective Sentence Quiz", &windowOpen, ImGuiWindowFlags_NoCollapse) )
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

                        ImGui::TextDisabled("Placement: %s", placementToText(question.placement).c_str());

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

                            ImGui::TextDisabled("Adjective meaning: %s", question.wordMeaning.c_str());
                            ImGui::TextDisabled("Adjective kana: %s", question.wordKana.c_str());
                            ImGui::TextDisabled("Adjective kanji: %s", question.wordKanji.c_str());
                            ImGui::TextDisabled("Adjective type: %s", question.wordType.c_str());
                            ImGui::TextDisabled("Adjective count: %u", question.adjectiveCount);
                            ImGui::TextDisabled("Map: %s", question.mapId.c_str());
                            ImGui::TextDisabled("Source form: %s", question.sourceForm.c_str());
                            ImGui::TextDisabled("Target form: %s", question.targetForm.c_str());
                            ImGui::TextDisabled("Source copula: %s", question.sourceCopula.c_str());
                            ImGui::TextDisabled("Target copula: %s", question.targetCopula.c_str());
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

            bool AdjectiveSentenceQuizWidget::loadFiles()
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
                    m_logger.log("AdjectiveSentenceQuizWidget::loadFiles exception: " + std::string(exception.what()), tools::LogLevel::PROBLEM);
                    m_statusMessage = exception.what();
                }
                catch( ... )
                {
                    m_logger.log("AdjectiveSentenceQuizWidget::loadFiles unknown exception.", tools::LogLevel::PROBLEM);
                    m_statusMessage = "Unknown error while loading files.";
                }

                return result;
            }

            bool AdjectiveSentenceQuizWidget::loadDictionary(const std::string& path)
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
                        DictionaryAdjective word;

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

                        loadFormsFromWordNode(wordNode, word);

                        if( !word.kana.empty() && !word.forms.empty() )
                        {
                            m_words.push_back(word);
                        }
                    }

                    result = !m_words.empty();
                }

                if( !result )
                {
                    m_logger.log("Could not load adjective quiz dictionary XML from path: " + fullPath + ". Parser status: " + parseResult.description(), tools::LogLevel::PROBLEM);
                }

                return result;
            }

            void AdjectiveSentenceQuizWidget::loadFormsFromWordNode(const pugi::xml_node& wordNode, DictionaryAdjective& r_word)
            {
                const pugi::xml_node formsNode = wordNode.child("forms");

                for( pugi::xml_node formNode = formsNode.child("form"); formNode; formNode = formNode.next_sibling("form") )
                {
                    FormData form;
                    form.name = GetAttribute(formNode, "name");
                    form.style = GetAttribute(formNode, "style");
                    form.sentencePrompt = GetAttribute(formNode, "sentence_prompt");
                    form.value = GetText(formNode);

                    if( !form.name.empty() && !form.value.empty() )
                    {
                        r_word.forms[form.name] = form;
                    }
                }

                const pugi::xml_node conjugationsNode = wordNode.child("conjugations");

                for( pugi::xml_node conjugationNode = conjugationsNode.child("conjugation"); conjugationNode; conjugationNode = conjugationNode.next_sibling("conjugation") )
                {
                    FormData form;
                    form.name = GetAttribute(conjugationNode, "name");
                    form.style = GetAttribute(conjugationNode, "style");
                    form.sentencePrompt = GetAttribute(conjugationNode, "sentence_prompt");
                    form.value = GetText(conjugationNode);

                    if( !form.name.empty() && !form.value.empty() )
                    {
                        r_word.forms[form.name] = form;
                    }
                }
            }

            bool AdjectiveSentenceQuizWidget::loadSentenceMaps(const std::string& path)
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
                        std::string placementText;

                        sentenceMap.id = GetAttribute(mapNode, "id");
                        sentenceMap.label = GetAttribute(mapNode, "label");
                        sentenceMap.requiredTag = GetAttribute(mapNode, "requires_tag");
                        sentenceMap.slotList = GetAttribute(mapNode, "slot_list");
                        sentenceMap.questionTemplate = GetText(mapNode.child("question"));
                        sentenceMap.answerTemplate = GetText(mapNode.child("answer"));

                        placementText = GetAttribute(mapNode, "placement");

                        if( placementText.empty() )
                        {
                            placementText = GetAttribute(mapNode, "position");
                        }

                        if( placementText.empty() )
                        {
                            placementText = GetAttribute(mapNode, "form_group");
                        }

                        sentenceMap.placement = parsePlacement(placementText);
                        loadCopulasFromMapNode(mapNode, sentenceMap);

                        if( !sentenceMap.id.empty() &&
                            !sentenceMap.requiredTag.empty() &&
                            !sentenceMap.answerTemplate.empty() )
                        {
                            m_sentenceMaps.push_back(sentenceMap);
                        }
                    }

                    result = !m_sentenceMaps.empty();
                }

                if( !result )
                {
                    m_logger.log("Could not load adjective sentence maps XML from path: " + fullPath + ". Parser status: " + parseResult.description(), tools::LogLevel::PROBLEM);
                }

                return result;
            }

            void AdjectiveSentenceQuizWidget::loadCopulasFromMapNode(const pugi::xml_node& mapNode, SentenceMap& r_sentenceMap)
            {
                const pugi::xml_node copulasNode = mapNode.child("copulas");

                for( pugi::xml_node copulaNode = copulasNode.child("copula"); copulaNode; copulaNode = copulaNode.next_sibling("copula") )
                {
                    CopulaData copula;
                    copula.name = GetAttribute(copulaNode, "name");
                    copula.prompt = GetAttribute(copulaNode, "prompt");
                    copula.value = GetText(copulaNode);

                    if( !copula.name.empty() && !copula.value.empty() )
                    {
                        r_sentenceMap.copulas[copula.name] = copula;
                    }
                }
            }

            std::vector<AdjectiveSentenceQuizWidget::AdjectiveRule> AdjectiveSentenceQuizWidget::buildEnabledTransformationRules() const
            {
                std::vector<AdjectiveRule> rules;

                if( m_enablePoliteToPlain )
                {
                    if( m_enablePredicateSentences )
                    {
                        if( m_enableNonPast )
                        {
                            rules.push_back({ SentencePlacement::Predicate, "predicate_polite_nonpast", "predicate_plain_nonpast", "", "", "plain non-past adjective predicate", true });
                        }

                        if( m_enableNegative )
                        {
                            rules.push_back({ SentencePlacement::Predicate, "predicate_polite_negative", "predicate_plain_negative", "", "", "plain non-past negative adjective predicate", true });
                        }

                        if( m_enablePast )
                        {
                            rules.push_back({ SentencePlacement::Predicate, "predicate_polite_past", "predicate_plain_past", "", "", "plain past adjective predicate", true });
                        }

                        if( m_enablePastNegative )
                        {
                            rules.push_back({ SentencePlacement::Predicate, "predicate_polite_past_negative", "predicate_plain_past_negative", "", "", "plain past negative adjective predicate", true });
                        }
                    }

                    if( m_enableAttributiveSentences )
                    {
                        if( m_enableNonPast )
                        {
                            rules.push_back({ SentencePlacement::Attributive, "attributive_nonpast", "attributive_nonpast", "polite_nonpast", "plain_nonpast", "plain non-past noun sentence", true });
                        }

                        if( m_enableNegative )
                        {
                            rules.push_back({ SentencePlacement::Attributive, "attributive_nonpast", "attributive_nonpast", "polite_negative", "plain_negative", "plain non-past negative noun sentence", true });
                        }

                        if( m_enablePast )
                        {
                            rules.push_back({ SentencePlacement::Attributive, "attributive_nonpast", "attributive_nonpast", "polite_past", "plain_past", "plain past noun sentence", true });
                        }

                        if( m_enablePastNegative )
                        {
                            rules.push_back({ SentencePlacement::Attributive, "attributive_nonpast", "attributive_nonpast", "polite_past_negative", "plain_past_negative", "plain past negative noun sentence", true });
                        }
                    }
                }

                if( m_enablePlainToPolite )
                {
                    if( m_enablePredicateSentences )
                    {
                        if( m_enableNonPast )
                        {
                            rules.push_back({ SentencePlacement::Predicate, "predicate_plain_nonpast", "predicate_polite_nonpast", "", "", "polite non-past adjective predicate", true });
                        }

                        if( m_enableNegative )
                        {
                            rules.push_back({ SentencePlacement::Predicate, "predicate_plain_negative", "predicate_polite_negative", "", "", "polite non-past negative adjective predicate", true });
                        }

                        if( m_enablePast )
                        {
                            rules.push_back({ SentencePlacement::Predicate, "predicate_plain_past", "predicate_polite_past", "", "", "polite past adjective predicate", true });
                        }

                        if( m_enablePastNegative )
                        {
                            rules.push_back({ SentencePlacement::Predicate, "predicate_plain_past_negative", "predicate_polite_past_negative", "", "", "polite past negative adjective predicate", true });
                        }
                    }

                    if( m_enableAttributiveSentences )
                    {
                        if( m_enableNonPast )
                        {
                            rules.push_back({ SentencePlacement::Attributive, "attributive_nonpast", "attributive_nonpast", "plain_nonpast", "polite_nonpast", "polite non-past noun sentence", true });
                        }

                        if( m_enableNegative )
                        {
                            rules.push_back({ SentencePlacement::Attributive, "attributive_nonpast", "attributive_nonpast", "plain_negative", "polite_negative", "polite non-past negative noun sentence", true });
                        }

                        if( m_enablePast )
                        {
                            rules.push_back({ SentencePlacement::Attributive, "attributive_nonpast", "attributive_nonpast", "plain_past", "polite_past", "polite past noun sentence", true });
                        }

                        if( m_enablePastNegative )
                        {
                            rules.push_back({ SentencePlacement::Attributive, "attributive_nonpast", "attributive_nonpast", "plain_past_negative", "polite_past_negative", "polite past negative noun sentence", true });
                        }
                    }
                }

                return rules;
            }

            std::vector<AdjectiveSentenceQuizWidget::AdjectiveRule> AdjectiveSentenceQuizWidget::buildEnabledMeaningToJapaneseRules() const
            {
                std::vector<AdjectiveRule> rules;

                appendMeaningRulesForPlacement(rules, SentencePlacement::Predicate, m_enablePlainOutput, m_enablePoliteOutput);
                appendMeaningRulesForPlacement(rules, SentencePlacement::Attributive, m_enablePlainOutput, m_enablePoliteOutput);

                return rules;
            }

            void AdjectiveSentenceQuizWidget::appendMeaningRulesForPlacement(std::vector<AdjectiveRule>& r_rules, SentencePlacement placement, bool plainOutput, bool politeOutput) const
            {
                if( isPlacementEnabled(placement) )
                {
                    if( placement == SentencePlacement::Predicate )
                    {
                        if( plainOutput )
                        {
                            if( m_enableNonPast )
                            {
                                r_rules.push_back({ placement, "source_language", "predicate_plain_nonpast", "", "", "plain non-past adjective predicate", true });
                            }

                            if( m_enableNegative )
                            {
                                r_rules.push_back({ placement, "source_language", "predicate_plain_negative", "", "", "plain non-past negative adjective predicate", true });
                            }

                            if( m_enablePast )
                            {
                                r_rules.push_back({ placement, "source_language", "predicate_plain_past", "", "", "plain past adjective predicate", true });
                            }

                            if( m_enablePastNegative )
                            {
                                r_rules.push_back({ placement, "source_language", "predicate_plain_past_negative", "", "", "plain past negative adjective predicate", true });
                            }
                        }

                        if( politeOutput )
                        {
                            if( m_enableNonPast )
                            {
                                r_rules.push_back({ placement, "source_language", "predicate_polite_nonpast", "", "", "polite non-past adjective predicate", true });
                            }

                            if( m_enableNegative )
                            {
                                r_rules.push_back({ placement, "source_language", "predicate_polite_negative", "", "", "polite non-past negative adjective predicate", true });
                            }

                            if( m_enablePast )
                            {
                                r_rules.push_back({ placement, "source_language", "predicate_polite_past", "", "", "polite past adjective predicate", true });
                            }

                            if( m_enablePastNegative )
                            {
                                r_rules.push_back({ placement, "source_language", "predicate_polite_past_negative", "", "", "polite past negative adjective predicate", true });
                            }
                        }
                    }
                    else
                    {
                        if( plainOutput )
                        {
                            if( m_enableNonPast )
                            {
                                r_rules.push_back({ placement, "source_language", "attributive_nonpast", "", "plain_nonpast", "plain non-past noun sentence", true });
                            }

                            if( m_enableNegative )
                            {
                                r_rules.push_back({ placement, "source_language", "attributive_nonpast", "", "plain_negative", "plain non-past negative noun sentence", true });
                            }

                            if( m_enablePast )
                            {
                                r_rules.push_back({ placement, "source_language", "attributive_nonpast", "", "plain_past", "plain past noun sentence", true });
                            }

                            if( m_enablePastNegative )
                            {
                                r_rules.push_back({ placement, "source_language", "attributive_nonpast", "", "plain_past_negative", "plain past negative noun sentence", true });
                            }
                        }

                        if( politeOutput )
                        {
                            if( m_enableNonPast )
                            {
                                r_rules.push_back({ placement, "source_language", "attributive_nonpast", "", "polite_nonpast", "polite non-past noun sentence", true });
                            }

                            if( m_enableNegative )
                            {
                                r_rules.push_back({ placement, "source_language", "attributive_nonpast", "", "polite_negative", "polite non-past negative noun sentence", true });
                            }

                            if( m_enablePast )
                            {
                                r_rules.push_back({ placement, "source_language", "attributive_nonpast", "", "polite_past", "polite past noun sentence", true });
                            }

                            if( m_enablePastNegative )
                            {
                                r_rules.push_back({ placement, "source_language", "attributive_nonpast", "", "polite_past_negative", "polite past negative noun sentence", true });
                            }
                        }
                    }
                }
            }

            std::vector<AdjectiveSentenceQuizWidget::AdjectiveChain> AdjectiveSentenceQuizWidget::buildAdjectiveChains(const DictionaryAdjective& targetWord, const SentenceMap& sentenceMap, const std::vector<WordFilterRule>& wordFilterRules, const std::string& finalFormName) const
            {
                std::vector<AdjectiveChain> chains;
                int requestedAdjectives = m_numberOfAdjectives;

                if( requestedAdjectives < 1 )
                {
                    requestedAdjectives = 1;
                }

                if( requestedAdjectives > 4 )
                {
                    requestedAdjectives = 4;
                }

                if( hasForm(targetWord, finalFormName) )
                {
                    if( requestedAdjectives == 1 )
                    {
                        AdjectiveChain chain;
                        chain.words.push_back(&targetWord);
                        chains.push_back(chain);
                    }
                    else
                    {
                        std::vector<const DictionaryAdjective*> candidates;

                        for( const DictionaryAdjective& candidate : m_words )
                        {
                            if( (&candidate != &targetWord) &&
                                wordMatchesFilter(candidate, wordFilterRules) &&
                                isAdjectiveTypeEnabled(candidate) &&
                                hasTag(candidate, sentenceMap.requiredTag) &&
                                hasForm(candidate, "connective") )
                            {
                                candidates.push_back(&candidate);
                            }
                        }

                        const size_t prefixCount = static_cast<size_t>(requestedAdjectives - 1);

                        if( candidates.size() >= prefixCount )
                        {
                            const size_t maxChainsPerTarget = 8U;
                            const size_t numberOfChains = std::min(candidates.size(), maxChainsPerTarget);

                            for( size_t chainIndex = 0U; chainIndex < numberOfChains; ++chainIndex )
                            {
                                AdjectiveChain chain;

                                for( size_t prefixIndex = 0U; prefixIndex < prefixCount; ++prefixIndex )
                                {
                                    const size_t candidateIndex = (chainIndex + prefixIndex) % candidates.size();
                                    chain.words.push_back(candidates[candidateIndex]);
                                }

                                chain.words.push_back(&targetWord);
                                chains.push_back(chain);
                            }
                        }
                    }
                }

                return chains;
            }

            std::string AdjectiveSentenceQuizWidget::buildCombinedForm(const AdjectiveChain& chain, const std::string& finalFormName) const
            {
                std::string result;

                if( !chain.words.empty() )
                {
                    const size_t lastIndex = chain.words.size() - 1U;

                    for( size_t index = 0U; index < chain.words.size(); ++index )
                    {
                        const DictionaryAdjective* word = chain.words[index];

                        if( word != nullptr )
                        {
                            if( index < lastIndex )
                            {
                                const std::string connectiveForm = getFormValue(*word, "connective");

                                if( !connectiveForm.empty() )
                                {
                                    if( !result.empty() )
                                    {
                                        result += "、";
                                    }

                                    result += connectiveForm;
                                }
                            }
                            else
                            {
                                const std::string finalForm = getFormValue(*word, finalFormName);

                                if( !finalForm.empty() )
                                {
                                    if( !result.empty() )
                                    {
                                        result += "、";
                                    }

                                    result += finalForm;
                                }
                            }
                        }
                    }
                }

                return result;
            }

            std::string AdjectiveSentenceQuizWidget::buildCombinedPrompt(const AdjectiveChain& chain, const std::string& finalFormName) const
            {
                std::string result;

                if( !chain.words.empty() )
                {
                    const size_t lastIndex = chain.words.size() - 1U;

                    if( chain.words.size() == 1U )
                    {
                        const DictionaryAdjective* word = chain.words.front();

                        if( word != nullptr )
                        {
                            result = getSentencePrompt(*word, finalFormName);
                        }
                    }
                    else
                    {
                        std::vector<std::string> parts;
                        std::string finalPrompt;
                        std::string promptPrefix;

                        for( size_t index = 0U; index < chain.words.size(); ++index )
                        {
                            const DictionaryAdjective* word = chain.words[index];

                            if( word != nullptr )
                            {
                                if( index < lastIndex )
                                {
                                    const std::string prompt = getSentencePrompt(*word, "connective");

                                    if( !prompt.empty() )
                                    {
                                        parts.push_back(prompt);
                                    }
                                }
                                else
                                {
                                    finalPrompt = getSentencePrompt(*word, finalFormName);
                                }
                            }
                        }

                        const std::vector<std::string> prefixes =
                        {
                            "nie był ",
                            "nie była ",
                            "nie było ",
                            "był ",
                            "była ",
                            "było ",
                            "nie jest ",
                            "jest "
                        };

                        for( const std::string& prefix : prefixes )
                        {
                            if( finalPrompt.rfind(prefix, 0U) == 0U )
                            {
                                promptPrefix = prefix;
                                finalPrompt = finalPrompt.substr(prefix.size());
                            }
                        }

                        if( !finalPrompt.empty() )
                        {
                            parts.push_back(finalPrompt);
                        }

                        result = promptPrefix;

                        for( size_t index = 0U; index < parts.size(); ++index )
                        {
                            if( index > 0U )
                            {
                                if( index == (parts.size() - 1U) )
                                {
                                    result += " i ";
                                }
                                else
                                {
                                    result += ", ";
                                }
                            }

                            result += parts[index];
                        }
                    }
                }

                return result;
            }

            std::string AdjectiveSentenceQuizWidget::buildChainKey(const AdjectiveChain& chain) const
            {
                std::string result;

                for( const DictionaryAdjective* word : chain.words )
                {
                    if( word != nullptr )
                    {
                        if( !result.empty() )
                        {
                            result += "+";
                        }

                        result += word->kana;
                    }
                }

                return result;
            }

            std::string AdjectiveSentenceQuizWidget::buildChainInfo(const AdjectiveChain& chain, const std::string& field) const
            {
                std::string result;

                for( const DictionaryAdjective* word : chain.words )
                {
                    if( word != nullptr )
                    {
                        std::string value;

                        if( field == "translation" )
                        {
                            value = word->translation;
                        }
                        else if( field == "kana" )
                        {
                            value = word->kana;
                        }
                        else if( field == "kanji" )
                        {
                            value = word->kanji;
                        }
                        else if( field == "type" )
                        {
                            value = word->type;
                        }

                        if( !value.empty() )
                        {
                            if( !result.empty() )
                            {
                                result += " + ";
                            }

                            result += value;
                        }
                    }
                }

                return result;
            }


            std::vector<AdjectiveSentenceQuizWidget::QuizQuestion> AdjectiveSentenceQuizWidget::buildTransformationQuestionBank() const
            {
                std::vector<QuizQuestion> questions;
                const std::vector<AdjectiveRule> rules = buildEnabledTransformationRules();
                const std::vector<WordFilterRule> wordFilterRules = parseWordFilter(std::string(m_wordFilter));

                for( const DictionaryAdjective& word : m_words )
                {
                    if( wordMatchesFilter(word, wordFilterRules) && isAdjectiveTypeEnabled(word) )
                    {
                        for( const SentenceMap& sentenceMap : m_sentenceMaps )
                        {
                            if( hasTag(word, sentenceMap.requiredTag) && isPlacementEnabled(sentenceMap.placement) )
                            {
                                for( const AdjectiveRule& rule : rules )
                                {
                                    if( rule.enabled &&
                                        (sentenceMap.placement == rule.placement) &&
                                        hasForm(word, rule.sourceForm) &&
                                        hasForm(word, rule.targetForm) &&
                                        hasCopula(sentenceMap, rule.sourceCopula) &&
                                        hasCopula(sentenceMap, rule.targetCopula) )
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

                                        const std::vector<AdjectiveChain> chains = buildAdjectiveChains(word, sentenceMap, wordFilterRules, rule.targetForm);

                                        for( size_t slotIndex = 0U; slotIndex < slots.size(); ++slotIndex )
                                        {
                                            const SlotItem& slot = slots[slotIndex];

                                            for( size_t chainIndex = 0U; chainIndex < chains.size(); ++chainIndex )
                                            {
                                                const AdjectiveChain& chain = chains[chainIndex];
                                                const std::string sourceForm = buildCombinedForm(chain, rule.sourceForm);
                                                const std::string targetForm = buildCombinedForm(chain, rule.targetForm);
                                                const std::string sourcePrompt = buildCombinedPrompt(chain, rule.sourceForm);
                                                const std::string targetPrompt = buildCombinedPrompt(chain, rule.targetForm);
                                                const std::string sourceCopulaValue = getCopulaValue(sentenceMap, rule.sourceCopula);
                                                const std::string targetCopulaValue = getCopulaValue(sentenceMap, rule.targetCopula);
                                                const std::string sourceCopulaPrompt = getCopulaPrompt(sentenceMap, rule.sourceCopula);
                                                const std::string targetCopulaPrompt = getCopulaPrompt(sentenceMap, rule.targetCopula);
                                                const std::unordered_map<std::string, std::string> sourceReplacements = buildReplacements(word, slot, sourceForm, sourceCopulaValue, sourceCopulaPrompt, sourcePrompt);
                                                const std::unordered_map<std::string, std::string> targetReplacements = buildReplacements(word, slot, targetForm, targetCopulaValue, targetCopulaPrompt, targetPrompt);

                                                QuizQuestion question;
                                                question.key = std::format("{}:{}:{}:{}:{}:{}:{}", buildChainKey(chain), sentenceMap.id, slotIndex, chainIndex, rule.sourceForm, rule.targetForm, rule.targetCopula);
                                                question.sourceSentence = renderTemplate(sentenceMap.answerTemplate, sourceReplacements);
                                                question.targetSentence = renderTemplate(sentenceMap.answerTemplate, targetReplacements);
                                                question.questionText = renderTemplate(sentenceMap.questionTemplate, targetReplacements);
                                                question.targetLabel = rule.label;
                                                question.sourceForm = rule.sourceForm;
                                                question.targetForm = rule.targetForm;
                                                question.sourceCopula = sourceCopulaValue;
                                                question.targetCopula = targetCopulaValue;
                                                question.wordMeaning = buildChainInfo(chain, "translation");
                                                question.wordKana = buildChainInfo(chain, "kana");
                                                question.wordKanji = buildChainInfo(chain, "kanji");
                                                question.wordType = buildChainInfo(chain, "type");
                                                question.mapId = sentenceMap.id;
                                                question.slotText = slot.text;
                                                question.slotJp = slot.jp;
                                                question.placement = sentenceMap.placement;
                                                question.adjectiveCount = static_cast<uint32_t>(chain.words.size());

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
                }

                return questions;
            }

            std::vector<AdjectiveSentenceQuizWidget::QuizQuestion> AdjectiveSentenceQuizWidget::buildMeaningToJapaneseQuestionBank() const
            {
                std::vector<QuizQuestion> questions;
                const std::vector<AdjectiveRule> rules = buildEnabledMeaningToJapaneseRules();
                const std::vector<WordFilterRule> wordFilterRules = parseWordFilter(std::string(m_wordFilter));

                for( const DictionaryAdjective& word : m_words )
                {
                    if( wordMatchesFilter(word, wordFilterRules) && isAdjectiveTypeEnabled(word) )
                    {
                        for( const SentenceMap& sentenceMap : m_sentenceMaps )
                        {
                            if( hasTag(word, sentenceMap.requiredTag) && isPlacementEnabled(sentenceMap.placement) && !sentenceMap.questionTemplate.empty() )
                            {
                                for( const AdjectiveRule& rule : rules )
                                {
                                    if( rule.enabled &&
                                        (sentenceMap.placement == rule.placement) &&
                                        hasForm(word, rule.targetForm) &&
                                        hasCopula(sentenceMap, rule.targetCopula) )
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

                                        const std::vector<AdjectiveChain> chains = buildAdjectiveChains(word, sentenceMap, wordFilterRules, rule.targetForm);

                                        for( size_t slotIndex = 0U; slotIndex < slots.size(); ++slotIndex )
                                        {
                                            const SlotItem& slot = slots[slotIndex];

                                            for( size_t chainIndex = 0U; chainIndex < chains.size(); ++chainIndex )
                                            {
                                                const AdjectiveChain& chain = chains[chainIndex];
                                                const std::string targetForm = buildCombinedForm(chain, rule.targetForm);
                                                const std::string targetPrompt = buildCombinedPrompt(chain, rule.targetForm);
                                                const std::string targetCopulaValue = getCopulaValue(sentenceMap, rule.targetCopula);
                                                const std::string targetCopulaPrompt = getCopulaPrompt(sentenceMap, rule.targetCopula);
                                                const std::unordered_map<std::string, std::string> targetReplacements = buildReplacements(word, slot, targetForm, targetCopulaValue, targetCopulaPrompt, targetPrompt);

                                                QuizQuestion question;
                                                question.key = std::format("{}:{}:{}:{}:language_to_japanese:{}:{}", buildChainKey(chain), sentenceMap.id, slotIndex, chainIndex, rule.targetForm, rule.targetCopula);
                                                question.sourceSentence = renderTemplate(sentenceMap.questionTemplate, targetReplacements);
                                                question.targetSentence = renderTemplate(sentenceMap.answerTemplate, targetReplacements);
                                                question.questionText.clear();
                                                question.targetLabel = rule.label;
                                                question.sourceForm = "source_language";
                                                question.targetForm = rule.targetForm;
                                                question.sourceCopula = rule.sourceCopula;
                                                question.targetCopula = targetCopulaValue;
                                                question.wordMeaning = buildChainInfo(chain, "translation");
                                                question.wordKana = buildChainInfo(chain, "kana");
                                                question.wordKanji = buildChainInfo(chain, "kanji");
                                                question.wordType = buildChainInfo(chain, "type");
                                                question.mapId = sentenceMap.id;
                                                question.slotText = slot.text;
                                                question.slotJp = slot.jp;
                                                question.placement = sentenceMap.placement;
                                                question.adjectiveCount = static_cast<uint32_t>(chain.words.size());

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
                }

                return questions;
            }

            void AdjectiveSentenceQuizWidget::startQuiz()
            {
                const bool loaded = loadFiles();

                if( loaded )
                {
                    if( m_quizMode == QuizMode::Transformation )
                    {
                        m_questions = buildTransformationQuestionBank();
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

            void AdjectiveSentenceQuizWidget::submitAnswer()
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

            void AdjectiveSentenceQuizWidget::scheduleAutoAdvanceAfterCorrectAnswer()
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

            void AdjectiveSentenceQuizWidget::processAutoAdvance()
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

            void AdjectiveSentenceQuizWidget::acceptCurrentAnswerAsCorrect()
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
                        m_statistics.generatedQuestions = static_cast<uint32_t>(buildTransformationQuestionBank().size());
                    }
                    else
                    {
                        m_statistics.generatedQuestions = static_cast<uint32_t>(buildMeaningToJapaneseQuestionBank().size());
                    }

                    m_statusMessage = "Statistics refreshed.";
                }
            }

            std::vector<AdjectiveSentenceQuizWidget::WordFilterRule> AdjectiveSentenceQuizWidget::parseWordFilter(const std::string& filterText) const
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

            bool AdjectiveSentenceQuizWidget::wordMatchesFilter(const DictionaryAdjective& word, const std::vector<WordFilterRule>& rules) const
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

            bool AdjectiveSentenceQuizWidget::wordFieldMatches(const DictionaryAdjective& word, const std::string& field, const std::string& value) const
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

            std::vector<std::string> AdjectiveSentenceQuizWidget::splitText(const std::string& text, char separator) const
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

            std::string AdjectiveSentenceQuizWidget::trimText(const std::string& text) const
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

            std::string AdjectiveSentenceQuizWidget::removeOptionalQuotes(const std::string& text) const
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

            std::string AdjectiveSentenceQuizWidget::buildFullPath(const std::string& path) const
            {
                std::filesystem::path exePath(getexepath());
                std::filesystem::path script(path);
                auto fullPath = ((exePath) / script).string();

                return fullPath;
            }

            std::string AdjectiveSentenceQuizWidget::renderTemplate(const std::string& text, const std::unordered_map<std::string, std::string>& replacements) const
            {
                std::string result = text;

                for( const auto& replacement : replacements )
                {
                    result = replaceAll(result, replacement.first, replacement.second);
                }

                return result;
            }

            std::string AdjectiveSentenceQuizWidget::replaceAll(std::string text, const std::string& token, const std::string& value) const
            {
                size_t position = 0U;

                while( (position = text.find(token, position)) != std::string::npos )
                {
                    text.replace(position, token.length(), value);
                    position += value.length();
                }

                return text;
            }

            std::string AdjectiveSentenceQuizWidget::normalizeAnswer(std::string text) const
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

            bool AdjectiveSentenceQuizWidget::hasTag(const DictionaryAdjective& word, const std::string& tag) const
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

            bool AdjectiveSentenceQuizWidget::hasForm(const DictionaryAdjective& word, const std::string& formName) const
            {
                bool result = false;
                const auto iterator = word.forms.find(formName);

                if( iterator != word.forms.end() )
                {
                    result = !iterator->second.value.empty();
                }

                return result;
            }

            std::string AdjectiveSentenceQuizWidget::getFormValue(const DictionaryAdjective& word, const std::string& formName) const
            {
                std::string result;
                const auto iterator = word.forms.find(formName);

                if( iterator != word.forms.end() )
                {
                    result = iterator->second.value;
                }

                return result;
            }

            std::string AdjectiveSentenceQuizWidget::getSentencePrompt(const DictionaryAdjective& word, const std::string& formName) const
            {
                std::string result;
                const auto iterator = word.forms.find(formName);

                if( iterator != word.forms.end() )
                {
                    result = iterator->second.sentencePrompt;
                }

                if( result.empty() )
                {
                    result = word.translation;
                }

                if( result.empty() )
                {
                    result = word.kana;
                }

                return result;
            }

            std::unordered_map<std::string, std::string> AdjectiveSentenceQuizWidget::buildReplacements(const DictionaryAdjective& word, const SlotItem& slot, const std::string& formValue, const std::string& copula, const std::string& copulaPrompt, const std::string& prompt) const
            {
                std::unordered_map<std::string, std::string> replacements;

                replacements["{slot}"] = slot.text;
                replacements["{slot_jp}"] = slot.jp;
                replacements["{form}"] = formValue;
                replacements["{adjective}"] = formValue;
                replacements["{adjective_form}"] = formValue;
                replacements["{predicate_form}"] = formValue;
                replacements["{attributive_form}"] = formValue;
                replacements["{copula}"] = copula;
                replacements["{copula_prompt}"] = copulaPrompt;
                replacements["{adjective_prompt}"] = prompt;
                replacements["{word}"] = word.kana;
                replacements["{word_kana}"] = word.kana;
                replacements["{word_kanji}"] = word.kanji;
                replacements["{translation}"] = word.translation;
                replacements["{type}"] = word.type;

                return replacements;
            }

            bool AdjectiveSentenceQuizWidget::hasCopula(const SentenceMap& sentenceMap, const std::string& copulaName) const
            {
                bool result = true;

                if( !copulaName.empty() )
                {
                    const auto iterator = sentenceMap.copulas.find(copulaName);
                    result = (iterator != sentenceMap.copulas.end()) && !iterator->second.value.empty();
                }

                return result;
            }

            std::string AdjectiveSentenceQuizWidget::getCopulaValue(const SentenceMap& sentenceMap, const std::string& copulaName) const
            {
                std::string result;

                if( !copulaName.empty() )
                {
                    const auto iterator = sentenceMap.copulas.find(copulaName);

                    if( iterator != sentenceMap.copulas.end() )
                    {
                        result = iterator->second.value;
                    }
                }

                return result;
            }

            std::string AdjectiveSentenceQuizWidget::getCopulaPrompt(const SentenceMap& sentenceMap, const std::string& copulaName) const
            {
                std::string result;

                if( !copulaName.empty() )
                {
                    const auto iterator = sentenceMap.copulas.find(copulaName);

                    if( iterator != sentenceMap.copulas.end() )
                    {
                        result = iterator->second.prompt;
                    }
                }

                return result;
            }




            AdjectiveSentenceQuizWidget::SentencePlacement AdjectiveSentenceQuizWidget::parsePlacement(const std::string& text) const
            {
                SentencePlacement result = SentencePlacement::Predicate;
                const std::string cleanedText = trimText(text);

                if( (cleanedText == "attributive") ||
                    (cleanedText == "before_noun") ||
                    (cleanedText == "adjective_before_noun") ||
                    (cleanedText == "noun") )
                {
                    result = SentencePlacement::Attributive;
                }

                return result;
            }

            std::string AdjectiveSentenceQuizWidget::placementToText(SentencePlacement placement) const
            {
                std::string result = "after topic noun";

                if( placement == SentencePlacement::Attributive )
                {
                    result = "before noun";
                }

                return result;
            }

            bool AdjectiveSentenceQuizWidget::isPlacementEnabled(SentencePlacement placement) const
            {
                bool result = false;

                if( placement == SentencePlacement::Predicate )
                {
                    result = m_enablePredicateSentences;
                }
                else
                {
                    result = m_enableAttributiveSentences;
                }

                return result;
            }

            bool AdjectiveSentenceQuizWidget::isAdjectiveTypeEnabled(const DictionaryAdjective& word) const
            {
                bool result = false;

                if( word.type == "i_adjective" )
                {
                    result = m_enableIAdjectives;
                }
                else if( word.type == "na_adjective" )
                {
                    result = m_enableNaAdjectives;
                }
                else
                {
                    result = true;
                }

                return result;
            }

            void AdjectiveSentenceQuizWidget::copyToBuffer(char* destination, size_t destinationSize, const std::string& source)
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
