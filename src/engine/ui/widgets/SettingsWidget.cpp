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
        : PanelWidget(), _config(config), _currentTab(SettingsTab::AUDIO) {
        
        // Set up the panel properties - match multiplayer widget size
        setSize(800.0f, 600.0f); // Reduced height to fit screen (720px height)
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
        createTabButtons();
        createAudioSection();
        createKeyBindingsSection();
        createPanelButtons();
        
        // Initialize to audio tab
        switchToTab(SettingsTab::AUDIO);
        
        // Update initial state
        updateButtonStates();
        updateKeyBindingDisplays();
    }

    void SettingsWidget::createAudioSection() {
        auto contentBounds = getContentBounds();
        
        // -- Audio Settings Title --
        _audioLabel = std::make_shared<TextWidget>("Audio Settings", 24);
        _audioLabel->setPosition(contentBounds.width / 2.0f - 80, 160.0f); // Moved down from tabs
        _audioLabel->setTextColor(UIColor(255, 255, 255, 255));
        _audioLabel->setBackgroundColor(UIColor::Transparent());
        addChild(_audioLabel);
        
        // -- Music Toggle Section --
        _musicLabel = std::make_shared<TextWidget>("Music:", 24);
        _musicLabel->setPosition(80.0f, 220.0f); // Adjusted for new layout
        _musicLabel->setTextColor(UIColor(200, 200, 200, 255));
        _musicLabel->setBackgroundColor(UIColor::Transparent());
        addChild(_musicLabel);
        
        // Music ON button
        _musicOnButton = std::make_shared<ButtonWidget>("ON");
        _musicOnButton->setPosition(200.0f, 215.0f); // Adjusted for new layout
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
        _musicOffButton->setPosition(340.0f, 215.0f); // Adjusted for new layout
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
        _musicVolumeLabel->setPosition(80.0f, 300.0f); // Adjusted for smaller panel
        _musicVolumeLabel->setTextColor(UIColor(200, 200, 200, 255));
        _musicVolumeLabel->setBackgroundColor(UIColor::Transparent());
        addChild(_musicVolumeLabel);
        
        _musicVolumeSlider = std::make_shared<SliderWidget>(_config.musicVolume / 100.0f);
        _musicVolumeSlider->setPosition(200.0f, 330.0f); // Adjusted for smaller panel
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
        _effectsVolumeLabel->setPosition(80.0f, 390.0f); // Adjusted for smaller panel
        _effectsVolumeLabel->setTextColor(UIColor(200, 200, 200, 255));
        _effectsVolumeLabel->setBackgroundColor(UIColor::Transparent());
        addChild(_effectsVolumeLabel);
        
        _effectsVolumeSlider = std::make_shared<SliderWidget>(_config.effectsVolume / 100.0f);
        _effectsVolumeSlider->setPosition(200.0f, 420.0f); // Adjusted for smaller panel
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
        _backButton->setPosition(contentBounds.width / 2.0f - 60, contentBounds.height - 30);
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

    void SettingsWidget::createKeyBindingsSection() {
        auto contentBounds = getContentBounds();
        
        // -- Controls Settings Title --
        _controlsLabel = std::make_shared<TextWidget>("CONTROLS");
        _controlsLabel->setPosition(contentBounds.width / 2.0f - 80, 160.0f); // Match audio title position
        _controlsLabel->setSize(200, 30);
        _controlsLabel->setTextColor(UIColor::White());
        _controlsLabel->setFontSize(24); // Match audio section font size
        addChild(_controlsLabel);
        
        // Only create key bindings if we have a settings manager
        if (!_config.settingsManager) {
            return;
        }
        
        // Get all key bindings from settings manager
        const auto& bindings = _config.settingsManager->getBindings();
        
        // Define action display names and order
        std::vector<std::pair<std::string, std::string>> actionList = {
            {"up", "Move Up"},
            {"down", "Move Down"},
            {"left", "Move Left"},
            {"right", "Move Right"},
            {"shoot", "Shoot"},
            {"bomb", "Bomb/Special"},
            {"orbSwitch", "Orb Switch"},
            {"pause", "Pause"}
        };
        
        float yOffset = 200; // Start below title, account for smaller panel
        float spacing = 35; // Reduced spacing for smaller panel
        
        for (const auto& [actionKey, displayName] : actionList) {
            // Find the current key binding
            auto bindingIt = bindings.find(actionKey);
            rtype::ecs::events::KeyCode currentKey = rtype::ecs::events::KeyCode::Up; // Default
            
            if (bindingIt != bindings.end()) {
                // Convert raylib key code to our KeyCode enum
                currentKey = rtype::ecs::events::InputUtils::raylibToKeyCode(bindingIt->second.keyboardKey);
            }
            
            // Create key binding widget
            auto keyBindingWidget = std::make_shared<KeyBindingWidget>(displayName, currentKey);
            keyBindingWidget->setPosition(20, yOffset);
            keyBindingWidget->setSize(contentBounds.width - 40, 35);
            
            // Set callback for when this key binding changes
            keyBindingWidget->setOnKeyChange([this, actionKey](rtype::ecs::events::KeyCode newKey) {
                handleKeyBindingChange(actionKey, newKey);
            });
            
            _keyBindingWidgets[actionKey] = keyBindingWidget;
            addChild(keyBindingWidget);
            
            yOffset += spacing;
        }
    }

    void SettingsWidget::updateKeyBindingDisplays() {
        if (!_config.settingsManager) {
            return;
        }
        
        const auto& bindings = _config.settingsManager->getBindings();
        
        for (const auto& [actionKey, widget] : _keyBindingWidgets) {
            auto bindingIt = bindings.find(actionKey);
            if (bindingIt != bindings.end()) {
                rtype::ecs::events::KeyCode currentKey = 
                    rtype::ecs::events::InputUtils::raylibToKeyCode(bindingIt->second.keyboardKey);
                widget->setCurrentKey(currentKey);
            }
        }
    }

    void SettingsWidget::handleKeyBindingChange(const std::string& action, rtype::ecs::events::KeyCode newKey) {
        if (!_config.settingsManager) {
            return;
        }
        
        // Check for conflicts
        std::string conflictingAction = checkKeyConflict(action, newKey);
        if (!conflictingAction.empty()) {
            showConflictWarning(conflictingAction, action);
            // Revert the key binding widget to its previous value
            updateKeyBindingDisplays();
            return;
        }
        
        // Convert our KeyCode to raylib key code and update the settings manager
        int raylibKey = rtype::ecs::events::InputUtils::keyCodeToRaylib(newKey);
        if (raylibKey != -1) {
            _config.settingsManager->bindKey(action, raylibKey);
            
            // Notify callback if set
            if (_callbacks.onKeyBindingChange) {
                _callbacks.onKeyBindingChange(action, newKey);
            }
            
            std::cout << "Key binding updated: " << action << " -> " 
                      << rtype::ecs::events::InputUtils::keyCodeToString(newKey) << std::endl;
        }
    }

    std::string SettingsWidget::checkKeyConflict(const std::string& excludeAction, rtype::ecs::events::KeyCode key) {
        if (!_config.settingsManager) {
            return "";
        }
        
        int raylibKey = rtype::ecs::events::InputUtils::keyCodeToRaylib(key);
        if (raylibKey == -1) {
            return "";
        }
        
        const auto& bindings = _config.settingsManager->getBindings();
        
        for (const auto& [actionName, binding] : bindings) {
            if (actionName != excludeAction && binding.keyboardKey == raylibKey) {
                return actionName;
            }
        }
        
        return "";
    }

    void SettingsWidget::showConflictWarning(const std::string& conflictingAction, const std::string& newAction) {
        std::cout << "Key binding conflict: Cannot bind to " << newAction 
                  << " because it's already used by " << conflictingAction << std::endl;
        
        // TODO: Could add a proper UI warning dialog here
        // For now, just print to console and revert the binding
    }

    void SettingsWidget::createTabButtons() {
        auto contentBounds = getContentBounds();
        
        // Audio tab button - match multiplayer tab positioning
        _audioTabButton = std::make_shared<ButtonWidget>("AUDIO");
        _audioTabButton->setPosition(100.0f, 90.0f); // Match multiplayer Y position
        _audioTabButton->setSize(150.0f, 50.0f); // Match multiplayer button height
        _audioTabButton->setTextColor(UIColor(255, 255, 255, 255));
        _audioTabButton->setBackgroundColor(UIColor(50, 100, 200, 255)); // Active color
        _audioTabButton->setOnClick([this]() {
            switchToTab(SettingsTab::AUDIO);
        });
        addChild(_audioTabButton);
        
        // Controls tab button - proper spacing from audio tab
        _controlsTabButton = std::make_shared<ButtonWidget>("CONTROLS");
        _controlsTabButton->setPosition(270.0f, 90.0f); // Spaced 20px after first button (100+150+20)
        _controlsTabButton->setSize(150.0f, 50.0f);
        _controlsTabButton->setTextColor(UIColor(255, 255, 255, 255));
        _controlsTabButton->setBackgroundColor(UIColor(80, 80, 80, 255)); // Inactive color
        _controlsTabButton->setOnClick([this]() {
            switchToTab(SettingsTab::CONTROLS);
        });
        addChild(_controlsTabButton);
    }

    void SettingsWidget::switchToTab(SettingsTab tab) {
        _currentTab = tab;
        
        // Update tab buttons appearance
        updateTabButtons();
        
        // Show/hide appropriate content sections
        bool showAudio = (tab == SettingsTab::AUDIO);
        bool showControls = (tab == SettingsTab::CONTROLS);
        
        // Audio section visibility
        if (_audioLabel) _audioLabel->setVisible(showAudio);
        if (_musicLabel) _musicLabel->setVisible(showAudio);
        if (_musicOnButton) _musicOnButton->setVisible(showAudio);
        if (_musicOffButton) _musicOffButton->setVisible(showAudio);
        if (_musicVolumeLabel) _musicVolumeLabel->setVisible(showAudio);
        if (_musicVolumeSlider) _musicVolumeSlider->setVisible(showAudio);
        if (_effectsVolumeLabel) _effectsVolumeLabel->setVisible(showAudio);
        if (_effectsVolumeSlider) _effectsVolumeSlider->setVisible(showAudio);
        
        // Controls section visibility
        if (_controlsLabel) _controlsLabel->setVisible(showControls);
        for (auto& [action, widget] : _keyBindingWidgets) {
            if (widget) widget->setVisible(showControls);
        }
    }

    void SettingsWidget::updateTabButtons() {
        if (_audioTabButton) {
            if (_currentTab == SettingsTab::AUDIO) {
                _audioTabButton->setBackgroundColor(UIColor(50, 100, 200, 255)); // Active
            } else {
                _audioTabButton->setBackgroundColor(UIColor(80, 80, 80, 255)); // Inactive
            }
        }
        
        if (_controlsTabButton) {
            if (_currentTab == SettingsTab::CONTROLS) {
                _controlsTabButton->setBackgroundColor(UIColor(50, 100, 200, 255)); // Active
            } else {
                _controlsTabButton->setBackgroundColor(UIColor(80, 80, 80, 255)); // Inactive
            }
        }
    }

    bool SettingsWidget::isWaitingForKeyInput() const {
        // Check if any key binding widget is currently waiting for input
        for (const auto& [action, widget] : _keyBindingWidgets) {
            if (widget && widget->isWaitingForInput()) {
                return true;
            }
        }
        return false;
    }

} // namespace rtype::ui