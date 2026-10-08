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
        } else if (win->state == WindowState::SnappedLeft || win->state == WindowState::SnappedRight) {
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

    // Allocate client surface matching initial bounds
    const auto& metrics = ThemeManager::instance().metrics();
    const uint32_t clientW = std::max(10, bounds.width - metrics.windowBorderWidth * 2);
    const uint32_t clientH = std::max(10, bounds.height - metrics.captionHeight - metrics.windowBorderWidth);
    win->clientSurface.resize(clientW, clientH, ThemeManager::instance().palette().windowClientBg);

    windows_.push_back(std::move(win));
    setWindowActive(id);

    if (stateChangedCb_) {
        stateChangedCb_(id, WindowState::Normal, true);
    }
    return id;
}

void WindowManager::closeWindow(uint32_t windowId) {
    std::erase_if(windows_, [&](const std::unique_ptr<WindowFrame>& w) { return w->id == windowId; });
    if (activeWindowId_ && *activeWindowId_ == windowId) {
        activeWindowId_ = std::nullopt;
        if (!windows_.empty()) {
            setWindowActive(windows_.back()->id);
        }
    }
    if (closedCb_) {
        closedCb_(windowId);
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
    if (it != windows_.end() && it != std::prev(windows_.end())) {
        auto winPtr = std::move(*it);
        windows_.erase(it);
        windows_.push_back(std::move(winPtr));
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

    if (snapState == WindowState::SnappedLeft) {
        win->state = WindowState::SnappedLeft;
        win->currentBounds = Rect{ws.x, ws.y, halfWidth, ws.height};
        win->isVisible = true;
    } else if (snapState == WindowState::SnappedRight) {
        win->state = WindowState::SnappedRight;
        win->currentBounds = Rect{ws.x + halfWidth, ws.y, halfWidth, ws.height};
        win->isVisible = true;
    }

    if (stateChangedCb_) {
        stateChangedCb_(windowId, win->state, win->isActive);
    }
}

bool WindowManager::onMouseDown(Point pt, MouseButton button) {
    if (button != MouseButton::Left) return false;

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
    if (isDragging_) {
        auto* win = findWindow(interactingWindowId_);
        if (win) {
            const int32_t dx = pt.x - dragStartMouse_.x;
            const int32_t dy = pt.y - dragStartMouse_.y;

            if (win->state != WindowState::Normal) {
                // Restore on drag away
                win->state = WindowState::Normal;
                win->currentBounds.width = win->normalBounds.width;
                win->currentBounds.height = win->normalBounds.height;
            }

            win->currentBounds.x = dragStartWindowBounds_.x + dx;
            win->currentBounds.y = dragStartWindowBounds_.y + dy;
            win->normalBounds = win->currentBounds;

            // Aero Snap triggers at screen edges
            if (pt.x <= 4) {
                snapWindow(win->id, WindowState::SnappedLeft);
                isDragging_ = false;
            } else if (pt.x >= static_cast<int32_t>(screenWidth_) - 4) {
                snapWindow(win->id, WindowState::SnappedRight);
                isDragging_ = false;
            } else if (pt.y <= 4) {
                setWindowState(win->id, WindowState::Maximized);
                isDragging_ = false;
            }
        }
        return true;
    }

    if (isResizing_) {
        auto* win = findWindow(interactingWindowId_);
        if (win && win->state == WindowState::Normal) {
            const int32_t dx = pt.x - dragStartMouse_.x;
            const int32_t dy = pt.y - dragStartMouse_.y;
            Rect nb = dragStartWindowBounds_;

            if (resizeEdge_ == HitTestResult::BorderRight || resizeEdge_ == HitTestResult::BorderTopRight || resizeEdge_ == HitTestResult::BorderBottomRight) {
                nb.width = std::max(240, dragStartWindowBounds_.width + dx);
            }
            if (resizeEdge_ == HitTestResult::BorderBottom || resizeEdge_ == HitTestResult::BorderBottomLeft || resizeEdge_ == HitTestResult::BorderBottomRight) {
                nb.height = std::max(160, dragStartWindowBounds_.height + dy);
            }
            if (resizeEdge_ == HitTestResult::BorderLeft || resizeEdge_ == HitTestResult::BorderTopLeft || resizeEdge_ == HitTestResult::BorderBottomLeft) {
                const int32_t nw = dragStartWindowBounds_.width - dx;
                if (nw >= 240) {
                    nb.x = dragStartWindowBounds_.x + dx;
                    nb.width = nw;
                }
            }
            if (resizeEdge_ == HitTestResult::BorderTop || resizeEdge_ == HitTestResult::BorderTopLeft || resizeEdge_ == HitTestResult::BorderTopRight) {
                const int32_t nh = dragStartWindowBounds_.height - dy;
                if (nh >= 160) {
                    nb.y = dragStartWindowBounds_.y + dy;
                    nb.height = nh;
                }
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
}

} // namespace surshell
