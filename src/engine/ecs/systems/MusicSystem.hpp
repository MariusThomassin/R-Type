/*
** R-Type ECS - MusicSystem
** Music playback management with event-driven control
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/events/definitions/AudioEvents.hpp"
#include "engine/ecs/events/definitions/GameEvents.hpp"

#include <raylib.h>
#include <string>
#include <iostream>

namespace rtype::ecs {

    class MusicSystem : public ISystem {
    public:
        explicit MusicSystem(EventBus& eventBus)
            : m_eventBus(eventBus) {
            
            m_eventBus.subscribe<events::PlayMusic>(
                [this](const events::PlayMusic& e) { handlePlayMusic(e); }
            );
            m_eventBus.subscribe<events::StopMusic>(
                [this](const events::StopMusic& e) { handleStopMusic(e); }
            );
            m_eventBus.subscribe<events::PauseMusic>(
                [this](const events::PauseMusic& e) { handlePauseMusic(e); }
            );
            m_eventBus.subscribe<events::LevelLoaded>(
                [this](const events::LevelLoaded&) { handleLevelLoaded(); }
            );
            m_eventBus.subscribe<events::BossPhaseChanged>(
                [this](const events::BossPhaseChanged& e) { handleBossPhase(e); }
            );
            m_eventBus.subscribe<events::LevelAssetsLoaded>(
                [this](const events::LevelAssetsLoaded& e) { handleLevelAssets(e); }
            );
        }
        
        ~MusicSystem() override { cleanup(); }
        
        void update(float) override {
            if (!m_enabled) return;
            
            if (m_musicLoaded && m_isPlaying) {
                UpdateMusicStream(m_currentMusic);
                
                if (m_loop && !IsMusicStreamPlaying(m_currentMusic)) {
                    PlayMusicStream(m_currentMusic);
                }
                
                if (!m_loop && !IsMusicStreamPlaying(m_currentMusic)) {
                    m_isPlaying = false;
                    m_eventBus.emit(events::MusicStateChanged{
                        events::MusicStateChanged::State::Stopped,
                        m_currentTrackPath
                    });
                    m_eventBus.emit(events::MusicFinished{m_currentTrackPath});
                }
            }
        }
        
        SystemPhase getPhase() const override { return SystemPhase::GameLogic; }
        
        void playTrack(const std::string& path, float volume = 1.0f, bool loop = true) {
            if (path.empty()) return;
            
            if (m_musicLoaded) {
                StopMusicStream(m_currentMusic);
                UnloadMusicStream(m_currentMusic);
                m_musicLoaded = false;
            }
            
            m_currentMusic = LoadMusicStream(path.c_str());
            if (m_currentMusic.frameCount > 0) {
                m_musicLoaded = true;
                m_currentTrackPath = path;
                m_volume = volume;
                m_loop = loop;
                
                SetMusicVolume(m_currentMusic, m_volume * m_masterVolume);
                PlayMusicStream(m_currentMusic);
                m_isPlaying = true;
                m_isPaused = false;
                
                m_eventBus.emit(events::MusicStateChanged{
                    events::MusicStateChanged::State::Playing,
                    m_currentTrackPath
                });
            }
        }
        
        void stop() {
            if (m_musicLoaded) {
                StopMusicStream(m_currentMusic);
                m_isPlaying = false;
                m_isPaused = false;
                m_eventBus.emit(events::MusicStateChanged{
                    events::MusicStateChanged::State::Stopped,
                    m_currentTrackPath
                });
            }
        }
        
        void pause() {
            if (m_musicLoaded && m_isPlaying && !m_isPaused) {
                PauseMusicStream(m_currentMusic);
                m_isPaused = true;
                m_eventBus.emit(events::MusicStateChanged{
                    events::MusicStateChanged::State::Paused,
                    m_currentTrackPath
                });
            }
        }
        
        void resume() {
            if (m_musicLoaded && m_isPaused) {
                ResumeMusicStream(m_currentMusic);
                m_isPaused = false;
                m_isPlaying = true;
                m_eventBus.emit(events::MusicStateChanged{
                    events::MusicStateChanged::State::Playing,
                    m_currentTrackPath
                });
            }
        }
        
        void setMasterVolume(float volume) {
            m_masterVolume = std::max(0.0f, std::min(1.0f, volume));
            if (m_musicLoaded) {
                SetMusicVolume(m_currentMusic, m_volume * m_masterVolume);
            }
        }
        
        void setVolume(float volume) {
            m_volume = std::max(0.0f, std::min(1.0f, volume));
            if (m_musicLoaded) {
                SetMusicVolume(m_currentMusic, m_volume * m_masterVolume);
            }
        }
        
        void setLevelMusic(const std::string& stageMusic, const std::string& bossMusic) {
            m_stageMusicPath = stageMusic;
            m_bossMusicPath = bossMusic;
        }
        
        void playBossMusic() {
            if (!m_bossMusicPath.empty()) {
                playTrack(m_bossMusicPath, m_volume, true);
                m_playingBossMusic = true;
            }
        }
        
        void playStageMusic() {
            if (!m_stageMusicPath.empty()) {
                playTrack(m_stageMusicPath, m_volume, true);
                m_playingBossMusic = false;
            }
        }
        
        bool isPlaying() const { return m_isPlaying; }
        bool isPaused() const { return m_isPaused; }
        bool isPlayingBossMusic() const { return m_playingBossMusic; }
        float getVolume() const { return m_volume; }
        float getMasterVolume() const { return m_masterVolume; }
        const std::string& getCurrentTrack() const { return m_currentTrackPath; }
        
    private:
        EventBus& m_eventBus;
        
        Music m_currentMusic = {};
        bool m_musicLoaded = false;
        bool m_isPlaying = false;
        bool m_isPaused = false;
        bool m_loop = true;
        
        std::string m_currentTrackPath;
        std::string m_stageMusicPath;
        std::string m_bossMusicPath;
        bool m_playingBossMusic = false;
        
        float m_volume = 1.0f;
        float m_masterVolume = 0.75f;
        
        void handlePlayMusic(const events::PlayMusic& e) { playTrack(e.musicId, e.volume, e.loop); }
        void handleStopMusic(const events::StopMusic&) { stop(); }
        void handlePauseMusic(const events::PauseMusic& e) { e.paused ? pause() : resume(); }
        void handleLevelLoaded() { playStageMusic(); }
        
        void handleBossPhase(const events::BossPhaseChanged& e) {
            if (e.newPhase == 1 && !m_playingBossMusic) playBossMusic();
        }
        
        void handleLevelAssets(const events::LevelAssetsLoaded& e) {
            m_stageMusicPath = e.stageMusicPath;
            m_bossMusicPath = e.bossMusicPath;
        }
        
        void cleanup() {
            if (m_musicLoaded) {
                StopMusicStream(m_currentMusic);
                UnloadMusicStream(m_currentMusic);
                m_musicLoaded = false;
            }
        }
    };

} // namespace rtype::ecs
