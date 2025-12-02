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

} // namespace rtype::ecs::events
