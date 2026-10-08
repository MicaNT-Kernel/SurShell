// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/action_center.hpp)
//
// Modern Windows 11-style Action Center & Calendar Flyout (Win+N / Tray Clock),
// integrating notification history, Focus Assist toggles, and interactive
// monthly calendar with multi-month navigation.
// ============================================================================

#pragma once

#include "surshell/types.hpp"
#include "surshell/compositor.hpp"
#include "surshell/icons.hpp"
#include "surshell/theme.hpp"
#include <string>
#include <vector>
#include <chrono>

namespace surshell {

struct ActionCenterNotification {
    std::string id;
    std::string title;
    std::string body;
    std::string timeStr;
    IconId icon{IconId::NotificationBell};
    Color accentColor{Color::fromHex(0x00D4FF)};
    Rect cardBounds{};
    Rect dismissBounds{};
};

class ActionCenterFlyout {
public:
    ActionCenterFlyout();
    ~ActionCenterFlyout() = default;

    void show() noexcept { visible_ = true; }
    void hide() noexcept { visible_ = false; }
    void toggle() noexcept { visible_ = !visible_; }
    [[nodiscard]] bool isVisible() const noexcept { return visible_; }

    void render(Surface& surface, int32_t screenWidth, int32_t screenHeight, int32_t taskbarHeight);
    bool onMouseDown(Point pt, MouseButton button);
    bool onMouseMove(Point pt);

    [[nodiscard]] Rect bounds() const noexcept { return bounds_; }

    // Calendar navigation
    void prevMonth();
    void nextMonth();
    int32_t currentYear() const noexcept { return year_; }
    int32_t currentMonth() const noexcept { return month_; }
    int32_t selectedDay() const noexcept { return selectedDay_; }

    // Notifications
    void addNotification(const std::string& title, const std::string& body, IconId icon = IconId::NotificationBell, Color accent = Color::fromHex(0x00D4FF));
    void clearAllNotifications();
    size_t notificationCount() const noexcept { return notifications_.size(); }

    // Focus session / DND
    bool focusAssist() const noexcept { return focusAssist_; }
    void setFocusAssist(bool enabled) noexcept { focusAssist_ = enabled; }

private:
    void renderNotifications(Surface& s, const ThemePalette& palette, Rect r);
    void renderCalendar(Surface& s, const ThemePalette& palette, Rect r);

    bool visible_{false};
    Rect bounds_{};

    // Header buttons
    Rect btnClearAll_{};
    Rect btnFocus_{};
    bool hoverClearAll_{false};
    bool hoverFocus_{false};
    bool focusAssist_{false};

    // Calendar state (defaults to October 2026)
    int32_t year_{2026};
    int32_t month_{10}; // 1-12
    int32_t todayDay_{8};
    int32_t selectedDay_{8};

    Rect btnPrevMonth_{};
    Rect btnNextMonth_{};
    bool hoverPrevMonth_{false};
    bool hoverNextMonth_{false};

    struct CalendarDayCell {
        int32_t day{0};
        bool isCurrentMonth{true};
        bool isToday{false};
        bool isSelected{false};
        Rect bounds{};
    };
    std::vector<CalendarDayCell> dayCells_;

    std::vector<ActionCenterNotification> notifications_;
    int32_t hoveredNotification_{-1};
    int32_t hoveredDismiss_{-1};
};

} // namespace surshell
