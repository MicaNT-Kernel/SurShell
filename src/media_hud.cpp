// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/media_hud.cpp)
// ============================================================================

#include "surshell/media_hud.hpp"
#include <algorithm>

namespace surshell {

MediaHud::MediaHud() = default;

void MediaHud::showVolume(int32_t volumePercent, bool isMuted) {
    volume_ = std::clamp(volumePercent, 0, 100);
    isMuted_ = isMuted;
    isVisible_ = true;
    ticksRemaining_ = 180; // ~3 seconds at 60fps
}

void MediaHud::showMedia(std::string trackTitle, std::string artist) {
    trackTitle_ = std::move(trackTitle);
    if (!artist.empty()) artist_ = std::move(artist);
    isVisible_ = true;
    ticksRemaining_ = 240; // ~4 seconds
}

void MediaHud::togglePlayPause() {
    isPlaying_ = !isPlaying_;
    ticksRemaining_ = 180;
    if (onPlayPause_) onPlayPause_();
}

void MediaHud::nextTrack() {
    ticksRemaining_ = 180;
    if (onNext_) onNext_();
}

void MediaHud::previousTrack() {
    ticksRemaining_ = 180;
    if (onPrev_) onPrev_();
}

void MediaHud::tick() {
    if (ticksRemaining_ > 0) {
        ticksRemaining_--;
        if (ticksRemaining_ == 0) {
            isVisible_ = false;
        }
    }
}

bool MediaHud::onMouseDown(Point pt, MouseButton btn) {
    if (!isVisible_ || btn != MouseButton::Left) return false;

    if (playPauseBtn_.contains(pt)) {
        togglePlayPause();
        return true;
    }
    if (nextBtn_.contains(pt)) {
        nextTrack();
        return true;
    }
    if (prevBtn_.contains(pt)) {
        previousTrack();
        return true;
    }
    if (volumeBarBounds_.contains(pt)) {
        const float t = static_cast<float>(pt.x - volumeBarBounds_.x) / static_cast<float>(volumeBarBounds_.width);
        volume_ = std::clamp(static_cast<int32_t>(t * 100.0f), 0, 100);
        isMuted_ = (volume_ == 0);
        ticksRemaining_ = 180;
        return true;
    }

    if (bounds_.contains(pt)) {
        ticksRemaining_ = 180; // keep visible on click
        return true;
    }

    return false;
}

bool MediaHud::onMouseMove(Point pt) {
    if (!isVisible_) return false;
    if (bounds_.contains(pt)) {
        ticksRemaining_ = 180; // keep visible while hovering
        return true;
    }
    return false;
}

void MediaHud::render(Surface& surface, int32_t screenWidth, int32_t screenHeight, int32_t taskbarHeight) const {
    if (!isVisible_) return;

    constexpr int32_t hudWidth = 360;
    constexpr int32_t hudHeight = 96;
    constexpr int32_t margin = 20;

    const int32_t x = (screenWidth - hudWidth) / 2;
    const int32_t y = screenHeight - taskbarHeight - margin - hudHeight;

    bounds_ = Rect{x, y, hudWidth, hudHeight};

    // 1. Drop shadow
    surface.drawDropShadow(bounds_, 18, 160);

    // 2. Acrylic frosted glass body & border
    surface.drawRoundedRect(bounds_, 10, Color::fromRgba(14, 20, 32, 235), true);
    surface.drawRoundedRect(bounds_, 10, Color::fromRgba(45, 65, 95, 180), false);

    // 3. Top Row: Volume & Slider
    const Rect volIconBounds{x + 16, y + 14, 18, 18};
    IconRenderer::draw(surface, isMuted_ ? IconId::VolumeMute : IconId::VolumeHigh,
                       Point{volIconBounds.x, volIconBounds.y}, 16,
                       isMuted_ ? Color::fromHex(0xFF5555) : Color::fromHex(0x00D4FF));

    const std::string volText = isMuted_ ? "Muted" : (std::to_string(volume_) + "%");
    surface.drawString(Point{x + 40, y + 16}, volText, Color::fromHex(0xD0DCF0));

    // Volume Track & Progress Bar
    const int32_t barX = x + 96;
    const int32_t barW = hudWidth - 96 - 16;
    volumeBarBounds_ = Rect{barX, y + 18, barW, 8};
    surface.drawRoundedRect(volumeBarBounds_, 4, Color::fromRgba(255, 255, 255, 25), true);

    if (!isMuted_ && volume_ > 0) {
        const int32_t fillW = std::clamp(barW * volume_ / 100, 4, barW);
        surface.drawRoundedRect(Rect{barX, y + 18, fillW, 8}, 4, Color::fromHex(0x00D4FF), true);
        // Knob head
        surface.drawRoundedRect(Rect{barX + fillW - 4, y + 16, 10, 12}, 5, Color::fromHex(0xFFFFFF), true);
    }

    // 4. Subtle divider rule
    surface.fillRect(Rect{x + 14, y + 42, hudWidth - 28, 1}, Color::fromRgba(255, 255, 255, 20));

    // 5. Bottom Row: Media Track Info & Transport
    // Track title and artist
    surface.drawString(Point{x + 16, y + 52}, trackTitle_, Color::fromHex(0xFFFFFF));
    surface.drawString(Point{x + 16, y + 72}, artist_, Color::fromHex(0x607898));

    // Transport buttons on the right
    const int32_t ctrlRight = x + hudWidth - 16;
    nextBtn_ = Rect{ctrlRight - 24, y + 56, 24, 24};
    playPauseBtn_ = Rect{ctrlRight - 24 - 32, y + 54, 28, 28};
    prevBtn_ = Rect{ctrlRight - 24 - 32 - 28, y + 56, 24, 24};

    // Prev Button [⏮]
    surface.drawRoundedRect(prevBtn_, 4, Color::fromRgba(255, 255, 255, 15), true);
    IconRenderer::draw(surface, IconId::MediaPrev, Point{prevBtn_.x + 4, prevBtn_.y + 4}, 16, Color::fromHex(0x00D4FF));

    // Play/Pause Button [⏯]
    surface.drawRoundedRect(playPauseBtn_, 14, Color::fromRgba(0, 212, 255, 40), true);
    surface.drawRoundedRect(playPauseBtn_, 14, Color::fromHex(0x00D4FF), false);
    IconRenderer::draw(surface, isPlaying_ ? IconId::MediaPause : IconId::MediaPlay,
                       Point{playPauseBtn_.x + 6, playPauseBtn_.y + 6}, 16, Color::fromHex(0x00D4FF));

    // Next Button [⏭]
    surface.drawRoundedRect(nextBtn_, 4, Color::fromRgba(255, 255, 255, 15), true);
    IconRenderer::draw(surface, IconId::MediaNext, Point{nextBtn_.x + 4, nextBtn_.y + 4}, 16, Color::fromHex(0x00D4FF));
}

} // namespace surshell
