// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/taskbar.cpp)
// ============================================================================

#include "surshell/taskbar.hpp"
#include "surshell/theme.hpp"

namespace surshell {

Taskbar::Taskbar(uint32_t screenWidth, uint32_t screenHeight)
    : screenWidth_(screenWidth), screenHeight_(screenHeight) {
    recalculateLayout();
}

void Taskbar::setScreenSize(uint32_t width, uint32_t height) {
    screenWidth_ = width;
    screenHeight_ = height;
    recalculateLayout();
}

Rect Taskbar::bounds() const noexcept {
    return Rect{0, static_cast<int32_t>(screenHeight_) - height_, static_cast<int32_t>(screenWidth_), height_};
}

void Taskbar::recalculateLayout() {
    const int32_t tbY = static_cast<int32_t>(screenHeight_) - height_;
    startButtonBounds_ = Rect{8, tbY + 4, 78, height_ - 8};

    int32_t curX = startButtonBounds_.right() + 10;
    const int32_t trayWidth = tray_.preferredWidth();
    const int32_t maxTaskX = static_cast<int32_t>(screenWidth_) - trayWidth - 16;
    const int32_t itemWidth = 160;

    for (auto& task : tasks_) {
        if (curX + itemWidth > maxTaskX) break;
        task.bounds = Rect{curX, tbY + 4, itemWidth, height_ - 8};
        curX += itemWidth + 6;
    }
}

void Taskbar::addOrUpdateTask(uint32_t windowId, std::string title, std::string glyph, bool active, bool minimized) {
    for (auto& task : tasks_) {
        if (task.windowId == windowId) {
            task.title = std::move(title);
            task.iconGlyph = std::move(glyph);
            task.isActive = active;
            task.isMinimized = minimized;
            recalculateLayout();
            return;
        }
    }

    tasks_.push_back(TaskItem{
        .windowId = windowId,
        .title = std::move(title),
        .iconGlyph = std::move(glyph),
        .isActive = active,
        .isMinimized = minimized,
        .bounds = Rect{0, 0, 0, 0}
    });

    recalculateLayout();
}

void Taskbar::removeTask(uint32_t windowId) {
    std::erase_if(tasks_, [&](const TaskItem& t) { return t.windowId == windowId; });
    recalculateLayout();
}

void Taskbar::setActiveTask(uint32_t windowId) {
    for (auto& task : tasks_) {
        task.isActive = (task.windowId == windowId);
    }
}

void Taskbar::onMouseMove(Point pt) {
    isStartButtonHovered_ = startButtonBounds_.contains(pt);

    hoveredTaskWindowId_ = -1;
    for (const auto& task : tasks_) {
        if (task.bounds.contains(pt)) {
            hoveredTaskWindowId_ = static_cast<int32_t>(task.windowId);
            break;
        }
    }

    tray_.onMouseMove(pt);
}

void Taskbar::onMouseDown(Point pt, MouseButton button) {
    if (button != MouseButton::Left) return;

    if (startButtonBounds_.contains(pt)) {
        if (startClickCallback_) {
            startClickCallback_();
        }
        return;
    }

    for (const auto& task : tasks_) {
        if (task.bounds.contains(pt)) {
            if (taskClickCallback_) {
                taskClickCallback_(task.windowId);
            }
            return;
        }
    }

    const int32_t trayWidth = tray_.preferredWidth();
    const Rect trayRect{static_cast<int32_t>(screenWidth_) - trayWidth - 8, bounds().y + 4, trayWidth, height_ - 8};
    if (trayRect.contains(pt)) {
        tray_.onMouseDown(pt, button);
    }
}

void Taskbar::render(Surface& surface) {
    const auto& palette = ThemeManager::instance().palette();
    const Rect tbRect = bounds();

    // 1. Taskbar Base Background with Translucent Acrylic/Mica
    surface.fillRect(tbRect, palette.taskbarBg);
    surface.fillRect(Rect{0, tbRect.y, tbRect.width, 1}, palette.taskbarBorderTop);

    // 2. Start Button (Dave Cutler DEC Prism Heritage)
    Color startBg = isStartButtonHovered_ ? Color::fromRgba(38, 54, 82, 220) : Color::fromRgba(24, 34, 52, 180);
    surface.drawRoundedRect(startButtonBounds_, 6, startBg, true);
    if (isStartButtonHovered_) {
        surface.drawRoundedRect(startButtonBounds_, 6, palette.accentColor, false);
    }

    // Mini Prism Logo (Triangle/Prism mark + text)
    surface.drawString(startButtonBounds_.x + 12, startButtonBounds_.y + 12, "MICA", palette.accentColor, 1);

    // 3. Running Task Items
    for (const auto& task : tasks_) {
        if (task.bounds.empty()) continue;

        Color itemBg = palette.taskbarItemBg;
        if (task.isActive) {
            itemBg = palette.taskbarItemActive;
        } else if (static_cast<int32_t>(task.windowId) == hoveredTaskWindowId_) {
            itemBg = palette.taskbarItemHover;
        }

        surface.drawRoundedRect(task.bounds, 6, itemBg, true);

        // Icon Glyph
        surface.drawString(task.bounds.x + 8, task.bounds.y + 12, task.iconGlyph, palette.accentColor, 1);

        // Truncated Window Title
        std::string dispTitle = task.title;
        if (dispTitle.size() > 14) {
            dispTitle = dispTitle.substr(0, 12) + "..";
        }
        surface.drawString(task.bounds.x + 36, task.bounds.y + 12, dispTitle, palette.textPrimary, 1);

        // Active Underline Indicator Bar
        if (task.isActive) {
            Rect indicator{task.bounds.x + 16, task.bounds.bottom() - 3, task.bounds.width - 32, 2};
            surface.fillRect(indicator, palette.taskbarItemIndicator);
        } else if (!task.isMinimized) {
            Rect indicator{task.bounds.x + (task.bounds.width / 2) - 8, task.bounds.bottom() - 2, 16, 2};
            surface.fillRect(indicator, palette.textSecondary);
        }
    }

    // 4. System Tray
    const int32_t trayWidth = tray_.preferredWidth();
    const Rect trayRect{static_cast<int32_t>(screenWidth_) - trayWidth - 8, tbRect.y + 4, trayWidth, height_ - 8};
    tray_.render(surface, trayRect);
}

} // namespace surshell
