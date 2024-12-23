#pragma once

#include "QuizItem.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <random>

namespace tadaima
{
    namespace quiz
    {
        /**
         * @class Quiz
         * @brief A generic quiz class to manage flashcards of any type derived from QuizItem.
         *
         * The Quiz class provides functionality to conduct a quiz using a set of generic flashcards.
         * It tracks the user's progress, manages correct answers, and provides information about flashcards with mistakes.
         */
        class Quiz
        {
        public:
            /**
             * @struct Statistics
             * @brief A structure to hold statistics for each quiz item.
             *
             * This structure keeps track of the number of good and bad attempts
             * for each item and whether the item has been learnt.
             */
            struct Statistics
            {
                int goodAttempts = 0; ///< Number of good attempts.
                int badAttempts = 0; ///< Number of bad attempts.
                bool learnt = false; ///< Whether the item has been learnt.
            };

            /**
             * @brief Constructor for the Quiz class.
             *
             * Initializes the quiz with a set of flashcards, the required number of correct
             * answers for each flashcard, and an option to shuffle the flashcards.
             *
             * @param items A vector of unique pointers to QuizItem objects.
             * @param requiredCorrectAnswers The number of correct answers required for each item.
             * @param enableShuffle Boolean indicating whether to shuffle the items.
             */
            Quiz(std::vector<std::unique_ptr<QuizItem>>& items, int requiredCorrectAnswers, bool enableShuffle = true)
                : m_items(std::move(items)), m_requiredCorrectAnswers(requiredCorrectAnswers), m_shuffleEnabled(enableShuffle)
            {
                if( m_shuffleEnabled )
                {
                    std::shuffle(m_items.begin(), m_items.end(), std::mt19937{ std::random_device{}() });
                }

                for( const auto& item : m_items )
                {
                    m_statistics[item->getKey()] = Statistics();
                }

                if( !m_items.empty() )
                {
                    m_currentItem = m_items.front().get();
                }
            }

            /**
             * @brief Advances the quiz to the next item based on the user's answer.
             *
             * This function checks the user's answer against the current item and updates
             * the progress of the quiz accordingly.
             *
             * @param userAnswer The answer provided by the user.
             * @return True if the user's answer was correct, false otherwise.
             */
            bool advance(const std::string& userAnswer)
            {
                if( !m_currentItem )
                    return false;

                const std::string key = m_currentItem->getKey();
                auto& stats = m_statistics[key];

                if( userAnswer == m_currentItem->getAnswer() )
                {
                    ++stats.goodAttempts;
                    if( stats.goodAttempts >= (stats.badAttempts + m_requiredCorrectAnswers) )
                    {
                        stats.learnt = true;
                    }
                    moveToNextItem();
                    return true;
                }
                else
                {
                    ++stats.badAttempts;
                    moveToNextItem();
                    return false;
                }
            }

            /**
             * @brief Checks if the user's answer is correct for the current item.
             *
             * @param userAnswer The answer provided by the user.
             * @return True if the user's answer was correct, false otherwise.
             */
            bool isCorrect(const std::string& userAnswer) const
            {
                return m_currentItem && (userAnswer == m_currentItem->getAnswer());
            }

            /**
             * @brief Checks if the quiz is complete.
             *
             * This function returns true if all items have been answered correctly the
             * required number of times.
             *
             * @return True if the quiz is complete, false otherwise.
             */
            bool isQuizComplete() const
            {
                if( m_items.empty() )
                {
                    return false; // Explicitly return false if no items are in the quiz
                }

                return std::all_of(m_items.begin(), m_items.end(), [&](const auto& item)
                    {
                        auto it = m_statistics.find(item->getKey());
                        if( it == m_statistics.end() )
                        {
                            return false; // Treat missing keys as not learnt
                        }
                        return it->second.learnt;
                    });
            }

            /**
             * @brief Retrieves the number of items in the quiz.
             *
             * @return The number of items.
             */
            uint32_t getNumberOfItems() const
            {
                return static_cast<uint32_t>(m_items.size());
            }

            /**
             * @brief Retrieves the number of learnt items in the quiz.
             *
             * @return The number of learnt items.
             */
            uint32_t getLearntItems() const
            {
                return std::count_if(m_statistics.begin(), m_statistics.end(), [](const auto& entry)
                    {
                        return entry.second.learnt;
                    });
            }

            /**
             * @brief Retrieves the current item.
             *
             * This function returns a pointer to the current item.
             *
             * @return A pointer to the current item.
             */
            const QuizItem* getCurrentItem() const
            {
                if( !m_currentItem )
                    throw std::invalid_argument("No current quiz item.");
                return m_currentItem;
            }

            /**
             * @brief Retrieves the statistics for each quiz item.
             *
             * @return A const reference to an unordered map of item statistics.
             */
            const std::unordered_map<std::string, Statistics>& getStatistics() const
            {
                return m_statistics;
            }

        private:
            /**
             * @brief Moves to the next item in the quiz.
             *
             * This function advances the quiz to the next item, updating the current index
             * and selecting the next unlearnt item.
             */
            void moveToNextItem()
            {
                if( isQuizComplete() )
                {
                    m_currentItem = nullptr;
                    return;
                }

                if( m_shuffleEnabled )
                {
                    std::vector<QuizItem*> unlearnedItems;
                    for( const auto& item : m_items )
                    {
                        if( !m_statistics[item->getKey()].learnt )
                            unlearnedItems.push_back(item.get());
                    }

                    if( !unlearnedItems.empty() )
                    {
                        std::shuffle(unlearnedItems.begin(), unlearnedItems.end(), std::mt19937{ std::random_device{}() });
                        m_currentItem = unlearnedItems.front();
                        return;
                    }
                }

                do
                {
                    m_currentIndex = (m_currentIndex + 1) % m_items.size();
                } while( m_statistics[m_items[m_currentIndex]->getKey()].learnt );

                m_currentItem = m_items[m_currentIndex].get();
            }

            std::vector<std::unique_ptr<QuizItem>> m_items; ///< The vector of quiz items.
            std::unordered_map<std::string, Statistics> m_statistics; ///< Map of item keys to their statistics.
            QuizItem* m_currentItem = nullptr; ///< Pointer to the current quiz item.
            int m_currentIndex = 0; ///< Index of the current quiz item.
            int m_requiredCorrectAnswers; ///< The number of correct answers required for each item.
            bool m_shuffleEnabled; ///< Boolean indicating whether shuffling is enabled.
        };
    }
}
