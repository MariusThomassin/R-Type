/**
 * @file PathUtils.hpp
 * @brief Utilities for resolving asset paths relative to the executable
 */

#pragma once

#include <string>
#include <cstring>

#ifdef _WIN32
// Must include winsock2.h before windows.h to avoid conflicts with ASIO
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

namespace rtype {

/**
 * @brief Get the directory containing the executable
 * @return Path to executable directory with trailing separator
 */
inline std::string getExecutableDir() {
    static std::string cachedDir;
    
    if (!cachedDir.empty()) {
        return cachedDir;
    }
    
#ifdef _WIN32
    char path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
        return "./";
    }
#else
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) {
        return "./";
    }
    path[len] = '\0';
#endif
    
    std::string fullPath(path);
    size_t lastSep = fullPath.find_last_of("/\\");
    if (lastSep != std::string::npos) {
        cachedDir = fullPath.substr(0, lastSep + 1);
    } else {
        cachedDir = "./";
    }
    
    return cachedDir;
}

/**
 * @brief Resolve an asset path relative to the executable
 * @param assetPath Path like "assets/sprites/player.png"
 * @return Resolved path that can be loaded
 */
inline std::string resolveAssetPath(const std::string& assetPath) {
    if (!assetPath.empty() && (assetPath[0] == '/' || 
        (assetPath.length() > 1 && assetPath[1] == ':'))) {
        return assetPath;
    }
    
    std::string cleanPath = assetPath;
    while (cleanPath.substr(0, 3) == "../") {
        cleanPath = cleanPath.substr(3);
    }
    while (cleanPath.substr(0, 2) == "./") {
        cleanPath = cleanPath.substr(2);
    }
    
    std::string exeDir = getExecutableDir();
    std::string distPath = exeDir + cleanPath;
    
    FILE* f = fopen(distPath.c_str(), "r");
    if (f) {
        fclose(f);
        return distPath;
    }
    
    std::string devPath = exeDir + "../" + cleanPath;
    f = fopen(devPath.c_str(), "r");
    if (f) {
        fclose(f);
        return devPath;
    }
    
    f = fopen(cleanPath.c_str(), "r");
    if (f) {
        fclose(f);
        return cleanPath;
    }
    
    return distPath;
}

/**
 * @brief Resolve a config path relative to the executable
 * @param configPath Path like "config/settings.json"
 * @return Resolved path that can be loaded
 */
inline std::string resolveConfigPath(const std::string& configPath) {
    return resolveAssetPath(configPath);
}

} // namespace rtype
