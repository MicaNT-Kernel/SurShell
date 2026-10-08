// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/taskbar.hpp)
//
// Desktop Taskbar (Shell_TrayWnd parity), Start button, task list, and system tray.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "tray.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <optional>

namespace surshell {

struct TaskItem {
    uint32_t windowId{0};
    std::string title;
    std::string iconGlyph;
    IconId iconId{IconId::Terminal};
    bool isActive{false};
    bool isMinimized{false};
    Rect bounds{};
};

class Taskbar {
public:
    using StartButtonClickCallback = std::function<void()>;
    using SearchButtonClickCallback = std::function<void()>;
    using TaskItemClickCallback = std::function<void(uint32_t windowId)>;
    using TrayClickCallback = std::function<void()>;
    using TaskViewClickCallback = std::function<void()>;

    Taskbar(uint32_t screenWidth, uint32_t screenHeight);

    void setScreenSize(uint32_t width, uint32_t height);
    [[nodiscard]] Rect bounds() const noexcept;

    void setStartButtonClickCallback(StartButtonClickCallback cb) { startClickCallback_ = std::move(cb); }
    void setSearchButtonClickCallback(SearchButtonClickCallback cb) { searchClickCallback_ = std::move(cb); }
    void setTaskItemClickCallback(TaskItemClickCallback cb) { taskClickCallback_ = std::move(cb); }
    void setTrayClickCallback(TrayClickCallback cb) { trayClickCallback_ = std::move(cb); }
    void setTaskViewClickCallback(TaskViewClickCallback cb) { taskViewClickCallback_ = std::move(cb); }

    void setStyle(TaskbarStyle style) noexcept { style_ = style; recalculateLayout(); }
    [[nodiscard]] TaskbarStyle style() const noexcept { return style_; }

    void setAlignment(TaskbarAlignment alignment) noexcept { alignment_ = alignment; recalculateLayout(); }
    [[nodiscard]] TaskbarAlignment alignment() const noexcept { return alignment_; }

    [[nodiscard]] Rect appIslandBounds() const noexcept { return appIslandBounds_; }
    [[nodiscard]] Rect trayIslandBounds() const noexcept { return trayIslandBounds_; }
    [[nodiscard]] Rect startButtonBounds() const noexcept { return startButtonBounds_; }
    [[nodiscard]] Rect searchButtonBounds() const noexcept { return searchButtonBounds_; }
    [[nodiscard]] Rect taskViewButtonBounds() const noexcept { return taskViewButtonBounds_; }

    void addOrUpdateTask(uint32_t windowId, std::string title, std::string glyph, bool active, bool minimized, std::optional<IconId> iconId = std::nullopt);
    void removeTask(uint32_t windowId);
    void setActiveTask(uint32_t windowId);

    [[nodiscard]] SystemTray& tray() noexcept { return tray_; }
    [[nodiscard]] const SystemTray& tray() const noexcept { return tray_; }

    [[nodiscard]] const std::vector<TaskItem>& tasks() const noexcept { return tasks_; }

    using WindowPreviewProvider = std::function<const Surface*(uint32_t windowId)>;
    using PreviewCloseCallback = std::function<void(uint32_t windowId)>;

    void setWindowPreviewProvider(WindowPreviewProvider provider) { previewProvider_ = std::move(provider); }
    void setPreviewCloseCallback(PreviewCloseCallback cb) { previewCloseCallback_ = std::move(cb); }

    [[nodiscard]] int32_t hoveredTaskWindowId() const noexcept { return hoveredTaskWindowId_; }
    void setHoveredTaskWindowId(int32_t id) noexcept { hoveredTaskWindowId_ = id; }

    [[nodiscard]] Rect hoverPreviewBounds(uint32_t windowId) const noexcept;
    [[nodiscard]] Rect hoverPreviewCloseButtonBounds(uint32_t windowId) const noexcept;

    // Input Events
    void onMouseDown(Point pt, MouseButton button);
    void onMouseMove(Point pt);

    // Rendering
    void render(Surface& surface);
    void renderHoverPreview(Surface& surface, const Surface* previewSurface = nullptr) const;

private:
    uint32_t screenWidth_{1920};
    uint32_t screenHeight_{1080};
    int32_t height_{48};
    TaskbarStyle style_{TaskbarStyle::FloatingIsland};
    TaskbarAlignment alignment_{TaskbarAlignment::Center};
    SystemTray tray_{};

    Rect appIslandBounds_{};
    Rect trayIslandBounds_{};
    Rect startButtonBounds_{};
    Rect searchButtonBounds_{};
    Rect taskViewButtonBounds_{};
    bool isStartButtonHovered_{false};
    bool isSearchHovered_{false};
    bool isTaskViewHovered_{false};
    int32_t hoveredTaskWindowId_{-1};

    std::vector<TaskItem> tasks_{};

    StartButtonClickCallback startClickCallback_{};
    SearchButtonClickCallback searchClickCallback_{};
    TaskItemClickCallback taskClickCallback_{};
    TrayClickCallback trayClickCallback_{};
    TaskViewClickCallback taskViewClickCallback_{};
    WindowPreviewProvider previewProvider_{};
    PreviewCloseCallback previewCloseCallback_{};

    void recalculateLayout();
};

} // namespace surshell
