// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/theme.hpp)
//
// Modern Mica & Acrylic visual theming engine, metric definitions, and color palette.
// ============================================================================

#pragma once

#include "types.hpp"

namespace surshell {

enum class ThemeMode {
    Dark = 0,
    Light,
    MicaMaterial,
    CarbonSlate
};

struct ThemeMetrics {
    int32_t taskbarHeight{48};
    int32_t taskbarIconSize{26};
    int32_t taskbarFloatingMargin{10};
    int32_t taskbarIslandRadius{12};
    TaskbarStyle taskbarStyle{TaskbarStyle::FloatingIsland};
    TaskbarAlignment taskbarAlignment{TaskbarAlignment::Center};
    int32_t captionHeight{32};
    int32_t windowBorderWidth{1};
    int32_t windowCornerRadius{8};
    int32_t buttonCornerRadius{6};
    int32_t startMenuWidth{520};
    int32_t startMenuHeight{580};
    int32_t startMenuFloatingMargin{12};
    int32_t desktopIconSize{48};
    int32_t desktopGridSpacingX{84};
    int32_t desktopGridSpacingY{96};
    int32_t shadowRadius{14};
    float shadowOpacity{0.50f};
};

struct ThemePalette {
    // Desktop & Background
    Color desktopBgTop{Color::fromHex(0x0E1420)};
    Color desktopBgBottom{Color::fromHex(0x06090F)};
    Color gridLineColor{Color::fromRgba(28, 38, 56, 40)};

    // Taskbar & Floating Islands
    Color taskbarBg{Color::fromRgba(14, 20, 32, 235)};
    Color taskbarBorderTop{Color::fromRgba(45, 62, 92, 180)};
    Color taskbarIslandBg{Color::fromRgba(18, 25, 40, 235)};
    Color taskbarIslandBorder{Color::fromRgba(56, 80, 120, 180)};
    Color taskbarItemBg{Color::fromRgba(24, 34, 52, 140)};
    Color taskbarItemHover{Color::fromRgba(42, 60, 92, 200)};
    Color taskbarItemActive{Color::fromRgba(52, 76, 116, 230)};
    Color taskbarItemIndicator{Color::fromHex(0x00D4FF)}; // Barrer Cyan

    // Prism Emblem & Accents
    Color prismAccent{Color::fromHex(0x00D4FF)};
    Color prismFacetDark{Color::fromHex(0x006699)};
    Color prismFacetLight{Color::fromHex(0x80EAFF)};

    // Start Prism Hub
    Color startMenuBg{Color::fromRgba(16, 22, 36, 245)};
    Color startMenuBorder{Color::fromRgba(48, 68, 104, 200)};
    Color startMenuSearchBg{Color::fromRgba(24, 34, 54, 220)};
    Color startMenuSearchBorder{Color::fromRgba(60, 84, 128, 180)};
    Color startCardBg{Color::fromRgba(25, 36, 56, 160)};
    Color startCardHover{Color::fromRgba(40, 58, 90, 220)};
    Color startCardBorder{Color::fromRgba(50, 72, 110, 140)};

    // Snap Layout Assistant HUD
    Color snapFlyoutBg{Color::fromRgba(16, 24, 38, 250)};
    Color snapFlyoutBorder{Color::fromRgba(64, 90, 136, 220)};
    Color snapZoneNormal{Color::fromRgba(32, 45, 68, 180)};
    Color snapZoneHover{Color::fromRgba(0, 212, 255, 90)};
    Color snapZoneBorder{Color::fromRgba(50, 70, 105, 180)};
    Color snapZoneBorderHover{Color::fromHex(0x00D4FF)};

    // Window Frames & Chrome
    Color windowFrameActiveBg{Color::fromRgba(18, 25, 40, 240)};
    Color windowFrameInactiveBg{Color::fromRgba(14, 19, 30, 220)};
    Color windowBorderActive{Color::fromRgba(0, 212, 255, 140)};
    Color windowBorderInactive{Color::fromRgba(40, 56, 84, 100)};
    Color windowClientBg{Color::fromRgba(10, 14, 22, 255)};

    // Caption Buttons
    Color captionBtnHover{Color::fromRgba(45, 65, 100, 180)};
    Color captionBtnPress{Color::fromRgba(60, 85, 130, 220)};
    Color closeBtnHover{Color::fromRgba(232, 17, 35, 230)};
    Color closeBtnPress{Color::fromRgba(200, 15, 30, 255)};

    // Typography & Accents
    Color textPrimary{Color::fromRgba(245, 248, 255, 255)};
    Color textSecondary{Color::fromRgba(160, 175, 200, 255)};
    Color textDisabled{Color::fromRgba(95, 105, 125, 255)};
    Color accentColor{Color::fromHex(0x00D4FF)};        // Cyan
    Color accentSecondary{Color::fromHex(0x00FF9D)};    // Emerald Neon
    Color accentTertiary{Color::fromHex(0x9C27B0)};     // DEC Purple
};

class ThemeManager {
public:
    static ThemeManager& instance() noexcept {
        static ThemeManager inst;
        return inst;
    }

    [[nodiscard]] const ThemePalette& palette() const noexcept { return palette_; }
    [[nodiscard]] const ThemeMetrics& metrics() const noexcept { return metrics_; }
    [[nodiscard]] ThemeMode mode() const noexcept { return mode_; }

    void setMode(ThemeMode mode) noexcept {
        mode_ = mode;
        applyModePreset();
    }

    void setAccentColor(Color accent) noexcept {
        palette_.accentColor = accent;
        palette_.taskbarItemIndicator = accent;
        palette_.windowBorderActive = accent.withAlpha(140);
    }

private:
    ThemeManager() {
        applyModePreset();
    }

    void applyModePreset() noexcept {
        if (mode_ == ThemeMode::Dark || mode_ == ThemeMode::CarbonSlate) {
            palette_ = ThemePalette{};
        } else if (mode_ == ThemeMode::Light) {
            palette_.desktopBgTop = Color::fromHex(0xE6ECF5);
            palette_.desktopBgBottom = Color::fromHex(0xC8D4E5);
            palette_.taskbarBg = Color::fromRgba(240, 244, 250, 235);
            palette_.taskbarBorderTop = Color::fromRgba(180, 195, 215, 180);
            palette_.taskbarItemBg = Color::fromRgba(220, 230, 245, 140);
            palette_.taskbarItemHover = Color::fromRgba(205, 220, 240, 180);
            palette_.taskbarItemActive = Color::fromRgba(190, 210, 235, 220);
            palette_.startMenuBg = Color::fromRgba(242, 246, 252, 245);
            palette_.startMenuBorder = Color::fromRgba(170, 190, 220, 200);
            palette_.windowFrameActiveBg = Color::fromRgba(240, 245, 252, 240);
            palette_.windowFrameInactiveBg = Color::fromRgba(230, 235, 242, 220);
            palette_.windowClientBg = Color::fromRgba(255, 255, 255, 255);
            palette_.textPrimary = Color::fromRgba(20, 26, 38, 255);
            palette_.textSecondary = Color::fromRgba(85, 98, 120, 255);
            palette_.textDisabled = Color::fromRgba(150, 160, 175, 255);
        }
    }

    ThemeMode mode_{ThemeMode::Dark};
    ThemeMetrics metrics_{};
    ThemePalette palette_{};
};

} // namespace surshell
