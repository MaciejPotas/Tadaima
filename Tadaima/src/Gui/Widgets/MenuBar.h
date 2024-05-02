#pragma once

#include "imgui.h"
#include "resources/IconsFontAwesome4.h"

namespace tadaima
{
    namespace gui
    {
        void ShowAboutWindow(bool* p_open)
        {
            if( !ImGui::Begin("About", p_open, ImGuiWindowFlags_AlwaysAutoResize) )
            {
                ImGui::End();
                return;
            }

            ImGui::SeparatorText("About Tadaima:");
            ImGui::Text("I am here to help you with vocabulary :-)");
            ImGui::Text("Tadaima revision: (%s)", tadaima::versioning::Revision.data());
            ImGui::Separator();
            ImGui::SeparatorText("Developers:");
            ImGui::Text("Maciek Potas: maciejpotas@gmail.com");

            ImGui::End();
        }

        void ShowMainMenu()
        {
            // Display help message when Help is clicked
            static bool show_help = false;
            if( show_help )
            {
                ShowAboutWindow(&show_help);
            }

            //if( !ImGui::Begin("Menu bar", NULL, ImGuiWindowFlags_NoDecoration) )
            //{
            //    ImGui::End();
            //    return;
            //}

            //// Navigation Bar with Icons
            //ImGui::Separator();
            //if( ImGui::Button(ICON_FA_HOME " Home") )
            //{
            //    // Home button logic
            //}
            //ImGui::SameLine();
            //if( ImGui::Button(ICON_FA_SEARCH " Explore") )
            //{
            //    // Explore button logic
            //}
            //ImGui::SameLine();
            //if( ImGui::Button(ICON_FA_TROPHY " Challenges") )
            //{
            //    // Challenges button logic
            //}
            //ImGui::SameLine();
            //if( ImGui::Button(ICON_FA_TROPHY " Progress") )
            //{
            //    // Progress button logic
            //}
            //ImGui::SameLine();
            //if( ImGui::Button(ICON_FA_MICROPHONE " About") )
            //{
            //    show_help = true;
            //}
            //ImGui::Separator();

            // Menu Bar
            if( ImGui::BeginMainMenuBar() )
            {
                if( ImGui::BeginMenu("File") )
                {
                    ImGui::MenuItem("Open", NULL, false, true);
                    ImGui::MenuItem("Save", NULL, false, true);
                    ImGui::EndMenu();
                }
                if( ImGui::BeginMenu("Dictionary") )
                {
                    if( ImGui::MenuItem("Show", NULL, false, true) )
                    {
                        // Trigger show dictionary
                    }
                    if( ImGui::MenuItem("Edit", NULL, false, true) )
                    {
                        // Trigger edit dictionary
                    }
                    ImGui::EndMenu();
                }
                if( ImGui::BeginMenu("Help") )
                {
                    if( ImGui::MenuItem("Open Help") )
                    {
                        show_help = true;
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            //ImGui::End();
        }

    }
}