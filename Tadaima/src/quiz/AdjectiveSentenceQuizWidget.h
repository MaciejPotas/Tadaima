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
             * @brief ImGui widget for practicing generated Japanese adjective sentence forms.
             *
             * The widget loads Japanese adjectives from a dictionary XML file and sentence templates
             * from a sentence map XML file. It can generate quiz questions in two modes:
             * - transforming one Japanese sentence form into another,
             * - writing a Japanese sentence from a source-language prompt.
             *
             * The widget supports both i-adjectives and na-adjectives. It can generate predicate
             * sentences where the adjective appears after the topic noun, for example
             * このテストはむずかしいです, and attributive noun sentences where the adjective appears
             * before the noun, for example これはむずかしいテストです.
             *
             * The dictionary XML is expected to contain forms such as:
             * @code
             * predicate_plain_nonpast
             * predicate_polite_nonpast
             * predicate_plain_negative
             * predicate_polite_negative
             * predicate_plain_past
             * predicate_polite_past
             * predicate_plain_past_negative
             * predicate_polite_past_negative
             * attributive_nonpast
             * connective
             * @endcode
             *
             * When more than one adjective per sentence is selected, every adjective before
             * the final adjective uses the connective form. For example, i-adjectives can use
             * むずかしくて and na-adjectives can use しずかで.
             *
             * The widget supports filtering words by XML attributes, for example:
             * @code
             * type=i_adjective
             * type=na_adjective
             * tag=test_description
             * type=i_adjective;tag=test_description
             * @endcode
             */
            class AdjectiveSentenceQuizWidget
            {
            public:
                /**
                 * @brief Creates the adjective sentence quiz widget.
                 * @param logger Logger used for diagnostic messages.
                 */
                explicit AdjectiveSentenceQuizWidget(tools::Logger& logger);

                /**
                 * @brief Initializes dictionary path, sentence map path, and quiz settings.
                 * @param r_package Application settings package.
                 */
                void initialize(const tools::DataPackage& r_package);

                /**
                 * @brief Draws the active widget window.
                 * @param p_open Pointer controlling visibility of the widget.
                 */
                void draw(bool* p_open);

            private:
                /**
                 * @enum QuizMode
                 * @brief Defines how quiz questions are generated.
                 */
                enum class QuizMode
                {
                    LanguageToJapanese = 0,     ///< Write a Japanese sentence from a source-language prompt.
                    Transformation = 1          ///< Transform one Japanese sentence form into another.

                };

                /**
                 * @enum SentencePlacement
                 * @brief Defines where the adjective is used in the generated sentence.
                 */
                enum class SentencePlacement
                {
                    Predicate = 0,   ///< The adjective appears after the topic noun, for example このテストはむずかしいです.
                    Attributive = 1  ///< The adjective appears before the noun, for example これはむずかしいテストです.
                };

                /**
                 * @struct FormData
                 * @brief Stores one adjective form loaded from XML.
                 */
                struct FormData
                {
                    std::string name;           ///< Form identifier, for example predicate_polite_nonpast.
                    std::string style;          ///< Optional style description from XML.
                    std::string sentencePrompt; ///< Prompt form used inside sentence templates.
                    std::string value;          ///< Actual Japanese form value.
                };

                /**
                 * @struct DictionaryAdjective
                 * @brief Stores one adjective loaded from XML.
                 */
                struct DictionaryAdjective
                {
                    std::string translation;                              ///< Source-language translation.
                    std::string kana;                                     ///< Kana form of the adjective.
                    std::string kanji;                                    ///< Kanji form of the adjective, if available.
                    std::string type;                                     ///< Adjective type, for example i_adjective or na_adjective.
                    std::string ending;                                   ///< Optional ending attribute.
                    std::vector<std::string> tags;                        ///< Tags used to match sentence maps.
                    std::unordered_map<std::string, FormData> forms;      ///< Forms indexed by name.
                    std::unordered_map<std::string, std::string> attributes; ///< All XML attributes indexed by name.
                };

                /**
                 * @struct WordFilterRule
                 * @brief One parsed filter rule from the filter input field.
                 */
                struct WordFilterRule
                {
                    std::string field;               ///< Attribute name or special field name, for example tag.
                    std::vector<std::string> values; ///< Accepted values for this field.
                };

                /**
                 * @struct SlotItem
                 * @brief One slot replacement item used by sentence templates.
                 */
                struct SlotItem
                {
                    std::string text; ///< Source-language slot text.
                    std::string jp;   ///< Japanese slot text.
                };

                /**
                 * @struct SentenceMap
                 * @brief Sentence template definition loaded from the sentence map XML file.
                 */
                struct SentenceMap
                {
                    std::string id;                         ///< Unique sentence map identifier.
                    std::string label;                      ///< Human-readable label.
                    std::string requiredTag;                ///< Required dictionary adjective tag.
                    SentencePlacement placement = SentencePlacement::Predicate; ///< Sentence placement used by this template.
                    std::string slotList;                   ///< Optional slot list identifier.
                    std::string questionTemplate;           ///< Source-language question template.
                    std::string answerTemplate;             ///< Japanese answer template.
                };

                /**
                 * @struct AdjectiveRule
                 * @brief Rule used to generate one sentence form variant.
                 */
                struct AdjectiveRule
                {
                    SentencePlacement placement = SentencePlacement::Predicate; ///< Placement where the rule is valid.
                    std::string sourceForm;                                      ///< Source form name.
                    std::string targetForm;                                      ///< Target form name.
                    std::string sourceCopula;                                    ///< Source copula for noun sentences.
                    std::string targetCopula;                                    ///< Target copula for noun sentences.
                    std::string label;                                           ///< Label shown to the user.
                    bool enabled = false;                                        ///< Indicates whether this rule is active.
                };

                /**
                 * @struct AdjectiveChain
                 * @brief Stores adjectives used together in one generated sentence.
                 *
                 * The last adjective in the chain is the adjective that receives the selected
                 * target form. All previous adjectives use the connective form from XML.
                 */
                struct AdjectiveChain
                {
                    std::vector<const DictionaryAdjective*> words; ///< Adjectives used in the generated sentence.
                };

                /**
                 * @struct QuizQuestion
                 * @brief Fully generated quiz question.
                 */
                struct QuizQuestion
                {
                    std::string key;                 ///< Unique generated question key.
                    std::string sourceSentence;      ///< Sentence shown to the user.
                    std::string targetSentence;      ///< Expected answer.
                    std::string questionText;        ///< Additional context or source-language prompt.
                    std::string targetLabel;         ///< Target form label shown to the user.
                    std::string sourceForm;          ///< Source adjective form name.
                    std::string targetForm;          ///< Target adjective form name.
                    std::string sourceCopula;        ///< Source copula for noun sentences.
                    std::string targetCopula;        ///< Target copula for noun sentences.
                    std::string wordMeaning;         ///< Adjective translation.
                    std::string wordKana;            ///< Adjective kana.
                    std::string wordKanji;           ///< Adjective kanji.
                    std::string wordType;            ///< Adjective type.
                    std::string mapId;               ///< Sentence map identifier.
                    std::string slotText;            ///< Source-language slot text.
                    std::string slotJp;              ///< Japanese slot text.
                    SentencePlacement placement = SentencePlacement::Predicate; ///< Placement used by this question.
                    uint32_t adjectiveCount = 1U; ///< Number of adjectives used in this question.
                };

                /**
                 * @struct FileStatistics
                 * @brief Statistics calculated from loaded XML files and current settings.
                 */
                struct FileStatistics
                {
                    uint32_t words = 0U;              ///< Number of loaded adjectives.
                    uint32_t sentenceMaps = 0U;       ///< Number of loaded sentence maps.
                    uint32_t slotLists = 0U;          ///< Number of loaded slot lists.
                    uint32_t slotItems = 0U;          ///< Number of loaded slot items.
                    uint32_t generatedQuestions = 0U; ///< Number of questions generated with current settings.
                };

                /**
                 * @brief Draws the quiz settings window.
                 * @param p_open Pointer controlling widget visibility.
                 */
                void drawSettingsWindow(bool* p_open);

                /**
                 * @brief Draws the active quiz window.
                 * @param p_open Pointer controlling widget visibility.
                 */
                void drawQuizWindow(bool* p_open);

                /**
                 * @brief Loads both dictionary and sentence map XML files.
                 * @return True if both files were loaded successfully, otherwise false.
                 */
                bool loadFiles();

                /**
                 * @brief Loads adjective words from an XML file.
                 * @param path Relative dictionary XML path.
                 * @return True if at least one valid adjective was loaded, otherwise false.
                 */
                bool loadDictionary(const std::string& path);

                /**
                 * @brief Loads forms from a word XML node.
                 * @param wordNode XML node that represents a word.
                 * @param r_word Word object that receives loaded forms.
                 */
                void loadFormsFromWordNode(const pugi::xml_node& wordNode, DictionaryAdjective& r_word);

                /**
                 * @brief Loads sentence maps and slot lists from an XML file.
                 * @param path Relative sentence map XML path.
                 * @return True if at least one valid sentence map was loaded, otherwise false.
                 */
                bool loadSentenceMaps(const std::string& path);

                /**
                 * @brief Builds enabled rules for Japanese sentence transformation mode.
                 * @return Vector of active transformation rules.
                 */
                std::vector<AdjectiveRule> buildEnabledTransformationRules() const;

                /**
                 * @brief Builds enabled rules for source-language-to-Japanese mode.
                 * @return Vector of active source-language-to-Japanese rules.
                 */
                std::vector<AdjectiveRule> buildEnabledMeaningToJapaneseRules() const;

                /**
                 * @brief Builds enabled rules for one placement and one output direction.
                 * @param r_rules Vector receiving generated rules.
                 * @param placement Sentence placement.
                 * @param plainOutput Indicates whether plain output rules should be generated.
                 * @param politeOutput Indicates whether polite output rules should be generated.
                 */
                void appendMeaningRulesForPlacement(std::vector<AdjectiveRule>& r_rules, SentencePlacement placement, bool plainOutput, bool politeOutput) const;

                /**
                 * @brief Builds adjective chains for one final adjective and one sentence map.
                 * @param targetWord Final adjective in the generated sentence.
                 * @param sentenceMap Sentence map used to match tags and placement.
                 * @param wordFilterRules Active word filter rules.
                 * @param finalFormName Form required by the final adjective.
                 * @return Generated adjective chains.
                 */
                std::vector<AdjectiveChain> buildAdjectiveChains(const DictionaryAdjective& targetWord, const SentenceMap& sentenceMap, const std::vector<WordFilterRule>& wordFilterRules, const std::string& finalFormName) const;

                /**
                 * @brief Builds a Japanese adjective form sequence for a chain.
                 * @param chain Adjective chain.
                 * @param finalFormName Form used by the final adjective.
                 * @return Combined Japanese adjective form text.
                 */
                std::string buildCombinedForm(const AdjectiveChain& chain, const std::string& finalFormName) const;

                /**
                 * @brief Builds a source-language prompt sequence for a chain.
                 * @param chain Adjective chain.
                 * @param finalFormName Form used by the final adjective.
                 * @return Combined source-language prompt text.
                 */
                std::string buildCombinedPrompt(const AdjectiveChain& chain, const std::string& finalFormName) const;

                /**
                 * @brief Builds a stable key fragment for an adjective chain.
                 * @param chain Adjective chain.
                 * @return Key fragment containing all adjective kana values.
                 */
                std::string buildChainKey(const AdjectiveChain& chain) const;

                /**
                 * @brief Builds a readable helper text from adjective chain metadata.
                 * @param chain Adjective chain.
                 * @param field Metadata field to join, for example translation, kana, kanji, or type.
                 * @return Joined metadata text.
                 */
                std::string buildChainInfo(const AdjectiveChain& chain, const std::string& field) const;

                /**
                 * @brief Builds the question bank for Japanese sentence transformation mode.
                 * @return Generated quiz questions.
                 */
                std::vector<QuizQuestion> buildTransformationQuestionBank() const;

                /**
                 * @brief Builds the question bank for source-language-to-Japanese mode.
                 * @return Generated quiz questions.
                 */
                std::vector<QuizQuestion> buildMeaningToJapaneseQuestionBank() const;

                /**
                 * @brief Loads files, generates questions, and starts the quiz.
                 */
                void startQuiz();

                /**
                 * @brief Checks the current user answer against the expected answer.
                 */
                void submitAnswer();

                /**
                 * @brief Accepts the current answer manually as correct.
                 */
                void acceptCurrentAnswerAsCorrect();

                /**
                 * @brief Skips the current question and counts it as manually accepted.
                 */
                void skipQuestion();

                /**
                 * @brief Moves to the next question and resets per-question state.
                 */
                void advanceQuestion();

                /**
                 * @brief Resets all quiz counters and per-question state.
                 */
                void resetQuizState();

                /**
                 * @brief Reloads files and recalculates statistics for current settings.
                 */
                void updateStatistics();

                /**
                 * @brief Schedules automatic advance after a correct answer.
                 */
                void scheduleAutoAdvanceAfterCorrectAnswer();

                /**
                 * @brief Advances to the next question when the auto-advance delay has passed.
                 */
                void processAutoAdvance();

                /**
                 * @brief Parses the word filter text into filter rules.
                 * @param filterText Filter string entered by the user.
                 * @return Parsed filter rules.
                 */
                std::vector<WordFilterRule> parseWordFilter(const std::string& filterText) const;

                /**
                 * @brief Checks whether a word matches all filter rules.
                 * @param word Dictionary adjective to check.
                 * @param rules Parsed filter rules.
                 * @return True if the word matches all rules, otherwise false.
                 */
                bool wordMatchesFilter(const DictionaryAdjective& word, const std::vector<WordFilterRule>& rules) const;

                /**
                 * @brief Checks whether a single word field matches a filter value.
                 * @param word Dictionary adjective to check.
                 * @param field Attribute name or special field name.
                 * @param value Expected value.
                 * @return True if the field matches the value, otherwise false.
                 */
                bool wordFieldMatches(const DictionaryAdjective& word, const std::string& field, const std::string& value) const;

                /**
                 * @brief Splits text by a separator character.
                 * @param text Text to split.
                 * @param separator Separator character.
                 * @return Vector of split parts.
                 */
                std::vector<std::string> splitText(const std::string& text, char separator) const;

                /**
                 * @brief Removes leading and trailing spaces and tabs.
                 * @param text Text to trim.
                 * @return Trimmed text.
                 */
                std::string trimText(const std::string& text) const;

                /**
                 * @brief Removes optional single or double quotes around text.
                 * @param text Text that may be quoted.
                 * @return Text without surrounding quotes.
                 */
                std::string removeOptionalQuotes(const std::string& text) const;

                /**
                 * @brief Builds a full file path relative to the executable path.
                 * @param path Relative file path.
                 * @return Full file path.
                 */
                std::string buildFullPath(const std::string& path) const;

                /**
                 * @brief Renders a sentence template by replacing supported tokens.
                 * @param text Template text.
                 * @param replacements Replacement values indexed by token name, including braces.
                 * @return Rendered sentence.
                 */
                std::string renderTemplate(const std::string& text, const std::unordered_map<std::string, std::string>& replacements) const;

                /**
                 * @brief Replaces all occurrences of a token in text.
                 * @param text Input text.
                 * @param token Token to replace.
                 * @param value Replacement value.
                 * @return Text after replacement.
                 */
                std::string replaceAll(std::string text, const std::string& token, const std::string& value) const;

                /**
                 * @brief Normalizes an answer before comparison.
                 * @param text Answer text.
                 * @return Normalized answer text.
                 */
                std::string normalizeAnswer(std::string text) const;

                /**
                 * @brief Checks whether a word has a required tag.
                 * @param word Dictionary adjective.
                 * @param tag Required tag.
                 * @return True if the word has the tag, otherwise false.
                 */
                bool hasTag(const DictionaryAdjective& word, const std::string& tag) const;

                /**
                 * @brief Checks whether a word has a non-empty form.
                 * @param word Dictionary adjective.
                 * @param formName Form name.
                 * @return True if the form exists and has a value, otherwise false.
                 */
                bool hasForm(const DictionaryAdjective& word, const std::string& formName) const;

                /**
                 * @brief Gets an adjective form value by name.
                 * @param word Dictionary adjective.
                 * @param formName Form name.
                 * @return Form value, or an empty string if not found.
                 */
                std::string getFormValue(const DictionaryAdjective& word, const std::string& formName) const;

                /**
                 * @brief Gets the sentence prompt for an adjective form.
                 * @param word Dictionary adjective.
                 * @param formName Form name.
                 * @return Sentence prompt, or word translation if no prompt is configured.
                 */
                std::string getSentencePrompt(const DictionaryAdjective& word, const std::string& formName) const;

                /**
                 * @brief Builds template replacements for one generated question.
                 * @param word Dictionary adjective.
                 * @param slot Slot item.
                 * @param formValue Adjective form value.
                 * @param copula Copula used by noun sentences.
                 * @param prompt Source-language prompt.
                 * @return Replacement map indexed by token name, including braces.
                 */
                std::unordered_map<std::string, std::string> buildReplacements(const DictionaryAdjective& word, const SlotItem& slot, const std::string& formValue, const std::string& copula, const std::string& prompt) const;


                /**
                 * @brief Converts a noun sentence copula into a source-language helper prompt.
                 * @param copula Japanese copula or negative copula expression.
                 * @return Source-language helper prompt.
                 */
                std::string copulaToPrompt(const std::string& copula) const;

                /**
                 * @brief Parses a placement string from XML.
                 * @param text Placement text.
                 * @return Parsed placement value.
                 */
                SentencePlacement parsePlacement(const std::string& text) const;

                /**
                 * @brief Converts sentence placement to a readable label.
                 * @param placement Placement value.
                 * @return Readable placement label.
                 */
                std::string placementToText(SentencePlacement placement) const;

                /**
                 * @brief Checks whether a placement is currently enabled in settings.
                 * @param placement Placement value.
                 * @return True if placement is enabled, otherwise false.
                 */
                bool isPlacementEnabled(SentencePlacement placement) const;

                /**
                 * @brief Checks whether an adjective type is currently enabled in settings.
                 * @param word Dictionary adjective.
                 * @return True if adjective type is enabled, otherwise false.
                 */
                bool isAdjectiveTypeEnabled(const DictionaryAdjective& word) const;

                /**
                 * @brief Copies a string into a fixed-size character buffer.
                 * @param destination Destination buffer.
                 * @param destinationSize Destination buffer size.
                 * @param source Source string.
                 */
                void copyToBuffer(char* destination, size_t destinationSize, const std::string& source);

                tools::Logger& m_logger; ///< Logger used for diagnostics.

                char m_dictionaryPath[512] = {}; ///< Dictionary XML path buffer.
                char m_sentenceMapPath[512] = {}; ///< Sentence map XML path buffer.
                char m_userAnswer[1024] = {}; ///< Current answer input buffer.
                char m_wordFilter[256] = {}; ///< Word filter input buffer.

                bool m_showSettingsWindow = false; ///< Indicates whether settings window is visible.
                bool m_showQuizWindow = false; ///< Indicates whether quiz window is visible.
                bool m_showStatistics = false; ///< Indicates whether statistics are visible.
                bool m_showFullAnswer = false; ///< Indicates whether full answer help is visible.
                bool m_filesLoaded = false; ///< Indicates whether XML files are loaded.
                bool m_waitingForNextQuestion = false; ///< Indicates whether current question is already answered.
                bool m_focusAnswerInput = true; ///< Requests keyboard focus for the answer input field.

                bool m_enablePoliteToPlain = true; ///< Enables polite-to-plain transformation rules.
                bool m_enablePlainToPolite = false; ///< Enables plain-to-polite transformation rules.
                bool m_enablePoliteOutput = true; ///< Enables polite output in source-language-to-Japanese mode.
                bool m_enablePlainOutput = true; ///< Enables plain output in source-language-to-Japanese mode.

                bool m_enablePredicateSentences = true; ///< Enables sentences where the adjective appears after the topic noun.
                bool m_enableAttributiveSentences = true; ///< Enables sentences where the adjective appears before the noun.
                bool m_enableIAdjectives = true; ///< Enables i-adjectives.
                bool m_enableNaAdjectives = true; ///< Enables na-adjectives.

                bool m_enableNonPast = true; ///< Enables non-past affirmative forms.
                bool m_enableNegative = true; ///< Enables non-past negative forms.
                bool m_enablePast = true; ///< Enables past affirmative forms.
                bool m_enablePastNegative = true; ///< Enables past negative forms.
                bool m_randomOrder = true; ///< Enables random question order.

                int m_numberOfQuestions = 25; ///< Requested number of quiz questions.
                int m_numberOfAdjectives = 1; ///< Number of adjectives used in one generated sentence.
                int m_numberOfTries = 2; ///< Number of tries before answer is treated as wrong.
                int m_currentTry = 0; ///< Current try counter for active question.
                int m_autoAdvanceDelayMs = 2000; ///< Delay before automatic advance after correct answer.

                double m_autoAdvanceDueTime = 0.0; ///< ImGui time when automatic advance should happen.

                QuizMode m_quizMode = QuizMode::LanguageToJapanese; ///< Current quiz mode.
                int m_quizModeIndex = 0; ///< ImGui combo index for quiz mode.

                uint32_t m_currentQuestionIndex = 0U; ///< Index of the active question.
                uint32_t m_correctAnswers = 0U; ///< Number of correct answers.
                uint32_t m_wrongAnswers = 0U; ///< Number of wrong answers.
                uint32_t m_acceptedAnswers = 0U; ///< Number of manually accepted answers.

                std::string m_statusMessage; ///< Status message shown in settings window.
                std::string m_feedbackMessage; ///< Feedback message shown in quiz window.

                std::vector<DictionaryAdjective> m_words; ///< Loaded adjective words.
                std::vector<SentenceMap> m_sentenceMaps; ///< Loaded sentence maps.
                std::unordered_map<std::string, std::vector<SlotItem>> m_slotLists; ///< Loaded slot lists indexed by ID.
                std::vector<QuizQuestion> m_questions; ///< Generated quiz questions.
                FileStatistics m_statistics; ///< Current file and generation statistics.

                std::mt19937 m_rng; ///< Random generator used for question shuffling.
            };
        }
    }
}
