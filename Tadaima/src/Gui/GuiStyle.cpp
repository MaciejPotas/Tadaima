#include "imgui.h"
#include "gui.h"

namespace tadaima
{
    namespace gui
    {
        void Gui::SetupImGuiStyle()
        {
            ImGuiStyle& style = ImGui::GetStyle();

            // Enable window borders
            style.WindowBorderSize = 1.0f;
            style.FrameBorderSize = 1.0f;
            style.PopupBorderSize = 1.0f;
            style.Alpha = 1.0f;
            style.DisabledAlpha = 0.6f;
            style.WindowPadding = ImVec2(4.0f, 6.0f);
            style.WindowRounding = 5.0f;
            style.ChildRounding = 5.0f;
            style.PopupRounding = 5.0f;
            style.FramePadding = ImVec2(8.0f, 6.0f);
            style.FrameRounding = 5.0f;
            style.ItemSpacing = ImVec2(8.0f, 6.0f);
            style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
            style.IndentSpacing = 20.0f;
            style.ScrollbarSize = 20.0f;
            style.ScrollbarRounding = 5.0f;
            style.GrabMinSize = 5.0f;
            style.GrabRounding = 5.0f;
            style.TabRounding = 5.0f;
            style.TabBorderSize = 1.0f;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

            // Custom colors
            style.Colors[ImGuiCol_Text] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.0f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.98f, 0.98f, 0.98f, 1.0f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.53f, 0.81f, 0.92f, 0.6f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.53f, 0.81f, 0.92f, 1.0f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.59f, 0.95f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.67f, 0.93f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.98f, 0.98f, 0.98f, 0.75f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.98f, 0.92f, 0.84f, 1.0f); // Light peach background for menu bar
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.87f, 0.93f, 0.97f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.77f, 0.87f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.53f, 0.81f, 0.92f, 0.6f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.53f, 0.81f, 0.92f, 1.0f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.77f, 0.06f, 1.0f); // Yellow
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.69f, 0.77f, 0.87f, 1.0f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.95f, 0.77f, 0.06f, 1.0f); // Yellow
            style.Colors[ImGuiCol_Button] = ImVec4(0.87f, 0.93f, 0.97f, 1.0f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.53f, 0.81f, 0.92f, 0.6f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.95f, 0.77f, 0.06f, 1.0f); // Yellow
            style.Colors[ImGuiCol_Header] = ImVec4(0.87f, 0.93f, 0.97f, 1.0f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.53f, 0.81f, 0.92f, 0.6f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.53f, 0.81f, 0.92f, 1.0f);
            style.Colors[ImGuiCol_Separator] = ImVec4(0.7f, 0.7f, 0.7f, 0.5f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.53f, 0.81f, 0.92f, 0.6f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.95f, 0.77f, 0.06f, 1.0f); // Yellow
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.53f, 0.81f, 0.92f, 0.6f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.53f, 0.81f, 0.92f, 0.8f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.95f, 0.77f, 0.06f, 1.0f); // Yellow
            style.Colors[ImGuiCol_Tab] = ImVec4(0.69f, 0.77f, 0.87f, 1.0f); // Light blue
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.95f, 0.77f, 0.06f, 0.8f); // Yellow
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.16f, 0.67f, 0.93f, 1.0f); // Bright blue
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.87f, 0.93f, 0.97f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.69f, 0.77f, 0.87f, 1.0f); // Light blue
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.69f, 0.77f, 0.87f, 1.0f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.95f, 0.77f, 0.06f, 1.0f); // Yellow
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.53f, 0.81f, 0.92f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.95f, 0.77f, 0.06f, 1.0f); // Yellow
            style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.87f, 0.93f, 0.97f, 1.0f);
            style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.69f, 0.77f, 0.87f, 1.0f);
            style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.87f, 0.93f, 0.97f, 1.0f);
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.87f, 0.93f, 0.97f, 1.0f);
            style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.87f, 0.93f, 0.97f, 0.5f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.95f, 0.77f, 0.06f, 0.35f); // Yellow
            style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.95f, 0.77f, 0.06f, 0.9f); // Yellow
            style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.95f, 0.77f, 0.06f, 1.0f); // Yellow
            style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
            style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
        }
    }
}
