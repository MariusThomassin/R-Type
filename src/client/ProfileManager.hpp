/*
** R-Type Client - ProfileManager
** Manages local player profile storage and retrieval
*/

#pragma once

#include "shared/PlayerProfile.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace rtype::client {

    /**
     * @brief Manages local player profile persistence
     * 
     * Handles loading and saving player profile to JSON file.
     * Profile includes name, avatar selection, color scheme, and stats.
     */
    class ProfileManager {
    public:
        /**
         * @brief Construct ProfileManager
         * @param profilePath Path to profile JSON file (default: config/profile.json)
         */
        explicit ProfileManager(const std::string& profilePath = "config/profile.json")
            : m_profilePath(profilePath) {
            load();
        }

        /**
         * @brief Load profile from disk
         * @return true if loaded successfully, false if using defaults
         */
        bool load() {
            try {
                // Ensure directory exists
                std::filesystem::path path(m_profilePath);
                if (path.has_parent_path()) {
                    std::filesystem::create_directories(path.parent_path());
                }

                // Try to open file
                std::ifstream file(m_profilePath);
                if (!file.is_open()) {
                    std::cout << "[ProfileManager] No profile found, using defaults" << std::endl;
                    return false;
                }

                // Parse JSON
                nlohmann::json j;
                file >> j;

                // Read profile data
                if (j.contains("name")) {
                    m_profile.setName(j["name"].get<std::string>());
                }
                if (j.contains("avatarId")) {
                    m_profile.avatarId = j["avatarId"].get<uint8_t>();
                }
                if (j.contains("colorScheme")) {
                    m_profile.colorScheme = j["colorScheme"].get<uint8_t>();
                }

                // Read stats
                if (j.contains("stats")) {
                    auto& stats = j["stats"];
                    if (stats.contains("gamesPlayed")) m_stats.gamesPlayed = stats["gamesPlayed"];
                    if (stats.contains("gamesCompleted")) m_stats.gamesCompleted = stats["gamesCompleted"];
                    if (stats.contains("highestLoop")) m_stats.highestLoop = stats["highestLoop"];
                    if (stats.contains("totalPlayTime")) m_stats.totalPlayTime = stats["totalPlayTime"];
                    if (stats.contains("totalKills")) m_stats.totalKills = stats["totalKills"];
                    if (stats.contains("totalDeaths")) m_stats.totalDeaths = stats["totalDeaths"];
                    if (stats.contains("bossesDefeated")) m_stats.bossesDefeated = stats["bossesDefeated"];
                }

                // Read unlocks
                if (j.contains("unlockedAvatars")) {
                    m_unlockedAvatars = j["unlockedAvatars"].get<std::vector<uint8_t>>();
                }

                // Validate
                m_profile.validate();

                std::cout << "[ProfileManager] Loaded profile: " << m_profile.getName() << std::endl;
                return true;

            } catch (const std::exception& e) {
                std::cerr << "[ProfileManager] Error loading profile: " << e.what() << std::endl;
                return false;
            }
        }

        /**
         * @brief Save profile to disk
         * @return true if saved successfully
         */
        bool save() {
            try {
                // Ensure directory exists
                std::filesystem::path path(m_profilePath);
                if (path.has_parent_path()) {
                    std::filesystem::create_directories(path.parent_path());
                }

                // Build JSON
                nlohmann::json j;
                j["version"] = 1;
                j["name"] = m_profile.getName();
                j["avatarId"] = m_profile.avatarId;
                j["colorScheme"] = m_profile.colorScheme;

                j["stats"] = {
                    {"gamesPlayed", m_stats.gamesPlayed},
                    {"gamesCompleted", m_stats.gamesCompleted},
                    {"highestLoop", m_stats.highestLoop},
                    {"totalPlayTime", m_stats.totalPlayTime},
                    {"totalKills", m_stats.totalKills},
                    {"totalDeaths", m_stats.totalDeaths},
                    {"bossesDefeated", m_stats.bossesDefeated}
                };

                j["unlockedAvatars"] = m_unlockedAvatars;

                // Write to file
                std::ofstream file(m_profilePath);
                if (!file.is_open()) {
                    std::cerr << "[ProfileManager] Failed to open file for writing: " << m_profilePath << std::endl;
                    return false;
                }

                file << j.dump(2);  // Pretty print with 2-space indent
                file.close();

                std::cout << "[ProfileManager] Profile saved: " << m_profile.getName() << std::endl;
                return true;

            } catch (const std::exception& e) {
                std::cerr << "[ProfileManager] Error saving profile: " << e.what() << std::endl;
                return false;
            }
        }

        // ============================================================
        // Profile Accessors
        // ============================================================

        /**
         * @brief Get current profile (const)
         */
        const PlayerProfile& getProfile() const { return m_profile; }

        /**
         * @brief Get current profile (mutable)
         */
        PlayerProfile& getProfile() { return m_profile; }

        /**
         * @brief Set player name
         */
        void setName(const std::string& name) {
            m_profile.setName(name);
        }

        /**
         * @brief Set avatar ID
         */
        void setAvatar(uint8_t avatarId) {
            if (avatarId < MAX_AVATARS) {
                m_profile.avatarId = avatarId;
            }
        }

        /**
         * @brief Set color scheme
         */
        void setColorScheme(uint8_t scheme) {
            if (scheme < MAX_COLOR_SCHEMES) {
                m_profile.colorScheme = scheme;
            }
        }

        // ============================================================
        // Stats Accessors
        // ============================================================

        /**
         * @brief Get player stats (const)
         */
        const PlayerStats& getStats() const { return m_stats; }

        /**
         * @brief Get player stats (mutable)
         */
        PlayerStats& getStats() { return m_stats; }

        /**
         * @brief Increment games played counter
         */
        void incrementGamesPlayed() { m_stats.gamesPlayed++; }

        /**
         * @brief Increment games completed counter
         */
        void incrementGamesCompleted() { m_stats.gamesCompleted++; }

        /**
         * @brief Add play time
         */
        void addPlayTime(uint32_t seconds) { m_stats.totalPlayTime += seconds; }

        /**
         * @brief Add kills
         */
        void addKills(uint32_t count) { m_stats.totalKills += count; }

        /**
         * @brief Add deaths
         */
        void addDeaths(uint32_t count) { m_stats.totalDeaths += count; }

        /**
         * @brief Update highest loop if higher
         */
        void updateHighestLoop(uint32_t loop) {
            if (loop > m_stats.highestLoop) {
                m_stats.highestLoop = loop;
            }
        }

        // ============================================================
        // Avatar Unlocks
        // ============================================================

        /**
         * @brief Check if avatar is unlocked
         */
        bool isAvatarUnlocked(uint8_t avatarId) const {
            // First 6 avatars are always unlocked
            if (avatarId < 6) return true;

            // Check unlock list
            for (uint8_t id : m_unlockedAvatars) {
                if (id == avatarId) return true;
            }
            return false;
        }

        /**
         * @brief Unlock an avatar
         */
        void unlockAvatar(uint8_t avatarId) {
            if (!isAvatarUnlocked(avatarId)) {
                m_unlockedAvatars.push_back(avatarId);
                std::cout << "[ProfileManager] Unlocked avatar: " << getAvatarInfo(avatarId).displayName << std::endl;
            }
        }

        /**
         * @brief Get list of unlocked avatars
         */
        const std::vector<uint8_t>& getUnlockedAvatars() const { return m_unlockedAvatars; }

    private:
        std::string m_profilePath;
        PlayerProfile m_profile;
        PlayerStats m_stats;
        std::vector<uint8_t> m_unlockedAvatars;
    };

} // namespace rtype::client
