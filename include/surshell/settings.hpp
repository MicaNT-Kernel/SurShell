// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/settings.hpp)
//
// Modern System Settings & Personalization Center (control.exe / settings.exe).
// Provides category navigation: System, Personalization (Theme, Accent, Wallpaper),
// Taskbar & Dock alignment, Network & Internet, and Sovereign About.
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

namespace surshell {

enum class SettingsCategory {
    System = 0,
    Personalization,
    TaskbarDock,
    Network,
    About
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
    void setActiveCategory(SettingsCategory cat) noexcept { activeCategory_ = cat; }

    // Callbacks to notify Desktop Coordinator
    using ThemeModeCallback = std::function<void(ThemeMode)>;
    using AccentColorCallback = std::function<void(Color)>;
    using WallpaperCallback = std::function<void(WallpaperStyle)>;
    using TaskbarAlignmentCallback = std::function<void(TaskbarAlignment)>;
    using TaskbarStyleCallback = std::function<void(TaskbarStyle)>;
    using TopBarCallback = std::function<void(bool)>;

    void setThemeModeCallback(ThemeModeCallback cb) { onThemeMode_ = std::move(cb); }
    void setAccentColorCallback(AccentColorCallback cb) { onAccentColor_ = std::move(cb); }
    void setWallpaperCallback(WallpaperCallback cb) { onWallpaper_ = std::move(cb); }
    void setTaskbarAlignmentCallback(TaskbarAlignmentCallback cb) { onTaskbarAlignment_ = std::move(cb); }
    void setTaskbarStyleCallback(TaskbarStyleCallback cb) { onTaskbarStyle_ = std::move(cb); }
    void setTopBarCallback(TopBarCallback cb) { onTopBar_ = std::move(cb); }

    // State getters/setters for initial synchronization
    void setCurrentThemeMode(ThemeMode mode) noexcept { currentThemeMode_ = mode; }
    void setCurrentWallpaper(WallpaperStyle ws) noexcept { currentWallpaper_ = ws; }
    void setCurrentTaskbarAlignment(TaskbarAlignment al) noexcept { currentTaskbarAlignment_ = al; }
    void setCurrentTaskbarStyle(TaskbarStyle st) noexcept { currentTaskbarStyle_ = st; }
    void setTopBarEnabled(bool enabled) noexcept { topBarEnabled_ = enabled; }

    [[nodiscard]] ThemeMode currentThemeMode() const noexcept { return currentThemeMode_; }
    [[nodiscard]] WallpaperStyle currentWallpaper() const noexcept { return currentWallpaper_; }
    [[nodiscard]] TaskbarAlignment currentTaskbarAlignment() const noexcept { return currentTaskbarAlignment_; }
    [[nodiscard]] TaskbarStyle currentTaskbarStyle() const noexcept { return currentTaskbarStyle_; }
    [[nodiscard]] bool topBarEnabled() const noexcept { return topBarEnabled_; }

    // IWindowContent overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;

private:
    SettingsCategory activeCategory_{SettingsCategory::System};
    int32_t hoveredCategory_{-1};

    ThemeMode currentThemeMode_{ThemeMode::Dark};
    WallpaperStyle currentWallpaper_{WallpaperStyle::MicaGrid};
    TaskbarAlignment currentTaskbarAlignment_{TaskbarAlignment::Center};
    TaskbarStyle currentTaskbarStyle_{TaskbarStyle::FloatingIsland};
    bool topBarEnabled_{false};

    std::vector<AccentColorOption> accentColors_{};
    std::vector<WallpaperOption> wallpapers_{};

    // Cached hit regions
    std::vector<Rect> categoryBounds_{};
    Rect btnDarkTheme_{};
    Rect btnLightTheme_{};
    Rect btnCarbonTheme_{};
    Rect btnAlignLeft_{};
    Rect btnAlignCenter_{};
    Rect btnStyleIsland_{};
    Rect btnStyleDock_{};
    Rect btnTopBar_{};

    ThemeModeCallback onThemeMode_{};
    AccentColorCallback onAccentColor_{};
    WallpaperCallback onWallpaper_{};
    TaskbarAlignmentCallback onTaskbarAlignment_{};
    TaskbarStyleCallback onTaskbarStyle_{};
    TopBarCallback onTopBar_{};

    void renderSidebar(Surface& s, const ThemePalette& palette);
    void renderSystemPage(Surface& s, const ThemePalette& palette, Rect contentR);
    void renderPersonalizationPage(Surface& s, const ThemePalette& palette, Rect contentR);
    void renderTaskbarPage(Surface& s, const ThemePalette& palette, Rect contentR);
    void renderNetworkPage(Surface& s, const ThemePalette& palette, Rect contentR);
    void renderAboutPage(Surface& s, const ThemePalette& palette, Rect contentR);
};

} // namespace surshell
