#include "LessonUtils.h"
#include "lessons/Lesson.h"

namespace tadaima::gui::widget
{
    tadaima::Lesson LessonUtils::copyWordsToNewLesson(const std::unordered_set<int>& wordIds, const std::deque<LessonTreeViewWidget::LessonGroup>& cachedLessons)
    {
        Lesson newLesson;
        newLesson.groupName = "Mixed vocabulary";
        newLesson.mainName = "Mixed vocabulary";
        newLesson.subName = "Mixed vocabulary";

        std::unordered_set<int> wordIdSet(wordIds.begin(), wordIds.end());

        for( const auto& lessonGroup : cachedLessons )
        {
            for( const auto& subLessons : lessonGroup.subLessons )
            {
                for( const auto& lesson : subLessons.second )
                {
                    for( const auto& word : lesson.words )
                    {
                        if( wordIdSet.find(word.id) != wordIdSet.end() )
                        {
                            newLesson.words.push_back(word);
                        }
                    }
                }
            }
        }

        return newLesson;
    }

    tadaima::Lesson LessonUtils::findLessonWithId(int id, const std::deque<LessonTreeViewWidget::LessonGroup>& cachedLessons)
    {
        std::unordered_map<int, Lesson> lessonMap;

        for( const auto& lessonGroup : cachedLessons )
        {
            for( const auto& subLessons : lessonGroup.subLessons )
            {
                for( const auto& lesson : subLessons.second )
                {
                    lessonMap[lesson.id] = lesson;
                }
            }
        }

        auto it = lessonMap.find(id);
        if( it != lessonMap.end() )
        {
            return it->second;
        }

        return Lesson();
    }

}
