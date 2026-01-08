/*
** R-Type - Settings Manager
** Manages game settings persistence with JSON format
*/

#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>

namespace rtype {

    /**
     * @brief Video settings
     */
    struct VideoSettings {
        bool fullscreen = false;
        bool vsync = true;
        int resolutionWidth = 1280;
        int resolutionHeight = 720;
    };

    /**
     * @brief Audio settings
     */
    struct AudioSettings {
        int masterVolume = 80;      // 0-100
        int sfxVolume = 100;        // 0-100
        int musicVolume = 70;       // 0-100
    };

    /**
     * @brief Key binding for an action
     */
    struct KeyBinding {
        std::string action;
        int keyboardKey;
        int gamepadButton;

        KeyBinding() : keyboardKey(-1), gamepadButton(-1) {}
        KeyBinding(const std::string& a, int k, int g = -1) 
            : action(a), keyboardKey(k), gamepadButton(g) {}
    };

    /**
     * @brief Gameplay settings
     */
    struct GameplaySettings {
        int difficulty = 1;         // 0=Easy, 1=Normal, 2=Hard
        bool friendlyFire = false;
        bool screenShake = true;
    };

    /**
     * @brief Manages all game settings with persistence
     * 
     * Handles saving/loading settings to/from a JSON file.
     * Provides defaults for all settings.
     */
    class SettingsManager {
    public:
        /**
         * @brief Construct a new Settings Manager with defaults
         */
        SettingsManager() {
            setDefaultBindings();
        }

        /**
         * @brief Load settings from a JSON file
         * @param path Path to the settings file
         * @return true if loaded successfully
         */
        bool load(const std::string& path) {
            m_filePath = path;

            std::ifstream file(path);
            if (!file.is_open()) {
                return false;  // Use defaults
            }

            std::string content((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
            file.close();

            return parseJson(content);
        }

        /**
         * @brief Save settings to a JSON file
         * @param path Path to save to (empty = use load path)
         * @return true if saved successfully
         */
        bool save(const std::string& path = "") {
            std::string savePath = path.empty() ? m_filePath : path;
            if (savePath.empty()) {
                savePath = "settings.json";
            }

            std::ofstream file(savePath);
            if (!file.is_open()) {
                return false;
            }

            file << toJson();
            file.close();
            m_dirty = false;
            return true;
        }

        // Video settings
        VideoSettings& video() { m_dirty = true; return m_video; }
        const VideoSettings& video() const { return m_video; }

        // Audio settings
        AudioSettings& audio() { m_dirty = true; return m_audio; }
        const AudioSettings& audio() const { return m_audio; }

        // Gameplay settings
        GameplaySettings& gameplay() { m_dirty = true; return m_gameplay; }
        const GameplaySettings& gameplay() const { return m_gameplay; }

        // Key bindings
        void bindKey(const std::string& action, int key) {
            m_bindings[action].keyboardKey = key;
            m_bindings[action].action = action;
            m_dirty = true;
        }

        void bindGamepad(const std::string& action, int button) {
            m_bindings[action].gamepadButton = button;
            m_bindings[action].action = action;
            m_dirty = true;
        }

        int getBoundKey(const std::string& action) const {
            auto it = m_bindings.find(action);
            return (it != m_bindings.end()) ? it->second.keyboardKey : -1;
        }

        int getBoundGamepad(const std::string& action) const {
            auto it = m_bindings.find(action);
            return (it != m_bindings.end()) ? it->second.gamepadButton : -1;
        }

        const std::unordered_map<std::string, KeyBinding>& getBindings() const {
            return m_bindings;
        }

        /**
         * @brief Reset all settings to defaults
         */
        void resetToDefaults() {
            m_video = VideoSettings();
            m_audio = AudioSettings();
            m_gameplay = GameplaySettings();
            setDefaultBindings();
            m_dirty = true;
        }

        /**
         * @brief Check if there are unsaved changes
         */
        bool isDirty() const { return m_dirty; }

    private:
        VideoSettings m_video;
        AudioSettings m_audio;
        GameplaySettings m_gameplay;
        std::unordered_map<std::string, KeyBinding> m_bindings;
        std::string m_filePath;
        bool m_dirty = false;

        void setDefaultBindings() {
            m_bindings.clear();
            // Movement (WASD + Arrows)
            m_bindings["up"] = KeyBinding("up", 87, 1);        // W, DPad Up
            m_bindings["down"] = KeyBinding("down", 83, 3);    // S, DPad Down
            m_bindings["left"] = KeyBinding("left", 65, 4);    // A, DPad Left
            m_bindings["right"] = KeyBinding("right", 68, 2);  // D, DPad Right
            // Actions
            m_bindings["shoot"] = KeyBinding("shoot", 32, 7);  // Space, A button
            m_bindings["bomb"] = KeyBinding("bomb", 66, 8);    // B, B button
            m_bindings["pause"] = KeyBinding("pause", 256, 6); // ESC, Start
        }

        bool parseJson(const std::string& content) {
            // Parse video settings
            m_video.fullscreen = extractBool(content, "fullscreen", false);
            m_video.vsync = extractBool(content, "vsync", true);
            m_video.resolutionWidth = extractInt(content, "resolutionWidth", 1280);
            m_video.resolutionHeight = extractInt(content, "resolutionHeight", 720);

            // Parse audio settings
            m_audio.masterVolume = extractInt(content, "masterVolume", 80);
            m_audio.sfxVolume = extractInt(content, "sfxVolume", 100);
            m_audio.musicVolume = extractInt(content, "musicVolume", 70);

            // Parse gameplay settings
            m_gameplay.difficulty = extractInt(content, "difficulty", 1);
            m_gameplay.friendlyFire = extractBool(content, "friendlyFire", false);
            m_gameplay.screenShake = extractBool(content, "screenShake", true);

            // Parse key bindings
            parseBindings(content);

            return true;
        }

        void parseBindings(const std::string& content) {
            size_t pos = content.find("\"bindings\"");
            if (pos == std::string::npos) return;

            pos = content.find('{', pos);
            if (pos == std::string::npos) return;

            size_t end = content.find('}', pos);
            if (end == std::string::npos) return;

            std::string bindingsStr = content.substr(pos, end - pos + 1);

            // Parse each known action
            for (auto& [action, binding] : m_bindings) {
                int key = extractInt(bindingsStr, action, binding.keyboardKey);
                binding.keyboardKey = key;
            }
        }

        std::string extractString(const std::string& json, const std::string& key, const std::string& def = "") {
            std::string searchKey = "\"" + key + "\"";
            size_t pos = json.find(searchKey);
            if (pos == std::string::npos) return def;

            pos = json.find(':', pos);
            if (pos == std::string::npos) return def;

            pos = json.find('"', pos + 1);
            if (pos == std::string::npos) return def;

            size_t endPos = json.find('"', pos + 1);
            if (endPos == std::string::npos) return def;

            return json.substr(pos + 1, endPos - pos - 1);
        }

        int extractInt(const std::string& json, const std::string& key, int def = 0) {
            std::string searchKey = "\"" + key + "\"";
            size_t pos = json.find(searchKey);
            if (pos == std::string::npos) return def;

            pos = json.find(':', pos);
            if (pos == std::string::npos) return def;

            pos++;
            while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

            bool negative = false;
            if (pos < json.size() && json[pos] == '-') {
                negative = true;
                pos++;
            }

            int value = 0;
            bool hasDigit = false;
            while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9') {
                value = value * 10 + (json[pos] - '0');
                hasDigit = true;
                pos++;
            }

            return hasDigit ? (negative ? -value : value) : def;
        }

        bool extractBool(const std::string& json, const std::string& key, bool def = false) {
            std::string searchKey = "\"" + key + "\"";
            size_t pos = json.find(searchKey);
            if (pos == std::string::npos) return def;

            pos = json.find(':', pos);
            if (pos == std::string::npos) return def;

            pos++;
            while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

            if (pos + 4 <= json.size() && json.substr(pos, 4) == "true") return true;
            if (pos + 5 <= json.size() && json.substr(pos, 5) == "false") return false;
            return def;
        }

        std::string toJson() const {
            std::ostringstream oss;
            oss << "{\n";
            
            // Video
            oss << "  \"video\": {\n";
            oss << "    \"fullscreen\": " << (m_video.fullscreen ? "true" : "false") << ",\n";
            oss << "    \"vsync\": " << (m_video.vsync ? "true" : "false") << ",\n";
            oss << "    \"resolutionWidth\": " << m_video.resolutionWidth << ",\n";
            oss << "    \"resolutionHeight\": " << m_video.resolutionHeight << "\n";
            oss << "  },\n";

            // Audio
            oss << "  \"audio\": {\n";
            oss << "    \"masterVolume\": " << m_audio.masterVolume << ",\n";
            oss << "    \"sfxVolume\": " << m_audio.sfxVolume << ",\n";
            oss << "    \"musicVolume\": " << m_audio.musicVolume << "\n";
            oss << "  },\n";

            // Gameplay
            oss << "  \"gameplay\": {\n";
            oss << "    \"difficulty\": " << m_gameplay.difficulty << ",\n";
            oss << "    \"friendlyFire\": " << (m_gameplay.friendlyFire ? "true" : "false") << ",\n";
            oss << "    \"screenShake\": " << (m_gameplay.screenShake ? "true" : "false") << "\n";
            oss << "  },\n";

            // Bindings
            oss << "  \"bindings\": {\n";
            size_t count = 0;
            for (const auto& [action, binding] : m_bindings) {
                oss << "    \"" << action << "\": " << binding.keyboardKey;
                if (++count < m_bindings.size()) oss << ",";
                oss << "\n";
            }
            oss << "  }\n";

            oss << "}\n";
            return oss.str();
        }
    };

} // namespace rtype
