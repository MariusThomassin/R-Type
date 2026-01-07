/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SliderWidget - Interactive slider for value selection
*/

#ifndef SLIDERWIDGET_HPP_
#define SLIDERWIDGET_HPP_

#include "ProgressBarWidget.hpp"
#include <functional>

namespace rtype::ui {

    /**
     * @brief An interactive slider widget
     * 
     * SliderWidget extends ProgressBarWidget to provide interactive
     * value selection through mouse dragging. Users can click and drag
     * to change the slider value, with optional callbacks for value changes.
     */
    class SliderWidget : public ProgressBarWidget {
    public:
        using ValueChangeCallback = std::function<void(float)>;

        /**
         * @brief Construct a new SliderWidget
         * @param value Initial value (0.0 to 1.0)
         */
        explicit SliderWidget(float value = 0.0f);

        ~SliderWidget() override = default;

        /**
         * @brief Set callback for value changes
         * @param callback Function called when value changes
         */
        void setOnValueChange(const ValueChangeCallback& callback);

        /**
         * @brief Set minimum and maximum values for display
         * @param min Minimum value for display/conversion
         * @param max Maximum value for display/conversion
         */
        void setRange(float min, float max);

        /**
         * @brief Get the minimum value
         * @return Minimum value
         */
        float getMinValue() const;

        /**
         * @brief Get the maximum value
         * @return Maximum value
         */
        float getMaxValue() const;

        /**
         * @brief Set value in actual range (converts to 0.0-1.0 internally)
         * @param value Value in the set range
         */
        void setRangeValue(float value);

        /**
         * @brief Get value in actual range (converts from internal 0.0-1.0)
         * @return Value in the set range
         */
        float getRangeValue() const;

        /**
         * @brief Set the handle (thumb) color
         * @param color Handle color
         */
        void setHandleColor(const Color& color);

        /**
         * @brief Get the handle color
         * @return Handle color
         */
        Color getHandleColor() const;

        /**
         * @brief Set handle size
         * @param size Handle size (width for horizontal, height for vertical)
         */
        void setHandleSize(float size);

        /**
         * @brief Get handle size
         * @return Handle size
         */
        float getHandleSize() const;

        /**
         * @brief Enable or disable dragging
         * @param draggable True to allow dragging
         */
        void setDraggable(bool draggable);

        /**
         * @brief Check if slider is draggable
         * @return True if draggable
         */
        bool isDraggable() const;

        /**
         * @brief Render the slider (bar + handle)
         */
        void renderSelf() const override;

        // Event handling overrides
        bool onMouseClick() override;
        bool onMouseMove(float x, float y) override;
        bool onMouseRelease() override;
        bool onMouseEnter() override;
        bool onMouseLeave() override;

    protected:
        /**
         * @brief Update value based on mouse position
         * @param mouseX Mouse X position
         * @param mouseY Mouse Y position
         */
        void updateValueFromMouse(float mouseX, float mouseY);

        /**
         * @brief Calculate handle position based on current value
         * @return Handle rectangle
         */
        Rectangle getHandleRect() const;

        /**
         * @brief Check if mouse is over the handle
         * @param mouseX Mouse X position
         * @param mouseY Mouse Y position
         * @return True if mouse is over handle
         */
        bool isMouseOverHandle(float mouseX, float mouseY) const;

    private:
        /**
         * @brief Callback for value changes
         */
        ValueChangeCallback _onValueChange = nullptr;

        /**
         * @brief Handle color
         */
        Color _handleColor = {200, 200, 200, 255};

        /**
         * @brief Handle size (radius or thickness)
         */
        float _handleSize = 12.0f;

        /**
         * @brief Whether slider is currently being dragged
         */
        bool _isDragging = false;

        /**
         * @brief Whether slider allows dragging
         */
        bool _draggable = true;

        /**
         * @brief Whether mouse is over the handle
         */
        bool _isHandleHovered = false;

        /**
         * @brief Minimum value for range conversion
         */
        float _minValue = 0.0f;

        /**
         * @brief Maximum value for range conversion
         */
        float _maxValue = 100.0f;
    };

} // namespace rtype::ui

#endif /* !SLIDERWIDGET_HPP_ */