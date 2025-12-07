/*
** R-Type ECS - Modes Debug Tab
** Controls for Showoff and Stress Test modes
*/

#pragma once

#include "DebugTab.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/events/InputEvents.hpp"

#include <string>
#include <algorithm>

namespace rtype::ecs::debug {

    class ModesTab : public IDebugTab {
    public:
        ModesTab(EventBus& eventBus) : m_eventBus(eventBus) {}

        const char* getName() const override { return "Modes"; }

        void setShowoffState(bool active, const std::string& patternName = "", 
                            int currentPhase = 0, int totalPhases = 0, float progress = 0.0f) {
            m_showoffActive = active;
            m_showoffPatternName = patternName;
            m_showoffCurrentPhase = currentPhase;
            m_showoffTotalPhases = totalPhases;
            m_showoffProgress = progress;
        }

        void setStressTestState(bool active, bool complete, int intensity = 0, 
                               float progress = 0.0f, const std::string& reportFile = "") {
            m_stressTestActive = active;
            m_stressTestComplete = complete;
            m_stressTestIntensity = intensity;
            m_stressTestProgress = progress;
            m_stressTestReportFile = reportFile;
        }

        void update(float dt) override {
            (void)dt;
        }

        void draw(int y) override {
            DrawText("Demo & Test Modes", 30, y, 20, WHITE);
            y += 40;

            // =============== SHOWOFF MODE SECTION ===============
            DrawRectangle(25, y - 5, m_screenWidth - 50, 140, {40, 40, 60, 255});
            DrawText("SHOWOFF MODE", 35, y, 18, {255, 200, 100, 255});
            y += 28;

            DrawText("Cycles through all bullet patterns and trajectories.", 35, y, 12, {150, 150, 150, 255});
            y += 20;

            m_showoffButtonY = y + 36;  // Store for hit testing

            if (m_showoffActive) {
                DrawText("Status: ACTIVE", 35, y, 14, {100, 255, 100, 255});
                y += 22;

                char buf[128];
                snprintf(buf, sizeof(buf), "Pattern: %s", m_showoffPatternName.c_str());
                DrawText(buf, 35, y, 14, WHITE);
                y += 20;

                snprintf(buf, sizeof(buf), "Phase: %d / %d", m_showoffCurrentPhase + 1, m_showoffTotalPhases);
                DrawText(buf, 35, y, 14, WHITE);
                y += 20;

                int barX = 35, barY = y, barW = 200, barH = 16;
                DrawRectangle(barX, barY, barW, barH, {60, 60, 80, 255});
                DrawRectangle(barX, barY, (int)(barW * m_showoffProgress), barH, {255, 200, 100, 255});
                DrawRectangleLines(barX, barY, barW, barH, {100, 100, 120, 255});
                y += 24;

                m_showoffButtonY = y;
                m_stopShowoffHover = isMouseOverButton(35, y, 120, 28);
                drawButton(35, y, 120, 28, "Stop Showoff", {180, 80, 80, 255}, m_stopShowoffHover);
                if (m_stopShowoffHover && m_mouse.leftPressed) {
                    m_eventBus.emit(events::ShowoffEndEvent{});
                }
            } else {
                bool canStart = !m_stressTestActive;
                Color statusColor = canStart ? Color{150, 150, 150, 255} : Color{255, 100, 100, 255};
                const char* statusText = canStart ? "Status: Ready" : "Status: Blocked (Stress Test active)";
                DrawText(statusText, 35, y, 14, statusColor);
                y += 44;

                m_showoffButtonY = y;
                m_startShowoffHover = isMouseOverButton(35, y, 120, 28);
                Color btnColor = canStart ? Color{80, 150, 80, 255} : Color{80, 80, 80, 255};
                drawButton(35, y, 120, 28, "Start Showoff", btnColor, m_startShowoffHover && canStart);
                if (m_startShowoffHover && m_mouse.leftPressed && canStart) {
                    m_eventBus.emit(events::ShowoffStartEvent{});
                }
            }
            y += 50;

            // =============== STRESS TEST MODE SECTION ===============
            m_stressTestSectionY = y;
            DrawRectangle(25, y - 5, m_screenWidth - 50, 180, {40, 40, 60, 255});
            DrawText("STRESS TEST MODE", 35, y, 18, {100, 200, 255, 255});
            y += 28;

            DrawText("Spawns increasing bullet loads to test ECS performance.", 35, y, 12, {150, 150, 150, 255});
            DrawText("5 intensity levels, 20 seconds each. Generates performance report.", 35, y + 14, 12, {150, 150, 150, 255});
            y += 36;

            m_stressTestButtonY = y + 36;  // Store for hit testing

            if (m_stressTestComplete && !m_stressTestActive) {
                DrawText("Status: COMPLETE", 35, y, 14, {100, 255, 100, 255});
                y += 22;

                char buf[256];
                snprintf(buf, sizeof(buf), "Report: %s", m_stressTestReportFile.c_str());
                DrawText(buf, 35, y, 12, {200, 200, 200, 255});
                y += 24;

                m_stressTestButtonY = y;
                m_startStressTestHover = isMouseOverButton(35, y, 140, 28);
                drawButton(35, y, 140, 28, "Run Again", {80, 150, 80, 255}, m_startStressTestHover);
                if (m_startStressTestHover && isButtonJustClicked()) {
                    m_eventBus.emit(events::StressTestToggleEvent{});
                }

            } else if (m_stressTestActive) {
                DrawText("Status: RUNNING", 35, y, 14, {255, 200, 100, 255});
                y += 22;

                char buf[64];
                snprintf(buf, sizeof(buf), "Intensity: %d / 5", m_stressTestIntensity);
                DrawText(buf, 35, y, 14, WHITE);
                y += 20;

                int barX = 35, barY = y, barW = 200, barH = 16;
                DrawRectangle(barX, barY, barW, barH, {60, 60, 80, 255});
                DrawRectangle(barX, barY, (int)(barW * m_stressTestProgress), barH, {100, 200, 255, 255});
                DrawRectangleLines(barX, barY, barW, barH, {100, 100, 120, 255});
                y += 24;

                float overallProgress = ((m_stressTestIntensity - 1) + m_stressTestProgress) / 5.0f;
                snprintf(buf, sizeof(buf), "Overall: %.0f%%", overallProgress * 100);
                DrawText(buf, 245, y - 22, 12, {150, 150, 150, 255});

                m_stressTestButtonY = y;
                m_stopStressTestHover = isMouseOverButton(35, y, 120, 28);
                drawButton(35, y, 120, 28, "Stop Test", {180, 80, 80, 255}, m_stopStressTestHover);
                if (m_stopStressTestHover && isButtonJustClicked()) {
                    m_eventBus.emit(events::StressTestToggleEvent{});
                }
            } else {
                bool canStart = !m_showoffActive;
                Color statusColor = canStart ? Color{150, 150, 150, 255} : Color{255, 100, 100, 255};
                const char* statusText = canStart ? "Status: Ready" : "Status: Blocked (Showoff active)";
                DrawText(statusText, 35, y, 14, statusColor);
                y += 44;

                m_stressTestButtonY = y;
                m_startStressTestHover = isMouseOverButton(35, y, 140, 28);
                Color btnColor = canStart ? Color{80, 150, 80, 255} : Color{80, 80, 80, 255};
                drawButton(35, y, 140, 28, "Start Stress Test", btnColor, m_startStressTestHover && canStart);
                if (m_startStressTestHover && isButtonJustClicked() && canStart) {
                    m_eventBus.emit(events::StressTestToggleEvent{});
                }
            }
            y += 50;

            // =============== INFO ===============
            DrawRectangle(25, y - 5, m_screenWidth - 50, 50, {30, 30, 45, 255});
            DrawText("Use the buttons above to start/stop demo and test modes.", 
                     35, y + 5, 12, {100, 100, 120, 255});
            DrawText("Press [O] to toggle Debug Mode at any time.", 35, y + 22, 12, {100, 100, 120, 255});
        }

        void handleMouse(const MouseInput& mouse) override {
            m_mouse = mouse;
            // Hover states are now computed during draw() when Y positions are known
        }

        bool isButtonJustClicked() {
            bool justClicked = m_mouse.leftPressed && !m_lastMousePressed;
            m_lastMousePressed = m_mouse.leftPressed;
            return justClicked;
        }

    private:
        EventBus& m_eventBus;
        MouseInput m_mouse;

        // Showoff state
        bool m_showoffActive = false;
        std::string m_showoffPatternName;
        int m_showoffCurrentPhase = 0;
        int m_showoffTotalPhases = 0;
        float m_showoffProgress = 0.0f;

        // Stress test state
        bool m_stressTestActive = false;
        bool m_stressTestComplete = false;
        int m_stressTestIntensity = 0;
        float m_stressTestProgress = 0.0f;
        std::string m_stressTestReportFile;

        // Click state tracking
        bool m_lastMousePressed = false;

        // Button positions (stored during draw for hit testing)
        int m_showoffButtonY = 0;
        int m_stressTestButtonY = 0;
        int m_stressTestSectionY = 0;

        // Button hover states
        bool m_startShowoffHover = false;
        bool m_stopShowoffHover = false;
        bool m_startStressTestHover = false;
        bool m_stopStressTestHover = false;

        bool isMouseOverButton(int x, int y, int w, int h) const {
            return m_mouse.x >= x && m_mouse.x < x + w &&
                   m_mouse.y >= y && m_mouse.y < y + h;
        }

        void drawButton(int x, int y, int w, int h, const char* text, Color baseColor, bool hover) {
            Color bg = hover ? Color{
                (unsigned char)std::min(255, baseColor.r + 40),
                (unsigned char)std::min(255, baseColor.g + 40),
                (unsigned char)std::min(255, baseColor.b + 40),
                255
            } : baseColor;
            
            DrawRectangle(x, y, w, h, bg);
            DrawRectangleLines(x, y, w, h, {200, 200, 200, 255});
            
            int textW = MeasureText(text, 14);
            DrawText(text, x + (w - textW) / 2, y + 7, 14, WHITE);
        }
    };

} // namespace rtype::ecs::debug
