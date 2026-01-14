/*
** R-Type Client - ScoreManager
** Manages local high score storage and session scoring
*/

#pragma once

#include "shared/PlayerProfile.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace rtype::client {

    /**
     * @brief High score entry
     */
    struct HighScoreEntry {
        std::string playerName;     // Player name (from profile)
        uint8_t avatarId = 0;       // Avatar at time of score
        uint32_t score = 0;         // Final score
        uint8_t loop = 0;           // Loop reached (0 = first playthrough)
        uint8_t level = 0;          // Level reached within loop
        std::string date;           // Date achieved (YYYY-MM-DD)

        bool operator>(const HighScoreEntry& other) const {
            return score > other.score;
        }
    };

    /**
     * @brief Manages high score persistence and session scoring
     * 
     * Features:
     * - Top 10 high scores stored locally
     * - Session score tracking during gameplay
     * - Auto-save when new high score achieved
     */
    class ScoreManager {
    public:
        /**
         * @brief Maximum number of high score entries to keep
         */
        static constexpr size_t MAX_HIGH_SCORES = 10;

        /**
         * @brief Construct ScoreManager
         * @param scoresPath Path to high scores JSON file
         */
        explicit ScoreManager(const std::string& scoresPath = "config/highscores.json")
            : m_scoresPath(scoresPath) {
            load();
        }

        /**
         * @brief Load high scores from disk
         */
        bool load() {
            try {
                std::filesystem::path path(m_scoresPath);
                if (path.has_parent_path()) {
                    std::filesystem::create_directories(path.parent_path());
                }

                std::ifstream file(m_scoresPath);
                if (!file.is_open()) {
                    std::cout << "[ScoreManager] No high scores file found" << std::endl;
                    return false;
                }

                nlohmann::json j;
                file >> j;

                m_highScores.clear();

                if (j.contains("entries") && j["entries"].is_array()) {
                    for (const auto& entry : j["entries"]) {
                        HighScoreEntry hs;
                        if (entry.contains("name")) hs.playerName = entry["name"];
                        if (entry.contains("avatarId")) hs.avatarId = entry["avatarId"];
                        if (entry.contains("score")) hs.score = entry["score"];
                        if (entry.contains("loop")) hs.loop = entry["loop"];
                        if (entry.contains("level")) hs.level = entry["level"];
                        if (entry.contains("date")) hs.date = entry["date"];
                        m_highScores.push_back(hs);
                    }
                }

                // Sort by score descending
                std::sort(m_highScores.begin(), m_highScores.end(), std::greater<HighScoreEntry>());

                // Keep only top N
                if (m_highScores.size() > MAX_HIGH_SCORES) {
                    m_highScores.resize(MAX_HIGH_SCORES);
                }

                std::cout << "[ScoreManager] Loaded " << m_highScores.size() << " high scores" << std::endl;
                return true;

            } catch (const std::exception& e) {
                std::cerr << "[ScoreManager] Error loading scores: " << e.what() << std::endl;
                return false;
            }
        }

        /**
         * @brief Save high scores to disk
         */
        bool save() {
            try {
                std::filesystem::path path(m_scoresPath);
                if (path.has_parent_path()) {
                    std::filesystem::create_directories(path.parent_path());
                }

                nlohmann::json j;
                j["version"] = 1;

                nlohmann::json entries = nlohmann::json::array();
                for (const auto& hs : m_highScores) {
                    entries.push_back({
                        {"name", hs.playerName},
                        {"avatarId", hs.avatarId},
                        {"score", hs.score},
                        {"loop", hs.loop},
                        {"level", hs.level},
                        {"date", hs.date}
                    });
                }
                j["entries"] = entries;

                std::ofstream file(m_scoresPath);
                if (!file.is_open()) {
                    std::cerr << "[ScoreManager] Failed to open file for writing" << std::endl;
                    return false;
                }

                file << j.dump(2);
                file.close();

                std::cout << "[ScoreManager] Saved " << m_highScores.size() << " high scores" << std::endl;
                return true;

            } catch (const std::exception& e) {
                std::cerr << "[ScoreManager] Error saving scores: " << e.what() << std::endl;
                return false;
            }
        }

        // ============================================================
        // Session Score Tracking
        // ============================================================

        /**
         * @brief Reset session score for new game
         */
        void resetSession() {
            m_sessionScore = 0;
            m_sessionLoop = 0;
            m_sessionLevel = 0;
            m_sessionKills = 0;
        }

        /**
         * @brief Update session score
         * @param delta Points to add
         */
        void updateSessionScore(int32_t delta) {
            m_sessionScore += delta;
            if (m_sessionScore < 0) m_sessionScore = 0;
        }

        /**
         * @brief Set session level progress
         */
        void setSessionProgress(uint8_t loop, uint8_t level) {
            m_sessionLoop = loop;
            m_sessionLevel = level;
        }

        /**
         * @brief Get current session score
         */
        uint32_t getSessionScore() const { return m_sessionScore; }

        /**
         * @brief Get session loop
         */
        uint8_t getSessionLoop() const { return m_sessionLoop; }

        /**
         * @brief Get session level
         */
        uint8_t getSessionLevel() const { return m_sessionLevel; }

        /**
         * @brief Increment kill counter
         */
        void addSessionKill() { m_sessionKills++; }

        /**
         * @brief Get session kills
         */
        uint32_t getSessionKills() const { return m_sessionKills; }

        // ============================================================
        // High Score Management
        // ============================================================

        /**
         * @brief Check if current session score qualifies for high scores
         */
        bool isHighScore() const {
            if (m_sessionScore == 0) return false;
            if (m_highScores.size() < MAX_HIGH_SCORES) return true;
            return m_sessionScore > m_highScores.back().score;
        }

        /**
         * @brief Check if a score qualifies for high scores
         */
        bool isHighScore(uint32_t score) const {
            if (score == 0) return false;
            if (m_highScores.size() < MAX_HIGH_SCORES) return true;
            return score > m_highScores.back().score;
        }

        /**
         * @brief Get rank for a score (1-based, 0 if not qualifying)
         */
        int getRank(uint32_t score) const {
            if (score == 0) return 0;

            int rank = 1;
            for (const auto& hs : m_highScores) {
                if (score > hs.score) {
                    return rank;
                }
                rank++;
            }

            // If we haven't filled all slots yet
            if (m_highScores.size() < MAX_HIGH_SCORES) {
                return static_cast<int>(m_highScores.size() + 1);
            }

            return 0;  // Doesn't qualify
        }

        /**
         * @brief Add current session as high score entry
         * @param profile Player profile (for name and avatar)
         * @return Rank achieved (1-10), or 0 if not added
         */
        int addHighScore(const PlayerProfile& profile) {
            return addHighScore(profile, m_sessionScore, m_sessionLoop, m_sessionLevel);
        }

        /**
         * @brief Add a high score entry
         */
        int addHighScore(const PlayerProfile& profile, uint32_t score, uint8_t loop, uint8_t level) {
            if (!isHighScore(score)) {
                return 0;
            }

            // Get current date
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::tm tm = *std::localtime(&time);
            std::ostringstream dateStream;
            dateStream << std::put_time(&tm, "%Y-%m-%d");

            HighScoreEntry entry;
            entry.playerName = profile.getName();
            entry.avatarId = profile.avatarId;
            entry.score = score;
            entry.loop = loop;
            entry.level = level;
            entry.date = dateStream.str();

            // Insert in sorted position
            m_highScores.push_back(entry);
            std::sort(m_highScores.begin(), m_highScores.end(), std::greater<HighScoreEntry>());

            // Trim to max size
            if (m_highScores.size() > MAX_HIGH_SCORES) {
                m_highScores.resize(MAX_HIGH_SCORES);
            }

            // Find rank of new entry
            int rank = 1;
            for (const auto& hs : m_highScores) {
                if (hs.score == score && hs.playerName == entry.playerName && hs.date == entry.date) {
                    break;
                }
                rank++;
            }

            // Auto-save
            save();

            std::cout << "[ScoreManager] New high score! Rank #" << rank << ": " << score << " by " << entry.playerName << std::endl;
            return rank;
        }

        /**
         * @brief Finish the current session
         * 
         * If the session score qualifies as a high score, it's automatically added.
         * Session state is then reset.
         * 
         * @param playerName Name to record in high scores
         * @return Rank achieved (1-10), or 0 if not a high score
         */
        int finishSession(const std::string& playerName) {
            int rank = 0;

            if (isHighScore()) {
                PlayerProfile tempProfile;
                std::strncpy(tempProfile.name, playerName.c_str(), sizeof(tempProfile.name) - 1);
                tempProfile.name[sizeof(tempProfile.name) - 1] = '\0';
                tempProfile.avatarId = 0;  // Default avatar

                rank = addHighScore(tempProfile, m_sessionScore, m_sessionLoop, m_sessionLevel);
                std::cout << "[ScoreManager] Session finished with score " << m_sessionScore 
                          << " (rank: " << rank << ")" << std::endl;
            } else {
                std::cout << "[ScoreManager] Session finished with score " << m_sessionScore 
                          << " (not a high score)" << std::endl;
            }

            // Reset session
            resetSession();

            return rank;
        }

        /**
         * @brief Get high scores list
         */
        const std::vector<HighScoreEntry>& getHighScores() const { return m_highScores; }

        /**
         * @brief Get high score at rank (1-based)
         */
        const HighScoreEntry* getHighScore(int rank) const {
            if (rank < 1 || rank > static_cast<int>(m_highScores.size())) {
                return nullptr;
            }
            return &m_highScores[rank - 1];
        }

        /**
         * @brief Get the top high score (or nullptr if none)
         */
        const HighScoreEntry* getTopScore() const {
            if (m_highScores.empty()) return nullptr;
            return &m_highScores[0];
        }

    private:
        std::string m_scoresPath;
        std::vector<HighScoreEntry> m_highScores;

        // Session tracking
        uint32_t m_sessionScore = 0;
        uint8_t m_sessionLoop = 0;
        uint8_t m_sessionLevel = 0;
        uint32_t m_sessionKills = 0;
    };

} // namespace rtype::client
