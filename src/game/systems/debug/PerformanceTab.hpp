/*
** R-Type ECS - Performance Debug Tab
** Shows frame timing and performance metrics using UI widget library
*/

#pragma once

#include "DebugTab.hpp"
#include "src/engine/ui/Widgets.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <memory>

namespace rtype::ecs::debug {

    class PerformanceTab : public IDebugTab {
    public:
        PerformanceTab() {
            initWidgets();
        }

        const char* getName() const override { return "Performance"; }

        void update(float dt) override {
            m_animTime += dt;
            m_frameTime = dt;
            
            m_frameTimes.push_back(dt);
            if (m_frameTimes.size() > 120) {
                m_frameTimes.erase(m_frameTimes.begin());
            }

            // Update text widgets
            char buf[128];
            snprintf(buf, sizeof(buf), "FPS: %d", GetFPS());
            m_fpsText->setText(buf);

            snprintf(buf, sizeof(buf), "Frame Time: %.3f ms", m_frameTime * 1000);
            m_frameTimeText->setText(buf);

            float avg = 0;
            for (float ft : m_frameTimes) avg += ft;
            if (!m_frameTimes.empty()) avg /= m_frameTimes.size();
            snprintf(buf, sizeof(buf), "Avg (120 frames): %.3f ms", avg * 1000);
            m_avgText->setText(buf);

            float minFt = 999, maxFt = 0;
            for (float ft : m_frameTimes) {
                minFt = std::min(minFt, ft);
                maxFt = std::max(maxFt, ft);
            }
            snprintf(buf, sizeof(buf), "Min: %.2f ms  Max: %.2f ms", minFt * 1000, maxFt * 1000);
            m_minMaxText->setText(buf);
        }

        void draw(int y) override {
            const int x = 30;

            // Title
            m_titleText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_titleText->renderSelf();
            y += 35;

            // FPS (large)
            m_fpsText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_fpsText->renderSelf();
            y += 30;

            // Frame time
            m_frameTimeText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_frameTimeText->renderSelf();
            y += 24;

            // Average
            m_avgText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_avgText->renderSelf();
            y += 24;

            // Min/Max
            m_minMaxText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_minMaxText->renderSelf();
            y += 30;

            // Graph (custom drawing - keeping manual as it's visualization-specific)
            drawFrameGraph(y);
        }

    private:
        float m_frameTime = 0.0f;
        std::vector<float> m_frameTimes;

        // Text widgets
        std::shared_ptr<ui::TextWidget> m_titleText;
        std::shared_ptr<ui::TextWidget> m_fpsText;
        std::shared_ptr<ui::TextWidget> m_frameTimeText;
        std::shared_ptr<ui::TextWidget> m_avgText;
        std::shared_ptr<ui::TextWidget> m_minMaxText;

        void initWidgets() {
            m_titleText = std::make_shared<ui::TextWidget>("Performance Metrics", 20);
            m_titleText->setTextColor(ui::Color::White());
            m_titleText->setBackgroundColor(ui::Color::Transparent());

            m_fpsText = std::make_shared<ui::TextWidget>("FPS: 0", 20);
            m_fpsText->setTextColor(ui::Color(100, 255, 100, 255));
            m_fpsText->setBackgroundColor(ui::Color::Transparent());

            m_frameTimeText = std::make_shared<ui::TextWidget>("Frame Time: 0.000 ms", 16);
            m_frameTimeText->setTextColor(ui::Color::White());
            m_frameTimeText->setBackgroundColor(ui::Color::Transparent());

            m_avgText = std::make_shared<ui::TextWidget>("Avg (120 frames): 0.000 ms", 16);
            m_avgText->setTextColor(ui::Color::White());
            m_avgText->setBackgroundColor(ui::Color::Transparent());

            m_minMaxText = std::make_shared<ui::TextWidget>("Min: 0.00 ms  Max: 0.00 ms", 14);
            m_minMaxText->setTextColor(ui::Color(150, 150, 150, 255));
            m_minMaxText->setBackgroundColor(ui::Color::Transparent());
        }

        void drawFrameGraph(int y) {
            DrawText("Frame Time Graph:", 30, y, 14, {150, 150, 150, 255}); y += 22;

            int gx = 30, gy = y, gw = m_screenWidth - 60, gh = 150;
            DrawRectangle(gx, gy, gw, gh, {30, 30, 40, 255});
            
            int targetY = gy + gh - (int)(16.67f / 50.0f * gh);
            DrawLine(gx, targetY, gx + gw, targetY, {100, 255, 100, 80});
            DrawText("16.67ms (60fps)", gx + gw - 100, targetY - 12, 10, {100, 255, 100, 200});

            int warnY = gy + gh - (int)(33.33f / 50.0f * gh);
            DrawLine(gx, warnY, gx + gw, warnY, {255, 200, 100, 60});
            DrawText("33.33ms (30fps)", gx + gw - 100, warnY - 12, 10, {255, 200, 100, 150});

            if (m_frameTimes.empty()) return;

            float barW = (float)gw / m_frameTimes.size();
            for (size_t i = 0; i < m_frameTimes.size(); i++) {
                float ft = m_frameTimes[i] * 1000;
                int barH = std::min((int)(ft / 50.0f * gh), gh);
                
                Color c;
                if (ft > 33.33f) c = {255, 80, 80, 255};       // Red: bad
                else if (ft > 16.67f) c = {255, 200, 100, 255}; // Orange: warning
                else c = {100, 200, 255, 255};                   // Blue: good
                
                DrawRectangle(gx + (int)(i * barW), gy + gh - barH, std::max(1, (int)barW - 1), barH, c);
            }
        }
    };

} // namespace rtype::ecs::debug
