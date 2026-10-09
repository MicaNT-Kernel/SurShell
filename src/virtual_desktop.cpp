// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/virtual_desktop.cpp)
// ============================================================================

#include "surshell/virtual_desktop.hpp"
#include <algorithm>
#include <cmath>

namespace surshell {

VirtualDesktopManager::VirtualDesktopManager() {
    desktops_.push_back(VirtualDesktop{
        .id = nextId_++,
        .name = "1: Sovereign NT",
        .windowIds = {}
    });
    desktops_.push_back(VirtualDesktop{
        .id = nextId_++,
        .name = "2: Dev & Tools",
        .windowIds = {}
    });
}

void VirtualDesktopManager::updateLayout(int32_t screenWidth, int32_t screenHeight, int32_t taskbarHeight) {
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;
    taskbarHeight_ = taskbarHeight;

    const size_t totalCards = desktops_.size() + 1; // Desktops + "New Desktop" card
    constexpr int32_t cardWidth = 200;
    constexpr int32_t cardHeight = 135;
    constexpr int32_t cardSpacing = 16;
    constexpr int32_t paddingX = 20;
    constexpr int32_t paddingY = 14;
    constexpr int32_t headerHeight = 28;

    const int32_t contentWidth = static_cast<int32_t>(totalCards) * cardWidth + static_cast<int32_t>(totalCards - 1) * cardSpacing;
    const int32_t stripWidth = std::min(screenWidth - 40, contentWidth + paddingX * 2);
    const int32_t stripHeight = cardHeight + paddingY * 2 + headerHeight;

    const int32_t stripX = (screenWidth - stripWidth) / 2;
    const int32_t stripY = screenHeight - taskbarHeight - stripHeight - 16;

    switcherBounds_ = Rect{stripX, stripY, stripWidth, stripHeight};

    for (size_t i = 0; i < desktops_.size(); ++i) {
        const int32_t curX = stripX + paddingX + static_cast<int32_t>(i) * (cardWidth + cardSpacing);
        const int32_t curY = stripY + paddingY + headerHeight;
        desktops_[i].switcherCardBounds = Rect{curX, curY, cardWidth, cardHeight};
        desktops_[i].closeButtonBounds = Rect{curX + cardWidth - 22, curY + 6, 16, 16};
    }

    const int32_t addX = stripX + paddingX + static_cast<int32_t>(desktops_.size()) * (cardWidth + cardSpacing);
    const int32_t addY = stripY + paddingY + headerHeight;
    addDesktopButtonBounds_ = Rect{addX, addY, cardWidth, cardHeight};

    layoutWindowCards();
}

void VirtualDesktopManager::layoutWindowCards() {
    if (windowsProvider_) {
        activeWindowCards_ = windowsProvider_();
    }

    if (activeWindowCards_.empty()) return;

    const int32_t availX = 60;
    const int32_t availY = 74;
    const int32_t availW = screenWidth_ - 120;
    const int32_t availH = std::max(200, switcherBounds_.y - availY - 24);

    const size_t n = activeWindowCards_.size();
    if (n == 1) {
        const int32_t cardW = 540;
        const int32_t cardH = 340;
        const int32_t startX = availX + (availW - cardW) / 2;
        const int32_t startY = availY + (availH - cardH) / 2;
        activeWindowCards_[0].cardBounds = Rect{startX, startY, cardW, cardH};
        activeWindowCards_[0].closeButtonBounds = Rect{startX + cardW - 24, startY + 6, 18, 18};
    } else if (n == 2) {
        constexpr int32_t cardW = 460;
        constexpr int32_t cardH = 300;
        constexpr int32_t spacing = 28;
        const int32_t totalW = 2 * cardW + spacing;
        const int32_t startX = availX + (availW - totalW) / 2;
        const int32_t startY = availY + (availH - cardH) / 2;
        activeWindowCards_[0].cardBounds = Rect{startX, startY, cardW, cardH};
        activeWindowCards_[0].closeButtonBounds = Rect{startX + cardW - 24, startY + 6, 18, 18};
        activeWindowCards_[1].cardBounds = Rect{startX + cardW + spacing, startY, cardW, cardH};
        activeWindowCards_[1].closeButtonBounds = Rect{startX + cardW + spacing + cardW - 24, startY + 6, 18, 18};
    } else {
        const int32_t cols = std::clamp(static_cast<int32_t>(std::ceil(std::sqrt(n))), 1, 4);
        const int32_t rows = (static_cast<int32_t>(n) + cols - 1) / cols;
        const int32_t cardW = std::clamp((availW - (cols - 1) * 20) / cols, 240, 420);
        const int32_t cardH = std::clamp((availH - (rows - 1) * 20) / rows, 160, 280);
        const int32_t totalGridW = cols * cardW + (cols - 1) * 20;
        const int32_t totalGridH = rows * cardH + (rows - 1) * 20;
        const int32_t gridStartX = availX + (availW - totalGridW) / 2;
        const int32_t gridStartY = availY + (availH - totalGridH) / 2;

        for (size_t i = 0; i < activeWindowCards_.size(); ++i) {
            const int32_t r = static_cast<int32_t>(i) / cols;
            const int32_t c = static_cast<int32_t>(i) % cols;
            const int32_t cx = gridStartX + c * (cardW + 20);
            const int32_t cy = gridStartY + r * (cardH + 20);
            activeWindowCards_[i].cardBounds = Rect{cx, cy, cardW, cardH};
            activeWindowCards_[i].closeButtonBounds = Rect{cx + cardW - 24, cy + 6, 18, 18};
        }
    }
}

void VirtualDesktopManager::switchDesktop(size_t index) {
    if (index >= desktops_.size() || index == activeIndex_) return;
    activeIndex_ = index;
    layoutWindowCards();
    if (onDesktopSwitched_) onDesktopSwitched_(activeIndex_);
}

void VirtualDesktopManager::nextDesktop() {
    if (desktops_.size() <= 1) return;
    switchDesktop((activeIndex_ + 1) % desktops_.size());
}

void VirtualDesktopManager::previousDesktop() {
    if (desktops_.size() <= 1) return;
    switchDesktop((activeIndex_ + desktops_.size() - 1) % desktops_.size());
}

uint32_t VirtualDesktopManager::createDesktop(std::string_view name) {
    const uint32_t id = nextId_++;
    std::string deskName = name.empty() ? ("Desktop " + std::to_string(id)) : std::string(name);

    desktops_.push_back(VirtualDesktop{
        .id = id,
        .name = std::move(deskName),
        .windowIds = {}
    });

    updateLayout(screenWidth_, screenHeight_, taskbarHeight_);
    return id;
}

bool VirtualDesktopManager::removeDesktop(size_t index) {
    if (desktops_.size() <= 1 || index >= desktops_.size()) return false;

    const size_t targetIdx = (index == 0) ? 1 : 0;
    for (uint32_t wid : desktops_[index].windowIds) {
        desktops_[targetIdx].windowIds.insert(wid);
    }

    desktops_.erase(desktops_.begin() + static_cast<ptrdiff_t>(index));
    if (activeIndex_ >= desktops_.size()) {
        activeIndex_ = desktops_.size() - 1;
    }

    updateLayout(screenWidth_, screenHeight_, taskbarHeight_);
    if (onDesktopSwitched_) onDesktopSwitched_(activeIndex_);
    return true;
}

void VirtualDesktopManager::assignWindowToDesktop(uint32_t windowId, size_t desktopIndex) {
    if (desktopIndex >= desktops_.size()) return;
    unassignWindow(windowId);
    desktops_[desktopIndex].windowIds.insert(windowId);
    layoutWindowCards();
    if (onDesktopSwitched_) onDesktopSwitched_(activeIndex_);
}

void VirtualDesktopManager::unassignWindow(uint32_t windowId) {
    for (auto& d : desktops_) {
        d.windowIds.erase(windowId);
    }
    layoutWindowCards();
    if (onDesktopSwitched_) onDesktopSwitched_(activeIndex_);
}

void VirtualDesktopManager::pinWindowToAllDesktops(uint32_t windowId, bool pinned) {
    if (pinned) {
        pinnedWindows_.insert(windowId);
    } else {
        pinnedWindows_.erase(windowId);
    }
    layoutWindowCards();
    if (onDesktopSwitched_) onDesktopSwitched_(activeIndex_);
}

bool VirtualDesktopManager::isWindowVisible(uint32_t windowId) const {
    if (pinnedWindows_.contains(windowId)) return true;
    if (activeIndex_ < desktops_.size()) {
        return desktops_[activeIndex_].windowIds.contains(windowId);
    }
    return true;
}

bool VirtualDesktopManager::onMouseDown(Point pt, MouseButton btn) {
    if (!isSwitcherVisible_) return false;

    // 1. Check window cards in upper area
    for (const auto& wc : activeWindowCards_) {
        if (wc.closeButtonBounds.contains(pt)) {
            if (btn == MouseButton::Left && windowCloseCallback_) {
                windowCloseCallback_(wc.windowId);
            }
            layoutWindowCards();
            return true;
        }
        if (wc.cardBounds.contains(pt)) {
            if (btn == MouseButton::Right) {
                // Quick-move window to next desktop on right click
                if (!desktops_.empty()) {
                    const size_t targetIdx = (activeIndex_ + 1) % desktops_.size();
                    assignWindowToDesktop(wc.windowId, targetIdx);
                    layoutWindowCards();
                }
                return true;
            }
            if (btn == MouseButton::Left) {
                dragCandidateWindowId_ = wc.windowId;
                dragStartPt_ = pt;
                dragCurrentPt_ = pt;
                isDraggingWindow_ = false;
                dropTargetDesktopIndex_ = -1;
                dropTargetIsNewDesktop_ = false;
                return true;
            }
        }
    }

    // 2. Check desktop close button [x]
    for (size_t i = 0; i < desktops_.size(); ++i) {
        if (desktops_.size() > 1 && desktops_[i].closeButtonBounds.contains(pt)) {
            if (btn == MouseButton::Left) {
                removeDesktop(i);
            }
            return true;
        }
        if (desktops_[i].switcherCardBounds.contains(pt)) {
            if (btn == MouseButton::Left) {
                switchDesktop(i);
                hideSwitcher();
            }
            return true;
        }
    }

    // 3. Check Add Desktop button
    if (addDesktopButtonBounds_.contains(pt)) {
        if (btn == MouseButton::Left) {
            createDesktop();
        }
        return true;
    }

    // 4. Click outside collapses Task View
    if (!switcherBounds_.contains(pt)) {
        hideSwitcher();
        return false;
    }

    return true;
}

bool VirtualDesktopManager::onMouseMove(Point pt) {
    if (!isSwitcherVisible_) return false;

    if (dragCandidateWindowId_ != 0) {
        const int32_t dx = pt.x - dragStartPt_.x;
        const int32_t dy = pt.y - dragStartPt_.y;
        if (std::abs(dx) > 6 || std::abs(dy) > 6) {
            isDraggingWindow_ = true;
            dragCurrentPt_ = pt;

            dropTargetDesktopIndex_ = -1;
            dropTargetIsNewDesktop_ = false;
            for (size_t i = 0; i < desktops_.size(); ++i) {
                if (desktops_[i].switcherCardBounds.contains(pt)) {
                    dropTargetDesktopIndex_ = static_cast<int32_t>(i);
                    break;
                }
            }
            if (dropTargetDesktopIndex_ == -1 && addDesktopButtonBounds_.contains(pt)) {
                dropTargetIsNewDesktop_ = true;
            }
            return true;
        }
    }

    hoveredWindowId_ = -1;
    hoveredWindowCloseId_ = -1;
    for (const auto& wc : activeWindowCards_) {
        if (wc.closeButtonBounds.contains(pt)) {
            hoveredWindowCloseId_ = static_cast<int32_t>(wc.windowId);
            hoveredWindowId_ = static_cast<int32_t>(wc.windowId);
            return true;
        }
        if (wc.cardBounds.contains(pt)) {
            hoveredWindowId_ = static_cast<int32_t>(wc.windowId);
            return true;
        }
    }

    hoveredDesktopIndex_ = -1;
    hoveredDesktopCloseIndex_ = -1;
    for (size_t i = 0; i < desktops_.size(); ++i) {
        if (desktops_[i].closeButtonBounds.contains(pt)) {
            hoveredDesktopCloseIndex_ = static_cast<int32_t>(i);
            hoveredDesktopIndex_ = static_cast<int32_t>(i);
            return true;
        }
        if (desktops_[i].switcherCardBounds.contains(pt)) {
            hoveredDesktopIndex_ = static_cast<int32_t>(i);
            return true;
        }
    }

    isAddDesktopHovered_ = addDesktopButtonBounds_.contains(pt);
    return isAddDesktopHovered_ || switcherBounds_.contains(pt);
}

bool VirtualDesktopManager::onMouseUp(Point pt, MouseButton btn) {
    (void)pt;
    if (!isSwitcherVisible_ || btn != MouseButton::Left) return false;

    if (dragCandidateWindowId_ != 0) {
        const uint32_t wid = dragCandidateWindowId_;
        dragCandidateWindowId_ = 0;

        if (isDraggingWindow_) {
            isDraggingWindow_ = false;
            if (dropTargetDesktopIndex_ >= 0 && static_cast<size_t>(dropTargetDesktopIndex_) < desktops_.size()) {
                assignWindowToDesktop(wid, static_cast<size_t>(dropTargetDesktopIndex_));
            } else if (dropTargetIsNewDesktop_) {
                createDesktop();
                assignWindowToDesktop(wid, desktops_.size() - 1);
            }
            dropTargetDesktopIndex_ = -1;
            dropTargetIsNewDesktop_ = false;
            layoutWindowCards();
            return true;
        } else {
            // Simple click without drag -> select window and hide Task View
            if (windowSelectCallback_) {
                windowSelectCallback_(wid);
            }
            hideSwitcher();
            return true;
        }
    }
    return false;
}

void VirtualDesktopManager::renderSwitcher(Surface& surface, const ThemePalette& theme) {
    if (!isSwitcherVisible_) return;

    // Refresh layout for window cards
    layoutWindowCards();

    // 1. Full-Screen Ambient Acrylic Wash
    surface.applyAcrylicTint(Rect{0, 0, screenWidth_, screenHeight_}, Color::fromRgba(8, 12, 22, 175), 4);

    // 2. Top Header Bar
    IconRenderer::draw(surface, IconId::TaskView, Rect{40, 22, 24, 24}, theme.prismAccent);
    surface.drawString(74, 22, "Task View", theme.prismAccent, 2);

    const std::string activeDeskName = (activeIndex_ < desktops_.size()) ? desktops_[activeIndex_].name : "Desktop";
    const std::string subtitle = "|  Active: " + activeDeskName +
                                 "  |  " + std::to_string(activeWindowCards_.size()) + " Open Windows  |  Select window or switch workspace below";
    surface.drawString(235, 29, subtitle, theme.textSecondary, 1);

    // 3. Open Windows Grid (Upper Area)
    for (const auto& wc : activeWindowCards_) {
        const bool isHov = (static_cast<int32_t>(wc.windowId) == hoveredWindowId_);

        surface.drawDropShadow(wc.cardBounds, 18, 0.45f);

        const Color cardBg = isHov ? Color::fromRgba(24, 35, 56, 240) : Color::fromRgba(16, 24, 40, 225);
        surface.drawRoundedRect(wc.cardBounds, 10, cardBg, true);
        surface.drawRoundedRect(wc.cardBounds, 10, isHov ? theme.prismAccent : Color::fromRgba(42, 58, 84, 180), false);

        // Header: Icon + Title + Close Button
        IconRenderer::draw(surface, wc.iconId, Rect{wc.cardBounds.x + 10, wc.cardBounds.y + 8, 18, 18});

        std::string title = wc.title;
        if (title.size() > 32) title = title.substr(0, 30) + "..";
        surface.drawString(wc.cardBounds.x + 36, wc.cardBounds.y + 13, title,
                           isHov ? Color::fromHex(0xFFFFFF) : theme.textPrimary, 1);

        const bool isCloseHov = (static_cast<int32_t>(wc.windowId) == hoveredWindowCloseId_);
        surface.drawRoundedRect(wc.closeButtonBounds, 4, isCloseHov ? Color::fromRgba(220, 40, 50, 240) : Color::fromRgba(160, 35, 45, 160), true);

        const int32_t cx1 = wc.closeButtonBounds.x + 5;
        const int32_t cy1 = wc.closeButtonBounds.y + 5;
        const int32_t span = wc.closeButtonBounds.width - 10;
        for (int32_t k = 0; k < span; ++k) {
            surface.putPixel(cx1 + k, cy1 + k, Color::fromHex(0xFFFFFF));
            surface.putPixel(cx1 + span - 1 - k, cy1 + k, Color::fromHex(0xFFFFFF));
        }

        // Live Client Preview Box
        const Rect prevBox{wc.cardBounds.x + 10, wc.cardBounds.y + 36, wc.cardBounds.width - 20, wc.cardBounds.height - 46};
        surface.fillRect(prevBox, Color::fromRgba(8, 12, 18, 255));
        surface.drawRoundedRect(prevBox, 4, Color::fromRgba(35, 48, 70, 180), false);

        if (wc.previewSurface && wc.previewSurface->width() > 0 && wc.previewSurface->height() > 0) {
            surface.blitScaled(*wc.previewSurface,
                               Rect{0, 0, static_cast<int32_t>(wc.previewSurface->width()), static_cast<int32_t>(wc.previewSurface->height())},
                               prevBox.inflate(-2, -2));
        } else {
            IconRenderer::draw(surface, wc.iconId,
                               Rect{prevBox.centerX() - 20, prevBox.centerY() - 20, 40, 40},
                               Color::fromRgba(80, 105, 140, 200));
        }
    }

    // 4. Bottom Dock: Virtual Desktops Carousel
    surface.drawDropShadow(switcherBounds_, 22, 0.55f);
    surface.applyAcrylicTint(switcherBounds_, Color::fromRgba(12, 18, 30, 235), 8);
    surface.drawRoundedRect(switcherBounds_, 14, Color::fromRgba(42, 58, 86, 220), false);

    surface.drawString(switcherBounds_.x + 22, switcherBounds_.y + 12, "DESKTOPS", theme.prismAccent, 1);
    surface.drawString(switcherBounds_.x + 105, switcherBounds_.y + 12,
                       "|  Drag windows or click to switch workspace", theme.textSecondary, 1);

    // Render Virtual Desktop Cards
    for (size_t i = 0; i < desktops_.size(); ++i) {
        const auto& d = desktops_[i];
        const bool isActive = (i == activeIndex_);
        const bool isHov = (static_cast<int32_t>(i) == hoveredDesktopIndex_);

        const Color cardBg = isActive ? Color::fromRgba(26, 40, 64, 240)
                                      : (isHov ? Color::fromRgba(20, 30, 48, 220)
                                               : Color::fromRgba(16, 24, 38, 200));
        surface.drawRoundedRect(d.switcherCardBounds, 10, cardBg, true);

        if (isActive) {
            surface.drawRoundedRect(d.switcherCardBounds, 10, theme.prismAccent, false);
            surface.drawRoundedRect(d.switcherCardBounds.inflate(-1, -1), 9, theme.prismAccent, false);
            surface.drawRoundedRect(Rect{d.switcherCardBounds.centerX() - 24, d.switcherCardBounds.bottom() - 4, 48, 2}, 1, theme.prismAccent, true);
        } else {
            surface.drawRoundedRect(d.switcherCardBounds, 10, Color::fromRgba(42, 58, 82, 160), false);
        }

        // 16:9 Mini Desktop Thumbnail
        const Rect thumbRect{d.switcherCardBounds.x + 8, d.switcherCardBounds.y + 8, d.switcherCardBounds.width - 16, 76};
        surface.drawVerticalGradient(thumbRect, Color::fromRgba(20, 32, 54, 255), Color::fromRgba(10, 16, 28, 255));
        surface.drawRoundedRect(thumbRect, 4, Color::fromRgba(45, 65, 95, 180), false);

        // Mini Taskbar dock at bottom of thumbnail
        surface.fillRect(Rect{thumbRect.x, thumbRect.bottom() - 4, thumbRect.width, 4}, Color::fromRgba(15, 22, 35, 230));
        surface.fillRect(Rect{thumbRect.centerX() - 10, thumbRect.bottom() - 3, 20, 2}, theme.prismAccent);

        // Mini Windows representation
        if (d.windowIds.empty()) {
            surface.drawString(thumbRect.centerX() - 20, thumbRect.centerY() - 4, "Empty", Color::fromRgba(90, 110, 140, 180), 1);
        } else {
            // Render proportional miniature window frames on the mini desktop
            surface.drawRoundedRect(Rect{thumbRect.x + 8, thumbRect.y + 8, thumbRect.width * 55 / 100, thumbRect.height - 20},
                                    2, Color::fromRgba(25, 38, 60, 230), true);
            surface.drawRoundedRect(Rect{thumbRect.x + 8, thumbRect.y + 8, thumbRect.width * 55 / 100, thumbRect.height - 20},
                                    2, theme.prismAccent, false);

            surface.drawRoundedRect(Rect{thumbRect.x + thumbRect.width * 62 / 100, thumbRect.y + 12, thumbRect.width * 32 / 100, thumbRect.height - 26},
                                    2, Color::fromRgba(20, 30, 48, 230), true);
            surface.drawRoundedRect(Rect{thumbRect.x + thumbRect.width * 62 / 100, thumbRect.y + 12, thumbRect.width * 32 / 100, thumbRect.height - 26},
                                    2, Color::fromRgba(60, 85, 120, 200), false);
        }

        // Drop target indicator when dragging window
        if (dropTargetDesktopIndex_ == static_cast<int32_t>(i)) {
            surface.drawRoundedRect(d.switcherCardBounds, 10, theme.prismAccent, false);
            surface.drawRoundedRect(d.switcherCardBounds.inflate(2, 2), 12, Color::fromRgba(0, 180, 216, 180), false);
            const Rect dropBadge{d.switcherCardBounds.centerX() - 48, d.switcherCardBounds.y + 40, 96, 22};
            surface.drawRoundedRect(dropBadge, 6, Color::fromRgba(0, 180, 216, 230), true);
            surface.drawString(dropBadge.x + 8, dropBadge.y + 5, "DROP TO MOVE", Color::fromHex(0x06090F), 1);
        }

        // Close Button [x] for non-primary desktop
        if (desktops_.size() > 1) {
            const bool isCloseHov = (static_cast<int32_t>(i) == hoveredDesktopCloseIndex_);
            surface.drawRoundedRect(d.closeButtonBounds, 4, isCloseHov ? Color::fromRgba(220, 40, 50, 240) : Color::fromRgba(160, 35, 45, 140), true);
            const int32_t dcx = d.closeButtonBounds.x + 4;
            const int32_t dcy = d.closeButtonBounds.y + 4;
            const int32_t dspan = d.closeButtonBounds.width - 8;
            for (int32_t k = 0; k < dspan; ++k) {
                surface.putPixel(dcx + k, dcy + k, Color::fromHex(0xFFFFFF));
                surface.putPixel(dcx + dspan - 1 - k, dcy + k, Color::fromHex(0xFFFFFF));
            }
        }

        // Desktop Name (centered, generous 200px width prevents collision)
        const std::string dispName = d.name;
        const int32_t textW = static_cast<int32_t>(dispName.size()) * 8;
        const int32_t nameX = d.switcherCardBounds.centerX() - textW / 2;
        surface.drawString(nameX, d.switcherCardBounds.y + 90, dispName,
                           isActive ? Color::fromHex(0xFFFFFF) : theme.textPrimary, 1);

        // Window Count Subtitle
        const std::string countStr = std::to_string(d.windowIds.size()) + " Windows";
        const int32_t countW = static_cast<int32_t>(countStr.size()) * 8;
        surface.drawString(d.switcherCardBounds.centerX() - countW / 2, d.switcherCardBounds.y + 108, countStr, theme.textSecondary, 1);
    }

    // Add Desktop Button Card
    const bool isAddHighlight = isAddDesktopHovered_ || dropTargetIsNewDesktop_;
    surface.drawRoundedRect(addDesktopButtonBounds_, 10,
                            isAddHighlight ? Color::fromRgba(24, 34, 56, 230) : Color::fromRgba(16, 24, 38, 190), true);
    surface.drawRoundedRect(addDesktopButtonBounds_, 10,
                            isAddHighlight ? theme.prismAccent : Color::fromRgba(42, 58, 82, 160), false);
    if (dropTargetIsNewDesktop_) {
        surface.drawRoundedRect(addDesktopButtonBounds_.inflate(2, 2), 12, Color::fromRgba(0, 180, 216, 180), false);
    }

    // Vector Plus Icon
    const int32_t pcx = addDesktopButtonBounds_.centerX();
    const int32_t pcy = addDesktopButtonBounds_.y + 44;
    surface.fillRect(Rect{pcx - 12, pcy - 2, 24, 4}, theme.prismAccent);
    surface.fillRect(Rect{pcx - 2, pcy - 12, 4, 24}, theme.prismAccent);

    surface.drawString(addDesktopButtonBounds_.centerX() - 44, addDesktopButtonBounds_.y + 94,
                       dropTargetIsNewDesktop_ ? "Move to New" : "New Desktop",
                       isAddHighlight ? Color::fromHex(0xFFFFFF) : theme.textPrimary, 1);

    // 5. Floating Dragged Window Ghost Card
    if (isDraggingWindow_ && dragCandidateWindowId_ != 0) {
        const TaskViewWindowCard* draggedCard = nullptr;
        for (const auto& wc : activeWindowCards_) {
            if (wc.windowId == dragCandidateWindowId_) {
                draggedCard = &wc;
                break;
            }
        }
        if (draggedCard) {
            constexpr int32_t ghostW = 240;
            constexpr int32_t ghostH = 140;
            const Rect ghostRect{dragCurrentPt_.x - ghostW / 2, dragCurrentPt_.y - ghostH / 2, ghostW, ghostH};
            surface.drawDropShadow(ghostRect, 18, 0.65f);
            surface.drawRoundedRect(ghostRect, 8, Color::fromRgba(14, 22, 36, 230), true);
            surface.drawRoundedRect(ghostRect, 8, theme.prismAccent, false);

            IconRenderer::draw(surface, draggedCard->iconId, Rect{ghostRect.x + 8, ghostRect.y + 8, 18, 18});
            std::string dtitle = draggedCard->title;
            if (dtitle.size() > 22) dtitle = dtitle.substr(0, 20) + "..";
            surface.drawString(ghostRect.x + 32, ghostRect.y + 12, dtitle, Color::fromHex(0xFFFFFF), 1);

            const Rect gPrev{ghostRect.x + 8, ghostRect.y + 32, ghostW - 16, ghostH - 40};
            surface.fillRect(gPrev, Color::fromRgba(8, 12, 18, 255));
            surface.drawRoundedRect(gPrev, 4, Color::fromRgba(40, 60, 90, 180), false);
            if (draggedCard->previewSurface && draggedCard->previewSurface->width() > 0 && draggedCard->previewSurface->height() > 0) {
                surface.blitScaled(*draggedCard->previewSurface,
                                   Rect{0, 0, static_cast<int32_t>(draggedCard->previewSurface->width()), static_cast<int32_t>(draggedCard->previewSurface->height())},
                                   gPrev.inflate(-2, -2));
            } else {
                IconRenderer::draw(surface, draggedCard->iconId,
                                   Rect{gPrev.centerX() - 14, gPrev.centerY() - 14, 28, 28},
                                   Color::fromRgba(80, 105, 140, 200));
            }
        }
    }
}

} // namespace surshell
