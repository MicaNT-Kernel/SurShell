// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/services.hpp)
//
// Sovereign Services Management Console (services.msc Parity)
// Clean-room ISO C++23, zero telemetry, Windows Service Control Manager (SCM)
// discovery, state control (Start, Stop, Pause, Restart), Startup configuration,
// PID tracking, and Extended Details Inspector.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "window_manager.hpp"
#include "theme.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace surshell {

enum class ServiceState {
    Running,
    Stopped,
    Paused,
    StartPending,
    StopPending,
    Unknown
};

enum class ServiceStartup {
    Automatic,
    AutomaticDelayed,
    Manual,
    Disabled,
    Boot,
    System,
    Unknown
};

inline std::string serviceStateToString(ServiceState st) {
    switch (st) {
        case ServiceState::Running: return "Running";
        case ServiceState::Stopped: return "Stopped";
        case ServiceState::Paused: return "Paused";
        case ServiceState::StartPending: return "Starting";
        case ServiceState::StopPending: return "Stopping";
        default: return "Unknown";
    }
}

inline std::string serviceStartupToString(ServiceStartup su) {
    switch (su) {
        case ServiceStartup::Automatic: return "Automatic";
        case ServiceStartup::AutomaticDelayed: return "Automatic (Delayed)";
        case ServiceStartup::Manual: return "Manual";
        case ServiceStartup::Disabled: return "Disabled";
        case ServiceStartup::Boot: return "Boot";
        case ServiceStartup::System: return "System";
        default: return "Unknown";
    }
}

struct ServiceEntry {
    std::string name;          // Service key / programmatic name (e.g. "Dhcp", "EventLog", "Spooler")
    std::string displayName;   // Human-readable title (e.g. "DHCP Client", "Windows Event Log")
    ServiceState state{ServiceState::Stopped};
    ServiceStartup startup{ServiceStartup::Manual};
    uint32_t pid{0};
    std::string logOnAs{"LocalSystem"};
    std::string description{};
    Rect bounds{};
};

class ServicesContent : public IWindowContent {
public:
    ServicesContent();

    void scanServices();
    void refresh() { scanServices(); }

    [[nodiscard]] size_t totalServicesCount() const noexcept { return services_.size(); }
    [[nodiscard]] size_t runningServicesCount() const noexcept;
    [[nodiscard]] size_t stoppedServicesCount() const noexcept;
    [[nodiscard]] const std::vector<ServiceEntry>& services() const noexcept { return services_; }
    [[nodiscard]] const ServiceEntry* selectedService() const noexcept;

    // Selection & Navigation
    void selectServiceByName(const std::string& name);
    void selectIndex(size_t index);
    [[nodiscard]] int32_t selectedIndex() const noexcept { return selectedIndex_; }

    // Service Actions
    bool startSelectedService();
    bool stopSelectedService();
    bool restartSelectedService();
    bool pauseSelectedService();

    // Filtering
    void setSearchQuery(const std::string& query);
    [[nodiscard]] const std::string& searchQuery() const noexcept { return searchQuery_; }

    // Properties Dialog
    void openPropertiesDialog();
    void closePropertiesDialog();
    [[nodiscard]] bool isPropertiesDialogOpen() const noexcept { return showPropertiesModal_; }

    // Callbacks
    void setToastCallback(std::function<void(const std::string&, const std::string&, IconId)> cb) {
        onToast_ = std::move(cb);
    }

    // IWindowContent overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseUp(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onMouseWheel(Point localPt, int32_t delta) override;
    bool onCharInput(char c) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;

private:
    void renderToolbar(Surface& s, int32_t w, const ThemePalette& palette);
    void renderServiceList(Surface& s, int32_t w, int32_t h, const ThemePalette& palette);
    void renderExtendedDetails(Surface& s, int32_t w, int32_t h, const ThemePalette& palette);
    void renderPropertiesModal(Surface& s, int32_t w, int32_t h, const ThemePalette& palette);
    void updateFilteredIndices();

    std::vector<ServiceEntry> services_{};
    std::vector<size_t> filteredIndices_{};
    int32_t selectedIndex_{-1};
    int32_t scrollOffset_{0};
    std::string searchQuery_{};

    // UI Bounds
    Rect searchBoxBounds_{};
    Rect btnStart_{};
    Rect btnPause_{};
    Rect btnStop_{};
    Rect btnRestart_{};
    Rect btnRefresh_{};
    Rect btnProperties_{};

    // Hover states
    bool hoverStart_{false};
    bool hoverPause_{false};
    bool hoverStop_{false};
    bool hoverRestart_{false};
    bool hoverRefresh_{false};
    bool hoverProperties_{false};
    bool searchFocused_{false};

    // Modal
    bool showPropertiesModal_{false};
    Rect modalBounds_{};
    Rect modalCloseBtn_{};

    std::function<void(const std::string&, const std::string&, IconId)> onToast_{};
};

} // namespace surshell
