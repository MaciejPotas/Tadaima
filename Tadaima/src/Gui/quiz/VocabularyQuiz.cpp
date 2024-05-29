#include "VocabularyQuiz.h"

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            VocabularyQuiz::VocabularyQuiz(std::vector<Flashcard>& flashcards, WordType answerType, int requiredCorrectAnswers, bool enableShuffle)
                : m_answerType(answerType), quizFlashcards(flashcards), requiredCorrectAnswers(requiredCorrectAnswers), shuffleEnabled(enableShuffle)
            {
                if( shuffleEnabled )
                {
                    std::shuffle(quizFlashcards.begin(), quizFlashcards.end(), std::mt19937{ std::random_device{}() });
                }
                if( !quizFlashcards.empty() )
                {
                    currentFlashcard = &quizFlashcards.front();
                }
            }

            bool VocabularyQuiz::advance(const std::string& userAnswer)
            {
                bool status = false;

                if( currentFlashcard )
                {
                    std::string correctAnswer;
                    switch( m_answerType )
                    {
                        case WordType::Romaji:
                            correctAnswer = currentFlashcard->word.romaji;
                            break;
                        case WordType::Kana:
                            correctAnswer = currentFlashcard->word.kana;
                            break;
                        case WordType::Translation:
                            correctAnswer = currentFlashcard->word.translation;
                            break;
                    }

                    if( correctAnswer != userAnswer )
                    {
                        currentFlashcard->badAttempts++;
                    }
                    else
                    {
                        status = true;
                        currentFlashcard->goodAttemps++;
                        if( std::abs(currentFlashcard->goodAttemps - currentFlashcard->badAttempts) >= requiredCorrectAnswers )
                        {
                            currentFlashcard->learned = true;
                        }
                    }

                    moveToNextFlashcard();
                }

                return status;
            }

            bool VocabularyQuiz::isCorrect(const std::string& userAnswer)
            {
                bool status = false;

                if( currentFlashcard )
                {
                    std::string correctAnswer;
                    switch( m_answerType )
                    {
                        case WordType::Romaji:
                            correctAnswer = currentFlashcard->word.romaji;
                            break;
                        case WordType::Kana:
                            correctAnswer = currentFlashcard->word.kana;
                            break;
                        case WordType::Translation:
                            correctAnswer = currentFlashcard->word.translation;
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
                    currentFlashcard = nullptr; // No more unlearned flashcards
                    return;
                }
                if( shuffleEnabled )
                {
                    std::vector<int> unlearnedIndices;
                    for( int index = 0; index < (int)quizFlashcards.size(); ++index )
                    {
                        if( !quizFlashcards[index].learned )
                        {
                            unlearnedIndices.push_back(index);
                        }
                    }

                    std::random_device rd;
                    std::mt19937 g(rd());
                    std::uniform_int_distribution<> distrib(0, unlearnedIndices.size() - 1);

                    int randomIndex = distrib(g);
                    currentIndex = unlearnedIndices[randomIndex];
                }
                else
                {
                    int initialIndex = currentIndex;
                    do
                    {
                        currentIndex = (currentIndex + 1) % quizFlashcards.size();
                    } while( quizFlashcards[currentIndex].learned && currentIndex != initialIndex );
                }

                currentFlashcard = &quizFlashcards[currentIndex];
            }

            bool VocabularyQuiz::isQuizComplete() const
            {
                return allFlashcardsLearned();
            }

            std::vector<Flashcard> VocabularyQuiz::getFlashcardsWithMistakes(int minMistakes) const
            {
                std::vector<Flashcard> result;
                for( const auto& flashcard : quizFlashcards )
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
                for( const auto& flashcard : quizFlashcards )
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
                return *currentFlashcard;
            }

            bool VocabularyQuiz::allFlashcardsLearned() const
            {
                return std::all_of(quizFlashcards.begin(), quizFlashcards.end(), [](const Flashcard& flashcard)
                    {
                        return flashcard.learned;
                    });
            }
        }
    }
}
