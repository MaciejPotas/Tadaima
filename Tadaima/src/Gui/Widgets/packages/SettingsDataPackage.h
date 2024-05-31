/**
 * @file ApplicationSettingsWidget.h
 * @brief Defines the ApplicationSettingsWidget class for managing application settings in the GUI.
 */

#pragma once

#include "PackageType.h"
#include "Tools/DataPackage.h"
#include <cstring>
#include <string>
#include "Application/ApplicationSettings.h"

namespace tools { class Logger; }

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @brief Enum class for package keys used in settings data packages.
             */
            enum class SettingsPackageKey : uint32_t
            {
                Username,       /**< Key for username. */
                DictionaryPath, /**< Key for dictionary path. */
                InputWord,      /**< Key for input word. */
                TranslatedWord  /**< Key for translated word. */
            };

            /**
             * @brief Represents a package containing settings data.
             */
            class SettingsDataPackage : public tools::ComplexDataPackage<SettingsPackageKey, std::string>
            {
            public:

                /**
                 * @brief Constructs a SettingsDataPackage object.
                 */
                SettingsDataPackage() : ComplexDataPackage(PackageType::Settings) {}

                SettingsDataPackage(const application::ApplicationSettings& settings) : SettingsDataPackage()
                {
                    set(gui::widget::SettingsPackageKey::Username, settings.userName);
                    set(gui::widget::SettingsPackageKey::DictionaryPath, settings.dictionaryPath);
                    set(gui::widget::SettingsPackageKey::InputWord, settings.inputWord);
                    set(gui::widget::SettingsPackageKey::TranslatedWord, settings.translatedWord);
                }

                /**
                 * @brief Decodes a data package into application settings.
                 * @param dataPackage Pointer to the data package to decode.
                 * @return The decoded application settings.
                 * @throws std::invalid_argument if the data package is null or cannot be cast to SettingsDataPackage.
                 */
                application::ApplicationSettings decode() const
                {
                    application::ApplicationSettings settings;

                    settings.userName = get<std::string>(SettingsPackageKey::Username);
                    settings.dictionaryPath = get<std::string>(SettingsPackageKey::DictionaryPath);
                    settings.inputWord = get<std::string>(SettingsPackageKey::InputWord);
                    settings.translatedWord = get<std::string>(SettingsPackageKey::TranslatedWord);

                    return settings;
                }
            };

        }
    }
}
