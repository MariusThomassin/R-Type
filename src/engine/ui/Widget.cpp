/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Widget
*/

#include <algorithm>
#include "Widget.hpp"

namespace rtype::ui {
    size_t Widget::s_nextID = 0;

    Widget::Widget() : _m_transform(), _m_style(), _m_children(), _m_parent(),
                       _m_visible(true), _m_enabled(true), _m_focused(false), _m_id(s_nextID++)
    {
    }

    void Widget::setPosition(float x, float y)
    {
        _m_transform.x = x;
        _m_transform.y = y;
    }

    void Widget::setSize(float width, float height)
    {
        _m_transform.width = width;
        _m_transform.height = height;
    }

    UITransform Widget::getTransform() const
    {
        return _m_transform;
    }

    UITransform Widget::getAbsoluteTransform() const
    {
        if (auto parent_ptr = _m_parent.lock()) {
            UITransform parent_abs = parent_ptr->getAbsoluteTransform();
            return {_m_transform.x + parent_abs.x, _m_transform.y + parent_abs.y, _m_transform.width, _m_transform.height};
        }
        return _m_transform;
    }

    void Widget::setBackgroundColor(UIColor color)
    {
        _m_style.backgroundColor = color;
    }

    void Widget::setBorderColor(UIColor color)
    {
        _m_style.borderColor = color;
    }

    void Widget::setTextColor(UIColor color)
    {
        _m_style.textColor = color;
    }

    void Widget::setFontSize(size_t size)
    {
        _m_style.fontSize = size;
    }

    void Widget::setBorderWidth(float width)
    {
        _m_style.borderWidth = width;
    }

    void Widget::setPadding(float padding)
    {
        _m_style.padding = padding;
    }

    void Widget::setStyle(const UIStyle& style)
    {
        _m_style = style;
    }

    const UIStyle& Widget::getStyle() const
    {
        return _m_style;
    }

    void Widget::addChild(std::shared_ptr<Widget> child)
    {
        if (!child)
            return;

        child->_m_parent = shared_from_this();
        _m_children.push_back(child);
    }

    void Widget::removeChild(std::shared_ptr<Widget> child)
    {
        if (!child)
            return;

        _m_children.erase(std::remove_if(_m_children.begin(), _m_children.end(),
            [&](const std::shared_ptr<Widget>& w) {
                return w.get() == child.get();
            }
        ), _m_children.end());

        std::shared_ptr<Widget> self = shared_from_this();
        if (child->_m_parent.lock() == self) {
            child->_m_parent.reset();
        }
    }

    const std::vector<std::shared_ptr<Widget>>& Widget::getChildren() const
    {
        return _m_children;
    }

    std::shared_ptr<Widget> Widget::getParent() const
    {
        return _m_parent.lock();
    }

    bool Widget::onMouseEnter()
    {
        return false;
    }

    bool Widget::onMouseLeave()
    {
        return false;
    }

    bool Widget::onMouseClick()
    {
        return false;
    }

    bool Widget::onMouseMove(float x, float y)
    {
        (void)x;
        (void)y;
        return false;
    }

    bool Widget::onMouseWheel(float delta)
    {
        (void)delta;
        return false;
    }

    bool Widget::onMouseRelease()
    {
        return false;
    }

    bool Widget::onKeyPress(rtype::ecs::events::KeyCode key)
    {
        (void)key;
        return false;
    }

    bool Widget::onKeyRelease(rtype::ecs::events::KeyCode key)
    {
        (void)key;
        return false;
    }

    void Widget::onFocus()
    {
        _m_focused = true;
    }

    void Widget::onBlur()
    {
        _m_focused = false;
    }

    void Widget::setVisible(bool visible)
    {
        _m_visible = visible;
    }

    void Widget::setEnabled(bool enabled)
    {
        _m_enabled = enabled;
    }
    
    bool Widget::isVisible() const
    {
        return _m_visible;
    }

    bool Widget::isEnabled() const
    {
        return _m_enabled;
    }

    bool Widget::isFocused() const
    {
        return _m_focused;
    }

    bool Widget::contains(float x, float y) const
    {
        UITransform absTransform = getAbsoluteTransform();
        return (x >= absTransform.x && x <= absTransform.x + absTransform.width &&
                y >= absTransform.y && y <= absTransform.y + absTransform.height);
    }

    size_t Widget::getID() const
    {
        return _m_id;
    }

    void Widget::update(float deltaTime)
    {
        if (!_m_visible)
            return;

        for (auto child : _m_children) {
            child->update(deltaTime);
        }
    }

    void Widget::render() const
    {
        if (!_m_visible)
            return;

        renderSelf();

        for (auto& child : _m_children) {
            child->render();
        }
    }

} // namespace rtype::ui
