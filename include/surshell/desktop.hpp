// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/desktop.hpp)
//
// Desktop Surface Manager (Progman / WorkerW parity), wallpaper compositor,
// desktop icon grid, multi-selection marquee, and context menu dispatch.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "icons.hpp"
#include <vector>
#include <string>
#include <functional>
#include <optional>

namespace surshell {

enum class WallpaperStyle {
    MicaGrid = 0,
    AuroraBorealis,
    SovereignSlate,
    MidnightNebula
};

struct DesktopIcon {
    std::string id;
    std::string label;
    std::string executable;
    std::string arguments;
    std::string iconGlyph{"[P]"};
    IconId iconId{IconId::StartPrism};
    int32_t gridX{0};
    int32_t gridY{0};
    Rect bounds{};
    bool selected{false};
};

struct DesktopContextMenuItem {
    std::string id;
    std::string label;
    std::string shortcut;
    IconId iconId{IconId::FileGeneric};
    bool isSeparator{false};
    Rect bounds{};
};

struct DesktopContextMenu {
    bool isOpen{false};
    Point position{0, 0};
    std::vector<DesktopContextMenuItem> items{};
    int32_t hoveredIndex{-1};
    Rect bounds{};
    std::string targetIconId{};
};

class DesktopManager {
public:
    using LaunchCallback = std::function<void(const DesktopIcon&)>;
    using ContextMenuActionCallback = std::function<void(const std::string& actionId, const std::string& targetIconId)>;

    DesktopManager(uint32_t screenWidth, uint32_t screenHeight);

    void setScreenSize(uint32_t width, uint32_t height);
    void addIcon(DesktopIcon icon);
    void removeIcon(std::string_view id);
    void arrangeIcons();
    void sortByName();
    void discoverHostDesktop();

    void setLaunchCallback(LaunchCallback cb) { launchCallback_ = std::move(cb); }
    void setContextMenuActionCallback(ContextMenuActionCallback cb) { contextMenuCallback_ = std::move(cb); }

    void openContextMenu(Point pt, std::string_view targetIconId = "");
    void closeContextMenu() noexcept { contextMenu_.isOpen = false; }
    [[nodiscard]] const DesktopContextMenu& contextMenu() const noexcept { return contextMenu_; }

    void setWallpaperStyle(WallpaperStyle style) noexcept { wallpaperStyle_ = style; }
    [[nodiscard]] WallpaperStyle wallpaperStyle() const noexcept { return wallpaperStyle_; }

    [[nodiscard]] const std::vector<DesktopIcon>& icons() const noexcept { return icons_; }
    [[nodiscard]] std::optional<std::reference_wrapper<const DesktopIcon>> getSelectedIcon() const noexcept;

    // Input Events
    void onMouseDown(Point pt, MouseButton button);
    void onMouseUp(Point pt, MouseButton button);
    void onMouseMove(Point pt);
    void onDoubleClick(Point pt);

    // Rendering
    void render(Surface& surface);
    void renderContextMenu(Surface& surface);

    [[nodiscard]] bool isSelecting() const noexcept { return isMarqueeActive_; }
    [[nodiscard]] Rect selectionMarquee() const noexcept { return marqueeRect_; }

private:
    uint32_t screenWidth_{1920};
    uint32_t screenHeight_{1080};
    std::vector<DesktopIcon> icons_{};

    bool isMarqueeActive_{false};
    Point marqueeStart_{0, 0};
    Rect marqueeRect_{0, 0, 0, 0};

    LaunchCallback launchCallback_{};
    ContextMenuActionCallback contextMenuCallback_{};
    WallpaperStyle wallpaperStyle_{WallpaperStyle::MicaGrid};
    DesktopContextMenu contextMenu_{};

    void updateMarquee(Point current);
    void recalculateIconBounds();
};

} // namespace surshell
