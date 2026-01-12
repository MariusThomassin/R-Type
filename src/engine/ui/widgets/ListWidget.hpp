/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ListWidget - A scrollable list with selectable items
*/

#ifndef LISTWIDGET_HPP_
#define LISTWIDGET_HPP_

#include "src/engine/ui/Widget.hpp"
#include <string>
#include <vector>
#include <functional>

namespace rtype::ui {

    /**
     * @brief A single item in the list
     */
    struct ListItem {
        std::string text;
        std::string id;          // Optional unique identifier
        bool enabled = true;
        Color textColor = Color::White();
    };

    /**
     * @brief A scrollable list widget with selectable items
     * 
     * ListWidget displays a vertical list of items that can be scrolled
     * and selected. Supports single selection mode with callbacks.
     */
    class ListWidget : public Widget {
    public:
        using SelectionCallback = std::function<void(int index, const ListItem& item)>;

        /**
         * @brief Construct a new ListWidget
         */
        ListWidget();

        ~ListWidget() override = default;

        /**
         * @brief Add an item to the list
         * @param text Display text
         * @param id Optional unique identifier
         * @return Index of the added item
         */
        int addItem(const std::string& text, const std::string& id = "");

        /**
         * @brief Add a ListItem directly
         * @param item The item to add
         * @return Index of the added item
         */
        int addItem(const ListItem& item);

        /**
         * @brief Remove item at index
         * @param index Index to remove
         */
        void removeItem(int index);

        /**
         * @brief Clear all items
         */
        void clearItems();

        /**
         * @brief Get item at index
         * @param index The index
         * @return The item (or empty item if invalid)
         */
        const ListItem& getItem(int index) const;

        /**
         * @brief Get number of items
         * @return Item count
         */
        int getItemCount() const;

        /**
         * @brief Set selected index
         * @param index Index to select (-1 for no selection)
         */
        void setSelectedIndex(int index);

        /**
         * @brief Get selected index
         * @return Selected index (-1 if none)
         */
        int getSelectedIndex() const;

        /**
         * @brief Get selected item
         * @return Selected item (or empty item if none)
         */
        const ListItem& getSelectedItem() const;

        /**
         * @brief Set selection callback
         * @param callback Called when selection changes
         */
        void setOnSelect(SelectionCallback callback);

        /**
         * @brief Set item height
         * @param height Height per item in pixels
         */
        void setItemHeight(float height);

        /**
         * @brief Get item height
         * @return Height per item
         */
        float getItemHeight() const;

        /**
         * @brief Set scroll offset (for external scroll control)
         * @param offset Scroll offset in pixels
         */
        void setScrollOffset(float offset);

        /**
         * @brief Get current scroll offset
         * @return Scroll offset in pixels
         */
        float getScrollOffset() const;

        /**
         * @brief Get total content height
         * @return Total height needed to display all items
         */
        float getContentHeight() const;

        /**
         * @brief Set colors for item states
         */
        void setItemColors(const Color& normal, const Color& hover, const Color& selected);

        /**
         * @brief Set whether to show scrollbar
         * @param show True to show scrollbar
         */
        void setShowScrollbar(bool show);

        // Event handlers
        bool onMouseClick(float x, float y) override;
        bool onMouseWheel(float delta) override;
        bool onMouseMove(float x, float y) override;

        /**
         * @brief Render the list
         */
        void renderSelf() override;

    protected:
        std::vector<ListItem> _items;
        int _selectedIndex = -1;
        int _hoveredIndex = -1;
        float _itemHeight = 24.0f;
        float _scrollOffset = 0.0f;
        bool _showScrollbar = true;

        SelectionCallback _onSelect;

        Color _itemNormalColor = Color(35, 35, 50, 255);
        Color _itemHoverColor = Color(50, 50, 70, 255);
        Color _itemSelectedColor = Color(60, 80, 120, 255);

        static ListItem s_emptyItem;

        /**
         * @brief Get item index at y position
         * @param y Y position relative to widget
         * @return Item index or -1 if none
         */
        int getItemAtY(float y) const;

        /**
         * @brief Clamp scroll offset to valid range
         */
        void clampScroll();
    };

} // namespace rtype::ui

#endif /* !LISTWIDGET_HPP_ */
