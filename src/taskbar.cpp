// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/taskbar.cpp)
// ============================================================================

#include "surshell/taskbar.hpp"
#include "surshell/theme.hpp"
#include "surshell/icons.hpp"

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
        const int32_t taskViewBtnW = 38;
        const int32_t taskItemW = 44;
        const int32_t itemGap = 6;
        const int32_t paddingX = 10;
        const int32_t totalTasksW = tasks_.empty() ? 0 : static_cast<int32_t>(tasks_.size()) * (taskItemW + itemGap);
        const int32_t appIslandW = paddingX * 2 + startBtnW + itemGap + taskViewBtnW + (totalTasksW > 0 ? (itemGap + totalTasksW) : 0);

        int32_t appIslandX = 14;
        if (alignment_ == TaskbarAlignment::Center) {
            appIslandX = (static_cast<int32_t>(screenWidth_) - appIslandW) / 2;
        }

        appIslandBounds_ = Rect{appIslandX, tbY + 2, appIslandW, height_ - 4};
        startButtonBounds_ = Rect{appIslandBounds_.x + paddingX, appIslandBounds_.y + (height_ - 4 - btnH) / 2, startBtnW, btnH};
        taskViewButtonBounds_ = Rect{startButtonBounds_.right() + itemGap, startButtonBounds_.y, taskViewBtnW, btnH};

        int32_t curX = taskViewButtonBounds_.right() + itemGap;
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

void Taskbar::addOrUpdateTask(uint32_t windowId, std::string title, std::string glyph, bool active, bool minimized, std::optional<IconId> iconId) {
    IconId resolvedIcon = iconId.value_or(IconId::Terminal);
    if (!iconId) {
        if (glyph == "[E]") resolvedIcon = IconId::FileExplorer;
        else if (glyph == ">_") resolvedIcon = IconId::Terminal;
        else if (glyph == "[T]") resolvedIcon = IconId::TaskManager;
        else if (glyph == "[S]") resolvedIcon = IconId::SentinelSec;
        else if (glyph == "[P]") resolvedIcon = IconId::StartPrism;
        else if (glyph == "[*]") resolvedIcon = IconId::Settings;
        else if (glyph == "[N]") resolvedIcon = IconId::NetBirdMesh;
        else resolvedIcon = IconId::FileGeneric;
    }

    for (auto& task : tasks_) {
        if (task.windowId == windowId) {
            task.title = std::move(title);
            task.iconGlyph = std::move(glyph);
            task.iconId = resolvedIcon;
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
        .iconId = resolvedIcon,
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

Rect Taskbar::hoverPreviewBounds(uint32_t windowId) const noexcept {
    for (const auto& task : tasks_) {
        if (task.windowId == windowId && !task.bounds.empty()) {
            constexpr int32_t CARD_W = 200;
            constexpr int32_t CARD_H = 150;
            int32_t cx = task.bounds.centerX() - CARD_W / 2;
            cx = std::clamp(cx, 10, static_cast<int32_t>(screenWidth_) - CARD_W - 10);
            int32_t cy = task.bounds.y - CARD_H - 8;
            return Rect{cx, cy, CARD_W, CARD_H};
        }
    }
    return Rect{};
}

Rect Taskbar::hoverPreviewCloseButtonBounds(uint32_t windowId) const noexcept {
    const Rect pb = hoverPreviewBounds(windowId);
    if (pb.empty()) return Rect{};
    return Rect{pb.right() - 22, pb.y + 4, 18, 18};
}

void Taskbar::onMouseMove(Point pt) {
    isStartButtonHovered_ = startButtonBounds_.contains(pt);
    isTaskViewHovered_ = taskViewButtonBounds_.contains(pt);

    // Keep preview open if mouse navigates into the floating preview card
    if (hoveredTaskWindowId_ >= 0) {
        const Rect prevCard = hoverPreviewBounds(static_cast<uint32_t>(hoveredTaskWindowId_));
        if (prevCard.contains(pt)) {
            tray_.onMouseMove(pt);
            return;
        }
    }

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

    if (hoveredTaskWindowId_ >= 0) {
        const uint32_t hwId = static_cast<uint32_t>(hoveredTaskWindowId_);
        const Rect closeBtn = hoverPreviewCloseButtonBounds(hwId);
        if (closeBtn.contains(pt)) {
            if (previewCloseCallback_) {
                previewCloseCallback_(hwId);
            }
            hoveredTaskWindowId_ = -1;
            return;
        }
        const Rect prevCard = hoverPreviewBounds(hwId);
        if (prevCard.contains(pt)) {
            if (taskClickCallback_) {
                taskClickCallback_(hwId);
            }
            hoveredTaskWindowId_ = -1;
            return;
        }
    }

    if (startButtonBounds_.contains(pt)) {
        if (startClickCallback_) {
            startClickCallback_();
        }
        return;
    }

    if (taskViewButtonBounds_.contains(pt)) {
        if (taskViewClickCallback_) {
            taskViewClickCallback_();
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
            if (trayClickCallback_) {
                trayClickCallback_();
            }
            return;
        }
    } else {
        const int32_t trayWidth = tray_.preferredWidth();
        const Rect trayRect{static_cast<int32_t>(screenWidth_) - trayWidth - 8, bounds().y + 4, trayWidth, height_ - 8};
        if (trayRect.contains(pt)) {
            tray_.onMouseDown(pt, button);
            if (trayClickCallback_) {
                trayClickCallback_();
            }
            return;
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

        // App Island container with translucent Mica Acrylic sub-surface blur
        surface.applyAcrylicTint(appIslandBounds_, palette.taskbarIslandBg, 8);
        surface.drawRoundedRect(appIslandBounds_, metrics.taskbarIslandRadius, palette.taskbarIslandBorder, false);

        // Tray Island container with translucent Mica Acrylic sub-surface blur
        surface.applyAcrylicTint(trayIslandBounds_, palette.taskbarIslandBg, 8);
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

        // Task View Button (Virtual Desktops)
        Color taskViewBg = isTaskViewHovered_ ? palette.taskbarItemHover : palette.taskbarItemBg;
        surface.drawRoundedRect(taskViewButtonBounds_, 8, taskViewBg, true);
        if (isTaskViewHovered_) {
            surface.drawRoundedRect(taskViewButtonBounds_, 8, palette.accentColor, false);
        }
        const int32_t tvCenterX = taskViewButtonBounds_.centerX();
        const int32_t tvCenterY = taskViewButtonBounds_.centerY();
        surface.drawRoundedRect(Rect{tvCenterX - 7, tvCenterY - 6, 11, 9}, 2, palette.textSecondary, false);
        surface.drawRoundedRect(Rect{tvCenterX - 3, tvCenterY - 3, 11, 9}, 2, palette.accentColor, false);

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

            // Draw procedural vector icon from Sovereign IconPack
            const Rect iconRect{task.bounds.x + (task.bounds.width - 24) / 2, task.bounds.y + (task.bounds.height - 24) / 2, 24, 24};
            IconRenderer::draw(surface, task.iconId, iconRect);

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
            IconRenderer::draw(surface, task.iconId, Point{task.bounds.x + 8, task.bounds.y + (task.bounds.height - 18) / 2}, 18);

            std::string dispTitle = task.title;
            if (dispTitle.size() > 14) dispTitle = dispTitle.substr(0, 12) + "..";
            surface.drawString(task.bounds.x + 32, task.bounds.y + 12, dispTitle, palette.textPrimary, 1);

            if (task.isActive) {
                surface.fillRect(Rect{task.bounds.x + 16, task.bounds.bottom() - 3, task.bounds.width - 32, 2}, palette.taskbarItemIndicator);
            }
        }

        const int32_t trayWidth = tray_.preferredWidth();
        const Rect trayRect{static_cast<int32_t>(screenWidth_) - trayWidth - 8, tbRect.y + 4, trayWidth, height_ - 8};
        tray_.render(surface, trayRect);
    }

    // 4. Render Live Hover Preview Card if hovering a task
    if (hoveredTaskWindowId_ >= 0) {
        renderHoverPreview(surface);
    }
}

void Taskbar::renderHoverPreview(Surface& surface, const Surface* previewSurface) const {
    if (hoveredTaskWindowId_ < 0) return;

    const uint32_t hwId = static_cast<uint32_t>(hoveredTaskWindowId_);
    const TaskItem* targetTask = nullptr;
    for (const auto& task : tasks_) {
        if (task.windowId == hwId) {
            targetTask = &task;
            break;
        }
    }
    if (!targetTask) return;

    const Rect card = hoverPreviewBounds(hwId);
    if (card.empty()) return;

    const auto& palette = ThemeManager::instance().palette();

    // 1. Drop shadow for floating preview card
    surface.drawDropShadow(card, 16, 0.45f);

    // 2. Translucent Mica Acrylic Container
    surface.applyAcrylicTint(card, palette.taskbarIslandBg, 8);
    surface.drawRoundedRect(card, 10, palette.taskbarIslandBorder, false);

    // 3. Header Strip (App Icon + Window Title)
    const Rect iconRect{card.x + 8, card.y + 5, 16, 16};
    IconRenderer::draw(surface, targetTask->iconId, iconRect, palette.accentColor);

    std::string dispTitle = targetTask->title;
    if (dispTitle.size() > 16) {
        dispTitle = dispTitle.substr(0, 14) + "..";
    }
    surface.drawString(card.x + 30, card.y + 9, dispTitle, palette.textPrimary, 1);

    // 4. Close Button [x]
    const Rect closeBtn = hoverPreviewCloseButtonBounds(hwId);
    surface.drawRoundedRect(closeBtn, 4, Color::fromRgba(180, 40, 50, 160), true);
    const int32_t cx1 = closeBtn.x + 5;
    const int32_t cy1 = closeBtn.y + 5;
    const int32_t span = closeBtn.width - 10;
    for (int32_t i = 0; i < span; ++i) {
        surface.putPixel(cx1 + i, cy1 + i, Color::fromHex(0xFFFFFF));
        surface.putPixel(cx1 + span - 1 - i, cy1 + i, Color::fromHex(0xFFFFFF));
    }

    // 5. Downscaled Live Preview Content Box
    const Rect previewBox{card.x + 8, card.y + 26, card.width - 16, card.height - 34};
    surface.fillRect(previewBox, Color::fromRgba(8, 12, 18, 255));
    surface.drawRoundedRect(previewBox, 4, Color::fromRgba(35, 48, 70, 180), false);

    const Surface* srcSurf = previewSurface;
    if (!srcSurf && previewProvider_) {
        srcSurf = previewProvider_(hwId);
    }

    if (srcSurf && srcSurf->width() > 0 && srcSurf->height() > 0) {
        surface.blitScaled(*srcSurf,
                           Rect{0, 0, static_cast<int32_t>(srcSurf->width()), static_cast<int32_t>(srcSurf->height())},
                           previewBox.inflate(-2, -2));
    } else {
        IconRenderer::draw(surface, targetTask->iconId,
                           Rect{previewBox.centerX() - 14, previewBox.centerY() - 14, 28, 28},
                           Color::fromRgba(70, 95, 130, 200));
    }
}

} // namespace surshell
