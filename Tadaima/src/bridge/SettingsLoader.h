/**
 * @file SettingsLoader.h
 * @brief Defines the SettingsLoader class for loading application settings into a data package.
 */

#pragma once

#include "Application/ApplicationSettings.h"
#include "widgets/ApplicationSettingsWidget.h"

namespace tadaima
{
    /**
     * @brief The SettingsLoader class loads application settings into a data package.
     */
    class SettingsLoader
    {
    public:
        /**
         * @brief Constructs a SettingsLoader object.
         * @param settings The application settings to load.
         */
        SettingsLoader(const application::ApplicationSettings& settings) : m_settings(settings)
        {

        }

        /**
         * @brief Loads the application settings into a SettingsDataPackage.
         * @return The loaded SettingsDataPackage.
         */
        gui::widget::ApplicationSettingsWidget::SettingsDataPackage load() const
        {
            gui::widget::ApplicationSettingsWidget::SettingsDataPackage package;
            package.set(gui::widget::ApplicationSettingsWidget::PackageKey::Username, m_settings.userName);
            package.set(gui::widget::ApplicationSettingsWidget::PackageKey::DictionaryPath, m_settings.dictionaryPath);
            package.set(gui::widget::ApplicationSettingsWidget::PackageKey::InputWord, m_settings.inputWord);
            package.set(gui::widget::ApplicationSettingsWidget::PackageKey::TranslatedWord, m_settings.translatedWord);

            return package;
        }

    private:
        const application::ApplicationSettings& m_settings; /**< Reference to the application settings. */
    };
}
