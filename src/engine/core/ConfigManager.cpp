/*
** R-Type Engine - ConfigManager Implementation
** JSON-based configuration system with hot-reload support
*/

#include "ConfigManager.hpp"
#include <iostream>
#include <shared_mutex>
#include <sstream>

namespace rtype::core {

    ConfigManager& ConfigManager::instance() {
        static ConfigManager instance;
        return instance;
    }

    bool ConfigManager::load(const std::string& path) {
        return load(std::filesystem::path(path));
    }

    bool ConfigManager::load(const std::filesystem::path& path) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        
        try {
            if (!std::filesystem::exists(path)) {
                std::cerr << "[ConfigManager] File not found: " << path << std::endl;
                return false;
            }

            std::ifstream file(path);
            if (!file.is_open()) {
                std::cerr << "[ConfigManager] Failed to open: " << path << std::endl;
                return false;
            }

            file >> m_config;
            m_configPath = path;
            m_lastModified = std::filesystem::last_write_time(path);

            std::cout << "[ConfigManager] Loaded configuration from: " << path << std::endl;
            return true;

        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "[ConfigManager] JSON parse error: " << e.what() << std::endl;
            return false;
        } catch (const std::exception& e) {
            std::cerr << "[ConfigManager] Load error: " << e.what() << std::endl;
            return false;
        }
    }

    bool ConfigManager::save(const std::string& path) {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        
        std::filesystem::path savePath = path.empty() ? m_configPath : std::filesystem::path(path);
        
        if (savePath.empty()) {
            std::cerr << "[ConfigManager] No save path specified" << std::endl;
            return false;
        }

        try {
            std::ofstream file(savePath);
            if (!file.is_open()) {
                std::cerr << "[ConfigManager] Failed to open for writing: " << savePath << std::endl;
                return false;
            }

            file << m_config.dump(4); // Pretty print with 4-space indent
            
            std::cout << "[ConfigManager] Saved configuration to: " << savePath << std::endl;
            return true;

        } catch (const std::exception& e) {
            std::cerr << "[ConfigManager] Save error: " << e.what() << std::endl;
            return false;
        }
    }

    bool ConfigManager::reload() {
        if (!hasChanged()) {
            return false;
        }

        std::filesystem::path currentPath;
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            currentPath = m_configPath;
        }

        if (load(currentPath)) {
            notifyChange("*");
            return true;
        }
        
        return false;
    }

    bool ConfigManager::hasChanged() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        
        if (m_configPath.empty()) {
            return false;
        }

        try {
            auto currentModTime = std::filesystem::last_write_time(m_configPath);
            return currentModTime > m_lastModified;
        } catch (const std::exception&) {
            return false;
        }
    }

    bool ConfigManager::has(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return navigateToKey(key) != nullptr;
    }

    bool ConfigManager::remove(const std::string& key) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        
        std::vector<std::string> parts = splitKey(key);
        if (parts.empty()) {
            return false;
        }

        nlohmann::json* current = &m_config;
        for (size_t i = 0; i < parts.size() - 1; ++i) {
            if (!current->contains(parts[i])) {
                return false;
            }
            current = &(*current)[parts[i]];
        }

        if (current->contains(parts.back())) {
            current->erase(parts.back());
            return true;
        }
        
        return false;
    }

    std::size_t ConfigManager::onChanged(const std::string& key, ChangeCallback callback) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        
        std::size_t id = m_nextCallbackId++;
        m_callbacks.push_back({id, key, std::move(callback)});
        
        return id;
    }

    void ConfigManager::removeCallback(std::size_t callbackId) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        
        auto it = std::find_if(m_callbacks.begin(), m_callbacks.end(),
            [callbackId](const CallbackEntry& entry) {
                return entry.id == callbackId;
            });
        
        if (it != m_callbacks.end()) {
            m_callbacks.erase(it);
        }
    }

    std::string ConfigManager::getConfigPath() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_configPath.string();
    }

    const nlohmann::json* ConfigManager::navigateToKey(const std::string& key) const {
        std::vector<std::string> parts = splitKey(key);
        
        const nlohmann::json* current = &m_config;
        for (const auto& part : parts) {
            if (!current->contains(part)) {
                return nullptr;
            }
            current = &(*current)[part];
        }
        
        return current;
    }

    std::vector<std::string> ConfigManager::splitKey(const std::string& key) const {
        std::vector<std::string> parts;
        std::stringstream ss(key);
        std::string part;
        
        while (std::getline(ss, part, '.')) {
            if (!part.empty()) {
                parts.push_back(part);
            }
        }
        
        return parts;
    }

    void ConfigManager::notifyChange(const std::string& key) {
        std::vector<CallbackEntry> callbacksCopy;
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            callbacksCopy = m_callbacks;
        }
        
        for (const auto& entry : callbacksCopy) {
            // Match if pattern is "*" (all) or exact match or prefix match
            if (entry.keyPattern == "*" || 
                entry.keyPattern == key ||
                key.find(entry.keyPattern) == 0) {
                entry.callback(key);
            }
        }
    }

} // namespace rtype::core
