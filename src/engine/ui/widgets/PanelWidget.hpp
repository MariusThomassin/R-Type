/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PanelWidget - A container widget with background
*/

#ifndef PANELWIDGET_HPP_
#define PANELWIDGET_HPP_

#include "../Widget.hpp"
#include <string>

namespace rtype::ui {

    /**
     * @brief A container widget with background and border
     * 
     * PanelWidget serves as a container for other widgets, providing
     * a visual background, border, and optional rounded corners.
     * Child widgets are positioned relative to the panel's position.
     */
    class PanelWidget : public Widget {
    public:
        /**
         * @brief Construct a new PanelWidget
         */
        PanelWidget();

        ~PanelWidget() override = default;

        /**
         * @brief Set whether the panel has a header
         * @param hasHeader True to show header
         */
        void setHasHeader(bool hasHeader);

        /**
         * @brief Check if panel has a header
         * @return True if panel has header
         */
        bool hasHeader() const;

        /**
         * @brief Set the header title
         * @param title The title text
         */
        void setTitle(const std::string& title);

        /**
         * @brief Get the header title
         * @return The title text
         */
        const std::string& getTitle() const;

        /**
         * @brief Set the header height
         * @param height Height in pixels
         */
        void setHeaderHeight(float height);

        /**
         * @brief Get the header height
         * @return Height in pixels
         */
        float getHeaderHeight() const;

        /**
         * @brief Set header background color
         * @param color The color
         */
        void setHeaderColor(const Color& color);

        /**
         * @brief Get header background color
         * @return The color
         */
        Color getHeaderColor() const;

        /**
         * @brief Get the content area (panel minus header)
         * @return Content area bounds
         */
        UITransform getContentBounds() const;

        /**
         * @brief Render the panel
         */
        void renderSelf() const override;

    protected:
        /**
         * @brief Whether the panel has a header
         */
        bool _hasHeader = false;
        /**
         * @brief Header title text
         */
        std::string _title;
        /**
         * @brief Header height in pixels
         */
        float _headerHeight = 25.0f;
        /**
         * @brief Header background color
         */
        Color _headerColor = {60, 60, 80, 255};
    };

} // namespace rtype::ui

#endif /* !PANELWIDGET_HPP_ */
