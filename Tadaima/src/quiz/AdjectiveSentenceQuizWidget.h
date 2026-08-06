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
             * @class AdjectiveSentenceQuizWidget
             * @brief ImGui adjective quiz backed by a Version 6 quiz project.
             *
             * SettingsPackageKey::SentenceQuizSentenceMapPath points to
             * quiz_project.xml. The manifest resolves the adjective dictionary,
             * reusable vocabulary, contexts, and sentence patterns.
             *
             * This widget deliberately accepts only these pattern IDs:
             * - adjective_predicate
             * - adjective_simple
             * - adjective_attributive
             *
             * For a chain of two or three adjectives, all adjectives except the
             * last use their XML connective form. The final adjective uses a
             * complete predicate form after a topic or in the subjectless
             * simple pattern. The attributive pattern uses an XML copula
             * after the noun.
             */
            class AdjectiveSentenceQuizWidget
            {
            public:
                explicit AdjectiveSentenceQuizWidget(tools::Logger& logger);

                void initialize(const tools::DataPackage& r_package);
                void draw(bool* p_open);

            private:
                enum class QuizMode
                {
                    LanguageToJapanese = 0,
                    Transformation = 1
                };

                enum class FormCategory
                {
                    NonPast,
                    Negative,
                    Past,
                    PastNegative
                };

                enum class PatternKind
                {
                    Invalid,
                    Predicate,
                    Simple,
                    Attributive
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
                    FormCategory category = FormCategory::NonPast;
                    bool hasSourceForm = false;
                    bool sourcePolite = false;
                    bool targetPolite = false;
                    std::string label;
                };

                /**
                 * One compact block of questions. A block has an optional
                 * subject, one final adjective, an optional XML copula, and
                 * zero to two ordered prefix adjectives. It can therefore be
                 * counted and sampled without materializing every sentence.
                 */
                struct QuestionSpace
                {
                    const SentencePatternData* sourcePattern = nullptr;
                    const SentencePatternData* targetPattern = nullptr;
                    const EntityData* subject = nullptr;
                    const DictionaryWord* finalAdjective = nullptr;
                    const DictionaryWord* copula = nullptr;
                    std::vector<const DictionaryWord*> prefixCandidates;
                    const ContextPoolData* contextPool = nullptr;
                    GenerationRule rule;
                    std::string sourceForm;
                    std::string targetForm;
                    std::string sourceCopulaForm;
                    std::string targetCopulaForm;
                    uint64_t permutationCount = 0U;
                    uint64_t questionCount = 0U;
                };

                struct QuestionSeed
                {
                    const QuestionSpace* space = nullptr;
                    std::vector<const DictionaryWord*> adjectiveChain;
                    const ContextOptionData* contextOption = nullptr;
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
                    uint32_t adjectiveCount = 1U;
                };

                struct FileStatistics
                {
                    uint32_t adjectives = 0U;
                    uint32_t sentencePatterns = 0U;
                    uint32_t entities = 0U;
                    uint32_t tagProfiles = 0U;
                    uint64_t eligibleFinalChoices = 0U;
                    uint64_t combinationSpaces = 0U;
                    uint64_t generatedQuestions = 0U;
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

                PatternValidation validatePattern(const SentencePatternData& pattern) const;
                const SentencePatternData* findValidPattern(const std::string& id) const;
                uint32_t countValidPatterns() const;

                std::vector<GenerationRule> buildEnabledTransformationRules() const;
                std::vector<GenerationRule> buildEnabledMeaningToJapaneseRules() const;
                std::vector<QuestionSpace> buildQuestionSpaces(bool transformationMode) const;
                void appendPredicateSpaces(
                    std::vector<QuestionSpace>& spaces,
                    const std::vector<GenerationRule>& rules) const;
                void appendSimpleSpaces(
                    std::vector<QuestionSpace>& spaces,
                    bool transformationMode) const;
                void appendAttributiveSpaces(
                    std::vector<QuestionSpace>& spaces,
                    bool transformationMode) const;
                void appendSpacesForPatternPair(
                    std::vector<QuestionSpace>& spaces,
                    const SentencePatternData& sourcePattern,
                    const SentencePatternData& targetPattern,
                    const GenerationRule& rule,
                    const std::string& sourceForm,
                    const std::string& targetForm,
                    const DictionaryWord* copula,
                    const std::string& sourceCopulaForm,
                    const std::string& targetCopulaForm) const;

                std::vector<QuizQuestion> buildQuestionsForMode(
                    bool transformationMode,
                    size_t maximumQuestions) const;
                bool seedFromGlobalIndex(
                    const std::vector<QuestionSpace>& spaces,
                    uint64_t globalIndex,
                    QuestionSeed& seed) const;
                std::vector<const DictionaryWord*> unrankPrefixPermutation(
                    const std::vector<const DictionaryWord*>& candidates,
                    size_t prefixCount,
                    uint64_t rank) const;
                bool renderQuestionSeed(
                    const QuestionSeed& seed,
                    bool transformationMode,
                    QuizQuestion& question) const;
                uint64_t countPossibleQuestions(bool transformationMode) const;
                uint64_t permutationCount(size_t candidateCount, size_t selectedCount) const;
                std::vector<uint64_t> chooseQuestionIndices(
                    uint64_t total,
                    size_t maximumQuestions) const;

                bool renderPattern(
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
                    std::string& error) const;
                bool renderTemplate(
                    const std::string& text,
                    const EntityData* subject,
                    const std::vector<const DictionaryWord*>& adjectiveChain,
                    const std::string& finalForm,
                    const DictionaryWord* copula,
                    const std::string& copulaForm,
                    const ContextPoolData& contextPool,
                    const ContextOptionData& contextOption,
                    std::string& rendered,
                    std::string& error) const;
                bool resolveToken(
                    const std::string& expression,
                    const EntityData* subject,
                    const std::vector<const DictionaryWord*>& adjectiveChain,
                    const std::string& finalForm,
                    const DictionaryWord* copula,
                    const std::string& copulaForm,
                    const ContextPoolData& contextPool,
                    const ContextOptionData& contextOption,
                    std::string& value,
                    std::string& error) const;
                bool resolveCopulaToken(
                    const DictionaryWord* copula,
                    const EntityData* subject,
                    const std::string& field,
                    const std::string& copulaForm,
                    std::string& value,
                    std::string& error) const;
                bool resolveSubjectToken(
                    const EntityData& subject,
                    const std::string& field,
                    std::string& value,
                    std::string& error) const;
                bool resolveAdjectiveToken(
                    const EntityData* subject,
                    const std::vector<const DictionaryWord*>& adjectiveChain,
                    const std::string& field,
                    const std::string& finalForm,
                    std::string& value,
                    std::string& error) const;
                std::string buildCombinedJapaneseForm(
                    const std::vector<const DictionaryWord*>& adjectiveChain,
                    const std::string& finalForm) const;
                std::string buildCombinedPrompt(
                    const EntityData* subject,
                    const std::vector<const DictionaryWord*>& adjectiveChain,
                    const std::string& finalForm) const;
                std::string buildChainInfo(
                    const std::vector<const DictionaryWord*>& adjectiveChain,
                    const std::string& field) const;
                std::string buildQuestionKey(
                    const QuestionSeed& seed,
                    const std::string& sourceForm,
                    const std::string& targetForm) const;

                std::string getFiniteFormName(FormCategory category, bool polite) const;
                std::string getCopulaFormName(FormCategory category, bool polite) const;
                const FormData* getForm(
                    const DictionaryWord& word,
                    const std::string& formName) const;
                std::string getFormPrompt(
                    const FormData& form,
                    const std::string& gender) const;
                const ContextPoolData* getContextPool(const std::string& contextId) const;

                std::vector<const DictionaryWord*> getCompatibleAdjectives(
                    const EntityData& entity,
                    bool requireConnective,
                    const std::vector<WordFilterRule>& filters) const;
                bool isAdjectiveTypeEnabled(const DictionaryWord& adjective) const;
                bool entityMatchesAdjective(
                    const EntityData& entity,
                    const DictionaryWord& adjective) const;

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
                bool wordMatchesFilter(
                    const DictionaryWord& word,
                    const std::vector<WordFilterRule>& rules) const;
                bool wordFieldMatches(
                    const DictionaryWord& word,
                    const std::string& field,
                    const std::string& value) const;
                bool wordHasTag(const DictionaryWord& word, const std::string& tag) const;
                bool tagsIntersect(
                    const std::vector<std::string>& first,
                    const std::vector<std::string>& second) const;
                bool containsValue(
                    const std::vector<std::string>& values,
                    const std::string& value) const;
                void appendUnique(std::vector<std::string>& values, const std::string& value) const;

                std::vector<std::string> extractTokens(const std::string& text) const;
                void splitToken(
                    const std::string& expression,
                    std::string& root,
                    std::string& field) const;
                std::vector<std::string> splitText(
                    const std::string& text,
                    char separator) const;
                std::string trimText(const std::string& text) const;
                std::string removeOptionalQuotes(const std::string& text) const;
                std::string joinText(
                    const std::vector<std::string>& values,
                    const std::string& separator) const;
                std::string replaceAll(
                    std::string text,
                    const std::string& token,
                    const std::string& value) const;
                std::string normalizeAnswer(std::string text) const;
                std::string buildConfiguredPath(const std::string& path) const;
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

                bool m_enablePoliteToPlain = true;
                bool m_enablePlainToPolite = false;
                bool m_enablePoliteOutput = true;
                bool m_enablePlainOutput = true;
                bool m_enablePredicateSentences = true;
                bool m_enableSimpleSentences = true;
                bool m_enableAttributiveSentences = true;
                bool m_enableIAdjectives = true;
                bool m_enableNaAdjectives = true;
                bool m_enableNonPast = true;
                bool m_enableNegative = true;
                bool m_enablePast = true;
                bool m_enablePastNegative = true;
                bool m_randomOrder = true;

                int m_numberOfQuestions = 25;
                int m_numberOfAdjectives = 1;
                int m_numberOfTries = 2;
                int m_currentTry = 0;
                int m_autoAdvanceDelayMs = 2000;

                double m_autoAdvanceDueTime = 0.0;

                QuizMode m_quizMode = QuizMode::LanguageToJapanese;
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
