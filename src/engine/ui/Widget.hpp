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

#define DEFAULT_BACKGROUND_COLOR rtype::ui::UIColor(200, 200, 200, 255)
#define DEFAULT_BORDER_COLOR rtype::ui::UIColor(0, 0, 0, 255)
#define DEFAULT_TEXT_COLOR rtype::ui::UIColor(0, 0, 0, 255)

#define DEFAULT_FONT_SIZE 16
#define DEFAULT_PADDING 5.0f
#define DEFAULT_BORDER_WIDTH 0.0f

namespace rtype::ui {
    struct UITransform {
        float x = 0.0f;
        float y = 0.0f;
        float width = 100.0f;
        float height = 100.0f;
    };

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
            Widget();
            virtual ~Widget() = default;

            // Transform
            void setPosition(float x, float y);
            void setSize(float width, float height);
            UITransform getTransform() const;
            UITransform getAbsoluteTransform() const;

            //Style getters and setters
            virtual void setBackgroundColor(UIColor color);
            virtual void setBorderColor(UIColor color);
            virtual void setTextColor(UIColor color);
            virtual void setFontSize(size_t size);
            virtual void setBorderWidth(float width);
            virtual void setPadding(float padding);
            void setStyle(const UIStyle& style);
            const UIStyle& getStyle() const;

            // Hierarchy
            void addChild(std::shared_ptr<Widget> child);
            void removeChild(std::shared_ptr<Widget> child);
            const std::vector<std::shared_ptr<Widget>>& getChildren() const;
            std::shared_ptr<Widget> getParent() const;

            // Mouse events (from ECS MouseButtonPressedEvent, MouseMoveEvent, etc.)
            virtual bool onMouseEnter();
            virtual bool onMouseLeave();
            virtual bool onMouseClick();
            virtual bool onMouseMove(float x, float y);
            virtual bool onMouseWheel(float delta);

            // Keyboard events (from ECS KeyPressedEvent, KeyReleasedEvent)
            virtual bool onKeyPress(rtype::ecs::events::KeyCode key);
            virtual bool onKeyRelease(rtype::ecs::events::KeyCode key);

            // Focus events (managed by UIManager)
            virtual void onFocus();
            virtual void onBlur();

            // State
            void setVisible(bool visible);
            void setEnabled(bool enabled);
            bool isVisible() const;
            bool isEnabled() const;
            bool isFocused() const;

            // Hit testing
            bool contains(float x, float y) const;

            // Identification
            size_t getID() const;

            void render() const;

            // Update (called by UIManager each frame)
            virtual void update(float deltaTime);

        protected:
            virtual void renderSelf() const = 0;

            UITransform _m_transform;
            UIStyle _m_style;
            std::vector<std::shared_ptr<Widget>> _m_children;
            std::weak_ptr<Widget> _m_parent;
            bool _m_visible = true;
            bool _m_enabled = true;
            bool _m_focused = false;
            size_t _m_id;

            // Allow UIManager to set focus state
            friend class UIManager;

        private:
            static size_t s_nextID;
    };
} // namespace rtype::ui

#endif /* !WIDGET_HPP_ */
