//#include "gtest/gtest.h"
//#include "gui/widgets/WidgetTypes.h"
//#include "lessons/Lesson.h"
//#include "Tools/DataPackage.h"
//#include "gui/helpers/LessonDataDecoder.h"
//#include "gui/widgets/LessonTreeViewWidget.h"
//
//using namespace tadaima;
//using namespace tadaima::gui::widget;
//
//class LessonDataDecoderTest : public ::testing::Test
//{
//protected:
//    LessonDataDecoder decoder;
//    LessonTreeViewWidget::LessonDataPackage lessonDataPackage;
//
//    void SetUp() override
//    {
//        tools::DataPackage::Identifier id = 1;
//        lessonDataPackage = LessonTreeViewWidget::LessonDataPackage(id);
//
//        LessonPackage lessonPackage1(1);
//        lessonPackage1.set(LessonDataKey::id, 1);
//        lessonPackage1.set(LessonDataKey::MainName, std::string("Main Lesson 1"));
//        lessonPackage1.set(LessonDataKey::SubName, std::string("Sub Lesson 1"));
//
//        WordDataPackage wordPackage1(1);
//        wordPackage1.set(LessonWordDataKey::id, 1);
//        wordPackage1.set(LessonWordDataKey::Kana, std::string("kana1"));
//        wordPackage1.set(LessonWordDataKey::Translation, std::string("translation1"));
//        wordPackage1.set(LessonWordDataKey::Romaji, std::string("romaji1"));
//        wordPackage1.set(LessonWordDataKey::ExampleSentence, std::string("example sentence 1"));
//        wordPackage1.set(LessonWordDataKey::Tags, std::vector<std::string>{"tag1", "tag2"});
//
//        lessonPackage1.set(LessonDataKey::Words, std::vector<WordDataPackage>{wordPackage1});
//
//        LessonPackage lessonPackage2(2);
//        lessonPackage2.set(LessonDataKey::id, 2);
//        lessonPackage2.set(LessonDataKey::MainName, std::string("Main Lesson 2"));
//        lessonPackage2.set(LessonDataKey::SubName, std::string("Sub Lesson 2"));
//
//        WordDataPackage wordPackage2(2);
//        wordPackage2.set(LessonWordDataKey::id, 2);
//        wordPackage2.set(LessonWordDataKey::Kana, std::string("kana2"));
//        wordPackage2.set(LessonWordDataKey::Translation, std::string("translation2"));
//        wordPackage2.set(LessonWordDataKey::Romaji, std::string("romaji2"));
//        wordPackage2.set(LessonWordDataKey::ExampleSentence, std::string("example sentence 2"));
//        wordPackage2.set(LessonWordDataKey::Tags, std::vector<std::string>{"tag3", "tag4"});
//
//        lessonPackage2.set(LessonDataKey::Words, std::vector<WordDataPackage>{wordPackage2});
//
//        lessonDataPackage.set(LessonTreeViewWidget::PackageKey::LessonsPackage, std::vector<LessonPackage>{lessonPackage1, lessonPackage2});
//    }
//};
//
//TEST_F(LessonDataDecoderTest, DecodeValidLessonDataPackage)
//{
//    std::vector<Lesson> lessons = decoder.decodeLessonDataPackage(&lessonDataPackage);
//
//    ASSERT_EQ(lessons.size(), 2);
//
//    EXPECT_EQ(lessons[0].id, 1);
//    EXPECT_EQ(lessons[0].mainName, "Main Lesson 1");
//    EXPECT_EQ(lessons[0].subName, "Sub Lesson 1");
//    ASSERT_EQ(lessons[0].words.size(), 1);
//    EXPECT_EQ(lessons[0].words[0].id, 1);
//    EXPECT_EQ(lessons[0].words[0].kana, "kana1");
//    EXPECT_EQ(lessons[0].words[0].translation, "translation1");
//    EXPECT_EQ(lessons[0].words[0].romaji, "romaji1");
//    EXPECT_EQ(lessons[0].words[0].exampleSentence, "example sentence 1");
//    EXPECT_EQ(lessons[0].words[0].tags, std::vector<std::string>({ "tag1", "tag2" }));
//
//    EXPECT_EQ(lessons[1].id, 2);
//    EXPECT_EQ(lessons[1].mainName, "Main Lesson 2");
//    EXPECT_EQ(lessons[1].subName, "Sub Lesson 2");
//    ASSERT_EQ(lessons[1].words.size(), 1);
//    EXPECT_EQ(lessons[1].words[0].id, 2);
//    EXPECT_EQ(lessons[1].words[0].kana, "kana2");
//    EXPECT_EQ(lessons[1].words[0].translation, "translation2");
//    EXPECT_EQ(lessons[1].words[0].romaji, "romaji2");
//    EXPECT_EQ(lessons[1].words[0].exampleSentence, "example sentence 2");
//    EXPECT_EQ(lessons[1].words[0].tags, std::vector<std::string>({ "tag3", "tag4" }));
//}
//
//TEST_F(LessonDataDecoderTest, DecodeNullDataPackage)
//{
//    EXPECT_THROW(decoder.decodeLessonDataPackage(nullptr), std::invalid_argument);
//}
//
//TEST_F(LessonDataDecoderTest, DecodeInvalidDataPackage)
//{
//    tools::DataPackage invalidDataPackage(1);
//    EXPECT_THROW(decoder.decodeLessonDataPackage(&invalidDataPackage), std::invalid_argument);
//}
