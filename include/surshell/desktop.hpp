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
#include <vector>
#include <string>
#include <functional>
#include <optional>

namespace surshell {

struct DesktopIcon {
    std::string id;
    std::string label;
    std::string executable;
    std::string arguments;
    std::string iconGlyph;
    int32_t gridX{0};
    int32_t gridY{0};
    Rect bounds{};
    bool selected{false};
};

class DesktopManager {
public:
    using LaunchCallback = std::function<void(const DesktopIcon&)>;

    DesktopManager(uint32_t screenWidth, uint32_t screenHeight);

    void setScreenSize(uint32_t width, uint32_t height);
    void addIcon(DesktopIcon icon);
    void removeIcon(std::string_view id);
    void arrangeIcons();

    void setLaunchCallback(LaunchCallback cb) { launchCallback_ = std::move(cb); }

    [[nodiscard]] const std::vector<DesktopIcon>& icons() const noexcept { return icons_; }
    [[nodiscard]] std::optional<std::reference_wrapper<const DesktopIcon>> getSelectedIcon() const noexcept;

    // Input Events
    void onMouseDown(Point pt, MouseButton button);
    void onMouseUp(Point pt, MouseButton button);
    void onMouseMove(Point pt);
    void onDoubleClick(Point pt);

    // Rendering
    void render(Surface& surface);

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

    void updateMarquee(Point current);
    void recalculateIconBounds();
};

} // namespace surshell
