/*
** R-Type ECS - Performance Debug Tab
** Shows frame timing and performance metrics
*/

#pragma once

#include "DebugTab.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

namespace rtype::ecs::debug {

    class PerformanceTab : public IDebugTab {
    public:
        const char* getName() const override { return "Performance"; }

        void update(float dt) override {
            m_animTime += dt;
            m_frameTime = dt;
            
            m_frameTimes.push_back(dt);
            if (m_frameTimes.size() > 120) {
                m_frameTimes.erase(m_frameTimes.begin());
            }
        }

        void draw(int y) override {
            DrawText("Performance Metrics", 30, y, 20, WHITE); y += 35;

            char buf[128];
            snprintf(buf, sizeof(buf), "FPS: %d", GetFPS());
            DrawText(buf, 30, y, 20, {100, 255, 100, 255}); y += 30;

            snprintf(buf, sizeof(buf), "Frame Time: %.3f ms", m_frameTime * 1000);
            DrawText(buf, 30, y, 16, WHITE); y += 24;

            float avg = 0;
            for (float ft : m_frameTimes) avg += ft;
            if (!m_frameTimes.empty()) avg /= m_frameTimes.size();
            snprintf(buf, sizeof(buf), "Avg (120 frames): %.3f ms", avg * 1000);
            DrawText(buf, 30, y, 16, WHITE); y += 24;

            float minFt = 999, maxFt = 0;
            for (float ft : m_frameTimes) {
                minFt = std::min(minFt, ft);
                maxFt = std::max(maxFt, ft);
            }
            snprintf(buf, sizeof(buf), "Min: %.2f ms  Max: %.2f ms", minFt * 1000, maxFt * 1000);
            DrawText(buf, 30, y, 14, {150, 150, 150, 255}); y += 30;

            drawFrameGraph(y);
        }

    private:
        float m_frameTime = 0.0f;
        std::vector<float> m_frameTimes;

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
