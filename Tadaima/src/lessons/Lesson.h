/**
 * @file LessonStructs.h
 * @brief Defines the Word and Lesson structs representing word and lesson entities.
 */

#pragma once

#include "Dictionary/Word.h"
#include <string>
#include <vector>

namespace tadaima
{
    /**
     * @brief Struct representing a lesson.
     */
    struct Lesson
    {
        int id = 0; /**< The ID of the lesson. */
        std::string groupName;  /**< The group name of the lesson. */
        std::string mainName; /**< The main name of the lesson. */
        std::string subName; /**< The sub name of the lesson. */
        std::vector<Word> words; /**< Words associated with the lesson. */

        // Comparison operators
        bool operator==(const Lesson& other) const
        {
            return id == other.id &&
                mainName == other.mainName &&
                subName == other.subName &&
                words == other.words &&
                groupName == other.groupName;
        }

        bool operator!=(const Lesson& other) const
        {
            return !(*this == other);
        }

        bool isEmpty() const
        {
            return groupName.empty() && mainName.empty() && subName.empty() && words.empty();
        }
    };
}
