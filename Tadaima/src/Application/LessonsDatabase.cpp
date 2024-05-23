#include "LessonsDatabase.h"
#include <iostream>

namespace tadaima
{
    namespace application
    {
        LessonsDatabase::LessonsDatabase(const std::string& dbPath) : db(nullptr)
        {
            if( sqlite3_open(dbPath.c_str(), &db) )
            {
                std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            }
            else
            {
                initDatabase();
            }
        }

        LessonsDatabase::~LessonsDatabase()
        {
            if( db )
            {
                sqlite3_close(db);
            }
        }

        bool LessonsDatabase::initDatabase()
        {
            const char* createLessonsTable =
                "CREATE TABLE IF NOT EXISTS lessons ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "main_name TEXT NOT NULL, "
                "sub_name TEXT NOT NULL);";
            const char* createWordsTable =
                "CREATE TABLE IF NOT EXISTS words ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "lesson_id INTEGER, "
                "kana TEXT NOT NULL, "
                "translation TEXT NOT NULL, "
                "romaji TEXT, "
                "example_sentence TEXT, "
                "FOREIGN KEY(lesson_id) REFERENCES lessons(id));";
            const char* createTagsTable =
                "CREATE TABLE IF NOT EXISTS tags ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "word_id INTEGER, "
                "tag TEXT NOT NULL, "
                "FOREIGN KEY(word_id) REFERENCES words(id));";

            char* errMsg = nullptr;
            if( sqlite3_exec(db, createLessonsTable, 0, 0, &errMsg) != SQLITE_OK )
            {
                std::cerr << "SQL error: " << errMsg << std::endl;
                sqlite3_free(errMsg);
                return false;
            }
            if( sqlite3_exec(db, createWordsTable, 0, 0, &errMsg) != SQLITE_OK )
            {
                std::cerr << "SQL error: " << errMsg << std::endl;
                sqlite3_free(errMsg);
                return false;
            }
            if( sqlite3_exec(db, createTagsTable, 0, 0, &errMsg) != SQLITE_OK )
            {
                std::cerr << "SQL error: " << errMsg << std::endl;
                sqlite3_free(errMsg);
                return false;
            }
            return true;
        }

        int LessonsDatabase::addLesson(const std::string& mainName, const std::string& subName)
        {
            const char* sql = "INSERT INTO lessons (main_name, sub_name) VALUES (?, ?);";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_text(stmt, 1, mainName.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, subName.c_str(), -1, SQLITE_STATIC);
                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
                    sqlite3_finalize(stmt);
                    return -1;
                }
                sqlite3_finalize(stmt);
                return sqlite3_last_insert_rowid(db);
            }
            return -1;
        }

        int LessonsDatabase::addWord(int lessonId, const Word& word)
        {
            const char* sql =
                "INSERT INTO words (lesson_id, kana, translation, romaji, example_sentence) "
                "VALUES (?, ?, ?, ?, ?);";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_int(stmt, 1, lessonId);
                sqlite3_bind_text(stmt, 2, word.kana.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, word.translation.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 4, word.romaji.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 5, word.exampleSentence.c_str(), -1, SQLITE_STATIC);
                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
                    sqlite3_finalize(stmt);
                    return -1;
                }
                int wordId = sqlite3_last_insert_rowid(db);
                sqlite3_finalize(stmt);
                return wordId;
            }
            return -1;
        }

        void LessonsDatabase::addTag(int wordId, const std::string& tag)
        {
            const char* sql = "INSERT INTO tags (word_id, tag) VALUES (?, ?);";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_int(stmt, 1, wordId);
                sqlite3_bind_text(stmt, 2, tag.c_str(), -1, SQLITE_STATIC);
                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_finalize(stmt);
            }
        }

        void LessonsDatabase::updateLesson(int lessonId, const std::string& newMainName, const std::string& newSubName)
        {
            const char* sql = "UPDATE lessons SET main_name = ?, sub_name = ? WHERE id = ?;";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_text(stmt, 1, newMainName.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, newSubName.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(stmt, 3, lessonId);
                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_finalize(stmt);
            }
        }

        void LessonsDatabase::updateWord(int wordId, const Word& updatedWord)
        {
            const char* sql = "UPDATE words SET kana = ?, translation = ?, romaji = ?, example_sentence = ? WHERE id = ?;";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_text(stmt, 1, updatedWord.kana.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, updatedWord.translation.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, updatedWord.romaji.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 4, updatedWord.exampleSentence.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(stmt, 5, wordId);
                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_finalize(stmt);
            }
        }

        void LessonsDatabase::deleteLesson(int lessonId)
        {
            const char* sql = "DELETE FROM lessons WHERE id = ?;";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_int(stmt, 1, lessonId);
                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_finalize(stmt);
            }
        }

        void LessonsDatabase::deleteWord(int wordId)
        {
            const char* sql = "DELETE FROM words WHERE id = ?;";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_int(stmt, 1, wordId);
                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_finalize(stmt);
            }
        }

        std::vector<std::string> LessonsDatabase::getLessonNames() const
        {
            std::vector<std::string> lessonNames;
            const char* sql = "SELECT main_name, sub_name FROM lessons;";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                while( sqlite3_step(stmt) == SQLITE_ROW )
                {
                    std::string lessonName = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))) + " - " + std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
                    lessonNames.push_back(lessonName);
                }
                sqlite3_finalize(stmt);
            }
            return lessonNames;
        }

        std::vector<Word> LessonsDatabase::getWordsInLesson(int lessonId) const
        {
            std::vector<Word> words;
            const char* sql = "SELECT id, kana, translation, romaji, example_sentence FROM words WHERE lesson_id = ?;";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_int(stmt, 1, lessonId);
                while( sqlite3_step(stmt) == SQLITE_ROW )
                {
                    Word word;
                    word.id = sqlite3_column_int(stmt, 0);
                    word.kana = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                    word.translation = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                    word.romaji = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                    word.exampleSentence = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

                    // Get tags for this word
                    const char* tagSql = "SELECT tag FROM tags WHERE word_id = ?;";
                    sqlite3_stmt* tagStmt;
                    if( sqlite3_prepare_v2(db, tagSql, -1, &tagStmt, 0) == SQLITE_OK )
                    {
                        sqlite3_bind_int(tagStmt, 1, word.id);
                        while( sqlite3_step(tagStmt) == SQLITE_ROW )
                        {
                            word.tags.push_back(reinterpret_cast<const char*>(sqlite3_column_text(tagStmt, 0)));
                        }
                        sqlite3_finalize(tagStmt);
                    }
                    words.push_back(word);
                }
                sqlite3_finalize(stmt);
            }
            return words;
        }

        std::vector<Lesson> LessonsDatabase::getAllLessons() const
        {
            std::vector<Lesson> lessons;
            const char* sql = "SELECT id, main_name, sub_name FROM lessons;";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                while( sqlite3_step(stmt) == SQLITE_ROW )
                {
                    Lesson lesson;
                    lesson.id = sqlite3_column_int(stmt, 0);
                    lesson.mainName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                    lesson.subName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                    lesson.words = getWordsInLesson(lesson.id);
                    lessons.push_back(lesson);
                }
                sqlite3_finalize(stmt);
            }
            return lessons;
        }
    }
}
