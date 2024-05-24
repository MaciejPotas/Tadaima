#pragma once

#include "lessons/Lesson.h"
#include "Tools/DataPackage.h"
#include <vector>
#include <stdexcept>

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            class LessonDataDecoder
            {
            public:
                /**
                 * @brief Decodes a LessonDataPackage into a vector of lessons.
                 *
                 * @param dataPackage The data package containing lesson information.
                 * @return A vector of Lesson objects decoded from the data package.
                 * @throws std::invalid_argument if the dataPackage cannot be cast to LessonDataPackage.
                 */
                std::vector<Lesson> decodeLessonDataPackage(const tools::DataPackage* dataPackage)
                {
                    if( !dataPackage )
                    {
                        throw std::invalid_argument("Invalid argument: dataPackage is null.");
                    }

                    const gui::widget::LessonTreeViewWidget::LessonDataPackage* lessonDataPackage = dynamic_cast<const gui::widget::LessonTreeViewWidget::LessonDataPackage*>(dataPackage);
                    if( !lessonDataPackage )
                    {
                        throw std::invalid_argument("Invalid argument: dataPackage cannot be cast to LessonDataPackage.");
                    }

                    auto lessonPackages = lessonDataPackage->get<std::vector<LessonTreeViewWidget::LessonPackage>>(LessonTreeViewWidget::PackageKey::LessonsPackage);

                    std::vector<Lesson> lessons;

                    for( const auto& lessonPackage : lessonPackages )
                    {
                        Lesson lesson;
                        lesson.id = lessonPackage.get<int>(LessonTreeViewWidget::LessonDataKey::id);
                        lesson.mainName = lessonPackage.get<std::string>(LessonTreeViewWidget::LessonDataKey::MainName);
                        lesson.subName = lessonPackage.get<std::string>(LessonTreeViewWidget::LessonDataKey::SubName);

                        auto wordPackages = lessonPackage.get<std::vector<LessonTreeViewWidget::WordDataPackage>>(gui::widget::LessonTreeViewWidget::LessonDataKey::Words);

                        for( const auto& wordPackage : wordPackages )
                        {
                            Word word;
                            word.id = wordPackage.get<int>(LessonTreeViewWidget::WordDataKey::id);
                            word.kana = wordPackage.get<std::string>(LessonTreeViewWidget::WordDataKey::Kana);
                            word.translation = wordPackage.get<std::string>(LessonTreeViewWidget::WordDataKey::Translation);
                            word.romaji = wordPackage.get<std::string>(LessonTreeViewWidget::WordDataKey::Romaji);
                            word.exampleSentence = wordPackage.get<std::string>(LessonTreeViewWidget::WordDataKey::ExampleSentence);
                            word.tags = wordPackage.get<std::vector<std::string>>(LessonTreeViewWidget::WordDataKey::Tags);

                            lesson.words.push_back(word);
                        }

                        lessons.push_back(lesson);
                    }

                    return lessons;
                }
            };
        }
    }
}
