// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/taskbar.cpp)
// ============================================================================

#include "surshell/taskbar.hpp"
#include "surshell/theme.hpp"

namespace surshell {

Taskbar::Taskbar(uint32_t screenWidth, uint32_t screenHeight)
    : screenWidth_(screenWidth), screenHeight_(screenHeight) {
    const auto& metrics = ThemeManager::instance().metrics();
    height_ = metrics.taskbarHeight;
    style_ = metrics.taskbarStyle;
    alignment_ = metrics.taskbarAlignment;
    recalculateLayout();
}

void Taskbar::setScreenSize(uint32_t width, uint32_t height) {
    screenWidth_ = width;
    screenHeight_ = height;
    recalculateLayout();
}

Rect Taskbar::bounds() const noexcept {
    const int32_t margin = (style_ == TaskbarStyle::FloatingIsland) ? 10 : 0;
    return Rect{0, static_cast<int32_t>(screenHeight_) - height_ - margin, static_cast<int32_t>(screenWidth_), height_ + margin};
}

void Taskbar::recalculateLayout() {
    const int32_t margin = (style_ == TaskbarStyle::FloatingIsland) ? 10 : 0;
    const int32_t tbY = static_cast<int32_t>(screenHeight_) - height_ - margin;
    const int32_t btnH = height_ - 8;

    if (style_ == TaskbarStyle::FloatingIsland) {
        // 1. Calculate Tray Island bounds on the right
        const int32_t trayContentWidth = tray_.preferredWidth();
        const int32_t trayIslandW = trayContentWidth + 24;
        trayIslandBounds_ = Rect{
            static_cast<int32_t>(screenWidth_) - trayIslandW - 14,
            tbY + 2,
            trayIslandW,
            height_ - 4
        };

        // 2. Calculate App Island bounds
        const int32_t startBtnW = 42;
        const int32_t taskItemW = 44;
        const int32_t itemGap = 6;
        const int32_t paddingX = 10;
        const int32_t totalTasksW = tasks_.empty() ? 0 : static_cast<int32_t>(tasks_.size()) * (taskItemW + itemGap);
        const int32_t appIslandW = paddingX * 2 + startBtnW + (totalTasksW > 0 ? (itemGap + totalTasksW) : 0);

        int32_t appIslandX = 14;
        if (alignment_ == TaskbarAlignment::Center) {
            appIslandX = (static_cast<int32_t>(screenWidth_) - appIslandW) / 2;
        }

        appIslandBounds_ = Rect{appIslandX, tbY + 2, appIslandW, height_ - 4};
        startButtonBounds_ = Rect{appIslandBounds_.x + paddingX, appIslandBounds_.y + (height_ - 4 - btnH) / 2, startBtnW, btnH};

        int32_t curX = startButtonBounds_.right() + itemGap;
        for (auto& task : tasks_) {
            task.bounds = Rect{curX, startButtonBounds_.y, taskItemW, btnH};
            curX += taskItemW + itemGap;
        }
    } else {
        // Classic Edge-to-Edge dock
        appIslandBounds_ = Rect{0, tbY, static_cast<int32_t>(screenWidth_), height_};
        trayIslandBounds_ = Rect{0, 0, 0, 0};

        startButtonBounds_ = Rect{8, tbY + 4, 78, btnH};
        int32_t curX = startButtonBounds_.right() + 10;
        const int32_t trayWidth = tray_.preferredWidth();
        const int32_t maxTaskX = static_cast<int32_t>(screenWidth_) - trayWidth - 16;
        const int32_t itemWidth = 140;

        for (auto& task : tasks_) {
            if (curX + itemWidth > maxTaskX) break;
            task.bounds = Rect{curX, tbY + 4, itemWidth, btnH};
            curX += itemWidth + 6;
        }
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

    if (style_ == TaskbarStyle::FloatingIsland) {
        if (trayIslandBounds_.contains(pt)) {
            tray_.onMouseDown(pt, button);
        }
    } else {
        const int32_t trayWidth = tray_.preferredWidth();
        const Rect trayRect{static_cast<int32_t>(screenWidth_) - trayWidth - 8, bounds().y + 4, trayWidth, height_ - 8};
        if (trayRect.contains(pt)) {
            tray_.onMouseDown(pt, button);
        }
    }
}

void Taskbar::render(Surface& surface) {
    const auto& palette = ThemeManager::instance().palette();
    const auto& metrics = ThemeManager::instance().metrics();

    if (style_ == TaskbarStyle::FloatingIsland) {
        // Drop shadow for floating islands
        surface.drawDropShadow(appIslandBounds_, metrics.shadowRadius, 0.40f);
        surface.drawDropShadow(trayIslandBounds_, metrics.shadowRadius, 0.40f);

        // App Island container
        surface.drawRoundedRect(appIslandBounds_, metrics.taskbarIslandRadius, palette.taskbarIslandBg, true);
        surface.drawRoundedRect(appIslandBounds_, metrics.taskbarIslandRadius, palette.taskbarIslandBorder, false);

        // Tray Island container
        surface.drawRoundedRect(trayIslandBounds_, metrics.taskbarIslandRadius, palette.taskbarIslandBg, true);
        surface.drawRoundedRect(trayIslandBounds_, metrics.taskbarIslandRadius, palette.taskbarIslandBorder, false);

        // 1. Modern Mica Prism Start Button
        Color startBg = isStartButtonHovered_ ? palette.taskbarItemHover : palette.taskbarItemBg;
        surface.drawRoundedRect(startButtonBounds_, 8, startBg, true);
        if (isStartButtonHovered_) {
            surface.drawRoundedRect(startButtonBounds_, 8, palette.prismAccent, false);
        }

        // Procedural Vector Prism Start Emblem (Mica Crystal)
        surface.drawPrismLogo(
            Point{startButtonBounds_.center().x, startButtonBounds_.center().y},
            11,
            palette.prismAccent,
            palette.prismFacetDark,
            palette.prismFacetLight
        );

        // 2. Running Task Items (Modern Centered Pill Icons)
        for (const auto& task : tasks_) {
            if (task.bounds.empty()) continue;

            Color itemBg = palette.taskbarItemBg;
            if (task.isActive) {
                itemBg = palette.taskbarItemActive;
            } else if (static_cast<int32_t>(task.windowId) == hoveredTaskWindowId_) {
                itemBg = palette.taskbarItemHover;
            }

            surface.drawRoundedRect(task.bounds, 8, itemBg, true);

            // Draw procedural icon or glyph
            const Rect iconRect{task.bounds.x + 8, task.bounds.y + 7, 26, 26};
            if (task.iconGlyph == "[E]") {
                surface.drawVectorFolder(iconRect, Color::fromHex(0x1E88E5), Color::fromHex(0x64B5F6));
            } else if (task.iconGlyph == ">_") {
                surface.drawVectorTerminal(iconRect, Color::fromHex(0x10141E), palette.accentColor);
            } else if (task.iconGlyph == "[T]") {
                surface.drawVectorTaskMgr(iconRect, Color::fromHex(0x141A28), Color::fromHex(0x00FF9D));
            } else if (task.iconGlyph == "[S]") {
                surface.drawVectorShield(iconRect, Color::fromHex(0x2E7D32), Color::fromHex(0x81C784));
            } else if (task.iconGlyph == "[P]") {
                surface.drawVectorPrismIcon(iconRect, palette.accentColor);
            } else {
                surface.drawString(task.bounds.x + 10, task.bounds.y + 14, task.iconGlyph, palette.accentColor, 1);
            }

            // Active underline indicator
            if (task.isActive) {
                Rect indicator{task.bounds.x + 12, task.bounds.bottom() - 3, task.bounds.width - 24, 2};
                surface.drawRoundedRect(indicator, 1, palette.taskbarItemIndicator, true);
            } else if (!task.isMinimized) {
                Rect indicator{task.bounds.x + (task.bounds.width / 2) - 3, task.bounds.bottom() - 3, 6, 2};
                surface.drawRoundedRect(indicator, 1, palette.textSecondary, true);
            }
        }

        // 3. Render System Tray inside Tray Island
        const Rect innerTrayRect{trayIslandBounds_.x + 10, trayIslandBounds_.y + 2, trayIslandBounds_.width - 20, trayIslandBounds_.height - 4};
        tray_.render(surface, innerTrayRect);

    } else {
        // Classic Edge-to-Edge dock
        const Rect tbRect = bounds();
        surface.fillRect(tbRect, palette.taskbarBg);
        surface.fillRect(Rect{0, tbRect.y, tbRect.width, 1}, palette.taskbarBorderTop);

        Color startBg = isStartButtonHovered_ ? palette.taskbarItemHover : palette.taskbarItemBg;
        surface.drawRoundedRect(startButtonBounds_, 6, startBg, true);
        if (isStartButtonHovered_) {
            surface.drawRoundedRect(startButtonBounds_, 6, palette.accentColor, false);
        }
        surface.drawString(startButtonBounds_.x + 12, startButtonBounds_.y + 14, "MICA", palette.accentColor, 1);

        for (const auto& task : tasks_) {
            if (task.bounds.empty()) continue;
            Color itemBg = task.isActive ? palette.taskbarItemActive : (static_cast<int32_t>(task.windowId) == hoveredTaskWindowId_ ? palette.taskbarItemHover : palette.taskbarItemBg);
            surface.drawRoundedRect(task.bounds, 6, itemBg, true);
            surface.drawString(task.bounds.x + 8, task.bounds.y + 12, task.iconGlyph, palette.accentColor, 1);

            std::string dispTitle = task.title;
            if (dispTitle.size() > 14) dispTitle = dispTitle.substr(0, 12) + "..";
            surface.drawString(task.bounds.x + 36, task.bounds.y + 12, dispTitle, palette.textPrimary, 1);

            if (task.isActive) {
                surface.fillRect(Rect{task.bounds.x + 16, task.bounds.bottom() - 3, task.bounds.width - 32, 2}, palette.taskbarItemIndicator);
            }
        }

        const int32_t trayWidth = tray_.preferredWidth();
        const Rect trayRect{static_cast<int32_t>(screenWidth_) - trayWidth - 8, tbRect.y + 4, trayWidth, height_ - 8};
        tray_.render(surface, trayRect);
    }
}

} // namespace surshell
