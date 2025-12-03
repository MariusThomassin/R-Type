/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** UIManager
*/

#include "UIManager.hpp"

namespace rtype::ui {
    UIManager::UIManager(rtype::ecs::EventBus& eventBus)  : m_eventBus(eventBus)
    {
    }

    UIManager::~UIManager()
    {
    }

    void UIManager::addWidget(std::shared_ptr<Widget> widget)
    {
        widgets.push_back(widget);
    }

    void UIManager::removeWidget(std::shared_ptr<Widget> widget)
    {
        widgets.erase(
            std::remove(widgets.begin(), widgets.end(), widget),
            widgets.end()
        );

        if (m_hoveredWidget == widget) {
            m_hoveredWidget->onMouseLeave();
            m_hoveredWidget = nullptr;
        }

        if (m_focusedWidget == widget) {
            m_focusedWidget = nullptr;
        }
    }

    void UIManager::handleMouseClick(const rtype::ecs::events::MouseButtonPressedEvent& event)
    {
        for (auto it = widgets.rbegin(); it != widgets.rend(); ++it) {
            std::shared_ptr<Widget> widget = *it;

            if (widget->isEnabled() && widget->isVisible() && widget->contains(event.x, event.y)) {
                UITransform absTransform = widget->getAbsoluteTransform();
                float localX = event.x - absTransform.x;
                float localY = event.y - absTransform.y;
                if (widget->onMouseClick(localX, localY)) {
                    m_focusedWidget = widget;
                    return;
                }
            }
        }
        m_focusedWidget = nullptr;
    }

    void UIManager::handleMouseMove(const rtype::ecs::events::MouseMoveEvent& event)
    {
        std::shared_ptr<Widget> newHoveredWidget = nullptr;

        for (auto it = widgets.rbegin(); it != widgets.rend(); ++it) {
            std::shared_ptr<Widget> widget = *it;

            if (widget->isEnabled() && widget->isVisible() && widget->contains(event.x, event.y)) {
                newHoveredWidget = widget;
                break;
            }
        }

        if (newHoveredWidget != m_hoveredWidget) {
            if (m_hoveredWidget) {
                m_hoveredWidget->onMouseLeave();
            }
            if (newHoveredWidget) {
                newHoveredWidget->onMouseEnter();
            }
            m_hoveredWidget = newHoveredWidget;
        }

        if (m_hoveredWidget) {
            UITransform absTransform = m_hoveredWidget->getAbsoluteTransform();
            float localX = event.x - absTransform.x;
            float localY = event.y - absTransform.y;
            m_hoveredWidget->onMouseMove(localX, localY);
        }
    }

    void UIManager::handleKeyPress(const rtype::ecs::events::KeyCode& key)
    {
        if (m_focusedWidget) {
            if (m_focusedWidget->isEnabled() && m_focusedWidget->isVisible()) {
                m_focusedWidget->onKeyPress(key);
            }
        }
    }

    void UIManager::update(float deltaTime)
    {
        for (const auto& widget : widgets) {
            if (widget->isVisible()) {
                widget->update(deltaTime);
            }
        }
    }

    void UIManager::clear()
    {
        widgets.clear();
        m_hoveredWidget = nullptr;
        m_focusedWidget = nullptr;
    }

    void UIManager::render(const rtype::ecs::RenderContext& ctx)
    {
        for (const auto& widget : widgets) {
            if (widget->isVisible()) {
                rtype::ecs::TransformComponent transform; // Default transform
                widget->render(transform, ctx);
            }
        }
    }
}
