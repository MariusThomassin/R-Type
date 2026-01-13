/*
** R-Type ECS - ProceduralBackgroundComponent
** Animated procedural backgrounds (sunrise, city, space, futuristic)
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include "engine/graphics/IRenderable.hpp"
#include "engine/ecs/components/TransformComponent.hpp"

#include <raylib.h>
#include <vector>
#include <cmath>
#include <cstdint>

namespace rtype::ecs {

    enum class ProceduralBgType {
        Sunrise,        // Gradient sky with sun/moon
        City,           // Silhouette cityscape parallax
        Space,          // Deep space with nebula
        Futuristic,     // Grid/wireframe 3D effect
        Ocean,          // Wavy ocean with reflection
        Mountains       // Layered mountain silhouettes
    };

    struct ProceduralBackgroundComponent : public IComponent, public IRenderable {
        ProceduralBgType type = ProceduralBgType::Space;
        int screenWidth = 1280;
        int screenHeight = 720;
        int layer = -101;  // Behind image backgrounds
        float time = 0.0f;
        float scrollSpeed = 50.0f;
        
        // Evolution settings (for day/night cycle, etc.)
        float cycleDuration = 120.0f;  // Full cycle in seconds (2 minutes default)
        float cycleProgress = 0.0f;    // 0.0 to 1.0 through the cycle
        
        // Color themes
        Color primaryColor = {20, 30, 60, 255};
        Color secondaryColor = {80, 40, 100, 255};
        Color accentColor = {255, 150, 50, 255};
        
        // Parallax layers data
        struct ParallaxLayer {
            std::vector<float> heights;
            float speed;
            Color color;
            float offset = 0.0f;
        };
        std::vector<ParallaxLayer> parallaxLayers;
        
        // Stars/particles for space backgrounds
        struct Particle {
            float x, y, z;
            float brightness;
            Color color;
        };
        std::vector<Particle> particles;
        
        // Grid lines for futuristic effect
        struct GridLine {
            float z;
            float speed;
        };
        std::vector<GridLine> gridLines;
        
        mutable uint32_t m_seed = 12345;

        ProceduralBackgroundComponent() = default;
        
        ProceduralBackgroundComponent(ProceduralBgType t, int w, int h)
            : type(t), screenWidth(w), screenHeight(h) {
            initialize();
        }
        
        std::string getTypeName() const override { return "ProceduralBackgroundComponent"; }
        
        uint32_t fastRand() const {
            m_seed ^= m_seed << 13;
            m_seed ^= m_seed >> 17;
            m_seed ^= m_seed << 5;
            return m_seed;
        }
        
        float randFloat() const {
            return static_cast<float>(fastRand() % 10000) / 10000.0f;
        }
        
        void initialize() {
            m_seed = 12345 + static_cast<uint32_t>(type);
            parallaxLayers.clear();
            particles.clear();
            gridLines.clear();
            
            switch (type) {
                case ProceduralBgType::Sunrise:
                    initSunrise();
                    break;
                case ProceduralBgType::City:
                    initCity();
                    break;
                case ProceduralBgType::Space:
                    initSpace();
                    break;
                case ProceduralBgType::Futuristic:
                    initFuturistic();
                    break;
                case ProceduralBgType::Ocean:
                    initOcean();
                    break;
                case ProceduralBgType::Mountains:
                    initMountains();
                    break;
            }
        }
        
        void initSunrise() {
            primaryColor = {15, 20, 40, 255};     // Dark sky top
            secondaryColor = {80, 40, 60, 255};   // Mid gradient
            accentColor = {255, 120, 50, 255};    // Sun glow
        }
        
        void initCity() {
            primaryColor = {10, 15, 30, 255};
            secondaryColor = {30, 35, 50, 255};
            
            // Create 3 parallax city layers
            for (int l = 0; l < 3; ++l) {
                ParallaxLayer layer;
                layer.speed = 10.0f + l * 15.0f;
                
                int brightness = 20 + l * 25;
                layer.color = {
                    static_cast<unsigned char>(brightness),
                    static_cast<unsigned char>(brightness + 5),
                    static_cast<unsigned char>(brightness + 15),
                    255
                };
                
                // Generate building heights
                int numBuildings = 30 + l * 10;
                layer.heights.resize(numBuildings);
                for (int i = 0; i < numBuildings; ++i) {
                    float baseHeight = 50.0f + l * 40.0f;
                    layer.heights[i] = baseHeight + randFloat() * (80.0f + l * 30.0f);
                }
                parallaxLayers.push_back(layer);
            }
            
            // City lights (particles as windows)
            for (int i = 0; i < 200; ++i) {
                Particle p;
                p.x = randFloat() * screenWidth * 2;
                p.y = screenHeight - 50 - randFloat() * 200;
                p.z = 1.0f + randFloat() * 2.0f;
                p.brightness = 0.3f + randFloat() * 0.7f;
                p.color = (fastRand() % 3 == 0) ? 
                    Color{255, 200, 100, 255} : Color{200, 220, 255, 255};
                particles.push_back(p);
            }
        }
        
        void initSpace() {
            primaryColor = {5, 5, 15, 255};
            secondaryColor = {20, 10, 40, 255};
            accentColor = {100, 50, 150, 255};
            
            // Distant stars
            for (int i = 0; i < 300; ++i) {
                Particle p;
                p.x = randFloat() * screenWidth;
                p.y = randFloat() * screenHeight;
                p.z = 0.5f + randFloat() * 2.5f;
                p.brightness = 0.2f + randFloat() * 0.8f;
                
                // Star colors
                int colorType = fastRand() % 4;
                switch (colorType) {
                    case 0: p.color = {255, 255, 255, 255}; break;  // White
                    case 1: p.color = {200, 220, 255, 255}; break;  // Blue
                    case 2: p.color = {255, 230, 200, 255}; break;  // Yellow
                    default: p.color = {255, 200, 200, 255}; break; // Red
                }
                particles.push_back(p);
            }
        }
        
        void initFuturistic() {
            primaryColor = {5, 10, 20, 255};
            secondaryColor = {0, 40, 60, 255};
            accentColor = {0, 255, 200, 255};
            
            // Horizontal grid lines at different depths
            for (int i = 0; i < 30; ++i) {
                GridLine line;
                line.z = 0.1f + static_cast<float>(i) / 30.0f;
                line.speed = 50.0f + line.z * 100.0f;
                gridLines.push_back(line);
            }
            
            // Glowing particles
            for (int i = 0; i < 50; ++i) {
                Particle p;
                p.x = randFloat() * screenWidth;
                p.y = randFloat() * screenHeight;
                p.z = 0.5f + randFloat() * 2.0f;
                p.brightness = 0.5f + randFloat() * 0.5f;
                p.color = accentColor;
                particles.push_back(p);
            }
        }
        
        void initOcean() {
            primaryColor = {10, 30, 60, 255};
            secondaryColor = {20, 60, 100, 255};
            accentColor = {150, 200, 255, 255};
        }
        
        void initMountains() {
            primaryColor = {20, 15, 35, 255};
            secondaryColor = {40, 35, 55, 255};
            
            // Create 4 mountain layers
            for (int l = 0; l < 4; ++l) {
                ParallaxLayer layer;
                layer.speed = 5.0f + l * 10.0f;
                
                int brightness = 15 + l * 20;
                layer.color = {
                    static_cast<unsigned char>(brightness + 10),
                    static_cast<unsigned char>(brightness),
                    static_cast<unsigned char>(brightness + 20),
                    255
                };
                
                // Generate mountain profile
                int points = 50;
                layer.heights.resize(points);
                float baseY = screenHeight - 100.0f - l * 60.0f;
                float prevH = baseY;
                for (int i = 0; i < points; ++i) {
                    float variation = (randFloat() - 0.5f) * (40.0f + l * 20.0f);
                    prevH = prevH * 0.7f + (baseY + variation) * 0.3f;
                    layer.heights[i] = prevH;
                }
                parallaxLayers.push_back(layer);
            }
        }
        
        void update(float dt) {
            time += dt;
            
            // Update cycle progress (0 to 1, then loops)
            cycleProgress = std::fmod(time / cycleDuration, 1.0f);
            
            // Update parallax offsets
            for (auto& layer : parallaxLayers) {
                layer.offset += layer.speed * dt;
                float maxOffset = static_cast<float>(screenWidth);
                if (layer.offset > maxOffset) layer.offset -= maxOffset;
            }
            
            // Update particles
            for (auto& p : particles) {
                p.x -= scrollSpeed * p.z * dt;
                if (p.x < -10) p.x += screenWidth + 20;
            }
            
            // Update grid lines
            for (auto& line : gridLines) {
                line.z += line.speed * dt * 0.001f;
                if (line.z > 1.0f) line.z -= 0.9f;
            }
        }
        
        bool isRenderable() const override { return true; }
        int getRenderLayer() const override { return layer; }
        
        void render([[maybe_unused]] const TransformComponent& transform,
                   [[maybe_unused]] const RenderContext& ctx) const override {
            switch (type) {
                case ProceduralBgType::Sunrise: renderSunrise(); break;
                case ProceduralBgType::City: renderCity(); break;
                case ProceduralBgType::Space: renderSpace(); break;
                case ProceduralBgType::Futuristic: renderFuturistic(); break;
                case ProceduralBgType::Ocean: renderOcean(); break;
                case ProceduralBgType::Mountains: renderMountains(); break;
            }
        }
        
        void renderSunrise() const {
            // Day cycle phases based on cycleProgress (0-1):
            // 0.0-0.2: Dawn (dark to orange)
            // 0.2-0.4: Morning (orange to bright blue)
            // 0.4-0.6: Midday (bright blue)
            // 0.6-0.8: Sunset (blue to orange/purple)
            // 0.8-1.0: Night (purple to dark)
            
            // Calculate phase colors
            Color skyTop, skyBottom, sunColor;
            float sunY, sunAlpha;
            
            if (cycleProgress < 0.2f) {
                // Dawn: dark purple to warm orange
                float t = cycleProgress / 0.2f;
                skyTop = lerpColor({10, 10, 30, 255}, {40, 30, 60, 255}, t);
                skyBottom = lerpColor({20, 15, 40, 255}, {255, 120, 60, 255}, t);
                sunColor = {255, 150, 80, 255};
                sunY = screenHeight * (0.9f - t * 0.5f);
                sunAlpha = t;
            } else if (cycleProgress < 0.4f) {
                // Morning: orange to bright day
                float t = (cycleProgress - 0.2f) / 0.2f;
                skyTop = lerpColor({40, 30, 60, 255}, {100, 150, 220, 255}, t);
                skyBottom = lerpColor({255, 120, 60, 255}, {180, 200, 240, 255}, t);
                sunColor = lerpColor({255, 150, 80, 255}, {255, 255, 200, 255}, t);
                sunY = screenHeight * (0.4f - t * 0.15f);
                sunAlpha = 1.0f;
            } else if (cycleProgress < 0.6f) {
                // Midday: bright blue sky
                float t = (cycleProgress - 0.4f) / 0.2f;
                skyTop = {100, 150, 220, 255};
                skyBottom = {180, 200, 240, 255};
                sunColor = {255, 255, 220, 255};
                sunY = screenHeight * (0.25f + std::sin(t * 3.14159f) * 0.05f);
                sunAlpha = 1.0f;
            } else if (cycleProgress < 0.8f) {
                // Sunset: blue to orange/purple
                float t = (cycleProgress - 0.6f) / 0.2f;
                skyTop = lerpColor({100, 150, 220, 255}, {60, 40, 80, 255}, t);
                skyBottom = lerpColor({180, 200, 240, 255}, {255, 100, 50, 255}, t);
                sunColor = lerpColor({255, 255, 220, 255}, {255, 80, 30, 255}, t);
                sunY = screenHeight * (0.25f + t * 0.5f);
                sunAlpha = 1.0f - t * 0.3f;
            } else {
                // Night: purple to dark
                float t = (cycleProgress - 0.8f) / 0.2f;
                skyTop = lerpColor({60, 40, 80, 255}, {10, 10, 30, 255}, t);
                skyBottom = lerpColor({255, 100, 50, 255}, {30, 20, 50, 255}, t);
                sunColor = {200, 180, 255, 255};  // Moon color
                sunY = screenHeight * (0.75f + t * 0.1f);
                sunAlpha = t * 0.8f;
            }
            
            // Draw sky gradient
            for (int y = 0; y < screenHeight; ++y) {
                float t = static_cast<float>(y) / screenHeight;
                Color lineColor = lerpColor(skyTop, skyBottom, t);
                DrawLine(0, y, screenWidth, y, lineColor);
            }
            
            // Draw stars at night
            if (cycleProgress > 0.75f || cycleProgress < 0.15f) {
                float starAlpha = (cycleProgress > 0.75f) ? 
                    (cycleProgress - 0.75f) / 0.25f : 
                    (0.15f - cycleProgress) / 0.15f;
                
                for (const auto& p : particles) {
                    float twinkle = 0.5f + 0.5f * std::sin(time * 3.0f + p.x * 0.1f);
                    unsigned char alpha = static_cast<unsigned char>(200 * starAlpha * twinkle * p.brightness);
                    DrawPixel(static_cast<int>(p.x), static_cast<int>(p.y), {255, 255, 255, alpha});
                }
            }
            
            // Draw sun/moon
            float sunX = screenWidth * 0.65f;
            
            // Glow
            for (int r = 60; r > 0; r -= 4) {
                unsigned char alpha = static_cast<unsigned char>((60 - r) * sunAlpha * 2);
                DrawCircle(static_cast<int>(sunX), static_cast<int>(sunY), 
                          static_cast<float>(r), {sunColor.r, sunColor.g, sunColor.b, alpha});
            }
            
            // Sun/moon disc
            unsigned char discAlpha = static_cast<unsigned char>(255 * sunAlpha);
            DrawCircle(static_cast<int>(sunX), static_cast<int>(sunY), 20, 
                      {sunColor.r, sunColor.g, sunColor.b, discAlpha});
        }
        
        static Color lerpColor(Color a, Color b, float t) {
            t = std::max(0.0f, std::min(1.0f, t));
            return {
                static_cast<unsigned char>(a.r + (b.r - a.r) * t),
                static_cast<unsigned char>(a.g + (b.g - a.g) * t),
                static_cast<unsigned char>(a.b + (b.b - a.b) * t),
                static_cast<unsigned char>(a.a + (b.a - a.a) * t)
            };
        }
        
        void renderCity() const {
            // Day/night cycle for city sky
            // Night (0.0-0.3), Dawn (0.3-0.4), Day (0.4-0.6), Dusk (0.6-0.7), Night (0.7-1.0)
            Color skyTop, skyBottom;
            float windowBrightness;
            
            if (cycleProgress < 0.3f || cycleProgress > 0.7f) {
                // Night
                skyTop = {10, 10, 25, 255};
                skyBottom = {25, 20, 40, 255};
                windowBrightness = 0.8f + 0.2f * std::sin(time * 0.5f);
            } else if (cycleProgress < 0.4f) {
                // Dawn
                float t = (cycleProgress - 0.3f) / 0.1f;
                skyTop = lerpColor({10, 10, 25, 255}, {80, 100, 140, 255}, t);
                skyBottom = lerpColor({25, 20, 40, 255}, {150, 120, 100, 255}, t);
                windowBrightness = 0.8f - t * 0.6f;
            } else if (cycleProgress < 0.6f) {
                // Day
                skyTop = {80, 120, 180, 255};
                skyBottom = {140, 160, 200, 255};
                windowBrightness = 0.15f;
            } else {
                // Dusk
                float t = (cycleProgress - 0.6f) / 0.1f;
                skyTop = lerpColor({80, 120, 180, 255}, {10, 10, 25, 255}, t);
                skyBottom = lerpColor({140, 160, 200, 255}, {80, 40, 60, 255}, t);
                windowBrightness = 0.2f + t * 0.6f;
            }
            
            // Draw sky gradient
            for (int y = 0; y < screenHeight; ++y) {
                float t = static_cast<float>(y) / screenHeight;
                Color lineColor = lerpColor(skyTop, skyBottom, t);
                DrawLine(0, y, screenWidth, y, lineColor);
            }
            
            // Render building layers back to front
            for (const auto& layer : parallaxLayers) {
                int numBuildings = static_cast<int>(layer.heights.size());
                float buildingWidth = static_cast<float>(screenWidth * 2) / numBuildings;
                
                for (int i = 0; i < numBuildings; ++i) {
                    float x = i * buildingWidth - std::fmod(layer.offset, buildingWidth);
                    float h = layer.heights[i];
                    
                    if (x > -buildingWidth && x < screenWidth) {
                        DrawRectangle(
                            static_cast<int>(x), 
                            screenHeight - static_cast<int>(h),
                            static_cast<int>(buildingWidth - 2),
                            static_cast<int>(h),
                            layer.color
                        );
                    }
                }
            }
            
            // Window lights (brighter at night)
            for (const auto& p : particles) {
                float x = p.x - std::fmod(time * 20.0f * p.z, static_cast<float>(screenWidth * 2));
                if (x < -5) x += screenWidth * 2;
                
                if (x >= 0 && x < screenWidth) {
                    float flicker = 0.8f + 0.2f * std::sin(time * 5.0f + p.x);
                    unsigned char alpha = static_cast<unsigned char>(255 * p.brightness * flicker * windowBrightness);
                    DrawPixel(static_cast<int>(x), static_cast<int>(p.y), 
                             {p.color.r, p.color.g, p.color.b, alpha});
                }
            }
        }
        
        void renderSpace() const {
            // Nebula colors shift over time (different cosmic regions)
            // Phase 0.0-0.25: Purple nebula
            // Phase 0.25-0.5: Blue/cyan nebula  
            // Phase 0.5-0.75: Red/orange nebula
            // Phase 0.75-1.0: Green nebula
            Color nebulaColor, bgTop, bgBottom;
            
            if (cycleProgress < 0.25f) {
                float t = cycleProgress / 0.25f;
                nebulaColor = lerpColor({150, 50, 200, 255}, {50, 100, 200, 255}, t);
                bgTop = lerpColor({10, 5, 20, 255}, {5, 10, 25, 255}, t);
                bgBottom = lerpColor({25, 10, 50, 255}, {10, 25, 60, 255}, t);
            } else if (cycleProgress < 0.5f) {
                float t = (cycleProgress - 0.25f) / 0.25f;
                nebulaColor = lerpColor({50, 100, 200, 255}, {200, 80, 50, 255}, t);
                bgTop = lerpColor({5, 10, 25, 255}, {20, 5, 5, 255}, t);
                bgBottom = lerpColor({10, 25, 60, 255}, {50, 15, 20, 255}, t);
            } else if (cycleProgress < 0.75f) {
                float t = (cycleProgress - 0.5f) / 0.25f;
                nebulaColor = lerpColor({200, 80, 50, 255}, {50, 180, 100, 255}, t);
                bgTop = lerpColor({20, 5, 5, 255}, {5, 15, 10, 255}, t);
                bgBottom = lerpColor({50, 15, 20, 255}, {15, 40, 25, 255}, t);
            } else {
                float t = (cycleProgress - 0.75f) / 0.25f;
                nebulaColor = lerpColor({50, 180, 100, 255}, {150, 50, 200, 255}, t);
                bgTop = lerpColor({5, 15, 10, 255}, {10, 5, 20, 255}, t);
                bgBottom = lerpColor({15, 40, 25, 255}, {25, 10, 50, 255}, t);
            }
            
            // Nebula gradient background
            for (int y = 0; y < screenHeight; ++y) {
                float t = static_cast<float>(y) / screenHeight;
                float wave = 0.5f + 0.5f * std::sin(t * 3.14159f + time * 0.1f);
                
                Color lineColor = lerpColor(bgTop, bgBottom, t);
                unsigned char r = static_cast<unsigned char>(std::min(255, lineColor.r + static_cast<int>(wave * 15)));
                unsigned char g = static_cast<unsigned char>(std::min(255, lineColor.g + static_cast<int>(wave * 5)));
                unsigned char b = static_cast<unsigned char>(std::min(255, lineColor.b + static_cast<int>(wave * 25)));
                DrawLine(0, y, screenWidth, y, {r, g, b, 255});
            }
            
            // Nebula clouds
            float cloudX = screenWidth * 0.6f + std::sin(time * 0.05f) * 100;
            float cloudY = screenHeight * 0.4f;
            for (int r = 200; r > 0; r -= 10) {
                unsigned char alpha = static_cast<unsigned char>((200 - r) / 10 + 5);
                DrawCircle(static_cast<int>(cloudX), static_cast<int>(cloudY),
                          static_cast<float>(r), {nebulaColor.r, nebulaColor.g, nebulaColor.b, alpha});
            }
            
            // Second nebula cloud
            float cloud2X = screenWidth * 0.3f + std::cos(time * 0.03f) * 80;
            float cloud2Y = screenHeight * 0.6f;
            for (int r = 150; r > 0; r -= 10) {
                unsigned char alpha = static_cast<unsigned char>((150 - r) / 10 + 3);
                Color altColor = {
                    static_cast<unsigned char>(255 - nebulaColor.r),
                    static_cast<unsigned char>(std::min(255, nebulaColor.g + 50)),
                    nebulaColor.b, alpha
                };
                DrawCircle(static_cast<int>(cloud2X), static_cast<int>(cloud2Y),
                          static_cast<float>(r), altColor);
            }
            
            // Stars
            for (const auto& p : particles) {
                float twinkle = 0.6f + 0.4f * std::sin(time * 3.0f + p.x * 0.1f);
                unsigned char alpha = static_cast<unsigned char>(255 * p.brightness * twinkle);
                
                int x = static_cast<int>(p.x);
                int y = static_cast<int>(p.y);
                Color c = {p.color.r, p.color.g, p.color.b, alpha};
                
                DrawPixel(x, y, c);
                if (p.z > 2.0f) {
                    DrawPixel(x + 1, y, {c.r, c.g, c.b, static_cast<unsigned char>(alpha / 2)});
                    DrawPixel(x - 1, y, {c.r, c.g, c.b, static_cast<unsigned char>(alpha / 2)});
                }
            }
        }
        
        void renderFuturistic() const {
            // Dark gradient
            for (int y = 0; y < screenHeight; ++y) {
                float t = static_cast<float>(y) / screenHeight;
                DrawLine(0, y, screenWidth, y, {
                    static_cast<unsigned char>(primaryColor.r),
                    static_cast<unsigned char>(primaryColor.g + t * 20),
                    static_cast<unsigned char>(primaryColor.b + t * 30),
                    255
                });
            }
            
            // Perspective grid floor
            int horizon = screenHeight / 2;
            
            // Horizontal lines
            for (const auto& line : gridLines) {
                float perspY = horizon + line.z * (screenHeight - horizon);
                unsigned char alpha = static_cast<unsigned char>(50 + line.z * 150);
                DrawLine(0, static_cast<int>(perspY), screenWidth, static_cast<int>(perspY),
                        {accentColor.r, accentColor.g, accentColor.b, alpha});
            }
            
            // Vertical lines with perspective
            for (int i = 0; i < 20; ++i) {
                float t = static_cast<float>(i) / 20.0f;
                float offset = std::fmod(time * 0.1f, 0.05f);
                float x = (t + offset) * 2.0f - 1.0f;
                
                int topX = screenWidth / 2 + static_cast<int>(x * 50);
                int bottomX = screenWidth / 2 + static_cast<int>(x * screenWidth);
                
                DrawLine(topX, horizon, bottomX, screenHeight, 
                        {accentColor.r, accentColor.g, accentColor.b, 60});
            }
            
            // Floating particles
            for (const auto& p : particles) {
                float pulse = 0.5f + 0.5f * std::sin(time * 2.0f + p.x * 0.05f);
                int size = 2 + static_cast<int>(p.z);
                unsigned char alpha = static_cast<unsigned char>(150 * p.brightness * pulse);
                
                DrawCircle(static_cast<int>(p.x), static_cast<int>(p.y), 
                          static_cast<float>(size), {accentColor.r, accentColor.g, accentColor.b, alpha});
            }
        }
        
        void renderOcean() const {
            // Day/night ocean sky
            // Dawn (0.0-0.2), Day (0.2-0.7), Dusk (0.7-0.9), Night (0.9-1.0)
            Color skyTop, skyBottom, oceanDeep, oceanSurf;
            
            if (cycleProgress < 0.1f) {
                // Late night to dawn
                float t = cycleProgress / 0.1f;
                skyTop = lerpColor({10, 15, 30, 255}, {60, 40, 80, 255}, t);
                skyBottom = lerpColor({20, 25, 50, 255}, {180, 100, 80, 255}, t);
                oceanDeep = lerpColor({10, 30, 50, 255}, {20, 50, 80, 255}, t);
                oceanSurf = lerpColor({30, 50, 80, 255}, {50, 80, 120, 255}, t);
            } else if (cycleProgress < 0.2f) {
                // Dawn
                float t = (cycleProgress - 0.1f) / 0.1f;
                skyTop = lerpColor({60, 40, 80, 255}, {80, 130, 200, 255}, t);
                skyBottom = lerpColor({180, 100, 80, 255}, {150, 180, 220, 255}, t);
                oceanDeep = lerpColor({20, 50, 80, 255}, {30, 80, 130, 255}, t);
                oceanSurf = lerpColor({50, 80, 120, 255}, {80, 140, 180, 255}, t);
            } else if (cycleProgress < 0.7f) {
                // Day
                skyTop = {80, 150, 220, 255};
                skyBottom = {160, 200, 240, 255};
                oceanDeep = {30, 80, 130, 255};
                oceanSurf = {80, 150, 200, 255};
            } else if (cycleProgress < 0.9f) {
                // Dusk
                float t = (cycleProgress - 0.7f) / 0.2f;
                skyTop = lerpColor({80, 150, 220, 255}, {80, 40, 60, 255}, t);
                skyBottom = lerpColor({160, 200, 240, 255}, {200, 100, 60, 255}, t);
                oceanDeep = lerpColor({30, 80, 130, 255}, {40, 50, 80, 255}, t);
                oceanSurf = lerpColor({80, 150, 200, 255}, {120, 80, 100, 255}, t);
            } else {
                // Night
                float t = (cycleProgress - 0.9f) / 0.1f;
                skyTop = lerpColor({80, 40, 60, 255}, {10, 15, 30, 255}, t);
                skyBottom = lerpColor({200, 100, 60, 255}, {20, 25, 50, 255}, t);
                oceanDeep = lerpColor({40, 50, 80, 255}, {10, 30, 50, 255}, t);
                oceanSurf = lerpColor({120, 80, 100, 255}, {30, 50, 80, 255}, t);
            }
            
            // Sky
            for (int y = 0; y < screenHeight / 2; ++y) {
                float t = static_cast<float>(y) / (screenHeight / 2);
                Color lineColor = lerpColor(skyTop, skyBottom, t);
                DrawLine(0, y, screenWidth, y, lineColor);
            }
            
            // Ocean waves
            for (int y = screenHeight / 2; y < screenHeight; ++y) {
                float t = static_cast<float>(y - screenHeight / 2) / (screenHeight / 2);
                float wave = std::sin(static_cast<float>(y) * 0.1f + time * 2.0f) * 
                            std::sin(static_cast<float>(y) * 0.03f + time);
                
                Color oceanColor = lerpColor(oceanSurf, oceanDeep, t);
                unsigned char r = static_cast<unsigned char>(std::min(255.0f, oceanColor.r + wave * 5));
                unsigned char g = static_cast<unsigned char>(std::min(255.0f, oceanColor.g + wave * 10));
                unsigned char b = static_cast<unsigned char>(std::min(255.0f, oceanColor.b + wave * 15));
                
                DrawLine(0, y, screenWidth, y, {r, g, b, 255});
            }
            
            // Wave lines (more visible during day)
            float waveAlphaMult = (cycleProgress > 0.2f && cycleProgress < 0.7f) ? 1.0f : 0.5f;
            for (int w = 0; w < 8; ++w) {
                float waveY = screenHeight / 2 + 30 + w * 40;
                float amplitude = 5.0f - w * 0.5f;
                
                for (int x = 0; x < screenWidth; x += 2) {
                    float y = waveY + amplitude * std::sin(x * 0.02f + time * 1.5f - w * 0.5f);
                    unsigned char alpha = static_cast<unsigned char>((150 - w * 15) * waveAlphaMult);
                    DrawPixel(x, static_cast<int>(y), {accentColor.r, accentColor.g, accentColor.b, alpha});
                }
            }
        }
        
        void renderMountains() const {
            // Day/night mountain sky
            Color skyTop, skyBottom;
            float mountainBrightness;
            
            if (cycleProgress < 0.15f) {
                // Dawn
                float t = cycleProgress / 0.15f;
                skyTop = lerpColor({20, 15, 40, 255}, {200, 100, 80, 255}, t);
                skyBottom = lerpColor({40, 30, 60, 255}, {255, 180, 120, 255}, t);
                mountainBrightness = 0.3f + t * 0.4f;
            } else if (cycleProgress < 0.4f) {
                // Morning
                float t = (cycleProgress - 0.15f) / 0.25f;
                skyTop = lerpColor({200, 100, 80, 255}, {100, 160, 220, 255}, t);
                skyBottom = lerpColor({255, 180, 120, 255}, {180, 200, 240, 255}, t);
                mountainBrightness = 0.7f + t * 0.3f;
            } else if (cycleProgress < 0.65f) {
                // Midday
                skyTop = {100, 160, 220, 255};
                skyBottom = {180, 200, 240, 255};
                mountainBrightness = 1.0f;
            } else if (cycleProgress < 0.85f) {
                // Sunset
                float t = (cycleProgress - 0.65f) / 0.2f;
                skyTop = lerpColor({100, 160, 220, 255}, {80, 40, 80, 255}, t);
                skyBottom = lerpColor({180, 200, 240, 255}, {200, 100, 60, 255}, t);
                mountainBrightness = 1.0f - t * 0.4f;
            } else {
                // Night
                float t = (cycleProgress - 0.85f) / 0.15f;
                skyTop = lerpColor({80, 40, 80, 255}, {20, 15, 40, 255}, t);
                skyBottom = lerpColor({200, 100, 60, 255}, {40, 30, 60, 255}, t);
                mountainBrightness = 0.6f - t * 0.3f;
            }
            
            // Sky gradient
            for (int y = 0; y < screenHeight; ++y) {
                float t = static_cast<float>(y) / screenHeight;
                Color lineColor = lerpColor(skyTop, skyBottom, t);
                DrawLine(0, y, screenWidth, y, lineColor);
            }
            
            // Mountain layers with lighting
            for (const auto& layer : parallaxLayers) {
                int numPoints = static_cast<int>(layer.heights.size());
                float segmentWidth = static_cast<float>(screenWidth * 2) / numPoints;
                
                Color litColor = {
                    static_cast<unsigned char>(layer.color.r * mountainBrightness),
                    static_cast<unsigned char>(layer.color.g * mountainBrightness),
                    static_cast<unsigned char>(layer.color.b * mountainBrightness),
                    layer.color.a
                };
                
                for (int i = 0; i < numPoints - 1; ++i) {
                    float x1 = i * segmentWidth - std::fmod(layer.offset, segmentWidth * numPoints);
                    float x2 = (i + 1) * segmentWidth - std::fmod(layer.offset, segmentWidth * numPoints);
                    
                    if (x2 < 0 || x1 > screenWidth) continue;
                    
                    float y1 = layer.heights[i];
                    float y2 = layer.heights[(i + 1) % numPoints];
                    
                    DrawTriangle(
                        {x1, y1}, {x2, y2}, {x2, static_cast<float>(screenHeight)}, litColor
                    );
                    DrawTriangle(
                        {x1, y1}, {x2, static_cast<float>(screenHeight)}, 
                        {x1, static_cast<float>(screenHeight)}, litColor
                    );
                }
            }
        }
        
        void setScreenSize(int w, int h) {
            screenWidth = w;
            screenHeight = h;
            initialize();
        }
        
        void setType(ProceduralBgType t) {
            type = t;
            initialize();
        }
    };

} // namespace rtype::ecs
