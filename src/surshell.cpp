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
      windowManager_(width, height, 40) {
    setupDefaultEnvironment();
    wireSubsystemCallbacks();
}

void SurShellDesktop::setScreenSize(uint32_t width, uint32_t height) {
    width_ = width;
    height_ = height;
    framebuffer_.resize(width, height);
    desktop_.setScreenSize(width, height);
    taskbar_.setScreenSize(width, height);
    windowManager_.setScreenSize(width, height, 40);
}

void SurShellDesktop::setupDefaultEnvironment() {
    // 1. Setup Standard Sovereign Desktop Icons
    desktop_.addIcon(DesktopIcon{
        .id = "this_pc",
        .label = "This PC",
        .executable = "C:\\Windows\\explorer.exe",
        .arguments = "",
        .iconGlyph = "[P]"
    });

    desktop_.addIcon(DesktopIcon{
        .id = "cmd",
        .label = "Terminal",
        .executable = "C:\\Windows\\System32\\cmd.exe",
        .arguments = "",
        .iconGlyph = ">_"
    });

    desktop_.addIcon(DesktopIcon{
        .id = "explorer",
        .label = "Explorer",
        .executable = "C:\\Windows\\explorer.exe",
        .arguments = "",
        .iconGlyph = "[E]"
    });

    desktop_.addIcon(DesktopIcon{
        .id = "sentinel",
        .label = "Sentinel",
        .executable = "C:\\Program Files\\Sentinel\\sentinel.exe",
        .arguments = "",
        .iconGlyph = "[S]"
    });

    desktop_.addIcon(DesktopIcon{
        .id = "settings",
        .label = "Settings",
        .executable = "C:\\Windows\\System32\\control.exe",
        .arguments = "",
        .iconGlyph = "[*]"
    });

    // 2. Spawn Initial Sovereign Windows: Command Prompt & File Explorer
    const uint32_t winCmd = windowManager_.createWindow("Command Prompt - [MicaNT ConHost: cmd.exe]", Rect{40, 60, 680, 420}, ">_");
    auto* cmdWin = windowManager_.findWindow(winCmd);
    if (cmdWin) {
        // Draw initial terminal text inside cmd client surface
        auto& cs = cmdWin->clientSurface;
        cs.clear(Color{12, 16, 24, 255});
        cs.drawString(14, 14, "MicaNT Sovereign Executive [Version 10.0.26100.1]", Color{0, 212, 255}, 1);
        cs.drawString(14, 30, "Dave Cutler 1988 Architecture | Clean-Room ISO C++23 | Zero Telemetry", Color{170, 185, 205}, 1);
        cs.drawString(14, 50, "C:\\Windows\\System32> whoami", Color{245, 248, 255}, 1);
        cs.drawString(14, 66, "MICANT-DESKTOP\\Administrator (S-1-5-18 LocalSystem)", Color{0, 255, 157}, 1);
        cs.drawString(14, 90, "C:\\Windows\\System32> surshell --status", Color{245, 248, 255}, 1);
        cs.drawString(14, 106, "[SurShell] Display Server: SurWin (CSRSS / Window Stations Active)", Color{245, 248, 255}, 1);
        cs.drawString(14, 122, "[SurShell] Compositor: PrismX DWM Software Composition 120Hz [OK]", Color{245, 248, 255}, 1);
        cs.drawString(14, 138, "[SurShell] Footprint: 14.8 MB Resident | 0 Background Daemons", Color{0, 255, 157}, 1);
        cs.drawString(14, 162, "C:\\Windows\\System32> _", Color{245, 248, 255}, 1);
    }

    const uint32_t winExp = windowManager_.createWindow("File Explorer - C:\\Windows\\System32", Rect{420, 160, 720, 460}, "[E]");
    auto* expWin = windowManager_.findWindow(winExp);
    if (expWin) {
        FileExplorer explorer("C:\\Windows\\System32");
        explorer.render(expWin->clientSurface);
    }
}

void SurShellDesktop::wireSubsystemCallbacks() {
    // 1. Taskbar Start Button clicks toggle the Start Menu
    taskbar_.setStartButtonClickCallback([this]() {
        startMenu_.toggle();
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

    // 3. Window Manager events update Taskbar tasks
    windowManager_.setCallbacks(
        [this](uint32_t windowId, WindowState state, bool active) {
            auto* win = windowManager_.findWindow(windowId);
            if (!win) return;
            taskbar_.addOrUpdateTask(windowId, win->title, win->iconGlyph, active, state == WindowState::Minimized);
        },
        [this](uint32_t windowId) {
            taskbar_.removeTask(windowId);
        }
    );

    // 4. Desktop Icon double-click launches window
    desktop_.setLaunchCallback([this](const DesktopIcon& icon) {
        windowManager_.createWindow(icon.label + " - [" + icon.executable + "]", Rect{200, 150, 640, 400}, icon.iconGlyph);
    });

    // 5. Start Menu App click launches window
    startMenu_.setLaunchCallback([this](const ShellAppEntry& app) {
        windowManager_.createWindow(app.title, Rect{240, 180, 660, 420}, app.iconGlyph);
    });
}

void SurShellDesktop::onMouseDown(Point pt, MouseButton button) {
    currentMousePos_ = pt;

    // Check Start Menu first if open
    const Rect smBounds = startMenu_.calculateBounds(width_, height_, taskbar_.bounds().height);
    if (startMenu_.isOpen()) {
        if (smBounds.contains(pt)) {
            startMenu_.onMouseDown(pt, button, smBounds);
            return;
        } else {
            startMenu_.close();
        }
    }

    // Check Taskbar
    if (taskbar_.bounds().contains(pt)) {
        taskbar_.onMouseDown(pt, button);
        return;
    }

    // Check Window Manager
    if (windowManager_.onMouseDown(pt, button)) {
        return;
    }

    // Fallthrough to Desktop icons & marquee
    desktop_.onMouseDown(pt, button);
}

void SurShellDesktop::onMouseUp(Point pt, MouseButton button) {
    currentMousePos_ = pt;
    windowManager_.onMouseUp(pt, button);
    desktop_.onMouseUp(pt, button);
}

void SurShellDesktop::onMouseMove(Point pt) {
    currentMousePos_ = pt;

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

void SurShellDesktop::onCharInput(char c) {
    if (startMenu_.isOpen()) {
        if (c == '\b') {
            startMenu_.handleBackspace();
        } else {
            startMenu_.handleCharInput(c);
        }
    }
}

void SurShellDesktop::render() {
    // 1. Render Desktop background and icons
    desktop_.render(framebuffer_);

    // 2. Render Windows in Z-order with drop shadows
    windowManager_.render(framebuffer_);

    // 3. Render Top Architectural Sovereign Header Bar
    framebuffer_.fillRect(Rect{0, 0, static_cast<int32_t>(width_), 26}, Color::fromRgba(16, 22, 34, 245));
    framebuffer_.fillRect(Rect{0, 25, static_cast<int32_t>(width_), 1}, Color::fromRgba(38, 52, 78, 200));

    framebuffer_.drawString(12, 8, "MicaNT 64-Bit OS", Color::fromHex(0x00D4FF), 1);
    framebuffer_.drawString(140, 8, "|  Dave Cutler 1988 Architecture  |  Zero Telemetry  |  SurWin Subsystem  |  120Hz VSync",
                            Color::fromRgba(165, 180, 205, 255), 1);

    // Right-aligned header badge
    const int32_t rightBadgeX = static_cast<int32_t>(width_) - 190;
    framebuffer_.drawRoundedRect(Rect{rightBadgeX, 5, 178, 16}, 4, Color::fromRgba(25, 35, 55, 220), true);
    framebuffer_.drawString(rightBadgeX + 8, 9, "PASSIVE_LEVEL [IRQL 0]", Color::fromHex(0x00FF9D), 1);

    // 4. Render Taskbar
    taskbar_.render(framebuffer_);

    // 5. Render Start Menu overlay (if open)
    if (startMenu_.isOpen()) {
        const Rect smBounds = startMenu_.calculateBounds(width_, height_, taskbar_.bounds().height);
        startMenu_.render(framebuffer_, smBounds);
    }

    // 6. Render Mouse Cursor Arrow
    const int32_t mx = currentMousePos_.x;
    const int32_t my = currentMousePos_.y;
    if (mx >= 0 && mx < static_cast<int32_t>(width_) && my >= 0 && my < static_cast<int32_t>(height_)) {
        // High-contrast clean cursor arrow
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
