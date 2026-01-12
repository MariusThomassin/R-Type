/*
** R-Type Engine - ConfigManager
** JSON-based configuration system with hot-reload support
*/

#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <functional>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace rtype::core {

    /**
     * @brief Type-safe configuration manager with hot-reload support
     *
     * Features:
     * - JSON-based configuration files
     * - Type-safe value retrieval with defaults
     * - Hot-reload detection
     * - Change callbacks
     * - Nested key access (e.g., "video.resolution.width")
     */
    class ConfigManager {
    public:
        using ConfigValue = std::variant<bool, int, float, double, std::string,
                                         std::vector<int>, std::vector<float>, std::vector<std::string>>;
        using ChangeCallback = std::function<void(const std::string& key)>;

        /**
         * @brief Get the singleton instance
         */
        static ConfigManager& instance();

        /**
         * @brief Load configuration from a JSON file
         * @param path Path to the configuration file
         * @return true if loaded successfully
         */
        bool load(const std::string& path);

        /**
         * @brief Load configuration from a JSON file (filesystem path)
         * @param path Path to the configuration file
         * @return true if loaded successfully
         */
        bool load(const std::filesystem::path& path);

        /**
         * @brief Save current configuration to file
         * @param path Path to save to (uses loaded path if empty)
         * @return true if saved successfully
         */
        bool save(const std::string& path = "");

        /**
         * @brief Reload configuration from disk if file changed
         * @return true if reloaded (file was modified)
         */
        bool reload();

        /**
         * @brief Check if the config file has been modified on disk
         * @return true if file was modified since last load
         */
        bool hasChanged() const;

        /**
         * @brief Get a value from the configuration
         * @tparam T The value type (bool, int, float, double, std::string)
         * @param key The key path (e.g., "video.resolution.width")
         * @param defaultValue Value to return if key not found
         * @return The value or default
         */
        template <typename T>
        T get(const std::string& key, const T& defaultValue = T{}) const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            
            try {
                const nlohmann::json* node = navigateToKey(key);
                if (node && !node->is_null()) {
                    return node->get<T>();
                }
            } catch (const nlohmann::json::exception&) {
                // Type mismatch or missing key
            }
            
            return defaultValue;
        }

        /**
         * @brief Get a value as optional (no default)
         * @tparam T The value type
         * @param key The key path
         * @return optional containing value or nullopt
         */
        template <typename T>
        std::optional<T> getOptional(const std::string& key) const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            
            try {
                const nlohmann::json* node = navigateToKey(key);
                if (node && !node->is_null()) {
                    return node->get<T>();
                }
            } catch (const nlohmann::json::exception&) {
                // Type mismatch or missing key
            }
            
            return std::nullopt;
        }

        /**
         * @brief Set a value in the configuration
         * @tparam T The value type
         * @param key The key path (e.g., "video.resolution.width")
         * @param value The value to set
         */
        template <typename T>
        void set(const std::string& key, const T& value) {
            {
                std::unique_lock<std::shared_mutex> lock(m_mutex);
                setValueAtKey(key, value);
            }
            
            // Notify listeners
            notifyChange(key);
        }

        /**
         * @brief Check if a key exists
         * @param key The key path
         * @return true if key exists
         */
        bool has(const std::string& key) const;

        /**
         * @brief Remove a key from the configuration
         * @param key The key path
         * @return true if key was removed
         */
        bool remove(const std::string& key);

        /**
         * @brief Register a callback for configuration changes
         * @param key The key to watch (or "*" for all changes)
         * @param callback Function to call when value changes
         * @return Callback ID for unregistering
         */
        std::size_t onChanged(const std::string& key, ChangeCallback callback);

        /**
         * @brief Unregister a change callback
         * @param callbackId The ID returned from onChanged
         */
        void removeCallback(std::size_t callbackId);

        /**
         * @brief Get the loaded configuration file path
         */
        std::string getConfigPath() const;

        /**
         * @brief Get the raw JSON object (for advanced use)
         */
        const nlohmann::json& getRawJson() const { return m_config; }

        // Delete copy/move for singleton
        ConfigManager(const ConfigManager&) = delete;
        ConfigManager& operator=(const ConfigManager&) = delete;

    private:
        ConfigManager() = default;
        ~ConfigManager() = default;

        /**
         * @brief Navigate to a nested key in the JSON
         * @param key Dot-separated key path
         * @return Pointer to the JSON node or nullptr
         */
        const nlohmann::json* navigateToKey(const std::string& key) const;

        /**
         * @brief Set a value at a nested key path, creating intermediate objects
         */
        template <typename T>
        void setValueAtKey(const std::string& key, const T& value) {
            std::vector<std::string> parts = splitKey(key);
            
            nlohmann::json* current = &m_config;
            for (size_t i = 0; i < parts.size() - 1; ++i) {
                if (!current->contains(parts[i]) || !(*current)[parts[i]].is_object()) {
                    (*current)[parts[i]] = nlohmann::json::object();
                }
                current = &(*current)[parts[i]];
            }
            
            (*current)[parts.back()] = value;
        }

        /**
         * @brief Split a key path by dots
         */
        std::vector<std::string> splitKey(const std::string& key) const;

        /**
         * @brief Notify change listeners
         */
        void notifyChange(const std::string& key);

        // Configuration data
        nlohmann::json m_config;
        
        // File tracking
        std::filesystem::path m_configPath;
        std::filesystem::file_time_type m_lastModified;

        // Thread safety
        mutable std::shared_mutex m_mutex;

        // Change callbacks
        struct CallbackEntry {
            std::size_t id;
            std::string keyPattern;
            ChangeCallback callback;
        };
        std::vector<CallbackEntry> m_callbacks;
        std::size_t m_nextCallbackId = 1;
    };

    // Convenience macro for accessing config
    #define CONFIG rtype::core::ConfigManager::instance()

} // namespace rtype::core
