// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/lock_screen.hpp)
//
// Sovereign Lock Screen & Authentication Center (Win+L), featuring ambient
// time/date display, Mica Gaussian blur, credentials login card, PIN entry,
// and session security controls.
// ============================================================================

#pragma once

#include "surshell/types.hpp"
#include "surshell/compositor.hpp"
#include "surshell/icons.hpp"
#include "surshell/theme.hpp"
#include <string>
#include <functional>

namespace surshell {

enum class LockState {
    LockedAmbient = 0,
    CredentialsLogon
};

using LockUnlockCallback = std::function<void()>;
using LockPowerCallback = std::function<void(const std::string& action)>;

class LockScreen {
public:
    LockScreen();
    ~LockScreen() = default;

    void lock() noexcept;
    void unlock() noexcept;
    [[nodiscard]] bool isLocked() const noexcept { return isLocked_; }
    [[nodiscard]] LockState lockState() const noexcept { return lockState_; }

    void showCredentials() noexcept { lockState_ = LockState::CredentialsLogon; }

    void render(Surface& surface, int32_t screenWidth, int32_t screenHeight);
    bool onMouseDown(Point pt, MouseButton button);
    bool onMouseMove(Point pt);
    bool onKeyDown(KeyCode key);
    bool onCharInput(char c);

    void setUnlockCallback(LockUnlockCallback cb) { onUnlock_ = std::move(cb); }
    void setPowerCallback(LockPowerCallback cb) { onPower_ = std::move(cb); }

    void setPin(const std::string& pin) { pinBuffer_ = pin; }
    const std::string& pin() const noexcept { return pinBuffer_; }

    void setCorrectPin(const std::string& pin) { correctPin_ = pin; }
    const std::string& correctPin() const noexcept { return correctPin_; }

private:
    void renderAmbient(Surface& s, int32_t w, int32_t h, const ThemePalette& palette);
    void renderLogonCard(Surface& s, int32_t w, int32_t h, const ThemePalette& palette);

    bool isLocked_{false};
    LockState lockState_{LockState::LockedAmbient};

    std::string timeStr_{"01:45 PM"};
    std::string dateStr_{"Thursday, October 8, 2026"};
    std::string username_{"admin (MicaNT Executive)"};
    std::string correctPin_{"1234"};
    std::string pinBuffer_;

    // UI Bounds
    Rect loginCardBounds_{};
    Rect pinBoxBounds_{};
    Rect btnUnlockBounds_{};
    Rect btnPower_{};
    Rect flyoutPower_{};
    Rect btnSleep_{};
    Rect btnRestart_{};
    Rect btnShutdown_{};

    bool hoverUnlock_{false};
    bool hoverPower_{false};
    bool showPowerMenu_{false};
    int32_t hoverPowerItem_{-1};

    LockUnlockCallback onUnlock_;
    LockPowerCallback onPower_;
};

} // namespace surshell
