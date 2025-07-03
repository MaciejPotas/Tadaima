#include "LessonFileIO.h"
#include "Tools/pugixml.hpp"
#include "Tools/Logger.h"
#include <unordered_set>

namespace tadaima::gui::widget
{
    std::vector<Lesson> LessonFileIO::importLessons(const std::string& filePath, tools::Logger& logger)
    {
        logger.log("Parsing and importing lessons from file: " + filePath);
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(filePath.c_str());
        std::vector<Lesson> parsedLessons;

        if( !result )
        {
            logger.log("Error: Could not load XML file!");
            return parsedLessons;
        }

        for( pugi::xml_node lessonNode : doc.child("lessons").children("lesson") )
        {
            Lesson lesson;
            lesson.groupName = lessonNode.attribute("groupName").as_string();
            lesson.mainName = lessonNode.attribute("mainName").as_string();
            lesson.subName = lessonNode.attribute("subName").as_string();

            for( pugi::xml_node wordNode : lessonNode.children("word") )
            {
                Word word;
                word.translation = wordNode.attribute("translation").as_string();
                word.romaji = wordNode.attribute("romaji").as_string();
                word.kana = wordNode.attribute("kana").as_string();
                word.kanji = wordNode.attribute("kanji").as_string();
                word.exampleSentence = wordNode.attribute("example").as_string();

                // tags
                for( pugi::xml_node tagNode : wordNode.children("tag") )
                {
                    word.tags.push_back(tagNode.attribute("name").as_string());
                }

                // conjugations (optional, in case you export them)
                int conjIdx = 0;
                for( pugi::xml_node conjNode : wordNode.children("conjugation") )
                {
                    if( conjIdx < (int)word.conjugations.size() )
                        word.conjugations[conjIdx++] = conjNode.attribute("value").as_string();
                }

                lesson.words.push_back(word);
            }
            parsedLessons.push_back(lesson);
        }

        logger.log("Lessons imported from file.");
        return parsedLessons;
    }

    void LessonFileIO::exportLessons(const std::string& filePath, const std::vector<Lesson>& lessons, tools::Logger& logger)
    {
        logger.log("Exporting lessons to file: " + filePath);

        pugi::xml_document doc;
        pugi::xml_node root = doc.append_child("lessons");

        for( const auto& lesson : lessons )
        {
            pugi::xml_node lessonNode = root.append_child("lesson");
            lessonNode.append_attribute("groupName") = lesson.groupName.c_str();
            lessonNode.append_attribute("mainName") = lesson.mainName.c_str();
            lessonNode.append_attribute("subName") = lesson.subName.c_str();

            for( const auto& word : lesson.words )
            {
                pugi::xml_node wordNode = lessonNode.append_child("word");
                wordNode.append_attribute("translation") = word.translation.c_str();
                wordNode.append_attribute("romaji") = word.romaji.c_str();
                wordNode.append_attribute("kana") = word.kana.c_str();
                wordNode.append_attribute("kanji") = word.kanji.c_str();
                wordNode.append_attribute("example") = word.exampleSentence.c_str();

                for( const auto& tag : word.tags )
                {
                    pugi::xml_node tagNode = wordNode.append_child("tag");
                    tagNode.append_attribute("name") = tag.c_str();
                }

                // Export conjugations
                for( size_t i = 0; i < word.conjugations.size(); ++i )
                {
                    if( !word.conjugations[i].empty() )
                    {
                        pugi::xml_node conjNode = wordNode.append_child("conjugation");
                        conjNode.append_attribute("index") = static_cast<int>(i);
                        conjNode.text() = word.conjugations[i].c_str();
                    }
                }
            }
        }

        if( !doc.save_file(filePath.c_str()) )
        {
            logger.log("Error: Could not save XML file!", tools::LogLevel::PROBLEM);
        }
        else
        {
            logger.log("Lessons successfully exported to file: " + filePath, tools::LogLevel::INFO);
        }
    }

} // namespace tadaima::gui::widget
