// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/tray.hpp)
//
// System Tray Notification Area (TrayNotifyWnd), Shell_NotifyIcon handler,
// system clock, network telemetry, and quick status indicators.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include <string>
#include <vector>
#include <chrono>
#include <optional>
#include <functional>

namespace surshell {

struct TrayIcon {
    std::string id;
    std::string glyph;
    std::string tooltip;
    bool visible{true};
    Rect bounds{};
};

class SystemTray {
public:
    using TrayIconClickCallback = std::function<void(const std::string& id, MouseButton button)>;

    SystemTray();

    void addIcon(std::string id, std::string glyph, std::string tooltip = "");
    void removeIcon(std::string_view id);
    void updateTooltip(std::string_view id, std::string tooltip);
    void setIconVisible(std::string_view id, bool visible);

    void setClickCallback(TrayIconClickCallback cb) { clickCallback_ = std::move(cb); }

    void setVolumeLevel(uint32_t percent) noexcept { volumePercent_ = std::clamp(percent, 0u, 100u); }
    void setNetworkOnline(bool online) noexcept { networkOnline_ = online; }
    void setTimeOverride(std::string timeStr) { timeOverride_ = std::move(timeStr); }

    [[nodiscard]] std::string currentTimeString() const;
    [[nodiscard]] int32_t preferredWidth() const noexcept;

    void onMouseMove(Point pt);
    void onMouseDown(Point pt, MouseButton button);

    void render(Surface& surface, Rect trayRect);

private:
    std::vector<TrayIcon> icons_{};
    uint32_t volumePercent_{80};
    bool networkOnline_{true};
    std::string timeOverride_{};
    std::optional<std::string> hoveredIconId_{};
    TrayIconClickCallback clickCallback_{};
};

} // namespace surshell
