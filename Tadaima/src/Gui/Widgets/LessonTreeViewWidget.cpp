#pragma once

#include "LessonTreeViewWidget.h"
#include "gui.h"
#include "resources/IconsFontAwesome4.h"
#include "imgui.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            void LessonTreeViewWidget::initialize([[maybe_unused]] const tools::DataPackage& r_package)
            {
            }

            void LessonTreeViewWidget::draw(bool* p_open)
            {
                if( !ImGui::Begin("Lessons Overview", p_open, ImGuiWindowFlags_NoDecoration) )
                {
                    ImGui::End();
                    return;
                }

                ImGui::Separator();
                if( ImGui::Button(ICON_FA_FREE_CODE_CAMP "") )
                {
                    // Logic to add new lesson
                    static bool open_add_new_lesson = true;
                    if( open_add_new_lesson )
                    {
                        ImGui::Begin("Add New Lesson", &open_add_new_lesson);
                        ImGui::End();
                    }
                }
                ImGui::SameLine();
                if( ImGui::Button(ICON_FA_SEARCH "") )
                {
                    // Logic to edit a lesson
                }
                ImGui::SameLine();
                if( ImGui::Button(ICON_FA_SEARCH "") )
                {
                    // Logic to delete a lesson
                }

                // Lesson tree
                if( ImGui::TreeNode("Beginner") )
                {
                    if( ImGui::TreeNode("Introduction to Hiragana") )
                    {
                        ImGui::Text("Content of Introduction to Hiragana");
                        ImGui::TreePop();
                    }
                    if( ImGui::TreeNode("Basic Greetings") )
                    {
                        ImGui::Text("Content of Basic Greetings");
                        ImGui::TreePop();
                    }
                    if( ImGui::TreeNode("Numbers") )
                    {
                        ImGui::Text("Content of Numbers");
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                if( ImGui::TreeNode("Intermediate") )
                {
                    if( ImGui::TreeNode("Kanji Essentials") )
                    {
                        ImGui::Text("Content of Kanji Essentials");
                        ImGui::TreePop();
                    }
                    if( ImGui::TreeNode("Conversational Phrases") )
                    {
                        ImGui::Text("Content of Conversational Phrases");
                        ImGui::TreePop();
                    }
                    if( ImGui::TreeNode("Transportation Vocabulary") )
                    {
                        ImGui::Text("Content of Transportation Vocabulary");
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                if( ImGui::TreeNode("Advanced") )
                {
                    if( ImGui::TreeNode("Business Japanese") )
                    {
                        ImGui::Text("Content of Business Japanese");
                        ImGui::TreePop();
                    }
                    if( ImGui::TreeNode("Keigo (Honorific Language)") )
                    {
                        ImGui::Text("Content of Keigo");
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }

                ImGui::End();
            }

        }
    }
}
