/*
** R-Type Engine - AssetManager Implementation
** Central asset management with reference counting and hot-reload
*/

#include "AssetManager.hpp"
#include <iostream>

namespace rtype::core {

    AssetManager& AssetManager::instance() {
        static AssetManager instance;
        return instance;
    }

    std::size_t AssetManager::garbageCollect() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        
        std::vector<std::string> toRemove;
        
        for (auto& [key, entry] : m_cache) {
            auto metaIt = m_metadata.find(key);
            if (metaIt != m_metadata.end() && metaIt->second.refCount == 0) {
                // Find the loader and call unload
                auto loaderIt = m_loaders.find(entry.typeIdx);
                if (loaderIt != m_loaders.end()) {
                    loaderIt->second->unload(entry.asset);
                }
                toRemove.push_back(key);
            }
        }

        for (const auto& key : toRemove) {
            m_cache.erase(key);
            m_metadata.erase(key);
        }

        return toRemove.size();
    }

    std::size_t AssetManager::reloadChanged() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        
        std::size_t reloaded = 0;

        for (auto& [key, meta] : m_metadata) {
            if (!meta.isLoaded || meta.path.empty()) {
                continue;
            }

            try {
                if (!std::filesystem::exists(meta.path)) {
                    continue;
                }

                auto currentModTime = std::filesystem::last_write_time(meta.path);
                if (currentModTime > meta.lastModified) {
                    // File has changed, reload it
                    auto it = m_cache.find(key);
                    if (it == m_cache.end()) {
                        continue;
                    }

                    auto loaderIt = m_loaders.find(it->second.typeIdx);
                    if (loaderIt == m_loaders.end()) {
                        continue;
                    }

                    // Unload old asset
                    loaderIt->second->unload(it->second.asset);

                    // Load new asset
                    auto newAsset = loaderIt->second->load(meta.path);
                    if (newAsset) {
                        it->second.asset = newAsset;
                        meta.lastModified = currentModTime;
                        meta.sizeBytes = std::filesystem::file_size(meta.path);
                        ++reloaded;
                        
                        std::cout << "[AssetManager] Reloaded: " << meta.path << std::endl;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "[AssetManager] Error checking " << meta.path 
                          << ": " << e.what() << std::endl;
            }
        }

        return reloaded;
    }

    std::size_t AssetManager::getLoadedAssetCount() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_cache.size();
    }

    std::size_t AssetManager::getTotalMemoryUsage() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        
        std::size_t total = 0;
        for (const auto& [key, meta] : m_metadata) {
            total += meta.sizeBytes;
        }
        return total;
    }

    void AssetManager::clearAll() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        
        // Unload all assets
        for (auto& [key, entry] : m_cache) {
            auto loaderIt = m_loaders.find(entry.typeIdx);
            if (loaderIt != m_loaders.end()) {
                loaderIt->second->unload(entry.asset);
            }
        }

        m_cache.clear();
        m_metadata.clear();
    }

    std::vector<std::string> AssetManager::getLoadedAssetPaths() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        
        std::vector<std::string> paths;
        paths.reserve(m_metadata.size());
        
        for (const auto& [key, meta] : m_metadata) {
            if (meta.isLoaded && !meta.path.empty()) {
                paths.push_back(meta.path);
            }
        }
        
        return paths;
    }

} // namespace rtype::core
