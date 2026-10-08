// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/virtual_desktop.hpp)
//
// Modern Virtual Desktop & Workspace Manager (Windows 11/2026-style ergonomics).
// Provides multi-workspace window grouping, switching, and Task View overview.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "theme.hpp"
#include <string>
#include <vector>
#include <unordered_set>
#include <functional>

namespace surshell {

struct VirtualDesktop {
    uint32_t id{1};
    std::string name{"Desktop 1"};
    std::unordered_set<uint32_t> windowIds{};
    Rect switcherCardBounds{};
};

class VirtualDesktopManager {
public:
    VirtualDesktopManager();

    void updateLayout(int32_t screenWidth, int32_t screenHeight, int32_t taskbarHeight);

    [[nodiscard]] size_t desktopCount() const noexcept { return desktops_.size(); }
    [[nodiscard]] size_t activeIndex() const noexcept { return activeIndex_; }
    [[nodiscard]] const VirtualDesktop& activeDesktop() const { return desktops_[activeIndex_]; }

    void switchDesktop(size_t index);
    void nextDesktop();
    void previousDesktop();
    uint32_t createDesktop(std::string_view name = "");
    bool removeDesktop(size_t index);

    void assignWindowToDesktop(uint32_t windowId, size_t desktopIndex);
    void unassignWindow(uint32_t windowId);
    void pinWindowToAllDesktops(uint32_t windowId, bool pinned = true);
    [[nodiscard]] bool isWindowVisible(uint32_t windowId) const;

    // Task View Switcher HUD
    [[nodiscard]] bool isSwitcherVisible() const noexcept { return isSwitcherVisible_; }
    void showSwitcher() noexcept { isSwitcherVisible_ = true; }
    void hideSwitcher() noexcept { isSwitcherVisible_ = false; }
    void toggleSwitcher() noexcept { isSwitcherVisible_ = !isSwitcherVisible_; }

    bool onMouseDown(Point pt, MouseButton btn);
    bool onMouseMove(Point pt);

    void renderSwitcher(Surface& surface, const ThemePalette& theme);

    using SwitchCallback = std::function<void(size_t)>;
    void setSwitchCallback(SwitchCallback cb) { onDesktopSwitched_ = std::move(cb); }

private:
    std::vector<VirtualDesktop> desktops_;
    size_t activeIndex_{0};
    uint32_t nextId_{1};
    std::unordered_set<uint32_t> pinnedWindows_;

    bool isSwitcherVisible_{false};
    Rect switcherBounds_{};
    Rect addDesktopButtonBounds_{};

    SwitchCallback onDesktopSwitched_;
};

} // namespace surshell
