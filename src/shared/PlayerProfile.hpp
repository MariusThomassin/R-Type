/*
** R-Type - PlayerProfile
** Shared player profile data structure
** 
** Used by both client (local storage) and server (session tracking).
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace rtype {

    /**
     * @brief Maximum length for player name (excluding null terminator)
     */
    constexpr size_t MAX_PLAYER_NAME_LENGTH = 9;

    /**
     * @brief Number of available avatar options
     */
    constexpr uint8_t MAX_AVATARS = 12;

    /**
     * @brief Number of color scheme options
     */
    constexpr uint8_t MAX_COLOR_SCHEMES = 8;

    /**
     * @brief Player profile data
     * 
     * Contains:
     * - Player display name (up to 9 characters)
     * - Avatar selection (index into avatar sprite list)
     * - Color scheme (for ship customization)
     * 
     * This struct is designed to be network-serializable (fixed size, POD-like).
     */
    struct PlayerProfile {
        char name[MAX_PLAYER_NAME_LENGTH + 1] = "PLAYER";  // 9 chars + null
        uint8_t avatarId = 0;                               // Avatar index (0-11)
        uint8_t colorScheme = 0;                            // Color scheme (0-7)

        /**
         * @brief Default constructor
         */
        PlayerProfile() {
            std::memset(name, 0, sizeof(name));
            std::strncpy(name, "PLAYER", MAX_PLAYER_NAME_LENGTH);
        }

        /**
         * @brief Construct with name
         */
        explicit PlayerProfile(const std::string& playerName, uint8_t avatar = 0, uint8_t color = 0)
            : avatarId(avatar), colorScheme(color) {
            setName(playerName);
        }

        /**
         * @brief Set player name (truncates to 9 characters)
         */
        void setName(const std::string& newName) {
            std::memset(name, 0, sizeof(name));
            std::strncpy(name, newName.c_str(), MAX_PLAYER_NAME_LENGTH);
        }

        /**
         * @brief Get player name as string
         */
        std::string getName() const {
            return std::string(name);
        }

        /**
         * @brief Validate and clamp values to valid ranges
         */
        void validate() {
            // Ensure name is null-terminated
            name[MAX_PLAYER_NAME_LENGTH] = '\0';

            // Clamp avatar and color scheme
            if (avatarId >= MAX_AVATARS) {
                avatarId = 0;
            }
            if (colorScheme >= MAX_COLOR_SCHEMES) {
                colorScheme = 0;
            }
        }

        /**
         * @brief Check if profile is valid (has non-empty name)
         */
        bool isValid() const {
            return name[0] != '\0';
        }

        /**
         * @brief Equality operator
         */
        bool operator==(const PlayerProfile& other) const {
            return std::strcmp(name, other.name) == 0 &&
                   avatarId == other.avatarId &&
                   colorScheme == other.colorScheme;
        }

        bool operator!=(const PlayerProfile& other) const {
            return !(*this == other);
        }
    };

    /**
     * @brief Player statistics (tracked locally)
     */
    struct PlayerStats {
        uint32_t gamesPlayed = 0;       // Total games started
        uint32_t gamesCompleted = 0;    // Games finished (all levels or game over)
        uint32_t highestLoop = 0;       // Highest loop reached
        uint32_t totalPlayTime = 0;     // Total play time in seconds
        uint32_t totalKills = 0;        // Total enemies killed
        uint32_t totalDeaths = 0;       // Total player deaths
        uint32_t bossesDefeated = 0;    // Total bosses defeated
    };

    /**
     * @brief Avatar metadata
     */
    struct AvatarInfo {
        uint8_t id;
        const char* filename;
        const char* displayName;
        bool locked;  // Requires unlock condition
    };

    /**
     * @brief Get avatar info by ID
     */
    inline AvatarInfo getAvatarInfo(uint8_t id) {
        // Default avatars (always available)
        static const AvatarInfo avatars[] = {
            {0, "avatar_default.png", "R-9A", false},
            {1, "avatar_pilot1.png", "Pilot 1", false},
            {2, "avatar_pilot2.png", "Pilot 2", false},
            {3, "avatar_pilot3.png", "Pilot 3", false},
            {4, "avatar_force.png", "Force", false},
            {5, "avatar_retro.png", "Retro", false},
            // Unlockable avatars
            {6, "avatar_boss.png", "Dobkeratops", true},
            {7, "avatar_ace.png", "Ace", true},
            {8, "avatar_legend.png", "Legend", true},
            {9, "avatar_r9b.png", "R-9B", true},
            {10, "avatar_r9c.png", "R-9C", true},
            {11, "avatar_custom.png", "Custom", true}
        };

        if (id < MAX_AVATARS) {
            return avatars[id];
        }
        return avatars[0];  // Default
    }

    /**
     * @brief Color scheme names
     */
    inline const char* getColorSchemeName(uint8_t scheme) {
        static const char* schemes[] = {
            "Default",      // 0 - Original R-Type colors
            "Red",          // 1 - Red variant
            "Blue",         // 2 - Blue variant
            "Green",        // 3 - Green variant
            "Gold",         // 4 - Gold/Yellow variant
            "Purple",       // 5 - Purple variant
            "Stealth",      // 6 - Dark gray/black
            "Neon"          // 7 - Bright neon colors
        };

        if (scheme < MAX_COLOR_SCHEMES) {
            return schemes[scheme];
        }
        return schemes[0];
    }

} // namespace rtype
