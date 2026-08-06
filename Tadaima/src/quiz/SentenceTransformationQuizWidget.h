#pragma once

#include "imgui.h"
#include "Tools/Logger.h"
#include "gui/widgets/packages/SettingsDataPackage.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @class SentenceTransformationQuizWidget
             * @brief ImGui quiz that builds Japanese sentences from a Version 6 quiz project.
             *
             * The configured SentenceQuizSentenceMapPath points to quiz_project.xml.
             * The manifest resolves dictionaries, reusable vocabulary, contexts, and
             * sentence patterns relative to its own directory.
             *
             * The widget remains a verb quiz. A usable pattern therefore has one
             * verb token selected by `.form`. Other verbs, adjectives, entities,
             * profiles, relations, and contexts are resolved generically from XML.
             */
            class SentenceTransformationQuizWidget
            {
            public:
                /**
                 * @brief Creates the sentence transformation quiz widget.
                 * @param logger Logger used for diagnostic messages.
                 */
                explicit SentenceTransformationQuizWidget(tools::Logger& logger);

                /**
                 * @brief Initializes the project path and quiz settings.
                 * @param r_package Application settings package.
                 */
                void initialize(const tools::DataPackage& r_package);

                /**
                 * @brief Draws the active widget window.
                 * @param p_open Pointer controlling visibility of the widget.
                 */
                void draw(bool* p_open);

            private:
                enum class QuizMode
                {
                    Transformation = 0,
                    LanguageToJapanese = 1
                };

                enum class SlotKind
                {
                    Verb,
                    Adjective,
                    Entity
                };

                enum class FormCategory
                {
                    NonPast,
                    Negative,
                    Past,
                    PastNegative
                };

                struct FormData
                {
                    std::string name;
                    std::string style;
                    std::string context;
                    std::string prompt;
                    std::string promptMasculine;
                    std::string promptFeminine;
                    std::string promptNeuter;
                    std::string promptPlural;
                    std::string value;
                };

                struct DictionaryWord
                {
                    std::string id;
                    std::string dictionaryId;
                    std::string translation;
                    std::string kana;
                    std::string kanji;
                    std::string type;
                    std::string ending;
                    std::vector<std::string> tags;
                    std::unordered_map<std::string, std::vector<std::string>> relations;
                    std::unordered_map<std::string, FormData> forms;
                    std::unordered_map<std::string, std::string> attributes;
                };

                struct WordFilterRule
                {
                    std::string field;
                    std::vector<std::string> values;
                };

                struct TagProfile
                {
                    std::string id;
                    std::vector<std::string> includes;
                    std::vector<std::string> tags;
                };

                struct EntityData
                {
                    std::string id;
                    std::string kana;
                    std::string kanji;
                    std::string gender;
                    std::unordered_map<std::string, std::string> polish;
                    std::vector<std::string> profiles;
                    std::vector<std::string> tags;
                };

                struct ContextOptionData
                {
                    std::string id;
                    std::string polishPrefix;
                    std::string japanesePrefix;
                    uint32_t weight = 1U;
                };

                struct ContextPoolData
                {
                    std::string id;
                    std::string label;
                    std::vector<ContextOptionData> options;
                };

                struct SentencePatternData
                {
                    std::string id;
                    std::string label;
                    std::string questionTemplate;
                    std::string answerTemplate;
                };

                struct PatternAnalysis
                {
                    bool valid = false;
                    std::string error;
                    std::vector<std::string> wordSlots;
                    std::vector<std::string> entitySlots;
                    std::string dynamicSlot;
                    std::unordered_map<std::string, std::string> primarySpecs;
                    std::unordered_map<std::string, std::string> driverForEntity;
                    std::unordered_map<std::string, std::string> entityForAdjective;
                };

                struct GenerationRule
                {
                    FormCategory category = FormCategory::NonPast;
                    bool hasSourceForm = false;
                    bool sourcePolite = false;
                    bool targetPolite = false;
                    std::string label;
                };

                struct SentenceBinding
                {
                    std::unordered_map<std::string, const DictionaryWord*> words;
                    std::unordered_map<std::string, const EntityData*> entities;
                };

                struct QuestionSeed
                {
                    const SentencePatternData* pattern = nullptr;
                    PatternAnalysis analysis;
                    GenerationRule rule;
                    SentenceBinding binding;
                    const ContextPoolData* contextPool = nullptr;
                    const ContextOptionData* contextOption = nullptr;
                    std::string sourceForm;
                    std::string targetForm;
                    uint32_t selectionWeight = 1U;
                };

                struct QuizQuestion
                {
                    std::string key;
                    std::string sourceSentence;
                    std::string targetSentence;
                    std::string questionText;
                    std::string targetLabel;
                    std::string sourceConjugation;
                    std::string targetConjugation;
                    std::string wordMeaning;
                    std::string wordKana;
                    std::string wordKanji;
                    std::string patternId;
                    std::string bindings;
                    uint32_t selectionWeight = 1U;
                };

                struct FileStatistics
                {
                    uint32_t words = 0U;
                    uint32_t sentencePatterns = 0U;
                    uint32_t entities = 0U;
                    uint32_t tagProfiles = 0U;
                    uint32_t generatedQuestions = 0U;
                };

                void drawSettingsWindow(bool* p_open);
                void drawQuizWindow(bool* p_open);

                bool loadFiles();
                bool loadProject(const std::string& path);
                bool loadDictionary(const std::string& dictionaryId, const std::string& fullPath);
                bool loadVocabulary(const std::string& fullPath);
                bool loadContexts(const std::string& fullPath);
                bool loadSentencePatterns(const std::string& fullPath);
                bool buildEffectiveEntityTags();
                bool appendProfileTags(
                    const std::string& profileId,
                    std::vector<std::string>& recursionPath,
                    std::vector<std::string>& tags,
                    std::string& error) const;

                std::vector<GenerationRule> buildEnabledRules() const;
                std::vector<GenerationRule> buildEnabledMeaningToJapaneseRules() const;
                std::vector<QuizQuestion> buildQuestionBank(size_t maximumQuestions) const;
                std::vector<QuizQuestion> buildMeaningToJapaneseQuestionBank(size_t maximumQuestions) const;
                std::vector<QuizQuestion> buildQuestionsForMode(
                    bool transformationMode,
                    size_t maximumQuestions) const;
                std::vector<QuestionSeed> buildQuestionSeeds(bool transformationMode) const;
                bool renderQuestionSeed(
                    const QuestionSeed& seed,
                    bool transformationMode,
                    QuizQuestion& question) const;
                uint64_t countPossibleQuestions(bool transformationMode) const;
                void applyWeightedRandomOrder(std::vector<QuestionSeed>& seeds) const;
                std::vector<SentenceBinding> buildBindings(
                    const PatternAnalysis& analysis,
                    const GenerationRule& rule) const;
                std::vector<const DictionaryWord*> getVerbCandidates(
                    const std::string& slotId,
                    const PatternAnalysis& analysis,
                    const GenerationRule& rule) const;
                std::vector<const DictionaryWord*> getAdjectiveCandidates(
                    const std::string& slotId,
                    const PatternAnalysis& analysis,
                    const SentenceBinding& binding,
                    const GenerationRule& rule) const;
                std::vector<const EntityData*> getEntityCandidates(
                    const std::string& slotId,
                    const PatternAnalysis& analysis,
                    const SentenceBinding& binding) const;

                PatternAnalysis analyzePattern(const SentencePatternData& pattern) const;
                SlotKind getSlotKind(const std::string& slotId) const;
                std::string getDefaultFormSpec(const std::string& slotId) const;
                std::string getWordSlotFilterTag(const std::string& slotId) const;
                std::string getDictionaryForSlot(const std::string& slotId) const;
                bool isFormSelector(const std::string& slotId, const std::string& field) const;
                bool isUsableVerbPattern(const PatternAnalysis& analysis) const;
                bool isEnabledForThisQuiz(const SentencePatternData& pattern) const;

                std::string getFiniteFormName(
                    const DictionaryWord& word,
                    FormCategory category,
                    bool polite) const;
                std::string getCanonicalFormName(
                    const DictionaryWord& word,
                    const std::string& formSpec,
                    const std::string& dynamicForm) const;
                const FormData* getForm(
                    const DictionaryWord& word,
                    const std::string& formSpec,
                    const std::string& dynamicForm) const;
                std::string getFormPrompt(const FormData& form, const std::string& gender) const;
                const ContextPoolData* getContextPool(const std::string& contextId) const;

                bool renderTemplate(
                    const std::string& text,
                    const PatternAnalysis& analysis,
                    const SentenceBinding& binding,
                    const std::string& dynamicForm,
                    const ContextPoolData& contextPool,
                    const ContextOptionData& contextOption,
                    std::string& rendered,
                    std::string& error) const;
                bool resolveToken(
                    const std::string& expression,
                    const PatternAnalysis& analysis,
                    const SentenceBinding& binding,
                    const std::string& dynamicForm,
                    const ContextPoolData& contextPool,
                    const ContextOptionData& contextOption,
                    std::string& value,
                    std::string& error) const;
                bool resolveWordToken(
                    const std::string& slotId,
                    const std::string& field,
                    const PatternAnalysis& analysis,
                    const SentenceBinding& binding,
                    const std::string& dynamicForm,
                    std::string& value,
                    std::string& error) const;
                bool resolveEntityToken(
                    const std::string& slotId,
                    const std::string& field,
                    const SentenceBinding& binding,
                    std::string& value,
                    std::string& error) const;

                void startQuiz();
                void submitAnswer();
                void acceptCurrentAnswerAsCorrect();
                void skipQuestion();
                void advanceQuestion();
                void resetQuizState();
                void updateStatistics();
                void scheduleAutoAdvanceAfterCorrectAnswer();
                void processAutoAdvance();

                std::vector<WordFilterRule> parseWordFilter(const std::string& filterText) const;
                bool wordMatchesFilter(const DictionaryWord& word, const std::vector<WordFilterRule>& rules) const;
                bool wordFieldMatches(const DictionaryWord& word, const std::string& field, const std::string& value) const;
                bool wordHasTag(const DictionaryWord& word, const std::string& tag) const;
                bool wordCanUseEntity(const DictionaryWord& word, const std::string& role, const EntityData& entity) const;
                bool entityMatchesAdjective(const EntityData& entity, const DictionaryWord& adjective) const;
                bool tagsIntersect(const std::vector<std::string>& first, const std::vector<std::string>& second) const;
                bool containsValue(const std::vector<std::string>& values, const std::string& value) const;
                void appendUnique(std::vector<std::string>& values, const std::string& value) const;

                std::vector<std::string> extractTokens(const std::string& text) const;
                void splitToken(const std::string& expression, std::string& root, std::string& field) const;
                std::vector<std::string> splitText(const std::string& text, char separator) const;
                std::string trimText(const std::string& text) const;
                std::string removeOptionalQuotes(const std::string& text) const;
                std::string joinText(const std::vector<std::string>& values, const std::string& separator) const;
                std::string replaceAll(std::string text, const std::string& token, const std::string& value) const;
                std::string normalizeAnswer(std::string text) const;
                std::string buildConfiguredPath(const std::string& path) const;
                std::string resolveProjectFile(const std::string& projectDirectory, const std::string& path) const;
                std::string buildBindingDescription(const PatternAnalysis& analysis, const SentenceBinding& binding) const;
                std::string buildQuestionKey(
                    const SentencePatternData& pattern,
                    const PatternAnalysis& analysis,
                    const SentenceBinding& binding,
                    const ContextOptionData& contextOption,
                    const std::string& sourceForm,
                    const std::string& targetForm) const;
                void copyToBuffer(char* destination, size_t destinationSize, const std::string& source);

                tools::Logger& m_logger;

                char m_projectPath[512] = {};
                char m_userAnswer[1024] = {};
                char m_wordFilter[256] = {};

                bool m_showSettingsWindow = false;
                bool m_showQuizWindow = false;
                bool m_showStatistics = false;
                bool m_showFullAnswer = false;
                bool m_filesLoaded = false;
                bool m_waitingForNextQuestion = false;
                bool m_focusAnswerInput = true;

                bool m_enablePoliteToPlain = true;
                bool m_enablePlainToPolite = false;
                bool m_enablePoliteOutput = true;
                bool m_enablePlainOutput = true;

                bool m_enableNonPast = true;
                bool m_enableNegative = true;
                bool m_enablePast = true;
                bool m_enablePastNegative = true;
                bool m_randomOrder = true;

                int m_numberOfQuestions = 25;
                int m_numberOfTries = 2;
                int m_currentTry = 0;
                int m_autoAdvanceDelayMs = 2000;

                double m_autoAdvanceDueTime = 0.0;

                QuizMode m_quizMode = QuizMode::Transformation;
                int m_quizModeIndex = 0;

                uint32_t m_currentQuestionIndex = 0U;
                uint32_t m_correctAnswers = 0U;
                uint32_t m_wrongAnswers = 0U;
                uint32_t m_acceptedAnswers = 0U;

                std::string m_statusMessage;
                std::string m_feedbackMessage;
                std::string m_projectFullPath;
                std::string m_projectDirectory;

                std::unordered_map<std::string, std::vector<DictionaryWord>> m_dictionaries;
                std::unordered_map<std::string, TagProfile> m_tagProfiles;
                std::unordered_map<std::string, EntityData> m_entities;
                std::unordered_map<std::string, std::vector<std::string>> m_effectiveEntityTags;
                std::unordered_map<std::string, ContextPoolData> m_contexts;
                std::vector<SentencePatternData> m_sentencePatterns;
                std::vector<QuizQuestion> m_questions;
                FileStatistics m_statistics;

                mutable std::mt19937 m_rng;
            };
        }
    }
}
