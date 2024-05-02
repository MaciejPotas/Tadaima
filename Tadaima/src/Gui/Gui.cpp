// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "Gui.h"
#include "Version.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <xutility>
#include "Widgets/widget.h"
#include "MainDashboard.h"
#include "LessonTreeView.h"
#include "VocabularySettings.h"
#include <string>
#include <windows.h>
#include "imgui_internal.h"
#include "Widgets/MenuBar.h"
#include "resources/IconsFontAwesome4.h"

#pragma execution_character_set("utf-8")

std::string getexepath()
{
    char buffer[MAX_PATH];
    ::GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");

    return std::string(buffer).substr(0, pos);

}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace tadaima
{
    namespace gui
    {
        static int window_width = 1280;
        static int window_height = 720;
        static uint16_t g_ResizeWidth = 0;
        static uint16_t g_ResizeHeight = 0;

        Gui::Gui(const config& r_config) : m_guiConfig(r_config)
        {

        }

        void Gui::addListener(const std::string widgetId, EventListener listener)
        {
            dispatcher.addListener(widgetId, listener);
        }

        void Gui::addListener(widget::Widget& widget, EventListener listener)
        {
            addListener(widget.getId(), listener);
        }

        void configurefont()
        {
            ImGuiIO& io = ImGui::GetIO();
            ImFontConfig icons_config;
            icons_config.MergeMode = true;
            icons_config.PixelSnapH = true;
            static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
            io.Fonts->AddFontFromFileTTF("path_to_fontawesome/fontawesome-webfont.ttf", 16.0f, &icons_config, icon_ranges);

            // Load your regular font
            io.Fonts->AddFontFromFileTTF("path_to_your_regular_font/regular-font.ttf", 16.0f);
        }

        void SetupImGuiStyle()
        {
            // Classic Steam style by metasprite from ImThemes
            ImGuiStyle& style = ImGui::GetStyle();

            style.Alpha = 1.0f;
            style.DisabledAlpha = 0.6000000238418579f;
            style.WindowPadding = ImVec2(8.0f, 8.0f);
            style.WindowRounding = 0.0f;
            style.WindowBorderSize = 1.0f;
            style.WindowMinSize = ImVec2(32.0f, 32.0f);
            style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
            style.WindowMenuButtonPosition = ImGuiDir_Left;
            style.ChildRounding = 0.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupRounding = 0.0f;
            style.PopupBorderSize = 1.0f;
            style.FramePadding = ImVec2(4.0f, 3.0f);
            style.FrameRounding = 0.0f;
            style.FrameBorderSize = 1.0f;
            style.ItemSpacing = ImVec2(8.0f, 4.0f);
            style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
            style.CellPadding = ImVec2(4.0f, 2.0f);
            style.IndentSpacing = 21.0f;
            style.ColumnsMinSpacing = 6.0f;
            style.ScrollbarSize = 14.0f;
            style.ScrollbarRounding = 0.0f;
            style.GrabMinSize = 10.0f;
            style.GrabRounding = 0.0f;
            style.TabRounding = 0.0f;
            style.TabBorderSize = 0.0f;
            style.TabMinWidthForCloseButton = 0.0f;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

            style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2862745225429535f, 0.3372549116611481f, 0.2588235437870026f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.2862745225429535f, 0.3372549116611481f, 0.2588235437870026f, 1.0f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.239215686917305f, 0.2666666805744171f, 0.2000000029802322f, 1.0f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 0.5f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.1372549086809158f, 0.1568627506494522f, 0.1098039224743843f, 0.5199999809265137f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.239215686917305f, 0.2666666805744171f, 0.2000000029802322f, 1.0f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2666666805744171f, 0.2980392277240753f, 0.2274509817361832f, 1.0f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2980392277240753f, 0.3372549116611481f, 0.2588235437870026f, 1.0f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.239215686917305f, 0.2666666805744171f, 0.2000000029802322f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2862745225429535f, 0.3372549116611481f, 0.2588235437870026f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.5099999904632568f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.239215686917305f, 0.2666666805744171f, 0.2000000029802322f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2784313857555389f, 0.3176470696926117f, 0.239215686917305f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2470588237047195f, 0.2980392277240753f, 0.2196078449487686f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.2274509817361832f, 0.2666666805744171f, 0.2078431397676468f, 1.0f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 1.0f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 0.5f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.2862745225429535f, 0.3372549116611481f, 0.2588235437870026f, 0.4000000059604645f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 0.5f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 1.0f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 0.6000000238418579f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 0.5f);
            style.Colors[ImGuiCol_Separator] = ImVec4(0.1372549086809158f, 0.1568627506494522f, 0.1098039224743843f, 1.0f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 1.0f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1882352977991104f, 0.2274509817361832f, 0.1764705926179886f, 0.0f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 1.0f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
            style.Colors[ImGuiCol_Tab] = ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 1.0f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 0.7799999713897705f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.239215686917305f, 0.2666666805744171f, 0.2000000029802322f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 1.0f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.7764706015586853f, 0.2784313857555389f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
            style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
            style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
            style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.729411780834198f, 0.6666666865348816f, 0.239215686917305f, 1.0f);
            style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
            style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
            style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
        }


        // Win32 message handler
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            if( ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) )
                return true;

            switch( msg )
            {
                case WM_SIZE:
                    if( wParam == SIZE_MINIMIZED )
                        return 0;
                    g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
                    g_ResizeHeight = (UINT)HIWORD(lParam);
                    return 0;
                case WM_SYSCOMMAND:
                    if( (wParam & 0xfff0) == SC_KEYMENU ) // Disable ALT application menu
                        return 0;
                    break;
                case WM_DESTROY:
                    ::PostQuitMessage(0);
                    return 0;
            }
            return ::DefWindowProcW(hWnd, msg, wParam, lParam);
        }

        void Gui::initializeWidget(const std::string& id, const tools::DataPackage& data)
        {
            auto it = std::find_if(m_widgets.begin(), m_widgets.end(), [&](const std::shared_ptr<widget::Widget>& widgetPtr)
                {
                    return widgetPtr->getId() == id;
                });

            if( it != m_widgets.end() )
            {
                (*it)->initialize(data);
            }
            else
            {
                throw std::runtime_error("Widget with ID '" + id + "' not found");
            }
        }

        void SetupImGui()
        {
            ImGuiIO& io = ImGui::GetIO(); (void)io;


        }

        int Gui::run()
        {


            bool floating = true;

            WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
            ::RegisterClassExW(&wc);
            HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Tadaima!", !floating ? (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME) : WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, nullptr, nullptr, wc.hInstance, nullptr);
            // WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME - ABY ZROBIc ZEBY SIE NIE ZMIENIALA

            // Initialize Direct3D
            if( !CreateDeviceD3D(hwnd) )
            {
                CleanupDeviceD3D();
                ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
                return 1;
            }

            // Show the window
            ::ShowWindow(hwnd, SW_SHOWDEFAULT);
            ::UpdateWindow(hwnd);

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

            //// Setup Dear ImGui style
            //ImGui::StyleColorsDark();
            //ImGui::StyleColorsLight();

            // Setup Platform/Renderer backends
            ImGui_ImplWin32_Init(hwnd);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

            // Load Fonts
            // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
            // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
            // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
            // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
            // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
            // - Read 'docs/FONTS.md' for more instructions and details.
            // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
            //io.Fonts->AddFontDefault();
            //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
            //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
            //IM_ASSERT(font != nullptr);

            SetupImGuiStyle();

            // Our state
            bool show_demo_window = false;
            bool show_another_window = false;
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

            // Load default font (optional)
            io.Fonts->AddFontDefault();

            // Specify the range of characters to include
            static const ImWchar icons_ranges[] = {
                0x0020, 0x007F, // Basic Latin (for general UI)
                0x3000, 0x30FF, // Japanese Punctuation, Hiragana, Katakana
                0x4E00, 0x9FAF, // Common Kanji
                0,
            };

            // Load Japanese font
            ImFontConfig font_config;
            font_config.MergeMode = false; // Set to true if adding to existing font
            font_config.PixelSnapH = true; // Align text to pixels


            std::string pathToFont = getexepath() + "\\fonts\\NotoSansJP-Regular.ttf";
            auto c = io.Fonts->AddFontFromFileTTF(pathToFont.c_str(), 18.0f, &font_config, icons_ranges);



            ImFontConfig icons_config;
            icons_config.MergeMode = true;
            icons_config.PixelSnapH = true;
            static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
            std::string pathToIcons = getexepath() + "\\fonts\\forkawesome-webfont.ttf";
            auto menuBarFont = io.Fonts->AddFontFromFileTTF(pathToIcons.c_str(), 16.0f, &icons_config, icon_ranges);


            // Build the font atlas
            io.Fonts->Build();







            // Main loop
            bool done = false;
            while( !done )
            {
                // Poll and handle messages (inputs, window resize, etc.)
                // See the WndProc() function below for our to dispatch events to the Win32 backend.
                MSG msg;
                while( ::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE) )
                {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                    if( msg.message == WM_QUIT )
                        done = true;
                }
                if( done )
                    break;

                // Handle window resize (we don't resize directly in the WM_SIZE handler)
                if( g_ResizeWidth != 0 && g_ResizeHeight != 0 )
                {
                    CleanupRenderTarget();
                    g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
                    g_ResizeWidth = g_ResizeHeight = 0;
                    CreateRenderTarget();
                }

                // Start the Dear ImGui frame
                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();

                ImGui::NewFrame();

                // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
                if( show_demo_window )
                {
                    ImGui::ShowDemoWindow(&show_demo_window);
                }


  

                // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
                {


                    /*          Main menu bar           */
                    ShowMainMenu();
                    /*          Main menu bar           */



                    ImGui::PushFont(c);


                    // Use these variables to scale your layout
                    ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_Always);

                    // In your rendering loop, check for window resize
                    // Assuming GLFW or similar window handling
                    //glfwGetFramebufferSize(window, &window_width, &window_height);

                    // H= 23
                    ImGui::SetNextWindowPos(ImVec2(0, 17), ImGuiCond_Always);
                    ImGui::SetNextWindowSize(ImVec2(250, 600), ImGuiCond_Always);  // Adjust size as needed
                    //ImGui::Begin("Lessons Overview", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
                    
                    
                    bool show_lesson_tree_view = true;
                    ShowLessonTreeView(&show_lesson_tree_view);
                    //ImGui::End();

                    // Set the Main Dashboard next to Lessons Overview and non-movable
                    ImGui::SetNextWindowPos(ImVec2(250, 17), ImGuiCond_Always);
                    ImGui::SetNextWindowSize(ImVec2(540, 600), ImGuiCond_Always);  // Adjust remaining width
                    //ImGui::Begin("Main Dashboard", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
                    bool show_main_dashboard = true;

                    ShowMainDashboard(&show_main_dashboard);


                    ImGui::PopFont();
                  //  ImGui::End();



                    // Optionally add Vocabulary Settings as a popup or another fixed window
                    static bool show_vocab_settings = false;
                    if( ImGui::Button("Open Vocabulary Settings") )
                    {
                        show_vocab_settings = true;
                    }
                    if( show_vocab_settings )
                    {
                        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver); // Use FirstUseEver to set initial size
                        ImGui::Begin("Vocabulary Settings", &show_vocab_settings, ImGuiWindowFlags_NoMove);
                        ShowVocabularySettings(&show_vocab_settings);
                        ImGui::End();
                    }

                }

                // 3. Show another simple window.
                if( show_another_window )
                {
                    ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                    ImGui::Text("Hello from another window!");
                    if( ImGui::Button("Close Me") )
                        show_another_window = false;
                    ImGui::End();
                }

                // Rendering
                ImGui::Render();
                const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
                g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
                g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

                g_pSwapChain->Present(1, 0); // Present with vsync
                //g_pSwapChain->Present(0, 0); // Present without vsync
            }

            // Cleanup
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();

            CleanupDeviceD3D();
            ::DestroyWindow(hwnd);
            ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

            return 0;
        }

        bool Gui::CreateDeviceD3D(HWND hWnd)
        {
            // Setup swap chain
            DXGI_SWAP_CHAIN_DESC sd;
            ZeroMemory(&sd, sizeof(sd));
            sd.BufferCount = 2;
            sd.BufferDesc.Width = 0;
            sd.BufferDesc.Height = 0;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.RefreshRate.Numerator = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.OutputWindow = hWnd;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.Windowed = TRUE;
            sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

            UINT createDeviceFlags = 0;
            //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
            D3D_FEATURE_LEVEL featureLevel;
            const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
            HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
            if( res == DXGI_ERROR_UNSUPPORTED ) // Try high-performance WARP software driver if hardware is not available.
                res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
            if( res != S_OK )
                return false;

            CreateRenderTarget();
            return true;
        }

        void Gui::CleanupDeviceD3D()
        {
            CleanupRenderTarget();
            if( g_pSwapChain ) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
            if( g_pd3dDeviceContext ) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
            if( g_pd3dDevice ) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
        }

        void Gui::CreateRenderTarget()
        {
            ID3D11Texture2D* pBackBuffer;
            g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
            pBackBuffer->Release();
        }

        void Gui::CleanupRenderTarget()
        {
            if( g_mainRenderTargetView ) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
        }




        void Gui::ShowAboutWindow(bool* p_open)
        {
            if( !ImGui::Begin("About", p_open, ImGuiWindowFlags_AlwaysAutoResize) )
            {
                ImGui::End();
                return;
            }

            ImGui::SeparatorText("ABOUT THIS APPLICATION:");
            ImGui::Text("Application used to surf over the external file systems.");
            ImGui::Text("Application revision: (%s)", tadaima::versioning::Revision.data());
            ImGui::Separator();
            ImGui::SeparatorText("Developers:");
            ImGui::Text("Maciek Potas: maciej.potas@globallogic.com");
            ImGui::Text("Lukasz Zambrowski: lukasz.zambrowski@globallogic.com");

            ImGui::End();
        }


        void Gui::handleWidgetEvent([[maybe_unused]] const widget::WidgetEvent& data)
        {
            dispatcher.emit(data.getWidget().getId(), &data);
        }
    }
}