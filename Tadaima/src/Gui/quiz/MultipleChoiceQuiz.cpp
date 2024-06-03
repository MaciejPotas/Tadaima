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

            MultipleChoiceQuiz::MultipleChoiceQuiz(quiz::WordType base, quiz::WordType desired, const std::vector<Lesson>& lessons, tools::Logger& logger)
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

            std::string getTranslation(const Word& word, quiz::WordType type)
            {
                std::string correctAnswer;

                switch( type )
                {
                    case quiz::WordType::BaseWord:
                        correctAnswer = word.translation;
                        break;

                    case quiz::WordType::Kana:
                        correctAnswer = word.kana;
                        break;

                    case quiz::WordType::Romaji:
                        correctAnswer = word.romaji;
                        break;

                    default:
                        break;
                }

                return correctAnswer;
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

            std::vector<std::string> MultipleChoiceQuiz::generateOptions(const Word& correctWord)
            {
                std::string correctWordTranslation = getTranslation(correctWord, m_inputWord);

                std::vector<std::string> options;
                options.push_back(correctWordTranslation);

                if( m_quizWords.size() < 4 )
                {
                    for( const auto& quizWord : m_quizWords )
                    {
                        if( getTranslation(quizWord, m_baseWord) != correctWordTranslation && options.size() < 4 )
                        {
                            options.push_back(correctWordTranslation);
                        }
                    }
                    while( options.size() < 4 )
                    {
                        options.push_back("dummy_option_" + std::to_string(options.size()));
                    }
                }
                else
                {
                    std::unordered_set<std::string> usedTranslations{ getTranslation(correctWord, m_inputWord) };
                    while( options.size() < 4 )
                    {
                        const Word& randomWord = m_quizWords[rng() % m_quizWords.size()];
                        auto optionstring = getTranslation(randomWord, m_inputWord);
                        if( usedTranslations.find(optionstring) == usedTranslations.end() )
                        {
                            options.push_back(optionstring);
                            usedTranslations.insert(optionstring);
                        }
                    }
                }

                std::shuffle(options.begin(), options.end(), rng);

                // Store the correct answer index
                auto it = std::find(options.begin(), options.end(), correctWordTranslation);
                correctAnswerIndex = std::distance(options.begin(), it);

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
                if( answerIndex < 0 || answerIndex >= (int)currentOptions.size() )
                {
                    throw std::out_of_range("Invalid answer choice.");
                }

                const auto& correctWord = m_quizWords[currentWordIndex];

                if( currentOptions[answerIndex] == correctWord.translation )
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
                return currentOptions;
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