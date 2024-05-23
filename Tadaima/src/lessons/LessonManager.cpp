#include "LessonManager.h"
#include "tools/Database.h"

namespace tadaima
{
    LessonManager::LessonManager(Database& db) : m_database(db) {}

    int LessonManager::addLesson(const std::string& mainName, const std::string& subName)
    {
        return m_database.addLesson(mainName, subName);
    }

    void LessonManager::addWordToLesson(int lessonId, const Word& word)
    {
        int wordId = m_database.addWord(lessonId, word);
        for( const auto& tag : word.tags )
        {
            m_database.addTag(wordId, tag);
        }
    }

    int LessonManager::addLesson(const Lesson& lesson)
    {
        // Add the lesson to the database
        int lessonId = m_database.addLesson(lesson.mainName, lesson.subName);

        // Add all words associated with the lesson
        for( const auto& word : lesson.words )
        {
            addWordToLesson(lessonId, word);
        }

        return lessonId;
    }

    void LessonManager::addLessons(const std::vector<Lesson>& lessons)
    {
        // Iterate over each lesson and add it to the database
        for( const auto& lesson : lessons )
        {
            addLesson(lesson);
        }
    }

    void LessonManager::renameLessons(const std::vector<Lesson>& lessons)
    {
        // Iterate over each lesson and add it to the database
        for( const auto& lesson : lessons )
        {
            renameLesson(lesson.id, lesson.mainName, lesson.subName);
        }
    }

    void LessonManager::renameLesson(int lessonId, const std::string& newMainName, const std::string& newSubName)
    {
        m_database.updateLesson(lessonId, newMainName, newSubName);
    }

    std::vector<std::string> LessonManager::getLessonNames() const
    {
        return m_database.getLessonNames();
    }

    std::vector<Word> LessonManager::getWordsInLesson(int lessonId) const
    {
        return m_database.getWordsInLesson(lessonId);
    }

    std::vector<Lesson> LessonManager::getAllLessons() const
    {
        return m_database.getAllLessons();
    }

}