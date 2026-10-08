// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/surshell.cpp)
// ============================================================================

#include "surshell/surshell.hpp"
#include <iostream>

namespace surshell {

SurShellDesktop::SurShellDesktop(uint32_t width, uint32_t height)
    : width_(width),
      height_(height),
      framebuffer_(width, height),
      desktop_(width, height),
      taskbar_(width, height),
      startMenu_(),
      windowManager_(width, height, 40),
      quickSettings_(),
      virtualDesktops_(),
      kernelBridge_() {
    wireSubsystemCallbacks();
    setupDefaultEnvironment();
    virtualDesktops_.updateLayout(width_, height_, taskbar_.bounds().height);
}

void SurShellDesktop::setScreenSize(uint32_t width, uint32_t height) {
    width_ = width;
    height_ = height;
    framebuffer_.resize(width, height);
    desktop_.setScreenSize(width, height);
    taskbar_.setScreenSize(width, height);
    windowManager_.setScreenSize(width, height, 40);
    quickSettings_.updateLayout(width, height, taskbar_.bounds().height);
    virtualDesktops_.updateLayout(width, height, taskbar_.bounds().height);
}

void SurShellDesktop::setupDefaultEnvironment() {
    // 1. Initialize Kernel Bridge Connection to SurWin/CSRSS LPC Executive
    kernelBridge_.connectToExecutive("\\RPC_Control\\SurWinLpc");

    // 2. Setup Standard Sovereign Desktop Icons
    desktop_.addIcon(DesktopIcon{
        .id = "this_pc",
        .label = "This PC",
        .executable = "C:\\Windows\\explorer.exe",
        .arguments = "",
        .iconGlyph = "[P]",
        .iconId = IconId::ThisPC
    });

    desktop_.addIcon(DesktopIcon{
        .id = "cmd",
        .label = "Terminal",
        .executable = "C:\\Windows\\System32\\cmd.exe",
        .arguments = "",
        .iconGlyph = ">_",
        .iconId = IconId::Terminal
    });

    desktop_.addIcon(DesktopIcon{
        .id = "explorer",
        .label = "Explorer",
        .executable = "C:\\Windows\\explorer.exe",
        .arguments = "",
        .iconGlyph = "[E]",
        .iconId = IconId::FileExplorer
    });

    desktop_.addIcon(DesktopIcon{
        .id = "sentinel",
        .label = "Sentinel",
        .executable = "C:\\Program Files\\Sentinel\\sentinel.exe",
        .arguments = "",
        .iconGlyph = "[S]",
        .iconId = IconId::SentinelSec
    });

    desktop_.addIcon(DesktopIcon{
        .id = "settings",
        .label = "Settings",
        .executable = "C:\\Windows\\System32\\control.exe",
        .arguments = "",
        .iconGlyph = "[*]",
        .iconId = IconId::Settings
    });

    // 3. Spawn Initial Sovereign Windows: Command Prompt & File Explorer
    openTerminalWindow("C:\\Windows\\System32");
    openFileExplorerWindow("C:\\Windows\\System32");

    // Initialize Layouts for flyouts
    quickSettings_.updateLayout(width_, height_, taskbar_.bounds().height);
    virtualDesktops_.updateLayout(width_, height_, taskbar_.bounds().height);
}

uint32_t SurShellDesktop::openFileExplorerWindow(std::string path) {
    const uint32_t winExp = windowManager_.createWindow("File Explorer - " + path, Rect{440, 130, 720, 460}, "[E]", IconId::FileExplorer);
    virtualDesktops_.assignWindowToDesktop(winExp, virtualDesktops_.activeIndex());
    auto* expWin = windowManager_.findWindow(winExp);
    if (expWin) {
        auto explorer = std::make_shared<FileExplorer>(path);

        explorer->setPathChangeCallback([this, winExp](const std::string& newPath) {
            auto* w = windowManager_.findWindow(winExp);
            if (w) {
                w->title = "File Explorer - " + newPath;
                taskbar_.addOrUpdateTask(winExp, w->title, w->iconGlyph, w->isActive, w->state == WindowState::Minimized, w->iconId);
            }
        });

        explorer->setExecuteCallback([this](const std::string& execPath) {
            kernelBridge_.spawnProcess(execPath, "");
        });

        explorer->setOpenEditorCallback([this](const std::string& filePath) {
            openTextEditorWindow(filePath);
        });

        explorer->setOpenTerminalCallback([this](const std::string& workingDir) {
            openTerminalWindow(workingDir);
        });

        expWin->content = explorer;
        explorer->render(expWin->clientSurface);
    }
    return winExp;
}

uint32_t SurShellDesktop::openTextEditorWindow(std::string filePath) {
    std::string title = "Sovereign Editor";
    if (!filePath.empty()) {
        const size_t slash = filePath.find_last_of("\\/");
        title += " - [" + (slash != std::string::npos ? filePath.substr(slash + 1) : filePath) + "]";
    }
    const uint32_t winId = windowManager_.createWindow(title, Rect{320, 160, 680, 440}, "[T]", IconId::FileCode);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto viewer = std::make_shared<TextViewerContent>(filePath);
        win->content = viewer;
        viewer->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openTerminalWindow(std::string workingDir) {
    const uint32_t winCmd = windowManager_.createWindow("Command Prompt - [" + workingDir + "]", Rect{50, 45, 680, 420}, ">_", IconId::Terminal);
    virtualDesktops_.assignWindowToDesktop(winCmd, virtualDesktops_.activeIndex());
    auto* cmdWin = windowManager_.findWindow(winCmd);
    if (cmdWin) {
        auto& cs = cmdWin->clientSurface;
        cs.clear(Color{12, 16, 24, 255});
        cs.drawString(14, 14, "MicaNT Sovereign Executive [Version 10.0.26100.1]", Color{0, 212, 255}, 1);
        cs.drawString(14, 30, "Dave Cutler 1988 Architecture | Clean-Room ISO C++23 | Zero Telemetry", Color{170, 185, 205}, 1);
        cs.drawString(14, 50, workingDir + "> whoami", Color{245, 248, 255}, 1);
        cs.drawString(14, 66, "MICANT-DESKTOP\\Administrator (S-1-5-18 LocalSystem)", Color{0, 255, 157}, 1);
        cs.drawString(14, 90, workingDir + "> surshell --status", Color{245, 248, 255}, 1);
        cs.drawString(14, 106, "[SurShell] Display Server: SurWin (CSRSS / Window Stations Active)", Color{245, 248, 255}, 1);
        cs.drawString(14, 122, "[SurShell] Compositor: PrismX DWM Software Composition 120Hz [OK]", Color{245, 248, 255}, 1);
        cs.drawString(14, 138, "[SurShell] Footprint: 14.8 MB Resident | 0 Background Daemons", Color{0, 255, 157}, 1);
        cs.drawString(14, 162, workingDir + "> _", Color{245, 248, 255}, 1);
    }
    return winCmd;
}

uint32_t SurShellDesktop::openTaskManagerWindow() {
    const uint32_t winId = windowManager_.createWindow("Task Manager", Rect{240, 120, 720, 480}, "[T]", IconId::TaskManager);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto taskMgr = std::make_shared<TaskManagerContent>(&kernelBridge_);
        taskMgr->setTerminatedCallback([this](uint32_t pid, const std::string& name) {
            toastManager_.showToast("Process Terminated", "Killed " + name + " (PID " + std::to_string(pid) + ")",
                                    IconId::TaskManager, Color::fromHex(0xFF5555));
        });
        win->content = taskMgr;
        taskMgr->render(win->clientSurface);
    }
    return winId;
}

void SurShellDesktop::wireSubsystemCallbacks() {
    // 1. Taskbar Start Button clicks toggle the Start Menu
    taskbar_.setStartButtonClickCallback([this]() {
        startMenu_.toggle();
        if (startMenu_.isOpen()) {
            quickSettings_.close();
            virtualDesktops_.hideSwitcher();
        }
    });

    // 2. Taskbar Task item clicks toggle/focus window
    taskbar_.setTaskItemClickCallback([this](uint32_t windowId) {
        auto* win = windowManager_.findWindow(windowId);
        if (!win) return;
        if (win->isActive && win->state != WindowState::Minimized) {
            windowManager_.toggleMinimize(windowId);
        } else {
            windowManager_.setWindowState(windowId, WindowState::Normal);
            windowManager_.setWindowActive(windowId);
        }
    });

    // 3. Taskbar Tray Island click toggles Quick Settings flyout
    taskbar_.setTrayClickCallback([this]() {
        quickSettings_.toggle();
        if (quickSettings_.isOpen()) {
            startMenu_.close();
            virtualDesktops_.hideSwitcher();
        }
    });

    // 3b. System Tray Icon individual click callbacks
    taskbar_.tray().setClickCallback([this](const std::string& id, MouseButton) {
        if (id == "network") {
            toastManager_.showToast("Network Telemetry", "Ethernet Connected: 1000/1000 Mbps | IPv4: 192.168.1.105", IconId::NetworkEthernet, Color::fromHex(0x00FF9D));
        } else if (id == "security") {
            toastManager_.showToast("SentinelSec Security", "Zero-Telemetry Protection Active | System Enclave Secure", IconId::SentinelSec, Color::fromHex(0x00D4FF));
        }
    });

    // 3c. Quick Settings Controls callbacks
    quickSettings_.setVolumeCallback([this](int32_t vol) {
        taskbar_.tray().setVolumeLevel(vol);
        mediaHud_.showVolume(vol);
    });

    quickSettings_.setToggleCallback([this](std::string_view id, bool enabled) {
        if (id == "network") {
            taskbar_.tray().setNetworkOnline(enabled);
            toastManager_.showToast("Network Configuration",
                                    enabled ? "Gigabit Ethernet: Connected (1.0 Gbps)" : "Network Interface Disabled",
                                    IconId::NetworkOnline,
                                    enabled ? Color::fromHex(0x00FF9D) : Color::fromHex(0xFF5555));
        }
    });

    // 3d. Media Transport callbacks
    mediaHud_.setPlayPauseCallback([this]() {
        toastManager_.showToast("Media Playback", mediaHud_.isPlaying() ? "Resumed: Symphony in C++23" : "Playback Paused", IconId::MediaPlay);
    });
    mediaHud_.setNextCallback([this]() {
        toastManager_.showToast("Track Changed", "Next: Cutler Kernel Suite Mov. 2", IconId::MediaNext);
    });
    mediaHud_.setPrevCallback([this]() {
        toastManager_.showToast("Track Changed", "Previous: Mica NT Overture", IconId::MediaPrev);
    });

    // 4. Taskbar Task View button toggles Virtual Desktops switcher strip
    taskbar_.setTaskViewClickCallback([this]() {
        virtualDesktops_.toggleSwitcher();
        if (virtualDesktops_.isSwitcherVisible()) {
            startMenu_.close();
            quickSettings_.close();
        }
    });

    // 5. Virtual Desktop switching updates window visibility
    virtualDesktops_.setSwitchCallback([this](size_t) {
        for (const auto& win : windowManager_.windows()) {
            win->isVisible = virtualDesktops_.isWindowVisible(win->id);
        }
    });

    // 5b. Task View Windows Provider and Action Handlers
    virtualDesktops_.setWindowsProvider([this]() {
        std::vector<TaskViewWindowCard> cards;
        for (const auto& win : windowManager_.windows()) {
            if (virtualDesktops_.isWindowVisible(win->id)) {
                cards.push_back(TaskViewWindowCard{
                    .windowId = win->id,
                    .title = win->title,
                    .iconId = win->iconId,
                    .originalBounds = win->currentBounds,
                    .previewSurface = &win->clientSurface,
                    .isActive = win->isActive,
                    .isMinimized = (win->state == WindowState::Minimized),
                    .cardBounds = {},
                    .closeButtonBounds = {}
                });
            }
        }
        return cards;
    });

    virtualDesktops_.setWindowSelectCallback([this](uint32_t windowId) {
        auto* win = windowManager_.findWindow(windowId);
        if (win) {
            if (win->state == WindowState::Minimized) {
                windowManager_.setWindowState(windowId, WindowState::Normal);
            }
            windowManager_.setWindowActive(windowId);
        }
    });

    virtualDesktops_.setWindowCloseCallback([this](uint32_t windowId) {
        windowManager_.closeWindow(windowId);
    });

    // 6. Window Manager events update Taskbar tasks
    windowManager_.setCallbacks(
        [this](uint32_t windowId, WindowState state, bool active) {
            auto* win = windowManager_.findWindow(windowId);
            if (!win) return;
            taskbar_.addOrUpdateTask(windowId, win->title, win->iconGlyph, active, state == WindowState::Minimized, win->iconId);
        },
        [this](uint32_t windowId) {
            taskbar_.removeTask(windowId);
            virtualDesktops_.unassignWindow(windowId);
        }
    );

    // 7. Desktop Icon double-click launches window & registers with kernel
    desktop_.setLaunchCallback([this](const DesktopIcon& icon) {
        if (icon.executable == "C:\\Windows\\explorer.exe") {
            openFileExplorerWindow("C:\\Users\\admin");
        } else if (icon.executable == "C:\\Windows\\System32\\cmd.exe") {
            openTerminalWindow("C:\\Users\\admin");
        } else {
            kernelBridge_.spawnProcess(icon.executable, icon.arguments);
            const uint32_t wid = windowManager_.createWindow(icon.label + " - [" + icon.executable + "]", Rect{200, 150, 640, 400}, icon.iconGlyph, icon.iconId);
            virtualDesktops_.assignWindowToDesktop(wid, virtualDesktops_.activeIndex());
        }
    });

    // 8. Start Menu App click launches window & registers with kernel
    startMenu_.setLaunchCallback([this](const ShellAppEntry& app) {
        if (app.executablePath == "C:\\Windows\\explorer.exe") {
            openFileExplorerWindow("C:\\Users\\admin");
        } else if (app.executablePath == "C:\\Windows\\System32\\cmd.exe") {
            openTerminalWindow("C:\\Users\\admin");
        } else if (app.executablePath == "C:\\Windows\\notepad.exe") {
            openTextEditorWindow("");
        } else if (app.executablePath == "C:\\Windows\\System32\\taskmgr.exe") {
            openTaskManagerWindow();
        } else {
            kernelBridge_.spawnProcess(app.executablePath, app.arguments);
            const uint32_t wid = windowManager_.createWindow(app.title, Rect{240, 180, 660, 420}, app.iconGlyph, IconRenderer::iconForAppId(app.id));
            virtualDesktops_.assignWindowToDesktop(wid, virtualDesktops_.activeIndex());
        }
    });

    // 9. Start Menu Power Option callback
    startMenu_.setPowerCallback([this](PowerAction action) {
        switch (action) {
            case PowerAction::Lock:
                kernelBridge_.spawnProcess("rundll32.exe", "user32.dll,LockWorkStation");
                break;
            case PowerAction::Sleep:
                kernelBridge_.spawnProcess("rundll32.exe", "powrprof.dll,SetSuspendState 0,1,0");
                break;
            case PowerAction::Hibernate:
                kernelBridge_.spawnProcess("shutdown.exe", "/h");
                break;
            case PowerAction::Restart:
                kernelBridge_.spawnProcess("shutdown.exe", "/r /t 0");
                break;
            case PowerAction::ShutDown:
                kernelBridge_.spawnProcess("shutdown.exe", "/s /t 0");
                break;
            case PowerAction::SignOut:
                kernelBridge_.spawnProcess("logoff.exe", "");
                break;
        }
    });

    // 10. Start Menu Recommended Item Open callback
    startMenu_.setOpenItemCallback([this](const std::string& path) {
        if (path.ends_with(".hpp") || path.ends_with(".cpp") || path.ends_with(".txt") || path.ends_with(".log")) {
            openTextEditorWindow(path);
        } else if (path.find('.') == std::string::npos || path.ends_with("\\")) {
            openFileExplorerWindow(path);
        } else {
            kernelBridge_.spawnProcess(path, "");
        }
    });

    // 11. Taskbar Live Hover Preview Provider & Close Callback
    taskbar_.setWindowPreviewProvider([this](uint32_t windowId) -> const Surface* {
        auto* win = windowManager_.findWindow(windowId);
        return win ? &win->clientSurface : nullptr;
    });

    taskbar_.setPreviewCloseCallback([this](uint32_t windowId) {
        windowManager_.closeWindow(windowId);
    });
}

void SurShellDesktop::triggerAltTab() {
    std::vector<AltTabItem> items;
    for (auto it = windowManager_.windows().rbegin(); it != windowManager_.windows().rend(); ++it) {
        const auto& win = *it;
        if (!win->isVisible && win->state != WindowState::Minimized) continue;
        items.push_back(AltTabItem{
            .windowId = win->id,
            .title = win->title,
            .iconGlyph = win->iconGlyph,
            .iconId = win->iconId,
            .isActive = win->isActive,
            .isMinimized = (win->state == WindowState::Minimized),
            .previewSurface = &win->clientSurface
        });
    }
    if (!items.empty()) {
        altTab_.show(std::move(items), 1);
    }
}

void SurShellDesktop::cycleAltTab(bool forward) {
    if (!altTab_.isActive()) {
        triggerAltTab();
    } else {
        if (forward) {
            altTab_.next();
        } else {
            altTab_.previous();
        }
    }
}

void SurShellDesktop::commitAltTab() {
    if (!altTab_.isActive()) return;
    if (auto chosenId = altTab_.confirm()) {
        auto* win = windowManager_.findWindow(*chosenId);
        if (win) {
            if (win->state == WindowState::Minimized) {
                windowManager_.setWindowState(*chosenId, WindowState::Normal);
            }
            windowManager_.setWindowActive(*chosenId);
        }
    }
}

void SurShellDesktop::dismissAltTab() {
    altTab_.dismiss();
}

void SurShellDesktop::onMouseDown(Point pt, MouseButton button) {
    currentMousePos_ = pt;

    // 0a. Toast notifications hit testing
    if (toastManager_.onMouseDown(pt, button)) {
        return;
    }

    // 0b. Media HUD hit testing
    if (mediaHud_.onMouseDown(pt, button)) {
        return;
    }

    // 0c. Alt+Tab HUD takes precedence when active
    if (altTab_.isActive()) {
        if (auto chosenId = altTab_.onMouseDown(pt, button, width_, height_)) {
            auto* win = windowManager_.findWindow(*chosenId);
            if (win) {
                if (win->state == WindowState::Minimized) {
                    windowManager_.setWindowState(*chosenId, WindowState::Normal);
                }
                windowManager_.setWindowActive(*chosenId);
            }
        }
        return;
    }

    // 1. Quick Settings Flyout (highest z-order when open)
    if (quickSettings_.isOpen()) {
        if (quickSettings_.bounds().contains(pt)) {
            quickSettings_.onMouseDown(pt, button);
            return;
        } else {
            quickSettings_.close();
        }
    }

    // 2. Virtual Desktops Switcher HUD (if visible)
    if (virtualDesktops_.isSwitcherVisible()) {
        if (virtualDesktops_.onMouseDown(pt, button)) {
            return;
        }
    }

    // 3. Start Menu card (if open)
    const Rect smBounds = startMenu_.calculateBounds(width_, height_, taskbar_.bounds().height);
    if (startMenu_.isOpen()) {
        const bool inFlyout = (startMenu_.isPowerFlyoutOpen() && startMenu_.powerFlyoutBounds(smBounds).contains(pt)) ||
                              (startMenu_.isUserFlyoutOpen() && startMenu_.userFlyoutBounds(smBounds).contains(pt));
        if (smBounds.contains(pt) || inFlyout) {
            startMenu_.onMouseDown(pt, button, smBounds);
            return;
        } else {
            startMenu_.close();
        }
    }

    // 4. Taskbar (App Island, Tray Island, Start, Task View)
    if (taskbar_.bounds().contains(pt)) {
        taskbar_.onMouseDown(pt, button);
        return;
    }

    // 5. Window Manager (Windows, Captions, Snap Flyout, Borders)
    if (windowManager_.onMouseDown(pt, button)) {
        return;
    }

    // 6. Fallthrough to Desktop icons & marquee
    desktop_.onMouseDown(pt, button);
}

void SurShellDesktop::onMouseUp(Point pt, MouseButton button) {
    currentMousePos_ = pt;
    quickSettings_.onMouseUp(pt, button);
    windowManager_.onMouseUp(pt, button);
    desktop_.onMouseUp(pt, button);
}

void SurShellDesktop::onMouseMove(Point pt) {
    currentMousePos_ = pt;

    if (altTab_.isActive()) {
        if (altTab_.onMouseMove(pt, width_, height_)) {
            return;
        }
    }

    if (quickSettings_.isOpen()) {
        quickSettings_.onMouseMove(pt);
    }

    if (toastManager_.onMouseMove(pt)) {
        // hovered toast
    }

    if (mediaHud_.onMouseMove(pt)) {
        // hovered media hud
    }

    if (virtualDesktops_.isSwitcherVisible()) {
        virtualDesktops_.onMouseMove(pt);
    }

    const Rect smBounds = startMenu_.calculateBounds(width_, height_, taskbar_.bounds().height);
    if (startMenu_.isOpen()) {
        startMenu_.onMouseMove(pt, smBounds);
    }

    taskbar_.onMouseMove(pt);
    if (windowManager_.onMouseMove(pt)) {
        return;
    }
    desktop_.onMouseMove(pt);
}

void SurShellDesktop::onDoubleClick(Point pt) {
    currentMousePos_ = pt;
    if (windowManager_.onDoubleClick(pt)) return;
    desktop_.onDoubleClick(pt);
}

void SurShellDesktop::onMouseWheel(Point pt, int32_t delta) {
    currentMousePos_ = pt;
    windowManager_.onMouseWheel(pt, delta);
}

void SurShellDesktop::onCharInput(char c) {
    if (startMenu_.isOpen()) {
        if (c == '\b') {
            startMenu_.handleBackspace();
        } else {
            startMenu_.handleCharInput(c);
        }
    } else {
        windowManager_.onCharInput(c);
    }
}

void SurShellDesktop::onKeyDown(KeyCode key, bool ctrl, bool shift, bool alt) {
    if (altTab_.isActive()) {
        if (key == KeyCode::Tab) {
            cycleAltTab(!shift);
            return;
        } else if (key == KeyCode::Escape) {
            dismissAltTab();
            return;
        } else if (key == KeyCode::Enter) {
            commitAltTab();
            return;
        }
    }
    windowManager_.onKeyDown(key, ctrl, shift, alt);
}

void SurShellDesktop::render() {
    // 1. Render Desktop background and icons
    desktop_.render(framebuffer_);

    // 2. Render Windows in Z-order with drop shadows
    windowManager_.render(framebuffer_);

    // 3. Render Top Architectural Sovereign Header Bar (Optional Diagnostic HUD)
    if (showTopBar_) {
        framebuffer_.fillRect(Rect{0, 0, static_cast<int32_t>(width_), 26}, Color::fromRgba(16, 22, 34, 245));
        framebuffer_.fillRect(Rect{0, 25, static_cast<int32_t>(width_), 1}, Color::fromRgba(38, 52, 78, 200));

        framebuffer_.drawString(12, 8, "MicaNT 64-Bit OS", Color::fromHex(0x00D4FF), 1);
        framebuffer_.drawString(140, 8, "|  Dave Cutler 1988 Architecture  |  Zero Telemetry  |  SurWin Subsystem  |  120Hz VSync",
                                Color::fromRgba(165, 180, 205, 255), 1);

        // Right-aligned header badge
        const int32_t rightBadgeX = static_cast<int32_t>(width_) - 190;
        framebuffer_.drawRoundedRect(Rect{rightBadgeX, 5, 178, 16}, 4, Color::fromRgba(25, 35, 55, 220), true);
        framebuffer_.drawString(rightBadgeX + 8, 9, "PASSIVE_LEVEL [IRQL 0]", Color::fromHex(0x00FF9D), 1);
    }

    // 4. Render Taskbar (Floating Island Dock)
    taskbar_.render(framebuffer_);

    // 5. Render Virtual Desktops Switcher Strip (if visible)
    if (virtualDesktops_.isSwitcherVisible()) {
        const auto& palette = ThemeManager::instance().palette();
        virtualDesktops_.renderSwitcher(framebuffer_, palette);
    }

    // 6. Render Start Menu overlay (if open)
    if (startMenu_.isOpen()) {
        const Rect smBounds = startMenu_.calculateBounds(width_, height_, taskbar_.bounds().height);
        startMenu_.render(framebuffer_, smBounds);
    }

    // 7. Render Quick Settings Flyout (if open)
    if (quickSettings_.isOpen()) {
        const auto& palette = ThemeManager::instance().palette();
        quickSettings_.render(framebuffer_, palette);
    }

    // 8. Render Media Playback HUD OSD (if visible)
    if (mediaHud_.isVisible()) {
        mediaHud_.render(framebuffer_, width_, height_, taskbar_.bounds().height);
    }

    // 9. Render Toast Notifications
    toastManager_.render(framebuffer_, width_, height_, taskbar_.bounds().height);

    // 10. Render Alt+Tab HUD (if active)
    if (altTab_.isActive()) {
        altTab_.render(framebuffer_, width_, height_);
    }

    // 9. Render Mouse Cursor Arrow
    const int32_t mx = currentMousePos_.x;
    const int32_t my = currentMousePos_.y;
    if (mx >= 0 && mx < static_cast<int32_t>(width_) && my >= 0 && my < static_cast<int32_t>(height_)) {
        for (int32_t cy = 0; cy < 12; ++cy) {
            for (int32_t cx = 0; cx <= cy && cx < 8; ++cx) {
                framebuffer_.putPixel(mx + cx, my + cy, Color::fromHex(0xFFFFFF));
            }
        }
        for (int32_t cy = 0; cy < 13; ++cy) {
            framebuffer_.putPixel(mx, my + cy, Color::fromHex(0x000000));
            framebuffer_.putPixel(mx + cy, my + cy, Color::fromHex(0x000000));
        }
    }
}

bool SurShellDesktop::exportSnapshot(const std::string& bmpPath) const {
    return framebuffer_.exportBmp(bmpPath);
}

} // namespace surshell
