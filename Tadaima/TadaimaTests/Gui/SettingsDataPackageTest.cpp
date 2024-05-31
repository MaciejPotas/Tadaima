#include <gtest/gtest.h>
#include "gui/widgets/packages/SettingsDataPackage.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            class SettingsDataPackageTest : public ::testing::Test
            {
            protected:
                SettingsDataPackageTest() : package() {}

                SettingsDataPackage package;
            };

            TEST_F(SettingsDataPackageTest, ConstructorInitializesPackageType)
            {
                EXPECT_EQ(package.id(), static_cast<tools::DataPackage::Identifier>(PackageType::Settings));
            }

            TEST_F(SettingsDataPackageTest, SetAndGetSettings)
            {
                package.set(SettingsPackageKey::Username, std::string("testuser"));
                package.set(SettingsPackageKey::DictionaryPath, std::string("/path/to/dictionary"));
                package.set(SettingsPackageKey::InputWord, std::string("hello"));
                package.set(SettingsPackageKey::TranslatedWord, std::string("latin"));

                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::Username), "testuser");
                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::DictionaryPath), "/path/to/dictionary");
                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::InputWord), "hello");
                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::TranslatedWord), "latin");
            }

            TEST_F(SettingsDataPackageTest, DecodeSettings)
            {
                package.set(SettingsPackageKey::Username, std::string("testuser"));
                package.set(SettingsPackageKey::DictionaryPath, std::string("/path/to/dictionary"));
                package.set(SettingsPackageKey::InputWord, std::string("hello"));
                package.set(SettingsPackageKey::TranslatedWord, std::string("latin"));

                auto settings = package.decode();
                EXPECT_EQ(settings.userName, "testuser");
                EXPECT_EQ(settings.dictionaryPath, "/path/to/dictionary");
                EXPECT_EQ(settings.inputWord, "hello");
                EXPECT_EQ(settings.translatedWord, "latin");
            }

            TEST_F(SettingsDataPackageTest, HandleEmptyValues)
            {
                package.set(SettingsPackageKey::Username, std::string(""));
                package.set(SettingsPackageKey::DictionaryPath, std::string(""));
                package.set(SettingsPackageKey::InputWord, std::string(""));
                package.set(SettingsPackageKey::TranslatedWord, std::string(""));
                package.set(SettingsPackageKey::TranslatedWord, std::string(""));

                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::Username), "");
                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::DictionaryPath), "");
                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::InputWord), "");
                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::TranslatedWord), "");
            }

            TEST_F(SettingsDataPackageTest, ConstructorWithSettings)
            {
                application::ApplicationSettings settings;
                settings.userName = "testuser";
                settings.dictionaryPath = "/path/to/dictionary";
                settings.inputWord = "hello";
                settings.translatedWord = "latin";

                SettingsDataPackage packageWithSettings(settings);

                EXPECT_EQ(packageWithSettings.get<std::string>(SettingsPackageKey::Username), "testuser");
                EXPECT_EQ(packageWithSettings.get<std::string>(SettingsPackageKey::DictionaryPath), "/path/to/dictionary");
                EXPECT_EQ(packageWithSettings.get<std::string>(SettingsPackageKey::InputWord), "hello");
                EXPECT_EQ(packageWithSettings.get<std::string>(SettingsPackageKey::TranslatedWord), "latin");
            }

        } // namespace widget
    } // namespace gui
} // namespace tadaima
