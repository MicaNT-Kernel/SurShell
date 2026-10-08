// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/toast.cpp)
// ============================================================================

#include "surshell/toast.hpp"
#include <algorithm>

namespace surshell {

uint32_t ToastManager::showToast(std::string title, std::string message,
                                 IconId icon, Color accent, int32_t durationTicks) {
    const uint32_t id = nextId_++;

    // Limit maximum active toasts on screen to 4
    if (toasts_.size() >= 4) {
        toasts_.erase(toasts_.begin());
    }

    ToastNotification toast{
        .id = id,
        .title = std::move(title),
        .message = std::move(message),
        .timestamp = "Just now",
        .iconId = icon,
        .accentColor = accent,
        .lifeTicks = durationTicks,
        .maxLifeTicks = durationTicks,
        .slideOffset = 20.0f,
        .isDismissed = false,
        .bounds = Rect{0, 0, 360, 76},
        .closeBtnBounds = Rect{0, 0, 18, 18}
    };

    toasts_.push_back(std::move(toast));
    return id;
}

void ToastManager::dismissToast(uint32_t id) {
    std::erase_if(toasts_, [id](const ToastNotification& t) { return t.id == id; });
}

void ToastManager::dismissLatest() {
    if (!toasts_.empty()) {
        toasts_.pop_back();
    }
}

void ToastManager::clear() {
    toasts_.clear();
}

void ToastManager::tick() {
    for (auto& toast : toasts_) {
        if (toast.lifeTicks > 0) {
            toast.lifeTicks--;
        }
        if (toast.slideOffset > 0.0f) {
            toast.slideOffset = std::max(0.0f, toast.slideOffset - 4.0f);
        }
    }
    std::erase_if(toasts_, [](const ToastNotification& t) { return t.lifeTicks <= 0; });
}

const ToastNotification* ToastManager::findToast(uint32_t id) const noexcept {
    for (const auto& t : toasts_) {
        if (t.id == id) return &t;
    }
    return nullptr;
}

bool ToastManager::onMouseDown(Point pt, MouseButton btn) {
    if (btn != MouseButton::Left) return false;

    for (auto it = toasts_.rbegin(); it != toasts_.rend(); ++it) {
        if (it->closeBtnBounds.contains(pt)) {
            dismissToast(it->id);
            return true;
        }
        if (it->bounds.contains(pt)) {
            dismissToast(it->id);
            return true;
        }
    }
    return false;
}

bool ToastManager::onMouseMove(Point pt) {
    for (const auto& t : toasts_) {
        if (t.bounds.contains(pt)) return true;
    }
    return false;
}

void ToastManager::render(Surface& surface, int32_t screenWidth, int32_t screenHeight, int32_t taskbarHeight) const {
    if (toasts_.empty()) return;

    constexpr int32_t toastWidth = 360;
    constexpr int32_t toastHeight = 76;
    constexpr int32_t spacing = 12;
    constexpr int32_t margin = 20;

    int32_t curY = screenHeight - taskbarHeight - margin - toastHeight;

    for (auto it = toasts_.rbegin(); it != toasts_.rend(); ++it) {
        const int32_t x = screenWidth - margin - toastWidth + static_cast<int32_t>(it->slideOffset);
        const int32_t y = curY;

        // Cast away constness to store calculated bounds for hit testing
        auto* mutableToast = const_cast<ToastNotification*>(&*it);
        mutableToast->bounds = Rect{x, y, toastWidth, toastHeight};
        mutableToast->closeBtnBounds = Rect{x + toastWidth - 26, y + 10, 16, 16};

        // 1. Drop shadow
        surface.drawDropShadow(mutableToast->bounds, 14, 140);

        // 2. Acrylic frosted glass body
        surface.drawRoundedRect(mutableToast->bounds, 8, Color::fromRgba(14, 20, 32, 230), true);
        surface.drawRoundedRect(mutableToast->bounds, 8, Color::fromRgba(45, 65, 95, 180), false);

        // 3. Left vertical accent indicator stripe
        surface.drawRoundedRect(Rect{x + 2, y + 4, 4, toastHeight - 8}, 2, it->accentColor, true);

        // 4. Vector Icon Badge
        const Rect iconBounds{x + 14, y + 16, 24, 24};
        surface.drawRoundedRect(iconBounds, 6, Color::fromRgba(255, 255, 255, 15), true);
        IconRenderer::draw(surface, it->iconId, Point{iconBounds.x + 4, iconBounds.y + 4}, 16, it->accentColor);

        // 5. Title Text
        surface.drawString(Point{x + 48, y + 14}, it->title, Color::fromHex(0x00D4FF));

        // 6. Message Body
        surface.drawString(Point{x + 48, y + 34}, it->message, Color::fromHex(0xD0DCF0));

        // 7. Timestamp & sub-info
        surface.drawString(Point{x + 48, y + 54}, it->timestamp, Color::fromHex(0x607898));

        // 8. Close [x] button in top right
        const Rect closeR = mutableToast->closeBtnBounds;
        surface.drawRoundedRect(closeR, 3, Color::fromRgba(255, 255, 255, 10), true);
        surface.drawString(Point{closeR.x + 4, closeR.y + 2}, "x", Color::fromHex(0x94A8C4));

        // 9. Life duration progress bar (diminishing along bottom edge)
        if (it->maxLifeTicks > 0) {
            const int32_t barW = (toastWidth - 16) * it->lifeTicks / it->maxLifeTicks;
            surface.fillRect(Rect{x + 8, y + toastHeight - 3, barW, 2}, it->accentColor);
        }

        curY -= (toastHeight + spacing);
    }
}

} // namespace surshell
