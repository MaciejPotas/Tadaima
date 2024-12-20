#include "ConjugationQuiz.h"
#include <algorithm>
#include <random>

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            ConjugationQuiz::ConjugationQuiz(std::vector<ConjugationFlashCard>& flashcards, int requiredCorrectAnswers, bool enableShuffle)
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

            bool ConjugationQuiz::advance(const std::string& userAnswer)
            {
                bool status = false;

                if( m_currentFlashcard )
                {
                    if( m_currentFlashcard->answer != userAnswer )
                    {
                        m_statistics[m_currentFlashcard->wordId].badAttempts++;
                    }
                    else
                    {
                        status = true;
                        m_statistics[m_currentFlashcard->wordId].goodAttempts++;
                        if( m_statistics[m_currentFlashcard->wordId].goodAttempts >=
                            (m_statistics[m_currentFlashcard->wordId].badAttempts + m_requiredCorrectAnswers) )
                        {
                            m_statistics[m_currentFlashcard->wordId].learnt = true;
                        }
                    }

                    moveToNextFlashcard();
                }

                return status;
            }

            bool ConjugationQuiz::isCorrect(const std::string& userAnswer)
            {
                return m_currentFlashcard != nullptr && (m_currentFlashcard->answer == userAnswer);
            }

            void ConjugationQuiz::moveToNextFlashcard()
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

            bool ConjugationQuiz::isQuizComplete() const
            {
                return std::all_of(m_flashcards.begin(), m_flashcards.end(), [&](const ConjugationFlashCard& flashcard)
                    {
                        auto it = m_statistics.find(flashcard.wordId);
                        return it != m_statistics.end() && it->second.learnt;
                    });
            }

            uint32_t ConjugationQuiz::getNumberOfFlashcards() const
            {
                return static_cast<uint32_t>(m_flashcards.size());
            }

            uint32_t ConjugationQuiz::getLearntConjugations() const
            {
                return static_cast<uint32_t>(std::count_if(m_statistics.begin(), m_statistics.end(), [](const auto& entry)
                    {
                        return entry.second.learnt;
                    }));
            }

            const std::unordered_map<int, ConjugationQuiz::WordStatistics>& ConjugationQuiz::getStatistics() const
            {
                return m_statistics;
            }

            const ConjugationQuiz::ConjugationFlashCard& ConjugationQuiz::getCurrentFlashCard() const
            {
                if( !m_currentFlashcard )
                {
                    throw std::invalid_argument("ConjugationQuiz::getCurrentFlashCard: flashcard was not set.");
                }

                return *m_currentFlashcard;
            }
        }
    }
}
