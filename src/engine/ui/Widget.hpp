/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Widget - Base class for UI elements with ECS event integration
*/

#ifndef WIDGET_HPP_
#define WIDGET_HPP_

#include <cstdint>
#include <vector>
#include <memory>
#include <algorithm>
#include "../graphics/IRenderable.hpp"
#include "UIColor.hpp"
#include "../ecs/events/InputEvents.hpp"

#define DEFAULT_BACKGROUND_COLOR rtype::ui::UIColor::White()
#define DEFAULT_BORDER_COLOR rtype::ui::UIColor::Black()
#define DEFAULT_TEXT_COLOR rtype::ui::UIColor::White()

#define DEFAULT_FONT_SIZE 16
#define DEFAULT_PADDING 5.0f
#define DEFAULT_BORDER_WIDTH 0.0f

namespace rtype::ui {
    /**
     * @brief UI Transform representing position and size
     * @note x, y represent the top-left corner
     * @note width and height represent dimensions
     */
    struct UITransform {
        float x = 0.0f;
        float y = 0.0f;
        float width = 100.0f;
        float height = 100.0f;
    };

    /**
     * @brief UI Style properties for widgets
     * 
     * UIStyle encapsulates visual styling options such as colors,
     * font size, border width, and padding.
     */
    struct UIStyle {
        UIColor backgroundColor = DEFAULT_BACKGROUND_COLOR;
        UIColor borderColor = DEFAULT_BORDER_COLOR;
        UIColor textColor = DEFAULT_TEXT_COLOR;
        size_t fontSize = DEFAULT_FONT_SIZE;
        float borderWidth = DEFAULT_BORDER_WIDTH;
        float padding = DEFAULT_PADDING;
    };

    /**
     * @brief Base class for UI widgets with ECS event integration
     * 
     * Widget provides the foundation for building UI elements with:
     * - Transform (position, size)
     * - Style (colors, borders, padding)
     * - Hierarchy (parent/child relationships)
     * - Event handling (integrated with ECS EventBus via UIManager)
     * - Visibility/enabled state
     * 
     * Event methods return true if event was consumed (stops propagation).
     * UIManager automatically routes ECS input events to widgets.
     */
    class Widget : public std::enable_shared_from_this<Widget> {
        public:
            /**
             * @brief Construct a new Widget object
             * 
             * @note Each widget is assigned a unique ID upon creation
             * Default transform and style are initialized
             * Default state: visible = true, enabled = true, not focused = false
             */
            Widget();
            virtual ~Widget() = default;

            /**
             * @brief Set the position of the widget
             * @param x X coordinate
             * @param y Y coordinate
             */
            void setPosition(float x, float y);
            /**
             * @brief Set the size of the widget
             * @param width Width of the widget
             * @param height Height of the widget
             */
            void setSize(float width, float height);

            /**
             * @brief Get the local transform of the widget
             * @return UITransform representing local position and size
             */
            UITransform getTransform() const;
            /**
             * @brief Get the absolute transform of the widget
             * @return UITransform representing absolute position and size
             */
            UITransform getAbsoluteTransform() const;

            /**
             * @brief Set style properties
             * @param color The color to set for the background
             */
            virtual void setBackgroundColor(UIColor color);
            /**
             * @brief Set the border color
             * @param color The color to set for the border
             */
            virtual void setBorderColor(UIColor color);
            /**
             * @brief Set the text color
             * @param color The color to set for the text
             */
            virtual void setTextColor(UIColor color);
            /**
             * @brief Set the font size
             * @param size The font size in points
             */
            virtual void setFontSize(size_t size);
            /**
             * @brief Set the border width
             * @param width The border width in pixels
             */
            virtual void setBorderWidth(float width);
            /**
             * @brief Set the padding
             * @param padding The padding in pixels
             */
            virtual void setPadding(float padding);
            /**
             * @brief Set the entire style at once
             * @param style The UIStyle to apply
             */
            void setStyle(const UIStyle& style);
            /**
             * @brief Get the current style
             * @return The UIStyle of the widget
             */
            const UIStyle& getStyle() const;

            /**
             * @brief Add a child widget
             * @param child Shared pointer to the child widget
             */
            void addChild(std::shared_ptr<Widget> child);
            /**
             * @brief Remove a child widget
             * @param child Shared pointer to the child widget to remove
             */
            void removeChild(std::shared_ptr<Widget> child);
            /**
             * @brief Get the list of child widgets
             * @return Vector of shared pointers to child widgets
             */
            const std::vector<std::shared_ptr<Widget>>& getChildren() const;
            /**
             * @brief Get the parent widget
             * @return Shared pointer to the parent widget
             */
            std::shared_ptr<Widget> getParent() const;

            /**
             * @brief Mouse enter event (from ECS MouseMoveEvent)
             * @return true if event was consumed
             */
            virtual bool onMouseEnter();
            /**
             * @brief Mouse leave event (from ECS MouseMoveEvent)
             * @return true if event was consumed
             */
            virtual bool onMouseLeave();
            /**
             * @brief Mouse click event (from ECS MouseButtonPressedEvent)
             * @return true if event was consumed
             */
            virtual bool onMouseClick();
            /**
             * @brief Mouse move event (from ECS MouseMoveEvent)
             * @param x The x position of the mouse
             * @param y The y position of the mouse
             * @return true if event was consumed
             */
            virtual bool onMouseMove(float x, float y);
            /**
             * @brief Mouse wheel event (from ECS MouseWheelEvent)
             * @param delta The amount the wheel has moved
             * @return true if event was consumed
             */
            virtual bool onMouseWheel(float delta);

            /**
             * @brief Mouse button release event (from ECS MouseButtonReleasedEvent)
             * @return True if event was handled (consumed)
             */
            virtual bool onMouseRelease();

            /**
             * @brief Key press event (from ECS KeyPressedEvent)
             * @param key The key code of the pressed key
             * @return true if event was consumed
             */
            virtual bool onKeyPress(rtype::ecs::events::KeyCode key);
            /**
             * @brief Key release event (from ECS KeyReleasedEvent)
             * @param key The key code of the released key
             * @return true if event was consumed
             */
            virtual bool onKeyRelease(rtype::ecs::events::KeyCode key);

            /**
             * @brief Focus gained event
             */
            virtual void onFocus();
            /**
             * @brief Focus lost event
             */
            virtual void onBlur();

            /**
             * @brief Set the visibility of the widget
             * @param visible True to make visible, false to hide
             */
            void setVisible(bool visible);
            /**
             * @brief Set the enabled state of the widget
             * @param enabled True to enable, false to disable
             */
            void setEnabled(bool enabled);
            /**
             * @brief Check if the widget is visible
             * @return True if visible, false otherwise
             */
            bool isVisible() const;
            /**
             * @brief Check if the widget is enabled
             * @return True if enabled, false otherwise
             */
            bool isEnabled() const;
            /**
             * @brief Check if the widget is focused
             * @return True if focused, false otherwise
             */
            bool isFocused() const;

            /**
             * @brief Check if a point is within the widget's bounds
             * @param x X coordinate of the point
             * @param y Y coordinate of the point
             * @return True if the point is inside the widget, false otherwise
             */
            bool contains(float x, float y) const;

            /**
             * @brief Get the unique ID of the widget
             * @return The widget's unique ID
             */
            size_t getID() const;

            /**
             * @brief Render the widget and its children
             */
            /**
             * @brief Render the widget and its children
             * Can be overridden for custom rendering behavior (e.g., clipping)
             */
            virtual void render() const;

            /**
             * @brief Update the widget (override for custom behavior)
             * @param deltaTime Time since last frame
             */
            virtual void update(float deltaTime);

        protected:
            /**
             * @brief Render the widget itself (override in subclasses)
             */
            virtual void renderSelf() const = 0;

            /**
             * @brief Widget's local transform
             */
            UITransform _m_transform;
            /**
             * @brief Widget's style properties
             */
            UIStyle _m_style;
            /**
             * @brief Child widgets
             */
            std::vector<std::shared_ptr<Widget>> _m_children;
            /**
             * @brief Parent widget (weak pointer to avoid cycles)
             */
            std::weak_ptr<Widget> _m_parent;
            /**
             * @brief Widget visibility state
             */
            bool _m_visible = true;
            /**
             * @brief Widget enabled state
             */
            bool _m_enabled = true;
            /**
             * @brief Widget focused state
             */
            bool _m_focused = false;
            /**
             * @brief Widget unique identifier
             */
            size_t _m_id;

            /**
             * @brief UIManager is a  friend class to allow event routing
             */
            friend class UIManager;

        private:
            /**
             * @brief Static counter to assign unique IDs
             */
            static size_t s_nextID;
    };
} // namespace rtype::ui

#endif /* !WIDGET_HPP_ */
