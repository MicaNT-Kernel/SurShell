// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/start_menu.hpp)
//
// Modern Windows 11-Style Mica Start Menu:
// Application catalog, Pinned vs All Apps views, Recommended activities,
// profile options, and interactive Power Flyout controls.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace surshell {

enum class PowerAction {
    Lock = 0,
    Sleep,
    Restart,
    ShutDown,
    SignOut,
    Hibernate
};

enum class StartViewMode {
    Pinned = 0,
    AllApps
};

struct PowerOptionItem {
    PowerAction action;
    std::string label;
    std::string description;
    IconId iconId;
};

struct RecommendedItem {
    std::string id;
    std::string title;
    std::string subtitle;
    std::string path;
    IconId iconId{IconId::FileGeneric};
};

class StartMenu {
public:
    using LaunchCallback = std::function<void(const ShellAppEntry&)>;
    using PowerCallback = std::function<void(PowerAction)>;
    using OpenItemCallback = std::function<void(const std::string& path)>;

    StartMenu();

    void registerApp(ShellAppEntry app);
    void unregisterApp(std::string_view appId);

    void setLaunchCallback(LaunchCallback cb) { launchCallback_ = std::move(cb); }
    void setPowerCallback(PowerCallback cb) { powerCallback_ = std::move(cb); }
    void setOpenItemCallback(OpenItemCallback cb) { openItemCallback_ = std::move(cb); }

    [[nodiscard]] bool isOpen() const noexcept { return isOpen_; }
    void open() noexcept;
    void close() noexcept;
    void toggle() noexcept;

    void setSearchQuery(std::string query);
    void handleCharInput(char c);
    void handleBackspace();

    [[nodiscard]] Rect calculateBounds(uint32_t screenWidth, uint32_t screenHeight, int32_t taskbarHeight) const noexcept;
    [[nodiscard]] const std::vector<ShellAppEntry>& filteredApps() const noexcept { return filteredApps_; }
    [[nodiscard]] const std::vector<RecommendedItem>& recommendedItems() const noexcept { return recommendedItems_; }

    [[nodiscard]] StartViewMode viewMode() const noexcept { return viewMode_; }
    void setViewMode(StartViewMode mode) noexcept { viewMode_ = mode; }
    void toggleViewMode() noexcept { viewMode_ = (viewMode_ == StartViewMode::Pinned) ? StartViewMode::AllApps : StartViewMode::Pinned; }

    [[nodiscard]] bool isPowerFlyoutOpen() const noexcept { return isPowerFlyoutOpen_; }
    void setPowerFlyoutOpen(bool open) noexcept { isPowerFlyoutOpen_ = open; if (open) isUserFlyoutOpen_ = false; }
    void togglePowerFlyout() noexcept { setPowerFlyoutOpen(!isPowerFlyoutOpen_); }

    [[nodiscard]] bool isUserFlyoutOpen() const noexcept { return isUserFlyoutOpen_; }
    void setUserFlyoutOpen(bool open) noexcept { isUserFlyoutOpen_ = open; if (open) isPowerFlyoutOpen_ = false; }
    void toggleUserFlyout() noexcept { setUserFlyoutOpen(!isUserFlyoutOpen_); }

    void addRecommendedItem(RecommendedItem item);

    void onMouseDown(Point pt, MouseButton button, Rect menuBounds);
    void onMouseMove(Point pt, Rect menuBounds);

    void render(Surface& surface, Rect menuBounds);

    // Layout helper queries
    [[nodiscard]] Rect powerButtonBounds(Rect menuBounds) const noexcept;
    [[nodiscard]] Rect userButtonBounds(Rect menuBounds) const noexcept;
    [[nodiscard]] Rect allAppsButtonBounds(Rect menuBounds) const noexcept;
    [[nodiscard]] Rect powerFlyoutBounds(Rect menuBounds) const noexcept;
    [[nodiscard]] Rect userFlyoutBounds(Rect menuBounds) const noexcept;

private:
    bool isOpen_{false};
    StartViewMode viewMode_{StartViewMode::Pinned};
    bool isPowerFlyoutOpen_{false};
    bool isUserFlyoutOpen_{false};

    std::string searchQuery_{};
    std::vector<ShellAppEntry> allApps_{};
    std::vector<ShellAppEntry> filteredApps_{};
    std::vector<RecommendedItem> recommendedItems_{};
    std::vector<PowerOptionItem> powerOptions_{};

    int32_t hoveredAppIndex_{-1};
    int32_t hoveredRecommendedIndex_{-1};
    int32_t hoveredPowerFlyoutIndex_{-1};
    int32_t hoveredUserFlyoutIndex_{-1};
    bool isAllAppsButtonHovered_{false};
    bool isPowerButtonHovered_{false};
    bool isUserButtonHovered_{false};

    LaunchCallback launchCallback_{};
    PowerCallback powerCallback_{};
    OpenItemCallback openItemCallback_{};

    void refreshFilter();
    void renderPinnedView(Surface& surface, Rect menuBounds);
    void renderAllAppsView(Surface& surface, Rect menuBounds);
    void renderPowerFlyout(Surface& surface, Rect flyoutBounds);
    void renderUserFlyout(Surface& surface, Rect flyoutBounds);
};

} // namespace surshell
