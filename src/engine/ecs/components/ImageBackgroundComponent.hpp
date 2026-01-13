/*
** R-Type ECS - ImageBackgroundComponent
** Static image background with transparency support
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include "engine/graphics/IRenderable.hpp"
#include "engine/ecs/components/TransformComponent.hpp"

#include <raylib.h>
#include <string>

namespace rtype::ecs {

    struct ImageBackgroundComponent : public IComponent, public IRenderable {
        std::string imagePath;
        int screenWidth = 1280;
        int screenHeight = 720;
        int layer = -100;
        
        unsigned char tintR = 255;
        unsigned char tintG = 255;
        unsigned char tintB = 255;
        unsigned char tintA = 255;
        
        Texture2D texture = {};
        bool textureLoaded = false;
        bool useExternalTexture = false;
        
        ImageBackgroundComponent() = default;
        
        ImageBackgroundComponent(int width, int height)
            : screenWidth(width), screenHeight(height) {}
        
        ImageBackgroundComponent(const std::string& path, int width, int height)
            : imagePath(path), screenWidth(width), screenHeight(height) {
            loadImage(path);
        }
        
        ~ImageBackgroundComponent() { unloadTexture(); }
        
        ImageBackgroundComponent(ImageBackgroundComponent&& other) noexcept
            : imagePath(std::move(other.imagePath))
            , screenWidth(other.screenWidth)
            , screenHeight(other.screenHeight)
            , layer(other.layer)
            , tintR(other.tintR)
            , tintG(other.tintG)
            , tintB(other.tintB)
            , tintA(other.tintA)
            , texture(other.texture)
            , textureLoaded(other.textureLoaded)
            , useExternalTexture(other.useExternalTexture) {
            other.textureLoaded = false;
            other.useExternalTexture = true;
        }
        
        ImageBackgroundComponent& operator=(ImageBackgroundComponent&& other) noexcept {
            if (this != &other) {
                unloadTexture();
                imagePath = std::move(other.imagePath);
                screenWidth = other.screenWidth;
                screenHeight = other.screenHeight;
                layer = other.layer;
                tintR = other.tintR;
                tintG = other.tintG;
                tintB = other.tintB;
                tintA = other.tintA;
                texture = other.texture;
                textureLoaded = other.textureLoaded;
                useExternalTexture = other.useExternalTexture;
                other.textureLoaded = false;
                other.useExternalTexture = true;
            }
            return *this;
        }
        
        ImageBackgroundComponent(const ImageBackgroundComponent&) = delete;
        ImageBackgroundComponent& operator=(const ImageBackgroundComponent&) = delete;
        
        std::string getTypeName() const override { return "ImageBackgroundComponent"; }
        
        bool loadImage(const std::string& path) {
            unloadTexture();
            if (path.empty()) return false;
            
            texture = LoadTexture(path.c_str());
            if (texture.id != 0) {
                textureLoaded = true;
                useExternalTexture = false;
                imagePath = path;
                return true;
            }
            return false;
        }
        
        void setTexture(const Texture2D& tex) {
            unloadTexture();
            texture = tex;
            textureLoaded = (tex.id != 0);
            useExternalTexture = true;
        }
        
        void unloadTexture() {
            if (textureLoaded && !useExternalTexture && texture.id != 0) {
                UnloadTexture(texture);
            }
            textureLoaded = false;
            texture = {};
        }
        
        bool hasValidTexture() const { return textureLoaded && texture.id != 0; }
        
        void setScreenSize(int width, int height) {
            screenWidth = width;
            screenHeight = height;
        }
        
        void setTint(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
            tintR = r; tintG = g; tintB = b; tintA = a;
        }
        
        void setAlpha(unsigned char alpha) { tintA = alpha; }
        Color getTint() const { return {tintR, tintG, tintB, tintA}; }
        
        bool isRenderable() const override { return textureLoaded && texture.id != 0; }
        int getRenderLayer() const override { return layer; }
        
        void render([[maybe_unused]] const TransformComponent& transform, 
                   [[maybe_unused]] const RenderContext& ctx) const override {
            if (!textureLoaded || texture.id == 0) return;
            
            Rectangle source = {0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height)};
            
            float scaleX = static_cast<float>(screenWidth) / texture.width;
            float scaleY = static_cast<float>(screenHeight) / texture.height;
            float scale = std::max(scaleX, scaleY);
            
            float destWidth = texture.width * scale;
            float destHeight = texture.height * scale;
            float destX = (screenWidth - destWidth) / 2.0f;
            float destY = (screenHeight - destHeight) / 2.0f;
            
            Rectangle dest = {destX, destY, destWidth, destHeight};
            DrawTexturePro(texture, source, dest, {0, 0}, 0.0f, getTint());
        }
    };

} // namespace rtype::ecs
