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
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace surshell {

struct TaskItem {
    uint32_t windowId{0};
    std::string title;
    std::string iconGlyph;
    bool isActive{false};
    bool isMinimized{false};
    Rect bounds{};
};

class Taskbar {
public:
    using StartButtonClickCallback = std::function<void()>;
    using TaskItemClickCallback = std::function<void(uint32_t windowId)>;

    Taskbar(uint32_t screenWidth, uint32_t screenHeight);

    void setScreenSize(uint32_t width, uint32_t height);
    [[nodiscard]] Rect bounds() const noexcept;

    void setStartButtonClickCallback(StartButtonClickCallback cb) { startClickCallback_ = std::move(cb); }
    void setTaskItemClickCallback(TaskItemClickCallback cb) { taskClickCallback_ = std::move(cb); }

    void addOrUpdateTask(uint32_t windowId, std::string title, std::string glyph, bool active, bool minimized);
    void removeTask(uint32_t windowId);
    void setActiveTask(uint32_t windowId);

    [[nodiscard]] SystemTray& tray() noexcept { return tray_; }
    [[nodiscard]] const SystemTray& tray() const noexcept { return tray_; }

    [[nodiscard]] const std::vector<TaskItem>& tasks() const noexcept { return tasks_; }

    // Input Events
    void onMouseDown(Point pt, MouseButton button);
    void onMouseMove(Point pt);

    // Rendering
    void render(Surface& surface);

private:
    uint32_t screenWidth_{1920};
    uint32_t screenHeight_{1080};
    int32_t height_{40};
    SystemTray tray_{};

    Rect startButtonBounds_{};
    bool isStartButtonHovered_{false};
    int32_t hoveredTaskWindowId_{-1};

    std::vector<TaskItem> tasks_{};

    StartButtonClickCallback startClickCallback_{};
    TaskItemClickCallback taskClickCallback_{};

    void recalculateLayout();
};

} // namespace surshell
