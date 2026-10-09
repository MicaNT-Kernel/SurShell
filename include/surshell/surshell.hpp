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
#include "text_viewer.hpp"
#include "quick_settings.hpp"
#include "virtual_desktop.hpp"
#include "kernel_bridge.hpp"
#include "icons.hpp"
#include "alt_tab.hpp"

#include "toast.hpp"
#include "media_hud.hpp"
#include "task_manager.hpp"
#include "settings.hpp"
#include "calculator.hpp"
#include "run_dialog.hpp"
#include "terminal.hpp"
#include "action_center.hpp"
#include "search_hub.hpp"
#include "lock_screen.hpp"
#include "registry_editor.hpp"
#include "image_viewer.hpp"
#include "paint.hpp"
#include "sysinfo.hpp"
#include "devicemanager.hpp"

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
    [[nodiscard]] AltTabSwitcher& altTab() noexcept { return altTab_; }
    [[nodiscard]] const AltTabSwitcher& altTab() const noexcept { return altTab_; }
    [[nodiscard]] ToastManager& toastManager() noexcept { return toastManager_; }
    [[nodiscard]] const ToastManager& toastManager() const noexcept { return toastManager_; }
    [[nodiscard]] MediaHud& mediaHud() noexcept { return mediaHud_; }
    [[nodiscard]] const MediaHud& mediaHud() const noexcept { return mediaHud_; }
    [[nodiscard]] ActionCenterFlyout& actionCenter() noexcept { return actionCenter_; }
    [[nodiscard]] const ActionCenterFlyout& actionCenter() const noexcept { return actionCenter_; }
    [[nodiscard]] SearchHub& searchHub() noexcept { return searchHub_; }
    [[nodiscard]] const SearchHub& searchHub() const noexcept { return searchHub_; }
    [[nodiscard]] LockScreen& lockScreen() noexcept { return lockScreen_; }
    [[nodiscard]] const LockScreen& lockScreen() const noexcept { return lockScreen_; }
    [[nodiscard]] KernelBridge& kernel() noexcept { return kernelBridge_; }
    [[nodiscard]] Surface& framebuffer() noexcept { return framebuffer_; }

    // Shell Hub Modals
    void openSearchHub();
    void openActionCenter();
    void lockSession();
    void toggleShowDesktop();

    // Alt+Tab Task Switcher Workflow
    void triggerAltTab();
    void cycleAltTab(bool forward = true);
    void commitAltTab();
    void dismissAltTab();

    // Master Input Dispatching
    void onMouseDown(Point pt, MouseButton button);
    void onMouseUp(Point pt, MouseButton button);
    void onMouseMove(Point pt);
    void onDoubleClick(Point pt);
    void onMouseWheel(Point pt, int32_t delta);
    void onCharInput(char c);
    void onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false, bool win = false);

    // Application Window Spawning Helpers
    uint32_t openFileExplorerWindow(std::string path = "C:\\Windows\\System32");
    uint32_t openTextEditorWindow(std::string filePath = "");
    uint32_t openTerminalWindow(std::string workingDir = "C:\\Windows\\System32");
    uint32_t openTaskManagerWindow();
    uint32_t openSettingsWindow();
    uint32_t openCalculatorWindow();
    uint32_t openRunDialogWindow();
    uint32_t openRegistryEditorWindow(std::string initialKey = "Computer\\HKEY_CURRENT_USER\\Software\\MicaNT\\SurShell");
    uint32_t openImageViewerWindow(std::string imagePath = "");
    uint32_t openPaintWindow(std::string filePath = "");
    uint32_t openSystemInfoWindow();
    uint32_t openDeviceManagerWindow();

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
    AltTabSwitcher altTab_;
    ToastManager toastManager_;
    MediaHud mediaHud_;
    ActionCenterFlyout actionCenter_;
    SearchHub searchHub_;
    LockScreen lockScreen_;
    KernelBridge kernelBridge_;

    Point currentMousePos_{0, 0};
    bool showTopBar_{false}; // Default false for authentic Windows edge-to-edge desktop experience

    void setupDefaultEnvironment();
    void wireSubsystemCallbacks();
};

} // namespace surshell
