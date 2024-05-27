#include "VocabularyQuiz.h"

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            VocabularyQuiz::VocabularyQuiz(std::vector<Flashcard>& flashcards, int requiredCorrectAnswers, bool enableShuffle)
                : quizFlashcards(flashcards), requiredCorrectAnswers(requiredCorrectAnswers), shuffleEnabled(enableShuffle)
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

            void VocabularyQuiz::advance(const std::string& userAnswer)
            {
                if( !currentFlashcard ) return;

                if( currentFlashcard->word.translation != userAnswer )
                {
                    currentFlashcard->badAttempts++;
                    correctAnswers[currentFlashcard->word.id] = std::max(0, correctAnswers[currentFlashcard->word.id] - 1); // Decrement success count
                }
                else
                {
                    correctAnswers[currentFlashcard->word.id]++;
                    if( correctAnswers[currentFlashcard->word.id] >= requiredCorrectAnswers )
                    {
                        currentFlashcard->learned = true;
                    }
                }

                moveToNextFlashcard();
            }

            void VocabularyQuiz::moveToNextFlashcard()
            {
                if( allFlashcardsLearned() )
                {
                    currentFlashcard = nullptr; // No more unlearned flashcards
                    return;
                }

                int initialIndex = currentIndex;
                do
                {
                    currentIndex = (currentIndex + 1) % quizFlashcards.size();
                } while( quizFlashcards[currentIndex].learned && currentIndex != initialIndex );

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