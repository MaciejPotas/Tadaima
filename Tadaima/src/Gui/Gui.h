/**
 * @file gui.h
 * @brief Header file for the GUI thread.
 */

#pragma once

#include "Widgets/Widget.h"
#include "Widgets/WidgetTypes.h"
#include "Tools/EventDispatcher.h"
#include <d3d11.h>
#include <memory>
#include <map>
#include "gui/widgets/quiz/QuizManagerWidget.h"

struct ImFont;
namespace tools { class Logger; }

namespace tadaima
{
    namespace gui
    {
        /**
         * @brief Represents the GUI thread.
         *
         * This class represents the GUI thread responsible for managing GUI components
         * and handling user interactions.
         */
        class Gui
        {
        public:
            using WidgetEventDispatcher = tools::EventDispatcher<widget::Type, const widget::WidgetEvent*>;
            using EventListener = WidgetEventDispatcher::EventHandler;

            struct config
            {
                bool floating = false; ///< Flag indicating whether the GUI window has a fixed size.
            };

            /**
             * @brief Constructor.
             *
             * @param r_config Configuration for the GUI.
             */
            Gui(tools::Logger& logger, const config& r_config);

            virtual ~Gui() = default;

            /**
             * @brief Initializes the GUI thread.
             */
            void initialize();

            /**
             * @brief Adds an event listener for a specific widget and action.
             *
             * @param widget The widget to listen to.
             * @param listener The event listener function.
             */
            virtual void addListener(widget::Type widget, EventListener listener);

            /**
             * @brief Initializes a widget with provided data.
             *
             * @param widget The widget to initialize.
             * @param data The data package for initialization.
             */
            virtual void initializeWidget(const tools::DataPackage& data);

            /**
             * @brief Runs the GUI thread.
             *
             * @return 0 if the GUI thread ran successfully.
             */
            int run();

        private:

            /**
             * @brief Sets up the ImGui style.
             */
            void SetupImGuiStyle();

            /**
             * @brief Creates the Direct3D device.
             *
             * @param hWnd Handle to the window.
             * @return True if creation succeeded, false otherwise.
             */
            bool CreateDeviceD3D(HWND hWnd);

            /**
             * @brief Cleans up the Direct3D device.
             */
            void CleanupDeviceD3D();

            /**
             * @brief Creates the render target.
             */
            void CreateRenderTarget();

            /**
             * @brief Cleans up the render target.
             */
            void CleanupRenderTarget();

            /**
             * @brief Handles widget events.
             *
             * @param data The widget event data.
             */
            void handleWidgetEvent(const widget::WidgetEvent& data);

            quiz::QuizManagerWidget m_quizManager;
            ID3D11Device* g_pd3dDevice = nullptr; ///< Direct3D device.
            ID3D11DeviceContext* g_pd3dDeviceContext = nullptr; ///< Direct3D device context.
            IDXGISwapChain* g_pSwapChain = nullptr; ///< Swap chain for rendering.
            ID3D11RenderTargetView* g_mainRenderTargetView = nullptr; ///< Render target view.
            WidgetEventDispatcher dispatcher; ///< Event dispatcher for widgets.
            uint8_t m_widgetId = 0; ///< ID for widgets.
            ImFont* m_fontToUse = nullptr;
            tools::Logger& m_logger; /**< Reference to the Logger instance for logging. */
            std::map<widget::Type, std::unique_ptr<widget::Widget>> m_widgets; ///< Vector of widgets.
            config m_guiConfig; ///< Configuration for the GUI.
        };
    }
}
