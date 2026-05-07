#pragma once

#include "imgui.h"
#include "Tools/Logger.h"
#include "gui/widgets/packages/SettingsDataPackage.h"

#include <cstdint>
#include <cstddef>
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
             * @brief ImGui widget for practicing generated Japanese sentence transformations.
             *
             * The widget loads Japanese words from a dictionary XML file and sentence templates
             * from a sentence map XML file. It can generate quiz questions in two modes:
             * - transforming one Japanese sentence form into another,
             * - writing a Japanese sentence from a source-language prompt.
             *
             * The widget supports filtering words by XML attributes, for example:
             * @code
             * ending=su
             * ending=su,ku,kuru
             * type=godan;ending=su,ku
             * tag=eat_food
             * @endcode
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
                    Transformation = 0,      ///< Transform one Japanese sentence form into another.
                    LanguageToJapanese = 1   ///< Write a Japanese sentence from a source-language prompt.
                };

                /**
                 * @struct ConjugationData
                 * @brief Stores one conjugated form of a dictionary word.
                 */
                struct ConjugationData
                {
                    std::string name;             ///< Conjugation identifier, for example "dictionary" or "past_plain".
                    std::string style;            ///< Optional style description from XML.
                    std::string sentencePrompt;   ///< Prompt form used inside sentence templates.
                    std::string value;            ///< Actual conjugated value.
                };

                /**
                 * @struct DictionaryWord
                 * @brief Stores one dictionary word loaded from XML.
                 */
                struct DictionaryWord
                {
                    std::string translation;                                      ///< Source-language translation.
                    std::string kana;                                             ///< Kana form of the word.
                    std::string kanji;                                            ///< Kanji form of the word, if available.
                    std::string type;                                             ///< Word type, for example verb type.
                    std::string ending;                                           ///< Word ending, useful for filtering.
                    std::vector<std::string> tags;                                ///< Tags used to match sentence maps.
                    std::unordered_map<std::string, ConjugationData> conjugations; ///< Conjugations indexed by name.
                    std::unordered_map<std::string, std::string> attributes;       ///< All XML attributes indexed by name.
                };

                /**
                 * @struct WordFilterRule
                 * @brief One parsed filter rule from the filter input field.
                 */
                struct WordFilterRule
                {
                    std::string field;                ///< Attribute name or special field name, for example "tag".
                    std::vector<std::string> values;  ///< Accepted values for this field.
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
                    std::string id;               ///< Unique sentence map identifier.
                    std::string label;            ///< Human-readable label.
                    std::string requiredTag;      ///< Required dictionary word tag.
                    std::string conjugation;      ///< Conjugation used by this template.
                    std::string slotList;         ///< Optional slot list identifier.
                    std::string questionTemplate; ///< Source-language question template.
                    std::string answerTemplate;   ///< Japanese answer template.
                };

                /**
                 * @struct TransformationRule
                 * @brief Rule used in Japanese-to-Japanese transformation mode.
                 */
                struct TransformationRule
                {
                    std::string sourceConjugation;   ///< Source conjugation name.
                    std::string targetConjugation;   ///< Expected target conjugation name.
                    std::string templateConjugation; ///< Sentence map conjugation required by this rule.
                    std::string label;               ///< Label shown to the user.
                    bool enabled = false;            ///< Indicates whether this rule is active.
                };

                /**
                 * @struct MeaningToJapaneseRule
                 * @brief Rule used in source-language-to-Japanese mode.
                 */
                struct MeaningToJapaneseRule
                {
                    std::string targetConjugation;   ///< Expected target conjugation name.
                    std::string templateConjugation; ///< Sentence map conjugation required by this rule.
                    std::string label;               ///< Label shown to the user.
                    bool enabled = false;            ///< Indicates whether this rule is active.
                };

                /**
                 * @struct QuizQuestion
                 * @brief Fully generated quiz question.
                 */
                struct QuizQuestion
                {
                    std::string key;                  ///< Unique generated question key.
                    std::string sourceSentence;       ///< Sentence shown to the user.
                    std::string targetSentence;       ///< Expected answer.
                    std::string questionText;         ///< Additional context or source-language prompt.
                    std::string targetLabel;          ///< Target form label shown to the user.
                    std::string sourceConjugation;    ///< Source conjugation name.
                    std::string targetConjugation;    ///< Target conjugation name.
                    std::string templateConjugation;  ///< Template conjugation name.
                    std::string wordMeaning;          ///< Word translation.
                    std::string wordKana;             ///< Word kana.
                    std::string wordKanji;            ///< Word kanji.
                    std::string mapId;                ///< Sentence map identifier.
                    std::string slotText;             ///< Source-language slot text.
                    std::string slotJp;               ///< Japanese slot text.
                };

                /**
                 * @struct FileStatistics
                 * @brief Statistics calculated from loaded XML files and current settings.
                 */
                struct FileStatistics
                {
                    uint32_t words = 0U;              ///< Number of loaded words.
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
                 * @brief Loads dictionary words from an XML file.
                 * @param path Relative dictionary XML path.
                 * @return True if at least one valid word was loaded, otherwise false.
                 */
                bool loadDictionary(const std::string& path);

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
                std::vector<TransformationRule> buildEnabledRules() const;

                /**
                 * @brief Builds enabled rules for source-language-to-Japanese mode.
                 * @return Vector of active source-language-to-Japanese rules.
                 */
                std::vector<MeaningToJapaneseRule> buildEnabledMeaningToJapaneseRules() const;

                /**
                 * @brief Builds the question bank for Japanese sentence transformation mode.
                 * @return Generated quiz questions.
                 */
                std::vector<QuizQuestion> buildQuestionBank() const;

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
                 * @param word Dictionary word to check.
                 * @param rules Parsed filter rules.
                 * @return True if the word matches all rules, otherwise false.
                 */
                bool wordMatchesFilter(const DictionaryWord& word, const std::vector<WordFilterRule>& rules) const;

                /**
                 * @brief Checks whether a single word field matches a filter value.
                 * @param word Dictionary word to check.
                 * @param field Attribute name or special field name.
                 * @param value Expected value.
                 * @return True if the field matches the value, otherwise false.
                 */
                bool wordFieldMatches(const DictionaryWord& word, const std::string& field, const std::string& value) const;

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
                 * @param slot Slot item used for {slot} and {slot_jp}.
                 * @param conjugation Conjugation value used for {conjugation}.
                 * @param verbPrompt Prompt value used for {verb_prompt}.
                 * @return Rendered sentence.
                 */
                std::string renderTemplate(const std::string& text, const SlotItem& slot, const std::string& conjugation, const std::string& verbPrompt) const;

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
                 * @param word Dictionary word.
                 * @param tag Required tag.
                 * @return True if the word has the tag, otherwise false.
                 */
                bool hasTag(const DictionaryWord& word, const std::string& tag) const;

                /**
                 * @brief Checks whether a word has a non-empty conjugation.
                 * @param word Dictionary word.
                 * @param conjugationName Conjugation name.
                 * @return True if the conjugation exists and has a value, otherwise false.
                 */
                bool hasConjugation(const DictionaryWord& word, const std::string& conjugationName) const;

                /**
                 * @brief Gets a conjugation value by name.
                 * @param word Dictionary word.
                 * @param conjugationName Conjugation name.
                 * @return Conjugation value, or an empty string if not found.
                 */
                std::string getConjugationValue(const DictionaryWord& word, const std::string& conjugationName) const;

                /**
                 * @brief Gets the sentence prompt for a conjugation.
                 * @param word Dictionary word.
                 * @param conjugationName Conjugation name.
                 * @return Sentence prompt, or word kana if no prompt is configured.
                 */
                std::string getSentencePrompt(const DictionaryWord& word, const std::string& conjugationName) const;

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

                bool m_enableNonPast = true; ///< Enables non-past affirmative forms.
                bool m_enableNegative = true; ///< Enables non-past negative forms.
                bool m_enablePast = true; ///< Enables past affirmative forms.
                bool m_enablePastNegative = true; ///< Enables past negative forms.
                bool m_randomOrder = true; ///< Enables random question order.

                int m_numberOfQuestions = 25; ///< Requested number of quiz questions.
                int m_numberOfTries = 2; ///< Number of tries before answer is treated as wrong.
                int m_currentTry = 0; ///< Current try counter for active question.
                int m_autoAdvanceDelayMs = 2000; ///< Delay before automatic advance after correct answer.

                double m_autoAdvanceDueTime = 0.0; ///< ImGui time when automatic advance should happen.

                QuizMode m_quizMode = QuizMode::Transformation; ///< Current quiz mode.
                int m_quizModeIndex = 0; ///< ImGui combo index for quiz mode.

                uint32_t m_currentQuestionIndex = 0U; ///< Index of the active question.
                uint32_t m_correctAnswers = 0U; ///< Number of correct answers.
                uint32_t m_wrongAnswers = 0U; ///< Number of wrong answers.
                uint32_t m_acceptedAnswers = 0U; ///< Number of manually accepted answers.

                std::string m_statusMessage; ///< Status message shown in settings window.
                std::string m_feedbackMessage; ///< Feedback message shown in quiz window.

                std::vector<DictionaryWord> m_words; ///< Loaded dictionary words.
                std::vector<SentenceMap> m_sentenceMaps; ///< Loaded sentence maps.
                std::unordered_map<std::string, std::vector<SlotItem>> m_slotLists; ///< Loaded slot lists indexed by ID.
                std::vector<QuizQuestion> m_questions; ///< Generated quiz questions.
                FileStatistics m_statistics; ///< Current file and generation statistics.

                std::mt19937 m_rng; ///< Random generator used for question shuffling.
            };
        }
    }
}