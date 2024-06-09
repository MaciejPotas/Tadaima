#include "ApplicationDatabase.h"
#include <iostream>
#include <Libraries/SQLite3/sqlite3.h>
#include "Tools/Logger.h"
#include "ApplicationSettings.h"

namespace tadaima
{
    namespace application
    {
        ApplicationDatabase::ApplicationDatabase(const std::string& dbPath, tools::Logger& logger)
            : db(nullptr), m_logger(logger)
        {
            if( sqlite3_open(dbPath.c_str(), &db) )
            {
                m_logger.log("Database: Can't open database: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
            }
            else
            {
                m_logger.log("Database: Opened database successfully at " + dbPath, tools::LogLevel::INFO);
                if( initDatabase() )
                {
                    m_logger.log("Database: Initialized database successfully.", tools::LogLevel::INFO);
                }
                else
                {
                    m_logger.log("Database: Failed to initialize database.", tools::LogLevel::PROBLEM);
                }
            }
        }

        ApplicationDatabase::~ApplicationDatabase()
        {
            if( db )
            {
                sqlite3_close(db);
                m_logger.log("Database: Closed database connection.", tools::LogLevel::INFO);
            }
        }

        bool ApplicationDatabase::initDatabase()
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
            const char* createSettingsTable =
                "CREATE TABLE IF NOT EXISTS settings ("
                "key TEXT PRIMARY KEY, "
                "value TEXT NOT NULL);";

            char* errMsg = nullptr;
            if( sqlite3_exec(db, createLessonsTable, 0, 0, &errMsg) != SQLITE_OK )
            {
                m_logger.log("Database: SQL error while creating lessons table: " + std::string(errMsg), tools::LogLevel::PROBLEM);
                sqlite3_free(errMsg);
                return false;
            }
            if( sqlite3_exec(db, createWordsTable, 0, 0, &errMsg) != SQLITE_OK )
            {
                m_logger.log("Database: SQL error while creating words table: " + std::string(errMsg), tools::LogLevel::PROBLEM);
                sqlite3_free(errMsg);
                return false;
            }
            if( sqlite3_exec(db, createTagsTable, 0, 0, &errMsg) != SQLITE_OK )
            {
                m_logger.log("Database: SQL error while creating tags table: " + std::string(errMsg), tools::LogLevel::PROBLEM);
                sqlite3_free(errMsg);
                return false;
            }
            if( sqlite3_exec(db, createSettingsTable, 0, 0, &errMsg) != SQLITE_OK )
            {
                m_logger.log("Database: SQL error while creating settings table: " + std::string(errMsg), tools::LogLevel::PROBLEM);
                sqlite3_free(errMsg);
                return false;
            }

            return true;
        }

        int ApplicationDatabase::addLesson(const std::string& mainName, const std::string& subName)
        {
            const char* sql = "INSERT INTO lessons (main_name, sub_name) VALUES (?, ?);";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_text(stmt, 1, mainName.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, subName.c_str(), -1, SQLITE_STATIC);
                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    m_logger.log("Database: SQL error while adding lesson: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                    sqlite3_finalize(stmt);
                    return -1;
                }
                int lessonId = static_cast<int>(sqlite3_last_insert_rowid(db));
                sqlite3_finalize(stmt);
                m_logger.log("Database: Added lesson with ID " + std::to_string(lessonId) + ", mainName: " + mainName + ", subName: " + subName, tools::LogLevel::INFO);
                return lessonId;
            }
            return -1;
        }

        int ApplicationDatabase::addWord(int lessonId, const Word& word)
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
                    m_logger.log("Database: SQL error while adding word: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                    sqlite3_finalize(stmt);
                    return -1;
                }
                int wordId = static_cast<int>(sqlite3_last_insert_rowid(db));
                sqlite3_finalize(stmt);
                m_logger.log("Database: Added word with ID " + std::to_string(wordId) + " to lesson ID " + std::to_string(lessonId), tools::LogLevel::INFO);
                return wordId;
            }
            return -1;
        }

        void ApplicationDatabase::addTag(int wordId, const std::string& tag)
        {
            const char* sql = "INSERT INTO tags (word_id, tag) VALUES (?, ?);";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_int(stmt, 1, wordId);
                sqlite3_bind_text(stmt, 2, tag.c_str(), -1, SQLITE_STATIC);
                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    m_logger.log("Database: SQL error while adding tag: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                }
                sqlite3_finalize(stmt);
                m_logger.log("Database: Added tag '" + tag + "' to word ID " + std::to_string(wordId), tools::LogLevel::INFO);
            }
        }

        void ApplicationDatabase::updateLesson(int lessonId, const std::string& newMainName, const std::string& newSubName)
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
                    m_logger.log("Database: SQL error while updating lesson: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                }
                sqlite3_finalize(stmt);
                m_logger.log("Database: Updated lesson ID " + std::to_string(lessonId) + " to mainName: " + newMainName + ", subName: " + newSubName, tools::LogLevel::INFO);
            }
        }

        bool ApplicationDatabase::editLesson(const Lesson& lesson)
        {
            try
            {
                // Start transaction
                const char* beginTransaction = "BEGIN TRANSACTION;";
                sqlite3_exec(db, beginTransaction, 0, 0, 0);

                // Check if lesson exists
                const char* checkLessonSql = "SELECT COUNT(*) FROM lessons WHERE id = ?;";
                sqlite3_stmt* checkLessonStmt;
                bool lessonExists = false;
                if( sqlite3_prepare_v2(db, checkLessonSql, -1, &checkLessonStmt, 0) == SQLITE_OK )
                {
                    sqlite3_bind_int(checkLessonStmt, 1, lesson.id);
                    if( sqlite3_step(checkLessonStmt) == SQLITE_ROW )
                    {
                        lessonExists = sqlite3_column_int(checkLessonStmt, 0) > 0;
                    }
                    sqlite3_finalize(checkLessonStmt);
                }

                if( lessonExists )
                {
                    // Update lesson details
                    const char* updateLessonSql = "UPDATE lessons SET main_name = ?, sub_name = ? WHERE id = ?;";
                    sqlite3_stmt* updateLessonStmt;
                    if( sqlite3_prepare_v2(db, updateLessonSql, -1, &updateLessonStmt, 0) == SQLITE_OK )
                    {
                        sqlite3_bind_text(updateLessonStmt, 1, lesson.mainName.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_text(updateLessonStmt, 2, lesson.subName.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_int(updateLessonStmt, 3, lesson.id);
                        if( sqlite3_step(updateLessonStmt) != SQLITE_DONE )
                        {
                            m_logger.log("Database: SQL error while updating lesson: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                            sqlite3_finalize(updateLessonStmt);
                            throw std::runtime_error("Failed to update lesson");
                        }
                        sqlite3_finalize(updateLessonStmt);
                    }

                    // Delete existing words associated with the lesson
                    const char* deleteWordsSql = "DELETE FROM words WHERE lesson_id = ?;";
                    sqlite3_stmt* deleteWordsStmt;
                    if( sqlite3_prepare_v2(db, deleteWordsSql, -1, &deleteWordsStmt, 0) == SQLITE_OK )
                    {
                        sqlite3_bind_int(deleteWordsStmt, 1, lesson.id);
                        if( sqlite3_step(deleteWordsStmt) != SQLITE_DONE )
                        {
                            m_logger.log("Database: SQL error while deleting words: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                            sqlite3_finalize(deleteWordsStmt);
                            throw std::runtime_error("Failed to delete words");
                        }
                        sqlite3_finalize(deleteWordsStmt);
                    }
                }
                else
                {
                    // Insert new lesson
                    const char* insertLessonSql = "INSERT INTO lessons (id, main_name, sub_name) VALUES (?, ?, ?);";
                    sqlite3_stmt* insertLessonStmt;
                    if( sqlite3_prepare_v2(db, insertLessonSql, -1, &insertLessonStmt, 0) == SQLITE_OK )
                    {
                        sqlite3_bind_int(insertLessonStmt, 1, lesson.id);
                        sqlite3_bind_text(insertLessonStmt, 2, lesson.mainName.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_text(insertLessonStmt, 3, lesson.subName.c_str(), -1, SQLITE_STATIC);
                        if( sqlite3_step(insertLessonStmt) != SQLITE_DONE )
                        {
                            m_logger.log("Database: SQL error while inserting lesson: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                            sqlite3_finalize(insertLessonStmt);
                            throw std::runtime_error("Failed to insert lesson");
                        }
                        sqlite3_finalize(insertLessonStmt);
                    }
                }

                // Insert updated words
                const char* insertWordSql = "INSERT INTO words (lesson_id, kana, translation, romaji, example_sentence) VALUES (?, ?, ?, ?, ?);";
                sqlite3_stmt* insertWordStmt;
                for( const auto& word : lesson.words )
                {
                    if( sqlite3_prepare_v2(db, insertWordSql, -1, &insertWordStmt, 0) == SQLITE_OK )
                    {
                        sqlite3_bind_int(insertWordStmt, 1, lesson.id);
                        sqlite3_bind_text(insertWordStmt, 2, word.kana.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_text(insertWordStmt, 3, word.translation.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_text(insertWordStmt, 4, word.romaji.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_text(insertWordStmt, 5, word.exampleSentence.c_str(), -1, SQLITE_STATIC);
                        if( sqlite3_step(insertWordStmt) != SQLITE_DONE )
                        {
                            m_logger.log("Database: SQL error while inserting word: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                            sqlite3_finalize(insertWordStmt);
                            throw std::runtime_error("Failed to insert word");
                        }
                        int wordId = static_cast<int>(sqlite3_last_insert_rowid(db));
                        sqlite3_finalize(insertWordStmt);

                        // Insert tags for the word
                        const char* insertTagSql = "INSERT INTO tags (word_id, tag) VALUES (?, ?);";
                        sqlite3_stmt* insertTagStmt;
                        for( const auto& tag : word.tags )
                        {
                            if( sqlite3_prepare_v2(db, insertTagSql, -1, &insertTagStmt, 0) == SQLITE_OK )
                            {
                                sqlite3_bind_int(insertTagStmt, 1, wordId);
                                sqlite3_bind_text(insertTagStmt, 2, tag.c_str(), -1, SQLITE_STATIC);
                                if( sqlite3_step(insertTagStmt) != SQLITE_DONE )
                                {
                                    m_logger.log("Database: SQL error while inserting tag: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                                    sqlite3_finalize(insertTagStmt);
                                    throw std::runtime_error("Failed to insert tag");
                                }
                                sqlite3_finalize(insertTagStmt);
                            }
                        }
                    }
                }

                // Commit transaction
                const char* commitTransaction = "COMMIT;";
                sqlite3_exec(db, commitTransaction, 0, 0, 0);
                return true;
            }
            catch( const std::exception& e )
            {
                // Rollback transaction in case of error
                const char* rollbackTransaction = "ROLLBACK;";
                sqlite3_exec(db, rollbackTransaction, 0, 0, 0);
                std::cerr << "Error updating lesson: " << e.what() << std::endl;
                return false;
            }
        }

        void ApplicationDatabase::updateWord(int wordId, const Word& updatedWord)
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
                    m_logger.log("Database: SQL error while updating word: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                }
                sqlite3_finalize(stmt);
                m_logger.log("Database: Updated word ID " + std::to_string(wordId), tools::LogLevel::INFO);
            }
        }

        void ApplicationDatabase::deleteLesson(int lessonId)
        {
            const char* sql = "DELETE FROM lessons WHERE id = ?;";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_int(stmt, 1, lessonId);
                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    m_logger.log("Database: SQL error while deleting lesson: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                }
                sqlite3_finalize(stmt);
                m_logger.log("Database: Deleted lesson ID " + std::to_string(lessonId), tools::LogLevel::INFO);
            }
        }

        void ApplicationDatabase::deleteWord(int wordId)
        {
            const char* sql = "DELETE FROM words WHERE id = ?;";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_int(stmt, 1, wordId);
                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    m_logger.log("Database: SQL error while deleting word: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                }
                sqlite3_finalize(stmt);
                m_logger.log("Database: Deleted word ID " + std::to_string(wordId), tools::LogLevel::INFO);
            }
        }

        std::vector<std::string> ApplicationDatabase::getLessonNames() const
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

        std::vector<Word> ApplicationDatabase::getWordsInLesson(int lessonId) const
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

        std::vector<Lesson> ApplicationDatabase::getAllLessons() const
        {
            std::vector<Lesson> lessons;
            const char* sql = "SELECT id, main_name, sub_name FROM lessons;";
            sqlite3_stmt* stmt;
            m_logger.log("Database: Loading lessons.", tools::LogLevel::INFO);

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

        void ApplicationDatabase::saveSettings(const ApplicationSettings& settings)
        {
            m_logger.log("Database: Saving application settings.", tools::LogLevel::INFO);

            const char* sql = "REPLACE INTO settings (key, value) VALUES (?, ?);";
            sqlite3_stmt* stmt;

            auto saveSetting = [&](const char* key, const std::string& value)
                {
                    if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
                    {
                        sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
                        sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_STATIC);
                        if( sqlite3_step(stmt) != SQLITE_DONE )
                        {
                            m_logger.log("Database: SQL error while saving setting " + std::string(key) + ": " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                        }
                        sqlite3_finalize(stmt);
                    }
                };

            saveSetting("userName", settings.userName);
            saveSetting("dictionaryPath", settings.dictionaryPath);
            saveSetting("inputWord", settings.inputWord);
            saveSetting("translatedWord", settings.translatedWord);
        }

        ApplicationSettings ApplicationDatabase::loadSettings()
        {
            ApplicationSettings settings;
            const char* sql = "SELECT value FROM settings WHERE key = ?;";
            sqlite3_stmt* stmt;

            m_logger.log("Database: Loading application settings.", tools::LogLevel::INFO);

            auto loadSetting = [&](const char* key, std::string& value)
                {
                    if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
                    {
                        sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
                        if( sqlite3_step(stmt) == SQLITE_ROW )
                        {
                            value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                        }
                        sqlite3_finalize(stmt);
                    }
                };

            loadSetting("userName", settings.userName);
            loadSetting("dictionaryPath", settings.dictionaryPath);
            loadSetting("inputWord", settings.inputWord);
            loadSetting("translatedWord", settings.translatedWord);

            return settings;
        }
    }
}
