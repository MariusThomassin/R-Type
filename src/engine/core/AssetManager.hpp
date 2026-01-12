/*
** R-Type Engine - AssetManager
** Central asset management with reference counting and hot-reload
*/

#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace rtype::core {

    /**
     * @brief Asset metadata for tracking loaded assets
     */
    struct AssetMetadata {
        std::string path;
        std::filesystem::file_time_type lastModified;
        std::size_t refCount = 0;
        std::size_t sizeBytes = 0;
        bool isLoaded = false;
    };

    /**
     * @brief Base interface for asset loaders
     */
    class IAssetLoader {
    public:
        virtual ~IAssetLoader() = default;
        virtual std::shared_ptr<void> load(const std::string& path) = 0;
        virtual void unload(std::shared_ptr<void> asset) = 0;
        virtual std::type_index getType() const = 0;
    };

    /**
     * @brief Typed asset loader template
     */
    template <typename T>
    class AssetLoader : public IAssetLoader {
    public:
        using LoadFunc = std::function<std::shared_ptr<T>(const std::string&)>;
        using UnloadFunc = std::function<void(std::shared_ptr<T>)>;

        AssetLoader(LoadFunc loadFunc, UnloadFunc unloadFunc = nullptr)
            : m_loadFunc(std::move(loadFunc))
            , m_unloadFunc(std::move(unloadFunc)) {}

        std::shared_ptr<void> load(const std::string& path) override {
            return m_loadFunc(path);
        }

        void unload(std::shared_ptr<void> asset) override {
            if (m_unloadFunc && asset) {
                m_unloadFunc(std::static_pointer_cast<T>(asset));
            }
        }

        std::type_index getType() const override {
            return std::type_index(typeid(T));
        }

    private:
        LoadFunc m_loadFunc;
        UnloadFunc m_unloadFunc;
    };

    /**
     * @brief Central asset manager with reference counting and hot-reload
     * 
     * Features:
     * - Type-safe asset loading and caching
     * - Automatic reference counting
     * - Hot-reload support for development
     * - Memory tracking
     * - Custom asset loaders
     * 
     * Usage:
     * ```cpp
     * // Register a texture loader
     * AssetManager::instance().registerLoader<Texture2D>(
     *     [](const std::string& path) {
     *         return std::make_shared<Texture2D>(LoadTexture(path.c_str()));
     *     },
     *     [](std::shared_ptr<Texture2D> tex) {
     *         UnloadTexture(*tex);
     *     }
     * );
     * 
     * // Load an asset
     * auto tex = AssetManager::instance().load<Texture2D>("assets/player.png");
     * 
     * // Asset is automatically cached and reference counted
     * ```
     */
    class AssetManager {
    public:
        /**
         * @brief Get the singleton instance
         */
        static AssetManager& instance();

        /**
         * @brief Register an asset loader for a type
         * @tparam T Asset type
         * @param loadFunc Function to load the asset
         * @param unloadFunc Optional function to unload (for cleanup)
         */
        template <typename T>
        void registerLoader(
            std::function<std::shared_ptr<T>(const std::string&)> loadFunc,
            std::function<void(std::shared_ptr<T>)> unloadFunc = nullptr)
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            std::type_index typeIdx(typeid(T));
            m_loaders[typeIdx] = std::make_unique<AssetLoader<T>>(
                std::move(loadFunc), std::move(unloadFunc));
        }

        /**
         * @brief Load an asset (or return cached version)
         * @tparam T Asset type
         * @param path Path to the asset file
         * @return Shared pointer to the asset
         */
        template <typename T>
        std::shared_ptr<T> load(const std::string& path) {
            std::type_index typeIdx(typeid(T));
            std::string cacheKey = makeCacheKey(typeIdx, path);

            // Check cache first (read lock)
            {
                std::shared_lock<std::shared_mutex> lock(m_mutex);
                auto it = m_cache.find(cacheKey);
                if (it != m_cache.end() && it->second.asset) {
                    ++m_metadata[cacheKey].refCount;
                    return std::static_pointer_cast<T>(it->second.asset);
                }
            }

            // Load asset (write lock)
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            
            // Double-check after acquiring write lock
            auto it = m_cache.find(cacheKey);
            if (it != m_cache.end() && it->second.asset) {
                ++m_metadata[cacheKey].refCount;
                return std::static_pointer_cast<T>(it->second.asset);
            }

            // Find loader
            auto loaderIt = m_loaders.find(typeIdx);
            if (loaderIt == m_loaders.end()) {
                throw std::runtime_error("No loader registered for asset type");
            }

            // Load the asset
            auto asset = loaderIt->second->load(path);
            if (!asset) {
                throw std::runtime_error("Failed to load asset: " + path);
            }

            // Cache it
            CacheEntry entry;
            entry.asset = asset;
            entry.typeIdx = typeIdx;
            m_cache[cacheKey] = std::move(entry);

            // Track metadata
            AssetMetadata& meta = m_metadata[cacheKey];
            meta.path = path;
            meta.isLoaded = true;
            meta.refCount = 1;
            
            if (std::filesystem::exists(path)) {
                meta.lastModified = std::filesystem::last_write_time(path);
                meta.sizeBytes = std::filesystem::file_size(path);
            }

            return std::static_pointer_cast<T>(asset);
        }

        /**
         * @brief Unload an asset by path
         * @tparam T Asset type
         * @param path Path to the asset
         */
        template <typename T>
        void unload(const std::string& path) {
            std::type_index typeIdx(typeid(T));
            std::string cacheKey = makeCacheKey(typeIdx, path);

            std::unique_lock<std::shared_mutex> lock(m_mutex);
            
            auto it = m_cache.find(cacheKey);
            if (it == m_cache.end()) {
                return;
            }

            auto& meta = m_metadata[cacheKey];
            if (meta.refCount > 0) {
                --meta.refCount;
            }

            // Don't actually unload if still referenced
            // This allows the asset to be garbage collected when all refs are gone
        }

        /**
         * @brief Force unload all unreferenced assets
         * @return Number of assets unloaded
         */
        std::size_t garbageCollect();

        /**
         * @brief Reload assets that have changed on disk
         * @return Number of assets reloaded
         */
        std::size_t reloadChanged();

        /**
         * @brief Get metadata for an asset
         * @param path Asset path
         * @return Metadata or nullopt if not loaded
         */
        template <typename T>
        std::optional<AssetMetadata> getMetadata(const std::string& path) const {
            std::type_index typeIdx(typeid(T));
            std::string cacheKey = makeCacheKey(typeIdx, path);

            std::shared_lock<std::shared_mutex> lock(m_mutex);
            auto it = m_metadata.find(cacheKey);
            if (it != m_metadata.end()) {
                return it->second;
            }
            return std::nullopt;
        }

        /**
         * @brief Get total number of loaded assets
         */
        std::size_t getLoadedAssetCount() const;

        /**
         * @brief Get total memory used by loaded assets (approximate)
         */
        std::size_t getTotalMemoryUsage() const;

        /**
         * @brief Clear all cached assets
         */
        void clearAll();

        /**
         * @brief Get all loaded asset paths
         */
        std::vector<std::string> getLoadedAssetPaths() const;

        // Singleton - delete copy/move
        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;

    private:
        AssetManager() = default;
        ~AssetManager() = default;

        std::string makeCacheKey(std::type_index type, const std::string& path) const {
            return std::string(type.name()) + "::" + path;
        }

        struct CacheEntry {
            std::shared_ptr<void> asset;
            std::type_index typeIdx = std::type_index(typeid(void));
        };

        mutable std::shared_mutex m_mutex;
        std::unordered_map<std::type_index, std::unique_ptr<IAssetLoader>> m_loaders;
        std::unordered_map<std::string, CacheEntry> m_cache;
        std::unordered_map<std::string, AssetMetadata> m_metadata;
    };

    // Convenience macro
    #define ASSETS rtype::core::AssetManager::instance()

} // namespace rtype::core
