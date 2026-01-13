/*
** R-Type ECS - BackgroundSystem
** Manages background entities and responds to level changes
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/components/ImageBackgroundComponent.hpp"
#include "engine/ecs/components/ProceduralBackgroundComponent.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/events/definitions/AudioEvents.hpp"

#include <string>
#include <optional>
#include <iostream>

namespace rtype::ecs {

    class BackgroundSystem : public ISystem {
    public:
        BackgroundSystem(Registry& registry, EventBus& eventBus, 
                        int screenWidth = 1280, int screenHeight = 720)
            : m_registry(registry)
            , m_eventBus(eventBus)
            , m_screenWidth(screenWidth)
            , m_screenHeight(screenHeight) {
            
            m_eventBus.subscribe<events::BackgroundChangeRequest>(
                [this](const events::BackgroundChangeRequest& e) { handleBackgroundChange(e); }
            );
            m_eventBus.subscribe<events::LevelAssetsLoaded>(
                [this](const events::LevelAssetsLoaded& e) { handleLevelAssets(e); }
            );
            m_eventBus.subscribe<events::ProceduralBgChangeRequest>(
                [this](const events::ProceduralBgChangeRequest& e) { handleProceduralBgChange(e); }
            );
        }
        
        void update(float dt) override {
            if (!m_enabled) return;
            
            // Update procedural background
            if (m_proceduralEntity.has_value()) {
                auto* proc = m_registry.tryGetComponent<ProceduralBackgroundComponent>(m_proceduralEntity.value());
                if (proc) proc->update(dt);
            }
        }
        
        SystemPhase getPhase() const override { return SystemPhase::Input; }
        
        void setBackgroundImage(const std::string& imagePath, int layer = -100) {
            if (m_imageBackgroundEntity.has_value()) {
                m_registry.destroyEntity(m_imageBackgroundEntity.value());
                m_imageBackgroundEntity = std::nullopt;
            }
            
            if (imagePath.empty()) return;
            
            Entity bgEntity = m_registry.createEntity();
            m_imageBackgroundEntity = bgEntity;
            
            m_registry.addComponent(bgEntity, TransformComponent(0, 0));
            
            ImageBackgroundComponent imgBg(imagePath, m_screenWidth, m_screenHeight);
            imgBg.layer = layer;
            
            if (imgBg.hasValidTexture()) {
                m_registry.addComponent(bgEntity, std::move(imgBg));
                std::cout << "[BackgroundSystem] Loaded image: " << imagePath << std::endl;
            } else {
                std::cout << "[BackgroundSystem] Failed to load: " << imagePath << std::endl;
                m_registry.destroyEntity(bgEntity);
                m_imageBackgroundEntity = std::nullopt;
            }
        }
        
        void setProceduralBackground(ProceduralBgType type, int layer = -101, float cycleDuration = 120.0f) {
            if (m_proceduralEntity.has_value()) {
                m_registry.destroyEntity(m_proceduralEntity.value());
                m_proceduralEntity = std::nullopt;
            }
            
            Entity procEntity = m_registry.createEntity();
            m_proceduralEntity = procEntity;
            
            m_registry.addComponent(procEntity, TransformComponent(0, 0));
            
            ProceduralBackgroundComponent proc(type, m_screenWidth, m_screenHeight);
            proc.layer = layer;
            proc.cycleDuration = cycleDuration;
            m_registry.addComponent(procEntity, std::move(proc));
        }
        
        void createDefaultBackground() {
            clearAllBackgrounds();
            setProceduralBackground(ProceduralBgType::Space, -101);
        }
        
        void clearAllBackgrounds() {
            if (m_imageBackgroundEntity.has_value()) {
                m_registry.destroyEntity(m_imageBackgroundEntity.value());
                m_imageBackgroundEntity = std::nullopt;
            }
            if (m_proceduralEntity.has_value()) {
                m_registry.destroyEntity(m_proceduralEntity.value());
                m_proceduralEntity = std::nullopt;
            }
        }
        
        void setScreenSize(int width, int height) {
            m_screenWidth = width;
            m_screenHeight = height;
            
            if (m_imageBackgroundEntity.has_value()) {
                auto* imgBg = m_registry.tryGetComponent<ImageBackgroundComponent>(
                    m_imageBackgroundEntity.value());
                if (imgBg) imgBg->setScreenSize(width, height);
            }
            if (m_proceduralEntity.has_value()) {
                auto* proc = m_registry.tryGetComponent<ProceduralBackgroundComponent>(
                    m_proceduralEntity.value());
                if (proc) proc->setScreenSize(width, height);
            }
        }
        
        bool hasImageBackground() const { return m_imageBackgroundEntity.has_value(); }
        bool hasProceduralBackground() const { return m_proceduralEntity.has_value(); }
        
    private:
        Registry& m_registry;
        EventBus& m_eventBus;
        int m_screenWidth;
        int m_screenHeight;
        
        std::optional<Entity> m_imageBackgroundEntity;
        std::optional<Entity> m_proceduralEntity;
        
        void handleBackgroundChange(const events::BackgroundChangeRequest& e) {
            setBackgroundImage(e.imagePath, e.layer);
        }
        
        void handleProceduralBgChange(const events::ProceduralBgChangeRequest& e) {
            setProceduralBackground(static_cast<ProceduralBgType>(e.bgType), e.layer, e.cycleDuration);
        }
        
        void handleLevelAssets(const events::LevelAssetsLoaded& e) {
            // Set procedural background underneath
            setProceduralBackground(ProceduralBgType::Space, -101);
            
            if (e.hasBackground && !e.backgroundPath.empty()) {
                setBackgroundImage(e.backgroundPath, -100);
            }
        }
    };

} // namespace rtype::ecs
