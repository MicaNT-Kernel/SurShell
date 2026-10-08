// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/action_center.cpp)
// ============================================================================

#include "surshell/action_center.hpp"
#include <algorithm>
#include <iomanip>

namespace surshell {

static const char* kMonthNames[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

static int getDaysInMonth(int year, int month) {
    if (month == 2) {
        const bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return isLeap ? 29 : 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
    return 31;
}

static int getDayOfWeek(int y, int m, int d) {
    // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) y -= 1;
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

ActionCenterFlyout::ActionCenterFlyout() {
    addNotification("SentinelSec Security", "Kernel enclave active. 0 anomalies detected.", IconId::SentinelSec, Color::fromHex(0x00FF9D));
    addNotification("Gigabit Ethernet", "Connected to Local Network (1 Gbps Full Duplex)", IconId::NetworkEthernet, Color::fromHex(0x00D4FF));
    addNotification("SurWin Subsystem", "CSRSS LPC clean-room bridge online.", IconId::StartPrism, Color::fromHex(0x2A69FF));
}

void ActionCenterFlyout::addNotification(const std::string& title, const std::string& body, IconId icon, Color accent) {
    ActionCenterNotification n;
    n.id = "notif_" + std::to_string(notifications_.size() + 1);
    n.title = title;
    n.body = body;
    n.timeStr = "Just now";
    n.icon = icon;
    n.accentColor = accent;
    notifications_.insert(notifications_.begin(), std::move(n));
}

void ActionCenterFlyout::clearAllNotifications() {
    notifications_.clear();
}

void ActionCenterFlyout::prevMonth() {
    month_--;
    if (month_ < 1) {
        month_ = 12;
        year_--;
    }
    selectedDay_ = 1;
}

void ActionCenterFlyout::nextMonth() {
    month_++;
    if (month_ > 12) {
        month_ = 1;
        year_++;
    }
    selectedDay_ = 1;
}

void ActionCenterFlyout::render(Surface& s, int32_t screenW, int32_t screenH, int32_t taskbarH) {
    if (!visible_) return;

    const int32_t flyoutW = 380;
    const int32_t flyoutH = 540;
    const int32_t flyoutX = screenW - flyoutW - 12;
    const int32_t flyoutY = screenH - taskbarH - flyoutH - 10;
    bounds_ = Rect{flyoutX, flyoutY, flyoutW, flyoutH};

    const auto& palette = ThemeManager::instance().palette();

    // 1. Drop shadow
    s.drawRoundedRect(bounds_.inflate(6, 6), 14, Color::fromRgba(0, 0, 0, 80), true);

    // 2. Acrylic container backdrop
    s.drawRoundedRect(bounds_, 10, Color::fromRgba(12, 16, 26, 240), true);
    s.drawRoundedRect(bounds_, 10, Color::fromRgba(40, 56, 80, 200), false);

    // Split into top Notifications section and bottom Calendar section
    const Rect notifR{bounds_.x, bounds_.y, bounds_.width, 220};
    const Rect calR{bounds_.x, bounds_.y + 220, bounds_.width, bounds_.height - 220};

    renderNotifications(s, palette, notifR);

    // Divider bar
    s.fillRect(Rect{bounds_.x + 16, bounds_.y + 220, bounds_.width - 32, 1}, Color::fromHex(0x243248));

    renderCalendar(s, palette, calR);
}

void ActionCenterFlyout::renderNotifications(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t padX = r.x + 16;
    int32_t curY = r.y + 14;

    // Header
    IconRenderer::draw(s, IconId::NotificationBell, Point{padX, curY}, 18, palette.accentColor);
    s.drawString(padX + 26, curY + 2, "Notification Center", palette.textPrimary, 1);

    // Clear All button
    btnClearAll_ = Rect{r.right() - 86, curY - 2, 70, 22};
    if (!notifications_.empty()) {
        if (hoverClearAll_) {
            s.drawRoundedRect(btnClearAll_, 4, Color::fromRgba(255, 255, 255, 25), true);
        }
        s.drawString(btnClearAll_.x + 6, btnClearAll_.y + 4, "Clear all",
                     hoverClearAll_ ? Color::fromHex(0xFFFFFF) : palette.textSecondary, 1);
    }

    curY += 28;

    // Focus / Do Not Disturb toggle pill
    btnFocus_ = Rect{padX, curY, 130, 24};
    s.drawRoundedRect(btnFocus_, 12, focusAssist_ ? Color::fromRgba(0, 212, 255, 40) : Color::fromHex(0x162234), true);
    s.drawRoundedRect(btnFocus_, 12, focusAssist_ ? palette.accentColor : Color::fromHex(0x354765), false);
    s.drawString(btnFocus_.x + 12, btnFocus_.y + 5, focusAssist_ ? "Focus: ON" : "Focus: OFF",
                 focusAssist_ ? Color::fromHex(0xFFFFFF) : palette.textSecondary, 1);

    curY += 34;

    // Notification Cards list
    if (notifications_.empty()) {
        s.drawString(padX + 20, curY + 24, "No new notifications", palette.textSecondary, 1);
        s.drawString(padX + 20, curY + 42, "You're all caught up!", Color::fromHex(0x7186A4), 1);
        return;
    }

    const int32_t cardH = 50;
    const size_t maxShow = std::min(notifications_.size(), size_t{2});

    for (size_t i = 0; i < maxShow; ++i) {
        auto& n = notifications_[i];
        n.cardBounds = Rect{padX, curY, r.width - 32, cardH};
        n.dismissBounds = Rect{n.cardBounds.right() - 20, n.cardBounds.y + 6, 14, 14};

        const bool isCardHover = (hoveredNotification_ == static_cast<int32_t>(i));
        const bool isDismissHover = (hoveredDismiss_ == static_cast<int32_t>(i));

        s.drawRoundedRect(n.cardBounds, 6, isCardHover ? Color::fromHex(0x1A2536) : Color::fromHex(0x141C2A), true);
        s.drawRoundedRect(n.cardBounds, 6, Color::fromHex(0x283850), false);

        // Severity accent stripe on left edge
        s.fillRect(Rect{n.cardBounds.x + 2, n.cardBounds.y + 4, 3, n.cardBounds.height - 8}, n.accentColor);

        // App Icon
        IconRenderer::draw(s, n.icon, Point{n.cardBounds.x + 12, n.cardBounds.y + 8}, 16, n.accentColor);

        // Title and time
        s.drawString(n.cardBounds.x + 36, n.cardBounds.y + 8, n.title, palette.textPrimary, 1);

        // Body message
        std::string bodyDisp = n.body;
        if (bodyDisp.size() > 36) bodyDisp = bodyDisp.substr(0, 33) + "...";
        s.drawString(n.cardBounds.x + 36, n.cardBounds.y + 26, bodyDisp, palette.textSecondary, 1);

        // Dismiss [x]
        if (isDismissHover) {
            s.drawRoundedRect(n.dismissBounds, 2, Color::fromHex(0xFF4D6D), true);
            s.drawString(n.dismissBounds.x + 3, n.dismissBounds.y + 1, "x", Color::fromHex(0xFFFFFF), 1);
        } else {
            s.drawString(n.dismissBounds.x + 3, n.dismissBounds.y + 1, "x", Color::fromHex(0x7186A4), 1);
        }

        curY += cardH + 8;
    }
}

void ActionCenterFlyout::renderCalendar(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t padX = r.x + 16;
    int32_t curY = r.y + 12;

    // Header: Month & Year with Navigation
    const std::string monthYearStr = std::string(kMonthNames[month_ - 1]) + " " + std::to_string(year_);
    s.drawString(padX, curY + 4, monthYearStr, palette.textPrimary, 1);

    btnPrevMonth_ = Rect{r.right() - 64, curY, 20, 20};
    btnNextMonth_ = Rect{r.right() - 36, curY, 20, 20};

    if (hoverPrevMonth_) s.drawRoundedRect(btnPrevMonth_, 4, Color::fromRgba(255, 255, 255, 25), true);
    if (hoverNextMonth_) s.drawRoundedRect(btnNextMonth_, 4, Color::fromRgba(255, 255, 255, 25), true);

    s.drawString(btnPrevMonth_.x + 6, btnPrevMonth_.y + 3, "<",
                 hoverPrevMonth_ ? Color::fromHex(0xFFFFFF) : palette.textSecondary, 1);
    s.drawString(btnNextMonth_.x + 6, btnNextMonth_.y + 3, ">",
                 hoverNextMonth_ ? Color::fromHex(0xFFFFFF) : palette.textSecondary, 1);

    curY += 28;

    // Weekday headers (Su Mo Tu We Th Fr Sa)
    static const char* kDaysOfWeek[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
    const int32_t colW = (r.width - 32) / 7;
    for (int i = 0; i < 7; ++i) {
        s.drawString(padX + i * colW + 10, curY, kDaysOfWeek[i], Color::fromHex(0x7186A4), 1);
    }

    curY += 20;

    // Calendar Grid
    dayCells_.clear();
    const int firstDayOfWeek = getDayOfWeek(year_, month_, 1);
    const int daysInMonth = getDaysInMonth(year_, month_);
    const int prevMonthDays = getDaysInMonth(month_ == 1 ? year_ - 1 : year_, month_ == 1 ? 12 : month_ - 1);

    int curCol = 0;
    int curRow = 0;
    const int32_t cellH = 26;

    // Leading days from previous month
    for (int i = 0; i < firstDayOfWeek; ++i) {
        CalendarDayCell cell;
        cell.day = prevMonthDays - firstDayOfWeek + 1 + i;
        cell.isCurrentMonth = false;
        cell.bounds = Rect{padX + curCol * colW, curY + curRow * cellH, colW, cellH};
        dayCells_.push_back(cell);

        s.drawString(cell.bounds.x + 12, cell.bounds.y + 6, std::to_string(cell.day), Color::fromHex(0x3B506E), 1);
        curCol++;
    }

    // Days of current month
    for (int d = 1; d <= daysInMonth; ++d) {
        CalendarDayCell cell;
        cell.day = d;
        cell.isCurrentMonth = true;
        cell.isToday = (year_ == 2026 && month_ == 10 && d == todayDay_);
        cell.isSelected = (d == selectedDay_);
        cell.bounds = Rect{padX + curCol * colW, curY + curRow * cellH, colW, cellH};
        dayCells_.push_back(cell);

        // Highlight today or selected
        if (cell.isToday) {
            s.drawRoundedRect(cell.bounds.deflate(2, 2), 10, palette.accentColor, true);
            s.drawString(cell.bounds.x + (d < 10 ? 14 : 10), cell.bounds.y + 6, std::to_string(d), Color::fromHex(0x000000), 1);
        } else if (cell.isSelected) {
            s.drawRoundedRect(cell.bounds.deflate(2, 2), 10, Color::fromRgba(0, 212, 255, 40), true);
            s.drawRoundedRect(cell.bounds.deflate(2, 2), 10, palette.accentColor, false);
            s.drawString(cell.bounds.x + (d < 10 ? 14 : 10), cell.bounds.y + 6, std::to_string(d), Color::fromHex(0xFFFFFF), 1);
        } else {
            s.drawString(cell.bounds.x + (d < 10 ? 14 : 10), cell.bounds.y + 6, std::to_string(d), palette.textPrimary, 1);
        }

        curCol++;
        if (curCol >= 7) {
            curCol = 0;
            curRow++;
        }
    }

    // Footer: Today's date
    const int32_t footerY = r.bottom() - 24;
    s.drawString(padX, footerY, "Today: Thursday, October 8, 2026", Color::fromHex(0x00D4FF), 1);
}

bool ActionCenterFlyout::onMouseDown(Point pt, MouseButton button) {
    if (!visible_ || button != MouseButton::Left) return false;

    if (!bounds_.contains(pt)) {
        hide();
        return true;
    }

    // Check Clear All
    if (btnClearAll_.contains(pt) && !notifications_.empty()) {
        clearAllNotifications();
        return true;
    }

    // Check Focus Assist
    if (btnFocus_.contains(pt)) {
        focusAssist_ = !focusAssist_;
        return true;
    }

    // Check Month Navigation
    if (btnPrevMonth_.contains(pt)) {
        prevMonth();
        return true;
    }
    if (btnNextMonth_.contains(pt)) {
        nextMonth();
        return true;
    }

    // Check notification dismiss buttons
    for (size_t i = 0; i < notifications_.size() && i < 2; ++i) {
        if (notifications_[i].dismissBounds.contains(pt)) {
            notifications_.erase(notifications_.begin() + static_cast<ptrdiff_t>(i));
            return true;
        }
    }

    // Check calendar day clicks
    for (const auto& cell : dayCells_) {
        if (cell.isCurrentMonth && cell.bounds.contains(pt)) {
            selectedDay_ = cell.day;
            return true;
        }
    }

    return true; // Click inside flyout consumed
}

bool ActionCenterFlyout::onMouseMove(Point pt) {
    if (!visible_) return false;

    hoverClearAll_ = btnClearAll_.contains(pt);
    hoverFocus_ = btnFocus_.contains(pt);
    hoverPrevMonth_ = btnPrevMonth_.contains(pt);
    hoverNextMonth_ = btnNextMonth_.contains(pt);

    hoveredNotification_ = -1;
    hoveredDismiss_ = -1;
    for (size_t i = 0; i < notifications_.size() && i < 2; ++i) {
        if (notifications_[i].dismissBounds.contains(pt)) {
            hoveredDismiss_ = static_cast<int32_t>(i);
        } else if (notifications_[i].cardBounds.contains(pt)) {
            hoveredNotification_ = static_cast<int32_t>(i);
        }
    }

    return bounds_.contains(pt);
}

} // namespace surshell
