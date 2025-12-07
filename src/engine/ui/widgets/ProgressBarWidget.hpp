/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ProgressBarWidget - A progress indicator widget
*/

#ifndef PROGRESSBARWIDGET_HPP_
#define PROGRESSBARWIDGET_HPP_

#include "../Widget.hpp"
#include <string>

namespace rtype::ui {

    /**
     * @brief Progress bar orientation
     */
    enum class ProgressOrientation {
        Horizontal,
        Vertical
    };

    /**
     * @brief A progress bar widget
     * 
     * ProgressBarWidget displays a visual progress indicator with
     * customizable colors for the fill and background. Supports
     * both horizontal and vertical orientations.
     */
    class ProgressBarWidget : public Widget {
    public:
        /**
         * @brief Construct a new ProgressBarWidget
         * @param value Initial progress value (0.0 to 1.0)
         */
        explicit ProgressBarWidget(float value = 0.0f);

        ~ProgressBarWidget() override = default;

        /**
         * @brief Set the progress value
         * @param value Progress from 0.0 to 1.0
         */
        void setValue(float value);

        /**
         * @brief Get the current progress value
         * @return Progress from 0.0 to 1.0
         */
        float getValue() const;

        /**
         * @brief Set the fill color
         * @param color The fill color
         */
        void setFillColor(const Color& color);

        /**
         * @brief Get the fill color
         * @return The fill color
         */
        Color getFillColor() const;

        /**
         * @brief Set the bar orientation
         * @param orientation Horizontal or Vertical
         */
        void setOrientation(ProgressOrientation orientation);

        /**
         * @brief Get the bar orientation
         * @return Current orientation
         */
        ProgressOrientation getOrientation() const;

        /**
         * @brief Enable or disable label display
         * @param showLabel True to show percentage label
         */
        void setShowLabel(bool showLabel);

        /**
         * @brief Check if label is shown
         * @return True if label is displayed
         */
        bool isShowLabel() const;

        /**
         * @brief Set custom label format (printf style)
         * @param format Format string (e.g., "%.0f%%")
         */
        void setLabelFormat(const std::string& format);

        /**
         * @brief Get the label format
         * @return Format string
         */
        const std::string& getLabelFormat() const;

        /**
         * @brief Render the progress bar
         */
        void renderSelf() const override;

    protected:
        /**
         * @brief Current progress value (0.0 to 1.0)
         */
        float _value = 0.0f;
        /**
         * @brief Fill color of the progress bar
         */
        Color _fillColor = {80, 160, 80, 255};
        /**
         * @brief Orientation of the progress bar
         */
        ProgressOrientation _orientation = ProgressOrientation::Horizontal;
        /**
         * @brief Whether to show the label
         */
        bool _showLabel = false;
        /**
         * @brief Label format string
         */
        std::string _labelFormat = "%.0f%%";
    };

} // namespace rtype::ui

#endif /* !PROGRESSBARWIDGET_HPP_ */
