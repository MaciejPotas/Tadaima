#pragma once
#include <deque>
#include <unordered_set>
#include "../LessonTreeViewWidget.h"

namespace tadaima::gui::widget
{
    // LessonUtils.h
    class LessonUtils
    {
    public:

        static Lesson copyWordsToNewLesson(const std::unordered_set<int>& wordIds, const std::deque<LessonTreeViewWidget::LessonGroup>& cachedLessons);

        static Lesson findLessonWithId(int id, const std::deque<LessonTreeViewWidget::LessonGroup>& cachedLessons);

        // ... more helpers as needed
    };
}