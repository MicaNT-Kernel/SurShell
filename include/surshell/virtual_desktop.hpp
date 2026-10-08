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
#include "icons.hpp"
#include <string>
#include <vector>
#include <unordered_set>
#include <functional>
#include <optional>

namespace surshell {

struct TaskViewWindowCard {
    uint32_t windowId{0};
    std::string title;
    IconId iconId{IconId::FileGeneric};
    Rect originalBounds{};
    const Surface* previewSurface{nullptr};
    bool isActive{false};
    bool isMinimized{false};
    Rect cardBounds{};
    Rect closeButtonBounds{};
};

struct VirtualDesktop {
    uint32_t id{1};
    std::string name{"Desktop 1"};
    std::unordered_set<uint32_t> windowIds{};
    Rect switcherCardBounds{};
    Rect closeButtonBounds{};
};

class VirtualDesktopManager {
public:
    using SwitchCallback = std::function<void(size_t)>;
    using WindowsProvider = std::function<std::vector<TaskViewWindowCard>()>;
    using WindowActionCallback = std::function<void(uint32_t windowId)>;

    VirtualDesktopManager();

    void updateLayout(int32_t screenWidth, int32_t screenHeight, int32_t taskbarHeight);

    [[nodiscard]] size_t desktopCount() const noexcept { return desktops_.size(); }
    [[nodiscard]] size_t activeIndex() const noexcept { return activeIndex_; }
    [[nodiscard]] const VirtualDesktop& activeDesktop() const { return desktops_[activeIndex_]; }
    [[nodiscard]] const std::vector<VirtualDesktop>& desktops() const noexcept { return desktops_; }

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

    [[nodiscard]] Rect switcherBounds() const noexcept { return switcherBounds_; }
    [[nodiscard]] Rect addDesktopButtonBounds() const noexcept { return addDesktopButtonBounds_; }
    [[nodiscard]] const std::vector<TaskViewWindowCard>& activeWindowCards() const noexcept { return activeWindowCards_; }

    bool onMouseDown(Point pt, MouseButton btn);
    bool onMouseMove(Point pt);

    void renderSwitcher(Surface& surface, const ThemePalette& theme);

    void setSwitchCallback(SwitchCallback cb) { onDesktopSwitched_ = std::move(cb); }
    void setWindowsProvider(WindowsProvider provider) { windowsProvider_ = std::move(provider); }
    void setWindowSelectCallback(WindowActionCallback cb) { windowSelectCallback_ = std::move(cb); }
    void setWindowCloseCallback(WindowActionCallback cb) { windowCloseCallback_ = std::move(cb); }

private:
    std::vector<VirtualDesktop> desktops_;
    size_t activeIndex_{0};
    uint32_t nextId_{1};
    std::unordered_set<uint32_t> pinnedWindows_;

    bool isSwitcherVisible_{false};
    Rect switcherBounds_{};
    Rect addDesktopButtonBounds_{};

    int32_t screenWidth_{1920};
    int32_t screenHeight_{1080};
    int32_t taskbarHeight_{48};

    int32_t hoveredDesktopIndex_{-1};
    int32_t hoveredDesktopCloseIndex_{-1};
    int32_t hoveredWindowId_{-1};
    int32_t hoveredWindowCloseId_{-1};
    bool isAddDesktopHovered_{false};

    std::vector<TaskViewWindowCard> activeWindowCards_{};

    SwitchCallback onDesktopSwitched_;
    WindowsProvider windowsProvider_;
    WindowActionCallback windowSelectCallback_;
    WindowActionCallback windowCloseCallback_;

    void layoutWindowCards();
};

} // namespace surshell
