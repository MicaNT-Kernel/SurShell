// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/lock_screen.cpp)
// ============================================================================

#include "surshell/lock_screen.hpp"
#include "surshell/kernel_bridge.hpp"
#include <algorithm>

namespace surshell {

LockScreen::LockScreen() {
    username_ = KernelBridge::queryCurrentUserName() + " (MicaNT Executive)";
}

void LockScreen::lock() noexcept {
    isLocked_ = true;
    lockState_ = LockState::LockedAmbient;
    pinBuffer_.clear();
    showPowerMenu_ = false;
}

void LockScreen::unlock() noexcept {
    isLocked_ = false;
    lockState_ = LockState::LockedAmbient;
    pinBuffer_.clear();
    showPowerMenu_ = false;
    if (onUnlock_) onUnlock_();
}

void LockScreen::render(Surface& s, int32_t screenW, int32_t screenH) {
    if (!isLocked_) return;

    const auto& palette = ThemeManager::instance().palette();

    // 1. Full-screen atmospheric wallpaper backdrop
    s.drawVerticalGradient(Rect{0, 0, screenW, screenH}, Color::fromHex(0x081220), Color::fromHex(0x03060C));

    // Subtle undulating auroral wave
    for (int32_t x = 0; x < screenW; x += 4) {
        const int32_t y = screenH / 2 + static_cast<int32_t>(100.0f * std::sin(x * 0.005f));
        s.blendPixel(x, y, Color::fromRgba(0, 212, 255, 60));
        s.blendPixel(x, y + 1, Color::fromRgba(0, 255, 157, 45));
    }

    if (lockState_ == LockState::LockedAmbient) {
        renderAmbient(s, screenW, screenH, palette);
    } else {
        renderLogonCard(s, screenW, screenH, palette);
    }

    // Bottom-right power & network status controls
    const int32_t btnSize = 36;
    btnPower_ = Rect{screenW - btnSize - 24, screenH - btnSize - 24, btnSize, btnSize};
    const Rect netStatusR{btnPower_.x - 44, btnPower_.y, btnSize, btnSize};

    // Network icon
    IconRenderer::draw(s, IconId::NetworkOnline, Point{netStatusR.x + 8, netStatusR.y + 8}, 20, Color::fromHex(0x00FF9D));

    // Power button
    s.drawRoundedRect(btnPower_, 6, hoverPower_ ? Color::fromHex(0x1E293B) : Color::fromHex(0x101724), true);
    s.drawRoundedRect(btnPower_, 6, hoverPower_ ? palette.accentColor : Color::fromHex(0x354765), false);
    IconRenderer::draw(s, IconId::Power, Point{btnPower_.x + 8, btnPower_.y + 8}, 20, palette.accentColor);

    // Power Flyout
    if (showPowerMenu_) {
        flyoutPower_ = Rect{btnPower_.right() - 150, btnPower_.y - 110, 150, 100};
        s.drawRoundedRect(flyoutPower_, 8, Color::fromRgba(14, 20, 32, 245), true);
        s.drawRoundedRect(flyoutPower_, 8, Color::fromHex(0x283850), false);

        btnSleep_ = Rect{flyoutPower_.x + 6, flyoutPower_.y + 6, flyoutPower_.width - 12, 26};
        btnRestart_ = Rect{flyoutPower_.x + 6, flyoutPower_.y + 36, flyoutPower_.width - 12, 26};
        btnShutdown_ = Rect{flyoutPower_.x + 6, flyoutPower_.y + 66, flyoutPower_.width - 12, 26};

        auto drawPowerItem = [&](const Rect& r, const std::string& label, IconId icon, int32_t idx) {
            const bool isHov = (hoverPowerItem_ == idx);
            if (isHov) s.drawRoundedRect(r, 4, Color::fromRgba(255, 255, 255, 25), true);
            IconRenderer::draw(s, icon, Point{r.x + 6, r.y + 5}, 16, palette.accentColor);
            s.drawString(r.x + 28, r.y + 6, label, isHov ? Color::fromHex(0xFFFFFF) : palette.textPrimary, 1);
        };

        drawPowerItem(btnSleep_, "Sleep", IconId::Sleep, 0);
        drawPowerItem(btnRestart_, "Restart", IconId::Restart, 1);
        drawPowerItem(btnShutdown_, "Shut down", IconId::Power, 2);
    }
}

void LockScreen::renderAmbient(Surface& s, int32_t screenW, int32_t screenH, const ThemePalette& palette) {
    // 1. Large Digital Clock
    const int32_t clockX = screenW / 2 - 90;
    const int32_t clockY = screenH / 3 - 40;
    s.drawString(clockX, clockY, timeStr_, Color::fromHex(0xFFFFFF), 2);

    // 2. Date String
    const int32_t dateX = screenW / 2 - static_cast<int32_t>(dateStr_.size() * 4);
    s.drawString(dateX, clockY + 44, dateStr_, palette.accentColor, 1);

    // 3. Status Badges
    const int32_t badgeY = clockY + 80;
    const int32_t badgeX = screenW / 2 - 130;
    IconRenderer::draw(s, IconId::BatteryCharging, Point{badgeX, badgeY}, 16, Color::fromHex(0x00FF9D));
    s.drawString(badgeX + 22, badgeY + 2, "100%", Color::fromHex(0xCBD5E1), 1);

    IconRenderer::draw(s, IconId::SentinelSec, Point{badgeX + 70, badgeY}, 16, palette.accentColor);
    s.drawString(badgeX + 92, badgeY + 2, "SentinelSec Guard", Color::fromHex(0xCBD5E1), 1);

    // 4. Prompt
    const std::string hint = "Press any key or click to unlock";
    const int32_t hintX = screenW / 2 - static_cast<int32_t>(hint.size() * 4);
    s.drawString(hintX, screenH - 120, hint, Color::fromHex(0x94A3B8), 1);
}

void LockScreen::renderLogonCard(Surface& s, int32_t screenW, int32_t screenH, const ThemePalette& palette) {
    const int32_t cardW = 460;
    const int32_t cardH = 340;
    const int32_t cardX = (screenW - cardW) / 2;
    const int32_t cardY = (screenH - cardH) / 2 - 30;
    loginCardBounds_ = Rect{cardX, cardY, cardW, cardH};

    // Drop shadow & Frosted Acrylic Chassis
    s.drawRoundedRect(loginCardBounds_.inflate(8, 8), 16, Color::fromRgba(0, 0, 0, 95), true);
    s.drawRoundedRect(loginCardBounds_, 12, Color::fromRgba(14, 20, 32, 235), true);
    s.drawRoundedRect(loginCardBounds_, 12, Color::fromHex(0x283850), false);

    // User Avatar Circle
    const int32_t avatarSize = 72;
    const int32_t avatarX = cardX + (cardW - avatarSize) / 2;
    const int32_t avatarY = cardY + 28;
    s.drawRoundedRect(Rect{avatarX, avatarY, avatarSize, avatarSize}, avatarSize / 2, Color::fromHex(0x182436), true);
    s.drawRoundedRect(Rect{avatarX, avatarY, avatarSize, avatarSize}, avatarSize / 2, palette.accentColor, false);
    IconRenderer::draw(s, IconId::User, Point{avatarX + 16, avatarY + 16}, 40, palette.accentColor);

    // Username
    const int32_t nameX = cardX + (cardW - static_cast<int32_t>(username_.size() * 8)) / 2;
    s.drawString(nameX, avatarY + avatarSize + 18, username_, Color::fromHex(0xFFFFFF), 1);

    // Subtitle (Fits with 118px margin on left and right)
    const std::string sub = "MicaNT Sovereign Workstation";
    const int32_t subX = cardX + (cardW - static_cast<int32_t>(sub.size() * 8)) / 2;
    s.drawString(subX, avatarY + avatarSize + 36, sub, Color::fromHex(0x00FF9D), 1);

    // PIN Input Box & Unlock Button (Centered compound group)
    const int32_t pinBoxW = 200;
    const int32_t pinBoxH = 34;
    const int32_t btnW = 34;
    const int32_t gap = 8;
    const int32_t groupW = pinBoxW + gap + btnW;
    const int32_t startX = cardX + (cardW - groupW) / 2;

    pinBoxBounds_ = Rect{startX, cardY + 210, pinBoxW, pinBoxH};
    s.drawRoundedRect(pinBoxBounds_, 6, Color::fromHex(0x0A0F18), true);
    s.drawRoundedRect(pinBoxBounds_, 6, palette.accentColor, false);

    // Masked password bullets
    std::string masked(pinBuffer_.size(), '*');
    s.drawString(pinBoxBounds_.x + 12, pinBoxBounds_.y + 10, masked, Color::fromHex(0xFFFFFF), 1);

    // Blinking cursor
    const int32_t curX = pinBoxBounds_.x + 12 + static_cast<int32_t>(masked.size()) * 8;
    s.fillRect(Rect{curX, pinBoxBounds_.y + 8, 2, 18}, palette.accentColor);

    // [➔] Unlock Arrow Button
    btnUnlockBounds_ = Rect{pinBoxBounds_.right() + gap, pinBoxBounds_.y, btnW, pinBoxH};
    s.drawRoundedRect(btnUnlockBounds_, 6, hoverUnlock_ ? Color::fromHex(0x1E293B) : Color::fromHex(0x101724), true);
    s.drawRoundedRect(btnUnlockBounds_, 6, hoverUnlock_ ? palette.accentColor : Color::fromHex(0x354765), false);
    s.drawString(btnUnlockBounds_.x + 12, btnUnlockBounds_.y + 9, "->", Color::fromHex(0x00D4FF), 1);

    // Bottom prompt hint (Centered)
    const std::string authHint = "Press Enter or [->] to authenticate";
    const int32_t hintX = cardX + (cardW - static_cast<int32_t>(authHint.size() * 8)) / 2;
    s.drawString(hintX, cardY + 265, authHint, palette.textSecondary, 1);
}

bool LockScreen::onMouseDown(Point pt, MouseButton button) {
    if (!isLocked_ || button != MouseButton::Left) return false;

    if (lockState_ == LockState::LockedAmbient) {
        showCredentials();
        return true;
    }

    // Check power button
    if (btnPower_.contains(pt)) {
        showPowerMenu_ = !showPowerMenu_;
        return true;
    }

    // Check power items
    if (showPowerMenu_) {
        if (btnSleep_.contains(pt)) {
            if (onPower_) onPower_("sleep");
            showPowerMenu_ = false;
            return true;
        }
        if (btnRestart_.contains(pt)) {
            if (onPower_) onPower_("restart");
            showPowerMenu_ = false;
            return true;
        }
        if (btnShutdown_.contains(pt)) {
            if (onPower_) onPower_("shutdown");
            showPowerMenu_ = false;
            return true;
        }
    }

    // Check Unlock button
    if (btnUnlockBounds_.contains(pt)) {
        if (pinBuffer_ == correctPin_) {
            unlock();
        } else {
            pinBuffer_.clear();
        }
        return true;
    }

    return true; // Click consumed
}

bool LockScreen::onMouseMove(Point pt) {
    if (!isLocked_) return false;

    hoverPower_ = btnPower_.contains(pt);
    hoverUnlock_ = btnUnlockBounds_.contains(pt);

    hoverPowerItem_ = -1;
    if (showPowerMenu_) {
        if (btnSleep_.contains(pt)) hoverPowerItem_ = 0;
        else if (btnRestart_.contains(pt)) hoverPowerItem_ = 1;
        else if (btnShutdown_.contains(pt)) hoverPowerItem_ = 2;
    }

    return true;
}

bool LockScreen::onKeyDown(KeyCode key) {
    if (!isLocked_) return false;

    if (lockState_ == LockState::LockedAmbient) {
        showCredentials();
        return true;
    }

    if (key == KeyCode::Enter) {
        if (pinBuffer_ == correctPin_) {
            unlock();
        } else {
            pinBuffer_.clear();
        }
        return true;
    } else if (key == KeyCode::Escape) {
        lockState_ = LockState::LockedAmbient;
        return true;
    } else if (key == KeyCode::Backspace) {
        if (!pinBuffer_.empty()) {
            pinBuffer_.pop_back();
            return true;
        }
    }

    return false;
}

bool LockScreen::onCharInput(char c) {
    if (!isLocked_) return false;

    if (lockState_ == LockState::LockedAmbient) {
        showCredentials();
    }

    if (c >= 32 && c <= 126) {
        pinBuffer_.push_back(c);
        return true;
    }
    return false;
}

} // namespace surshell
