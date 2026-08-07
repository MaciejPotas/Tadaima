#pragma once

#include "imgui.h"
#include "Tools/Logger.h"
#include "gui/widgets/packages/SettingsDataPackage.h"

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace pugi
{
    class xml_node;
}

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @class AvailabilitySentenceQuizWidget
             * @brief ImGui quiz for dictionary-form verb + noun + が + ある.
             *
             * The widget reads the Version 6 quiz project selected by
             * SettingsPackageKey::SentenceQuizSentenceMapPath. Grammar,
             * compatible complements, availability nouns and all ある forms
             * are taken from XML.
             *
             * Supported sentence pattern IDs:
             * - verb_noun_ga_aru_simple
             * - verb_noun_ga_aru_object
             * - verb_noun_ga_aru_destination
             * - verb_noun_ga_aru_companion
             * - verb_noun_ga_aru_place
             */
            class AvailabilitySentenceQuizWidget
            {
            public:
                explicit AvailabilitySentenceQuizWidget(tools::Logger& logger);

                void initialize(const tools::DataPackage& r_package);
                void draw(bool* p_open);

            private:
                enum class PatternKind
                {
                    Invalid,
                    Simple,
                    Object,
                    Destination,
                    Companion,
                    Place
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

                struct PatternValidation
                {
                    PatternKind kind = PatternKind::Invalid;
                    bool valid = false;
                    std::string error;
                };

                struct GenerationRule
                {
                    std::string formName;
                    std::string label;
                };

                /**
                 * One compact block of questions. Availability nouns,
                 * compatible complement entities and context options are
                 * unranked only when a question is selected.
                 */
                struct QuestionSpace
                {
                    const SentencePatternData* pattern = nullptr;
                    PatternKind kind = PatternKind::Invalid;
                    const DictionaryWord* mainVerb = nullptr;
                    const DictionaryWord* existenceVerb = nullptr;
                    const ContextPoolData* contextPool = nullptr;
                    GenerationRule rule;
                    std::vector<const EntityData*> availabilityNouns;
                    std::vector<const EntityData*> roleEntities;
                    uint64_t questionCount = 0U;
                };

                struct QuestionSeed
                {
                    const QuestionSpace* space = nullptr;
                    const EntityData* availabilityNoun = nullptr;
                    const EntityData* roleEntity = nullptr;
                    const ContextOptionData* contextOption = nullptr;
                };

                struct QuizQuestion
                {
                    std::string key;
                    std::string sourceSentence;
                    std::string targetSentence;
                    std::string targetLabel;
                    std::string patternId;
                    std::string mainVerbMeaning;
                    std::string mainVerbKana;
                    std::string availabilityNounKana;
                    std::string availabilityNounKanji;
                    std::string existenceForm;
                    std::string bindings;
                };

                struct FileStatistics
                {
                    uint32_t verbs = 0U;
                    uint32_t availabilityNouns = 0U;
                    uint32_t sentencePatterns = 0U;
                    uint32_t entities = 0U;
                    uint32_t tagProfiles = 0U;
                    uint64_t combinationSpaces = 0U;
                    uint64_t generatedQuestions = 0U;
                };

                void drawSettingsWindow(bool* p_open);
                void drawQuizWindow(bool* p_open);

                bool loadFiles();
                bool loadProject(const std::string& path);
                bool loadDictionary(
                    const std::string& dictionaryId,
                    const std::string& fullPath);
                bool loadVocabulary(const std::string& fullPath);
                bool loadContexts(const std::string& fullPath);
                bool loadSentencePatterns(const std::string& fullPath);
                bool buildEffectiveEntityTags();
                bool appendProfileTags(
                    const std::string& profileId,
                    std::vector<std::string>& recursionPath,
                    std::vector<std::string>& tags,
                    std::string& error) const;

                PatternValidation validatePattern(
                    const SentencePatternData& pattern) const;
                const SentencePatternData* findValidPattern(
                    const std::string& id) const;
                uint32_t countValidPatterns() const;
                std::string getPatternId(PatternKind kind) const;
                std::string getRoleName(PatternKind kind) const;
                bool isPatternEnabled(PatternKind kind) const;

                std::vector<GenerationRule> buildEnabledRules() const;
                std::vector<QuestionSpace> buildQuestionSpaces() const;
                std::vector<QuizQuestion> buildQuestions(
                    size_t maximumQuestions) const;
                bool seedFromGlobalIndex(
                    const std::vector<QuestionSpace>& spaces,
                    uint64_t globalIndex,
                    QuestionSeed& seed) const;
                bool renderQuestionSeed(
                    const QuestionSeed& seed,
                    QuizQuestion& question) const;
                uint64_t countPossibleQuestions() const;
                std::vector<uint64_t> chooseQuestionIndices(
                    uint64_t total,
                    size_t maximumQuestions) const;

                std::vector<const DictionaryWord*> getMainVerbCandidates(
                    PatternKind kind,
                    const std::vector<WordFilterRule>& filters) const;
                std::vector<const DictionaryWord*> getExistenceVerbs() const;
                std::vector<const EntityData*> getAvailabilityNouns() const;
                std::vector<const EntityData*> getCompatibleRoleEntities(
                    const DictionaryWord& word,
                    const std::string& role) const;
                bool entityMatchesRelation(
                    const EntityData& entity,
                    const DictionaryWord& word,
                    const std::string& role) const;

                bool renderPattern(
                    const QuestionSeed& seed,
                    std::string& question,
                    std::string& answer,
                    std::string& error) const;
                bool renderTemplate(
                    const std::string& text,
                    const QuestionSeed& seed,
                    std::string& rendered,
                    std::string& error) const;
                bool resolveToken(
                    const std::string& expression,
                    const QuestionSeed& seed,
                    std::string& value,
                    std::string& error) const;
                bool resolveWordToken(
                    const DictionaryWord& word,
                    const std::string& field,
                    const std::string& selectedForm,
                    bool dynamicForm,
                    std::string& value,
                    std::string& error) const;
                bool resolveEntityToken(
                    const EntityData& entity,
                    const std::string& field,
                    std::string& value,
                    std::string& error) const;
                const FormData* getForm(
                    const DictionaryWord& word,
                    const std::string& formName) const;
                const ContextPoolData* getContextPool(
                    const std::string& contextId) const;

                void startQuiz();
                void submitAnswer();
                void acceptCurrentAnswerAsCorrect();
                void skipQuestion();
                void advanceQuestion();
                void resetQuizState();
                void updateStatistics();
                void scheduleAutoAdvanceAfterCorrectAnswer();
                void processAutoAdvance();

                std::vector<WordFilterRule> parseWordFilter(
                    const std::string& filterText) const;
                bool wordMatchesFilter(
                    const DictionaryWord& word,
                    const std::vector<WordFilterRule>& rules) const;
                bool wordFieldMatches(
                    const DictionaryWord& word,
                    const std::string& field,
                    const std::string& value) const;
                bool wordHasTag(
                    const DictionaryWord& word,
                    const std::string& tag) const;
                bool tagsIntersect(
                    const std::vector<std::string>& first,
                    const std::vector<std::string>& second) const;
                bool containsValue(
                    const std::vector<std::string>& values,
                    const std::string& value) const;
                void appendUnique(
                    std::vector<std::string>& values,
                    const std::string& value) const;

                std::vector<std::string> extractTokens(
                    const std::string& text) const;
                void splitToken(
                    const std::string& expression,
                    std::string& root,
                    std::string& field) const;
                std::vector<std::string> splitText(
                    const std::string& text,
                    char separator) const;
                std::string trimText(const std::string& text) const;
                std::string removeOptionalQuotes(
                    const std::string& text) const;
                std::string replaceAll(
                    std::string text,
                    const std::string& token,
                    const std::string& value) const;
                std::string normalizeAnswer(std::string text) const;
                std::string buildConfiguredPath(
                    const std::string& path) const;
                std::string resolveProjectFile(
                    const std::string& projectDirectory,
                    const std::string& path) const;
                void copyToBuffer(
                    char* destination,
                    size_t destinationSize,
                    const std::string& source);

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

                bool m_enablePlainOutput = true;
                bool m_enablePoliteOutput = true;
                bool m_enableNonPast = true;
                bool m_enableNegative = true;
                bool m_enablePast = true;
                bool m_enablePastNegative = true;
                bool m_enableSimple = true;
                bool m_enableObject = true;
                bool m_enableDestination = true;
                bool m_enableCompanion = true;
                bool m_enablePlace = true;
                bool m_randomOrder = true;

                int m_numberOfQuestions = 25;
                int m_numberOfTries = 2;
                int m_currentTry = 0;
                int m_autoAdvanceDelayMs = 2000;
                double m_autoAdvanceDueTime = 0.0;

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
