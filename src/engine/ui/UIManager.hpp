/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** UIManager
*/

#ifndef UIMANAGER_HPP_
#define UIMANAGER_HPP_

#include <memory>
#include <vector>
#include "Widget.hpp"

namespace rtype::ui
{
    class UIManager {
    public:
        UIManager();
        ~UIManager();

        void addWidget(std::shared_ptr<Widget> widget);
        void removeWidget(std::shared_ptr<Widget> widget);

        void update(float deltaTime);
        void render();

    private:
        std::unordered_map<size_t, std::shared_ptr<Widget>> widgets;

        std::shared_ptr<Widget> m_hoveredWidget;
        std::shared_ptr<Widget> m_focusedWidget;
    };
} // namespace rtype::ui

#endif /* !UIMANAGER_HPP_ */
