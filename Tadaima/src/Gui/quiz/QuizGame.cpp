#include "QuizGame.h"
#include <unordered_set>
#include <stdexcept>
#include <sstream>

namespace tadaima
{
    namespace gui
    {
        QuizGame::QuizGame(tools::Logger& logger, const std::vector<Lesson>& lessons)
            : m_logger(logger), rng(std::random_device{}()), currentWordIndex(0), correctCount(0)
        {
            initialize(lessons);
            if( !words.empty() )
            {
                shuffleWords();
            }
            else
            {
                throw std::invalid_argument("No words available in the provided lessons.");
            }
        }

        void QuizGame::initialize(const std::vector<Lesson>& lessons)
        {
            for( const auto& lesson : lessons )
            {
                for( const auto& word : lesson.words )
                {
                    words.push_back(word);
                }
            }
            if( words.empty() )
            {
                throw std::invalid_argument("No words available in the provided lessons.");
            }
        }

        std::vector<std::string> QuizGame::generateOptions(const Word& correctWord)
        {
            std::vector<std::string> options;
            options.push_back(correctWord.translation);

            if( words.size() < 4 )
            {
                for( const auto& word : words )
                {
                    if( word.translation != correctWord.translation && options.size() < 4 )
                    {
                        options.push_back(word.translation);
                    }
                }
                while( options.size() < 4 )
                {
                    options.push_back("dummy_option_" + std::to_string(options.size()));
                }
            }
            else
            {
                std::unordered_set<std::string> usedTranslations{ correctWord.translation };
                while( options.size() < 4 )
                {
                    const Word& randomWord = words[rng() % words.size()];
                    if( usedTranslations.find(randomWord.translation) == usedTranslations.end() )
                    {
                        options.push_back(randomWord.translation);
                        usedTranslations.insert(randomWord.translation);
                    }
                }
            }

            std::shuffle(options.begin(), options.end(), rng);

            // Store the correct answer index
            auto it = std::find(options.begin(), options.end(), correctWord.translation);
            correctAnswerIndex = std::distance(options.begin(), it);

            return options;
        }

        void QuizGame::shuffleWords()
        {
            std::shuffle(words.begin(), words.end(), rng);
        }

        void QuizGame::start()
        {
            currentWordIndex = 0;
            correctCount = 0;
            if( !words.empty() )
            {
                currentOptions = generateOptions(words[currentWordIndex]);
            }
            else
            {
                throw std::runtime_error("No words available to start the quiz.");
            }
        }

        void QuizGame::advance(char answer)
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

            const auto& correctWord = words[currentWordIndex];

            if( currentOptions[answerIndex] == correctWord.translation )
            {
                correctCount++;
            }

            currentWordIndex++;

            if( !isFinished() )
            {
                currentOptions = generateOptions(words[currentWordIndex]);
            }
        }

        bool QuizGame::isFinished() const
        {
            return currentWordIndex >= words.size();
        }

        std::string QuizGame::getCurrentQuestion() const
        {
            if( !isFinished() )
            {
                return "Translate the word: " + words[currentWordIndex].kana;
            }
            return "";
        }

        std::vector<std::string> QuizGame::getCurrentOptions() const
        {
            return currentOptions;
        }

        std::string QuizGame::getResults() const
        {
            std::ostringstream oss;
            oss << "Quiz finished!\nYou got " << correctCount << " out of " << words.size() << " correct!";
            return oss.str();
        }

        int QuizGame::getCurrentQuestionIndex() const
        {
            return static_cast<int>(currentWordIndex);
        }

        int QuizGame::getTotalQuestions() const
        {
            return static_cast<int>(words.size());
        }

        int QuizGame::getCorrectAnswerIndex() const
        {
            return correctAnswerIndex;
        }
    }
}
