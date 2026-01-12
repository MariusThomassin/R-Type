/*
** R-Type - HUDWidget
** Heads-Up Display for in-game information
*/

#ifndef HUDWIDGET_HPP_
#define HUDWIDGET_HPP_

#include "../Widget.hpp"
#include <string>
#include <sstream>
#include <iomanip>

namespace rtype::ui {

    /**
     * @brief HUD Widget for displaying game state during gameplay
     * 
     * Shows:
     * - Score with combo multiplier
     * - Lives remaining (visual hearts or numeric)
     * - Weapon power level indicator
     * - Force orb status
     * - Bombs remaining
     * - Wave/level progress
     */
    class HUDWidget : public Widget {
    public:
        HUDWidget();
        ~HUDWidget() override = default;

        // Score
        void setScore(int score);
        int getScore() const { return m_score; }
        
        void setHighScore(int highScore);
        int getHighScore() const { return m_highScore; }

        // Lives
        void setLives(int lives);
        int getLives() const { return m_lives; }
        void setMaxLives(int maxLives) { m_maxLives = maxLives; }

        // Weapon
        void setWeaponLevel(int level);
        int getWeaponLevel() const { return m_weaponLevel; }
        static constexpr int MAX_WEAPON_LEVEL = 4;

        // Force Orb
        void setForceOrbActive(bool active);
        void setForceOrbLevel(int level);
        bool hasForceOrb() const { return m_forceOrbActive; }

        // Bombs
        void setBombs(int count);
        int getBombs() const { return m_bombs; }

        // Wave/Level info
        void setCurrentWave(int wave);
        void setTotalWaves(int total);
        void setLevelName(const std::string& name);

        // Combo system
        void setCombo(int combo);
        void setComboMultiplier(float multiplier);

        // Boss health
        void setBossActive(bool active);
        void setBossHealth(float healthPercent);
        void setBossName(const std::string& name);

        void render() const override;

    private:
        void renderScore() const;
        void renderLives() const;
        void renderWeaponLevel() const;
        void renderForceOrb() const;
        void renderBombs() const;
        void renderWaveInfo() const;
        void renderCombo() const;
        void renderBossHealth() const;

        std::string formatScore(int score) const;

        // Game state
        int m_score = 0;
        int m_highScore = 0;
        int m_lives = 3;
        int m_maxLives = 5;
        int m_weaponLevel = 0;
        bool m_forceOrbActive = false;
        int m_forceOrbLevel = 0;
        int m_bombs = 0;
        int m_currentWave = 1;
        int m_totalWaves = 1;
        std::string m_levelName;
        int m_combo = 0;
        float m_comboMultiplier = 1.0f;
        bool m_bossActive = false;
        float m_bossHealth = 1.0f;
        std::string m_bossName;

        // Layout constants
        static constexpr float MARGIN = 10.0f;
        static constexpr float ELEMENT_SPACING = 20.0f;
        static constexpr int SCORE_FONT_SIZE = 28;
        static constexpr int LABEL_FONT_SIZE = 18;
        static constexpr int SMALL_FONT_SIZE = 14;
    };

} // namespace rtype::ui

#endif /* !HUDWIDGET_HPP_ */
