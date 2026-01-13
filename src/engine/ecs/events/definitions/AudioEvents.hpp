/*
** R-Type ECS - Audio Events
** Events for sound and music
*/

#pragma once

#include <string>

namespace rtype::ecs::events {

    /**
     * @brief Request to play a sound effect
     */
    struct PlaySound {
        std::string soundId;
        float volume = 1.0f;
        float pitch = 1.0f;
        float pan = 0.0f;  // -1.0 left, 0 center, 1.0 right
        bool loop = false;
    };

    /**
     * @brief Request to stop a sound
     */
    struct StopSound {
        std::string soundId;
        bool fadeOut = false;
        float fadeTime = 0.0f;
    };

    /**
     * @brief Request to play music
     */
    struct PlayMusic {
        std::string musicId;
        float volume = 1.0f;
        bool loop = true;
        float fadeInTime = 0.0f;
    };

    /**
     * @brief Request to stop music
     */
    struct StopMusic {
        float fadeOutTime = 0.0f;
    };

    /**
     * @brief Request to pause/resume music
     */
    struct PauseMusic {
        bool paused;
    };

    /**
     * @brief Master volume changed
     */
    struct VolumeChanged {
        enum class Category { Master, Music, SFX, Voice } category;
        float oldVolume;
        float newVolume;
    };

    // ==================== Music State Events ====================

    /**
     * @brief Emitted when music state changes
     */
    struct MusicStateChanged {
        enum class State {
            Stopped,
            Playing,
            Paused
        };
        
        State newState;
        std::string trackPath;
    };

    /**
     * @brief Emitted when music finishes playing (if not looping)
     */
    struct MusicFinished {
        std::string trackPath;
    };

    // ==================== Background Events ====================

    /**
     * @brief Request to change the background image
     */
    struct BackgroundChangeRequest {
        std::string imagePath;      // Path to background image (empty = use starfield)
        int layer = -100;           // Render layer
    };

    /**
     * @brief Request to enable/disable starfield overlay
     */
    struct StarfieldToggleRequest {
        bool enabled = true;
        int layer = -99;            // Render layer (above background image)
    };

    /**
     * @brief Request to change procedural background type
     */
    struct ProceduralBgChangeRequest {
        int bgType = 0;             // ProceduralBgType enum value
        int layer = -101;           // Render layer (behind image)
        float cycleDuration = 120.0f; // Duration of full day/night cycle (seconds)
    };

    /**
     * @brief Emitted when background assets are loaded for a level
     */
    struct LevelAssetsLoaded {
        std::string backgroundPath;
        std::string stageMusicPath;
        std::string bossMusicPath;
        bool hasBackground;
        bool hasStageMusic;
        bool hasBossMusic;
    };

} // namespace rtype::ecs::events
