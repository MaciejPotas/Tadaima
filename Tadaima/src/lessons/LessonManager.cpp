#include "LessonManager.h"
#include "tools/Database.h"

namespace tadaima
{
    LessonManager::LessonManager(Database& db) : db(db) {}

    int LessonManager::addLesson(const std::string& mainName, const std::string& subName)
    {
        return db.addLesson(mainName, subName);
    }

    void LessonManager::addWordToLesson(int lessonId, const Word& word)
    {
        int wordId = db.addWord(lessonId, word);
        for( const auto& tag : word.tags )
        {
            db.addTag(wordId, tag);
        }
    }

    std::vector<std::string> LessonManager::getLessonNames() const
    {
        return db.getLessonNames();
    }

    std::vector<Word> LessonManager::getWordsInLesson(int lessonId) const
    {
        return db.getWordsInLesson(lessonId);
    }

    std::vector<Lesson> LessonManager::getAllLessons() const
    {
        return db.getAllLessons();
    }
}