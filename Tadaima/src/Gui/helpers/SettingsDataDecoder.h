/**
 * @file SettingsDataDecoder.h
 * @brief Defines the SettingsDataDecoder class for decoding data packages into application settings.
 */

#pragma once

#include "Tools/DataPackage.h"
#include <vector>
#include <stdexcept>
#include "Application/ApplicationSettings.h"
#include "widgets/ApplicationSettingsWidget.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            /**
             * @brief The SettingsDataDecoder class provides functionality to decode data packages into application settings.
             */
            class SettingsDataDecoder
            {
            public:
                /**
                 * @brief Decodes a data package into application settings.
                 * @param dataPackage Pointer to the data package to decode.
                 * @return The decoded application settings.
                 * @throws std::invalid_argument if the data package is null or cannot be cast to SettingsDataPackage.
                 */
                application::ApplicationSettings decodeDataPackage(const tools::DataPackage* dataPackage)
                {
                    if( !dataPackage )
                    {
                        throw std::invalid_argument("Invalid argument: dataPackage is null.");
                    }

                    const ApplicationSettingsWidget::SettingsDataPackage* settingsDataPackage = dynamic_cast<const ApplicationSettingsWidget::SettingsDataPackage*>(dataPackage);
                    if( !settingsDataPackage )
                    {
                        throw std::invalid_argument("Invalid argument: dataPackage cannot be cast to settingsDataPackage.");
                    }

                    application::ApplicationSettings settings;

                    settings.userName = settingsDataPackage->get<std::string>(ApplicationSettingsWidget::PackageKey::Username);
                    settings.dictionaryPath = settingsDataPackage->get<std::string>(ApplicationSettingsWidget::PackageKey::DictionaryPath);
                    settings.inputWord = settingsDataPackage->get<std::string>(ApplicationSettingsWidget::PackageKey::InputWord);
                    settings.translatedWord = settingsDataPackage->get<std::string>(ApplicationSettingsWidget::PackageKey::TranslatedWord);

                    return settings;
                }
            };
        }
    }
}
