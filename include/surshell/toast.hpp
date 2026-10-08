// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/toast.hpp)
//
// Sovereign Acrylic Toast Notification Engine.
// Sliding acrylic notification cards in the bottom-right action corner,
// with dismiss timers, priority levels, action buttons, and vector badges.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <functional>

namespace surshell {

struct ToastNotification {
    uint32_t id{0};
    std::string title;
    std::string message;
    std::string timestamp{"Just now"};
    IconId iconId{IconId::NotificationBell};
    Color accentColor{Color::fromHex(0x00D4FF)};
    int32_t lifeTicks{300}; // ~5 seconds at 60fps
    int32_t maxLifeTicks{300};
    float slideOffset{0.0f}; // 0.0 = fully in place, >0 = sliding
    bool isDismissed{false};
    Rect bounds{};
    Rect closeBtnBounds{};
};

class ToastManager {
public:
    ToastManager() = default;

    uint32_t showToast(std::string title, std::string message,
                       IconId icon = IconId::NotificationBell,
                       Color accent = Color::fromHex(0x00D4FF),
                       int32_t durationTicks = 300);

    void dismissToast(uint32_t id);
    void dismissLatest();
    void clear();

    void tick(); // decrements timers and updates animations

    [[nodiscard]] size_t toastCount() const noexcept { return toasts_.size(); }
    [[nodiscard]] const std::vector<ToastNotification>& toasts() const noexcept { return toasts_; }
    [[nodiscard]] const ToastNotification* findToast(uint32_t id) const noexcept;

    // Hit Testing & Input
    bool onMouseDown(Point pt, MouseButton btn);
    bool onMouseMove(Point pt);

    // Rendering
    void render(Surface& surface, int32_t screenWidth, int32_t screenHeight, int32_t taskbarHeight) const;

private:
    uint32_t nextId_{1};
    std::vector<ToastNotification> toasts_{};
};

} // namespace surshell
