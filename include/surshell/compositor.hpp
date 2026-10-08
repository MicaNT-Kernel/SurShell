// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/compositor.hpp)
//
// High-performance 2D software compositor, sub-pixel rasterizer, surface buffer,
// and DirectComposition-compatible blitting pipeline.
// ============================================================================

#pragma once

#include "types.hpp"
#include <span>
#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <algorithm>
#include <cmath>

namespace surshell {

class Surface {
public:
    Surface() = default;
    Surface(uint32_t width, uint32_t height, Color fill = Color{0, 0, 0, 255});

    void resize(uint32_t width, uint32_t height, Color fill = Color{0, 0, 0, 255});

    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }
    [[nodiscard]] std::span<const uint32_t> pixels() const noexcept { return pixels_; }
    [[nodiscard]] std::span<uint32_t> pixels() noexcept { return pixels_; }

    void clear(Color color) noexcept;
    void putPixel(int32_t x, int32_t y, Color color) noexcept;
    [[nodiscard]] Color getPixel(int32_t x, int32_t y) const noexcept;

    void fillRect(Rect rect, Color color) noexcept;
    void drawRect(Rect rect, Color color) noexcept;
    void drawVerticalGradient(Rect rect, Color top, Color bottom) noexcept;
    void drawHorizontalGradient(Rect rect, Color left, Color right) noexcept;

    void drawRoundedRect(Rect rect, int32_t radius, Color color, bool filled = true) noexcept;
    void drawDropShadow(Rect rect, int32_t radius, float opacity) noexcept;

    void blit(const Surface& src, Rect srcRect, Point dstPos, uint8_t alpha = 255) noexcept;
    void drawString(int32_t x, int32_t y, std::string_view text, Color color, int32_t scale = 1) noexcept;

    // Export surface to standard 32-bit BMP file
    bool exportBmp(const std::string& filepath) const;

private:
    uint32_t width_{0};
    uint32_t height_{0};
    std::vector<uint32_t> pixels_{};

    void drawChar(int32_t x, int32_t y, char c, Color color, int32_t scale) noexcept;
};

} // namespace surshell
