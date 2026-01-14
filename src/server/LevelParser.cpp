/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** LevelParser
*/

#include "LevelParser.hpp"
#include "GameServer.hpp"

namespace rtype::server {
    LevelParser::LevelParser()
    {
    }

    LevelParser::~LevelParser()
    {
    }

    LevelConfig LevelParser::parseLevel(const std::string& levelPath)
    {
        LevelConfig config;
        std::ifstream file(levelPath);
        std::string line;

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open level file: " + levelPath);
        }

        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            std::stringstream ss(line);
            std::string item;
            std::vector<std::string> row;

            while (std::getline(ss, item, ',')) {
                row.push_back(item);
            }

            if (row.size() >= 3) {
                LevelElement element;
                element.type = static_cast<network::EntityType>(std::stoi(row[0]));
                element.x = std::stof(row[1]);
                element.y = std::stof(row[2]);
                element.variant = (row.size() >= 4) ? static_cast<uint8_t>(std::stoi(row[3])) : 0;

                config.elements.push_back(element);
            }
            
        }

        file.close();
        config.levelName = "Dynamic Level";
        config.width = 1600.0f;
        config.height = 720.0f;
        return config;
    }
}
