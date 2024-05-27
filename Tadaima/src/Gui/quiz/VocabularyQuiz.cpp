#include "VocabularyQuiz.h"

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            VocabularyQuiz::VocabularyQuiz(std::vector<Flashcard>& flashcards, AnswerType answerType, int requiredCorrectAnswers, bool enableShuffle)
                : m_answerType(answerType), quizFlashcards(flashcards), requiredCorrectAnswers(requiredCorrectAnswers), shuffleEnabled(enableShuffle)
            {
                for( const auto& flashcard : flashcards )
                {
                    correctAnswers[flashcard.word.id] = 0; // Initialize correct answers count
                }
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
                        case AnswerType::Romaji:
                            correctAnswer = currentFlashcard->word.romaji;
                            break;
                        case AnswerType::Kana:
                            correctAnswer = currentFlashcard->word.kana;
                            break;
                        case AnswerType::Translation:
                            correctAnswer = currentFlashcard->word.translation;
                            break;
                    }


                    if( correctAnswer != userAnswer )
                    {
                        currentFlashcard->badAttempts++;
                        correctAnswers[currentFlashcard->word.id] = std::max(0, correctAnswers[currentFlashcard->word.id] - 1); // Decrement success count
                    }
                    else
                    {
                        status = true;
                        correctAnswers[currentFlashcard->word.id]++;
                        if( correctAnswers[currentFlashcard->word.id] >= requiredCorrectAnswers )
                        {
                            currentFlashcard->learned = true;
                        }
                    }

                    moveToNextFlashcard();
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
                    for( int i = 0; i < quizFlashcards.size(); ++i )
                    {
                        if( !quizFlashcards[i].learned )
                        {
                            unlearnedIndices.push_back(i);
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

            const tadaima::gui::quiz::Flashcard& VocabularyQuiz::getCurrentFlashCard() const
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