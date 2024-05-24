#include "gui/quiz/QuizGame.h"
#include <stdexcept>
#include <unordered_set>
#include "Tools/Logger.h"
#include <sstream>

namespace tadaima
{
    namespace gui
    {
        QuizGame::QuizGame(tools::Logger& logger, const std::vector<Lesson>& lessons)
            : m_logger(logger), rng(std::random_device{}()), currentWordIndex(0), correctCount(0)
        {
            m_logger.log("Initializing QuizGame.", tools::LogLevel::INFO);
            initialize(lessons);
            if( !words.empty() )
            {
                shuffleWords();
                m_logger.log("QuizGame initialized with " + std::to_string(words.size()) + " words.", tools::LogLevel::INFO);
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
                m_logger.log("Not enough words to generate unique options. Using available words and dummy options.", tools::LogLevel::WARNING);
                // Not enough words to generate unique options
                for( const auto& word : words )
                {
                    if( word.translation != correctWord.translation && options.size() < 4 )
                    {
                        options.push_back(word.translation);
                    }
                }
                // Fill the rest with dummy options if necessary
                while( options.size() < 4 )
                {
                    options.push_back("dummy_option_" + std::to_string(options.size()));
                }
            }
            else
            {
                // Generate three random wrong options
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

            // Shuffle the options to randomize their order
            std::shuffle(options.begin(), options.end(), rng);
            return options;
        }

        void QuizGame::shuffleWords()
        {
            m_logger.log("Shuffling words.", tools::LogLevel::DEBUG);
            std::shuffle(words.begin(), words.end(), rng);
        }

        void QuizGame::start()
        {
            currentWordIndex = 0;
            correctCount = 0;
            if( !words.empty() )
            {
                currentOptions = generateOptions(words[currentWordIndex]);
                m_logger.log("Quiz started with first question: " + getCurrentQuestion(), tools::LogLevel::INFO);
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
                m_logger.log("Quiz already finished.", tools::LogLevel::WARNING);
                return;
            }

            int answerIndex = answer - 'a';
            if( answerIndex < 0 || answerIndex >= static_cast<int>(currentOptions.size()) )
            {
                throw std::out_of_range("Invalid answer choice.");
            }

            const auto& correctWord = words[currentWordIndex];

            if( currentOptions[answerIndex] == correctWord.translation )
            {
                correctCount++;
                m_logger.log("Correct answer for word: " + correctWord.kana, tools::LogLevel::INFO);
            }
            else
            {
                m_logger.log("Wrong answer for word: " + correctWord.kana, tools::LogLevel::INFO);
            }

            currentWordIndex++;

            if( !isFinished() )
            {
                currentOptions = generateOptions(words[currentWordIndex]);
                m_logger.log("Next question: " + getCurrentQuestion(), tools::LogLevel::INFO);
            }
            else
            {
                m_logger.log(getResults(), tools::LogLevel::INFO);
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
    }
}
