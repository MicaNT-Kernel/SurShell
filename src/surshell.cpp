// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/surshell.cpp)
// ============================================================================

#include "surshell/surshell.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#include <shellapi.h>
#endif

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

    // Populate Host Applications into Search Hub
    searchHub_.populateHostApplications(startMenu_.allApps());

    // Discover Host Desktop shortcuts & items
    desktop_.discoverHostDesktop();

    // 3. Spawn Initial Sovereign Windows: Command Prompt & File Explorer
    openTerminalWindow("C:\\Windows\\System32");
    openFileExplorerWindow("C:\\Windows\\System32");

    // Initialize Layouts for flyouts
    quickSettings_.updateLayout(width_, height_, taskbar_.bounds().height);
    virtualDesktops_.updateLayout(width_, height_, taskbar_.bounds().height);
}

uint32_t SurShellDesktop::openFileExplorerWindow(std::string path) {
    const uint32_t winExp = windowManager_.createWindow("File Explorer - " + path, Rect{340, 80, 960, 600}, "[E]", IconId::FileExplorer);
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

        explorer->setOpenImageViewerCallback([this](const std::string& imagePath) {
            openImageViewerWindow(imagePath);
        });

        explorer->setOpenPaintCallback([this](const std::string& filePath) {
            openPaintWindow(filePath);
        });

        explorer->setToastCallback([this](const std::string& title, const std::string& message, IconId icon) {
            toastManager_.showToast(title, message, icon);
        });

        expWin->content = explorer;
        explorer->render(expWin->clientSurface);
    }
    return winExp;
}

uint32_t SurShellDesktop::openTextEditorWindow(std::string filePath) {
    std::string baseName = "Untitled.txt";
    if (!filePath.empty()) {
        const size_t slash = filePath.find_last_of("\\/");
        baseName = (slash != std::string::npos ? filePath.substr(slash + 1) : filePath);
    }
    const std::string title = baseName + " - Sovereign Notepad";
    const uint32_t winId = windowManager_.createWindow(title, Rect{320, 140, 740, 480}, "[N]", IconId::Edit);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto editor = std::make_shared<TextViewerContent>(filePath);

        editor->setTitleChangedCallback([this, winId](const std::string& newTitle) {
            auto* w = windowManager_.findWindow(winId);
            if (w) {
                w->title = newTitle;
                taskbar_.addOrUpdateTask(winId, w->title, w->iconGlyph, w->isActive, w->state == WindowState::Minimized, w->iconId);
            }
        });

        editor->setFileSavedCallback([this](const std::string& path, uint64_t bytes) {
            const size_t slash = path.find_last_of("\\/");
            const std::string name = (slash != std::string::npos ? path.substr(slash + 1) : path);
            toastManager_.showToast("File Saved", name + " (" + std::to_string(bytes) + " bytes)", IconId::Save, Color::fromHex(0x00FF9D));
        });

        win->content = editor;
        editor->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openTerminalWindow(std::string workingDir) {
    const uint32_t winCmd = windowManager_.createWindow("Command Prompt", Rect{50, 45, 720, 440}, ">_", IconId::Terminal);
    virtualDesktops_.assignWindowToDesktop(winCmd, virtualDesktops_.activeIndex());
    auto* cmdWin = windowManager_.findWindow(winCmd);
    if (cmdWin) {
        auto term = std::make_shared<TerminalContent>();
        term->setKernelBridge(&kernelBridge_);
        if (!workingDir.empty()) {
            term->setActiveTabCwd(workingDir);
        }
        term->setAppSpawnCallback([this](const std::string& app, const std::string& args) {
            if (app == "calc") openCalculatorWindow();
            else if (app == "settings") openSettingsWindow();
            else if (app == "explorer") openFileExplorerWindow(args.empty() ? "C:\\Users\\admin" : args);
            else if (app == "taskmgr") openTaskManagerWindow();
            else if (app == "regedit" || app == "registry") openRegistryEditorWindow();
            else if (app == "services" || app == "services.msc") openServicesWindow();
            else if (app == "eventvwr" || app == "eventvwr.msc" || app == "eventlog") openEventViewerWindow();
            else if (app == "winget" || app == "store" || app == "app_hub" || app == "hub") openAppHubWindow(args);
            else if (app == "cmd") openTerminalWindow(args.empty() ? "C:\\Users\\admin" : args);
            else kernelBridge_.spawnProcess(app, args);
        });
        cmdWin->content = term;
        term->render(cmdWin->clientSurface);
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

uint32_t SurShellDesktop::openSettingsWindow() {
    const uint32_t winId = windowManager_.createWindow("System Settings", Rect{200, 60, 880, 560}, "[*]", IconId::Settings);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto settings = std::make_shared<SettingsContent>();
        settings->setCurrentThemeMode(ThemeManager::instance().mode());
        settings->setCurrentWallpaper(desktop_.wallpaperStyle());
        settings->setCurrentTaskbarAlignment(taskbar_.alignment());
        settings->setCurrentTaskbarStyle(taskbar_.style());
        settings->setTopBarEnabled(showTopBar_);

        settings->setThemeModeCallback([this](ThemeMode mode) {
            ThemeManager::instance().setMode(mode);
            toastManager_.showToast("Theme Changed",
                                    mode == ThemeMode::Dark ? "Dark Theme Applied" : (mode == ThemeMode::Light ? "Light Theme Applied" : "Carbon Slate Applied"),
                                    IconId::Personalization);
        });

        settings->setAccentColorCallback([this](Color accent) {
            ThemeManager::instance().setAccentColor(accent);
            toastManager_.showToast("Personalization", "System Accent Color Updated", IconId::Personalization, accent);
        });

        settings->setWallpaperCallback([this](WallpaperStyle style) {
            desktop_.setWallpaperStyle(style);
            toastManager_.showToast("Personalization", "Desktop Wallpaper Updated", IconId::Display);
        });

        settings->setTaskbarAlignmentCallback([this](TaskbarAlignment al) {
            taskbar_.setAlignment(al);
        });

        settings->setTaskbarStyleCallback([this](TaskbarStyle st) {
            taskbar_.setStyle(st);
        });

        settings->setTopBarCallback([this](bool enabled) {
            setTopBarVisible(enabled);
        });

        settings->setToastCallback([this](const std::string& title, const std::string& body, IconId icon) {
            toastManager_.showToast(title, body, icon);
        });

        settings->setVolumeCallback([this](int32_t vol, bool muted) {
            mediaHud_.showVolume(vol, muted);
        });

        settings->setTimeFormatCallback([this](bool is24H) {
            taskbar_.tray().setTimeOverride(is24H ? "14:30" : "02:30 PM");
            toastManager_.showToast("Time Format", is24H ? "24-Hour Time Enabled" : "12-Hour AM/PM Enabled", IconId::Clock);
        });

        settings->setLaunchAppCallback([this](const std::string& appId) {
            if (appId == "taskmgr") {
                openTaskManagerWindow();
            } else if (appId == "cmd" || appId == "terminal") {
                openTerminalWindow();
            } else if (appId == "regedit" || appId == "registry") {
                openRegistryEditorWindow();
                toastManager_.showToast("Registry Editor", "Launched Sovereign Registry Editor (regedit.exe)", IconId::Registry);
            }
        });

        win->content = settings;
        settings->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openCalculatorWindow() {
    const uint32_t winId = windowManager_.createWindow("Calculator", Rect{460, 140, 340, 480}, "[CALC]", IconId::Calculator);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto calc = std::make_shared<CalculatorContent>();
        win->content = calc;
        calc->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openRunDialogWindow() {
    const int32_t dlgW = 440;
    const int32_t dlgH = 190;
    const int32_t dlgX = 40;
    const int32_t dlgY = static_cast<int32_t>(height_) - taskbar_.bounds().height - dlgH - 20;

    const uint32_t winId = windowManager_.createWindow("Run", Rect{dlgX, dlgY, dlgW, dlgH}, "[RUN]", IconId::RunDialog);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto runDlg = std::make_shared<RunDialogContent>("cmd");

        runDlg->setExecuteCallback([this, winId](const std::string& cmd) {
            std::string lowerCmd = cmd;
            std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (lowerCmd == "taskmgr" || lowerCmd == "taskmgr.exe") {
                openTaskManagerWindow();
            } else if (lowerCmd == "control" || lowerCmd == "settings" || lowerCmd == "control.exe") {
                openSettingsWindow();
            } else if (lowerCmd == "calc" || lowerCmd == "calculator" || lowerCmd == "calc.exe") {
                openCalculatorWindow();
            } else if (lowerCmd == "explorer" || lowerCmd == "explorer.exe") {
                openFileExplorerWindow("C:\\Users\\admin");
            } else if (lowerCmd == "notepad" || lowerCmd == "notepad.exe") {
                openTextEditorWindow("");
            } else if (lowerCmd == "cmd" || lowerCmd == "terminal" || lowerCmd == "cmd.exe") {
                openTerminalWindow("C:\\Users\\admin");
            } else if (lowerCmd == "regedit" || lowerCmd == "regedit.exe" || lowerCmd == "registry") {
                openRegistryEditorWindow();
            } else if (lowerCmd == "photos" || lowerCmd == "photos.exe" || lowerCmd == "image") {
                openImageViewerWindow("");
            } else if (lowerCmd == "paint" || lowerCmd == "mspaint" || lowerCmd == "mspaint.exe" || lowerCmd == "pbrush" || lowerCmd == "draw") {
                openPaintWindow("");
            } else if (lowerCmd == "sysinfo" || lowerCmd == "msinfo32" || lowerCmd == "msinfo32.exe" || lowerCmd == "systeminfo") {
                openSystemInfoWindow();
            } else if (lowerCmd == "devmgmt" || lowerCmd == "devmgmt.msc" || lowerCmd == "devices" || lowerCmd == "devicemanager") {
                openDeviceManagerWindow();
            } else if (lowerCmd == "diskmgmt" || lowerCmd == "diskmgmt.msc" || lowerCmd == "diskmanagement" || lowerCmd == "partitions" || lowerCmd == "disks") {
                openDiskManagementWindow();
            } else if (lowerCmd == "services" || lowerCmd == "services.msc" || lowerCmd == "service") {
                openServicesWindow();
            } else if (lowerCmd == "eventvwr" || lowerCmd == "eventvwr.msc" || lowerCmd == "eventlog" || lowerCmd == "events") {
                openEventViewerWindow();
            } else if (lowerCmd == "winget" || lowerCmd == "store" || lowerCmd == "app_hub" || lowerCmd == "hub") {
                openAppHubWindow();
            } else {
                kernelBridge_.spawnProcess(cmd, "");
            }

            toastManager_.showToast("Run Command", "Dispatched: " + cmd, IconId::RunDialog, Color::fromHex(0x00FF9D));
            windowManager_.closeWindow(winId);
        });

        runDlg->setCloseCallback([this, winId]() {
            windowManager_.closeWindow(winId);
        });

        runDlg->setBrowseCallback([this]() {
            openFileExplorerWindow("C:\\Windows\\System32");
        });

        win->content = runDlg;
        runDlg->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openRegistryEditorWindow(std::string initialKey) {
    const uint32_t winId = windowManager_.createWindow("Registry Editor", Rect{220, 60, 920, 580}, "[REG]", IconId::Registry);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto regEdit = std::make_shared<RegistryEditorContent>(initialKey);

        regEdit->setValueModifiedCallback([this](const std::string& keyPath, const std::string& valName) {
            toastManager_.showToast("Registry Modified", keyPath + "\\" + valName, IconId::Registry, Color::fromHex(0x00FF9D));
        });

        regEdit->setCloseCallback([this, winId]() {
            windowManager_.closeWindow(winId);
        });

        win->content = regEdit;
        regEdit->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openImageViewerWindow(std::string imagePath) {
    std::string title = "Photos";
    if (!imagePath.empty()) {
        const size_t slash = imagePath.find_last_of("\\/");
        title += " - [" + (slash != std::string::npos ? imagePath.substr(slash + 1) : imagePath) + "]";
    }
    const uint32_t winId = windowManager_.createWindow(title, Rect{260, 90, 840, 560}, "[P]", IconId::ImageViewer);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto viewer = std::make_shared<ImageViewerContent>(imagePath);
        win->content = viewer;
        viewer->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openPaintWindow(std::string filePath) {
    std::string title = "Sovereign Paint";
    if (!filePath.empty()) {
        const size_t slash = filePath.find_last_of("\\/");
        title = (slash != std::string::npos ? filePath.substr(slash + 1) : filePath) + " - Sovereign Paint";
    }
    const uint32_t winId = windowManager_.createWindow(title, Rect{240, 80, 860, 580}, "[PNT]", IconId::Paint);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto paint = std::make_shared<PaintContent>(filePath);

        paint->setTitleChangedCallback([this, winId](const std::string& newTitle) {
            auto* w = windowManager_.findWindow(winId);
            if (w) {
                w->title = newTitle;
                taskbar_.addOrUpdateTask(winId, w->title, w->iconGlyph, w->isActive, w->state == WindowState::Minimized, w->iconId);
            }
        });

        paint->setFileSavedCallback([this](const std::string& path, uint64_t bytes) {
            const size_t slash = path.find_last_of("\\/");
            const std::string name = (slash != std::string::npos ? path.substr(slash + 1) : path);
            toastManager_.showToast("Canvas Saved", name + " (" + std::to_string(bytes) + " bytes exported)", IconId::Save, Color::fromHex(0x00FF9D));
        });

        win->content = paint;
        paint->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openSystemInfoWindow() {
    const uint32_t winId = windowManager_.createWindow("System Information", Rect{200, 70, 940, 600}, "[SYS]", IconId::SystemInfo);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto sysInfo = std::make_shared<SysInfoContent>();
        sysInfo->setToastCallback([this](const std::string& title, const std::string& msg, IconId icon) {
            toastManager_.showToast(title, msg, icon);
        });
        win->content = sysInfo;
        sysInfo->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openDeviceManagerWindow() {
    const uint32_t winId = windowManager_.createWindow("Device Manager", Rect{220, 80, 860, 580}, "[DEV]", IconId::DeviceManager);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto devMgr = std::make_shared<DeviceManagerContent>();
        devMgr->setToastCallback([this](const std::string& title, const std::string& msg, IconId icon) {
            toastManager_.showToast(title, msg, icon);
        });
        win->content = devMgr;
        devMgr->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openDiskManagementWindow() {
    const uint32_t winId = windowManager_.createWindow("Disk Management", Rect{200, 60, 920, 620}, "[DSK]", IconId::DiskManagement);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto diskMgr = std::make_shared<DiskManagementContent>();
        diskMgr->setToastCallback([this](const std::string& title, const std::string& msg, IconId icon) {
            toastManager_.showToast(title, msg, icon);
        });
        win->content = diskMgr;
        diskMgr->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openServicesWindow() {
    const uint32_t winId = windowManager_.createWindow("Services", Rect{210, 70, 940, 620}, "[SVC]", IconId::Services);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto servicesMgr = std::make_shared<ServicesContent>();
        servicesMgr->setToastCallback([this](const std::string& title, const std::string& msg, IconId icon) {
            toastManager_.showToast(title, msg, icon);
        });
        win->content = servicesMgr;
        servicesMgr->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openEventViewerWindow(std::string initialLog) {
    const uint32_t winId = windowManager_.createWindow("Event Viewer", Rect{160, 50, 1020, 640}, "[LOG]", IconId::EventViewer);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto eventViewer = std::make_shared<EventViewerContent>(initialLog);
        eventViewer->setToastCallback([this](const std::string& title, const std::string& msg, IconId icon) {
            toastManager_.showToast(title, msg, icon);
        });
        win->content = eventViewer;
        eventViewer->render(win->clientSurface);
    }
    return winId;
}

uint32_t SurShellDesktop::openAppHubWindow(std::string initialQuery) {
    const uint32_t winId = windowManager_.createWindow("Sovereign App Hub (winget)", Rect{160, 45, 960, 640}, "[HUB]", IconId::AppHub);
    virtualDesktops_.assignWindowToDesktop(winId, virtualDesktops_.activeIndex());
    auto* win = windowManager_.findWindow(winId);
    if (win) {
        auto hub = std::make_shared<AppHubContent>();
        if (!initialQuery.empty()) {
            hub->setSearchQuery(initialQuery);
        }
        hub->setInstallCallback([this](const std::string& title, const std::string& msg, bool success) {
            toastManager_.showToast(title, msg, success ? IconId::AppHub : IconId::NotificationBell);
        });
        win->content = hub;
        hub->render(win->clientSurface);
    }
    return winId;
}

void SurShellDesktop::openSearchHub() {
    searchHub_.toggle();
    if (searchHub_.isVisible()) {
        startMenu_.close();
        quickSettings_.close();
        actionCenter_.hide();
        virtualDesktops_.hideSwitcher();
    }
}

void SurShellDesktop::openActionCenter() {
    actionCenter_.toggle();
    if (actionCenter_.isVisible()) {
        startMenu_.close();
        quickSettings_.close();
        searchHub_.hide();
        virtualDesktops_.hideSwitcher();
    }
}

void SurShellDesktop::lockSession() {
    lockScreen_.lock();
    startMenu_.close();
    quickSettings_.close();
    searchHub_.hide();
    actionCenter_.hide();
    virtualDesktops_.hideSwitcher();
}

void SurShellDesktop::wireSubsystemCallbacks() {
    // 1. Taskbar Start Button clicks toggle the Start Menu
    taskbar_.setStartButtonClickCallback([this]() {
        startMenu_.toggle();
        if (startMenu_.isOpen()) {
            quickSettings_.close();
            actionCenter_.hide();
            searchHub_.hide();
            virtualDesktops_.hideSwitcher();
        }
    });

    // 1b. Taskbar Search Button clicks toggle Universal Search Hub
    taskbar_.setSearchButtonClickCallback([this]() {
        openSearchHub();
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
        if (id == "clock") {
            openActionCenter();
        } else if (id == "network") {
            const auto net = KernelBridge::queryPrimaryNetworkAdapter();
            toastManager_.showToast("Network Telemetry", "Ethernet Connected: " + net.linkSpeed + " | IPv4: " + net.ipv4Address, IconId::NetworkEthernet, Color::fromHex(0x00FF9D));
        } else if (id == "security") {
            toastManager_.showToast("SentinelSec Security", "Zero-Telemetry Protection Active | System Enclave Secure", IconId::SentinelSec, Color::fromHex(0x00D4FF));
        }
    });

    // 3b2. Universal Search Hub Execute callback
    searchHub_.setExecuteCallback([this](const std::string& targetApp, const std::string& args, bool asAdmin) {
        if (asAdmin) {
            toastManager_.showToast("Sovereign Elevation", "Elevated to Administrator: " + targetApp, IconId::ShieldAdmin, Color::fromHex(0xFFB703));
        }
        if (targetApp == "calc") openCalculatorWindow();
        else if (targetApp == "cmd") openTerminalWindow(args.empty() ? "C:\\Windows\\System32" : args);
        else if (targetApp == "explorer") openFileExplorerWindow(args.empty() ? "C:\\Users\\admin" : args);
        else if (targetApp == "taskmgr") openTaskManagerWindow();
        else if (targetApp == "settings") openSettingsWindow();
        else if (targetApp == "run") openRunDialogWindow();
        else if (targetApp == "editor") openTextEditorWindow(args);
        else if (targetApp == "regedit" || targetApp == "registry") openRegistryEditorWindow();
        else if (targetApp == "photos" || targetApp == "image" || targetApp == "viewer") openImageViewerWindow(args);
        else if (targetApp == "paint" || targetApp == "mspaint" || targetApp == "draw" || targetApp == "canvas") openPaintWindow(args);
        else if (targetApp == "sysinfo" || targetApp == "msinfo32" || targetApp == "systeminfo") openSystemInfoWindow();
        else if (targetApp == "devmgmt" || targetApp == "devmgmt.msc" || targetApp == "devices" || targetApp == "devicemanager") openDeviceManagerWindow();
        else if (targetApp == "diskmgmt" || targetApp == "diskmgmt.msc" || targetApp == "diskmanagement" || targetApp == "partitions" || targetApp == "disks") openDiskManagementWindow();
        else if (targetApp == "services" || targetApp == "services.msc" || targetApp == "service") openServicesWindow();
        else if (targetApp == "eventvwr" || targetApp == "eventvwr.msc" || targetApp == "eventlog" || targetApp == "events") openEventViewerWindow();
        else if (targetApp == "winget" || targetApp == "store" || targetApp == "app_hub" || targetApp == "hub" || targetApp == "market") openAppHubWindow(args);
        else kernelBridge_.spawnProcess(targetApp, args);
    });

    // 3b3. Lock Screen Power Action callback
    lockScreen_.setPowerCallback([this](const std::string& action) {
        if (action == "sleep") {
            kernelBridge_.setPowerState("Standby (S3)");
            toastManager_.showToast("Power Management", "System entering Sovereign S3 Sleep", IconId::Sleep);
        } else if (action == "restart") {
            kernelBridge_.setPowerState("Reboot (S5)");
            toastManager_.showToast("Power Management", "Kernel restarting...", IconId::Restart);
        } else if (action == "shutdown") {
            kernelBridge_.setPowerState("Shutdown (G2/S5)");
            toastManager_.showToast("Power Management", "System shutting down cleanly", IconId::Power);
        }
    });

    // 3c. Quick Settings Controls callbacks
    quickSettings_.setVolumeCallback([this](int32_t vol) {
        taskbar_.tray().setVolumeLevel(vol);
        mediaHud_.showVolume(vol);
#if defined(_WIN32)
        const DWORD winVol = static_cast<DWORD>((vol * 0xFFFF) / 100);
        const DWORD stereoVol = winVol | (winVol << 16);
        waveOutSetVolume(0, stereoVol);
#endif
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

    quickSettings_.setSettingsClickCallback([this]() {
        openSettingsWindow();
    });

    // 3d. Media Transport callbacks
    mediaHud_.setPlayPauseCallback([this]() {
        toastManager_.showToast("Media Playback", mediaHud_.isPlaying() ? "Resumed: Symphony in C++23" : "Playback Paused", IconId::MediaPlay);
    });
    mediaHud_.setNextCallback([this]() {
        toastManager_.showToast("Track Changed", "Next: Barrer Kernel Suite Mov. 2", IconId::MediaNext);
    });
    mediaHud_.setPrevCallback([this]() {
        toastManager_.showToast("Track Changed", "Previous: Mica NT Overture", IconId::MediaPrev);
    });

    // 4. Taskbar Task View button toggles Virtual Desktops switcher strip
    taskbar_.setTaskViewClickCallback([this]() {
        virtualDesktops_.toggleSwitcher();
        if (virtualDesktops_.isSwitcherVisible()) {
            virtualDesktops_.updateLayout(width_, height_, taskbar_.bounds().height);
            startMenu_.close();
            quickSettings_.close();
            actionCenter_.hide();
            searchHub_.hide();
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
        if (icon.id == "this_pc") {
            openFileExplorerWindow("This PC");
        } else if (icon.executable == "C:\\Windows\\explorer.exe" || icon.id == "explorer") {
            openFileExplorerWindow("C:\\Users\\admin");
        } else if (icon.executable == "C:\\Windows\\System32\\cmd.exe" || icon.id == "cmd") {
            openTerminalWindow("C:\\Users\\admin");
        } else if (icon.executable == "C:\\Windows\\System32\\control.exe" || icon.id == "settings") {
            openSettingsWindow();
        } else if (icon.executable == "C:\\Windows\\System32\\calc.exe" || icon.id == "calc") {
            openCalculatorWindow();
        } else if (icon.executable == "C:\\Windows\\System32\\photos.exe" || icon.id == "photos") {
            openImageViewerWindow();
        } else if (icon.executable == "C:\\Windows\\System32\\mspaint.exe" || icon.id == "paint" || icon.id == "mspaint") {
            openPaintWindow();
        } else if (icon.executable == "C:\\Windows\\System32\\msinfo32.exe" || icon.id == "sysinfo") {
            openSystemInfoWindow();
        } else if (icon.executable == "C:\\Windows\\System32\\devmgmt.msc" || icon.id == "devmgmt" || icon.id == "devices") {
            openDeviceManagerWindow();
        } else if (icon.executable == "C:\\Windows\\System32\\diskmgmt.msc" || icon.id == "diskmgmt" || icon.id == "disks" || icon.id == "diskmanagement") {
            openDiskManagementWindow();
        } else {
            const bool spawned = kernelBridge_.spawnProcess(icon.executable, icon.arguments).has_value();
            toastManager_.showToast("Launched", icon.label, icon.iconId);
            if (!spawned) {
                const uint32_t wid = windowManager_.createWindow(icon.label + " - [" + icon.executable + "]", Rect{200, 150, 640, 400}, icon.iconGlyph, icon.iconId);
                virtualDesktops_.assignWindowToDesktop(wid, virtualDesktops_.activeIndex());
            }
        }
    });

    // 7b. Desktop Context Menu actions (Right-click wallpaper & icons)
    desktop_.setContextMenuActionCallback([this](const std::string& actionId, const std::string& targetIconId) {
        if (actionId == "refresh") {
            desktop_.discoverHostDesktop();
            toastManager_.showToast("Desktop Refreshed", "Discovered host shortcuts and documents", IconId::NavRefresh);
        } else if (actionId == "sort_name") {
            toastManager_.showToast("Desktop Arranged", "Sorted icons by name", IconId::SortAsc);
        } else if (actionId == "terminal") {
            openTerminalWindow("C:\\Users\\admin\\Desktop");
        } else if (actionId == "display" || actionId == "personalize") {
            openSettingsWindow();
        } else if (actionId == "new_folder") {
            std::string userProf = "C:\\Users\\admin";
            if (const char* p = std::getenv("USERPROFILE"); p && p[0] != '\0') userProf = p;
            std::filesystem::path dt = std::filesystem::path(userProf) / "Desktop" / "New Folder";
            std::error_code ec;
            int c = 2;
            while (std::filesystem::exists(dt, ec)) {
                dt = std::filesystem::path(userProf) / "Desktop" / ("New Folder (" + std::to_string(c++) + ")");
            }
            if (std::filesystem::create_directory(dt, ec)) {
                desktop_.discoverHostDesktop();
                toastManager_.showToast("Folder Created", dt.filename().string() + " on Desktop", IconId::Folder);
            }
        } else if (actionId == "new_file") {
            std::string userProf = "C:\\Users\\admin";
            if (const char* p = std::getenv("USERPROFILE"); p && p[0] != '\0') userProf = p;
            std::filesystem::path dt = std::filesystem::path(userProf) / "Desktop" / "New Text Document.txt";
            std::error_code ec;
            int c = 2;
            while (std::filesystem::exists(dt, ec)) {
                dt = std::filesystem::path(userProf) / "Desktop" / ("New Text Document (" + std::to_string(c++) + ").txt");
            }
            std::ofstream ofs(dt);
            if (ofs.is_open()) {
                ofs.close();
                desktop_.discoverHostDesktop();
                toastManager_.showToast("File Created", dt.filename().string() + " on Desktop", IconId::NewFile);
            }
        } else if (actionId == "open") {
            for (const auto& icon : desktop_.icons()) {
                if (icon.id == targetIconId) {
                    if (icon.id == "this_pc") openFileExplorerWindow("This PC");
                    else if (icon.id == "explorer") openFileExplorerWindow("C:\\Users\\admin");
                    else if (icon.id == "cmd") openTerminalWindow("C:\\Users\\admin");
                    else if (icon.id == "settings") openSettingsWindow();
                    else if (icon.id == "calc") openCalculatorWindow();
                    else if (icon.id == "diskmgmt") openDiskManagementWindow();
                    else kernelBridge_.spawnProcess(icon.executable, icon.arguments);
                    break;
                }
            }
        } else if (actionId == "delete") {
            for (const auto& icon : desktop_.icons()) {
                if (icon.id == targetIconId) {
                    bool deleted = false;
#if defined(_WIN32)
                    std::wstring wpath;
                    int wlen = MultiByteToWideChar(CP_UTF8, 0, icon.executable.c_str(), -1, nullptr, 0);
                    if (wlen > 0) {
                        wpath.resize(wlen);
                        MultiByteToWideChar(CP_UTF8, 0, icon.executable.c_str(), -1, wpath.data(), wlen);
                        wpath.push_back(L'\0');
                        SHFILEOPSTRUCTW op{};
                        op.wFunc = FO_DELETE;
                        op.pFrom = wpath.c_str();
                        op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
                        if (SHFileOperationW(&op) == 0 && !op.fAnyOperationsAborted) {
                            deleted = true;
                        }
                    }
#endif
                    if (!deleted) {
                        std::error_code ec;
                        deleted = (std::filesystem::remove_all(icon.executable, ec) > 0);
                    }
                    if (deleted) {
                        desktop_.removeIcon(targetIconId);
                        toastManager_.showToast("Recycle Bin", icon.label + " sent to Recycle Bin", IconId::Delete);
                    }
                    break;
                }
            }
        }
    });

    // 8. Start Menu App click launches window & registers with kernel
    startMenu_.setLaunchCallback([this](const ShellAppEntry& app) {
        if (app.executablePath == "C:\\Windows\\explorer.exe" || app.id == "explorer") {
            openFileExplorerWindow("C:\\Users\\admin");
        } else if (app.executablePath == "C:\\Windows\\System32\\cmd.exe" || app.id == "cmd") {
            openTerminalWindow("C:\\Users\\admin");
        } else if (app.executablePath == "C:\\Windows\\notepad.exe" || app.id == "notepad") {
            openTextEditorWindow("");
        } else if (app.executablePath == "C:\\Windows\\System32\\taskmgr.exe" || app.id == "taskmgr") {
            openTaskManagerWindow();
        } else if (app.executablePath == "C:\\Windows\\System32\\control.exe" || app.id == "settings") {
            openSettingsWindow();
        } else if (app.executablePath == "C:\\Windows\\System32\\calc.exe" || app.id == "calc") {
            openCalculatorWindow();
        } else if (app.executablePath == "C:\\Windows\\System32\\run.exe" || app.id == "run") {
            openRunDialogWindow();
        } else if (app.executablePath == "C:\\Windows\\System32\\regedit.exe" || app.id == "regedit") {
            openRegistryEditorWindow();
        } else if (app.executablePath == "C:\\Windows\\System32\\photos.exe" || app.id == "photos") {
            openImageViewerWindow();
        } else if (app.executablePath == "C:\\Windows\\System32\\mspaint.exe" || app.id == "paint" || app.id == "mspaint") {
            openPaintWindow();
        } else if (app.executablePath == "C:\\Windows\\System32\\msinfo32.exe" || app.id == "sysinfo" || app.id == "msinfo32") {
            openSystemInfoWindow();
        } else if (app.executablePath == "C:\\Windows\\System32\\devmgmt.msc" || app.id == "devmgmt" || app.id == "devmgmt.msc") {
            openDeviceManagerWindow();
        } else if (app.executablePath == "C:\\Windows\\System32\\diskmgmt.msc" || app.id == "diskmgmt" || app.id == "diskmgmt.msc") {
            openDiskManagementWindow();
        } else if (app.executablePath == "C:\\Windows\\System32\\services.msc" || app.id == "services" || app.id == "services.msc") {
            openServicesWindow();
        } else if (app.executablePath == "C:\\Windows\\System32\\eventvwr.msc" || app.id == "eventvwr" || app.id == "eventvwr.msc" || app.id == "eventlog") {
            openEventViewerWindow();
        } else if (app.executablePath == "C:\\Windows\\System32\\winget.exe" || app.id == "app_hub" || app.id == "winget" || app.id == "store") {
            openAppHubWindow();
        } else {
            const bool spawned = kernelBridge_.spawnProcess(app.executablePath, app.arguments).has_value();
            toastManager_.showToast("Launched Application", app.title, IconRenderer::iconForAppId(app.id));
            if (!spawned) {
                const uint32_t wid = windowManager_.createWindow(app.title, Rect{240, 180, 660, 420}, app.iconGlyph, IconRenderer::iconForAppId(app.id));
                virtualDesktops_.assignWindowToDesktop(wid, virtualDesktops_.activeIndex());
            }
        }
    });

    // 9. Start Menu Power Option callback
    startMenu_.setPowerCallback([this](PowerAction action) {
        switch (action) {
            case PowerAction::Lock:
                lockSession();
                break;
            case PowerAction::Sleep:
                kernelBridge_.setPowerState("Standby (S3)");
                toastManager_.showToast("Power Management", "System entering Sovereign S3 Sleep", IconId::Sleep);
                break;
            case PowerAction::Hibernate:
                kernelBridge_.setPowerState("Hibernate (S4)");
                toastManager_.showToast("Power Management", "System hibernating...", IconId::Hibernate);
                break;
            case PowerAction::Restart:
                kernelBridge_.setPowerState("Reboot (S5)");
                toastManager_.showToast("Power Management", "Kernel restarting...", IconId::Restart);
                break;
            case PowerAction::ShutDown:
                kernelBridge_.setPowerState("Shutdown (G2/S5)");
                toastManager_.showToast("Power Management", "System shutting down cleanly", IconId::Power);
                break;
            case PowerAction::SignOut:
                lockSession();
                toastManager_.showToast("Session", "User signed out", IconId::SignOut);
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

void SurShellDesktop::toggleShowDesktop() {
    bool anyNonMinimized = false;
    for (const auto& win : windowManager_.windows()) {
        if (win->state != WindowState::Minimized) {
            anyNonMinimized = true;
            break;
        }
    }
    if (anyNonMinimized) {
        for (const auto& win : windowManager_.windows()) {
            if (win->state != WindowState::Minimized) {
                windowManager_.setWindowState(win->id, WindowState::Minimized);
            }
        }
        toastManager_.showToast("Show Desktop", "All windows minimized (Win+D)", IconId::Desktop);
    } else {
        for (const auto& win : windowManager_.windows()) {
            if (win->state == WindowState::Minimized) {
                windowManager_.setWindowState(win->id, WindowState::Normal);
            }
        }
        toastManager_.showToast("Show Desktop", "All windows restored (Win+D)", IconId::Desktop);
    }
}

void SurShellDesktop::onMouseDown(Point pt, MouseButton button) {
    currentMousePos_ = pt;

    // -1. Lock screen takes complete precedence when active
    if (lockScreen_.isLocked()) {
        lockScreen_.onMouseDown(pt, button);
        return;
    }

    // 0. Desktop Context Menu hit testing
    if (desktop_.contextMenu().isOpen) {
        desktop_.onMouseDown(pt, button);
        return;
    }

    // 0a. Search Hub takes precedence when open
    if (searchHub_.isVisible()) {
        if (searchHub_.onMouseDown(pt, button)) {
            return;
        }
    }

    // 0b. Action Center & Calendar Flyout takes precedence when open
    if (actionCenter_.isVisible()) {
        if (actionCenter_.onMouseDown(pt, button)) {
            return;
        }
    }

    // 0c. Toast notifications hit testing
    if (toastManager_.onMouseDown(pt, button)) {
        return;
    }

    // 0d. Media HUD hit testing
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
    if (virtualDesktops_.isSwitcherVisible()) {
        if (virtualDesktops_.onMouseUp(pt, button)) {
            return;
        }
    }
    quickSettings_.onMouseUp(pt, button);
    windowManager_.onMouseUp(pt, button);
    desktop_.onMouseUp(pt, button);
}

void SurShellDesktop::onMouseMove(Point pt) {
    currentMousePos_ = pt;

    if (lockScreen_.isLocked()) {
        lockScreen_.onMouseMove(pt);
        return;
    }

    if (searchHub_.isVisible()) {
        searchHub_.onMouseMove(pt);
    }

    if (actionCenter_.isVisible()) {
        actionCenter_.onMouseMove(pt);
    }

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

    if (desktop_.contextMenu().isOpen) {
        desktop_.onMouseMove(pt);
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
    if (lockScreen_.isLocked()) {
        lockScreen_.onCharInput(c);
        return;
    }

    if (searchHub_.isVisible()) {
        searchHub_.onCharInput(c);
        return;
    }

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

void SurShellDesktop::onKeyDown(KeyCode key, bool ctrl, bool shift, bool alt, bool win) {
    if (lockScreen_.isLocked()) {
        lockScreen_.onKeyDown(key);
        return;
    }

    if (searchHub_.isVisible()) {
        if (searchHub_.onKeyDown(key)) {
            return;
        }
    }

    if (actionCenter_.isVisible()) {
        if (key == KeyCode::Escape) {
            actionCenter_.hide();
            return;
        }
    }

    if (desktop_.contextMenu().isOpen) {
        if (key == KeyCode::Escape) {
            desktop_.closeContextMenu();
            return;
        }
    }

    if (virtualDesktops_.isSwitcherVisible()) {
        if (key == KeyCode::Escape) {
            virtualDesktops_.hideSwitcher();
            return;
        }
        if (key == KeyCode::Left) {
            virtualDesktops_.previousDesktop();
            return;
        }
        if (key == KeyCode::Right) {
            virtualDesktops_.nextDesktop();
            return;
        }
    }

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

    // Win + Tab: Toggle Task View
    if (win && key == KeyCode::Tab) {
        virtualDesktops_.toggleSwitcher();
        if (virtualDesktops_.isSwitcherVisible()) {
            virtualDesktops_.updateLayout(width_, height_, taskbar_.bounds().height);
            startMenu_.close();
            quickSettings_.close();
            actionCenter_.hide();
            searchHub_.hide();
        }
        return;
    }

    // Alt + Tab: Cycle Application Windows
    if (alt && key == KeyCode::Tab) {
        if (!altTab_.isActive()) {
            triggerAltTab();
        } else {
            cycleAltTab(!shift);
        }
        return;
    }

    // Ctrl + Win + Left / Right: Switch Virtual Desktops
    if (ctrl && win && key == KeyCode::Left) {
        virtualDesktops_.previousDesktop();
        toastManager_.showToast("Virtual Desktop", virtualDesktops_.activeDesktop().name, IconId::TaskView);
        return;
    }

    if (ctrl && win && key == KeyCode::Right) {
        virtualDesktops_.nextDesktop();
        toastManager_.showToast("Virtual Desktop", virtualDesktops_.activeDesktop().name, IconId::TaskView);
        return;
    }

    // Ctrl + Win + D: Create New Virtual Desktop
    if (ctrl && win && key == KeyCode::KeyD) {
        virtualDesktops_.createDesktop();
        virtualDesktops_.switchDesktop(virtualDesktops_.desktopCount() - 1);
        toastManager_.showToast("Desktop Created", virtualDesktops_.activeDesktop().name, IconId::TaskView);
        return;
    }

    // Ctrl + Win + F4: Close Active Virtual Desktop
    if (ctrl && win && key == KeyCode::F4) {
        if (virtualDesktops_.desktopCount() > 1) {
            std::string closedName = virtualDesktops_.activeDesktop().name;
            virtualDesktops_.removeDesktop(virtualDesktops_.activeIndex());
            toastManager_.showToast("Desktop Closed", closedName, IconId::TaskView);
        }
        return;
    }

    if (key == KeyCode::Super) {
        startMenu_.toggle();
        if (startMenu_.isOpen()) {
            quickSettings_.close();
            actionCenter_.hide();
            searchHub_.hide();
            virtualDesktops_.hideSwitcher();
        }
        return;
    }

    if ((win || alt || ctrl) && key == KeyCode::KeyD) {
        toggleShowDesktop();
        return;
    }

    if ((win || alt || ctrl) && key == KeyCode::KeyE) {
        openFileExplorerWindow("This PC");
        return;
    }

    if ((win || alt || ctrl) && key == KeyCode::KeyS) {
        openSearchHub();
        return;
    }

    if ((win || alt || ctrl) && key == KeyCode::KeyN) {
        openActionCenter();
        return;
    }

    if ((win || alt || ctrl) && key == KeyCode::KeyL) {
        lockSession();
        return;
    }

    if ((win || alt || ctrl) && key == KeyCode::KeyR) {
        openRunDialogWindow();
        return;
    }

    if (key == KeyCode::F5) {
        desktop_.discoverHostDesktop();
        toastManager_.showToast("Desktop Refreshed", "Discovered host shortcuts and documents", IconId::NavRefresh);
        return;
    }

    windowManager_.onKeyDown(key, ctrl, shift, alt);
}

void SurShellDesktop::render() {
    if (lockScreen_.isLocked()) {
        lockScreen_.render(framebuffer_, width_, height_);

        // Render Mouse Cursor Arrow
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
        return;
    }

    // 1. Render Desktop background and icons
    desktop_.render(framebuffer_);

    // 2. Render Windows in Z-order with drop shadows
    windowManager_.render(framebuffer_);

    // 3. Render Top Architectural Sovereign Header Bar (Optional Diagnostic HUD)
    if (showTopBar_) {
        framebuffer_.fillRect(Rect{0, 0, static_cast<int32_t>(width_), 26}, Color::fromRgba(16, 22, 34, 245));
        framebuffer_.fillRect(Rect{0, 25, static_cast<int32_t>(width_), 1}, Color::fromRgba(38, 52, 78, 200));

        framebuffer_.drawString(12, 8, "MicaNT 64-Bit OS", Color::fromHex(0x00D4FF), 1);
        framebuffer_.drawString(140, 8, "|  Barrer Software & MicaNT Community  |  Zero Telemetry  |  SurWin Subsystem  |  120Hz VSync",
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

    // 11. Render Action Center & Calendar Flyout (if visible)
    if (actionCenter_.isVisible()) {
        actionCenter_.render(framebuffer_, width_, height_, taskbar_.bounds().height);
    }

    // 12. Render Universal Search Hub (if visible)
    if (searchHub_.isVisible()) {
        searchHub_.render(framebuffer_, width_, height_);
    }

    // 12b. Render Desktop Context Menu (if open)
    if (desktop_.contextMenu().isOpen) {
        desktop_.renderContextMenu(framebuffer_);
    }

    // 13. Render Mouse Cursor Arrow
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
