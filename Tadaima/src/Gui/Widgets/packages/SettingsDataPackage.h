/**
 * @file ApplicationSettingsWidget.h
 * @brief Defines the ApplicationSettingsWidget class for managing application settings in the GUI.
 */

#pragma once

#include "PackageType.h"
#include "gui/quiz/QuizType.h"
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
                Username,               /**< Key for username. */
                DictionaryPath,         /**< Key for dictionary path. */
                ConjugationPath,        /**< Key for conjugation path */
                QuizzesScriptsPath,     /**< Key for scripted quizes path. */
                TriesForQuiz,           /**< Key for number of tries per word */
                AskedWordType,          /**< Key for input word. */
                AnswerWordType,         /**< Key for translated word. */
                ShowLogs,               /**< Key for showing console logs */
                ConjugationMask         /**< Key for conjugation path */
            };

            /**
             * @brief Represents a package containing settings data.
             */
            class SettingsDataPackage : public tools::ComplexDataPackage<SettingsPackageKey, bool, uint16_t, std::string, quiz::WordType>
            {
            public:

                /**
                 * @brief Constructs a SettingsDataPackage object.
                 */
                SettingsDataPackage() : ComplexDataPackage(PackageType::Settings) {}
            };

        }
    }
}
