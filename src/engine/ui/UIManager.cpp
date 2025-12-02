/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** UIManager
*/

#include "UIManager.hpp"

namespace rtype::ui {
    UIManager::UIManager()
    {
    }

    UIManager::~UIManager()
    {
    }

    void UIManager::addWidget(std::shared_ptr<Widget> widget)
    {
        size_t id = widget->getID();
        widgets[id] = widget;
    }

    void UIManager::removeWidget(std::shared_ptr<Widget> widget)
    {
        size_t id = widget->getID();
        widgets.erase(id);
    }

    void UIManager::update(float deltaTime)
    {
        // Update logic for widgets can be added here
        (void)deltaTime;
    }

    void UIManager::render()
    {
        for (auto& [id, widget] : widgets) {
            if (widget->isVisible()) {
                widget->render();
            }
        }
    }
}
