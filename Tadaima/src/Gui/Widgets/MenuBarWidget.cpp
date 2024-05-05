#pragma once

#include "imgui.h"
#include "gui.h"
#include "Version.h"
#include "MenuBarWidget.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            void MenuBarWidget::initialize([[maybe_unused]] const tools::DataPackage& r_package)
            {

            }

            void MenuBarWidget::showAboutWindow(bool* p_open)
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

            void MenuBarWidget::draw([[maybe_unused]] bool* p_open)
            {
                // Display help message when Help is clicked
                static bool show_help = false;
                if( show_help )
                {
                    showAboutWindow(&show_help);
                }

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
            }

        }
    }
}