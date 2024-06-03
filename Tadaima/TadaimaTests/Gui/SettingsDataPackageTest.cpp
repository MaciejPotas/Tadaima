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
                package.set(SettingsPackageKey::AnswerWordType, std::string("hello"));
                package.set(SettingsPackageKey::AskedWordType, std::string("latin"));

                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::Username), "testuser");
                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::DictionaryPath), "/path/to/dictionary");
                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::AnswerWordType), "hello");
                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::AskedWordType), "latin");
            }

            TEST_F(SettingsDataPackageTest, HandleEmptyValues)
            {
                package.set(SettingsPackageKey::Username, std::string(""));
                package.set(SettingsPackageKey::DictionaryPath, std::string(""));
                package.set(SettingsPackageKey::AnswerWordType, std::string(""));
                package.set(SettingsPackageKey::AskedWordType, std::string(""));

                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::Username), "");
                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::DictionaryPath), "");
                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::AnswerWordType), "");
                EXPECT_EQ(package.get<std::string>(SettingsPackageKey::AskedWordType), "");
            }

        } // namespace widget
    } // namespace gui
} // namespace tadaima
