/*
** R-Type ECS - UI Library Demo Tab
** Demonstrates the new UI widget library with ECS EventBus integration
*/

#pragma once

#include "DebugTab.hpp"
#include "../../../engine/ui/widgets/TextWidget.hpp"
#include "../../../engine/ui/widgets/ButtonWidget.hpp"
#include "../../../engine/ui/widgets/ProgressBarWidget.hpp"
#include "../../../engine/ui/widgets/PanelWidget.hpp"
#include "../../../engine/ui/UIColor.hpp"
#include <vector>
#include <memory>
#include <cmath>

namespace rtype::ecs::debug {

    /**
     * @brief Demo tab showcasing the UI widget library
     * 
     * Demonstrates:
     * - TextWidget with various alignments
     * - ButtonWidget with hover/click states (via ECS events)
     * - PanelWidget as containers
     * - ProgressBarWidget with animations
     * 
     * Note: In a real application, UIManager would handle event routing.
     * This demo manually routes events from the debug tab's mouse input.
     */
    class UILibraryTab : public IDebugTab {
    public:
        UILibraryTab() {
            initWidgets();
        }

        const char* getName() const override { return "UI Library"; }

        void update(float dt) override {
            m_animTime += dt;
            
            // Animate progress bars using sine waves
            m_progress1 = (std::sin(m_animTime * 0.5f) + 1.0f) / 2.0f;
            m_progress2 = (std::sin(m_animTime * 0.8f + 1.0f) + 1.0f) / 2.0f;
            m_progress3 = (std::sin(m_animTime * 1.2f + 2.0f) + 1.0f) / 2.0f;
            
            // Update click counter display
            if (m_clickCountText) {
                char buf[64];
                snprintf(buf, sizeof(buf), "Button clicked %d times", m_clickCount);
                m_clickCountText->setText(buf);
            }

            // Update progress bars
            if (m_progressBar1) m_progressBar1->setValue(m_progress1);
            if (m_progressBar2) m_progressBar2->setValue(m_progress2);
            if (m_progressBar3) m_progressBar3->setValue(m_progress3);
        }

        void draw(int startY) override {
            const int leftCol = 30;
            const int midCol = 320;
            const int rightCol = 610;
            int y = startY;

            // Title
            DrawText("UI Widget Library Demo", leftCol, y, 22, {100, 200, 255, 255});
            y += 35;

            DrawText("Widgets receive events via ECS EventBus -> UIManager", leftCol, y, 14, {180, 180, 180, 255});
            y += 20;
            DrawText("(This demo manually routes events for demonstration)", leftCol, y, 12, {120, 120, 120, 255});
            y += 25;

            // === LEFT COLUMN: Text Widgets ===
            drawTextWidgetSection(leftCol, y);

            // === MIDDLE COLUMN: Buttons ===
            drawButtonSection(midCol, y);

            // === RIGHT COLUMN: Progress Bars ===
            drawProgressBarSection(rightCol, y);

            // === Bottom: Panel Demo ===
            drawPanelSection(leftCol, y + 280);
        }

        void handleMouse(const MouseInput& mouse) override {
            // Route mouse events to buttons (simulating what UIManager does automatically)
            handleButtonEvents(m_demoButton1, mouse, m_wasOverButton1);
            handleButtonEvents(m_demoButton2, mouse, m_wasOverButton2);
            handleButtonEvents(m_demoButton3, mouse, m_wasOverButton3);
        }

    private:
        // Text widgets
        std::shared_ptr<ui::TextWidget> m_textLeft;
        std::shared_ptr<ui::TextWidget> m_textCenter;
        std::shared_ptr<ui::TextWidget> m_textRight;
        std::shared_ptr<ui::TextWidget> m_clickCountText;

        // Buttons
        std::shared_ptr<ui::ButtonWidget> m_demoButton1;
        std::shared_ptr<ui::ButtonWidget> m_demoButton2;
        std::shared_ptr<ui::ButtonWidget> m_demoButton3;
        bool m_wasOverButton1 = false;
        bool m_wasOverButton2 = false;
        bool m_wasOverButton3 = false;

        // Progress bars
        std::shared_ptr<ui::ProgressBarWidget> m_progressBar1;
        std::shared_ptr<ui::ProgressBarWidget> m_progressBar2;
        std::shared_ptr<ui::ProgressBarWidget> m_progressBar3;

        // Panels
        std::shared_ptr<ui::PanelWidget> m_demoPanel;

        // State
        int m_clickCount = 0;
        float m_progress1 = 0.0f;
        float m_progress2 = 0.0f;
        float m_progress3 = 0.0f;

        /**
         * @brief Route mouse events to a button widget
         * This simulates what UIManager does automatically via EventBus
         */
        void handleButtonEvents(std::shared_ptr<ui::ButtonWidget>& button, 
                               const MouseInput& mouse, bool& wasOver) {
            if (!button) return;
            
            auto transform = button->getAbsoluteTransform();
            bool over = mouse.x >= transform.x && mouse.x < transform.x + transform.width &&
                       mouse.y >= transform.y && mouse.y < transform.y + transform.height;
            
            // Handle enter/leave events
            if (over && !wasOver) {
                button->onMouseEnter();
            } else if (!over && wasOver) {
                button->onMouseLeave();
            }
            
            if (over && mouse.leftPressed) {
                button->onMouseClick();
            }
            
            wasOver = over;
        }

        void initWidgets() {
            // Create text widgets
            m_textLeft = std::make_shared<ui::TextWidget>("Left-aligned text", 14);
            m_textLeft->setTextAlign(ui::TextAlign::Left);
            m_textLeft->setTextColor(ui::UIColor::White());

            m_textCenter = std::make_shared<ui::TextWidget>("Centered text", 14);
            m_textCenter->setTextAlign(ui::TextAlign::Center);
            m_textCenter->setTextColor(ui::UIColor(100, 255, 100, 255));

            m_textRight = std::make_shared<ui::TextWidget>("Right-aligned text", 14);
            m_textRight->setTextAlign(ui::TextAlign::Right);
            m_textRight->setTextColor(ui::UIColor(100, 200, 255, 255));

            m_clickCountText = std::make_shared<ui::TextWidget>("Button clicked 0 times", 14);
            m_clickCountText->setTextColor(ui::UIColor(255, 200, 100, 255));

            // Create buttons
            m_demoButton1 = std::make_shared<ui::ButtonWidget>("Click Me!");
            m_demoButton1->setSize(140, 35);
            m_demoButton1->setBackgroundColor(ui::UIColor(60, 100, 60, 255));
            // m_demoButton1->setStateColor(ui::ButtonState::Hovered, ui::UIColor(80, 140, 80, 255)); // Not supported in current API
            // m_demoButton1->setStateColor(ui::ButtonState::Pressed, ui::UIColor(40, 80, 40, 255)); // Not supported in current API
            m_demoButton1->setOnClick([this]() { m_clickCount++; });

            m_demoButton2 = std::make_shared<ui::ButtonWidget>("Blue Button");
            m_demoButton2->setSize(140, 35);
            m_demoButton2->setBackgroundColor(ui::UIColor(60, 80, 140, 255));
            // m_demoButton2->setStateColor(ui::ButtonState::Hovered, ui::UIColor(80, 100, 180, 255)); // Not supported in current API
            // m_demoButton2->setStateColor(ui::ButtonState::Pressed, ui::UIColor(40, 60, 100, 255)); // Not supported in current API
            m_demoButton2->setOnClick([this]() { m_clickCount += 5; });

            m_demoButton3 = std::make_shared<ui::ButtonWidget>("Disabled");
            m_demoButton3->setSize(140, 35);
            m_demoButton3->setEnabled(false);

            // Create progress bars
            m_progressBar1 = std::make_shared<ui::ProgressBarWidget>(0.0f);
            m_progressBar1->setSize(200, 25);
            m_progressBar1->setFillColor({80, 180, 80, 255});
            m_progressBar1->setShowLabel(true);

            m_progressBar2 = std::make_shared<ui::ProgressBarWidget>(0.0f);
            m_progressBar2->setSize(200, 25);
            m_progressBar2->setFillColor({80, 140, 200, 255});
            m_progressBar2->setShowLabel(true);

            m_progressBar3 = std::make_shared<ui::ProgressBarWidget>(0.0f);
            m_progressBar3->setSize(200, 25);
            m_progressBar3->setFillColor({200, 140, 80, 255});
            m_progressBar3->setShowLabel(true);

            // Create demo panel
            m_demoPanel = std::make_shared<ui::PanelWidget>();
            m_demoPanel->setTitle("Sample Panel");
            m_demoPanel->setSize(250, 120);
        }

        void drawTextWidgetSection(int x, int y) {
            DrawText("TextWidget Examples", x, y, 16, {255, 200, 100, 255});
            y += 25;

            // Draw background boxes for text alignment demo
            DrawRectangle(x, y, 250, 24, {40, 40, 60, 255});
            m_textLeft->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_textLeft->setSize(250, 24);
            m_textLeft->renderSelf();
            y += 30;

            DrawRectangle(x, y, 250, 24, {40, 40, 60, 255});
            m_textCenter->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_textCenter->setSize(250, 24);
            m_textCenter->renderSelf();
            y += 30;

            DrawRectangle(x, y, 250, 24, {40, 40, 60, 255});
            m_textRight->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_textRight->setSize(250, 24);
            m_textRight->renderSelf();
            y += 40;

            DrawText("Features:", x, y, 12, {150, 150, 150, 255});
            y += 18;
            DrawText("- Left/Center/Right alignment", x + 10, y, 11, {120, 120, 120, 255});
            y += 14;
            DrawText("- Vertical alignment options", x + 10, y, 11, {120, 120, 120, 255});
            y += 14;
            DrawText("- Optional word wrapping", x + 10, y, 11, {120, 120, 120, 255});
        }

        void drawButtonSection(int x, int y) {
            DrawText("ButtonWidget Examples", x, y, 16, {255, 200, 100, 255});
            y += 25;

            // Position and render buttons
            m_demoButton1->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_demoButton1->renderSelf();
            y += 45;

            m_demoButton2->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_demoButton2->renderSelf();
            y += 45;

            m_demoButton3->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_demoButton3->renderSelf();
            y += 50;

            // Click counter
            m_clickCountText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_clickCountText->setSize(200, 20);
            m_clickCountText->renderSelf();
            y += 35;

            DrawText("Features:", x, y, 12, {150, 150, 150, 255});
            y += 18;
            DrawText("- Normal/Hover/Pressed/Disabled states", x + 10, y, 11, {120, 120, 120, 255});
            y += 14;
            DrawText("- Click callbacks", x + 10, y, 11, {120, 120, 120, 255});
            y += 14;
            DrawText("- Custom colors per state", x + 10, y, 11, {120, 120, 120, 255});
        }

        void drawProgressBarSection(int x, int y) {
            DrawText("ProgressBarWidget Examples", x, y, 16, {255, 200, 100, 255});
            y += 25;

            DrawText("Slow wave:", x, y, 12, {150, 150, 150, 255});
            y += 16;
            m_progressBar1->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_progressBar1->renderSelf();
            y += 35;

            DrawText("Medium wave:", x, y, 12, {150, 150, 150, 255});
            y += 16;
            m_progressBar2->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_progressBar2->renderSelf();
            y += 35;

            DrawText("Fast wave:", x, y, 12, {150, 150, 150, 255});
            y += 16;
            m_progressBar3->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_progressBar3->renderSelf();
            y += 40;

            DrawText("Features:", x, y, 12, {150, 150, 150, 255});
            y += 18;
            DrawText("- Value from 0.0 to 1.0", x + 10, y, 11, {120, 120, 120, 255});
            y += 14;
            DrawText("- Optional percentage label", x + 10, y, 11, {120, 120, 120, 255});
            y += 14;
            DrawText("- Horizontal/Vertical orientation", x + 10, y, 11, {120, 120, 120, 255});
        }

        void drawPanelSection(int x, int y) {
            DrawText("PanelWidget Example", x, y, 16, {255, 200, 100, 255});
            y += 25;

            m_demoPanel->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_demoPanel->renderSelf();

            // Draw content inside panel
            auto bounds = m_demoPanel->getContentBounds();
            DrawText("Panel with header", 
                     static_cast<int>(bounds.x) + 5, 
                     static_cast<int>(bounds.y) + 5, 
                     12, {200, 200, 200, 255});
            DrawText("Content area below", 
                     static_cast<int>(bounds.x) + 5, 
                     static_cast<int>(bounds.y) + 22, 
                     12, {150, 150, 150, 255});

            // Features list next to panel
            int featX = x + 270;
            DrawText("Features:", featX, y, 12, {150, 150, 150, 255});
            y += 18;
            DrawText("- Optional header with title", featX + 10, y, 11, {120, 120, 120, 255});
            y += 14;
            DrawText("- Background and border styling", featX + 10, y, 11, {120, 120, 120, 255});
            y += 14;
            DrawText("- Content bounds calculation", featX + 10, y, 11, {120, 120, 120, 255});
            y += 14;
            DrawText("- Widget hierarchy support", featX + 10, y, 11, {120, 120, 120, 255});
        }
    };

} // namespace rtype::ecs::debug
