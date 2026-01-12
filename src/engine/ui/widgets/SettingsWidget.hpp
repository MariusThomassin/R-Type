/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SettingsWidget - Dedicated settings panel with audio controls
*/

#ifndef SETTINGSWIDGET_HPP_
#define SETTINGSWIDGET_HPP_

#include "PanelWidget.hpp"
#include "ButtonWidget.hpp"
#include "TextWidget.hpp"
#include "SliderWidget.hpp"
#include "KeyBindingWidget.hpp"
#include "../../../shared/SettingsManager.hpp"
#include <functional>
#include <memory>
#include <unordered_map>

namespace rtype::ui {

    /**
     * @brief Settings configuration structure
     */
    struct SettingsConfig {
        bool musicEnabled = true;
        float musicVolume = 75.0f;
        float effectsVolume = 75.0f;  // Separate volume for sound effects
        
        // Key binding settings (pointer to external SettingsManager)
        rtype::SettingsManager* settingsManager = nullptr;
        
        // Add more settings here as needed
    };

    /**
     * @brief Callbacks for settings changes
     */
    struct SettingsCallbacks {
        std::function<void(bool)> onMusicToggle = nullptr;
        std::function<void(float)> onMusicVolumeChange = nullptr;    // Music volume callback
        std::function<void(float)> onEffectsVolumeChange = nullptr;  // Effects volume callback
        std::function<void(const std::string&, rtype::ecs::events::KeyCode)> onKeyBindingChange = nullptr; // Key binding callback
        std::function<void()> onClose = nullptr;
    };

    /**
     * @brief Tab enumeration for settings categories
     */
    enum class SettingsTab {
        AUDIO = 0,
        CONTROLS = 1
    };

    /**
     * @brief A complete settings panel widget
     * 
     * SettingsWidget provides a self-contained settings interface with
     * audio controls, organized in a clean panel layout. Supports
     * music enable/disable toggle and volume slider control.
     */
    class SettingsWidget : public PanelWidget {
    public:
        /**
         * @brief Construct a new SettingsWidget
         * @param config Initial settings configuration
         */
        explicit SettingsWidget(const SettingsConfig& config = SettingsConfig{});

        ~SettingsWidget() override = default;

        /**
         * @brief Set callbacks for settings changes
         * @param callbacks Function callbacks for various settings events
         */
        void setCallbacks(const SettingsCallbacks& callbacks);

        /**
         * @brief Update the music enabled state
         * @param enabled Whether music is enabled
         */
        void setMusicEnabled(bool enabled);

        /**
         * @brief Get the current music enabled state
         * @return True if music is enabled
         */
        bool isMusicEnabled() const;

        /**
         * @brief Set the music volume
         * @param volume Volume percentage (0-100)
         */
        void setMusicVolume(float volume);

        /**
         * @brief Get the current music volume
         * @return Volume percentage (0-100)
         */
        float getMusicVolume() const;

        /**
         * @brief Set the effects volume
         * @param volume Volume percentage (0-100)
         */
        void setEffectsVolume(float volume);

        /**
         * @brief Get the current effects volume
         * @return Volume percentage (0-100)
         */
        float getEffectsVolume() const;

        /**
         * @brief Get the current settings configuration
         * @return Current settings state
         */
        SettingsConfig getConfig() const;

        /**
         * @brief Show the settings panel
         */
        void show();

        /**
         * @brief Hide the settings panel
         */
        void hide();

        /**
         * @brief Initialize the widget after construction
         * 
         * This must be called after the widget has been added to the UI manager
         * to avoid shared_from_this() issues during construction.
         */
        void initialize();

        /**
         * @brief Check if any key binding widget is currently waiting for input
         * @return True if any key binding is waiting for input
         */
        bool isWaitingForKeyInput() const;

    private:
        /**
         * @brief Initialize the settings panel layout
         */
        void initializeLayout();

        /**
         * @brief Create the tab buttons
         */
        void createTabButtons();

        /**
         * @brief Create the audio settings section
         */
        void createAudioSection();

        /**
         * @brief Create the key binding controls section
         */
        void createKeyBindingsSection();

        /**
         * @brief Create the panel buttons (back, etc.)
         */
        void createPanelButtons();

        /**
         * @brief Switch to a different tab
         * @param tab The tab to switch to
         */
        void switchToTab(SettingsTab tab);

        /**
         * @brief Update tab button appearances
         */
        void updateTabButtons();

        /**
         * @brief Update button states based on current settings
         */
        void updateButtonStates();

        /**
         * @brief Update key binding displays based on current settings
         */
        void updateKeyBindingDisplays();

        /**
         * @brief Handle key binding change for a specific action
         * @param action The action name (e.g., "up", "shoot")
         * @param newKey The newly bound key
         */
        void handleKeyBindingChange(const std::string& action, rtype::ecs::events::KeyCode newKey);

        /**
         * @brief Check for key binding conflicts
         * @param excludeAction Action to exclude from conflict check (current one being changed)
         * @param key Key to check for conflicts
         * @return Action name that conflicts, or empty string if no conflict
         */
        std::string checkKeyConflict(const std::string& excludeAction, rtype::ecs::events::KeyCode key);

        /**
         * @brief Show warning about key binding conflict
         * @param conflictingAction The action that already uses this key
         * @param newAction The action trying to use this key
         */
        void showConflictWarning(const std::string& conflictingAction, const std::string& newAction);

        // Settings state
        SettingsConfig _config;
        SettingsCallbacks _callbacks;
        SettingsTab _currentTab;

        // UI Elements - Tabs
        std::shared_ptr<ButtonWidget> _audioTabButton;
        std::shared_ptr<ButtonWidget> _controlsTabButton;
        
        // UI Elements - Audio
        std::shared_ptr<TextWidget> _titleText;
        std::shared_ptr<TextWidget> _audioLabel;
        std::shared_ptr<TextWidget> _musicLabel;
        std::shared_ptr<ButtonWidget> _musicOnButton;
        std::shared_ptr<ButtonWidget> _musicOffButton;
        std::shared_ptr<TextWidget> _musicVolumeLabel;      // Music volume label
        std::shared_ptr<SliderWidget> _musicVolumeSlider;   // Music volume slider
        std::shared_ptr<TextWidget> _effectsVolumeLabel;    // Effects volume label
        std::shared_ptr<SliderWidget> _effectsVolumeSlider; // Effects volume slider
        
        // UI Elements - Key Bindings
        std::shared_ptr<TextWidget> _controlsLabel;
        std::unordered_map<std::string, std::shared_ptr<KeyBindingWidget>> _keyBindingWidgets;
        
        // UI Elements - Panel
        std::shared_ptr<ButtonWidget> _backButton;
    };

} // namespace rtype::ui

#endif /* !SETTINGSWIDGET_HPP_ */