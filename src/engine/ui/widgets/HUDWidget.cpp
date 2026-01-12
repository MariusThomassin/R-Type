/*
** R-Type - HUDWidget Implementation
** Heads-Up Display for in-game information
*/

#include "HUDWidget.hpp"
#include <raylib.h>

namespace rtype::ui {

    HUDWidget::HUDWidget() {
        // HUD typically spans the screen and is transparent
        UIStyle style;
        style.backgroundColor = UIColor(0, 0, 0, 0);      // Transparent background
        style.borderColor = UIColor(0, 0, 0, 0);          // No border
        style.textColor = UIColor(255, 255, 255, 255);    // White text
        style.fontSize = SCORE_FONT_SIZE;
        style.borderWidth = 0.0f;
        style.padding = 0.0f;
        setStyle(style);
    }

    void HUDWidget::setScore(int score) {
        m_score = score;
    }

    void HUDWidget::setHighScore(int highScore) {
        m_highScore = highScore;
    }

    void HUDWidget::setLives(int lives) {
        m_lives = lives;
    }

    void HUDWidget::setWeaponLevel(int level) {
        m_weaponLevel = std::min(level, MAX_WEAPON_LEVEL);
    }

    void HUDWidget::setForceOrbActive(bool active) {
        m_forceOrbActive = active;
    }

    void HUDWidget::setForceOrbLevel(int level) {
        m_forceOrbLevel = level;
    }

    void HUDWidget::setBombs(int count) {
        m_bombs = count;
    }

    void HUDWidget::setCurrentWave(int wave) {
        m_currentWave = wave;
    }

    void HUDWidget::setTotalWaves(int total) {
        m_totalWaves = total;
    }

    void HUDWidget::setLevelName(const std::string& name) {
        m_levelName = name;
    }

    void HUDWidget::setCombo(int combo) {
        m_combo = combo;
    }

    void HUDWidget::setComboMultiplier(float multiplier) {
        m_comboMultiplier = multiplier;
    }

    void HUDWidget::setBossActive(bool active) {
        m_bossActive = active;
    }

    void HUDWidget::setBossHealth(float healthPercent) {
        m_bossHealth = healthPercent;
    }

    void HUDWidget::setBossName(const std::string& name) {
        m_bossName = name;
    }

    std::string HUDWidget::formatScore(int score) const {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(8) << score;
        return ss.str();
    }

    void HUDWidget::render() const {
        if (!isVisible()) return;

        // Render each HUD element
        renderScore();
        renderLives();
        renderWeaponLevel();
        
        if (m_forceOrbActive) {
            renderForceOrb();
        }
        
        if (m_bombs > 0) {
            renderBombs();
        }
        
        renderWaveInfo();
        
        if (m_combo > 1) {
            renderCombo();
        }
        
        if (m_bossActive) {
            renderBossHealth();
        }

        // Render children
        Widget::render();
    }

    void HUDWidget::renderScore() const {
        auto transform = getAbsoluteTransform();
        float x = transform.x + MARGIN;
        float y = transform.y + MARGIN;

        // "SCORE" label
        DrawText("SCORE", static_cast<int>(x), static_cast<int>(y), LABEL_FONT_SIZE, LIGHTGRAY);

        // Score value with leading zeros
        std::string scoreStr = formatScore(m_score);
        DrawText(scoreStr.c_str(), static_cast<int>(x), static_cast<int>(y + LABEL_FONT_SIZE + 2), SCORE_FONT_SIZE, WHITE);

        // High score on the right
        float rightX = transform.x + transform.width - MARGIN;
        DrawText("HI-SCORE", static_cast<int>(rightX - 150), static_cast<int>(y), LABEL_FONT_SIZE, LIGHTGRAY);
        
        std::string hiScoreStr = formatScore(m_highScore);
        DrawText(hiScoreStr.c_str(), static_cast<int>(rightX - 150), static_cast<int>(y + LABEL_FONT_SIZE + 2), SCORE_FONT_SIZE, GOLD);
    }

    void HUDWidget::renderLives() const {
        auto transform = getAbsoluteTransform();
        float x = transform.x + MARGIN;
        float y = transform.y + transform.height - MARGIN - 30;

        // Draw heart icons for lives
        DrawText("LIVES", static_cast<int>(x), static_cast<int>(y), SMALL_FONT_SIZE, LIGHTGRAY);
        
        float heartX = x + 50;
        for (int i = 0; i < m_maxLives; i++) {
            Color heartColor = (i < m_lives) ? RED : DARKGRAY;
            // Draw a simple heart shape using circles
            DrawCircle(static_cast<int>(heartX + i * 25), static_cast<int>(y + 8), 6, heartColor);
            DrawCircle(static_cast<int>(heartX + i * 25 + 8), static_cast<int>(y + 8), 6, heartColor);
            DrawTriangle(
                {heartX + i * 25 - 6, y + 10},
                {heartX + i * 25 + 14, y + 10},
                {heartX + i * 25 + 4, y + 22},
                heartColor
            );
        }
    }

    void HUDWidget::renderWeaponLevel() const {
        auto transform = getAbsoluteTransform();
        float x = transform.x + 250;
        float y = transform.y + transform.height - MARGIN - 30;

        DrawText("POWER", static_cast<int>(x), static_cast<int>(y), SMALL_FONT_SIZE, LIGHTGRAY);

        // Draw power level bars
        float barX = x + 50;
        float barWidth = 20;
        float barHeight = 15;
        float barSpacing = 4;

        for (int i = 0; i <= MAX_WEAPON_LEVEL; i++) {
            Color barColor;
            if (i <= m_weaponLevel) {
                // Color gradient from cyan to magenta as power increases
                switch (i) {
                    case 0: barColor = {80, 200, 255, 255}; break;   // Cyan
                    case 1: barColor = {80, 255, 80, 255}; break;    // Green
                    case 2: barColor = {255, 255, 80, 255}; break;   // Yellow
                    case 3: barColor = {255, 150, 50, 255}; break;   // Orange
                    case 4: barColor = {255, 80, 255, 255}; break;   // Magenta
                    default: barColor = WHITE; break;
                }
            } else {
                barColor = DARKGRAY;
            }

            DrawRectangle(
                static_cast<int>(barX + i * (barWidth + barSpacing)),
                static_cast<int>(y + 2),
                static_cast<int>(barWidth),
                static_cast<int>(barHeight),
                barColor
            );
            DrawRectangleLines(
                static_cast<int>(barX + i * (barWidth + barSpacing)),
                static_cast<int>(y + 2),
                static_cast<int>(barWidth),
                static_cast<int>(barHeight),
                WHITE
            );
        }
    }

    void HUDWidget::renderForceOrb() const {
        auto transform = getAbsoluteTransform();
        float x = transform.x + 420;
        float y = transform.y + transform.height - MARGIN - 30;

        DrawText("ORB", static_cast<int>(x), static_cast<int>(y), SMALL_FONT_SIZE, LIGHTGRAY);

        // Draw orb indicator
        Color orbColor;
        switch (m_forceOrbLevel) {
            case 1: orbColor = {100, 150, 255, 255}; break;  // Blue
            case 2: orbColor = {255, 200, 100, 255}; break;  // Orange
            case 3: orbColor = {255, 100, 100, 255}; break;  // Red
            default: orbColor = {100, 150, 255, 255}; break;
        }

        DrawCircle(static_cast<int>(x + 50), static_cast<int>(y + 10), 12, orbColor);
        DrawCircleLines(static_cast<int>(x + 50), static_cast<int>(y + 10), 14, WHITE);

        // Level indicator
        char levelStr[8];
        snprintf(levelStr, sizeof(levelStr), "Lv%d", m_forceOrbLevel);
        DrawText(levelStr, static_cast<int>(x + 70), static_cast<int>(y + 3), SMALL_FONT_SIZE, orbColor);
    }

    void HUDWidget::renderBombs() const {
        auto transform = getAbsoluteTransform();
        float x = transform.x + 530;
        float y = transform.y + transform.height - MARGIN - 30;

        DrawText("BOMB", static_cast<int>(x), static_cast<int>(y), SMALL_FONT_SIZE, LIGHTGRAY);

        // Draw bomb icons
        float bombX = x + 45;
        for (int i = 0; i < m_bombs && i < 5; i++) {
            DrawCircle(static_cast<int>(bombX + i * 22), static_cast<int>(y + 10), 8, {255, 100, 50, 255});
            // Fuse
            DrawLine(
                static_cast<int>(bombX + i * 22),
                static_cast<int>(y + 2),
                static_cast<int>(bombX + i * 22 + 5),
                static_cast<int>(y - 3),
                ORANGE
            );
        }
    }

    void HUDWidget::renderWaveInfo() const {
        auto transform = getAbsoluteTransform();
        float centerX = transform.x + transform.width / 2;
        float y = transform.y + MARGIN;

        // Level name (centered)
        if (!m_levelName.empty()) {
            int nameWidth = MeasureText(m_levelName.c_str(), LABEL_FONT_SIZE);
            DrawText(m_levelName.c_str(), static_cast<int>(centerX - nameWidth / 2), static_cast<int>(y), LABEL_FONT_SIZE, SKYBLUE);
        }

        // Wave counter
        char waveStr[32];
        snprintf(waveStr, sizeof(waveStr), "WAVE %d/%d", m_currentWave, m_totalWaves);
        int waveWidth = MeasureText(waveStr, SMALL_FONT_SIZE);
        DrawText(waveStr, static_cast<int>(centerX - waveWidth / 2), static_cast<int>(y + LABEL_FONT_SIZE + 4), SMALL_FONT_SIZE, WHITE);
    }

    void HUDWidget::renderCombo() const {
        auto transform = getAbsoluteTransform();
        float rightX = transform.x + transform.width - MARGIN - 100;
        float y = transform.y + 80;

        // Combo counter with pulsing effect
        char comboStr[32];
        snprintf(comboStr, sizeof(comboStr), "%d HIT", m_combo);
        
        // Color based on combo size
        Color comboColor;
        if (m_combo >= 50) comboColor = {255, 100, 255, 255};      // Magenta
        else if (m_combo >= 25) comboColor = {255, 200, 50, 255};  // Gold
        else if (m_combo >= 10) comboColor = {255, 150, 50, 255};  // Orange
        else comboColor = {255, 255, 100, 255};                     // Yellow

        DrawText(comboStr, static_cast<int>(rightX), static_cast<int>(y), LABEL_FONT_SIZE + 4, comboColor);

        // Multiplier
        if (m_comboMultiplier > 1.0f) {
            char multStr[16];
            snprintf(multStr, sizeof(multStr), "x%.1f", m_comboMultiplier);
            DrawText(multStr, static_cast<int>(rightX + 10), static_cast<int>(y + LABEL_FONT_SIZE + 6), SMALL_FONT_SIZE, comboColor);
        }
    }

    void HUDWidget::renderBossHealth() const {
        auto transform = getAbsoluteTransform();
        float centerX = transform.x + transform.width / 2;
        float y = transform.y + 60;
        float barWidth = 400;
        float barHeight = 16;

        // Boss name
        if (!m_bossName.empty()) {
            int nameWidth = MeasureText(m_bossName.c_str(), LABEL_FONT_SIZE);
            DrawText(m_bossName.c_str(), static_cast<int>(centerX - nameWidth / 2), static_cast<int>(y - LABEL_FONT_SIZE - 4), LABEL_FONT_SIZE, RED);
        }

        // Health bar background
        DrawRectangle(
            static_cast<int>(centerX - barWidth / 2),
            static_cast<int>(y),
            static_cast<int>(barWidth),
            static_cast<int>(barHeight),
            DARKGRAY
        );

        // Health bar fill
        Color healthColor;
        if (m_bossHealth > 0.6f) healthColor = {100, 255, 100, 255};      // Green
        else if (m_bossHealth > 0.3f) healthColor = {255, 200, 50, 255};  // Yellow
        else healthColor = {255, 80, 80, 255};                             // Red

        DrawRectangle(
            static_cast<int>(centerX - barWidth / 2),
            static_cast<int>(y),
            static_cast<int>(barWidth * m_bossHealth),
            static_cast<int>(barHeight),
            healthColor
        );

        // Border
        DrawRectangleLines(
            static_cast<int>(centerX - barWidth / 2),
            static_cast<int>(y),
            static_cast<int>(barWidth),
            static_cast<int>(barHeight),
            WHITE
        );
    }

} // namespace rtype::ui
