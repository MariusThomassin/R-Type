/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SliderWidget - A draggable slider for value selection
*/

#ifndef SLIDERWIDGET_HPP_
#define SLIDERWIDGET_HPP_

#include "src/engine/ui/Widget.hpp"
#include <string>
#include <functional>

namespace rtype::ui {

    /**
     * @brief A horizontal slider widget for value selection
     * 
     * SliderWidget allows users to select a value within a range
     * by dragging a thumb along a track.
     */
    class SliderWidget : public Widget {
    public:
        using ValueCallback = std::function<void(float value)>;

        /**
         * @brief Construct a new SliderWidget
         * @param minValue Minimum value
         * @param maxValue Maximum value
         * @param value Initial value
         */
        SliderWidget(float minValue = 0.0f, float maxValue = 1.0f, float value = 0.5f);

        ~SliderWidget() override = default;

        /**
         * @brief Set the current value
         * @param value Value to set (will be clamped to range)
         */
        void setValue(float value);

        /**
         * @brief Get the current value
         * @return Current value
         */
        float getValue() const;

        /**
         * @brief Set the value range
         * @param minValue Minimum value
         * @param maxValue Maximum value
         */
        void setRange(float minValue, float maxValue);

        /**
         * @brief Get minimum value
         * @return Minimum value
         */
        float getMinValue() const;

        /**
         * @brief Get maximum value
         * @return Maximum value
         */
        float getMaxValue() const;

        /**
         * @brief Set value change callback
         * @param callback Called when value changes
         */
        void setOnChange(ValueCallback callback);

        /**
         * @brief Set track color
         * @param color The track color
         */
        void setTrackColor(const Color& color);

        /**
         * @brief Set fill color (left of thumb)
         * @param color The fill color
         */
        void setFillColor(const Color& color);

        /**
         * @brief Set thumb color
         * @param color The thumb color
         */
        void setThumbColor(const Color& color);

        /**
         * @brief Set thumb size
         * @param width Thumb width
         * @param height Thumb height
         */
        void setThumbSize(float width, float height);

        /**
         * @brief Enable/disable showing value label
         * @param show True to show label
         */
        void setShowLabel(bool show);

        /**
         * @brief Set label format (printf style)
         * @param format Format string
         */
        void setLabelFormat(const std::string& format);

        // Event handlers
        bool onMouseClick(float x, float y) override;
        bool onMouseRelease(float x, float y) override;
        bool onMouseMove(float x, float y) override;

        /**
         * @brief Render the slider
         */
        void renderSelf() override;

    protected:
        float _value;
        float _minValue;
        float _maxValue;
        bool _dragging = false;
        bool _showLabel = false;
        std::string _labelFormat = "%.1f";

        float _thumbWidth = 12.0f;
        float _thumbHeight = 20.0f;

        ValueCallback _onChange;

        Color _trackColor = Color(40, 40, 50, 255);
        Color _fillColor = Color(80, 140, 200, 255);
        Color _thumbColor = Color(200, 200, 200, 255);

        /**
         * @brief Calculate value from x position
         * @param x X position relative to widget
         * @return Corresponding value
         */
        float valueFromX(float x) const;

        /**
         * @brief Calculate x position from value
         * @return X position of thumb center
         */
        float xFromValue() const;
    };

} // namespace rtype::ui

#endif /* !SLIDERWIDGET_HPP_ */
