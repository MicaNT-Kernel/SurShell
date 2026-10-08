// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/media_hud.hpp)
//
// Modern Audio & Media Playback HUD (On-Screen Display OSD).
// Centered floating acrylic pill displaying real-time volume levels,
// mute status, active track title, and media transport controls.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "icons.hpp"
#include <string>
#include <functional>

namespace surshell {

class MediaHud {
public:
    MediaHud();

    void showVolume(int32_t volumePercent, bool isMuted = false);
    void showMedia(std::string trackTitle, std::string artist = "");
    void hide() noexcept { isVisible_ = false; ticksRemaining_ = 0; }

    void togglePlayPause();
    void nextTrack();
    void previousTrack();

    void tick(); // decrements auto-dismiss timer

    [[nodiscard]] bool isVisible() const noexcept { return isVisible_; }
    [[nodiscard]] int32_t volume() const noexcept { return volume_; }
    [[nodiscard]] bool isMuted() const noexcept { return isMuted_; }
    [[nodiscard]] bool isPlaying() const noexcept { return isPlaying_; }
    [[nodiscard]] const std::string& trackTitle() const noexcept { return trackTitle_; }
    [[nodiscard]] Rect bounds() const noexcept { return bounds_; }

    // Hit Testing & Input
    bool onMouseDown(Point pt, MouseButton btn);
    bool onMouseMove(Point pt);

    // Rendering
    void render(Surface& surface, int32_t screenWidth, int32_t screenHeight, int32_t taskbarHeight) const;

    // Callbacks
    using MediaCallback = std::function<void()>;
    void setPlayPauseCallback(MediaCallback cb) { onPlayPause_ = std::move(cb); }
    void setNextCallback(MediaCallback cb) { onNext_ = std::move(cb); }
    void setPrevCallback(MediaCallback cb) { onPrev_ = std::move(cb); }

private:
    bool isVisible_{false};
    int32_t ticksRemaining_{0};
    int32_t volume_{80};
    bool isMuted_{false};
    bool isPlaying_{true};
    std::string trackTitle_{"Dave Cutler - Symphony in C++23"};
    std::string artist_{"MicaNT Sovereign Audio"};

    mutable Rect bounds_{};
    mutable Rect playPauseBtn_{};
    mutable Rect nextBtn_{};
    mutable Rect prevBtn_{};
    mutable Rect volumeBarBounds_{};

    MediaCallback onPlayPause_{};
    MediaCallback onNext_{};
    MediaCallback onPrev_{};
};

} // namespace surshell
