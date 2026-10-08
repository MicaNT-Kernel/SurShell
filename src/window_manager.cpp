// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/window_manager.cpp)
// ============================================================================

#include "surshell/window_manager.hpp"
#include "surshell/theme.hpp"

namespace surshell {

Rect WindowFrame::captionBounds() const noexcept {
    const auto& metrics = ThemeManager::instance().metrics();
    return Rect{currentBounds.x, currentBounds.y, currentBounds.width, metrics.captionHeight};
}

Rect WindowFrame::minButtonBounds() const noexcept {
    const auto& metrics = ThemeManager::instance().metrics();
    return Rect{currentBounds.right() - 96, currentBounds.y + 4, 28, metrics.captionHeight - 8};
}

Rect WindowFrame::maxButtonBounds() const noexcept {
    const auto& metrics = ThemeManager::instance().metrics();
    return Rect{currentBounds.right() - 64, currentBounds.y + 4, 28, metrics.captionHeight - 8};
}

Rect WindowFrame::closeButtonBounds() const noexcept {
    const auto& metrics = ThemeManager::instance().metrics();
    return Rect{currentBounds.right() - 32, currentBounds.y + 4, 28, metrics.captionHeight - 8};
}

Rect WindowFrame::clientAreaBounds() const noexcept {
    const auto& metrics = ThemeManager::instance().metrics();
    const int32_t b = metrics.windowBorderWidth;
    return Rect{
        currentBounds.x + b,
        currentBounds.y + metrics.captionHeight,
        currentBounds.width - (b * 2),
        currentBounds.height - metrics.captionHeight - b
    };
}

HitTestResult WindowFrame::hitTest(Point pt) const noexcept {
    if (!isVisible || state == WindowState::Minimized) return HitTestResult::None;
    if (!currentBounds.contains(pt)) return HitTestResult::None;

    if (closeButtonBounds().contains(pt)) return HitTestResult::CloseButton;
    if (maxButtonBounds().contains(pt)) return HitTestResult::MaxButton;
    if (minButtonBounds().contains(pt)) return HitTestResult::MinButton;

    // Resizing borders (only in Normal state)
    if (state == WindowState::Normal) {
        constexpr int32_t BORDER_RESIZE = 5;
        const bool onLeft = (pt.x <= currentBounds.x + BORDER_RESIZE);
        const bool onRight = (pt.x >= currentBounds.right() - BORDER_RESIZE);
        const bool onTop = (pt.y <= currentBounds.y + BORDER_RESIZE);
        const bool onBottom = (pt.y >= currentBounds.bottom() - BORDER_RESIZE);

        if (onTop && onLeft) return HitTestResult::BorderTopLeft;
        if (onTop && onRight) return HitTestResult::BorderTopRight;
        if (onBottom && onLeft) return HitTestResult::BorderBottomLeft;
        if (onBottom && onRight) return HitTestResult::BorderBottomRight;
        if (onLeft) return HitTestResult::BorderLeft;
        if (onRight) return HitTestResult::BorderRight;
        if (onTop) return HitTestResult::BorderTop;
        if (onBottom) return HitTestResult::BorderBottom;
    }

    if (captionBounds().contains(pt)) return HitTestResult::Caption;
    return HitTestResult::Client;
}

// ============================================================================
// WindowManager Implementation
// ============================================================================

WindowManager::WindowManager(uint32_t screenWidth, uint32_t screenHeight, int32_t taskbarHeight)
    : screenWidth_(screenWidth), screenHeight_(screenHeight), taskbarHeight_(taskbarHeight) {}

void WindowManager::setScreenSize(uint32_t width, uint32_t height, int32_t taskbarHeight) {
    screenWidth_ = width;
    screenHeight_ = height;
    taskbarHeight_ = taskbarHeight;

    // Re-clamp maximized and snapped windows to new workspace
    for (auto& win : windows_) {
        if (win->state == WindowState::Maximized) {
            win->currentBounds = availableWorkspace();
        } else if (win->state != WindowState::Normal && win->state != WindowState::Minimized) {
            snapWindow(win->id, win->state);
        }
    }
}

Rect WindowManager::availableWorkspace() const noexcept {
    return Rect{0, 0, static_cast<int32_t>(screenWidth_), static_cast<int32_t>(screenHeight_) - taskbarHeight_};
}

uint32_t WindowManager::createWindow(std::string title, Rect bounds, std::string glyph) {
    const uint32_t id = nextWindowId_++;
    auto win = std::make_unique<WindowFrame>();
    win->id = id;
    win->title = std::move(title);
    win->iconGlyph = std::move(glyph);
    win->normalBounds = bounds;
    win->currentBounds = bounds;
    win->state = WindowState::Normal;
    win->isActive = true;
    win->isVisible = true;

    const auto& metrics = ThemeManager::instance().metrics();
    const int32_t clientW = std::max(10, bounds.width - metrics.windowBorderWidth * 2);
    const int32_t clientH = std::max(10, bounds.height - metrics.captionHeight - metrics.windowBorderWidth);
    win->clientSurface.resize(clientW, clientH, Color{10, 14, 22, 255});

    windows_.push_back(std::move(win));
    setWindowActive(id);

    return id;
}

void WindowManager::closeWindow(uint32_t windowId) {
    std::erase_if(windows_, [&](const std::unique_ptr<WindowFrame>& w) { return w->id == windowId; });

    if (snapFlyoutWindowId_ == windowId) {
        hideSnapFlyout();
    }

    if (closedCb_) {
        closedCb_(windowId);
    }

    // Activate the top window
    if (!windows_.empty()) {
        setWindowActive(windows_.back()->id);
    } else {
        activeWindowId_.reset();
    }
}

WindowFrame* WindowManager::findWindow(uint32_t windowId) noexcept {
    for (auto& win : windows_) {
        if (win->id == windowId) return win.get();
    }
    return nullptr;
}

const WindowFrame* WindowManager::findWindow(uint32_t windowId) const noexcept {
    for (const auto& win : windows_) {
        if (win->id == windowId) return win.get();
    }
    return nullptr;
}

void WindowManager::bringToFront(uint32_t windowId) {
    auto it = std::find_if(windows_.begin(), windows_.end(), [&](const std::unique_ptr<WindowFrame>& w) {
        return w->id == windowId;
    });

    if (it != windows_.end() && it != windows_.end() - 1) {
        auto win = std::move(*it);
        windows_.erase(it);
        windows_.push_back(std::move(win));
    }
}

void WindowManager::setWindowActive(uint32_t windowId) {
    activeWindowId_ = windowId;
    bringToFront(windowId);

    for (auto& win : windows_) {
        const bool active = (win->id == windowId);
        win->isActive = active;
        if (stateChangedCb_) {
            stateChangedCb_(win->id, win->state, active);
        }
    }
}

void WindowManager::setWindowState(uint32_t windowId, WindowState state) {
    auto* win = findWindow(windowId);
    if (!win) return;

    win->state = state;
    if (state == WindowState::Normal) {
        win->currentBounds = win->normalBounds;
        win->isVisible = true;
    } else if (state == WindowState::Minimized) {
        win->isVisible = false;
        win->isActive = false;
    } else if (state == WindowState::Maximized) {
        win->currentBounds = availableWorkspace();
        win->isVisible = true;
    }

    if (stateChangedCb_) {
        stateChangedCb_(windowId, state, win->isActive);
    }
}

void WindowManager::toggleMinimize(uint32_t windowId) {
    auto* win = findWindow(windowId);
    if (!win) return;

    if (win->state == WindowState::Minimized) {
        setWindowState(windowId, WindowState::Normal);
        setWindowActive(windowId);
    } else {
        setWindowState(windowId, WindowState::Minimized);
        // Focus top remaining visible window
        for (auto it = windows_.rbegin(); it != windows_.rend(); ++it) {
            if ((*it)->isVisible && (*it)->id != windowId) {
                setWindowActive((*it)->id);
                break;
            }
        }
    }
}

void WindowManager::toggleMaximize(uint32_t windowId) {
    auto* win = findWindow(windowId);
    if (!win) return;

    if (win->state == WindowState::Maximized) {
        setWindowState(windowId, WindowState::Normal);
    } else {
        setWindowState(windowId, WindowState::Maximized);
    }
}

void WindowManager::snapWindow(uint32_t windowId, WindowState snapState) {
    auto* win = findWindow(windowId);
    if (!win) return;

    const Rect ws = availableWorkspace();
    const int32_t halfWidth = ws.width / 2;
    const int32_t halfHeight = ws.height / 2;

    win->state = snapState;
    win->isVisible = true;

    switch (snapState) {
        case WindowState::SnappedLeft:
            win->currentBounds = Rect{ws.x, ws.y, halfWidth, ws.height};
            break;
        case WindowState::SnappedRight:
            win->currentBounds = Rect{ws.x + halfWidth, ws.y, halfWidth, ws.height};
            break;
        case WindowState::SnappedPriorityLeft: // 67% left
            win->currentBounds = Rect{ws.x, ws.y, ws.width * 2 / 3, ws.height};
            break;
        case WindowState::SnappedSidebarRight: // 33% right
            win->currentBounds = Rect{ws.x + ws.width * 2 / 3, ws.y, ws.width - ws.width * 2 / 3, ws.height};
            break;
        case WindowState::SnappedTopLeft:
            win->currentBounds = Rect{ws.x, ws.y, halfWidth, halfHeight};
            break;
        case WindowState::SnappedTopRight:
            win->currentBounds = Rect{ws.x + halfWidth, ws.y, halfWidth, halfHeight};
            break;
        case WindowState::SnappedBottomLeft:
            win->currentBounds = Rect{ws.x, ws.y + halfHeight, halfWidth, halfHeight};
            break;
        case WindowState::SnappedBottomRight:
            win->currentBounds = Rect{ws.x + halfWidth, ws.y + halfHeight, halfWidth, halfHeight};
            break;
        default:
            break;
    }

    if (stateChangedCb_) {
        stateChangedCb_(windowId, win->state, win->isActive);
    }
}

void WindowManager::showSnapFlyout(uint32_t windowId, Point triggerPt) {
    snapFlyoutWindowId_ = windowId;
    isSnapFlyoutVisible_ = true;
    buildSnapZones(triggerPt);
}

void WindowManager::hideSnapFlyout() noexcept {
    isSnapFlyoutVisible_ = false;
    snapFlyoutWindowId_ = 0;
    snapZones_.clear();
}

void WindowManager::buildSnapZones(Point anchor) {
    snapZones_.clear();
    const int32_t flyoutW = 280;
    const int32_t flyoutH = 110;
    snapFlyoutBounds_ = Rect{anchor.x - flyoutW / 2, anchor.y, flyoutW, flyoutH};

    const Rect ws = availableWorkspace();
    if (snapFlyoutBounds_.right() > ws.right() - 8) {
        snapFlyoutBounds_.x = ws.right() - flyoutW - 8;
    }
    if (snapFlyoutBounds_.x < ws.x + 8) {
        snapFlyoutBounds_.x = ws.x + 8;
    }

    // Card 1: 50 / 50 Split
    const int32_t card1X = snapFlyoutBounds_.x + 12;
    const int32_t cardY = snapFlyoutBounds_.y + 26;
    const int32_t cardW = 76;
    const int32_t cardH = 68;

    snapZones_.push_back(SnapZone{
        .bounds = Rect{card1X, cardY, cardW / 2 - 1, cardH},
        .targetState = WindowState::SnappedLeft,
        .isHovered = false
    });
    snapZones_.push_back(SnapZone{
        .bounds = Rect{card1X + cardW / 2 + 1, cardY, cardW / 2 - 1, cardH},
        .targetState = WindowState::SnappedRight,
        .isHovered = false
    });

    // Card 2: 67 / 33 Priority Split
    const int32_t card2X = card1X + cardW + 14;
    const int32_t priW = cardW * 2 / 3;
    snapZones_.push_back(SnapZone{
        .bounds = Rect{card2X, cardY, priW - 1, cardH},
        .targetState = WindowState::SnappedPriorityLeft,
        .isHovered = false
    });
    snapZones_.push_back(SnapZone{
        .bounds = Rect{card2X + priW + 1, cardY, cardW - priW - 1, cardH},
        .targetState = WindowState::SnappedSidebarRight,
        .isHovered = false
    });

    // Card 3: 2x2 4-Quadrant Quad
    const int32_t card3X = card2X + cardW + 14;
    const int32_t qW = cardW / 2 - 1;
    const int32_t qH = cardH / 2 - 1;
    snapZones_.push_back(SnapZone{
        .bounds = Rect{card3X, cardY, qW, qH},
        .targetState = WindowState::SnappedTopLeft,
        .isHovered = false
    });
    snapZones_.push_back(SnapZone{
        .bounds = Rect{card3X + qW + 2, cardY, qW, qH},
        .targetState = WindowState::SnappedTopRight,
        .isHovered = false
    });
    snapZones_.push_back(SnapZone{
        .bounds = Rect{card3X, cardY + qH + 2, qW, qH},
        .targetState = WindowState::SnappedBottomLeft,
        .isHovered = false
    });
    snapZones_.push_back(SnapZone{
        .bounds = Rect{card3X + qW + 2, cardY + qH + 2, qW, qH},
        .targetState = WindowState::SnappedBottomRight,
        .isHovered = false
    });
}

bool WindowManager::onMouseDown(Point pt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    // Check click on Snap Flyout
    if (isSnapFlyoutVisible_) {
        if (snapFlyoutBounds_.contains(pt)) {
            for (const auto& zone : snapZones_) {
                if (zone.bounds.contains(pt)) {
                    snapWindow(snapFlyoutWindowId_, zone.targetState);
                    hideSnapFlyout();
                    return true;
                }
            }
            return true;
        } else {
            hideSnapFlyout();
        }
    }

    // Iterate in reverse Z-order (top to bottom)
    for (auto it = windows_.rbegin(); it != windows_.rend(); ++it) {
        auto& win = *it;
        if (!win->isVisible) continue;

        HitTestResult hit = win->hitTest(pt);
        if (hit == HitTestResult::None) continue;

        setWindowActive(win->id);

        if (hit == HitTestResult::CloseButton) {
            closeWindow(win->id);
            return true;
        } else if (hit == HitTestResult::MinButton) {
            toggleMinimize(win->id);
            return true;
        } else if (hit == HitTestResult::MaxButton) {
            toggleMaximize(win->id);
            return true;
        } else if (hit == HitTestResult::Caption) {
            isDragging_ = true;
            interactingWindowId_ = win->id;
            dragStartMouse_ = pt;
            dragStartWindowBounds_ = win->currentBounds;
            return true;
        } else if (hit >= HitTestResult::BorderLeft && hit <= HitTestResult::BorderBottomRight) {
            isResizing_ = true;
            interactingWindowId_ = win->id;
            resizeEdge_ = hit;
            dragStartMouse_ = pt;
            dragStartWindowBounds_ = win->currentBounds;
            return true;
        }
        return true;
    }
    return false;
}

bool WindowManager::onMouseMove(Point pt) {
    // Snap Flyout hover handling
    if (isSnapFlyoutVisible_) {
        if (snapFlyoutBounds_.contains(pt)) {
            for (auto& zone : snapZones_) {
                zone.isHovered = zone.bounds.contains(pt);
            }
            return true;
        } else if (!snapFlyoutBounds_.inflate(30, 30).contains(pt)) {
            hideSnapFlyout();
        }
    } else {
        // Check hover over Maximize button of top active window
        if (activeWindowId_) {
            auto* activeWin = findWindow(*activeWindowId_);
            if (activeWin && activeWin->isVisible && activeWin->maxButtonBounds().contains(pt)) {
                showSnapFlyout(activeWin->id, Point{activeWin->maxButtonBounds().center().x, activeWin->maxButtonBounds().bottom() + 4});
            }
        }
    }

    if (isDragging_) {
        auto* win = findWindow(interactingWindowId_);
        if (win) {
            const int32_t dx = pt.x - dragStartMouse_.x;
            const int32_t dy = pt.y - dragStartMouse_.y;

            if (win->state != WindowState::Normal) {
                win->state = WindowState::Normal;
                win->currentBounds.width = win->normalBounds.width;
                win->currentBounds.height = win->normalBounds.height;
            }

            win->currentBounds.x = dragStartWindowBounds_.x + dx;
            win->currentBounds.y = dragStartWindowBounds_.y + dy;
            win->normalBounds = win->currentBounds;

            // Aero Snap triggers at screen edges
            if (pt.x <= 2) {
                snapWindow(win->id, WindowState::SnappedLeft);
                isDragging_ = false;
            } else if (pt.x >= static_cast<int32_t>(screenWidth_) - 3) {
                snapWindow(win->id, WindowState::SnappedRight);
                isDragging_ = false;
            } else if (pt.y <= 2) {
                setWindowState(win->id, WindowState::Maximized);
                isDragging_ = false;
            }
        }
        return true;
    }

    if (isResizing_) {
        auto* win = findWindow(interactingWindowId_);
        if (win) {
            const int32_t dx = pt.x - dragStartMouse_.x;
            const int32_t dy = pt.y - dragStartMouse_.y;
            Rect nb = dragStartWindowBounds_;

            switch (resizeEdge_) {
                case HitTestResult::BorderRight:
                    nb.width = std::max(200, dragStartWindowBounds_.width + dx);
                    break;
                case HitTestResult::BorderBottom:
                    nb.height = std::max(120, dragStartWindowBounds_.height + dy);
                    break;
                case HitTestResult::BorderLeft:
                    nb.x = std::min(dragStartWindowBounds_.right() - 200, dragStartWindowBounds_.x + dx);
                    nb.width = dragStartWindowBounds_.right() - nb.x;
                    break;
                case HitTestResult::BorderTop:
                    nb.y = std::min(dragStartWindowBounds_.bottom() - 120, dragStartWindowBounds_.y + dy);
                    nb.height = dragStartWindowBounds_.bottom() - nb.y;
                    break;
                case HitTestResult::BorderBottomRight:
                    nb.width = std::max(200, dragStartWindowBounds_.width + dx);
                    nb.height = std::max(120, dragStartWindowBounds_.height + dy);
                    break;
                default:
                    break;
            }

            win->currentBounds = nb;
            win->normalBounds = nb;
        }
        return true;
    }

    return false;
}

bool WindowManager::onMouseUp(Point pt, MouseButton button) {
    (void)pt;
    if (button == MouseButton::Left) {
        isDragging_ = false;
        isResizing_ = false;
        interactingWindowId_ = 0;
        resizeEdge_ = HitTestResult::None;
        return true;
    }
    return false;
}

bool WindowManager::onDoubleClick(Point pt) {
    for (auto it = windows_.rbegin(); it != windows_.rend(); ++it) {
        auto& win = *it;
        if (!win->isVisible) continue;

        if (win->captionBounds().contains(pt)) {
            toggleMaximize(win->id);
            return true;
        }
    }
    return false;
}

void WindowManager::renderSnapFlyout(Surface& surface) {
    const auto& palette = ThemeManager::instance().palette();

    // Drop shadow
    surface.drawDropShadow(snapFlyoutBounds_, 16, 0.50f);

    // Modern rounded container with translucent Mica Acrylic blur
    surface.applyAcrylicTint(snapFlyoutBounds_, palette.snapFlyoutBg, 8);
    surface.drawRoundedRect(snapFlyoutBounds_, 10, palette.snapFlyoutBorder, false);

    // Flyout Header
    surface.drawString(snapFlyoutBounds_.x + 14, snapFlyoutBounds_.y + 8, "SNAP LAYOUTS", palette.accentColor, 1);

    // Render snap layout zones
    for (const auto& zone : snapZones_) {
        Color fillCol = zone.isHovered ? palette.snapZoneHover : palette.snapZoneNormal;
        Color borderCol = zone.isHovered ? palette.snapZoneBorderHover : palette.snapZoneBorder;

        surface.drawRoundedRect(zone.bounds, 4, fillCol, true);
        surface.drawRoundedRect(zone.bounds, 4, borderCol, false);
    }
}

void WindowManager::render(Surface& surface) {
    const auto& palette = ThemeManager::instance().palette();
    const auto& metrics = ThemeManager::instance().metrics();

    // Render windows back-to-front
    for (const auto& win : windows_) {
        if (!win->isVisible || win->state == WindowState::Minimized) continue;

        const Rect bounds = win->currentBounds;

        // 1. Window Drop Shadow (larger when active)
        const int32_t shadowR = win->isActive ? metrics.shadowRadius + 4 : metrics.shadowRadius;
        const float shadowOp = win->isActive ? metrics.shadowOpacity + 0.1f : metrics.shadowOpacity;
        surface.drawDropShadow(bounds, shadowR, shadowOp);

        // 2. Window Frame Background & Border
        Color frameBg = win->isActive ? palette.windowFrameActiveBg : palette.windowFrameInactiveBg;
        Color frameBorder = win->isActive ? palette.windowBorderActive : palette.windowBorderInactive;

        surface.drawRoundedRect(bounds, metrics.windowCornerRadius, frameBg, true);
        surface.drawRoundedRect(bounds, metrics.windowCornerRadius, frameBorder, false);

        // 3. Window Caption Bar (Active Mica Gradient)
        Rect capRect = win->captionBounds();
        if (win->isActive) {
            surface.drawVerticalGradient(capRect, Color::fromRgba(25, 38, 62, 230), Color::fromRgba(16, 24, 40, 230));
        }

        // Icon Glyph
        surface.drawString(capRect.x + 10, capRect.y + 10, win->iconGlyph, palette.accentColor, 1);

        // Window Title
        surface.drawString(capRect.x + 36, capRect.y + 10, win->title, palette.textPrimary, 1);

        // 4. Caption Buttons: Minimize [_], Maximize [[]], Close [X]
        Rect minBtn = win->minButtonBounds();
        Rect maxBtn = win->maxButtonBounds();
        Rect closeBtn = win->closeButtonBounds();

        surface.drawRoundedRect(minBtn, metrics.buttonCornerRadius, Color::fromRgba(35, 48, 72, 180), true);
        surface.drawString(minBtn.x + 10, minBtn.y + 8, "_", palette.textSecondary, 1);

        surface.drawRoundedRect(maxBtn, metrics.buttonCornerRadius, Color::fromRgba(35, 48, 72, 180), true);
        surface.drawString(maxBtn.x + 8, maxBtn.y + 8, "[]", palette.textSecondary, 1);

        surface.drawRoundedRect(closeBtn, metrics.buttonCornerRadius, Color::fromRgba(180, 35, 45, 180), true);
        surface.drawString(closeBtn.x + 10, closeBtn.y + 8, "X", Color::fromHex(0xFFFFFF), 1);

        // 5. Client Area
        Rect clientRect = win->clientAreaBounds();
        surface.fillRect(clientRect, palette.windowClientBg);

        // Blit client surface
        surface.blit(win->clientSurface, Rect{0, 0, clientRect.width, clientRect.height}, Point{clientRect.x, clientRect.y});
    }

    // 6. Render Snap Layouts Flyout if active
    if (isSnapFlyoutVisible_) {
        renderSnapFlyout(surface);
    }
}

} // namespace surshell
