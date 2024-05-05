#pragma once

#include "VocabularySettingsWidget.h"
#include "imgui.h"
#include "gui.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            void VocabularySettingsWidget::initialize([[maybe_unused]] const tools::DataPackage& r_package)
            {

            }

            void VocabularySettingsWidget::draw(bool* p_open)
            {
                if( !ImGui::Begin("Vocabulary Settings", p_open) )
                {
                    ImGui::End();
                    return;
                }

                // Buttons for vocabulary actions
                if( ImGui::Button("Add New Word") )
                {
                    // Add new word logic here
                }
                ImGui::SameLine();
                if( ImGui::Button("Edit Words") )
                {
                    // Edit words logic here
                }
                ImGui::SameLine();
                if( ImGui::Button("Import/Export") )
                {
                    // Import/Export logic here
                }

                // List of words
                ImGui::Separator();
                ImGui::Text("Word List:");
                ImGui::Text("1. 車 (くるま) - Car");
                ImGui::Text("2. 学校 (がっこう) - School");
                ImGui::Text("3. 食べる (たべる) - To eat");

                bool showFurigana = true;
                bool includeExamples = true;

                // Search and settings
                ImGui::Separator();
                static char wordSearch[255];
                ImGui::InputText("Search Word", wordSearch, IM_ARRAYSIZE(wordSearch));
                ImGui::Checkbox("Show Furigana", &showFurigana);
                ImGui::Checkbox("Include Examples in Tests", &includeExamples);
                if( ImGui::Button("Save Changes") )
                {
                    // Save settings logic here
                }

                ImGui::End();
            }
        }
    }
}