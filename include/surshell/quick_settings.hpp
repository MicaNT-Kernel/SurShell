// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/quick_settings.hpp)
//
// Modern Quick Settings & Action Center Flyout (Windows 11/2026-style ergonomics).
// Provides tactile system toggles, audio/brightness sliders, and status telemetry.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "theme.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <functional>

namespace surshell {

struct QuickToggle {
    std::string id;
    std::string label;
    std::string statusText;
    std::string iconGlyph;
    IconId iconId{IconId::Settings};
    bool enabled{false};
    Rect bounds{};
};

class QuickSettingsFlyout {
public:
    QuickSettingsFlyout();

    void updateLayout(int32_t screenWidth, int32_t screenHeight, int32_t taskbarHeight);

    [[nodiscard]] bool isOpen() const noexcept { return isOpen_; }
    void open() noexcept { isOpen_ = true; }
    void close() noexcept { isOpen_ = false; }
    void toggle() noexcept { isOpen_ = !isOpen_; }

    [[nodiscard]] Rect bounds() const noexcept { return bounds_; }

    // Toggle states
    [[nodiscard]] bool isToggleEnabled(std::string_view id) const;
    void setToggleEnabled(std::string_view id, bool enabled);

    // Sliders (0 - 100)
    [[nodiscard]] int32_t volume() const noexcept { return volume_; }
    void setVolume(int32_t vol) noexcept { volume_ = std::clamp(vol, 0, 100); }

    [[nodiscard]] int32_t brightness() const noexcept { return brightness_; }
    void setBrightness(int32_t b) noexcept { brightness_ = std::clamp(b, 0, 100); }

    // Hit Testing & Input
    bool onMouseDown(Point pt, MouseButton btn);
    bool onMouseMove(Point pt);
    void onMouseUp(Point pt, MouseButton btn);

    // Rendering
    void render(Surface& surface, const ThemePalette& theme);

    // Callbacks
    using ToggleCallback = std::function<void(std::string_view, bool)>;
    using SliderCallback = std::function<void(int32_t)>;
    using SettingsClickCallback = std::function<void()>;
    void setToggleCallback(ToggleCallback cb) { onToggleChanged_ = std::move(cb); }
    void setVolumeCallback(SliderCallback cb) { onVolumeChanged_ = std::move(cb); }
    void setBrightnessCallback(SliderCallback cb) { onBrightnessChanged_ = std::move(cb); }
    void setSettingsClickCallback(SettingsClickCallback cb) { onSettingsClicked_ = std::move(cb); }

private:
    bool isOpen_{false};
    Rect bounds_{0, 0, 360, 420};
    int32_t volume_{85};
    int32_t brightness_{100};
    bool draggingVolume_{false};
    bool draggingBrightness_{false};

    Rect volumeSliderBounds_{};
    Rect brightnessSliderBounds_{};
    Rect settingsButtonBounds_{};

    std::vector<QuickToggle> toggles_;

    ToggleCallback onToggleChanged_;
    SliderCallback onVolumeChanged_;
    SliderCallback onBrightnessChanged_;
    SettingsClickCallback onSettingsClicked_;
};

} // namespace surshell
