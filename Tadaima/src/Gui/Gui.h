// ----------------------------------------------------------------------------
// File:        gui.h
// Description: Header file for the GUI thread.
// ----------------------------------------------------------------------------

#pragma once

#include "Widgets/Widget.h"
#include "Tools/EventDispatcher.h"
#include <d3d11.h>
#include <string>
#include <memory>
#include "Tools/Types.h"

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

            using WidgetEventDispatcher = tools::EventDispatcher<std::string, const widget::WidgetEvent*>;
            using EventListener = WidgetEventDispatcher::EventHandler;

            struct config
            {
                bool floating = false; ///< Flag indicating whether the GUI window has a fixed size.
            };

            /**
             * @brief Constructor.
             */
            Gui(const config& r_config);

            /**
             * @brief Adds an event listener for a specific entity and action.
             *
             * @param entity The entity to listen to.
             * @param action The action to listen for.
             * @param listener The event listener function.
             */
             //void addListener(const widget::Type& entity, EventListener listener);

             /**
              * @brief Adds an event listener for a specific entity and action.
              *
              * @param entity The entity to listen to.
              * @param action The action to listen for.
              * @param listener The event listener function.
              */
            void addListener(const std::string widgetId, EventListener listener);

            /**
             * @brief Adds an event listener for a specific entity and action.
             *
             * @param entity The entity to listen to.
             * @param action The action to listen for.
             * @param listener The event listener function.
             */
            void addListener(widget::Widget& widget, EventListener listener);

            template<typename T>
            void create(const std::string& id)
            {
                m_widgets.push_back(std::make_shared<T>(id));
                m_widgets.back()->setObserver(std::bind(&Gui::handleWidgetEvent, this, std::placeholders::_1));
            }

            void initializeWidget(const std::string& id, const tools::DataPackage& data);

            /**
             * @brief Runs the GUI thread.
             *
             * @return 0 if the GUI thread ran successfully.
             */
            int run();

        private:

            /**
            * Desc
            *
            * @param File name.
            * @return EN_ErrorCode::EC_NoError if everything was fine.
            */
            tools::IntPair calculateWindowSize();

            bool CreateDeviceD3D(HWND hWnd); ///< Creates the Direct3D device.
            void CleanupDeviceD3D(); ///< Cleans up the Direct3D device.
            void CreateRenderTarget(); ///< Creates the render target.
            void CleanupRenderTarget(); ///< Cleans up the render target.
            void DrawMenuWidget(); ///< Draws the menu widget.
            void DrawInfoWidget(); ///< Draws the info widget.
            void DrawFileWidgets(); ///< Draws the file widgets.
            void ShowAboutWindow(bool* p_open); ///< Shows the about window.
            void handleWidgetEvent(const widget::WidgetEvent& data); ///< Handles widget events.

            ID3D11Device* g_pd3dDevice = nullptr; ///< Direct3D device.
            ID3D11DeviceContext* g_pd3dDeviceContext = nullptr; ///< Direct3D device context.
            IDXGISwapChain* g_pSwapChain = nullptr; ///< Swap chain for rendering.
            ID3D11RenderTargetView* g_mainRenderTargetView = nullptr; ///< Render target view.
            WidgetEventDispatcher dispatcher; ///< Event dispatcher for widgets.
            uint8_t m_widgetId = 0; ///< ID for widgets.

            std::vector<std::shared_ptr<widget::Widget>> m_widgets; ///< Vector of widgets.
            config m_guiConfig; ///< Configuration for the GUI.
        };
    }
}