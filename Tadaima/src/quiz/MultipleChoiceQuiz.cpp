#include "MultipleChoiceQuiz.h"
#include <unordered_set>
#include <stdexcept>
#include <sstream>

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {

            MultipleChoiceQuiz::MultipleChoiceQuiz(tadaima::quiz::WordType base, tadaima::quiz::WordType desired, const std::vector<Lesson>& lessons, tools::Logger& logger)
                : m_baseWord(base), m_inputWord(desired), m_logger(logger), rng(std::random_device{}()), currentWordIndex(0), correctCount(0)
            {
                initialize(lessons);
                if( !m_quizWords.empty() )
                {
                    shuffleWords();
                }
                else
                {
                    throw std::invalid_argument("No words available in the provided lessons.");
                }
            }

            std::string getTranslation(const Word& word, tadaima::quiz::WordType type)
            {
                std::string correctAnswer;

                switch( type )
                {
                    case tadaima::quiz::WordType::BaseWord:
                        correctAnswer = word.translation;
                        break;

                    case tadaima::quiz::WordType::Kana:
                        correctAnswer = word.kana;
                        break;

                    case tadaima::quiz::WordType::Romaji:
                        correctAnswer = word.romaji;
                        break;

                    case tadaima::quiz::WordType::Kanji:
                        correctAnswer = word.kanji;
                        break;

                    default:
                        break;
                }

                return correctAnswer;
            }

            std::string getDisplayValueOrDash(const std::string& value)
            {
                std::string displayValue = value;

                if( displayValue.empty() )
                {
                    displayValue = "-";
                }

                return displayValue;
            }

            std::string MultipleChoiceQuiz::buildOptionDetails(const Word& word) const
            {
                std::ostringstream oss;

                oss << getDisplayValueOrDash(word.translation)
                    << ", " << getDisplayValueOrDash(word.kana)
                    << ", " << getDisplayValueOrDash(word.kanji);

                return oss.str();
            }

            std::vector<std::string> MultipleChoiceQuiz::buildDisplayOptions(bool includeDetails) const
            {
                std::vector<std::string> displayOptions;

                for( const auto& option : currentOptions )
                {
                    std::string displayText = option.answer;

                    if( includeDetails )
                    {
                        displayText += " (" + option.details + ")";
                    }

                    displayOptions.push_back(displayText);
                }

                return displayOptions;
            }

            void MultipleChoiceQuiz::initialize(const std::vector<Lesson>& lessons)
            {
                for( const auto& lesson : lessons )
                {
                    for( const auto& word : lesson.words )
                    {
                        m_quizWords.push_back(word);
                    }
                }
                if( m_quizWords.empty() )
                {
                    throw std::invalid_argument("No words available in the provided lessons.");
                }
            }

            std::vector<MultipleChoiceQuiz::QuizOption> MultipleChoiceQuiz::generateOptions(const Word& correctWord)
            {
                const std::string correctWordTranslation = getTranslation(correctWord, m_inputWord);

                std::vector<QuizOption> options;
                std::unordered_set<std::string> usedTranslations;

                options.push_back({ correctWordTranslation, buildOptionDetails(correctWord) });
                usedTranslations.insert(correctWordTranslation);

                std::vector<Word> candidateWords;

                for( const auto& quizWord : m_quizWords )
                {
                    const std::string optionString = getTranslation(quizWord, m_inputWord);

                    if( !optionString.empty() && usedTranslations.find(optionString) == usedTranslations.end() )
                    {
                        candidateWords.push_back(quizWord);
                        usedTranslations.insert(optionString);
                    }
                }

                std::shuffle(candidateWords.begin(), candidateWords.end(), rng);

                for( const auto& candidateWord : candidateWords )
                {
                    if( options.size() < 4U )
                    {
                        options.push_back({ getTranslation(candidateWord, m_inputWord), buildOptionDetails(candidateWord) });
                    }
                }

                while( options.size() < 4U )
                {
                    const std::string dummyText = "dummy_option_" + std::to_string(options.size());
                    options.push_back({ dummyText, "base: -, hiragana: -, kanji: -" });
                }

                std::shuffle(options.begin(), options.end(), rng);

                auto it = std::find_if(options.begin(), options.end(),
                    [&correctWordTranslation](const QuizOption& option)
                    {
                        return option.answer == correctWordTranslation;
                    });

                correctAnswerIndex = static_cast<int>(std::distance(options.begin(), it));

                return options;
            }

            void MultipleChoiceQuiz::shuffleWords()
            {
                std::shuffle(m_quizWords.begin(), m_quizWords.end(), rng);
            }

            void MultipleChoiceQuiz::start()
            {
                currentWordIndex = 0;
                correctCount = 0;
                if( !m_quizWords.empty() )
                {
                    currentOptions = generateOptions(m_quizWords[currentWordIndex]);
                }
                else
                {
                    throw std::runtime_error("No words available to start the quiz.");
                }
            }

            void MultipleChoiceQuiz::advance(char answer)
            {
                if( isFinished() )
                {
                    return;
                }

                int answerIndex = answer - 'a';

                if( answerIndex < 0 || answerIndex >= static_cast<int>(currentOptions.size()) )
                {
                    throw std::out_of_range("Invalid answer choice.");
                }

                if( currentOptions[answerIndex].answer == getTranslation(m_quizWords[currentWordIndex], m_inputWord) )
                {
                    correctCount++;
                }

                currentWordIndex++;

                if( !isFinished() )
                {
                    currentOptions = generateOptions(m_quizWords[currentWordIndex]);
                }
            }

            bool MultipleChoiceQuiz::isFinished() const
            {
                return currentWordIndex >= m_quizWords.size();
            }

            std::string MultipleChoiceQuiz::getCurrentQuestion() const
            {
                if( !isFinished() )
                {
                    return "Translate the word: " + getTranslation(m_quizWords[currentWordIndex], m_baseWord);
                }
                return "";
            }

            std::vector<std::string> MultipleChoiceQuiz::getCurrentOptions() const
            {
                return buildDisplayOptions(false);
            }

            std::vector<std::string> MultipleChoiceQuiz::getCurrentOptionsWithDetails() const
            {
                return buildDisplayOptions(true);
            }

            std::string MultipleChoiceQuiz::getResults() const
            {
                std::ostringstream oss;
                oss << "Quiz finished!\nYou got " << correctCount << " out of " << m_quizWords.size() << " correct!";
                return oss.str();
            }

            int MultipleChoiceQuiz::getCurrentQuestionIndex() const
            {
                return static_cast<int>(currentWordIndex);
            }

            int MultipleChoiceQuiz::getTotalQuestions() const
            {
                return static_cast<int>(m_quizWords.size());
            }

            int MultipleChoiceQuiz::getCorrectAnswerIndex() const
            {
                return correctAnswerIndex;
            }
        }
    }
}