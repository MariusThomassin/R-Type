/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** LevelParser to parse level files .txt
*/

#ifndef LEVELPARSER_HPP_
#define LEVELPARSER_HPP_

// Forward declarations to avoid circular dependencies
namespace rtype::server {
    struct LevelElement;
    struct LevelConfig;
}

#include "shared/network/Protocol.hpp"

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

namespace rtype::server {

    class LevelParser {
        public:
            LevelParser();
            ~LevelParser();

            LevelConfig parseLevel(const std::string& levelPath);
    };
} // namespace rtype::server

#endif /* !LEVELPARSER_HPP_ */
