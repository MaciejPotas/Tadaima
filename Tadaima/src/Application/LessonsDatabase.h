/**
 * @file LessonsDatabase.h
 * @brief Defines the LessonsDatabase class, providing functionality to interact with a SQLite database for managing lessons and words.
 */

#pragma once

#include "tools/Database.h"
#include "Lessons/Lesson.h"
#include <vector>
#include <string>
#include <Libraries/SQLite3/sqlite3.h>

namespace tadaima
{
    namespace application
    {
        /**
         * @brief The LessonsDatabase class provides functionality to interact with a SQLite database
         * for managing lessons and words.
         */
        class LessonsDatabase : public Database
        {
        public:
            /**
             * @brief Constructs a LessonsDatabase object.
             * @param dbPath The file path to the SQLite database.
             */
            LessonsDatabase(const std::string& dbPath);

            /**
             * @brief Destroys the LessonsDatabase object.
             */
            ~LessonsDatabase();

            /**
             * @brief Initializes the SQLite database.
             * @return True if initialization is successful, false otherwise.
             */
            bool initDatabase();

            // Database interface implementation
            virtual int addLesson(const std::string& mainName, const std::string& subName) override;
            virtual int addWord(int lessonId, const Word& word) override;
            virtual void addTag(int wordId, const std::string& tag) override;
            virtual std::vector<std::string> getLessonNames() const override;
            virtual std::vector<Word> getWordsInLesson(int lessonId) const override;
            virtual std::vector<Lesson> getAllLessons() const override;

        private:
            /**
             * @brief Pointer to the SQLite database.
             */
            sqlite3* db;
        };
    }
}
