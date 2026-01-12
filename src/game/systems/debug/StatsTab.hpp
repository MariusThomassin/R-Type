/*
** R-Type ECS - Stats Debug Tab
** Shows entity counts and system statistics using UI widget library
*/

#pragma once

#include "DebugTab.hpp"
#include "engine/ui/widgets/TextWidget.hpp"
#include "engine/ui/UIColor.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/SpriteComponent.hpp"
#include "game/components/SpritesheetComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include <memory>
#include <raylib.h>

namespace rtype::ecs::debug {

    class StatsTab : public IDebugTab {
    public:
        StatsTab() {
            initWidgets();
        }

        const char* getName() const override { return "Stats"; }

        void update(float dt) override {
            m_animTime += dt;
            m_frameTime = dt;
            
            // Update dynamic text content
            if (m_registry) {
                int entities = static_cast<int>(m_registry->getEntityCount());
                int bullets = 0, sprites = 0, sheets = 0;
                m_registry->forEach<ProjectileComponent>([&bullets](EntityId) { bullets++; });
                m_registry->forEach<SpriteComponent>([&sprites](EntityId) { sprites++; });
                m_registry->forEach<SpritesheetComponent>([&sheets](EntityId) { sheets++; });

                char buf[128];
                snprintf(buf, sizeof(buf), "Total Entities: %d", entities);
                m_entityCountText->setText(buf);
                
                snprintf(buf, sizeof(buf), "  - With SpriteComponent: %d", sprites);
                m_spriteCountText->setText(buf);
                
                snprintf(buf, sizeof(buf), "  - With SpritesheetComponent: %d", sheets);
                m_sheetCountText->setText(buf);
                
                snprintf(buf, sizeof(buf), "  - Projectiles: %d", bullets);
                m_bulletCountText->setText(buf);
                
                snprintf(buf, sizeof(buf), "FPS: %d  |  Frame: %.2fms", GetFPS(), m_frameTime * 1000);
                m_fpsText->setText(buf);
            }
        }

        void draw(int y) override {
            const int x = 30;
            
            // Title
            m_titleText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_titleText->renderSelf();
            y += 35;

            // Entity counts
            m_entityCountText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_entityCountText->renderSelf();
            y += 24;
            
            m_spriteCountText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_spriteCountText->renderSelf();
            y += 20;
            
            m_sheetCountText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_sheetCountText->renderSelf();
            y += 20;
            
            m_bulletCountText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_bulletCountText->renderSelf();
            y += 30;

            // FPS
            m_fpsText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_fpsText->renderSelf();
            y += 40;

            // Architecture info
            m_archTitleText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_archTitleText->renderSelf();
            y += 24;
            
            m_archDesc1Text->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_archDesc1Text->renderSelf();
            y += 20;
            
            m_archDesc2Text->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_archDesc2Text->renderSelf();
            y += 30;

            // Controls
            m_controlsTitleText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_controlsTitleText->renderSelf();
            y += 24;
            
            m_controlsDescText->setPosition(static_cast<float>(x), static_cast<float>(y));
            m_controlsDescText->renderSelf();
        }

    private:
        float m_frameTime = 0.0f;

        // Text widgets
        std::shared_ptr<ui::TextWidget> m_titleText;
        std::shared_ptr<ui::TextWidget> m_entityCountText;
        std::shared_ptr<ui::TextWidget> m_spriteCountText;
        std::shared_ptr<ui::TextWidget> m_sheetCountText;
        std::shared_ptr<ui::TextWidget> m_bulletCountText;
        std::shared_ptr<ui::TextWidget> m_fpsText;
        std::shared_ptr<ui::TextWidget> m_archTitleText;
        std::shared_ptr<ui::TextWidget> m_archDesc1Text;
        std::shared_ptr<ui::TextWidget> m_archDesc2Text;
        std::shared_ptr<ui::TextWidget> m_controlsTitleText;
        std::shared_ptr<ui::TextWidget> m_controlsDescText;

        void initWidgets() {
            // Title
            m_titleText = std::make_shared<ui::TextWidget>("Engine Statistics", 20);
            m_titleText->setTextColor(ui::UIColor::White());
            m_titleText->setBackgroundColor(ui::UIColor::Transparent());

            // Entity stats (dynamic)
            m_entityCountText = std::make_shared<ui::TextWidget>("Total Entities: 0", 16);
            m_entityCountText->setTextColor(ui::UIColor::White());
            m_entityCountText->setBackgroundColor(ui::UIColor::Transparent());

            m_spriteCountText = std::make_shared<ui::TextWidget>("  - With SpriteComponent: 0", 14);
            m_spriteCountText->setTextColor(ui::UIColor(180, 180, 180, 255));
            m_spriteCountText->setBackgroundColor(ui::UIColor::Transparent());

            m_sheetCountText = std::make_shared<ui::TextWidget>("  - With SpritesheetComponent: 0", 14);
            m_sheetCountText->setTextColor(ui::UIColor(180, 180, 180, 255));
            m_sheetCountText->setBackgroundColor(ui::UIColor::Transparent());

            m_bulletCountText = std::make_shared<ui::TextWidget>("  - Projectiles: 0", 14);
            m_bulletCountText->setTextColor(ui::UIColor(180, 180, 180, 255));
            m_bulletCountText->setBackgroundColor(ui::UIColor::Transparent());

            // FPS (dynamic)
            m_fpsText = std::make_shared<ui::TextWidget>("FPS: 0  |  Frame: 0.00ms", 16);
            m_fpsText->setTextColor(ui::UIColor(100, 255, 100, 255));
            m_fpsText->setBackgroundColor(ui::UIColor::Transparent());

            // Architecture info (static)
            m_archTitleText = std::make_shared<ui::TextWidget>("Architecture: IRenderable Pattern", 16);
            m_archTitleText->setTextColor(ui::UIColor(100, 200, 255, 255));
            m_archTitleText->setBackgroundColor(ui::UIColor::Transparent());

            m_archDesc1Text = std::make_shared<ui::TextWidget>("Components render themselves via render(transform, ctx)", 14);
            m_archDesc1Text->setTextColor(ui::UIColor(150, 150, 150, 255));
            m_archDesc1Text->setBackgroundColor(ui::UIColor::Transparent());

            m_archDesc2Text = std::make_shared<ui::TextWidget>("RenderSystem is a thin coordinator (~220 lines)", 14);
            m_archDesc2Text->setTextColor(ui::UIColor(150, 150, 150, 255));
            m_archDesc2Text->setBackgroundColor(ui::UIColor::Transparent());

            // Controls (static)
            m_controlsTitleText = std::make_shared<ui::TextWidget>("Controls:", 16);
            m_controlsTitleText->setTextColor(ui::UIColor(255, 255, 100, 255));
            m_controlsTitleText->setBackgroundColor(ui::UIColor::Transparent());

            m_controlsDescText = std::make_shared<ui::TextWidget>("WASD/Arrows - Move  |  Space - Shoot  |  G - Bullet Pattern", 14);
            m_controlsDescText->setTextColor(ui::UIColor(150, 150, 150, 255));
            m_controlsDescText->setBackgroundColor(ui::UIColor::Transparent());
        }
    };

} // namespace rtype::ecs::debug
