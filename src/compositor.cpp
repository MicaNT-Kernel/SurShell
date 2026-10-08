// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/compositor.cpp)
// ============================================================================

#include "surshell/compositor.hpp"
#include <cstring>
#include <fstream>
#include <iostream>

namespace surshell {

namespace {

// Standard embedded 8x8 font glyphs (ASCII 32 to 126)
// Clean-room authored proportional/monospace 8x8 dot matrix font.
constexpr uint8_t FONT_8X8[95][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 32 ' '
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // 33 '!'
    {0x66, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00}, // 34 '"'
    {0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00}, // 35 '#'
    {0x18, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x18, 0x00}, // 36 '$'
    {0x00, 0x66, 0xA6, 0xD8, 0x1B, 0x65, 0x66, 0x00}, // 37 '%'
    {0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00}, // 38 '&'
    {0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00}, // 39 '''
    {0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00}, // 40 '('
    {0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00}, // 41 ')'
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // 42 '*'
    {0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00}, // 43 '+'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30}, // 44 ','
    {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00}, // 45 '-'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, // 46 '.'
    {0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00}, // 47 '/'
    {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00}, // 48 '0'
    {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00}, // 49 '1'
    {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x7E, 0x00}, // 50 '2'
    {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00}, // 51 '3'
    {0x0C, 0x1C, 0x34, 0x64, 0x7E, 0x04, 0x04, 0x00}, // 52 '4'
    {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00}, // 53 '5'
    {0x3C, 0x66, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00}, // 54 '6'
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00}, // 55 '7'
    {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00}, // 56 '8'
    {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C, 0x00}, // 57 '9'
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00}, // 58 ':'
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30, 0x00}, // 59 ';'
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // 60 '<'
    {0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00}, // 61 '='
    {0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00}, // 62 '>'
    {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x00, 0x18, 0x00}, // 63 '?'
    {0x3C, 0x42, 0x99, 0xA5, 0xA5, 0x99, 0x42, 0x3C}, // 64 '@'
    {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00}, // 65 'A'
    {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00}, // 66 'B'
    {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00}, // 67 'C'
    {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00}, // 68 'D'
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00}, // 69 'E'
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00}, // 70 'F'
    {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3E, 0x00}, // 71 'G'
    {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00}, // 72 'H'
    {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // 73 'I'
    {0x0E, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00}, // 74 'J'
    {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00}, // 75 'K'
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00}, // 76 'L'
    {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00}, // 77 'M'
    {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00}, // 78 'N'
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // 79 'O'
    {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00}, // 80 'P'
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x6E, 0x3C, 0x06}, // 81 'Q'
    {0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x00}, // 82 'R'
    {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00}, // 83 'S'
    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 84 'T'
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // 85 'U'
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00}, // 86 'V'
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // 87 'W'
    {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00}, // 88 'X'
    {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00}, // 89 'Y'
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00}, // 90 'Z'
    {0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00}, // 91 '['
    {0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00}, // 92 '\'
    {0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00}, // 93 ']'
    {0x18, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00}, // 94 '^'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}, // 95 '_'
    {0x30, 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00}, // 96 '`'
    {0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3B, 0x00}, // 97 'a'
    {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00}, // 98 'b'
    {0x00, 0x00, 0x3C, 0x66, 0x60, 0x66, 0x3C, 0x00}, // 99 'c'
    {0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00}, // 100 'd'
    {0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00}, // 101 'e'
    {0x1C, 0x30, 0x7C, 0x30, 0x30, 0x30, 0x30, 0x00}, // 102 'f'
    {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x3C}, // 103 'g'
    {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00}, // 104 'h'
    {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00}, // 105 'i'
    {0x06, 0x00, 0x0E, 0x06, 0x06, 0x66, 0x3C, 0x00}, // 106 'j'
    {0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00}, // 107 'k'
    {0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // 108 'l'
    {0x00, 0x00, 0x66, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, // 109 'm'
    {0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00}, // 110 'n'
    {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00}, // 111 'o'
    {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60}, // 112 'p'
    {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06}, // 113 'q'
    {0x00, 0x00, 0x7C, 0x66, 0x60, 0x60, 0x60, 0x00}, // 114 'r'
    {0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00}, // 115 's'
    {0x18, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x0E, 0x00}, // 116 't'
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3B, 0x00}, // 117 'u'
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00}, // 118 'v'
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x3E, 0x36, 0x00}, // 119 'w'
    {0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00}, // 120 'x'
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x3C}, // 121 'y'
    {0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00}, // 122 'z'
    {0x0E, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0E, 0x00}, // 123 '{'
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 124 '|'
    {0x70, 0x18, 0x18, 0x0E, 0x18, 0x18, 0x70, 0x00}, // 125 '}'
    {0x36, 0x5C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 126 '~'
};

} // anonymous namespace

Surface::Surface(uint32_t width, uint32_t height, Color fill)
    : width_(width), height_(height), pixels_(width * height, fill.toBgra()) {}

void Surface::resize(uint32_t width, uint32_t height, Color fill) {
    width_ = width;
    height_ = height;
    pixels_.assign(width * height, fill.toBgra());
}

void Surface::clear(Color color) noexcept {
    const uint32_t val = color.toBgra();
    std::fill(pixels_.begin(), pixels_.end(), val);
}

void Surface::putPixel(int32_t x, int32_t y, Color color) noexcept {
    if (x < 0 || x >= static_cast<int32_t>(width_) ||
        y < 0 || y >= static_cast<int32_t>(height_)) {
        return;
    }
    const size_t idx = static_cast<size_t>(y) * width_ + static_cast<size_t>(x);
    if (color.a == 255) {
        pixels_[idx] = color.toBgra();
    } else if (color.a > 0) {
        Color dst = getPixel(x, y);
        pixels_[idx] = color.blendOver(dst).toBgra();
    }
}

Color Surface::getPixel(int32_t x, int32_t y) const noexcept {
    if (x < 0 || x >= static_cast<int32_t>(width_) ||
        y < 0 || y >= static_cast<int32_t>(height_)) {
        return Color{0, 0, 0, 0};
    }
    const uint32_t val = pixels_[static_cast<size_t>(y) * width_ + static_cast<size_t>(x)];
    return Color{
        static_cast<uint8_t>((val >> 16) & 0xFF),
        static_cast<uint8_t>((val >> 8) & 0xFF),
        static_cast<uint8_t>(val & 0xFF),
        static_cast<uint8_t>((val >> 24) & 0xFF)
    };
}

void Surface::fillRect(Rect rect, Color color) noexcept {
    const int32_t xStart = std::max(0, rect.x);
    const int32_t yStart = std::max(0, rect.y);
    const int32_t xEnd = std::min(static_cast<int32_t>(width_), rect.right());
    const int32_t yEnd = std::min(static_cast<int32_t>(height_), rect.bottom());

    if (xEnd <= xStart || yEnd <= yStart) return;

    if (color.a == 255) {
        const uint32_t val = color.toBgra();
        for (int32_t y = yStart; y < yEnd; ++y) {
            uint32_t* row = &pixels_[static_cast<size_t>(y) * width_ + static_cast<size_t>(xStart)];
            std::fill_n(row, xEnd - xStart, val);
        }
    } else if (color.a > 0) {
        for (int32_t y = yStart; y < yEnd; ++y) {
            for (int32_t x = xStart; x < xEnd; ++x) {
                putPixel(x, y, color);
            }
        }
    }
}

void Surface::drawRect(Rect rect, Color color) noexcept {
    if (rect.empty()) return;
    fillRect(Rect{rect.x, rect.y, rect.width, 1}, color);
    fillRect(Rect{rect.x, rect.bottom() - 1, rect.width, 1}, color);
    fillRect(Rect{rect.x, rect.y, 1, rect.height}, color);
    fillRect(Rect{rect.right() - 1, rect.y, 1, rect.height}, color);
}

void Surface::drawVerticalGradient(Rect rect, Color top, Color bottom) noexcept {
    const int32_t xStart = std::max(0, rect.x);
    const int32_t yStart = std::max(0, rect.y);
    const int32_t xEnd = std::min(static_cast<int32_t>(width_), rect.right());
    const int32_t yEnd = std::min(static_cast<int32_t>(height_), rect.bottom());

    if (xEnd <= xStart || yEnd <= yStart || rect.height <= 0) return;

    for (int32_t y = yStart; y < yEnd; ++y) {
        const float t = static_cast<float>(y - rect.y) / static_cast<float>(rect.height);
        const Color rowColor = Color::lerp(top, bottom, t);
        const uint32_t val = rowColor.toBgra();
        uint32_t* row = &pixels_[static_cast<size_t>(y) * width_ + static_cast<size_t>(xStart)];
        std::fill_n(row, xEnd - xStart, val);
    }
}

void Surface::drawHorizontalGradient(Rect rect, Color left, Color right) noexcept {
    const int32_t xStart = std::max(0, rect.x);
    const int32_t yStart = std::max(0, rect.y);
    const int32_t xEnd = std::min(static_cast<int32_t>(width_), rect.right());
    const int32_t yEnd = std::min(static_cast<int32_t>(height_), rect.bottom());

    if (xEnd <= xStart || yEnd <= yStart || rect.width <= 0) return;

    for (int32_t x = xStart; x < xEnd; ++x) {
        const float t = static_cast<float>(x - rect.x) / static_cast<float>(rect.width);
        const Color col = Color::lerp(left, right, t);
        for (int32_t y = yStart; y < yEnd; ++y) {
            putPixel(x, y, col);
        }
    }
}

void Surface::drawRoundedRect(Rect rect, int32_t radius, Color color, bool filled) noexcept {
    if (rect.empty()) return;
    radius = std::clamp(radius, 0, std::min(rect.width, rect.height) / 2);

    if (radius <= 0) {
        if (filled) fillRect(rect, color);
        else drawRect(rect, color);
        return;
    }

    if (filled) {
        // Center rectangle
        fillRect(Rect{rect.x, rect.y + radius, rect.width, rect.height - radius * 2}, color);
        // Top and bottom slabs
        fillRect(Rect{rect.x + radius, rect.y, rect.width - radius * 2, radius}, color);
        fillRect(Rect{rect.x + radius, rect.bottom() - radius, rect.width - radius * 2, radius}, color);

        // Four corner circles
        for (int32_t cy = 0; cy < radius; ++cy) {
            for (int32_t cx = 0; cx < radius; ++cx) {
                const int32_t dx = radius - cx - 1;
                const int32_t dy = radius - cy - 1;
                if ((dx * dx + dy * dy) <= (radius * radius)) {
                    putPixel(rect.x + cx, rect.y + cy, color);                               // Top-Left
                    putPixel(rect.right() - 1 - cx, rect.y + cy, color);                     // Top-Right
                    putPixel(rect.x + cx, rect.bottom() - 1 - cy, color);                    // Bottom-Left
                    putPixel(rect.right() - 1 - cx, rect.bottom() - 1 - cy, color);          // Bottom-Right
                }
            }
        }
    } else {
        // Outline mode
        fillRect(Rect{rect.x + radius, rect.y, rect.width - radius * 2, 1}, color);
        fillRect(Rect{rect.x + radius, rect.bottom() - 1, rect.width - radius * 2, 1}, color);
        fillRect(Rect{rect.x, rect.y + radius, 1, rect.height - radius * 2}, color);
        fillRect(Rect{rect.right() - 1, rect.y + radius, 1, rect.height - radius * 2}, color);
    }
}

void Surface::drawDropShadow(Rect rect, int32_t radius, float opacity) noexcept {
    if (radius <= 0 || opacity <= 0.0f) return;

    for (int32_t r = radius; r >= 1; --r) {
        const float falloff = std::pow(1.0f - (static_cast<float>(r) / static_cast<float>(radius)), 1.5f);
        const auto alpha = static_cast<uint8_t>(std::clamp(opacity * falloff * 255.0f, 0.0f, 255.0f));
        if (alpha == 0) continue;

        Rect shadowLayer = rect.inflate(r, r).offset(0, r / 2);
        drawRoundedRect(shadowLayer, 8 + r, Color{0, 0, 0, alpha}, false);
    }
}

void Surface::blit(const Surface& src, Rect srcRect, Point dstPos, uint8_t alpha) noexcept {
    const int32_t sxStart = std::max(0, srcRect.x);
    const int32_t syStart = std::max(0, srcRect.y);
    const int32_t sxEnd = std::min(static_cast<int32_t>(src.width_), srcRect.right());
    const int32_t syEnd = std::min(static_cast<int32_t>(src.height_), srcRect.bottom());

    if (sxEnd <= sxStart || syEnd <= syStart) return;

    for (int32_t sy = syStart; sy < syEnd; ++sy) {
        const int32_t dy = dstPos.y + (sy - srcRect.y);
        if (dy < 0 || dy >= static_cast<int32_t>(height_)) continue;

        for (int32_t sx = sxStart; sx < sxEnd; ++sx) {
            const int32_t dx = dstPos.x + (sx - srcRect.x);
            if (dx < 0 || dx >= static_cast<int32_t>(width_)) continue;

            Color srcColor = src.getPixel(sx, sy);
            if (alpha < 255) {
                srcColor.a = static_cast<uint8_t>((srcColor.a * alpha) / 255);
            }
            putPixel(dx, dy, srcColor);
        }
    }
}

void Surface::drawChar(int32_t x, int32_t y, char c, Color color, int32_t scale) noexcept {
    if (c < 32 || c > 126) c = '?';
    const uint8_t* glyph = FONT_8X8[c - 32];

    for (int32_t row = 0; row < 8; ++row) {
        uint8_t line = glyph[row];
        for (int32_t col = 0; col < 8; ++col) {
            if ((line & (0x80 >> col)) != 0) {
                if (scale == 1) {
                    putPixel(x + col, y + row, color);
                } else {
                    fillRect(Rect{x + col * scale, y + row * scale, scale, scale}, color);
                }
            }
        }
    }
}

void Surface::drawString(int32_t x, int32_t y, std::string_view text, Color color, int32_t scale) noexcept {
    int32_t curX = x;
    const int32_t charWidth = 8 * scale;

    for (char c : text) {
        if (c == '\n') {
            y += 10 * scale;
            curX = x;
            continue;
        }
        drawChar(curX, y, c, color, scale);
        curX += charWidth;
    }
}

bool Surface::exportBmp(const std::string& filepath) const {
    std::ofstream out(filepath, std::ios::binary);
    if (!out.is_open()) return false;

    // Standard 32-bit BMP header structure
    #pragma pack(push, 1)
    struct BmpHeader {
        uint16_t bfType{0x4D42};        // "BM"
        uint32_t bfSize{0};
        uint16_t bfReserved1{0};
        uint16_t bfReserved2{0};
        uint32_t bfOffBits{54};
        uint32_t biSize{40};
        int32_t  biWidth{0};
        int32_t  biHeight{0};          // Negative for top-down DIB
        uint16_t biPlanes{1};
        uint16_t biBitCount{32};
        uint32_t biCompression{0};     // BI_RGB
        uint32_t biSizeImage{0};
        int32_t  biXPelsPerMeter{2835}; // 72 DPI
        int32_t  biYPelsPerMeter{2835};
        uint32_t biClrUsed{0};
        uint32_t biClrImportant{0};
    };
    #pragma pack(pop)

    const uint32_t imageSize = width_ * height_ * 4;
    BmpHeader header;
    header.bfSize = sizeof(BmpHeader) + imageSize;
    header.biWidth = static_cast<int32_t>(width_);
    header.biHeight = -static_cast<int32_t>(height_); // Top-down
    header.biSizeImage = imageSize;

    out.write(reinterpret_cast<const char*>(&header), sizeof(header));
    out.write(reinterpret_cast<const char*>(pixels_.data()), pixels_.size() * sizeof(uint32_t));
    return true;
}

} // namespace surshell
