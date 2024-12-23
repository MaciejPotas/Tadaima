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
                "kanji TEXT, "  // New Kanji column
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

            const char* createConjugationsTable =
                "CREATE TABLE IF NOT EXISTS conjugations ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "word_id INTEGER NOT NULL, "
                "type INTEGER NOT NULL, " // Stores ConjugationType as an integer
                "conjugated_word TEXT NOT NULL, "
                "FOREIGN KEY(word_id) REFERENCES words(id));";

            const char* alterLessonsTable = "ALTER TABLE lessons ADD COLUMN group_name TEXT;";

            const char* alterWordsTable = "ALTER TABLE words ADD COLUMN kanji TEXT;";

            char* errMsg = nullptr;

            if( sqlite3_exec(db, createLessonsTable, 0, 0, &errMsg) != SQLITE_OK )
            {
                m_logger.log("Database: SQL error while creating lessons table: " + std::string(errMsg), tools::LogLevel::PROBLEM);
                sqlite3_free(errMsg);
                return false;
            }

            if( sqlite3_exec(db, alterLessonsTable, 0, 0, &errMsg) != SQLITE_OK )
            {
                std::string errorMsg = std::string(errMsg);
                if( errorMsg.find("duplicate column name") == std::string::npos )
                {
                    m_logger.log("Database: SQL error while altering lessons table: " + errorMsg, tools::LogLevel::PROBLEM);
                    sqlite3_free(errMsg);
                    return false;
                }
                sqlite3_free(errMsg); // Free the error message
            }

            // Backfill group_name for existing rows where it is NULL
            const char* backfillGroupName =
                "UPDATE lessons "
                "SET group_name = SUBSTR(main_name, 1, INSTR(main_name, ' ') - 1) "
                "WHERE group_name IS NULL OR group_name = '';";

            if( sqlite3_exec(db, backfillGroupName, 0, 0, &errMsg) != SQLITE_OK )
            {
                m_logger.log("Database: SQL error while backfilling group_name: " + std::string(errMsg), tools::LogLevel::PROBLEM);
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

            // Attempt to alter the `words` table to add `kanji` column
            if( sqlite3_exec(db, alterWordsTable, 0, 0, &errMsg) != SQLITE_OK )
            {
                // Ignore the error if it indicates that the column already exists
                std::string errorMsg = std::string(errMsg);
                if( errorMsg.find("duplicate column name") == std::string::npos )
                {
                    m_logger.log("Database: SQL error while altering words table: " + errorMsg, tools::LogLevel::PROBLEM);
                    sqlite3_free(errMsg);
                    return false;
                }
                sqlite3_free(errMsg);  // Free the error message
            }

            // Attempt to alter the lessons table to add group_name column
            if( sqlite3_exec(db, alterLessonsTable, 0, 0, &errMsg) != SQLITE_OK )
            {
                // Ignore the error if it indicates that the column already exists
                std::string errorMsg = std::string(errMsg);
                if( errorMsg.find("duplicate column name") == std::string::npos )
                {
                    m_logger.log("Database: SQL error while altering lessons table: " + errorMsg, tools::LogLevel::PROBLEM);
                    sqlite3_free(errMsg);
                    return false;
                }
                sqlite3_free(errMsg);
            }

            if( sqlite3_exec(db, createConjugationsTable, 0, 0, &errMsg) != SQLITE_OK )
            {
                m_logger.log("Database: SQL error while creating conjugations table: " + std::string(errMsg), tools::LogLevel::PROBLEM);
                sqlite3_free(errMsg);
                return false;
            }

            return true;
        }

        int ApplicationDatabase::addLesson(const std::string& mainName, const std::string& subName, const std::string& groupName)
        {
            const char* sql = "INSERT INTO lessons (main_name, sub_name, group_name) VALUES (?, ?, ?);";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_text(stmt, 1, mainName.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, subName.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, groupName.c_str(), -1, SQLITE_STATIC);
                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    m_logger.log("Database: SQL error while adding lesson: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                    sqlite3_finalize(stmt);
                    return -1;
                }
                int lessonId = static_cast<int>(sqlite3_last_insert_rowid(db));
                sqlite3_finalize(stmt);
                m_logger.log("Database: Added lesson with ID " + std::to_string(lessonId) + ", mainName: " + mainName + ", subName: " + subName + ", groupName: " + groupName, tools::LogLevel::INFO);
                return lessonId;
            }
            return -1;
        }

        int ApplicationDatabase::addWord(int lessonId, const Word& word)
        {
            const char* sql =
                "INSERT INTO words (lesson_id, kana, kanji, translation, romaji, example_sentence) "
                "VALUES (?, ?, ?, ?, ?, ?);";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_int(stmt, 1, lessonId);
                sqlite3_bind_text(stmt, 2, word.kana.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, word.kanji.empty() ? "N/A" : word.kanji.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 4, word.translation.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 5, word.romaji.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 6, word.exampleSentence.c_str(), -1, SQLITE_STATIC);

                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    m_logger.log("Database: SQL error while adding word: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                    sqlite3_finalize(stmt);
                    return -1;
                }

                int wordId = static_cast<int>(sqlite3_last_insert_rowid(db));
                sqlite3_finalize(stmt);

                // Add conjugations if they exist
                for( int i = 0; i < CONJUGATION_COUNT; ++i )
                {
                    if( !word.conjugations[i].empty() )
                    {
                        addConjugation(wordId, static_cast<ConjugationType>(i), word.conjugations[i]);
                    }
                }

                m_logger.log("Database: Added word with ID " + std::to_string(wordId) + " to lesson ID " + std::to_string(lessonId), tools::LogLevel::INFO);
                return wordId;
            }
            m_logger.log("Database: Failed to prepare statement for adding word.", tools::LogLevel::PROBLEM);
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

        void ApplicationDatabase::updateLesson(int lessonId, const std::string& newGroupName, const std::string& newMainName, const std::string& newSubName)
        {
            const char* sql = "UPDATE lessons SET group_name = ?, main_name = ?, sub_name = ? WHERE id = ?;";
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_text(stmt, 1, newGroupName.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, newMainName.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, newSubName.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(stmt, 4, lessonId);

                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    m_logger.log("Database: SQL error while updating lesson: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                }
                else
                {
                    m_logger.log("Database: Updated lesson ID " + std::to_string(lessonId) + " to groupName: " + newGroupName + ", mainName: " + newMainName + ", subName: " + newSubName, tools::LogLevel::INFO);
                }

                sqlite3_finalize(stmt);
            }
            else
            {
                m_logger.log("Database: Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
            }
        }

        bool ApplicationDatabase::editLesson(const Lesson& lesson)
        {
            try
            {
                int lessonId = lesson.id;

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
                    // Update lesson details, including groupName
                    const char* updateLessonSql = "UPDATE lessons SET group_name = ?, main_name = ?, sub_name = ? WHERE id = ?;";
                    sqlite3_stmt* updateLessonStmt;
                    if( sqlite3_prepare_v2(db, updateLessonSql, -1, &updateLessonStmt, 0) == SQLITE_OK )
                    {
                        sqlite3_bind_text(updateLessonStmt, 1, lesson.groupName.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_text(updateLessonStmt, 2, lesson.mainName.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_text(updateLessonStmt, 3, lesson.subName.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_int(updateLessonStmt, 4, lesson.id);
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
                    // Insert new lesson with groupName
                    int newLessonId = addLesson(lesson.mainName, lesson.subName, lesson.groupName);
                    if( newLessonId == -1 )
                    {
                        throw std::runtime_error("Failed to insert lesson");
                    }
                    lessonId = newLessonId;
                }

                // Insert updated words
                const char* insertWordSql = "INSERT INTO words (lesson_id, kana, kanji, translation, romaji, example_sentence) VALUES (?, ?, ?, ?, ?, ?);";
                sqlite3_stmt* insertWordStmt;
                for( const auto& word : lesson.words )
                {
                    if( sqlite3_prepare_v2(db, insertWordSql, -1, &insertWordStmt, 0) == SQLITE_OK )
                    {
                        sqlite3_bind_int(insertWordStmt, 1, lessonId);
                        sqlite3_bind_text(insertWordStmt, 2, word.kana.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_text(insertWordStmt, 3, word.kanji.empty() ? "N/A" : word.kanji.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_text(insertWordStmt, 4, word.translation.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_text(insertWordStmt, 5, word.romaji.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_text(insertWordStmt, 6, word.exampleSentence.c_str(), -1, SQLITE_STATIC);
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

                        // Add conjugations if they exist
                        for( int i = 0; i < CONJUGATION_COUNT; ++i )
                        {
                            if( !word.conjugations[i].empty() )
                            {
                                addConjugation(wordId, static_cast<ConjugationType>(i), word.conjugations[i]);
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
                m_logger.log("Error updating lesson: " + std::string(e.what()), tools::LogLevel::PROBLEM);
                return false;
            }
        }

        void ApplicationDatabase::updateWord(int wordId, const Word& updatedWord)
        {
            const char* sql = "UPDATE words SET kana = ?, kanji = ?, translation = ?, romaji = ?, example_sentence = ? WHERE id = ?;";  // Added kanji column
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_text(stmt, 1, updatedWord.kana.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, updatedWord.kanji.empty() ? "N/A" : updatedWord.kanji.c_str(), -1, SQLITE_STATIC);  // Bind kanji with default value if empty
                sqlite3_bind_text(stmt, 3, updatedWord.translation.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 4, updatedWord.romaji.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 5, updatedWord.exampleSentence.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(stmt, 6, wordId);

                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    m_logger.log("Database: SQL error while updating word: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                }
                else
                {
                    m_logger.log("Database: Updated word ID " + std::to_string(wordId), tools::LogLevel::INFO);
                }
                sqlite3_finalize(stmt);
            }
            else
            {
                m_logger.log("Database: Failed to prepare statement for updating word.", tools::LogLevel::PROBLEM);
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
            const char* sql = "SELECT id, kana, kanji, translation, romaji, example_sentence FROM words WHERE lesson_id = ?;"; // Added kanji to the SELECT statement
            sqlite3_stmt* stmt;
            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_int(stmt, 1, lessonId);
                while( sqlite3_step(stmt) == SQLITE_ROW )
                {
                    Word word;
                    word.id = sqlite3_column_int(stmt, 0);
                    word.kana = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                    // Check if the kanji column is NULL
                    const char* kanjiText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                    word.kanji = kanjiText ? kanjiText : "N/A";  // Use "N/A" if kanji is NULL
                    word.translation = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                    word.romaji = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                    word.exampleSentence = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

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

                    // Fetch conjugations for the word
                    word.conjugations = getConjugations(word.id);

                    words.push_back(word);
                }
                sqlite3_finalize(stmt);
            }
            return words;
        }

        std::vector<Lesson> ApplicationDatabase::getAllLessons() const
        {
            std::vector<Lesson> lessons;
            const char* sql = "SELECT id, main_name, sub_name, group_name FROM lessons;";
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
                    const char* groupNameText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                    if( groupNameText && strlen(groupNameText) > 0 )
                    {
                        // If groupNameText is not null and not empty, use it
                        lesson.groupName = groupNameText;
                    }
                    else
                    {
                        // Derive group name from mainName
                        size_t pos = lesson.mainName.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
                        lesson.groupName = (pos != std::string::npos) ? lesson.mainName.substr(0, pos) : lesson.mainName;
                    }
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
            saveSetting("QuizzesPath", settings.quizzesPaths);
            saveSetting("inputWord", settings.inputWord);
            saveSetting("translatedWord", settings.translatedWord);
            saveSetting("showLogs", settings.showLogs ? "true" : "false");
            saveSetting("maxTriesForQuiz", settings.maxTriesForQuiz); // New field for quiz max tries
            saveSetting("ConjugationPath", settings.conjugationPath);
            saveSetting("ConjugationMask", std::to_string(settings.conjugationMask));
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

            std::string showLogs = "";
            std::string conjugationMask = "";
            loadSetting("userName", settings.userName);
            loadSetting("dictionaryPath", settings.dictionaryPath);
            loadSetting("QuizzesPath", settings.quizzesPaths);
            loadSetting("inputWord", settings.inputWord);
            loadSetting("translatedWord", settings.translatedWord);
            loadSetting("showLogs", showLogs);
            settings.showLogs = showLogs == "true" ? true : false;
            loadSetting("maxTriesForQuiz", settings.maxTriesForQuiz);
            loadSetting("ConjugationPath", settings.conjugationPath);
            loadSetting("ConjugationMask", conjugationMask);
            if( conjugationMask != "" )
                settings.conjugationMask = static_cast<uint16_t>(std::stoi(conjugationMask));

            return settings;
        }

        void ApplicationDatabase::addConjugation(int wordId, ConjugationType type, const std::string& conjugatedWord)
        {
            const char* sql = "INSERT INTO conjugations (word_id, type, conjugated_word) VALUES (?, ?, ?);";
            sqlite3_stmt* stmt;

            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_int(stmt, 1, wordId);
                sqlite3_bind_int(stmt, 2, static_cast<int>(type));
                sqlite3_bind_text(stmt, 3, conjugatedWord.c_str(), -1, SQLITE_STATIC);

                if( sqlite3_step(stmt) != SQLITE_DONE )
                {
                    m_logger.log("Database: SQL error while adding conjugation: " + std::string(sqlite3_errmsg(db)), tools::LogLevel::PROBLEM);
                }
                else
                {
                    m_logger.log("Database: Added conjugation for word ID " + std::to_string(wordId) + " (" + std::to_string(type) + ": " + conjugatedWord + ")", tools::LogLevel::INFO);
                }

                sqlite3_finalize(stmt);
            }
            else
            {
                m_logger.log("Database: Failed to prepare statement for adding conjugation.", tools::LogLevel::PROBLEM);
            }
        }

        std::array<std::string, CONJUGATION_COUNT> ApplicationDatabase::getConjugations(int wordId) const
        {
            std::array<std::string, CONJUGATION_COUNT> conjugations;
            conjugations.fill(""); // Initialize all entries as empty strings

            const char* sql = "SELECT type, conjugated_word FROM conjugations WHERE word_id = ?;";
            sqlite3_stmt* stmt;

            if( sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK )
            {
                sqlite3_bind_int(stmt, 1, wordId);

                while( sqlite3_step(stmt) == SQLITE_ROW )
                {
                    int type = sqlite3_column_int(stmt, 0);

                    // Ensure type is within valid range
                    if( type >= 0 && type < CONJUGATION_COUNT )
                    {
                        const char* conjugationText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                        conjugations[type] = conjugationText ? conjugationText : ""; // Handle NULL safely
                    }
                }

                sqlite3_finalize(stmt);
            }
            else
            {
                m_logger.log("Database: Failed to prepare statement for fetching conjugations.", tools::LogLevel::PROBLEM);
            }

            return conjugations;
        }

    }
}
