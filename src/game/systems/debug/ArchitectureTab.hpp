/*
** R-Type ECS - Architecture Tab
** Displays ECS architecture diagram and pattern explanation
*/

#pragma once

#include "DebugTab.hpp"
#include <cmath>

namespace rtype::ecs::debug {

    class ArchitectureTab : public IDebugTab {
    public:
        const char* getName() const override { return "Architecture"; }

        void update(float dt) override {
            m_animTime += dt;
        }

        void draw(int y) override {
            DrawText("ECS Architecture", 30, y, 20, WHITE);
            y += 35;

            drawArchitectureDiagram(y);
            y += 200;

            DrawText("Self-Rendering Component Pattern (IRenderable):", 30, y, 16, {100, 200, 255, 255});
            y += 28;

            const char* pattern[] = {
                "1. Components implement IRenderable::render(transform, ctx)",
                "2. RenderSystem creates RenderContext with textures & timing",
                "3. RenderSystem iterates entities, calls component.render()",
                "4. Each component draws itself (SpriteComponent, PlayerShipComponent...)",
                "5. Result: ~220 line RenderSystem, rendering logic in components"
            };
            
            for (int i = 0; i < 5; ++i) {
                float offset = std::sin(m_animTime * 2.0f + i * 0.3f);
                float pulse = 0.8f + 0.2f * offset;
                DrawText(pattern[i], 40, y, 14, {
                    static_cast<unsigned char>(180 * pulse), 
                    static_cast<unsigned char>(180 * pulse), 
                    static_cast<unsigned char>(180 * pulse), 255
                });
                y += 22;
            }
            y += 20;

            DrawText("Event-Driven Input Architecture:", 30, y, 16, {255, 200, 100, 255});
            y += 28;

            const char* events[] = {
                "InputManager polls Raylib -> emits KeyStateEvent/KeyPressedEvent",
                "Systems subscribe to events: inputSystem.subscribe<ShootEvent>(...)",
                "EventBus decouples input from game logic",
                "Fixed timestep (60Hz) for game logic, uncapped rendering"
            };
            
            for (const char* line : events) {
                DrawText(line, 40, y, 14, {180, 180, 180, 255});
                y += 20;
            }
        }

        void handleMouse(const MouseInput& mouse) override {
            m_hoveredBox = -1;
            
            int centerX = m_screenWidth / 2;
            int baseY = 125;  // Approximate Y from draw
            
            if (isMouseOver(centerX - 60, baseY + 60, 120, 50)) m_hoveredBox = 0;  // Registry
            if (isMouseOver(centerX - 180, baseY + 20, 90, 35)) m_hoveredBox = 1;  // Entities
            if (isMouseOver(centerX + 90, baseY + 20, 110, 35)) m_hoveredBox = 2;  // Components
            if (isMouseOver(centerX - 60, baseY + 130, 120, 35)) m_hoveredBox = 3; // Systems
            if (isMouseOver(centerX + 120, baseY + 100, 100, 30)) m_hoveredBox = 4; // EventBus
            
            (void)mouse;
        }

    private:
        int m_hoveredBox = -1;

        void drawArchitectureDiagram(int y) {
            int centerX = m_screenWidth / 2;
            
            int regX = centerX - 60, regY = y + 60;
            bool regHover = (m_hoveredBox == 0);
            DrawRectangle(regX, regY, 120, 50, regHover ? Color{80, 80, 130, 255} : Color{60, 60, 100, 255});
            DrawRectangleLines(regX, regY, 120, 50, regHover ? Color{150, 200, 255, 255} : Color{100, 150, 255, 255});
            DrawText("Registry", regX + 25, regY + 17, 16, WHITE);
            if (regHover) {
                DrawText("Central entity & component manager", regX - 50, regY + 55, 12, {150, 150, 200, 255});
            }

            int entX = centerX - 180, entY = y + 20;
            bool entHover = (m_hoveredBox == 1);
            DrawRectangle(entX, entY, 90, 35, entHover ? Color{100, 80, 80, 255} : Color{80, 60, 60, 255});
            DrawRectangleLines(entX, entY, 90, 35, entHover ? Color{255, 200, 150, 255} : Color{255, 150, 100, 255});
            DrawText("Entities", entX + 15, entY + 10, 14, WHITE);
            DrawLine(entX + 90, entY + 17, regX, regY + 25, {150, 150, 150, 255});
            if (entHover) {
                DrawText("Unique IDs (uint64_t)", entX - 20, entY + 40, 12, {200, 150, 150, 255});
            }

            int compX = centerX + 90, compY = y + 20;
            bool compHover = (m_hoveredBox == 2);
            DrawRectangle(compX, compY, 110, 35, compHover ? Color{80, 100, 80, 255} : Color{60, 80, 60, 255});
            DrawRectangleLines(compX, compY, 110, 35, compHover ? Color{150, 255, 150, 255} : Color{100, 255, 100, 255});
            DrawText("Components", compX + 12, compY + 10, 14, WHITE);
            DrawLine(regX + 120, regY + 25, compX, compY + 17, {150, 150, 150, 255});
            if (compHover) {
                DrawText("Data structs (IComponent)", compX - 10, compY + 40, 12, {150, 200, 150, 255});
            }

            int sysX = centerX - 60, sysY = y + 130;
            bool sysHover = (m_hoveredBox == 3);
            DrawRectangle(sysX, sysY, 120, 35, sysHover ? Color{80, 80, 100, 255} : Color{60, 60, 80, 255});
            DrawRectangleLines(sysX, sysY, 120, 35, sysHover ? Color{200, 200, 255, 255} : Color{150, 150, 255, 255});
            DrawText("Systems", sysX + 30, sysY + 10, 14, WHITE);
            DrawLine(regX + 60, regY + 50, sysX + 60, sysY, {150, 150, 150, 255});
            if (sysHover) {
                DrawText("Logic (ISystem)", sysX + 20, sysY + 40, 12, {150, 150, 200, 255});
            }

            int ebX = centerX + 120, ebY = y + 100;
            bool ebHover = (m_hoveredBox == 4);
            DrawRectangle(ebX, ebY, 100, 30, ebHover ? Color{100, 80, 100, 255} : Color{80, 60, 80, 255});
            DrawRectangleLines(ebX, ebY, 100, 30, ebHover ? Color{255, 200, 255, 255} : Color{200, 150, 200, 255});
            DrawText("EventBus", ebX + 18, ebY + 8, 13, WHITE);
            DrawLine(sysX + 120, sysY + 17, ebX, ebY + 15, {150, 100, 150, 255});
            if (ebHover) {
                DrawText("Pub/Sub messaging", ebX - 10, ebY + 35, 12, {200, 150, 200, 255});
            }

            float flowPhase = std::fmod(m_animTime * 2.0f, 1.0f);
            int flowX = static_cast<int>(entX + 90 + (regX - entX - 90) * flowPhase);
            DrawCircle(flowX, entY + 17, 3, {255, 200, 100, 200});
        }
    };

} // namespace rtype::ecs::debug
