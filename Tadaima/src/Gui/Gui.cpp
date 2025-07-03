// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "Gui.h"

#include "Widgets/widget.h"
#include "Widgets/LessonTreeViewWidget.h"
#include "Widgets/MenuBarWidget.h"
#include "Widgets/MainDashboardWidget.h"
#include "resources/IconsFontAwesome4.h"
#include "resources/resource.h"
#include "tools/SystemTools.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <xutility>
#include <format> 
#include <string>
#include <windows.h>
#include "Tools/Logger.h"
#include "Widgets/WidgetTypes.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace tadaima
{
    namespace gui
    {
        static float window_width = 1280;
        static float window_height = 820;
        static uint16_t g_ResizeWidth = 0;
        static uint16_t g_ResizeHeight = 0;

        Gui::Gui(tools::Logger& logger, const config& r_config) : m_logger(logger), m_quizManager(logger), m_guiConfig(r_config)
        {

            /*
            *  Initialize widgets.
            */
            m_widgets[widget::Type::MenuBar] = std::make_unique<widget::MenuBarWidget>(m_logger);
            m_widgets[widget::Type::LessonTreeView] = std::make_unique<widget::LessonTreeViewWidget>(logger);
            m_widgets[widget::Type::Dashboard] = std::make_unique<widget::MainDashboardWidget>();

            for( const auto& [key, value] : m_widgets )
            {
                value->setObserver(std::bind(&Gui::handleWidgetEvent, this, std::placeholders::_1));
            }
        }

        void Gui::initialize()
        {
            /*
            * Initialize fonts.
            */
            std::string pathToFont = getexepath() + "\\fonts\\NotoSansJP-Regular.ttf";
            std::string pathToIcons = getexepath() + "\\fonts\\forkawesome-webfont.ttf";
            std::string p = getexepath() +  "\\fonts\\NotoSans-Regular.ttf";

            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->AddFontDefault();

            // Specify the range of characters to include
            static const ImWchar icons_ranges[] = { 0x0020, 0x007F, // Basic Latin (for general UI)
                0x3000, 0x30FF, // Japanese Punctuation, Hiragana, Katakana
                0x4E00, 0x9FAF, // Common Kanji
                0,
            };
            static const ImWchar ranges[] =
            {
                0x0020, 0x00FF, // Basic Latin + Latin Supplement
                0x00A0, 0x02D9, // Polish characters 
                0,
            };

         //   ImGuiIO& io = ImGui::GetIO();
            float fontSize = 17.0f * io.FontGlobalScale; // Adjust based on your DPI scaling
            io.Fonts->Clear();

            static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

            // Load Japanese font
            ImFontConfig font_config;
            font_config.MergeMode = true; // Set to true if adding to existing font
            font_config.PixelSnapH = true; // Align text to pixels

            font_config.OversampleH = 3; // Horizontal oversampling
            font_config.OversampleV = 3; // Vertical oversampling
            font_config.PixelSnapH = true; // Align text to pixel boundaries

            io.Fonts->TexUvScale = ImVec2(1.0f, 1.0f); // Ensure UV scaling is correct
            io.Fonts->TexUvWhitePixel = ImVec2(1.0f, 1.0f); // Ensure white pixel texture is correct
            ImGui::GetStyle().AntiAliasedFill = true;
            ImGui::GetStyle().AntiAliasedLines = true;

            io.Fonts->AddFontFromFileTTF(p.c_str(), fontSize, NULL, ranges);

            io.Fonts->AddFontFromFileTTF(pathToIcons.c_str(), fontSize, &font_config, icon_ranges);
            m_fontToUse = io.Fonts->AddFontFromFileTTF(pathToFont.c_str(), fontSize, &font_config, icons_ranges);

            io.Fonts->Build();
        }

        void Gui::addListener(widget::Type widget, EventListener listener)
        {
            dispatcher.addListener(widget, listener);
        }

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

        void Gui::initializeWidget(const tools::DataPackage& data)
        {
            try
            {
                for( const auto& [key, value] : m_widgets )
                {
                    value->initialize(data);
                }

                m_quizManager.initialize(data);
            }
            catch( std::exception& exception )
            {
                m_logger.log(std::format("Gui::initializeWidget: Can't initialize a wiget. Message: {}", exception.what()), tools::LogLevel::PROBLEM);
            }
            catch( ... )
            {
                m_logger.log("Gui::initializeWidget: Can't initialize a wiget.", tools::LogLevel::PROBLEM);
            }
        }

        int Gui::run()
        {
            bool floating = true;

            WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr),
                LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON1)), // Main Icon
                LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, L"Tadaima",
                LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON1)) // Small Icon
            };
            ::RegisterClassExW(&wc);

            HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Tadaima!", !floating ? (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME) : WS_OVERLAPPEDWINDOW, 100, 100, 900, 700, nullptr, nullptr, wc.hInstance, nullptr);
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

            // Setup Platform/Renderer backends
            ImGui_ImplWin32_Init(hwnd);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

            SetupImGuiStyle();

            // Our state
            bool show_demo_window = false;
            bool show_another_window = false;
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
            bool showDashboard = true;
            initialize();

            // Main loop
            bool done = false;
            while( !done )
            {
                bool showLessonTreeView = true;

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
                    m_widgets[widget::Type::MenuBar]->draw();
                    /*          Main menu bar           */


                    ImGui::PushFont(m_fontToUse);

                    /*          VARIABLES LIST          */

                    ImGui::SetNextWindowPos(ImVec2(0, 29), ImGuiCond_Always);
                    ImGui::SetNextWindowSize(ImVec2(250, 700), ImGuiCond_Always);  // Adjust size as needed
                    m_widgets[widget::Type::LessonTreeView]->draw(&showLessonTreeView);
                    /*          VARIABLES LIST          */

                    /*          DASHBOARD          */
                    // Set the Main Dashboard next to Lessons Overview and non-movable
                    ImGui::SetNextWindowPos(ImVec2(250, 29), ImGuiCond_Always);
                    ImGui::SetNextWindowSize(ImVec2(640, 700), ImGuiCond_Always);  // Adjust remaining width
                    m_widgets[widget::Type::Dashboard]->draw(&showDashboard);
                    /*          DASHBOARD          */

                    ImGui::PopFont();

                    m_quizManager.draw();
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

        void Gui::handleWidgetEvent(const widget::WidgetEvent& data)
        {
            try
            {
                if( data.getEventType() == tadaima::gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnPlayMultipleChoiceQuiz )
                {
                    widget::LessonDataPackage* package = dynamic_cast<widget::LessonDataPackage*>(data.getEventData());
                    if( nullptr != package )
                    {
                        m_quizManager.startQuiz(quiz::QuizType::MultipleChoiceQuiz, package->m_lessons);
                    }
                }
                else if( data.getEventType() == tadaima::gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnPlayVocabularyQuiz )
                {
                    widget::LessonDataPackage* package = dynamic_cast<widget::LessonDataPackage*>(data.getEventData());
                    if( nullptr != package )
                    {
                        m_quizManager.startQuiz(quiz::QuizType::VocabularyQuiz, package->m_lessons);
                    }
                }
                else if( data.getEventType() == tadaima::gui::widget::LessonTreeViewWidget::LessonTreeViewWidgetEvent::OnConjuactionQuiz )
                {
                    widget::LessonDataPackage* package = dynamic_cast<widget::LessonDataPackage*>(data.getEventData());
                    if( nullptr != package )
                    {
                        m_quizManager.startQuiz(quiz::QuizType::ConjuactionQuiz, package->m_lessons);
                    }
                }
                else
                {
                    dispatcher.emit(data.getWidget().getType(), &data);
                }
            }
            catch( const std::exception& e )
            {
                m_logger.log("Exception caught in handleWidgetEvent: " + std::string(e.what()), tools::LogLevel::PROBLEM);
            }
            catch( ... )
            {
                m_logger.log("Unknown exception caught in handleWidgetEvent", tools::LogLevel::PROBLEM);
            }
        }
    }
}