/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SettingsWidget - Implementation of settings panel
*/

#include "SettingsWidget.hpp"
#include <iostream>

namespace rtype::ui {

    SettingsWidget::SettingsWidget(const SettingsConfig& config)
        : PanelWidget(), _config(config) {
        
        // Set up the panel properties
        setSize(600.0f, 500.0f);
        setBackgroundColor(UIColor(20, 20, 30, 240)); // Dark background
        setBorderColor(UIColor(100, 150, 255, 255)); // Blue border
        setBorderWidth(3.0f);
        setTitle("SETTINGS");
        setVisible(false); // Start hidden

        // Note: Don't call initializeLayout() in constructor due to shared_from_this() issues
    }

    void SettingsWidget::setCallbacks(const SettingsCallbacks& callbacks) {
        _callbacks = callbacks;
    }

    void SettingsWidget::setMusicEnabled(bool enabled) {
        _config.musicEnabled = enabled;
        updateButtonStates();
        
        if (_callbacks.onMusicToggle) {
            _callbacks.onMusicToggle(enabled);
        }
    }

    bool SettingsWidget::isMusicEnabled() const {
        return _config.musicEnabled;
    }

    void SettingsWidget::setMusicVolume(float volume) {
        _config.musicVolume = std::clamp(volume, 0.0f, 100.0f);
        
        if (_musicVolumeSlider) {
            _musicVolumeSlider->setRangeValue(_config.musicVolume);
        }
        
        if (_callbacks.onMusicVolumeChange) {
            _callbacks.onMusicVolumeChange(_config.musicVolume);
        }
    }

    float SettingsWidget::getMusicVolume() const {
        return _config.musicVolume;
    }

    void SettingsWidget::setEffectsVolume(float volume) {
        _config.effectsVolume = std::clamp(volume, 0.0f, 100.0f);
        
        if (_effectsVolumeSlider) {
            _effectsVolumeSlider->setRangeValue(_config.effectsVolume);
        }
        
        if (_callbacks.onEffectsVolumeChange) {
            _callbacks.onEffectsVolumeChange(_config.effectsVolume);
        }
    }

    float SettingsWidget::getEffectsVolume() const {
        return _config.effectsVolume;
    }

    SettingsConfig SettingsWidget::getConfig() const {
        return _config;
    }

    void SettingsWidget::show() {
        setVisible(true);
    }

    void SettingsWidget::hide() {
        setVisible(false);
        
        if (_callbacks.onClose) {
            _callbacks.onClose();
        }
    }

    void SettingsWidget::initialize() {
        initializeLayout();
    }

    void SettingsWidget::initializeLayout() {
        auto contentBounds = getContentBounds();
        
        // Create all sections
        createAudioSection();
        createPanelButtons();
        
        // Update initial state
        updateButtonStates();
    }

    void SettingsWidget::createAudioSection() {
        auto contentBounds = getContentBounds();
        
        // -- Audio Settings Title --
        _audioLabel = std::make_shared<TextWidget>("Audio Settings", 24);
        _audioLabel->setPosition(contentBounds.width / 2.0f - 80, 20.0f);
        _audioLabel->setTextColor(UIColor(255, 255, 255, 255));
        _audioLabel->setBackgroundColor(UIColor::Transparent());
        addChild(_audioLabel);
        
        // -- Music Toggle Section --
        _musicLabel = std::make_shared<TextWidget>("Music:", 24);
        _musicLabel->setPosition(80.0f, 120.0f);
        _musicLabel->setTextColor(UIColor(200, 200, 200, 255));
        _musicLabel->setBackgroundColor(UIColor::Transparent());
        addChild(_musicLabel);
        
        // Music ON button
        _musicOnButton = std::make_shared<ButtonWidget>("ON");
        _musicOnButton->setPosition(200.0f, 115.0f);
        _musicOnButton->setSize(120.0f, 50.0f);
        _musicOnButton->setBackgroundColor(UIColor(0, 150, 0, 255)); // Green
        _musicOnButton->setBorderColor(UIColor(0, 200, 0, 255));
        _musicOnButton->setBorderWidth(2.0f);
        _musicOnButton->setTextColor(UIColor::White());
        
        _musicOnButton->setOnClick([this]() {
            setMusicEnabled(true);
            std::cout << "Music enabled!" << std::endl;
        });
        
        addChild(_musicOnButton);
        
        // Music OFF button
        _musicOffButton = std::make_shared<ButtonWidget>("OFF");
        _musicOffButton->setPosition(340.0f, 115.0f);
        _musicOffButton->setSize(120.0f, 50.0f);
        _musicOffButton->setBackgroundColor(UIColor(150, 0, 0, 255)); // Red
        _musicOffButton->setBorderColor(UIColor(200, 0, 0, 255));
        _musicOffButton->setBorderWidth(2.0f);
        _musicOffButton->setTextColor(UIColor::White());
        
        _musicOffButton->setOnClick([this]() {
            setMusicEnabled(false);
            std::cout << "Music disabled!" << std::endl;
        });
        
        addChild(_musicOffButton);
        
        // -- Music Volume Slider Section --
        _musicVolumeLabel = std::make_shared<TextWidget>("Music Volume:", 22);
        _musicVolumeLabel->setPosition(80.0f, 180.0f);
        _musicVolumeLabel->setTextColor(UIColor(200, 200, 200, 255));
        _musicVolumeLabel->setBackgroundColor(UIColor::Transparent());
        addChild(_musicVolumeLabel);
        
        _musicVolumeSlider = std::make_shared<SliderWidget>(_config.musicVolume / 100.0f);
        _musicVolumeSlider->setPosition(200.0f, 210.0f);
        _musicVolumeSlider->setSize(260.0f, 30.0f);
        _musicVolumeSlider->setRange(0.0f, 100.0f);
        _musicVolumeSlider->setRangeValue(_config.musicVolume);
        _musicVolumeSlider->setFillColor({0, 150, 255, 255}); // Blue fill for music
        _musicVolumeSlider->setBackgroundColor(UIColor(60, 60, 60, 255));
        _musicVolumeSlider->setBorderColor(UIColor(100, 100, 100, 255));
        _musicVolumeSlider->setBorderWidth(1.0f);
        _musicVolumeSlider->setLabelFormat("%.0f%%");
        _musicVolumeSlider->setShowLabel(true);
        
        _musicVolumeSlider->setOnValueChange([this](float newVolume) {
            _config.musicVolume = newVolume;
            
            std::cout << "Music volume changed to: " << static_cast<int>(newVolume) << "%" << std::endl;
            
            if (_callbacks.onMusicVolumeChange) {
                _callbacks.onMusicVolumeChange(newVolume);
            }
        });
        
        addChild(_musicVolumeSlider);

        // -- Effects Volume Slider Section --
        _effectsVolumeLabel = std::make_shared<TextWidget>("Effects Volume:", 22);
        _effectsVolumeLabel->setPosition(80.0f, 260.0f);
        _effectsVolumeLabel->setTextColor(UIColor(200, 200, 200, 255));
        _effectsVolumeLabel->setBackgroundColor(UIColor::Transparent());
        addChild(_effectsVolumeLabel);
        
        _effectsVolumeSlider = std::make_shared<SliderWidget>(_config.effectsVolume / 100.0f);
        _effectsVolumeSlider->setPosition(200.0f, 290.0f);
        _effectsVolumeSlider->setSize(260.0f, 30.0f);
        _effectsVolumeSlider->setRange(0.0f, 100.0f);
        _effectsVolumeSlider->setRangeValue(_config.effectsVolume);
        _effectsVolumeSlider->setFillColor({255, 150, 0, 255}); // Orange fill for effects
        _effectsVolumeSlider->setBackgroundColor(UIColor(60, 60, 60, 255));
        _effectsVolumeSlider->setBorderColor(UIColor(100, 100, 100, 255));
        _effectsVolumeSlider->setBorderWidth(1.0f);
        _effectsVolumeSlider->setLabelFormat("%.0f%%");
        _effectsVolumeSlider->setShowLabel(true);
        
        _effectsVolumeSlider->setOnValueChange([this](float newVolume) {
            _config.effectsVolume = newVolume;
            
            std::cout << "Effects volume changed to: " << static_cast<int>(newVolume) << "%" << std::endl;
            
            if (_callbacks.onEffectsVolumeChange) {
                _callbacks.onEffectsVolumeChange(newVolume);
            }
        });
        
        addChild(_effectsVolumeSlider);
    }

    void SettingsWidget::createPanelButtons() {
        auto contentBounds = getContentBounds();
        
        // -- Back button --
        _backButton = std::make_shared<ButtonWidget>("BACK");
        _backButton->setPosition(contentBounds.width / 2.0f - 60, contentBounds.height - 60);
        _backButton->setSize(120.0f, 40.0f);
        _backButton->setBackgroundColor(UIColor(80, 80, 120, 200));
        _backButton->setBorderColor(UIColor(120, 120, 180, 255));
        _backButton->setBorderWidth(2.0f);
        _backButton->setTextColor(UIColor::White());
        
        _backButton->setOnClick([this]() {
            hide();
            std::cout << "Settings panel closed." << std::endl;
        });
        
        addChild(_backButton);
    }

    void SettingsWidget::updateButtonStates() {
        if (!_musicOnButton || !_musicOffButton) return;
        
        // Update button appearances based on current state
        if (_config.musicEnabled) {
            // ON button active (darker green when pressed)
            UIStyle activeStyle;
            activeStyle.backgroundColor = UIColor(0, 100, 0, 255);
            activeStyle.borderColor = UIColor(0, 200, 0, 255);
            activeStyle.borderWidth = 2.0f;
            activeStyle.textColor = UIColor::White();
            activeStyle.fontSize = 16;
            activeStyle.padding = 8.0f;
            _musicOnButton->setStateStyle(ButtonState::PRESSED, activeStyle);
            
            // OFF button normal
            _musicOffButton->setBackgroundColor(UIColor(150, 0, 0, 255));
        } else {
            // OFF button active (darker red when pressed)  
            UIStyle activeStyle;
            activeStyle.backgroundColor = UIColor(100, 0, 0, 255);
            activeStyle.borderColor = UIColor(200, 0, 0, 255);
            activeStyle.borderWidth = 2.0f;
            activeStyle.textColor = UIColor::White();
            activeStyle.fontSize = 16;
            activeStyle.padding = 8.0f;
            _musicOffButton->setStateStyle(ButtonState::PRESSED, activeStyle);
            
            // ON button normal
            _musicOnButton->setBackgroundColor(UIColor(0, 150, 0, 255));
        }
    }

} // namespace rtype::ui