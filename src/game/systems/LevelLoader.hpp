/*
** R-Type - LevelLoader
** JSON-based level configuration loader
*/

#pragma once

#include "EnemySpawnerSystem.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <optional>

namespace rtype::ecs {

    /**
     * @brief Loads level configurations from JSON files
     * 
     * Level JSON format:
     * {
     *   "name": "Level 1",
     *   "difficulty": 1,
     *   "waveDelay": 2.0,
     *   "waves": [
     *     {
     *       "delayBefore": 0.0,
     *       "spawnInterval": 0.5,
     *       "simultaneous": false,
     *       "enemies": [
     *         {
     *           "type": "basic",
     *           "x": 1300,
     *           "y": 200,
     *           "vx": -100,
     *           "vy": 0,
     *           "health": 1,
     *           "scoreValue": 100
     *         }
     *       ]
     *     }
     *   ]
     * }
     */
    class LevelLoader {
    public:
        /**
         * @brief Load a level configuration from a JSON file
         * @param filename Path to the JSON file
         * @return LevelConfig if successful, std::nullopt on failure
         */
        static std::optional<LevelConfig> loadFromFile(const std::string& filename) {
            try {
                std::ifstream file(filename);
                if (!file.is_open()) {
                    std::cerr << "[LevelLoader] Failed to open file: " << filename << std::endl;
                    return std::nullopt;
                }

                nlohmann::json j;
                file >> j;
                
                return parseLevel(j);
            } catch (const std::exception& e) {
                std::cerr << "[LevelLoader] Error loading " << filename << ": " << e.what() << std::endl;
                return std::nullopt;
            }
        }

        /**
         * @brief Load a level from JSON string
         * @param jsonStr JSON string containing level data
         * @return LevelConfig if successful, std::nullopt on failure
         */
        static std::optional<LevelConfig> loadFromString(const std::string& jsonStr) {
            try {
                nlohmann::json j = nlohmann::json::parse(jsonStr);
                return parseLevel(j);
            } catch (const std::exception& e) {
                std::cerr << "[LevelLoader] Error parsing JSON: " << e.what() << std::endl;
                return std::nullopt;
            }
        }

        /**
         * @brief Get list of available level files in a directory
         * @param directory Path to levels directory
         * @return Vector of level file paths
         */
        static std::vector<std::string> discoverLevels(const std::string& directory) {
            std::vector<std::string> levels;
            
            try {
                for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                    if (entry.path().extension() == ".json") {
                        levels.push_back(entry.path().string());
                    }
                }
                // Sort by filename for consistent ordering
                std::sort(levels.begin(), levels.end());
            } catch (const std::exception& e) {
                std::cerr << "[LevelLoader] Error scanning directory " << directory << ": " << e.what() << std::endl;
            }
            
            return levels;
        }

        /**
         * @brief Save a level configuration to JSON file
         * @param config The level configuration to save
         * @param filename Path to save file
         * @return True if successful
         */
        static bool saveToFile(const LevelConfig& config, const std::string& filename) {
            try {
                nlohmann::json j = serializeLevel(config);
                
                std::ofstream file(filename);
                if (!file.is_open()) {
                    std::cerr << "[LevelLoader] Failed to create file: " << filename << std::endl;
                    return false;
                }
                
                file << j.dump(2);  // Pretty print with 2-space indent
                return true;
            } catch (const std::exception& e) {
                std::cerr << "[LevelLoader] Error saving " << filename << ": " << e.what() << std::endl;
                return false;
            }
        }

    private:
        /**
         * @brief Convert string to EnemyType enum
         */
        static EnemyType parseEnemyType(const std::string& typeStr) {
            if (typeStr == "basic" || typeStr == "Basic") return EnemyType::Basic;
            if (typeStr == "shooter" || typeStr == "Shooter") return EnemyType::Shooter;
            if (typeStr == "chaser" || typeStr == "Chaser") return EnemyType::Chaser;
            if (typeStr == "boss" || typeStr == "Boss") return EnemyType::Boss;
            if (typeStr == "turret" || typeStr == "Turret") return EnemyType::Turret;
            
            std::cerr << "[LevelLoader] Unknown enemy type: " << typeStr << ", defaulting to Basic" << std::endl;
            return EnemyType::Basic;
        }

        /**
         * @brief Convert EnemyType enum to string
         */
        static std::string enemyTypeToString(EnemyType type) {
            switch (type) {
                case EnemyType::Basic: return "basic";
                case EnemyType::Shooter: return "shooter";
                case EnemyType::Chaser: return "chaser";
                case EnemyType::Boss: return "boss";
                case EnemyType::Turret: return "turret";
                default: return "basic";
            }
        }

        /**
         * @brief Parse JSON into LevelConfig
         */
        static LevelConfig parseLevel(const nlohmann::json& j) {
            LevelConfig config;
            
            config.waveDelay = j.value("waveDelay", 2.0f);
            config.difficulty = j.value("difficulty", 1);
            
            // Parse level assets
            config.name = j.value("name", "");
            config.background = j.value("background", "");
            config.stageMusic = j.value("stageMusic", "");
            config.bossMusic = j.value("bossMusic", "");
            
            if (j.contains("waves") && j["waves"].is_array()) {
                for (const auto& waveJson : j["waves"]) {
                    config.waves.push_back(parseWave(waveJson));
                }
            }
            
            return config;
        }

        /**
         * @brief Parse JSON into WaveConfig
         */
        static WaveConfig parseWave(const nlohmann::json& j) {
            WaveConfig config;
            
            config.delayBefore = j.value("delayBefore", 0.0f);
            config.spawnInterval = j.value("spawnInterval", 0.5f);
            config.simultaneous = j.value("simultaneous", false);
            
            if (j.contains("enemies") && j["enemies"].is_array()) {
                for (const auto& enemyJson : j["enemies"]) {
                    config.enemies.push_back(parseEnemy(enemyJson));
                }
            }
            
            return config;
        }

        /**
         * @brief Parse JSON into EnemySpawnConfig
         */
        static EnemySpawnConfig parseEnemy(const nlohmann::json& j) {
            EnemySpawnConfig config;
            
            // Parse type (string or int)
            if (j.contains("type")) {
                if (j["type"].is_string()) {
                    config.type = parseEnemyType(j["type"].get<std::string>());
                } else if (j["type"].is_number()) {
                    config.type = static_cast<EnemyType>(j["type"].get<int>());
                }
            }
            
            config.x = j.value("x", 1300.0f);
            config.y = j.value("y", 360.0f);
            config.vx = j.value("vx", -100.0f);
            config.vy = j.value("vy", 0.0f);
            config.health = j.value("health", 1);
            config.scoreValue = j.value("scoreValue", 100);
            
            return config;
        }

        /**
         * @brief Serialize LevelConfig to JSON
         */
        static nlohmann::json serializeLevel(const LevelConfig& config) {
            nlohmann::json j;
            
            j["waveDelay"] = config.waveDelay;
            j["difficulty"] = config.difficulty;
            
            // Serialize level assets
            if (!config.name.empty()) {
                j["name"] = config.name;
            }
            if (!config.background.empty()) {
                j["background"] = config.background;
            }
            if (!config.stageMusic.empty()) {
                j["stageMusic"] = config.stageMusic;
            }
            if (!config.bossMusic.empty()) {
                j["bossMusic"] = config.bossMusic;
            }
            
            j["waves"] = nlohmann::json::array();
            
            for (const auto& wave : config.waves) {
                j["waves"].push_back(serializeWave(wave));
            }
            
            return j;
        }

        /**
         * @brief Serialize WaveConfig to JSON
         */
        static nlohmann::json serializeWave(const WaveConfig& config) {
            nlohmann::json j;
            
            j["delayBefore"] = config.delayBefore;
            j["spawnInterval"] = config.spawnInterval;
            j["simultaneous"] = config.simultaneous;
            j["enemies"] = nlohmann::json::array();
            
            for (const auto& enemy : config.enemies) {
                j["enemies"].push_back(serializeEnemy(enemy));
            }
            
            return j;
        }

        /**
         * @brief Serialize EnemySpawnConfig to JSON
         */
        static nlohmann::json serializeEnemy(const EnemySpawnConfig& config) {
            nlohmann::json j;
            
            j["type"] = enemyTypeToString(config.type);
            j["x"] = config.x;
            j["y"] = config.y;
            j["vx"] = config.vx;
            j["vy"] = config.vy;
            j["health"] = config.health;
            j["scoreValue"] = config.scoreValue;
            
            return j;
        }
    };

} // namespace rtype::ecs
