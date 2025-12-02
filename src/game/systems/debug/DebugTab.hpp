/*
** R-Type ECS - Debug Tab Interface
** Base interface for debug overlay tabs with event-based input
*/

#pragma once

#include "../../../engine/ecs/core/Registry.hpp"
#include "../../../engine/ecs/events/InputEvents.hpp"
#include <raylib.h>
#include <unordered_map>
#include <string>
#include <algorithm>

namespace rtype::ecs::debug {

    /**
     * @brief Mouse input state for debug tabs (from events)
     */
    struct MouseInput {
        float x = 0, y = 0;
        float wheelDelta = 0;
        bool leftPressed = false;
        bool leftDown = false;
        bool rightPressed = false;
        bool rightDown = false;
    };

    /**
     * @brief Base interface for debug tabs with event-based input
     */
    class IDebugTab {
    public:
        virtual ~IDebugTab() = default;

        virtual const char* getName() const = 0;

        virtual void update(float dt) = 0;
        virtual void draw(int startY) = 0;
        
        /**
         * @brief Handle mouse input from events
         * @param mouse Current mouse state from events
         */
        virtual void handleMouse(const MouseInput& mouse) { 
            (void)mouse; 
        }

        /**
         * @brief Update mouse state from events (called by DebugSystem)
         */
        void updateMouseState(const MouseInput& mouse) {
            m_mouse = mouse;
            handleMouse(mouse);
        }

        void setRegistry(Registry* registry) { m_registry = registry; }
        void setTextures(const std::unordered_map<std::string, Texture2D>* textures) { 
            m_textures = textures; 
        }
        void setScreenSize(int w, int h) { m_screenWidth = w; m_screenHeight = h; }

    protected:
        Registry* m_registry = nullptr;
        const std::unordered_map<std::string, Texture2D>* m_textures = nullptr;
        int m_screenWidth = 1280;
        int m_screenHeight = 720;
        float m_animTime = 0.0f;
        MouseInput m_mouse;  // Current mouse state from events

        // ==================== UI Helpers ====================

        /**
         * @brief Check if mouse is over a rectangle (uses m_mouse)
         */
        bool isMouseOver(int x, int y, int w, int h) const {
            return m_mouse.x >= x && m_mouse.x < x + w && 
                   m_mouse.y >= y && m_mouse.y < y + h;
        }

        /**
         * @brief Draw a clickable button, returns true if clicked (uses m_mouse)
         */
        bool drawButton(const char* text, int x, int y, int w, int h, 
                       Color normal, Color hover, Color textColor) {
            bool over = isMouseOver(x, y, w, h);
            DrawRectangle(x, y, w, h, over ? hover : normal);
            DrawRectangleLines(x, y, w, h, {100, 100, 120, 255});
            
            int textW = MeasureText(text, 14);
            DrawText(text, x + (w - textW) / 2, y + (h - 14) / 2, 14, textColor);
            
            return over && m_mouse.leftPressed;
        }

        /**
         * @brief Draw a clickable list item, returns true if clicked (uses m_mouse)
         */
        bool drawListItem(const char* text, int x, int y, int w, int h, bool selected) {
            bool over = isMouseOver(x, y, w, h);
            
            Color bg = selected ? Color{60, 80, 120, 255} : 
                       (over ? Color{50, 50, 70, 255} : Color{35, 35, 50, 255});
            DrawRectangle(x, y, w, h, bg);
            
            Color textCol = selected ? WHITE : (over ? Color{220, 220, 220, 255} : Color{180, 180, 180, 255});
            DrawText(text, x + 8, y + (h - 14) / 2, 14, textCol);
            
            return over && m_mouse.leftPressed;
        }

        /**
         * @brief Handle scroll in an area (uses m_mouse.wheelDelta)
         * @return Updated scroll offset
         */
        int handleScroll(int x, int y, int w, int h, int contentHeight, int& scrollOffset) {
            if (isMouseOver(x, y, w, h)) {
                scrollOffset -= static_cast<int>(m_mouse.wheelDelta * 30);
                
                int maxScroll = std::max(0, contentHeight - h);
                scrollOffset = std::clamp(scrollOffset, 0, maxScroll);
            }
            return scrollOffset;
        }
    };

} // namespace rtype::ecs::debug
