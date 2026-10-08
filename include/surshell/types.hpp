// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/types.hpp)
//
// Named in tribute to Dave Cutler's historic Windows NT 4.0 "SUR" (Shell Update Release).
// Conforms strictly to Microsoft's MIT-licensed win32metadata specifications.
// ============================================================================

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <algorithm>
#include <vector>
#include <memory>
#include <cmath>

namespace surshell {

// ============================================================================
// 1. Color Representation & Blending (RGBA 32-bit)
// ============================================================================

struct Color {
    uint8_t r{0};
    uint8_t g{0};
    uint8_t b{0};
    uint8_t a{255};

    [[nodiscard]] constexpr uint32_t toRgba() const noexcept {
        return (static_cast<uint32_t>(a) << 24) |
               (static_cast<uint32_t>(r) << 16) |
               (static_cast<uint32_t>(g) << 8)  |
               static_cast<uint32_t>(b);
    }

    [[nodiscard]] constexpr uint32_t toBgra() const noexcept {
        return (static_cast<uint32_t>(a) << 24) |
               (static_cast<uint32_t>(r) << 16) |
               (static_cast<uint32_t>(g) << 8)  |
               static_cast<uint32_t>(b);
    }

    [[nodiscard]] static constexpr Color fromRgb(uint8_t r, uint8_t g, uint8_t b) noexcept {
        return Color{r, g, b, 255};
    }

    [[nodiscard]] static constexpr Color fromRgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
        return Color{r, g, b, a};
    }

    [[nodiscard]] static constexpr Color fromHex(uint32_t hex) noexcept {
        return Color{
            static_cast<uint8_t>((hex >> 16) & 0xFF),
            static_cast<uint8_t>((hex >> 8) & 0xFF),
            static_cast<uint8_t>(hex & 0xFF),
            255
        };
    }

    [[nodiscard]] Color withAlpha(uint8_t newAlpha) const noexcept {
        return Color{r, g, b, newAlpha};
    }

    // Alpha blending: over operator (this over dst)
    [[nodiscard]] Color blendOver(const Color& dst) const noexcept {
        if (a == 255) return *this;
        if (a == 0) return dst;

        const float alphaTop = static_cast<float>(a) / 255.0f;
        const float alphaBot = (static_cast<float>(dst.a) / 255.0f) * (1.0f - alphaTop);
        const float outAlpha = alphaTop + alphaBot;

        if (outAlpha <= 0.0001f) return Color{0, 0, 0, 0};

        const auto outR = static_cast<uint8_t>(std::clamp((r * alphaTop + dst.r * alphaBot) / outAlpha, 0.0f, 255.0f));
        const auto outG = static_cast<uint8_t>(std::clamp((g * alphaTop + dst.g * alphaBot) / outAlpha, 0.0f, 255.0f));
        const auto outB = static_cast<uint8_t>(std::clamp((b * alphaTop + dst.b * alphaBot) / outAlpha, 0.0f, 255.0f));
        const auto outA = static_cast<uint8_t>(std::clamp(outAlpha * 255.0f, 0.0f, 255.0f));

        return Color{outR, outG, outB, outA};
    }

    // Linear interpolation between two colors
    [[nodiscard]] static Color lerp(const Color& c1, const Color& c2, float t) noexcept {
        t = std::clamp(t, 0.0f, 1.0f);
        return Color{
            static_cast<uint8_t>(c1.r + (c2.r - c1.r) * t),
            static_cast<uint8_t>(c1.g + (c2.g - c1.g) * t),
            static_cast<uint8_t>(c1.b + (c2.b - c1.b) * t),
            static_cast<uint8_t>(c1.a + (c2.a - c1.a) * t)
        };
    }

    constexpr bool operator==(const Color& other) const noexcept = default;
};

// ============================================================================
// 2. 2D Geometry Primitives
// ============================================================================

struct Point {
    int32_t x{0};
    int32_t y{0};

    constexpr bool operator==(const Point& other) const noexcept = default;
};

struct Size {
    int32_t width{0};
    int32_t height{0};

    constexpr bool operator==(const Size& other) const noexcept = default;
};

struct Rect {
    int32_t x{0};
    int32_t y{0};
    int32_t width{0};
    int32_t height{0};

    [[nodiscard]] constexpr int32_t left() const noexcept { return x; }
    [[nodiscard]] constexpr int32_t top() const noexcept { return y; }
    [[nodiscard]] constexpr int32_t right() const noexcept { return x + width; }
    [[nodiscard]] constexpr int32_t bottom() const noexcept { return y + height; }
    [[nodiscard]] constexpr int32_t centerX() const noexcept { return x + width / 2; }
    [[nodiscard]] constexpr int32_t centerY() const noexcept { return y + height / 2; }
    [[nodiscard]] constexpr Point center() const noexcept { return Point{x + width / 2, y + height / 2}; }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return width <= 0 || height <= 0;
    }

    [[nodiscard]] constexpr bool contains(Point pt) const noexcept {
        return pt.x >= x && pt.x < (x + width) &&
               pt.y >= y && pt.y < (y + height);
    }

    [[nodiscard]] constexpr bool contains(int32_t px, int32_t py) const noexcept {
        return px >= x && px < (x + width) &&
               py >= y && py < (y + height);
    }

    [[nodiscard]] constexpr bool intersects(const Rect& other) const noexcept {
        return x < other.right() && right() > other.x &&
               y < other.bottom() && bottom() > other.y;
    }

    [[nodiscard]] Rect intersectWith(const Rect& other) const noexcept {
        const int32_t nx = std::max(x, other.x);
        const int32_t ny = std::max(y, other.y);
        const int32_t nr = std::min(right(), other.right());
        const int32_t nb = std::min(bottom(), other.bottom());
        if (nr > nx && nb > ny) {
            return Rect{nx, ny, nr - nx, nb - ny};
        }
        return Rect{0, 0, 0, 0};
    }

    [[nodiscard]] Rect inflate(int32_t dx, int32_t dy) const noexcept {
        return Rect{x - dx, y - dy, width + dx * 2, height + dy * 2};
    }

    [[nodiscard]] Rect deflate(int32_t dx, int32_t dy) const noexcept {
        return Rect{x + dx, y + dy, std::max(0, width - dx * 2), std::max(0, height - dy * 2)};
    }

    [[nodiscard]] Rect offset(int32_t dx, int32_t dy) const noexcept {
        return Rect{x + dx, y + dy, width, height};
    }

    constexpr bool operator==(const Rect& other) const noexcept = default;
};

// ============================================================================
// 3. Shell Enumerations & Input Enums
// ============================================================================

enum class MouseButton {
    None = 0,
    Left,
    Right,
    Middle
};

enum class HitTestResult {
    None = 0,
    Client,
    Caption,
    MinButton,
    MaxButton,
    CloseButton,
    BorderLeft,
    BorderRight,
    BorderTop,
    BorderBottom,
    BorderTopLeft,
    BorderTopRight,
    BorderBottomLeft,
    BorderBottomRight,
    TaskbarStartButton,
    TaskbarTaskItem,
    TaskbarTrayItem,
    SnapLayoutZone
};

enum class WindowState {
    Normal = 0,
    Minimized,
    Maximized,
    SnappedLeft,
    SnappedRight,
    SnappedPriorityLeft,   // 67% width
    SnappedSidebarRight,   // 33% width
    SnappedTopLeft,        // 25% top-left quadrant
    SnappedTopRight,       // 25% top-right quadrant
    SnappedBottomLeft,     // 25% bottom-left quadrant
    SnappedBottomRight     // 25% bottom-right quadrant
};

enum class SnapLayoutPreset {
    None = 0,
    Split50_50,
    SplitPriority67_33,
    Quad4Grid
};

enum class TaskbarPosition {
    Bottom = 0,
    Top,
    Left,
    Right
};

enum class TaskbarAlignment {
    Center = 0,
    Left
};

enum class TaskbarStyle {
    FloatingIsland = 0,
    EdgeToEdge
};

enum class AppCategory {
    Accessories = 0,
    SystemTools,
    Development,
    Multimedia,
    Settings,
    Utilities
};

struct ShellAppEntry {
    std::string id;
    std::string title;
    std::string subtitle;
    std::string executablePath;
    std::string arguments;
    std::string iconGlyph;
    AppCategory category{AppCategory::Accessories};
    bool pinnedToTaskbar{false};
    bool pinnedToStart{false};
};

enum class KeyCode {
    Unknown = 0,
    Enter,
    Escape,
    Backspace,
    Tab,
    Delete,
    Up,
    Down,
    Left,
    Right,
    Home,
    End,
    PageUp,
    PageDown,
    F2,
    F5,
    Num0,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    KeyA,
    KeyC,
    KeyD,
    KeyE,
    KeyF,
    KeyL,
    KeyN,
    KeyR,
    KeyS,
    KeyT,
    KeyV,
    KeyW,
    KeyX,
    KeyZ
};

struct KeyEvent {
    KeyCode key{KeyCode::Unknown};
    char charCode{0};
    bool ctrl{false};
    bool shift{false};
    bool alt{false};
};

} // namespace surshell
