// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/start_menu.hpp)
//
// Modern Mica Start Menu, application catalog, live search filter, and power controls.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace surshell {

enum class PowerAction {
    Lock = 0,
    Sleep,
    Restart,
    ShutDown
};

class StartMenu {
public:
    using LaunchCallback = std::function<void(const ShellAppEntry&)>;
    using PowerCallback = std::function<void(PowerAction)>;

    StartMenu();

    void registerApp(ShellAppEntry app);
    void unregisterApp(std::string_view appId);

    void setLaunchCallback(LaunchCallback cb) { launchCallback_ = std::move(cb); }
    void setPowerCallback(PowerCallback cb) { powerCallback_ = std::move(cb); }

    [[nodiscard]] bool isOpen() const noexcept { return isOpen_; }
    void open() noexcept;
    void close() noexcept;
    void toggle() noexcept;

    void setSearchQuery(std::string query);
    void handleCharInput(char c);
    void handleBackspace();

    [[nodiscard]] Rect calculateBounds(uint32_t screenWidth, uint32_t screenHeight, int32_t taskbarHeight) const noexcept;
    [[nodiscard]] const std::vector<ShellAppEntry>& filteredApps() const noexcept { return filteredApps_; }

    void onMouseDown(Point pt, MouseButton button, Rect menuBounds);
    void onMouseMove(Point pt, Rect menuBounds);

    void render(Surface& surface, Rect menuBounds);

private:
    bool isOpen_{false};
    std::string searchQuery_{};
    std::vector<ShellAppEntry> allApps_{};
    std::vector<ShellAppEntry> filteredApps_{};
    int32_t hoveredAppIndex_{-1};
    int32_t hoveredPowerIndex_{-1};

    LaunchCallback launchCallback_{};
    PowerCallback powerCallback_{};

    void refreshFilter();
};

} // namespace surshell
