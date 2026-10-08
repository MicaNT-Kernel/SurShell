// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/surshell.hpp)
//
// Master Umbrella Header & Unified Desktop Environment Coordinator.
// ============================================================================

#pragma once

#include "types.hpp"
#include "theme.hpp"
#include "compositor.hpp"
#include "desktop.hpp"
#include "taskbar.hpp"
#include "start_menu.hpp"
#include "tray.hpp"
#include "window_manager.hpp"
#include "explorer.hpp"
#include "quick_settings.hpp"
#include "virtual_desktop.hpp"
#include "kernel_bridge.hpp"

namespace surshell {

class SurShellDesktop {
public:
    SurShellDesktop(uint32_t width = 1920, uint32_t height = 1080);

    void setScreenSize(uint32_t width, uint32_t height);
    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }

    [[nodiscard]] DesktopManager& desktop() noexcept { return desktop_; }
    [[nodiscard]] Taskbar& taskbar() noexcept { return taskbar_; }
    [[nodiscard]] StartMenu& startMenu() noexcept { return startMenu_; }
    [[nodiscard]] WindowManager& windowManager() noexcept { return windowManager_; }
    [[nodiscard]] QuickSettingsFlyout& quickSettings() noexcept { return quickSettings_; }
    [[nodiscard]] VirtualDesktopManager& virtualDesktops() noexcept { return virtualDesktops_; }
    [[nodiscard]] KernelBridge& kernel() noexcept { return kernelBridge_; }
    [[nodiscard]] Surface& framebuffer() noexcept { return framebuffer_; }

    // Master Input Dispatching
    void onMouseDown(Point pt, MouseButton button);
    void onMouseUp(Point pt, MouseButton button);
    void onMouseMove(Point pt);
    void onDoubleClick(Point pt);
    void onCharInput(char c);

    // Master Render Loop
    void render();

    // Top Architectural HUD Bar Visibility
    [[nodiscard]] bool isTopBarVisible() const noexcept { return showTopBar_; }
    void setTopBarVisible(bool visible) noexcept { showTopBar_ = visible; }
    void toggleTopBar() noexcept { showTopBar_ = !showTopBar_; }

    // Export visual snapshot to standard 32-bit BMP
    bool exportSnapshot(const std::string& bmpPath) const;

private:
    uint32_t width_{1920};
    uint32_t height_{1080};
    Surface framebuffer_{1920, 1080};

    DesktopManager desktop_;
    Taskbar taskbar_;
    StartMenu startMenu_;
    WindowManager windowManager_;
    QuickSettingsFlyout quickSettings_;
    VirtualDesktopManager virtualDesktops_;
    KernelBridge kernelBridge_;

    Point currentMousePos_{0, 0};
    bool showTopBar_{false}; // Default false for authentic Windows edge-to-edge desktop experience

    void setupDefaultEnvironment();
    void wireSubsystemCallbacks();
};

} // namespace surshell
