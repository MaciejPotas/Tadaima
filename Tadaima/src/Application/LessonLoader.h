/**
 * @file LessonDataLoader.h
 * @brief Defines the LessonDataLoader class, responsible for loading lesson data from the database into LessonDataPackages for use in the LessonTreeViewWidget.
 */

#pragma once

#include "Tools/Database.h"
#include "Gui/Widgets/LessonTreeViewWidget.h"

namespace tadaima
{
    /**
     * @brief The LessonDataLoader class loads lesson data from the database into LessonDataPackages
     * for use in the LessonTreeViewWidget.
     */
    class LessonDataLoader
    {
    public:
        /**
         * @brief Constructs a LessonDataLoader object with the specified database.
         * @param db The database to load lesson data from.
         */
        LessonDataLoader(Database& db) : db(db) {}

        /**
         * @brief Load all lessons from the database into LessonDataPackages.
         * @return A LessonDataPackage filled with data from the database.
         */
        gui::widget::LessonTreeViewWidget::LessonDataPackage loadAllLessons() const
        {
            std::vector<gui::widget::LessonTreeViewWidget::LessonPackage> lessonPackages;
            gui::widget::LessonTreeViewWidget::LessonDataPackage lessonPackage(1);
            auto lessons = db.getAllLessons();

            for( const auto& lesson : lessons )
            {
                gui::widget::LessonTreeViewWidget::LessonPackage package(lesson.id);

                package.set(gui::widget::LessonTreeViewWidget::LessonDataKey::id, lesson.id);
                package.set(gui::widget::LessonTreeViewWidget::LessonDataKey::MainName, lesson.mainName);
                package.set(gui::widget::LessonTreeViewWidget::LessonDataKey::SubName, lesson.subName);

                std::vector<gui::widget::LessonTreeViewWidget::WordDataPackage> wordPackages;
                for( const auto& word : lesson.words )
                {
                    gui::widget::LessonTreeViewWidget::WordDataPackage wordPackage(word.id);
                    wordPackage.set(gui::widget::LessonTreeViewWidget::WordDataKey::id, word.id);
                    wordPackage.set(gui::widget::LessonTreeViewWidget::WordDataKey::Kana, word.kana);
                    wordPackage.set(gui::widget::LessonTreeViewWidget::WordDataKey::Translation, word.translation);
                    wordPackage.set(gui::widget::LessonTreeViewWidget::WordDataKey::Romaji, word.romaji);
                    wordPackage.set(gui::widget::LessonTreeViewWidget::WordDataKey::ExampleSentence, word.exampleSentence);
                    wordPackage.set(gui::widget::LessonTreeViewWidget::WordDataKey::Tags, word.tags);

                    wordPackages.push_back(wordPackage);
                }

                package.set(gui::widget::LessonTreeViewWidget::LessonDataKey::Words, wordPackages);
                lessonPackages.push_back(package);
            }

            lessonPackage.set(gui::widget::LessonTreeViewWidget::PackageKey::LessonsPackage, lessonPackages);

            return lessonPackage;
        }

    private:
        /**
         * @brief Reference to the database object.
         */
        Database& db;
    };
}
