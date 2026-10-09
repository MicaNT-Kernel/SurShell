// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/quick_settings.cpp)
// ============================================================================

#include "surshell/quick_settings.hpp"
#include "surshell/icons.hpp"
#include "surshell/kernel_bridge.hpp"
#include <algorithm>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace surshell {

QuickSettingsFlyout::QuickSettingsFlyout() {
    toggles_ = {
        QuickToggle{.id = "network", .label = "Wi-Fi & Ethernet", .statusText = "1 Gbps Online", .iconGlyph = "[NET]", .iconId = IconId::NetworkOnline, .enabled = true},
        QuickToggle{.id = "sentinel", .label = "SentinelSec", .statusText = "Shield Guard", .iconGlyph = "[S]", .iconId = IconId::SentinelSec, .enabled = true},
        QuickToggle{.id = "nightlight", .label = "Night Light", .statusText = "Warm 4500K", .iconGlyph = "[N]", .iconId = IconId::Clock, .enabled = false},
        QuickToggle{.id = "focus", .label = "Focus Session", .statusText = "Quiet Hours", .iconGlyph = "[F]", .iconId = IconId::Clock, .enabled = false},
        QuickToggle{.id = "eco", .label = "Daytona Eco", .statusText = "Balanced", .iconGlyph = "[E]", .iconId = IconId::BatteryCharging, .enabled = false},
        QuickToggle{.id = "prism_audio", .label = "Prism Spatial", .statusText = "3D HRTF", .iconGlyph = "[A]", .iconId = IconId::VolumeHigh, .enabled = true}
    };
}

void QuickSettingsFlyout::updateLayout(int32_t screenWidth, int32_t screenHeight, int32_t taskbarHeight) {
    constexpr int32_t cardWidth = 360;
    constexpr int32_t cardHeight = 440;
    constexpr int32_t margin = 16;

    bounds_ = Rect{
        screenWidth - cardWidth - margin,
        screenHeight - cardHeight - (taskbarHeight + margin),
        cardWidth,
        cardHeight
    };

    // Layout toggles (2 columns x 3 rows)
    const int32_t startX = bounds_.x + 16;
    int32_t curY = bounds_.y + 70;
    constexpr int32_t colWidth = 158;
    constexpr int32_t rowHeight = 48;
    constexpr int32_t colSpacing = 12;
    constexpr int32_t rowSpacing = 8;

    for (size_t i = 0; i < toggles_.size(); ++i) {
        const int32_t col = static_cast<int32_t>(i % 2);
        const int32_t row = static_cast<int32_t>(i / 2);
        toggles_[i].bounds = Rect{
            startX + col * (colWidth + colSpacing),
            curY + row * (rowHeight + rowSpacing),
            colWidth,
            rowHeight
        };
    }

    curY += 3 * (rowHeight + rowSpacing) + 16;

    // Sliders
    volumeSliderBounds_ = Rect{startX, curY, cardWidth - 32, 28};
    curY += 36;
    brightnessSliderBounds_ = Rect{startX, curY, cardWidth - 32, 28};

    // Footer Settings Button
    settingsButtonBounds_ = Rect{bounds_.right() - 110, bounds_.bottom() - 38, 94, 26};
}

bool QuickSettingsFlyout::isToggleEnabled(std::string_view id) const {
    for (const auto& t : toggles_) {
        if (t.id == id) return t.enabled;
    }
    return false;
}

void QuickSettingsFlyout::setToggleEnabled(std::string_view id, bool enabled) {
    for (auto& t : toggles_) {
        if (t.id == id) {
            t.enabled = enabled;
            if (onToggleChanged_) onToggleChanged_(id, enabled);
            return;
        }
    }
}

bool QuickSettingsFlyout::onMouseDown(Point pt, MouseButton btn) {
    if (!isOpen_ || btn != MouseButton::Left) return false;
    if (!bounds_.contains(pt)) {
        close();
        return false;
    }

    // Check toggles
    for (auto& t : toggles_) {
        if (t.bounds.contains(pt)) {
            t.enabled = !t.enabled;
            if (onToggleChanged_) onToggleChanged_(t.id, t.enabled);
            return true;
        }
    }

    // Check Volume Slider
    if (volumeSliderBounds_.contains(pt)) {
        draggingVolume_ = true;
        const float ratio = static_cast<float>(pt.x - volumeSliderBounds_.x) / static_cast<float>(volumeSliderBounds_.width);
        setVolume(static_cast<int32_t>(ratio * 100.0f));
        if (onVolumeChanged_) onVolumeChanged_(volume_);
        return true;
    }

    // Check Brightness Slider
    if (brightnessSliderBounds_.contains(pt)) {
        draggingBrightness_ = true;
        const float ratio = static_cast<float>(pt.x - brightnessSliderBounds_.x) / static_cast<float>(brightnessSliderBounds_.width);
        setBrightness(static_cast<int32_t>(ratio * 100.0f));
        if (onBrightnessChanged_) onBrightnessChanged_(brightness_);
        return true;
    }

    // Check Settings button
    if (settingsButtonBounds_.contains(pt)) {
        close();
        if (onSettingsClicked_) onSettingsClicked_();
        return true;
    }

    return true; // Click inside flyout consumed
}

bool QuickSettingsFlyout::onMouseMove(Point pt) {
    if (!isOpen_) return false;

    if (draggingVolume_) {
        const float ratio = static_cast<float>(pt.x - volumeSliderBounds_.x) / static_cast<float>(volumeSliderBounds_.width);
        setVolume(static_cast<int32_t>(ratio * 100.0f));
        if (onVolumeChanged_) onVolumeChanged_(volume_);
        return true;
    }

    if (draggingBrightness_) {
        const float ratio = static_cast<float>(pt.x - brightnessSliderBounds_.x) / static_cast<float>(brightnessSliderBounds_.width);
        setBrightness(static_cast<int32_t>(ratio * 100.0f));
        if (onBrightnessChanged_) onBrightnessChanged_(brightness_);
        return true;
    }

    return bounds_.contains(pt);
}

void QuickSettingsFlyout::onMouseUp(Point, MouseButton btn) {
    if (btn == MouseButton::Left) {
        draggingVolume_ = false;
        draggingBrightness_ = false;
    }
}

void QuickSettingsFlyout::render(Surface& surface, const ThemePalette& theme) {
    if (!isOpen_) return;

    // 1. Soft Ambient Drop Shadow
    surface.drawDropShadow(bounds_, 16, 0.45f);

    // 2. Translucent Mica Acrylic Sub-Surface Blur & Card Backdrop
    surface.applyAcrylicTint(bounds_, theme.startMenuBg, 8);
    surface.drawRoundedRect(bounds_, 14, theme.startMenuBorder, false);

    // 3. Header: Title, Telemetry & Status
    surface.drawString(bounds_.x + 18, bounds_.y + 16, "Quick Controls", theme.textPrimary, 1);

    std::string subTitle = "MicaNT Sentinel & Mesh Active";
    std::string powerStr = "100% AC";
    IconId battIcon = IconId::BatteryCharging;

#if defined(_WIN32)
    char hostBuf[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD hostLen = sizeof(hostBuf);
    if (GetComputerNameA(hostBuf, &hostLen) && hostLen > 0) {
        subTitle = std::string(hostBuf) + " | Sovereign Node";
    }

    SYSTEM_POWER_STATUS sps{};
    if (GetSystemPowerStatus(&sps)) {
        if (sps.BatteryFlag == 128 || sps.BatteryLifePercent == 255) {
            powerStr = "AC Power";
            battIcon = IconId::BatteryCharging;
        } else {
            powerStr = std::to_string(static_cast<int>(sps.BatteryLifePercent)) + "% " +
                       (sps.ACLineStatus == 1 ? "AC" : "Batt");
            battIcon = IconId::BatteryCharging;
        }
    }
#endif

    surface.drawString(bounds_.x + 18, bounds_.y + 32, subTitle, theme.accentColor, 1);
    IconRenderer::draw(surface, battIcon, Rect{bounds_.right() - 104, bounds_.y + 15, 14, 14}, theme.accentSecondary);
    surface.drawString(bounds_.right() - 86, bounds_.y + 16, powerStr, theme.accentSecondary, 1);

    surface.fillRect(Rect{bounds_.x + 16, bounds_.y + 52, bounds_.width - 32, 1}, Color::fromRgba(255, 255, 255, 25));

    // 4. Quick Toggles Grid
    for (const auto& t : toggles_) {
        const Color bg = t.enabled ? Color::fromRgba(0, 180, 240, 210) : theme.startCardBg;
        const Color border = t.enabled ? theme.accentColor : theme.startCardBorder;
        const Color textCol = t.enabled ? Color::fromHex(0x06090F) : theme.textPrimary;
        const Color subCol = t.enabled ? Color::fromHex(0x102030) : theme.textSecondary;

        surface.drawRoundedRect(t.bounds, 8, bg, true);
        surface.drawRoundedRect(t.bounds, 8, border, false);

        // Procedural vector icon
        IconRenderer::draw(surface, t.iconId, Rect{t.bounds.x + 10, t.bounds.y + (t.bounds.height - 18) / 2, 18, 18}, textCol);

        // Label and status text
        surface.drawString(t.bounds.x + 36, t.bounds.y + 10, t.label, textCol, 1);
        surface.drawString(t.bounds.x + 36, t.bounds.y + 24, t.statusText, subCol, 1);
    }

    // 5. Volume Slider
    const IconId volIcon = (volume_ == 0) ? IconId::VolumeMute : IconId::VolumeHigh;
    IconRenderer::draw(surface, volIcon, Rect{volumeSliderBounds_.x, volumeSliderBounds_.y - 14, 14, 14}, theme.textSecondary);
    surface.drawString(volumeSliderBounds_.x + 18, volumeSliderBounds_.y - 12, "Volume (" + std::to_string(volume_) + "%)", theme.textSecondary, 1);
    surface.drawRoundedRect(volumeSliderBounds_, 6, Color::fromRgba(35, 50, 75, 220), true);
    const int32_t volFillWidth = static_cast<int32_t>((volumeSliderBounds_.width * volume_) / 100);
    if (volFillWidth > 0) {
        surface.drawRoundedRect(Rect{volumeSliderBounds_.x, volumeSliderBounds_.y, volFillWidth, volumeSliderBounds_.height}, 6, theme.accentColor, true);
    }
    // Slider handle
    const int32_t volThumbX = volumeSliderBounds_.x + volFillWidth;
    surface.drawRoundedRect(Rect{volThumbX - 4, volumeSliderBounds_.y - 2, 8, volumeSliderBounds_.height + 4}, 4, theme.textPrimary, true);

    // 6. Brightness Slider
    IconRenderer::draw(surface, IconId::StartPrism, Rect{brightnessSliderBounds_.x, brightnessSliderBounds_.y - 14, 14, 14}, Color::fromHex(0xFFB900));
    surface.drawString(brightnessSliderBounds_.x + 18, brightnessSliderBounds_.y - 12, "Brightness (" + std::to_string(brightness_) + "%)", theme.textSecondary, 1);
    surface.drawRoundedRect(brightnessSliderBounds_, 6, Color::fromRgba(35, 50, 75, 220), true);
    const int32_t brightFillWidth = static_cast<int32_t>((brightnessSliderBounds_.width * brightness_) / 100);
    if (brightFillWidth > 0) {
        surface.drawRoundedRect(Rect{brightnessSliderBounds_.x, brightnessSliderBounds_.y, brightFillWidth, brightnessSliderBounds_.height}, 6, Color::fromHex(0xFFB900), true);
    }
    // Slider handle
    const int32_t brightThumbX = brightnessSliderBounds_.x + brightFillWidth;
    surface.drawRoundedRect(Rect{brightThumbX - 4, brightnessSliderBounds_.y - 2, 8, brightnessSliderBounds_.height + 4}, 4, theme.textPrimary, true);

    // 7. Footer Divider & Action Controls
    surface.fillRect(Rect{bounds_.x + 16, bounds_.bottom() - 50, bounds_.width - 32, 1}, Color::fromRgba(255, 255, 255, 25));
    const std::string hostName = KernelBridge::queryComputerName();
    surface.drawString(bounds_.x + 18, bounds_.bottom() - 32, "Host: " + hostName, theme.textSecondary, 1);

    // Settings Button
    surface.drawRoundedRect(settingsButtonBounds_, 6, theme.startCardBg, true);
    surface.drawRoundedRect(settingsButtonBounds_, 6, theme.startCardBorder, false);
    IconRenderer::draw(surface, IconId::Settings, Rect{settingsButtonBounds_.x + 8, settingsButtonBounds_.y + 5, 16, 16}, theme.textPrimary);
    surface.drawString(settingsButtonBounds_.x + 28, settingsButtonBounds_.y + 8, "Settings", theme.textPrimary, 1);
}

} // namespace surshell
