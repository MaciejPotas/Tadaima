#include <gtest/gtest.h>


namespace tadaima
{
    namespace gui
    {
        namespace widget
        {

            class LessonDataPackageTest : public ::testing::Test
            {
            protected:
                LessonDataPackageTest() : package() {}

                LessonDataPackage package;
            };

            TEST_F(LessonDataPackageTest, ConstructorInitializesPackageType)
            {
                EXPECT_EQ(package.id(), static_cast<tools::DataPackage::Identifier>(PackageType::Lessons));
            }

            TEST_F(LessonDataPackageTest, SetAndGetLessonPackages)
            {
                LessonPackage lessonPackage1;
                lessonPackage1.set(LessonDataKey::id, 1);
                lessonPackage1.set(LessonDataKey::MainName, std::string("MainName1"));
                lessonPackage1.set(LessonDataKey::SubName, std::string("SubName1"));

                std::vector<LessonPackage> lessonPackages = { lessonPackage1 };
                package.set(LessonPackageKey::LessonsPackage, lessonPackages);

                auto retrievedLessonPackages = package.get<std::vector<LessonPackage>>(LessonPackageKey::LessonsPackage);
                ASSERT_EQ(retrievedLessonPackages.size(), 1);
                EXPECT_EQ(retrievedLessonPackages[0].get<int>(LessonDataKey::id), 1);
                EXPECT_EQ(retrievedLessonPackages[0].get<std::string>(LessonDataKey::MainName), "MainName1");
                EXPECT_EQ(retrievedLessonPackages[0].get<std::string>(LessonDataKey::SubName), "SubName1");
            }

            TEST_F(LessonDataPackageTest, DecodeLessons)
            {
                LessonPackage lessonPackage1;
                lessonPackage1.set(LessonDataKey::id, 1);
                lessonPackage1.set(LessonDataKey::MainName, std::string("MainName1"));
                lessonPackage1.set(LessonDataKey::SubName, std::string("SubName1"));

                WordDataPackage wordPackage1;
                wordPackage1.set(LessonWordDataKey::id, 1);
                wordPackage1.set(LessonWordDataKey::Kana, std::string("Kana1"));
                wordPackage1.set(LessonWordDataKey::Translation, std::string("Translation1"));
                wordPackage1.set(LessonWordDataKey::Romaji, std::string("Romaji1"));
                wordPackage1.set(LessonWordDataKey::ExampleSentence, std::string("ExampleSentence1"));
                wordPackage1.set(LessonWordDataKey::Tags, std::vector<std::string>{"tag1"});

                std::vector<WordDataPackage> wordPackages = { wordPackage1 };
                lessonPackage1.set(LessonDataKey::Words, wordPackages);

                std::vector<LessonPackage> lessonPackages = { lessonPackage1 };
                package.set(LessonPackageKey::LessonsPackage, lessonPackages);

                auto decodedLessons = package.decode();
                ASSERT_EQ(decodedLessons.size(), 1);
                EXPECT_EQ(decodedLessons[0].id, 1);
                EXPECT_EQ(decodedLessons[0].mainName, "MainName1");
                EXPECT_EQ(decodedLessons[0].subName, "SubName1");

                ASSERT_EQ(decodedLessons[0].words.size(), 1);
                EXPECT_EQ(decodedLessons[0].words[0].id, 1);
                EXPECT_EQ(decodedLessons[0].words[0].kana, "Kana1");
                EXPECT_EQ(decodedLessons[0].words[0].translation, "Translation1");
                EXPECT_EQ(decodedLessons[0].words[0].romaji, "Romaji1");
                EXPECT_EQ(decodedLessons[0].words[0].exampleSentence, "ExampleSentence1");
                EXPECT_EQ(decodedLessons[0].words[0].tags.size(), 1);
                EXPECT_EQ(decodedLessons[0].words[0].tags[0], "tag1");
            }

        } // namespace widget
    } // namespace gui
} // namespace tadaima
