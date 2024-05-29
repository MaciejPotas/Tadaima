#include "VocabularyQuiz.h"

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            VocabularyQuiz::VocabularyQuiz(std::vector<Flashcard>& flashcards, WordType answerType, int requiredCorrectAnswers, bool enableShuffle)
                : m_flashcards(flashcards), m_answerType(answerType), m_requiredCorrectAnswers(requiredCorrectAnswers), m_shuffleEnabled(enableShuffle)
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
                    std::string correctAnswer;
                    switch( m_answerType )
                    {
                        case WordType::Romaji:
                            correctAnswer = m_currentFlashcard->word.romaji;
                            break;
                        case WordType::Kana:
                            correctAnswer = m_currentFlashcard->word.kana;
                            break;
                        case WordType::Translation:
                            correctAnswer = m_currentFlashcard->word.translation;
                            break;
                    }

                    if( correctAnswer != userAnswer )
                    {
                        m_currentFlashcard->badAttempts++;
                    }
                    else
                    {
                        status = true;
                        m_currentFlashcard->goodAttemps++;
                        if( m_currentFlashcard->goodAttemps >= (m_currentFlashcard->badAttempts + m_requiredCorrectAnswers) )
                        {
                            m_currentFlashcard->learned = true;
                        }
                    }

                    moveToNextFlashcard();
                }

                return status;
            }

            bool VocabularyQuiz::isCorrect(const std::string& userAnswer)
            {
                bool status = false;

                if( m_currentFlashcard )
                {
                    std::string correctAnswer;
                    switch( m_answerType )
                    {
                        case WordType::Romaji:
                            correctAnswer = m_currentFlashcard->word.romaji;
                            break;
                        case WordType::Kana:
                            correctAnswer = m_currentFlashcard->word.kana;
                            break;
                        case WordType::Translation:
                            correctAnswer = m_currentFlashcard->word.translation;
                            break;
                    }

                    if( correctAnswer == userAnswer )
                    {
                        status = true;
                    }
                }

                return status;
            }

            void VocabularyQuiz::moveToNextFlashcard()
            {
                if( allFlashcardsLearned() )
                {
                    m_currentFlashcard = nullptr; // No more unlearned flashcards
                    return;
                }
                if( m_shuffleEnabled )
                {
                    std::vector<int> unlearnedIndices;
                    for( int index = 0; index < static_cast<int>(m_flashcards.size()); ++index )
                    {
                        if( !m_flashcards[index].learned )
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
                    } while( m_flashcards[m_currentIndex].learned && m_currentIndex != initialIndex );
                }

                m_currentFlashcard = &m_flashcards[m_currentIndex];
            }

            bool VocabularyQuiz::isQuizComplete() const
            {
                return allFlashcardsLearned();
            }

            std::vector<Flashcard> VocabularyQuiz::getFlashcardsWithMistakes(int minMistakes) const
            {
                std::vector<Flashcard> result;
                for( const auto& flashcard : m_flashcards )
                {
                    if( flashcard.badAttempts >= minMistakes )
                    {
                        result.push_back(flashcard);
                    }
                }
                return result;
            }

            std::vector<Flashcard> VocabularyQuiz::getLearntWords() const
            {
                std::vector<Flashcard> result;
                for( const auto& flashcard : m_flashcards )
                {
                    if( flashcard.learned )
                    {
                        result.push_back(flashcard);
                    }
                }
                return result;
            }

            const Flashcard& VocabularyQuiz::getCurrentFlashCard() const
            {
                return *m_currentFlashcard;
            }

            bool VocabularyQuiz::allFlashcardsLearned() const
            {
                return std::all_of(m_flashcards.begin(), m_flashcards.end(), [](const Flashcard& flashcard)
                    {
                        return flashcard.learned;
                    });
            }
        }
    }
}
