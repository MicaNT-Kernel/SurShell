// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/desktop.cpp)
// ============================================================================

#include "surshell/desktop.hpp"
#include "surshell/theme.hpp"

namespace surshell {

DesktopManager::DesktopManager(uint32_t screenWidth, uint32_t screenHeight)
    : screenWidth_(screenWidth), screenHeight_(screenHeight) {}

void DesktopManager::setScreenSize(uint32_t width, uint32_t height) {
    screenWidth_ = width;
    screenHeight_ = height;
    arrangeIcons();
}

void DesktopManager::addIcon(DesktopIcon icon) {
    icons_.push_back(std::move(icon));
    arrangeIcons();
}

void DesktopManager::removeIcon(std::string_view id) {
    std::erase_if(icons_, [&](const DesktopIcon& icon) { return icon.id == id; });
    arrangeIcons();
}

void DesktopManager::arrangeIcons() {
    const auto& metrics = ThemeManager::instance().metrics();
    const int32_t startX = 24;
    const int32_t startY = 32;
    const int32_t spacingX = metrics.desktopGridSpacingX;
    const int32_t spacingY = metrics.desktopGridSpacingY;
    const int32_t maxUsableY = static_cast<int32_t>(screenHeight_) - metrics.taskbarHeight - 60;

    int32_t curX = startX;
    int32_t curY = startY;

    for (auto& icon : icons_) {
        icon.bounds = Rect{curX, curY, 74, 82};
        curY += spacingY;
        if (curY > maxUsableY) {
            curY = startY;
            curX += spacingX;
        }
    }
}

void DesktopManager::recalculateIconBounds() {
    arrangeIcons();
}

std::optional<std::reference_wrapper<const DesktopIcon>> DesktopManager::getSelectedIcon() const noexcept {
    for (const auto& icon : icons_) {
        if (icon.selected) return std::cref(icon);
    }
    return std::nullopt;
}

void DesktopManager::onMouseDown(Point pt, MouseButton button) {
    if (button != MouseButton::Left) return;

    bool hitAny = false;
    for (auto& icon : icons_) {
        if (icon.bounds.contains(pt)) {
            icon.selected = true;
            hitAny = true;
        } else {
            icon.selected = false;
        }
    }

    if (!hitAny) {
        isMarqueeActive_ = true;
        marqueeStart_ = pt;
        marqueeRect_ = Rect{pt.x, pt.y, 0, 0};
    }
}

void DesktopManager::onMouseMove(Point pt) {
    if (isMarqueeActive_) {
        updateMarquee(pt);
        for (auto& icon : icons_) {
            icon.selected = marqueeRect_.intersects(icon.bounds);
        }
    }
}

void DesktopManager::onMouseUp(Point pt, MouseButton button) {
    if (button == MouseButton::Left && isMarqueeActive_) {
        updateMarquee(pt);
        isMarqueeActive_ = false;
        marqueeRect_ = Rect{0, 0, 0, 0};
    }
}

void DesktopManager::onDoubleClick(Point pt) {
    for (const auto& icon : icons_) {
        if (icon.bounds.contains(pt)) {
            if (launchCallback_) {
                launchCallback_(icon);
            }
            break;
        }
    }
}

void DesktopManager::updateMarquee(Point current) {
    const int32_t minX = std::min(marqueeStart_.x, current.x);
    const int32_t minY = std::min(marqueeStart_.y, current.y);
    const int32_t maxX = std::max(marqueeStart_.x, current.x);
    const int32_t maxY = std::max(marqueeStart_.y, current.y);
    marqueeRect_ = Rect{minX, minY, maxX - minX, maxY - minY};
}

void DesktopManager::render(Surface& surface) {
    const auto& palette = ThemeManager::instance().palette();
    const int32_t sw = static_cast<int32_t>(screenWidth_);
    const int32_t sh = static_cast<int32_t>(screenHeight_);

    // 1. Procedural Wallpaper Rendering
    switch (wallpaperStyle_) {
        case WallpaperStyle::MicaGrid: {
            surface.drawVerticalGradient(Rect{0, 0, sw, sh}, palette.desktopBgTop, palette.desktopBgBottom);
            // Subtle architectural grid lines (MicaNT Sovereign signature)
            const int32_t gridStep = 64;
            for (int32_t x = 0; x < sw; x += gridStep) {
                for (int32_t y = 0; y < sh - 40; y += 4) {
                    surface.putPixel(x, y, palette.gridLineColor);
                }
            }
            for (int32_t y = 0; y < sh - 40; y += gridStep) {
                for (int32_t x = 0; x < sw; x += 4) {
                    surface.putPixel(x, y, palette.gridLineColor);
                }
            }
            break;
        }
        case WallpaperStyle::AuroraBorealis: {
            surface.drawVerticalGradient(Rect{0, 0, sw, sh}, Color::fromHex(0x061224), Color::fromHex(0x03060C));
            // Undulating luminous auroral bands
            for (int32_t x = 0; x < sw; ++x) {
                const float fx = static_cast<float>(x) * 0.005f;
                // Primary cyan curtain
                const int32_t cy1 = static_cast<int32_t>(sh * 0.38f + 60.0f * std::sin(fx) + 30.0f * std::cos(fx * 2.2f));
                for (int32_t dy = -40; dy <= 40; ++dy) {
                    const int32_t py = cy1 + dy;
                    if (py >= 0 && py < sh) {
                        const uint8_t a = static_cast<uint8_t>(std::max(0, 45 - std::abs(dy)));
                        surface.blendPixel(x, py, Color::fromRgba(0, 212, 255, a));
                    }
                }
                // Secondary emerald ribbon
                const int32_t cy2 = static_cast<int32_t>(sh * 0.46f + 70.0f * std::sin(fx * 1.4f + 1.2f));
                for (int32_t dy = -35; dy <= 35; ++dy) {
                    const int32_t py = cy2 + dy;
                    if (py >= 0 && py < sh) {
                        const uint8_t a = static_cast<uint8_t>(std::max(0, 40 - std::abs(dy)));
                        surface.blendPixel(x, py, Color::fromRgba(0, 255, 157, a));
                    }
                }
            }
            break;
        }
        case WallpaperStyle::SovereignSlate: {
            surface.drawVerticalGradient(Rect{0, 0, sw, sh}, Color::fromHex(0x1A2332), Color::fromHex(0x0A0F16));
            // Centered subtle sovereign diamond watermark
            const int32_t cx = sw / 2;
            const int32_t cy = (sh - 40) / 2;
            for (int32_t r = 80; r <= 240; r += 80) {
                for (int32_t d = 0; d < r; ++d) {
                    surface.blendPixel(cx + d, cy - r + d, Color::fromRgba(0, 212, 255, 25));
                    surface.blendPixel(cx + r - d, cy + d, Color::fromRgba(0, 212, 255, 25));
                    surface.blendPixel(cx - d, cy + r - d, Color::fromRgba(0, 212, 255, 25));
                    surface.blendPixel(cx - r + d, cy - d, Color::fromRgba(0, 212, 255, 25));
                }
            }
            break;
        }
        case WallpaperStyle::MidnightNebula: {
            surface.drawVerticalGradient(Rect{0, 0, sw, sh}, Color::fromHex(0x1B0E28), Color::fromHex(0x06030A));
            // Star dust particles
            for (int32_t i = 0; i < 400; ++i) {
                const int32_t sx = (i * 997 + 101) % sw;
                const int32_t sy = (i * 701 + 233) % (sh - 40);
                const uint8_t alpha = static_cast<uint8_t>(60 + (i % 180));
                surface.blendPixel(sx, sy, Color::fromRgba(200, 220, 255, alpha));
                if (i % 8 == 0) {
                    surface.blendPixel(sx + 1, sy, Color::fromRgba(0, 212, 255, 80));
                    surface.blendPixel(sx, sy + 1, Color::fromRgba(0, 212, 255, 80));
                }
            }
            break;
        }
    }

    // 2. Render Desktop Icons
    for (const auto& icon : icons_) {
        // Selection highlight box
        if (icon.selected) {
            surface.drawRoundedRect(icon.bounds, 6, Color::fromRgba(0, 212, 255, 45), true);
            surface.drawRoundedRect(icon.bounds, 6, Color::fromRgba(0, 212, 255, 160), false);
        }

        // Icon Graphic Container
        Rect iconBox{icon.bounds.x + (icon.bounds.width - 40) / 2, icon.bounds.y + 4, 40, 40};
        surface.drawRoundedRect(iconBox, 8, Color::fromRgba(25, 35, 55, 230), true);
        surface.drawRoundedRect(iconBox, 8, Color::fromRgba(60, 85, 125, 180), false);

        // Procedural vector icon from Sovereign IconPack
        const Rect innerIcon{iconBox.x + 4, iconBox.y + 4, 32, 32};
        IconId actualId = icon.iconId;
        if (icon.id == "this_pc") actualId = IconId::ThisPC;
        else if (icon.id == "explorer") actualId = IconId::FileExplorer;
        else if (icon.id == "cmd") actualId = IconId::Terminal;
        else if (icon.id == "sentinel") actualId = IconId::SentinelSec;
        else if (icon.id == "settings") actualId = IconId::Settings;
        else if (icon.id == "network") actualId = IconId::NetworkEthernet;
        else if (icon.id == "taskmgr") actualId = IconId::TaskManager;
        else if (icon.id == "calc") actualId = IconId::Calculator;

        IconRenderer::draw(surface, actualId, innerIcon);

        // Icon Label
        int32_t textX = icon.bounds.x + 4;
        int32_t textY = icon.bounds.y + 50;
        std::string displayLabel = icon.label;
        if (displayLabel.size() > 9) {
            displayLabel = displayLabel.substr(0, 8) + "..";
        }
        surface.drawString(textX, textY, displayLabel, palette.textPrimary, 1);
    }

    // 4. Marquee Selection Box
    if (isMarqueeActive_ && !marqueeRect_.empty()) {
        surface.fillRect(marqueeRect_, Color::fromRgba(0, 212, 255, 30));
        surface.drawRect(marqueeRect_, Color::fromRgba(0, 212, 255, 200));
    }
}

} // namespace surshell
