#pragma once

#include "MainDashboardWidget.h"
#include "imgui.h"
#include "gui.h"
#include "packages/SettingsDataPackage.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            void MainDashboardWidget::initialize(const tools::DataPackage& r_package)
            {
                const SettingsDataPackage* package = dynamic_cast<const SettingsDataPackage*>(&r_package);
                if( package )
                {
                    m_username = package->get<std::string>(SettingsPackageKey::Username);
                }
            }

            void MainDashboardWidget::draw(bool* p_open)
            {
                if( !ImGui::Begin("Main Dashboard", p_open, ImGuiWindowFlags_NoDecoration) )
                {
                    ImGui::End();
                    return;
                }

                // Header
                ImGui::Text((const char*)std::format("Ohayou, {}!", m_username).c_str());
                ImGui::Text((const char*)u8"I am so lucky to see you again here :-)");
                ImGui::Text((const char*)u8"Let's learn some new words together!");
                ImGui::ProgressBar(0.65f, ImVec2(-1.0f, 0.0f), "65% complete");

                // Word of the Day
                ImGui::Separator();
                ImGui::Text((const char*)u8"Word of the Day: \"茶\" (ちゃ) - Tea");

                // Performance Graphs
                ImGui::Separator();
                ImGui::Text("Performance Graphs:");
                ImGui::Text("Vocab Acquisition [▓▓▓▓▒▒▒] Pronunciation [▓▓▓▒▒▒▒]");

                // Cultural Insight
                ImGui::Separator();
                ImGui::Text("Cultural Insight: \"Golden Week\" - A week of holidays in Japan...");

                ImGui::End();
            }
        }
    }
}