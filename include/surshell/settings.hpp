// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/settings.hpp)
//
// Modern System Settings & Personalization Center (control.exe / settings.exe).
// Comprehensive Master Control Center for MicaNT:
// - System (Display, Audio, Power, Storage, Multitasking)
// - Personalization (Themes, Accents, Wallpapers, Translucency)
// - Taskbar & Dock (Alignment, Style, Icons, Behaviors, Top Diagnostic Bar)
// - Network & Internet (Ethernet, Wi-Fi, Sovereign DNS, Stealth Firewall)
// - Apps & Features (Default Terminal, Startup Services, Installed Catalog)
// - Privacy & Security (SentinelSec Enclave, HVCI, Zero-Telemetry Purge)
// - Time & Language (24H Clock, NTP Synchronization, Time Zones, Locale)
// - Developer & Enclave Diagnostics (Developer Mode, LPC Port, System Launchers)
// - About MicaNT (System Specs, Device Rename, Provenance Certificate)
// ============================================================================

#pragma once

#include "types.hpp"
#include "window_manager.hpp"
#include "theme.hpp"
#include "desktop.hpp"
#include "taskbar.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <functional>
#include <algorithm>

namespace surshell {

enum class SettingsCategory {
    System = 0,
    Personalization,
    TaskbarDock,
    Network,
    Apps,
    PrivacySecurity,
    TimeLanguage,
    Developer,
    About
};

enum class PowerMode {
    BestPerformance = 0,
    Balanced,
    PowerSaver
};

enum class DnsResolver {
    CloudflareSovereign = 0,
    Quad9,
    LocalGateway
};

enum class DefaultTerminal {
    Cmd = 0,
    Pwsh,
    SurShell
};

enum class DisplayScaling {
    Scale100 = 0,
    Scale125
};

struct AccentColorOption {
    std::string name;
    Color color;
    Rect bounds{};
};

struct WallpaperOption {
    std::string name;
    WallpaperStyle style;
    Rect bounds{};
};

class SettingsContent : public IWindowContent {
public:
    SettingsContent();

    [[nodiscard]] SettingsCategory activeCategory() const noexcept { return activeCategory_; }
    void setActiveCategory(SettingsCategory cat) noexcept { 
        activeCategory_ = cat;
        scrollY_ = 0;
    }

    // Callbacks to notify Desktop Coordinator / Subsystems
    using ThemeModeCallback = std::function<void(ThemeMode)>;
    using AccentColorCallback = std::function<void(Color)>;
    using WallpaperCallback = std::function<void(WallpaperStyle)>;
    using TaskbarAlignmentCallback = std::function<void(TaskbarAlignment)>;
    using TaskbarStyleCallback = std::function<void(TaskbarStyle)>;
    using TopBarCallback = std::function<void(bool)>;
    using VolumeCallback = std::function<void(int32_t volume, bool muted)>;
    using TimeFormatCallback = std::function<void(bool is24H)>;
    using LaunchAppCallback = std::function<void(const std::string& appId)>;
    using ToastNotificationCallback = std::function<void(const std::string& title, const std::string& body, IconId icon)>;

    void setThemeModeCallback(ThemeModeCallback cb) { onThemeMode_ = std::move(cb); }
    void setAccentColorCallback(AccentColorCallback cb) { onAccentColor_ = std::move(cb); }
    void setWallpaperCallback(WallpaperCallback cb) { onWallpaper_ = std::move(cb); }
    void setTaskbarAlignmentCallback(TaskbarAlignmentCallback cb) { onTaskbarAlignment_ = std::move(cb); }
    void setTaskbarStyleCallback(TaskbarStyleCallback cb) { onTaskbarStyle_ = std::move(cb); }
    void setTopBarCallback(TopBarCallback cb) { onTopBar_ = std::move(cb); }
    void setVolumeCallback(VolumeCallback cb) { onVolume_ = std::move(cb); }
    void setTimeFormatCallback(TimeFormatCallback cb) { onTimeFormat_ = std::move(cb); }
    void setLaunchAppCallback(LaunchAppCallback cb) { onLaunchApp_ = std::move(cb); }
    void setToastCallback(ToastNotificationCallback cb) { onToast_ = std::move(cb); }

    // State getters/setters for initial synchronization & inspection
    void setCurrentThemeMode(ThemeMode mode) noexcept { currentThemeMode_ = mode; }
    void setCurrentWallpaper(WallpaperStyle ws) noexcept { currentWallpaper_ = ws; }
    void setCurrentTaskbarAlignment(TaskbarAlignment al) noexcept { currentTaskbarAlignment_ = al; }
    void setCurrentTaskbarStyle(TaskbarStyle st) noexcept { currentTaskbarStyle_ = st; }
    void setTopBarEnabled(bool enabled) noexcept { topBarEnabled_ = enabled; }

    void setMasterVolume(int32_t vol) noexcept { masterVolume_ = std::clamp(vol, 0, 100); }
    void setIsMuted(bool muted) noexcept { isMuted_ = muted; }
    void setSpatialAudio(bool sa) noexcept { spatialAudio_ = sa; }
    void setDisplayScaling(DisplayScaling sc) noexcept { displayScaling_ = sc; }
    void setRefreshRate120Hz(bool r120) noexcept { refreshRate120Hz_ = r120; }
    void setHdrEnabled(bool hdr) noexcept { hdrEnabled_ = hdr; }
    void setPowerMode(PowerMode pm) noexcept { powerMode_ = pm; }
    void setStorageSense(bool ss) noexcept { storageSense_ = ss; }
    void setAeroSnapAssist(bool as) noexcept { aeroSnapAssist_ = as; }
    void setTransparencyEffects(bool te) noexcept { transparencyEffects_ = te; }
    void setTaskbarAutoHide(bool ah) noexcept { taskbarAutoHide_ = ah; }
    void setTaskbarBadges(bool tb) noexcept { taskbarBadges_ = tb; }
    void setWifiEnabled(bool wifi) noexcept { wifiEnabled_ = wifi; }
    void setDnsResolver(DnsResolver dns) noexcept { dnsResolver_ = dns; }
    void setStealthFirewall(bool fw) noexcept { stealthFirewall_ = fw; }
    void setDefaultTerminal(DefaultTerminal dt) noexcept { defaultTerminal_ = dt; }
    void setClockFormat24H(bool c24) noexcept { clockFormat24H_ = c24; }
    void setDeveloperMode(bool dev) noexcept { developerMode_ = dev; }
    void setDeviceName(std::string name) { deviceName_ = std::move(name); }

    [[nodiscard]] ThemeMode currentThemeMode() const noexcept { return currentThemeMode_; }
    [[nodiscard]] WallpaperStyle currentWallpaper() const noexcept { return currentWallpaper_; }
    [[nodiscard]] TaskbarAlignment currentTaskbarAlignment() const noexcept { return currentTaskbarAlignment_; }
    [[nodiscard]] TaskbarStyle currentTaskbarStyle() const noexcept { return currentTaskbarStyle_; }
    [[nodiscard]] bool topBarEnabled() const noexcept { return topBarEnabled_; }
    [[nodiscard]] int32_t masterVolume() const noexcept { return masterVolume_; }
    [[nodiscard]] bool isMuted() const noexcept { return isMuted_; }
    [[nodiscard]] bool spatialAudio() const noexcept { return spatialAudio_; }
    [[nodiscard]] DisplayScaling displayScaling() const noexcept { return displayScaling_; }
    [[nodiscard]] bool refreshRate120Hz() const noexcept { return refreshRate120Hz_; }
    [[nodiscard]] bool hdrEnabled() const noexcept { return hdrEnabled_; }
    [[nodiscard]] PowerMode powerMode() const noexcept { return powerMode_; }
    [[nodiscard]] bool storageSense() const noexcept { return storageSense_; }
    [[nodiscard]] bool aeroSnapAssist() const noexcept { return aeroSnapAssist_; }
    [[nodiscard]] bool transparencyEffects() const noexcept { return transparencyEffects_; }
    [[nodiscard]] bool taskbarAutoHide() const noexcept { return taskbarAutoHide_; }
    [[nodiscard]] bool taskbarBadges() const noexcept { return taskbarBadges_; }
    [[nodiscard]] bool wifiEnabled() const noexcept { return wifiEnabled_; }
    [[nodiscard]] DnsResolver dnsResolver() const noexcept { return dnsResolver_; }
    [[nodiscard]] bool stealthFirewall() const noexcept { return stealthFirewall_; }
    [[nodiscard]] DefaultTerminal defaultTerminal() const noexcept { return defaultTerminal_; }
    [[nodiscard]] bool clockFormat24H() const noexcept { return clockFormat24H_; }
    [[nodiscard]] bool developerMode() const noexcept { return developerMode_; }
    [[nodiscard]] const std::string& deviceName() const noexcept { return deviceName_; }
    [[nodiscard]] const std::string& searchQuery() const noexcept { return searchQuery_; }

    void setSearchQuery(std::string q);

    // IWindowContent overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onMouseWheel(Point localPt, int32_t delta) override;
    bool onCharInput(char c) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;

private:
    SettingsCategory activeCategory_{SettingsCategory::System};
    int32_t hoveredCategory_{-1};
    int32_t scrollY_{0};
    int32_t maxScrollY_{0};

    // State storage
    ThemeMode currentThemeMode_{ThemeMode::Dark};
    WallpaperStyle currentWallpaper_{WallpaperStyle::MicaGrid};
    TaskbarAlignment currentTaskbarAlignment_{TaskbarAlignment::Center};
    TaskbarStyle currentTaskbarStyle_{TaskbarStyle::FloatingIsland};
    bool topBarEnabled_{false};

    int32_t masterVolume_{85};
    bool isMuted_{false};
    bool spatialAudio_{true};
    DisplayScaling displayScaling_{DisplayScaling::Scale100};
    bool refreshRate120Hz_{true};
    bool hdrEnabled_{true};
    PowerMode powerMode_{PowerMode::BestPerformance};
    bool storageSense_{true};
    bool aeroSnapAssist_{true};
    bool transparencyEffects_{true};
    bool taskbarAutoHide_{false};
    bool taskbarBadges_{true};
    bool wifiEnabled_{true};
    DnsResolver dnsResolver_{DnsResolver::CloudflareSovereign};
    bool stealthFirewall_{true};
    DefaultTerminal defaultTerminal_{DefaultTerminal::Cmd};
    bool clockFormat24H_{false};
    bool developerMode_{true};
    std::string deviceName_{"MICANT-WORKSTATION"};

    // Startup toggles
    bool startupSentinel_{true};
    bool startupCompositor_{true};
    bool startupLpc_{true};

    // App permissions
    bool permCameraPrompt_{true};
    bool permMicPrompt_{true};
    bool coreIsolationHvci_{true};
    bool kernelDmaProtect_{true};

    // Time settings
    std::string timeZone_{"UTC-07:00 Pacific Time (US & Canada)"};
    bool ntpAutoSync_{true};

    // Search state
    std::string searchQuery_{};
    bool searchFocused_{false};

    std::vector<AccentColorOption> accentColors_{};
    std::vector<WallpaperOption> wallpapers_{};

    // Cached hit regions
    Rect searchBoxBounds_{};
    Rect searchClearBounds_{};
    std::vector<Rect> categoryBounds_{};

    // Personalization hit regions
    Rect btnDarkTheme_{};
    Rect btnLightTheme_{};
    Rect btnCarbonTheme_{};
    Rect btnTransparency_{};

    // Taskbar hit regions
    Rect btnAlignLeft_{};
    Rect btnAlignCenter_{};
    Rect btnStyleIsland_{};
    Rect btnStyleDock_{};
    Rect btnAutoHide_{};
    Rect btnBadges_{};
    Rect btnTopBar_{};

    // System hit regions
    Rect btnScale100_{};
    Rect btnScale125_{};
    Rect btnRefresh60_{};
    Rect btnRefresh120_{};
    Rect btnHdrToggle_{};
    Rect volumeSliderTrackR_{};
    Rect btnVolumeMute_{};
    Rect btnSpatialAudio_{};
    Rect btnPowerPerf_{};
    Rect btnPowerBal_{};
    Rect btnPowerSave_{};
    Rect btnStorageSense_{};
    Rect btnAeroSnap_{};

    // Network hit regions
    Rect btnWifiToggle_{};
    Rect btnDnsCloudflare_{};
    Rect btnDnsQuad9_{};
    Rect btnDnsDhcp_{};
    Rect btnFirewall_{};

    // Apps hit regions
    Rect btnTermCmd_{};
    Rect btnTermPwsh_{};
    Rect btnTermSurShell_{};
    Rect btnStartupSentinel_{};
    Rect btnStartupCompositor_{};
    Rect btnStartupLpc_{};

    // Privacy hit regions
    Rect btnCoreIsolation_{};
    Rect btnDmaProtect_{};
    Rect btnCameraEnclave_{};
    Rect btnMicEnclave_{};

    // Time hit regions
    Rect btnClock24H_{};
    Rect btnNtpSync_{};
    Rect btnSyncNow_{};
    Rect btnTzPacific_{};
    Rect btnTzEastern_{};
    Rect btnTzUtc_{};
    Rect btnTzCet_{};

    // Developer hit regions
    Rect btnDevMode_{};
    Rect btnLaunchTaskMgr_{};
    Rect btnLaunchTerminal_{};
    Rect btnLaunchRegEdit_{};

    // About hit regions
    Rect btnRenamePc_{};
    Rect btnCopySpecs_{};

    // Callbacks
    ThemeModeCallback onThemeMode_{};
    AccentColorCallback onAccentColor_{};
    WallpaperCallback onWallpaper_{};
    TaskbarAlignmentCallback onTaskbarAlignment_{};
    TaskbarStyleCallback onTaskbarStyle_{};
    TopBarCallback onTopBar_{};
    VolumeCallback onVolume_{};
    TimeFormatCallback onTimeFormat_{};
    LaunchAppCallback onLaunchApp_{};
    ToastNotificationCallback onToast_{};

    void renderSidebar(Surface& s, const ThemePalette& palette);
    void renderSystemPage(Surface& s, const ThemePalette& palette, Rect contentR);
    void renderPersonalizationPage(Surface& s, const ThemePalette& palette, Rect contentR);
    void renderTaskbarPage(Surface& s, const ThemePalette& palette, Rect contentR);
    void renderNetworkPage(Surface& s, const ThemePalette& palette, Rect contentR);
    void renderAppsPage(Surface& s, const ThemePalette& palette, Rect contentR);
    void renderPrivacyPage(Surface& s, const ThemePalette& palette, Rect contentR);
    void renderTimePage(Surface& s, const ThemePalette& palette, Rect contentR);
    void renderDeveloperPage(Surface& s, const ThemePalette& palette, Rect contentR);
    void renderAboutPage(Surface& s, const ThemePalette& palette, Rect contentR);

    void executeSearchFilter();
};

} // namespace surshell
