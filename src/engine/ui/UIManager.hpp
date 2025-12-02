/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** UIManager - Manages UI widgets with ECS EventBus integration
*/

#ifndef UIMANAGER_HPP_
#define UIMANAGER_HPP_

#include <memory>
#include <vector>
#include "Widget.hpp"
#include "src/engine/ecs/core/EventBus.hpp"
#include "src/engine/ecs/events/InputEvents.hpp"

namespace rtype::ui
{
    /**
     * @brief Manages UI widgets and routes ECS events to them
     * 
     * UIManager automatically subscribes to EventBus input events and
     * forwards them to the appropriate widgets. It handles:
     * - Mouse click/release/move/wheel events
     * - Keyboard press/release events
     * - Widget focus management
     * - Hover state tracking
     * 
     * Events are propagated in reverse order (topmost widget first).
     * If a widget consumes an event (returns true), propagation stops.
     */
    class UIManager {
    public:
        /**
         * @brief Construct UIManager and subscribe to EventBus
         * @param eventBus Reference to the ECS EventBus
         */
        explicit UIManager(rtype::ecs::EventBus& eventBus);
        
        /**
         * @brief Destructor unsubscribes from all events
         */
        ~UIManager();

        // Widget management
        void addWidget(std::shared_ptr<Widget> widget);
        void removeWidget(std::shared_ptr<Widget> widget);
        void clearWidgets();
        
        /**
         * @brief Get all root-level widgets
         */
        const std::vector<std::shared_ptr<Widget>>& getWidgets() const;

        /**
         * @brief Set focus to a specific widget (or nullptr to clear focus)
         */
        void setFocus(std::shared_ptr<Widget> widget);
        
        /**
         * @brief Get currently focused widget
         */
        std::shared_ptr<Widget> getFocusedWidget() const;

        /**
         * @brief Get currently hovered widget
         */
        std::shared_ptr<Widget> getHoveredWidget() const;

        /**
         * @brief Update all widgets
         * @param deltaTime Time since last frame
         */
        void update(float deltaTime);
        
        /**
         * @brief Render all visible widgets
         */
        void render();

        /**
         * @brief Check if UI is currently capturing mouse input
         * @return true if mouse is over any widget
         */
        bool isCapturingMouse() const;

        /**
         * @brief Check if UI is currently capturing keyboard input
         * @return true if a widget has focus
         */
        bool isCapturingKeyboard() const;

    private:
        // Event handlers (called by EventBus subscriptions)
        void handleMouseClick(const rtype::ecs::events::MouseButtonPressedEvent& event);
        void handleMouseRelease(const rtype::ecs::events::MouseButtonReleasedEvent& event);
        void handleMouseMove(const rtype::ecs::events::MouseMoveEvent& event);
        void handleMouseWheel(const rtype::ecs::events::MouseWheelEvent& event);
        void handleKeyPress(const rtype::ecs::events::KeyPressedEvent& event);
        void handleKeyRelease(const rtype::ecs::events::KeyReleasedEvent& event);

        // Find widget at position (topmost first)
        std::shared_ptr<Widget> findWidgetAt(float x, float y);

        std::vector<std::shared_ptr<Widget>> m_widgets;
        std::shared_ptr<Widget> m_hoveredWidget;
        std::shared_ptr<Widget> m_focusedWidget;

        rtype::ecs::EventBus& m_eventBus;

        // Event subscription IDs for cleanup
        rtype::ecs::EventBus::SubscriberId m_mouseClickSubId = 0;
        rtype::ecs::EventBus::SubscriberId m_mouseReleaseSubId = 0;
        rtype::ecs::EventBus::SubscriberId m_mouseMoveSubId = 0;
        rtype::ecs::EventBus::SubscriberId m_mouseWheelSubId = 0;
        rtype::ecs::EventBus::SubscriberId m_keyPressSubId = 0;
        rtype::ecs::EventBus::SubscriberId m_keyReleaseSubId = 0;
    };
} // namespace rtype::ui

#endif /* !UIMANAGER_HPP_ */
