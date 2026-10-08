// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/desktop.cpp)
// ============================================================================

#include "surshell/desktop.hpp"
#include "surshell/theme.hpp"
#include <filesystem>
#include <algorithm>

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

void DesktopManager::discoverHostDesktop() {
    std::string userProfile = "C:\\Users\\admin";
    if (const char* envProf = std::getenv("USERPROFILE"); envProf && envProf[0] != '\0') {
        userProfile = envProf;
    }

    std::vector<std::string> desktopPaths = {
        userProfile + "\\Desktop",
        "C:\\Users\\Public\\Desktop"
    };

    std::error_code ec;
    size_t added = 0;

    std::vector<std::filesystem::directory_entry> shortcutsAndDirs;
    std::vector<std::filesystem::directory_entry> otherFiles;

    for (const auto& dp : desktopPaths) {
        if (!std::filesystem::exists(dp, ec)) continue;

        for (const auto& entry : std::filesystem::directory_iterator(dp, std::filesystem::directory_options::skip_permission_denied, ec)) {
            if (ec) break;
            const auto name = entry.path().filename().string();
            if (name.empty() || name == "desktop.ini") continue;

            const bool isDir = entry.is_directory(ec);
            const auto ext = isDir ? "" : entry.path().extension().string();

            if (ext == ".lnk" || isDir) {
                shortcutsAndDirs.push_back(entry);
            } else {
                otherFiles.push_back(entry);
            }
        }
    }

    std::vector<std::filesystem::directory_entry> allEntries = std::move(shortcutsAndDirs);
    allEntries.insert(allEntries.end(), otherFiles.begin(), otherFiles.end());

    for (const auto& entry : allEntries) {
        const auto name = entry.path().filename().string();
        const bool isDir = entry.is_directory(ec);
        const auto ext = isDir ? "" : entry.path().extension().string();

        // Clean title: strip .lnk
        std::string label = (ext == ".lnk") ? entry.path().stem().string() : name;

        // Check if already in icons_
        bool exists = false;
        for (const auto& icon : icons_) {
            if (icon.label == label || icon.executable == entry.path().string()) {
                exists = true;
                break;
            }
        }
        if (exists) continue;

        IconId iconId = IconRenderer::iconForExtension(ext, isDir);
        if (ext == ".lnk") {
            std::string lowerLabel = label;
            std::transform(lowerLabel.begin(), lowerLabel.end(), lowerLabel.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            iconId = IconRenderer::iconForAppId(lowerLabel);
        }

        icons_.push_back(DesktopIcon{
            .id = "host_dt_" + std::to_string(added++),
            .label = label,
            .executable = entry.path().string(),
            .arguments = "",
            .iconGlyph = isDir ? "[D]" : "[F]",
            .iconId = iconId,
            .gridX = 0,
            .gridY = 0,
            .bounds = Rect{},
            .selected = false
        });

        // Limit to a reasonable number so screen isn't completely flooded
        if (icons_.size() >= 24) break;
    }

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

void DesktopManager::sortByName() {
    std::sort(icons_.begin(), icons_.end(), [](const DesktopIcon& a, const DesktopIcon& b) {
        return a.label < b.label;
    });
    arrangeIcons();
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

void DesktopManager::openContextMenu(Point pt, std::string_view targetIconId) {
    contextMenu_.isOpen = true;
    contextMenu_.position = pt;
    contextMenu_.items.clear();
    contextMenu_.hoveredIndex = -1;
    contextMenu_.targetIconId = std::string(targetIconId);

    if (targetIconId.empty()) {
        // Desktop wallpaper right-click
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "view_large", .label = "Large icons", .shortcut = "", .iconId = IconId::ViewGrid});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "view_medium", .label = "Medium icons", .shortcut = "", .iconId = IconId::ViewList});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "sort_name", .label = "Sort by Name", .shortcut = "", .iconId = IconId::SortAsc});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "refresh", .label = "Refresh", .shortcut = "F5", .iconId = IconId::NavRefresh});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "sep1", .label = "", .shortcut = "", .iconId = IconId::FileGeneric, .isSeparator = true});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "new_folder", .label = "New Folder", .shortcut = "", .iconId = IconId::NewFolder});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "new_file", .label = "New Text Document", .shortcut = "", .iconId = IconId::NewFile});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "sep2", .label = "", .shortcut = "", .iconId = IconId::FileGeneric, .isSeparator = true});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "terminal", .label = "Open in Terminal", .shortcut = "", .iconId = IconId::Terminal});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "display", .label = "Display settings", .shortcut = "", .iconId = IconId::Display});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "personalize", .label = "Personalize", .shortcut = "", .iconId = IconId::Personalization});
    } else {
        // Desktop icon right-click
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "open", .label = "Open", .shortcut = "Enter", .iconId = IconId::StartPrism});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "sep1", .label = "", .shortcut = "", .iconId = IconId::FileGeneric, .isSeparator = true});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "delete", .label = "Delete", .shortcut = "Del", .iconId = IconId::Delete});
        contextMenu_.items.push_back(DesktopContextMenuItem{.id = "properties", .label = "Properties", .shortcut = "", .iconId = IconId::Properties});
    }

    constexpr int32_t itemH = 26;
    constexpr int32_t sepH = 6;
    int32_t totalH = 12;
    for (const auto& it : contextMenu_.items) {
        totalH += it.isSeparator ? sepH : itemH;
    }

    constexpr int32_t menuW = 200;
    int32_t mx = pt.x;
    int32_t my = pt.y;
    if (mx + menuW > static_cast<int32_t>(screenWidth_) - 10) {
        mx = static_cast<int32_t>(screenWidth_) - menuW - 10;
    }
    if (my + totalH > static_cast<int32_t>(screenHeight_) - 50) {
        my = static_cast<int32_t>(screenHeight_) - totalH - 50;
    }
    contextMenu_.bounds = Rect{mx, my, menuW, totalH};
}

void DesktopManager::onMouseDown(Point pt, MouseButton button) {
    if (contextMenu_.isOpen) {
        if (contextMenu_.bounds.contains(pt)) {
            for (const auto& item : contextMenu_.items) {
                if (!item.isSeparator && item.bounds.contains(pt)) {
                    const std::string actionId = item.id;
                    const std::string targetId = contextMenu_.targetIconId;
                    closeContextMenu();
                    if (actionId == "sort_name") {
                        sortByName();
                    }
                    if (contextMenuCallback_) {
                        contextMenuCallback_(actionId, targetId);
                    }
                    return;
                }
            }
        }
        closeContextMenu();
        return;
    }

    if (button == MouseButton::Right) {
        for (auto& icon : icons_) {
            if (icon.bounds.contains(pt)) {
                icon.selected = true;
                openContextMenu(pt, icon.id);
                return;
            }
        }
        openContextMenu(pt, "");
        return;
    }

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
    if (contextMenu_.isOpen) {
        contextMenu_.hoveredIndex = -1;
        for (size_t i = 0; i < contextMenu_.items.size(); ++i) {
            if (!contextMenu_.items[i].isSeparator && contextMenu_.items[i].bounds.contains(pt)) {
                contextMenu_.hoveredIndex = static_cast<int32_t>(i);
                break;
            }
        }
    }

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
    if (contextMenu_.isOpen) {
        closeContextMenu();
    }
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

        // Icon Label: Clean two-line wrapped or centered presentation
        const int32_t textY = icon.bounds.y + 48;
        std::string displayLabel = icon.label;
        if (displayLabel.size() <= 8) {
            const int32_t tw = static_cast<int32_t>(displayLabel.size()) * 8;
            const int32_t lx = icon.bounds.x + std::max(0, (icon.bounds.width - tw) / 2);
            surface.drawString(lx, textY, displayLabel, palette.textPrimary, 1);
        } else {
            std::string line1;
            std::string line2;
            const size_t sp = displayLabel.find(' ');
            if (sp != std::string::npos && sp <= 9) {
                line1 = displayLabel.substr(0, sp);
                line2 = displayLabel.substr(sp + 1);
            } else {
                line1 = displayLabel.substr(0, 8);
                line2 = displayLabel.substr(8);
            }
            if (line1.size() > 9) line1 = line1.substr(0, 8);
            if (line2.size() > 9) line2 = line2.substr(0, 7) + "..";
            const int32_t tw1 = static_cast<int32_t>(line1.size()) * 8;
            const int32_t tw2 = static_cast<int32_t>(line2.size()) * 8;
            surface.drawString(icon.bounds.x + std::max(0, (icon.bounds.width - tw1) / 2), textY, line1, palette.textPrimary, 1);
            surface.drawString(icon.bounds.x + std::max(0, (icon.bounds.width - tw2) / 2), textY + 12, line2, palette.textPrimary, 1);
        }
    }

    // 4. Marquee Selection Box
    if (isMarqueeActive_ && !marqueeRect_.empty()) {
        surface.fillRect(marqueeRect_, Color::fromRgba(0, 212, 255, 30));
        surface.drawRect(marqueeRect_, Color::fromRgba(0, 212, 255, 200));
    }
}

void DesktopManager::renderContextMenu(Surface& surface) {
    if (!contextMenu_.isOpen) return;

    surface.drawDropShadow(contextMenu_.bounds, 12, 0.45f);
    surface.applyAcrylicTint(contextMenu_.bounds, Color::fromRgba(18, 25, 40, 240), 6);
    surface.drawRoundedRect(contextMenu_.bounds, 8, Color::fromRgba(60, 85, 125, 180), false);

    int32_t itemY = contextMenu_.bounds.y + 6;
    for (size_t i = 0; i < contextMenu_.items.size(); ++i) {
        auto& mItem = contextMenu_.items[i];
        if (mItem.isSeparator) {
            mItem.bounds = Rect{contextMenu_.bounds.x + 8, itemY + 2, contextMenu_.bounds.width - 16, 1};
            surface.fillRect(mItem.bounds, Color::fromRgba(48, 68, 104, 160));
            itemY += 6;
        } else {
            mItem.bounds = Rect{contextMenu_.bounds.x + 4, itemY, contextMenu_.bounds.width - 8, 24};
            const bool isHovered = (static_cast<int32_t>(i) == contextMenu_.hoveredIndex);
            if (isHovered) {
                surface.drawRoundedRect(mItem.bounds, 4, Color::fromRgba(0, 180, 240, 60), true);
                surface.drawRoundedRect(mItem.bounds, 4, Color::fromRgba(0, 212, 255, 120), false);
            }

            IconRenderer::draw(surface, mItem.iconId, Rect{mItem.bounds.x + 6, mItem.bounds.y + 4, 16, 16},
                               isHovered ? std::make_optional(Color::fromHex(0x00D4FF)) : std::nullopt);

            const Color txtCol = isHovered ? Color::fromHex(0xFFFFFF) : Color::fromHex(0xCBD5E1);
            surface.drawString(mItem.bounds.x + 28, mItem.bounds.y + 6, mItem.label, txtCol, 1);

            if (!mItem.shortcut.empty()) {
                surface.drawString(mItem.bounds.right() - 28, mItem.bounds.y + 6, mItem.shortcut, Color::fromHex(0x7186A4), 1);
            }

            itemY += 26;
        }
    }
}

} // namespace surshell
