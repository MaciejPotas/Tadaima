#pragma once

#include "MainDashboardWidget.h"
#include "imgui.h"
#include "gui.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {

            void MainDashboardWidget::initialize([[maybe_unused]] const tools::DataPackage& r_package)
            {

            }

            void MainDashboardWidget::draw(bool* p_open)
            {
                if( !ImGui::Begin("Main Dashboard", p_open, ImGuiWindowFlags_NoDecoration) )
                {
                    ImGui::End();
                    return;
                }

                // Header
                ImGui::Text((const char*)u8"Good morning, [Username]! 今日の天気: 晴れ, 25°C");
                ImGui::ProgressBar(0.65f, ImVec2(-1.0f, 0.0f), "65% complete");

                // Word of the Day
                ImGui::Separator();
                ImGui::Text((const char*)u8"Word of the Day: \"茶\" (ちゃ) - Tea");
                ImGui::Text((const char*)u8"Example: 彼は毎朝茶を飲む。 (He drinks tea every morning.)");

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