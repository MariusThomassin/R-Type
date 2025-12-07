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
#include "../ecs/core/EventBus.hpp"
#include "../ecs/events/InputEvents.hpp"
#include "../ecs/components/TransformComponent.hpp"

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

        /**
         * @brief Add a root-level widget
         * @param widget The widget to add
         */
        void addWidget(std::shared_ptr<Widget> widget);
        /**
         * @brief Remove a root-level widget
         * @param widget The widget to remove
         */
        void removeWidget(std::shared_ptr<Widget> widget);
        /**
         * @brief Clear all root-level widgets
         */
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
        void render() const;

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
        /**
         * @brief Event handlers for ECS input events
         * @param event The input event data
         */
        void handleMouseClick(const rtype::ecs::events::MouseButtonPressedEvent& event);
        /**
         * @brief Handle mouse release event
         * @param event The mouse button released event data
         */
        void handleMouseRelease(const rtype::ecs::events::MouseButtonReleasedEvent& event);
        /**
         * @brief Handle mouse move event
         * @param event The mouse move event data
         */
        void handleMouseMove(const rtype::ecs::events::MouseMoveEvent& event);
        /**
         * @brief Handle mouse wheel event
         * @param event The mouse wheel event data
         */
        void handleMouseWheel(const rtype::ecs::events::MouseWheelEvent& event);
        /**
         * @brief Handle key press event
         * @param event The key pressed event data
         */
        void handleKeyPress(const rtype::ecs::events::KeyPressedEvent& event);
        /**
         * @brief Handle key release event
         * @param event The key released event data
         */
        void handleKeyRelease(const rtype::ecs::events::KeyReleasedEvent& event);

        /**
         * @brief Find the topmost widget at given coordinates
         * @param x X coordinate
         * @param y Y coordinate
         * @return Shared pointer to the widget at (x, y) or nullptr if none
         */
        std::shared_ptr<Widget> findWidgetAt(float x, float y);

        /**
         * @brief Recursively find widget at coordinates within a parent widget's children
         * @param parent The parent widget to search within
         * @param x X coordinate
         * @param y Y coordinate
         * @return Shared pointer to the child widget at (x, y) or nullptr if none
         */
        std::shared_ptr<Widget> findWidgetAtRecursive(std::shared_ptr<Widget> parent, float x, float y);

        /**
         * @brief List of root-level widgets
         */
        std::vector<std::shared_ptr<Widget>> m_widgets;
        /**
         * @brief Currently hovered widget (if any)
         */
        std::shared_ptr<Widget> m_hoveredWidget;
        /**
         * @brief Currently focused widget (if any)
         */
        std::shared_ptr<Widget> m_focusedWidget;

        /**
         * @brief Reference to the ECS EventBus
         */
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
