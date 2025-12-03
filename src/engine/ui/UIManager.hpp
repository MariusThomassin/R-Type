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
#include "../ecs/core/EventBus.hpp"
#include "../ecs/events/InputEvents.hpp"
#include "../ecs/components/TransformComponent.hpp"

namespace rtype::ui
{
    class UIManager {
    public:
        UIManager(rtype::ecs::EventBus& eventBus);
        ~UIManager();

        void addWidget(std::shared_ptr<Widget> widget);
        void removeWidget(std::shared_ptr<Widget> widget);
        void clear();

        void handleMouseClick(const rtype::ecs::events::MouseButtonPressedEvent& event);
        void handleMouseMove(const rtype::ecs::events::MouseMoveEvent& event);
        void handleKeyPress(const rtype::ecs::events::KeyCode& key);

        void update(float deltaTime);
        void render(const rtype::ecs::RenderContext& ctx);

    private:
        std::vector<std::shared_ptr<Widget>> widgets;

        std::shared_ptr<Widget> m_hoveredWidget;
        std::shared_ptr<Widget> m_focusedWidget;

        rtype::ecs::EventBus& m_eventBus;
    };
} // namespace rtype::ui

#endif /* !UIMANAGER_HPP_ */
