// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/tray.cpp)
// ============================================================================

#include "surshell/tray.hpp"
#include "surshell/theme.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>

namespace surshell {

SystemTray::SystemTray() {
    // Standard sovereign status icons
    addIcon("security", "[SEC]", "Sentinel Security Authority: Active", IconId::SentinelSec);
    addIcon("network", "[NET]", "Network: Gigabit Ethernet Connected (1000/1000 Mbps)", IconId::NetworkEthernet);
}

void SystemTray::addIcon(std::string id, std::string glyph, std::string tooltip, std::optional<IconId> iconId) {
    icons_.push_back(TrayIcon{
        .id = std::move(id),
        .glyph = std::move(glyph),
        .tooltip = std::move(tooltip),
        .iconId = iconId,
        .visible = true,
        .bounds = Rect{0, 0, 0, 0}
    });
}

void SystemTray::removeIcon(std::string_view id) {
    std::erase_if(icons_, [&](const TrayIcon& icon) { return icon.id == id; });
}

void SystemTray::updateTooltip(std::string_view id, std::string tooltip) {
    for (auto& icon : icons_) {
        if (icon.id == id) {
            icon.tooltip = std::move(tooltip);
            break;
        }
    }
}

void SystemTray::setIconVisible(std::string_view id, bool visible) {
    for (auto& icon : icons_) {
        if (icon.id == id) {
            icon.visible = visible;
            break;
        }
    }
}

std::string SystemTray::currentTimeString() const {
    if (!timeOverride_.empty()) return timeOverride_;

    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tmBuf{};
#if defined(_WIN32)
    localtime_s(&tmBuf, &t);
#else
    localtime_r(&t, &tmBuf);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%I:%M %p", &tmBuf);
    return std::string(buf);
}

std::string SystemTray::currentDateString() const {
    if (!dateOverride_.empty()) return dateOverride_;

    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tmBuf{};
#if defined(_WIN32)
    localtime_s(&tmBuf, &t);
#else
    localtime_r(&t, &tmBuf);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%m/%d/%Y", &tmBuf);
    return std::string(buf);
}

int32_t SystemTray::preferredWidth() const noexcept {
    size_t visibleIcons = 0;
    for (const auto& icon : icons_) {
        if (icon.visible) visibleIcons++;
    }
    // Chevron (16px) + Quick Controls pill (82px) + Pill margin (8px) + Extra icons + Clock pill (88px) + Peek (10px)
    return static_cast<int32_t>(16 + 82 + 8 + visibleIcons * 28 + 88 + 10);
}

void SystemTray::onMouseMove(Point pt) {
    hoveredIconId_ = std::nullopt;
    for (const auto& icon : icons_) {
        if (icon.visible && icon.bounds.contains(pt)) {
            hoveredIconId_ = icon.id;
            break;
        }
    }
}

void SystemTray::onMouseDown(Point pt, MouseButton button) {
    for (const auto& icon : icons_) {
        if (icon.visible && icon.bounds.contains(pt)) {
            if (clickCallback_) {
                clickCallback_(icon.id, button);
            }
            break;
        }
    }
}

void SystemTray::render(Surface& surface, Rect trayRect) {
    const auto& palette = ThemeManager::instance().palette();

    int32_t curX = trayRect.x + 6;
    const int32_t trayH = trayRect.height;

    // 1. Windows Hidden Icons Chevron [^]
    IconRenderer::draw(surface, IconId::NavUp, Point{curX, trayRect.y + (trayH - 12) / 2}, 12, palette.textSecondary);
    curX += 16;

    // 2. Windows 11 Quick Controls Pill (Network + Volume + Battery)
    const Rect quickPillRect{curX, trayRect.y + 4, 82, trayH - 8};
    surface.drawRoundedRect(quickPillRect, 6, Color::fromRgba(28, 38, 58, 160), true);
    surface.drawRoundedRect(quickPillRect, 6, Color::fromRgba(56, 76, 114, 120), false);

    // Network vector icon
    Color netColor = networkOnline_ ? palette.accentColor : Color::fromHex(0xFF4D4D);
    IconRenderer::draw(surface, IconId::NetworkOnline, Point{quickPillRect.x + 8, quickPillRect.y + (quickPillRect.height - 14) / 2}, 14, netColor);

    // Volume vector icon
    IconRenderer::draw(surface, (volumePercent_ == 0 ? IconId::VolumeMute : IconId::VolumeHigh), Point{quickPillRect.x + 32, quickPillRect.y + (quickPillRect.height - 14) / 2}, 14, palette.textPrimary);

    // Power / Battery vector icon
    IconRenderer::draw(surface, IconId::BatteryCharging, Point{quickPillRect.x + 56, quickPillRect.y + (quickPillRect.height - 14) / 2}, 16, Color::fromHex(0x00FF9D));

    curX = quickPillRect.right() + 8;

    // 3. User Tray Icons (if any)
    for (auto& icon : icons_) {
        if (!icon.visible) continue;

        icon.bounds = Rect{curX, trayRect.y + 4, 26, trayH - 8};
        if (hoveredIconId_ && *hoveredIconId_ == icon.id) {
            surface.drawRoundedRect(icon.bounds, 4, Color::fromRgba(255, 255, 255, 25), true);
        }

        if (icon.iconId.has_value()) {
            IconRenderer::draw(surface, *icon.iconId, Point{icon.bounds.x + 5, icon.bounds.y + (icon.bounds.height - 16) / 2}, 16);
        } else {
            surface.drawString(icon.bounds.x + 4, icon.bounds.y + 8, icon.glyph, palette.accentColor, 1);
        }
        curX += 28;
    }

    // 4. Windows 11 Digital Clock & Date Pill
    const Rect clockPillRect{curX, trayRect.y + 4, 88, trayH - 8};
    surface.drawRoundedRect(clockPillRect, 6, Color::fromRgba(28, 38, 58, 160), true);
    surface.drawRoundedRect(clockPillRect, 6, Color::fromRgba(56, 76, 114, 120), false);

    const std::string timeStr = currentTimeString();
    const std::string dateStr = currentDateString();

    // Stacked Windows Clock (Time on top, Date below)
    surface.drawString(clockPillRect.x + 10, clockPillRect.y + 4, timeStr, palette.textPrimary, 1);
    surface.drawString(clockPillRect.x + 12, clockPillRect.y + 16, dateStr, palette.textSecondary, 1);

    // 5. Far right "Show Desktop" Peek Strip (Windows signature)
    const int32_t peekX = trayRect.right() - 4;
    surface.fillRect(Rect{peekX - 2, trayRect.y + 8, 1, trayH - 16}, Color::fromRgba(255, 255, 255, 30));
}

} // namespace surshell
