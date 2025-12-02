/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Widget
*/

#ifndef WIDGET_HPP_
#define WIDGET_HPP_

#include <cstdint>
#include <vector>
#include <memory>
#include <algorithm>
#include "src/engine/graphics/IRenderable.hpp"
#include "src/engine/ui/Color.hpp"
#include "src/engine/ecs/events/InputEvents.hpp"

#define DEFAULT_BACKGROUND_COLOR rtype::ui::Color(200, 200, 200, 255)
#define DEFAULT_BORDER_COLOR rtype::ui::Color(0, 0, 0, 255)
#define DEFAULT_TEXT_COLOR rtype::ui::Color(0, 0, 0, 255)

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
        Color backgroundColor = DEFAULT_BACKGROUND_COLOR;
        Color borderColor = DEFAULT_BORDER_COLOR;
        Color textColor = DEFAULT_TEXT_COLOR;
        size_t fontSize = DEFAULT_FONT_SIZE;
        float borderWidth = DEFAULT_BORDER_WIDTH;
        float padding = DEFAULT_PADDING;
    };

    class Widget : public rtype::ecs::IRenderable, public std::enable_shared_from_this<Widget> {
        public:
            Widget() : _m_visible(true), _m_enabled(true), _m_parent() {};
            virtual ~Widget() = default;

            //Transform getters and setters
            void setPosition(float x, float y);
            void setSize(float width, float height);

            UITransform getTransform() const;
            UITransform getAbsoluteTransform() const;

            //Style getters and setters
            void setBackgroundColor(Color color);
            void setBorderColor(Color color);
            void setTextColor(Color color);
            void setFontSize(size_t size);
            void setBorderWidth(float width);
            void setPadding(float padding);
            void setStyle(const UIStyle& style);

            const UIStyle& getStyle() const;

            //Hierarchy management
            void addChild(std::shared_ptr<Widget> child);
            void removeChild(std::shared_ptr<Widget> child);
            const std::vector<std::shared_ptr<Widget>>& getChildren() const;

            //Events
            virtual bool onMouseEnter();
            virtual bool onMouseLeave();
            virtual bool onMouseClick(float x, float y);
            virtual bool onMouseMove(float x, float y);
            virtual bool onKeyPress(rtype::ecs::events::KeyCode key);

            //State management
            void setVisible(bool visible);
            void setEnabled(bool enabled);

            bool isVisible() const;
            bool isEnabled() const;

            bool contains(float x, float y) const;

            //Unique ID
            size_t getID() const;

            //IRenderable interface
            void render();
            virtual void renderSelf() = 0;

        protected:
            UITransform _m_transform;
            UIStyle _m_style;
            std::vector<std::shared_ptr<Widget>> _m_children;
            std::weak_ptr<Widget> _m_parent;
            bool _m_visible = true;
            bool _m_enabled = true;
            size_t _m_id;

        private:
            static size_t s_nextID;
    };
} // namespace rtype::ui

#endif /* !WIDGET_HPP_ */
