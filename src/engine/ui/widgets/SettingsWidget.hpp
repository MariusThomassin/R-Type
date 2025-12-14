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
#include <functional>
#include <memory>

namespace rtype::ui {

    /**
     * @brief Settings configuration structure
     */
    struct SettingsConfig {
        bool musicEnabled = true;
        float musicVolume = 75.0f;
        // Add more settings here as needed
    };

    /**
     * @brief Callbacks for settings changes
     */
    struct SettingsCallbacks {
        std::function<void(bool)> onMusicToggle = nullptr;
        std::function<void(float)> onVolumeChange = nullptr;
        std::function<void()> onClose = nullptr;
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

    private:
        /**
         * @brief Initialize the settings panel layout
         */
        void initializeLayout();

        /**
         * @brief Create the audio settings section
         */
        void createAudioSection();

        /**
         * @brief Create the panel buttons (back, etc.)
         */
        void createPanelButtons();

        /**
         * @brief Update button states based on current settings
         */
        void updateButtonStates();

        // Settings state
        SettingsConfig _config;
        SettingsCallbacks _callbacks;

        // UI Elements
        std::shared_ptr<TextWidget> _titleText;
        std::shared_ptr<TextWidget> _audioLabel;
        std::shared_ptr<TextWidget> _musicLabel;
        std::shared_ptr<ButtonWidget> _musicOnButton;
        std::shared_ptr<ButtonWidget> _musicOffButton;
        std::shared_ptr<TextWidget> _volumeLabel;
        std::shared_ptr<SliderWidget> _volumeSlider;
        std::shared_ptr<TextWidget> _volumeDisplay;
        std::shared_ptr<ButtonWidget> _backButton;
    };

} // namespace rtype::ui

#endif /* !SETTINGSWIDGET_HPP_ */