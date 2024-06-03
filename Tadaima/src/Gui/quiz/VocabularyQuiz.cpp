#include "VocabularyQuiz.h"
#include <algorithm>
#include <random>

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            VocabularyQuiz::VocabularyQuiz(std::vector<QuizWord>& flashcards, int requiredCorrectAnswers, bool enableShuffle)
                : m_flashcards(flashcards), m_requiredCorrectAnswers(requiredCorrectAnswers), m_shuffleEnabled(enableShuffle)
            {
                if( m_shuffleEnabled )
                {
                    std::shuffle(m_flashcards.begin(), m_flashcards.end(), std::mt19937{ std::random_device{}() });
                }

                if( !m_flashcards.empty() )
                {
                    m_currentFlashcard = &m_flashcards.front();
                }
            }

            bool VocabularyQuiz::advance(const std::string& userAnswer)
            {
                bool status = false;

                if( m_currentFlashcard )
                {
                    if( m_currentFlashcard->word != userAnswer )
                    {
                        m_statistics[m_currentFlashcard->wordId].badAttempts++;
                    }
                    else
                    {
                        status = true;
                        m_statistics[m_currentFlashcard->wordId].goodAttempts++;
                        if( m_statistics[m_currentFlashcard->wordId].goodAttempts >= (m_statistics[m_currentFlashcard->wordId].badAttempts + m_requiredCorrectAnswers) )
                        {
                            m_statistics[m_currentFlashcard->wordId].learnt = true;
                        }
                    }

                    moveToNextFlashcard();
                }

                return status;
            }

            bool VocabularyQuiz::isCorrect(const std::string& userAnswer)
            {
                return m_currentFlashcard != nullptr ? (m_currentFlashcard->word == userAnswer) : false;
            }

            void VocabularyQuiz::moveToNextFlashcard()
            {
                if( isQuizComplete() )
                {
                    m_currentFlashcard = nullptr; // No more unlearned flashcards
                    return;
                }
                if( m_shuffleEnabled )
                {
                    std::vector<int> unlearnedIndices;
                    for( int index = 0; index < static_cast<int>(m_flashcards.size()); ++index )
                    {
                        if( !m_statistics[m_flashcards[index].wordId].learnt )
                        {
                            unlearnedIndices.push_back(index);
                        }
                    }

                    std::random_device rd;
                    std::mt19937 g(rd());
                    std::uniform_int_distribution<> distrib(0, unlearnedIndices.size() - 1);

                    int randomIndex = distrib(g);
                    m_currentIndex = unlearnedIndices[randomIndex];
                }
                else
                {
                    int initialIndex = m_currentIndex;
                    do
                    {
                        m_currentIndex = (m_currentIndex + 1) % m_flashcards.size();
                    } while( m_statistics[m_flashcards[m_currentIndex].wordId].learnt && m_currentIndex != initialIndex );
                }

                m_currentFlashcard = &m_flashcards[m_currentIndex];
            }

            bool VocabularyQuiz::isQuizComplete() const
            {
                return std::all_of(m_flashcards.begin(), m_flashcards.end(), [&](const QuizWord& flashcard)
                    {
                        auto it = m_statistics.find(flashcard.wordId);
                        return it != m_statistics.end() && it->second.learnt;
                    });
            }

            uint32_t VocabularyQuiz::getNumberOflashcards() const
            {
                return m_flashcards.size();
            }

            uint32_t VocabularyQuiz::getLearntWords() const
            {
                return std::count_if(m_statistics.begin(), m_statistics.end(), [](const auto& entry)
                    {
                        return entry.second.learnt;
                    });
            }

            const std::unordered_map<int, tadaima::gui::quiz::VocabularyQuiz::WordStatistics>& VocabularyQuiz::getStatistics() const
            {
                return m_statistics;
            }

            const QuizWord& VocabularyQuiz::getCurrentFlashCard() const
            {
                if( nullptr == m_currentFlashcard )
                {
                    throw std::invalid_argument("VocabularyQuiz::getCurrentFlashCard: flashcard was not set.");
                }

                return *m_currentFlashcard;
            }
        }
    }
}
