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
    addIcon("security", "[SEC]", "Sentinel Security Authority: Active");
    addIcon("rosenpass", "[PQ]", "Rosenpass Post-Quantum WireGuard: Enabled");
}

void SystemTray::addIcon(std::string id, std::string glyph, std::string tooltip) {
    icons_.push_back(TrayIcon{
        .id = std::move(id),
        .glyph = std::move(glyph),
        .tooltip = std::move(tooltip),
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

int32_t SystemTray::preferredWidth() const noexcept {
    size_t visibleIcons = 0;
    for (const auto& icon : icons_) {
        if (icon.visible) visibleIcons++;
    }
    // Icon width (28px each) + Status badges (140px) + Clock (70px) + padding
    return static_cast<int32_t>(visibleIcons * 32 + 220);
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

    // Subtle container background for system tray
    surface.drawRoundedRect(trayRect, 6, Color::fromRgba(18, 25, 38, 160), true);

    int32_t curX = trayRect.x + 8;
    const int32_t centerY = trayRect.y + 12;

    // 1. Telemetry Status Badge
    surface.drawString(curX, centerY, "0 TEL", Color::fromHex(0x00FF9D), 1);
    curX += 48;

    // 2. Network Status
    Color netColor = networkOnline_ ? Color::fromHex(0x00D4FF) : Color::fromHex(0xFF4D4D);
    surface.drawString(curX, centerY, networkOnline_ ? "NET" : "DISC", netColor, 1);
    curX += 36;

    // 3. Volume
    std::string volStr = std::to_string(volumePercent_) + "%";
    surface.drawString(curX, centerY, volStr, palette.textSecondary, 1);
    curX += 38;

    // 4. Tray Icons
    for (auto& icon : icons_) {
        if (!icon.visible) continue;

        icon.bounds = Rect{curX, trayRect.y + 4, 30, trayRect.height - 8};
        if (hoveredIconId_ && *hoveredIconId_ == icon.id) {
            surface.drawRoundedRect(icon.bounds, 4, Color::fromRgba(255, 255, 255, 25), true);
        }

        surface.drawString(icon.bounds.x + 3, icon.bounds.y + 8, icon.glyph, palette.accentColor, 1);
        curX += 32;
    }

    // 5. Digital Clock
    const std::string timeStr = currentTimeString();
    surface.drawString(curX + 6, centerY, timeStr, palette.textPrimary, 1);
}

} // namespace surshell
