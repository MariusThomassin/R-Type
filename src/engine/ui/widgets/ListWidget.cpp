/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ListWidget implementation
*/

#include "ListWidget.hpp"
#include <raylib.h>
#include <algorithm>
#include <cmath>

namespace rtype::ui {

    ListItem ListWidget::s_emptyItem = {"", "", false, Color::White()};

    ListWidget::ListWidget()
    {
        _m_transform.width = 200.0f;
        _m_transform.height = 150.0f;
        
        _m_style.backgroundColor = Color(30, 30, 45, 255);
        _m_style.borderWidth = 1.0f;
        _m_style.borderColor = Color(60, 60, 80, 255);
    }

    int ListWidget::addItem(const std::string& text, const std::string& id)
    {
        ListItem item;
        item.text = text;
        item.id = id.empty() ? text : id;
        item.enabled = true;
        item.textColor = Color::White();
        _items.push_back(item);
        return static_cast<int>(_items.size()) - 1;
    }

    int ListWidget::addItem(const ListItem& item)
    {
        _items.push_back(item);
        return static_cast<int>(_items.size()) - 1;
    }

    void ListWidget::removeItem(int index)
    {
        if (index >= 0 && index < static_cast<int>(_items.size())) {
            _items.erase(_items.begin() + index);
            if (_selectedIndex == index) {
                _selectedIndex = -1;
            } else if (_selectedIndex > index) {
                _selectedIndex--;
            }
        }
    }

    void ListWidget::clearItems()
    {
        _items.clear();
        _selectedIndex = -1;
        _hoveredIndex = -1;
        _scrollOffset = 0.0f;
    }

    const ListItem& ListWidget::getItem(int index) const
    {
        if (index >= 0 && index < static_cast<int>(_items.size())) {
            return _items[index];
        }
        return s_emptyItem;
    }

    int ListWidget::getItemCount() const
    {
        return static_cast<int>(_items.size());
    }

    void ListWidget::setSelectedIndex(int index)
    {
        if (index < -1) index = -1;
        if (index >= static_cast<int>(_items.size())) index = static_cast<int>(_items.size()) - 1;
        
        if (_selectedIndex != index) {
            _selectedIndex = index;
            if (_onSelect && index >= 0) {
                _onSelect(index, _items[index]);
            }
        }
    }

    int ListWidget::getSelectedIndex() const
    {
        return _selectedIndex;
    }

    const ListItem& ListWidget::getSelectedItem() const
    {
        return getItem(_selectedIndex);
    }

    void ListWidget::setOnSelect(SelectionCallback callback)
    {
        _onSelect = std::move(callback);
    }

    void ListWidget::setItemHeight(float height)
    {
        _itemHeight = std::max(16.0f, height);
    }

    float ListWidget::getItemHeight() const
    {
        return _itemHeight;
    }

    void ListWidget::setScrollOffset(float offset)
    {
        _scrollOffset = offset;
        clampScroll();
    }

    float ListWidget::getScrollOffset() const
    {
        return _scrollOffset;
    }

    float ListWidget::getContentHeight() const
    {
        return static_cast<float>(_items.size()) * _itemHeight;
    }

    void ListWidget::setItemColors(const Color& normal, const Color& hover, const Color& selected)
    {
        _itemNormalColor = normal;
        _itemHoverColor = hover;
        _itemSelectedColor = selected;
    }

    void ListWidget::setShowScrollbar(bool show)
    {
        _showScrollbar = show;
    }

    bool ListWidget::onMouseClick(float x, float y)
    {
        if (!_m_enabled) return false;

        auto transform = getAbsoluteTransform();
        float localY = y - transform.y + _scrollOffset;
        int index = getItemAtY(localY);
        
        if (index >= 0 && index < static_cast<int>(_items.size())) {
            if (_items[index].enabled) {
                setSelectedIndex(index);
                return true;
            }
        }
        return false;
    }

    bool ListWidget::onMouseWheel(float delta)
    {
        if (!_m_enabled) return false;

        _scrollOffset -= delta * 30.0f;
        clampScroll();
        return true;
    }

    bool ListWidget::onMouseMove(float x, float y)
    {
        if (!_m_enabled) return false;

        auto transform = getAbsoluteTransform();
        float localY = y - transform.y + _scrollOffset;
        _hoveredIndex = getItemAtY(localY);
        
        return false;  // Don't consume move events
    }

    int ListWidget::getItemAtY(float y) const
    {
        if (y < 0) return -1;
        int index = static_cast<int>(y / _itemHeight);
        if (index >= static_cast<int>(_items.size())) return -1;
        return index;
    }

    void ListWidget::clampScroll()
    {
        float maxScroll = std::max(0.0f, getContentHeight() - _m_transform.height);
        _scrollOffset = std::clamp(_scrollOffset, 0.0f, maxScroll);
    }

    void ListWidget::renderSelf()
    {
        auto transform = getAbsoluteTransform();
        
        // Background
        DrawRectangle(
            static_cast<int>(transform.x),
            static_cast<int>(transform.y),
            static_cast<int>(transform.width),
            static_cast<int>(transform.height),
            _m_style.backgroundColor.toRaylib()
        );

        // Enable scissor to clip items
        BeginScissorMode(
            static_cast<int>(transform.x),
            static_cast<int>(transform.y),
            static_cast<int>(transform.width),
            static_cast<int>(transform.height)
        );

        // Draw items
        float itemY = transform.y - _scrollOffset;
        float scrollbarWidth = _showScrollbar ? 8.0f : 0.0f;
        float itemWidth = transform.width - scrollbarWidth;

        for (int i = 0; i < static_cast<int>(_items.size()); i++) {
            // Skip if completely outside visible area
            if (itemY + _itemHeight < transform.y) {
                itemY += _itemHeight;
                continue;
            }
            if (itemY > transform.y + transform.height) {
                break;
            }

            const auto& item = _items[i];
            
            // Determine background color
            Color bgColor;
            if (i == _selectedIndex) {
                bgColor = _itemSelectedColor;
            } else if (i == _hoveredIndex && item.enabled) {
                bgColor = _itemHoverColor;
            } else {
                bgColor = _itemNormalColor;
            }

            // Draw item background
            DrawRectangle(
                static_cast<int>(transform.x),
                static_cast<int>(itemY),
                static_cast<int>(itemWidth),
                static_cast<int>(_itemHeight),
                bgColor.toRaylib()
            );

            // Draw item text
            Color textColor = item.enabled ? item.textColor : item.textColor.withAlpha(128);
            if (i == _selectedIndex) {
                textColor = Color::White();
            }
            
            int textY = static_cast<int>(itemY + (_itemHeight - 14) / 2);
            DrawText(
                item.text.c_str(),
                static_cast<int>(transform.x + 8),
                textY,
                14,
                textColor.toRaylib()
            );

            itemY += _itemHeight;
        }

        EndScissorMode();

        // Draw scrollbar if needed
        if (_showScrollbar && getContentHeight() > transform.height) {
            float scrollbarX = transform.x + transform.width - scrollbarWidth;
            float scrollbarHeight = transform.height;
            
            // Scrollbar track
            DrawRectangle(
                static_cast<int>(scrollbarX),
                static_cast<int>(transform.y),
                static_cast<int>(scrollbarWidth),
                static_cast<int>(scrollbarHeight),
                Color(20, 20, 30, 255).toRaylib()
            );

            // Scrollbar thumb
            float visibleRatio = transform.height / getContentHeight();
            float thumbHeight = std::max(20.0f, scrollbarHeight * visibleRatio);
            float thumbMaxY = scrollbarHeight - thumbHeight;
            float scrollRatio = _scrollOffset / std::max(1.0f, getContentHeight() - transform.height);
            float thumbY = transform.y + scrollRatio * thumbMaxY;

            DrawRectangle(
                static_cast<int>(scrollbarX + 1),
                static_cast<int>(thumbY),
                static_cast<int>(scrollbarWidth - 2),
                static_cast<int>(thumbHeight),
                Color(80, 80, 100, 255).toRaylib()
            );
        }

        // Border
        if (_m_style.borderWidth > 0) {
            DrawRectangleLinesEx(
                Rectangle{transform.x, transform.y, transform.width, transform.height},
                _m_style.borderWidth,
                _m_style.borderColor.toRaylib()
            );
        }
    }

} // namespace rtype::ui
