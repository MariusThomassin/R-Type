/*
** R-Type ECS - Modes Debug Tab
** Controls for Showoff and Stress Test modes using UI widget library
*/

#pragma once

#include "DebugTab.hpp"
#include "src/engine/ui/Widgets.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/events/InputEvents.hpp"

#include <string>
#include <algorithm>
#include <memory>

namespace rtype::ecs::debug {

    class ModesTab : public IDebugTab {
    public:
        ModesTab(EventBus& eventBus) : m_eventBus(eventBus) {
            initWidgets();
        }

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
            
            // Update dynamic text
            if (m_showoffActive) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Pattern: %s", m_showoffPatternName.c_str());
                m_showoffPatternText->setText(buf);
                snprintf(buf, sizeof(buf), "Phase: %d / %d", m_showoffCurrentPhase + 1, m_showoffTotalPhases);
                m_showoffPhaseText->setText(buf);
                m_showoffProgressBar->setValue(m_showoffProgress);
            }

            if (m_stressTestActive) {
                char buf[64];
                snprintf(buf, sizeof(buf), "Intensity: %d / 5", m_stressTestIntensity);
                m_stressIntensityText->setText(buf);
                m_stressProgressBar->setValue(m_stressTestProgress);
                
                float overallProgress = ((m_stressTestIntensity - 1) + m_stressTestProgress) / 5.0f;
                snprintf(buf, sizeof(buf), "Overall: %.0f%%", overallProgress * 100);
                m_stressOverallText->setText(buf);
            }

            if (m_stressTestComplete && !m_stressTestActive) {
                char buf[256];
                snprintf(buf, sizeof(buf), "Report: %s", m_stressTestReportFile.c_str());
                m_stressReportText->setText(buf);
            }
        }

        void draw(int y) override {
            const int x = 30;
            const int panelWidth = m_screenWidth - 50;

            // Title
            m_titleText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_titleText->renderSelf();
            y += 40;

            // =============== SHOWOFF MODE SECTION ===============
            m_showoffPanel->setPosition(static_cast<float>(x - 5), static_cast<float>(y - 5));
            m_showoffPanel->setSize(static_cast<float>(panelWidth), 140.0f);
            m_showoffPanel->renderSelf();

            m_showoffTitleText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
            m_showoffTitleText->renderSelf();
            y += 28;

            m_showoffDescText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
            m_showoffDescText->renderSelf();
            y += 20;

            if (m_showoffActive) {
                m_showoffStatusText->setText("Status: ACTIVE");
                m_showoffStatusText->setTextColor(ui::Color(100, 255, 100, 255));
                m_showoffStatusText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_showoffStatusText->renderSelf();
                y += 22;

                m_showoffPatternText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_showoffPatternText->renderSelf();
                y += 20;

                m_showoffPhaseText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_showoffPhaseText->renderSelf();
                y += 20;

                m_showoffProgressBar->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_showoffProgressBar->renderSelf();
                y += 24;

                // Stop button
                m_stopShowoffBtn->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_stopShowoffBtn->renderSelf();
                handleButtonEvents(m_stopShowoffBtn, m_mouse, m_wasOverStopShowoff);
                if (m_wasOverStopShowoff && m_mouse.leftPressed) {
                    m_eventBus.emit(events::ShowoffEndEvent{});
                }
            } else {
                bool canStart = !m_stressTestActive;
                if (canStart) {
                    m_showoffStatusText->setText("Status: Ready");
                    m_showoffStatusText->setTextColor(ui::Color(150, 150, 150, 255));
                } else {
                    m_showoffStatusText->setText("Status: Blocked (Stress Test active)");
                    m_showoffStatusText->setTextColor(ui::Color(255, 100, 100, 255));
                }
                m_showoffStatusText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_showoffStatusText->renderSelf();
                y += 44;

                // Start button
                m_startShowoffBtn->setEnabled(canStart);
                m_startShowoffBtn->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_startShowoffBtn->renderSelf();
                handleButtonEvents(m_startShowoffBtn, m_mouse, m_wasOverStartShowoff);
                if (canStart && m_wasOverStartShowoff && m_mouse.leftPressed) {
                    m_eventBus.emit(events::ShowoffStartEvent{});
                }
            }
            y += 50;

            // =============== STRESS TEST MODE SECTION ===============
            m_stressPanel->setPosition(static_cast<float>(x - 5), static_cast<float>(y - 5));
            m_stressPanel->setSize(static_cast<float>(panelWidth), 180.0f);
            m_stressPanel->renderSelf();

            m_stressTitleText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
            m_stressTitleText->renderSelf();
            y += 28;

            m_stressDescText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
            m_stressDescText->renderSelf();
            y += 14;
            m_stressDesc2Text->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
            m_stressDesc2Text->renderSelf();
            y += 22;

            if (m_stressTestComplete && !m_stressTestActive) {
                m_stressStatusText->setText("Status: COMPLETE");
                m_stressStatusText->setTextColor(ui::Color(100, 255, 100, 255));
                m_stressStatusText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_stressStatusText->renderSelf();
                y += 22;

                m_stressReportText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_stressReportText->renderSelf();
                y += 24;

                // Run Again button
                m_runAgainBtn->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_runAgainBtn->renderSelf();
                handleButtonEvents(m_runAgainBtn, m_mouse, m_wasOverRunAgain);
                if (m_wasOverRunAgain && m_mouse.leftPressed) {
                    m_eventBus.emit(events::StressTestToggleEvent{});
                }

            } else if (m_stressTestActive) {
                m_stressStatusText->setText("Status: RUNNING");
                m_stressStatusText->setTextColor(ui::Color(255, 200, 100, 255));
                m_stressStatusText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_stressStatusText->renderSelf();
                y += 22;

                m_stressIntensityText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_stressIntensityText->renderSelf();
                y += 20;

                m_stressProgressBar->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_stressProgressBar->renderSelf();

                m_stressOverallText->setPosition(static_cast<float>(x + 215), static_cast<float>(y));
                m_stressOverallText->renderSelf();
                y += 24;

                // Stop button
                m_stopStressBtn->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_stopStressBtn->renderSelf();
                handleButtonEvents(m_stopStressBtn, m_mouse, m_wasOverStopStress);
                if (m_wasOverStopStress && m_mouse.leftPressed) {
                    m_eventBus.emit(events::StressTestToggleEvent{});
                }
            } else {
                bool canStart = !m_showoffActive;
                if (canStart) {
                    m_stressStatusText->setText("Status: Ready");
                    m_stressStatusText->setTextColor(ui::Color(150, 150, 150, 255));
                } else {
                    m_stressStatusText->setText("Status: Blocked (Showoff active)");
                    m_stressStatusText->setTextColor(ui::Color(255, 100, 100, 255));
                }
                m_stressStatusText->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_stressStatusText->renderSelf();
                y += 44;

                // Start button
                m_startStressBtn->setEnabled(canStart);
                m_startStressBtn->setPosition(static_cast<float>(x + 5), static_cast<float>(y));
                m_startStressBtn->renderSelf();
                handleButtonEvents(m_startStressBtn, m_mouse, m_wasOverStartStress);
                if (canStart && m_wasOverStartStress && m_mouse.leftPressed) {
                    m_eventBus.emit(events::StressTestToggleEvent{});
                }
            }
            y += 50;

            // =============== INFO SECTION ===============
            m_infoPanel->setPosition(static_cast<float>(x - 5), static_cast<float>(y - 5));
            m_infoPanel->setSize(static_cast<float>(panelWidth), 50.0f);
            m_infoPanel->renderSelf();

            m_infoText1->setPosition(static_cast<float>(x + 5), static_cast<float>(y + 5));
            m_infoText1->renderSelf();
            m_infoText2->setPosition(static_cast<float>(x + 5), static_cast<float>(y + 22));
            m_infoText2->renderSelf();
        }

        void handleMouse(const MouseInput& mouse) override {
            m_mouse = mouse;
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

        // Button hover tracking
        bool m_wasOverStartShowoff = false;
        bool m_wasOverStopShowoff = false;
        bool m_wasOverStartStress = false;
        bool m_wasOverStopStress = false;
        bool m_wasOverRunAgain = false;

        // Widgets - Title
        std::shared_ptr<ui::TextWidget> m_titleText;

        // Widgets - Showoff section
        std::shared_ptr<ui::PanelWidget> m_showoffPanel;
        std::shared_ptr<ui::TextWidget> m_showoffTitleText;
        std::shared_ptr<ui::TextWidget> m_showoffDescText;
        std::shared_ptr<ui::TextWidget> m_showoffStatusText;
        std::shared_ptr<ui::TextWidget> m_showoffPatternText;
        std::shared_ptr<ui::TextWidget> m_showoffPhaseText;
        std::shared_ptr<ui::ProgressBarWidget> m_showoffProgressBar;
        std::shared_ptr<ui::ButtonWidget> m_startShowoffBtn;
        std::shared_ptr<ui::ButtonWidget> m_stopShowoffBtn;

        // Widgets - Stress test section
        std::shared_ptr<ui::PanelWidget> m_stressPanel;
        std::shared_ptr<ui::TextWidget> m_stressTitleText;
        std::shared_ptr<ui::TextWidget> m_stressDescText;
        std::shared_ptr<ui::TextWidget> m_stressDesc2Text;
        std::shared_ptr<ui::TextWidget> m_stressStatusText;
        std::shared_ptr<ui::TextWidget> m_stressIntensityText;
        std::shared_ptr<ui::TextWidget> m_stressOverallText;
        std::shared_ptr<ui::TextWidget> m_stressReportText;
        std::shared_ptr<ui::ProgressBarWidget> m_stressProgressBar;
        std::shared_ptr<ui::ButtonWidget> m_startStressBtn;
        std::shared_ptr<ui::ButtonWidget> m_stopStressBtn;
        std::shared_ptr<ui::ButtonWidget> m_runAgainBtn;

        // Widgets - Info section
        std::shared_ptr<ui::PanelWidget> m_infoPanel;
        std::shared_ptr<ui::TextWidget> m_infoText1;
        std::shared_ptr<ui::TextWidget> m_infoText2;

        void handleButtonEvents(std::shared_ptr<ui::ButtonWidget>& button, 
                               const MouseInput& mouse, bool& wasOver) {
            if (!button || !button->isEnabled()) {
                wasOver = false;
                return;
            }
            
            auto transform = button->getAbsoluteTransform();
            bool over = mouse.x >= transform.x && mouse.x < transform.x + transform.width &&
                       mouse.y >= transform.y && mouse.y < transform.y + transform.height;
            
            if (over && !wasOver) button->onMouseEnter();
            else if (!over && wasOver) button->onMouseLeave();
            
            wasOver = over;
        }

        void initWidgets() {
            // Title
            m_titleText = std::make_shared<ui::TextWidget>("Demo & Test Modes", 20);
            m_titleText->setTextColor(ui::Color::White());
            m_titleText->setBackgroundColor(ui::Color::Transparent());

            // Showoff section
            m_showoffPanel = std::make_shared<ui::PanelWidget>();
            m_showoffPanel->setBackgroundColor(ui::Color(40, 40, 60, 255));
            m_showoffPanel->setBorderWidth(0);

            m_showoffTitleText = std::make_shared<ui::TextWidget>("SHOWOFF MODE", 18);
            m_showoffTitleText->setTextColor(ui::Color(255, 200, 100, 255));
            m_showoffTitleText->setBackgroundColor(ui::Color::Transparent());

            m_showoffDescText = std::make_shared<ui::TextWidget>("Cycles through all bullet patterns and trajectories.", 12);
            m_showoffDescText->setTextColor(ui::Color(150, 150, 150, 255));
            m_showoffDescText->setBackgroundColor(ui::Color::Transparent());

            m_showoffStatusText = std::make_shared<ui::TextWidget>("Status: Ready", 14);
            m_showoffStatusText->setTextColor(ui::Color(150, 150, 150, 255));
            m_showoffStatusText->setBackgroundColor(ui::Color::Transparent());

            m_showoffPatternText = std::make_shared<ui::TextWidget>("Pattern: ", 14);
            m_showoffPatternText->setTextColor(ui::Color::White());
            m_showoffPatternText->setBackgroundColor(ui::Color::Transparent());

            m_showoffPhaseText = std::make_shared<ui::TextWidget>("Phase: 0 / 0", 14);
            m_showoffPhaseText->setTextColor(ui::Color::White());
            m_showoffPhaseText->setBackgroundColor(ui::Color::Transparent());

            m_showoffProgressBar = std::make_shared<ui::ProgressBarWidget>(0.0f);
            m_showoffProgressBar->setSize(200, 16);
            m_showoffProgressBar->setFillColor(ui::Color(255, 200, 100, 255));
            m_showoffProgressBar->setBackgroundColor(ui::Color(60, 60, 80, 255));

            m_startShowoffBtn = std::make_shared<ui::ButtonWidget>("Start Showoff");
            m_startShowoffBtn->setSize(120, 28);
            m_startShowoffBtn->setStateColor(ui::ButtonState::Normal, ui::Color(80, 150, 80, 255));
            m_startShowoffBtn->setStateColor(ui::ButtonState::Hovered, ui::Color(100, 180, 100, 255));

            m_stopShowoffBtn = std::make_shared<ui::ButtonWidget>("Stop Showoff");
            m_stopShowoffBtn->setSize(120, 28);
            m_stopShowoffBtn->setStateColor(ui::ButtonState::Normal, ui::Color(180, 80, 80, 255));
            m_stopShowoffBtn->setStateColor(ui::ButtonState::Hovered, ui::Color(220, 100, 100, 255));

            // Stress test section
            m_stressPanel = std::make_shared<ui::PanelWidget>();
            m_stressPanel->setBackgroundColor(ui::Color(40, 40, 60, 255));
            m_stressPanel->setBorderWidth(0);

            m_stressTitleText = std::make_shared<ui::TextWidget>("STRESS TEST MODE", 18);
            m_stressTitleText->setTextColor(ui::Color(100, 200, 255, 255));
            m_stressTitleText->setBackgroundColor(ui::Color::Transparent());

            m_stressDescText = std::make_shared<ui::TextWidget>("Spawns increasing bullet loads to test ECS performance.", 12);
            m_stressDescText->setTextColor(ui::Color(150, 150, 150, 255));
            m_stressDescText->setBackgroundColor(ui::Color::Transparent());

            m_stressDesc2Text = std::make_shared<ui::TextWidget>("5 intensity levels, 20 seconds each. Generates performance report.", 12);
            m_stressDesc2Text->setTextColor(ui::Color(150, 150, 150, 255));
            m_stressDesc2Text->setBackgroundColor(ui::Color::Transparent());

            m_stressStatusText = std::make_shared<ui::TextWidget>("Status: Ready", 14);
            m_stressStatusText->setTextColor(ui::Color(150, 150, 150, 255));
            m_stressStatusText->setBackgroundColor(ui::Color::Transparent());

            m_stressIntensityText = std::make_shared<ui::TextWidget>("Intensity: 0 / 5", 14);
            m_stressIntensityText->setTextColor(ui::Color::White());
            m_stressIntensityText->setBackgroundColor(ui::Color::Transparent());

            m_stressOverallText = std::make_shared<ui::TextWidget>("Overall: 0%", 12);
            m_stressOverallText->setTextColor(ui::Color(150, 150, 150, 255));
            m_stressOverallText->setBackgroundColor(ui::Color::Transparent());

            m_stressReportText = std::make_shared<ui::TextWidget>("Report: ", 12);
            m_stressReportText->setTextColor(ui::Color(200, 200, 200, 255));
            m_stressReportText->setBackgroundColor(ui::Color::Transparent());

            m_stressProgressBar = std::make_shared<ui::ProgressBarWidget>(0.0f);
            m_stressProgressBar->setSize(200, 16);
            m_stressProgressBar->setFillColor(ui::Color(100, 200, 255, 255));
            m_stressProgressBar->setBackgroundColor(ui::Color(60, 60, 80, 255));

            m_startStressBtn = std::make_shared<ui::ButtonWidget>("Start Stress Test");
            m_startStressBtn->setSize(140, 28);
            m_startStressBtn->setStateColor(ui::ButtonState::Normal, ui::Color(80, 150, 80, 255));
            m_startStressBtn->setStateColor(ui::ButtonState::Hovered, ui::Color(100, 180, 100, 255));

            m_stopStressBtn = std::make_shared<ui::ButtonWidget>("Stop Test");
            m_stopStressBtn->setSize(120, 28);
            m_stopStressBtn->setStateColor(ui::ButtonState::Normal, ui::Color(180, 80, 80, 255));
            m_stopStressBtn->setStateColor(ui::ButtonState::Hovered, ui::Color(220, 100, 100, 255));

            m_runAgainBtn = std::make_shared<ui::ButtonWidget>("Run Again");
            m_runAgainBtn->setSize(140, 28);
            m_runAgainBtn->setStateColor(ui::ButtonState::Normal, ui::Color(80, 150, 80, 255));
            m_runAgainBtn->setStateColor(ui::ButtonState::Hovered, ui::Color(100, 180, 100, 255));

            // Info section
            m_infoPanel = std::make_shared<ui::PanelWidget>();
            m_infoPanel->setBackgroundColor(ui::Color(30, 30, 45, 255));
            m_infoPanel->setBorderWidth(0);

            m_infoText1 = std::make_shared<ui::TextWidget>("Use the buttons above to start/stop demo and test modes.", 12);
            m_infoText1->setTextColor(ui::Color(100, 100, 120, 255));
            m_infoText1->setBackgroundColor(ui::Color::Transparent());

            m_infoText2 = std::make_shared<ui::TextWidget>("Press [O] to toggle Debug Mode at any time.", 12);
            m_infoText2->setTextColor(ui::Color(100, 100, 120, 255));
            m_infoText2->setBackgroundColor(ui::Color::Transparent());
        }
    };

} // namespace rtype::ecs::debug
