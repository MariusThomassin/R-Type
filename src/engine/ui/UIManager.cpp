/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** UIManager - Manages UI widgets with ECS EventBus integration
*/

#include "UIManager.hpp"

namespace rtype::ui {
    UIManager::UIManager(rtype::ecs::EventBus& eventBus) : m_eventBus(eventBus)
    {
        // Subscribe to all input events from EventBus
        m_mouseClickSubId = m_eventBus.subscribe<rtype::ecs::events::MouseButtonPressedEvent>(
            [this](const rtype::ecs::events::MouseButtonPressedEvent& e) {
                handleMouseClick(e);
            }
        );

        m_mouseReleaseSubId = m_eventBus.subscribe<rtype::ecs::events::MouseButtonReleasedEvent>(
            [this](const rtype::ecs::events::MouseButtonReleasedEvent& e) {
                handleMouseRelease(e);
            }
        );

        m_mouseMoveSubId = m_eventBus.subscribe<rtype::ecs::events::MouseMoveEvent>(
            [this](const rtype::ecs::events::MouseMoveEvent& e) {
                handleMouseMove(e);
            }
        );

        m_mouseWheelSubId = m_eventBus.subscribe<rtype::ecs::events::MouseWheelEvent>(
            [this](const rtype::ecs::events::MouseWheelEvent& e) {
                handleMouseWheel(e);
            }
        );

        m_keyPressSubId = m_eventBus.subscribe<rtype::ecs::events::KeyPressedEvent>(
            [this](const rtype::ecs::events::KeyPressedEvent& e) {
                handleKeyPress(e);
            }
        );

        m_keyReleaseSubId = m_eventBus.subscribe<rtype::ecs::events::KeyReleasedEvent>(
            [this](const rtype::ecs::events::KeyReleasedEvent& e) {
                handleKeyRelease(e);
            }
        );
    }

    UIManager::~UIManager()
    {
        // Unsubscribe from all events
        m_eventBus.unsubscribe<rtype::ecs::events::MouseButtonPressedEvent>(m_mouseClickSubId);
        m_eventBus.unsubscribe<rtype::ecs::events::MouseButtonReleasedEvent>(m_mouseReleaseSubId);
        m_eventBus.unsubscribe<rtype::ecs::events::MouseMoveEvent>(m_mouseMoveSubId);
        m_eventBus.unsubscribe<rtype::ecs::events::MouseWheelEvent>(m_mouseWheelSubId);
        m_eventBus.unsubscribe<rtype::ecs::events::KeyPressedEvent>(m_keyPressSubId);
        m_eventBus.unsubscribe<rtype::ecs::events::KeyReleasedEvent>(m_keyReleaseSubId);
    }

    void UIManager::addWidget(std::shared_ptr<Widget> widget)
    {
        if (widget) {
            m_widgets.push_back(widget);
        }
    }

    void UIManager::removeWidget(std::shared_ptr<Widget> widget)
    {
        m_widgets.erase(
            std::remove(m_widgets.begin(), m_widgets.end(), widget),
            m_widgets.end()
        );

        if (m_hoveredWidget == widget) {
            m_hoveredWidget->onMouseLeave();
            m_hoveredWidget = nullptr;
        }

        if (m_focusedWidget == widget) {
            m_focusedWidget->onBlur();
            m_focusedWidget = nullptr;
        }
    }

    void UIManager::clearWidgets()
    {
        if (m_hoveredWidget) {
            m_hoveredWidget->onMouseLeave();
            m_hoveredWidget = nullptr;
        }
        if (m_focusedWidget) {
            m_focusedWidget->onBlur();
            m_focusedWidget = nullptr;
        }
        m_widgets.clear();
    }

    const std::vector<std::shared_ptr<Widget>>& UIManager::getWidgets() const
    {
        return m_widgets;
    }

    void UIManager::setFocus(std::shared_ptr<Widget> widget)
    {
        if (m_focusedWidget == widget) return;

        if (m_focusedWidget) {
            m_focusedWidget->_m_focused = false;
            m_focusedWidget->onBlur();
        }

        m_focusedWidget = widget;

        if (m_focusedWidget) {
            m_focusedWidget->_m_focused = true;
            m_focusedWidget->onFocus();
        }
    }

    std::shared_ptr<Widget> UIManager::getFocusedWidget() const
    {
        return m_focusedWidget;
    }

    std::shared_ptr<Widget> UIManager::getHoveredWidget() const
    {
        return m_hoveredWidget;
    }

    std::shared_ptr<Widget> UIManager::findWidgetAt(float x, float y)
    {
        // Search in reverse order (topmost first)
        for (auto it = m_widgets.rbegin(); it != m_widgets.rend(); ++it) {
            std::shared_ptr<Widget> widget = *it;
            if (widget->isEnabled() && widget->isVisible()) {
                // First check if any child widgets contain the point
                auto childWidget = findWidgetAtRecursive(*it, x, y);
                if (childWidget) {
                    return childWidget;
                }
                
                // If no child contains it, check if the parent widget does
                if (widget->contains(x, y)) {
                    return widget;
                }
            }
        }
        return nullptr;
    }

    std::shared_ptr<Widget> UIManager::findWidgetAtRecursive(std::shared_ptr<Widget> parent, float x, float y)
    {
        if (!parent || !parent->isEnabled() || !parent->isVisible()) {
            return nullptr;
        }

        // Search through children in reverse order (topmost first)
        const auto& children = parent->getChildren();
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            std::shared_ptr<Widget> child = *it;
            if (child && child->isEnabled() && child->isVisible()) {
                // First check grandchildren recursively
                auto grandChild = findWidgetAtRecursive(child, x, y);
                if (grandChild) {
                    return grandChild;
                }
                
                // Then check if this child contains the point
                if (child->contains(x, y)) {
                    return child;
                }
            }
        }
        
        return nullptr;
    }

    void UIManager::handleMouseClick(const rtype::ecs::events::MouseButtonPressedEvent& event)
    {
        // Only handle left mouse button for now
        if (event.button != rtype::ecs::events::MouseButton::Left) return;

        auto widget = findWidgetAt(event.x, event.y);
        
        if (widget) {
            // Update focus
            setFocus(widget);
            widget->onMouseClick();
        } else {
            // Clicked outside all widgets - clear focus
            setFocus(nullptr);
        }
    }

    void UIManager::handleMouseMove(const rtype::ecs::events::MouseMoveEvent& event)
    {
        auto newHoveredWidget = findWidgetAt(event.x, event.y);

        // Handle hover state changes
        if (newHoveredWidget != m_hoveredWidget) {
            if (m_hoveredWidget) {
                m_hoveredWidget->onMouseLeave();
            }
            if (newHoveredWidget) {
                newHoveredWidget->onMouseEnter();
            }
            m_hoveredWidget = newHoveredWidget;
        }

        // Send move event to hovered widget
        if (m_hoveredWidget) {
            UITransform absTransform = m_hoveredWidget->getAbsoluteTransform();
            float localX = event.x - absTransform.x;
            float localY = event.y - absTransform.y;
            m_hoveredWidget->onMouseMove(localX, localY);
        }
    }

    void UIManager::handleMouseWheel(const rtype::ecs::events::MouseWheelEvent& event)
    {
        // Send wheel event to hovered widget
        if (m_hoveredWidget && m_hoveredWidget->isEnabled()) {
            m_hoveredWidget->onMouseWheel(event.delta);
        }
    }

    void UIManager::handleKeyPress(const rtype::ecs::events::KeyPressedEvent& event)
    {
        // Send key events to focused widget
        if (m_focusedWidget && m_focusedWidget->isEnabled() && m_focusedWidget->isVisible()) {
            m_focusedWidget->onKeyPress(event.key);
        }
    }

    void UIManager::handleKeyRelease(const rtype::ecs::events::KeyReleasedEvent& event)
    {
        if (m_focusedWidget && m_focusedWidget->isEnabled() && m_focusedWidget->isVisible()) {
            m_focusedWidget->onKeyRelease(event.key);
        }
    }

    void UIManager::update(float deltaTime)
    {
        for (const auto& widget : m_widgets) {
            if (widget->isVisible()) {
                widget->update(deltaTime);
            }
        }
    }

    void UIManager::render() const
    {
        for (const auto& widget : m_widgets) {
            if (widget->isVisible()) {
                widget->render();
            }
        }
    }

    bool UIManager::isCapturingMouse() const
    {
        return m_hoveredWidget != nullptr;
    }

    bool UIManager::isCapturingKeyboard() const
    {
        return m_focusedWidget != nullptr;
    }

    void UIManager::handleMouseRelease(const rtype::ecs::events::MouseButtonReleasedEvent& event)
    {
        // Forward mouse release to hovered widget
        if (m_hoveredWidget && m_hoveredWidget->isEnabled()) {
            m_hoveredWidget->onMouseRelease();
        }
        (void)event; // Suppress unused parameter warning for event details
    }

} // namespace rtype::ui
