/*
** R-Type Engine - WindowSerializer Implementation
** Saves and loads window state for persistence across sessions
*/

#include "WindowSerializer.hpp"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

namespace rtype::ui {

    bool WindowSerializer::saveStates(const WindowManager& manager, const std::string& filepath) {
        try {
            auto states = captureStates(manager);
            std::string json = toJson(states);
            
            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            file << json;
            file.close();
            return true;
        } catch (...) {
            return false;
        }
    }

    bool WindowSerializer::loadStates(WindowManager& manager, const std::string& filepath) {
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            
            auto states = fromJson(buffer.str());
            applyStates(manager, states);
            return true;
        } catch (...) {
            return false;
        }
    }

    std::vector<WindowState> WindowSerializer::captureStates(const WindowManager& manager) {
        std::vector<WindowState> states;
        
        // Get all window IDs from the manager
        auto windowIds = manager.getWindowIds();
        
        for (const auto& id : windowIds) {
            auto window = const_cast<WindowManager&>(manager).getWindow(id);
            if (!window) continue;
            
            WindowState state;
            state.id = id;
            
            auto transform = window->getAbsoluteTransform();
            state.x = transform.x;
            state.y = transform.y;
            state.width = transform.width;
            state.height = transform.height;
            state.visible = window->isVisible();
            state.collapsed = window->isCollapsed();
            state.scrollOffset = window->getScrollOffset();
            
            states.push_back(state);
        }
        
        return states;
    }

    void WindowSerializer::applyStates(WindowManager& manager, const std::vector<WindowState>& states) {
        for (const auto& state : states) {
            auto window = manager.getWindow(state.id);
            if (window) {
                window->setPosition(state.x, state.y);
                window->setSize(state.width, state.height);
                window->setVisible(state.visible);
                window->setCollapsed(state.collapsed);
                window->setScrollOffset(state.scrollOffset);
            }
        }
    }

    std::string WindowSerializer::toJson(const std::vector<WindowState>& states) {
        nlohmann::json j = nlohmann::json::array();
        
        for (const auto& state : states) {
            nlohmann::json windowJson;
            windowJson["id"] = state.id;
            windowJson["x"] = state.x;
            windowJson["y"] = state.y;
            windowJson["width"] = state.width;
            windowJson["height"] = state.height;
            windowJson["visible"] = state.visible;
            windowJson["collapsed"] = state.collapsed;
            windowJson["scrollOffset"] = state.scrollOffset;
            j.push_back(windowJson);
        }
        
        return j.dump(2);  // Pretty print with 2-space indent
    }

    std::vector<WindowState> WindowSerializer::fromJson(const std::string& json) {
        std::vector<WindowState> states;
        
        try {
            nlohmann::json j = nlohmann::json::parse(json);
            
            if (!j.is_array()) {
                return states;
            }
            
            for (const auto& windowJson : j) {
                WindowState state;
                
                if (windowJson.contains("id")) {
                    state.id = windowJson["id"].get<std::string>();
                }
                if (windowJson.contains("x")) {
                    state.x = windowJson["x"].get<float>();
                }
                if (windowJson.contains("y")) {
                    state.y = windowJson["y"].get<float>();
                }
                if (windowJson.contains("width")) {
                    state.width = windowJson["width"].get<float>();
                }
                if (windowJson.contains("height")) {
                    state.height = windowJson["height"].get<float>();
                }
                if (windowJson.contains("visible")) {
                    state.visible = windowJson["visible"].get<bool>();
                }
                if (windowJson.contains("collapsed")) {
                    state.collapsed = windowJson["collapsed"].get<bool>();
                }
                if (windowJson.contains("scrollOffset")) {
                    state.scrollOffset = windowJson["scrollOffset"].get<float>();
                }
                
                states.push_back(state);
            }
        } catch (...) {
            // Return empty on parse error
            states.clear();
        }
        
        return states;
    }

} // namespace rtype::ui
