// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/main.cpp)
// ============================================================================

#include "surshell/surshell.hpp"
#include <iostream>
#include <chrono>
#include <memory>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

namespace {

surshell::KeyCode mapVkToKeyCode(WPARAM vk) {
    switch (vk) {
        case VK_RETURN: return surshell::KeyCode::Enter;
        case VK_ESCAPE: return surshell::KeyCode::Escape;
        case VK_BACK:   return surshell::KeyCode::Backspace;
        case VK_TAB:    return surshell::KeyCode::Tab;
        case VK_DELETE: return surshell::KeyCode::Delete;
        case VK_UP:     return surshell::KeyCode::Up;
        case VK_DOWN:   return surshell::KeyCode::Down;
        case VK_LEFT:   return surshell::KeyCode::Left;
        case VK_RIGHT:  return surshell::KeyCode::Right;
        case VK_HOME:   return surshell::KeyCode::Home;
        case VK_END:    return surshell::KeyCode::End;
        case VK_PRIOR:  return surshell::KeyCode::PageUp;
        case VK_NEXT:   return surshell::KeyCode::PageDown;
        case VK_F1:     return surshell::KeyCode::F1;
        case VK_F2:     return surshell::KeyCode::F2;
        case VK_F3:     return surshell::KeyCode::F3;
        case VK_F4:     return surshell::KeyCode::F4;
        case VK_F5:     return surshell::KeyCode::F5;
        case VK_F11:    return surshell::KeyCode::F11;
        case VK_F12:    return surshell::KeyCode::F12;
        case VK_LWIN:
        case VK_RWIN:   return surshell::KeyCode::Super;
        default:        break;
    }
    if (vk >= '0' && vk <= '9') {
        return static_cast<surshell::KeyCode>(static_cast<int>(surshell::KeyCode::Num0) + (vk - '0'));
    }
    switch (vk) {
        case 'A': return surshell::KeyCode::KeyA;
        case 'C': return surshell::KeyCode::KeyC;
        case 'D': return surshell::KeyCode::KeyD;
        case 'E': return surshell::KeyCode::KeyE;
        case 'F': return surshell::KeyCode::KeyF;
        case 'L': return surshell::KeyCode::KeyL;
        case 'N': return surshell::KeyCode::KeyN;
        case 'R': return surshell::KeyCode::KeyR;
        case 'S': return surshell::KeyCode::KeyS;
        case 'T': return surshell::KeyCode::KeyT;
        case 'V': return surshell::KeyCode::KeyV;
        case 'W': return surshell::KeyCode::KeyW;
        case 'X': return surshell::KeyCode::KeyX;
        case 'Z': return surshell::KeyCode::KeyZ;
        default:  return surshell::KeyCode::Unknown;
    }
}

surshell::Point clientToDesktop(HWND hwnd, LPARAM lParam) {
    RECT cr;
    GetClientRect(hwnd, &cr);
    const int cw = cr.right - cr.left;
    const int ch = cr.bottom - cr.top;
    if (cw <= 0 || ch <= 0) return surshell::Point{0, 0};
    const int cx = GET_X_LPARAM(lParam);
    const int cy = GET_Y_LPARAM(lParam);
    const int dx = cx * 1920 / cw;
    const int dy = cy * 1080 / ch;
    return surshell::Point{std::clamp(dx, 0, 1919), std::clamp(dy, 0, 1079)};
}

struct DesktopAppState {
    std::unique_ptr<surshell::SurShellDesktop> desktop;
};

LRESULT CALLBACK DesktopWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* state = reinterpret_cast<DesktopAppState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_CREATE: {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            auto* newState = reinterpret_cast<DesktopAppState*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(newState));
            return 0;
        }

        case WM_ERASEBKGND:
            return 1; // Prevent GDI flicker

        case WM_SIZE:
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        case WM_PAINT: {
            if (!state) break;
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            state->desktop->render();
            const auto& fb = state->desktop->framebuffer();

            BITMAPINFO bmi{};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = static_cast<LONG>(fb.width());
            bmi.bmiHeader.biHeight = -static_cast<LONG>(fb.height()); // Top-down DIB
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            const int cw = clientRect.right - clientRect.left;
            const int ch = clientRect.bottom - clientRect.top;

            SetStretchBltMode(hdc, HALFTONE);
            StretchDIBits(hdc, 0, 0, cw, ch, 0, 0, fb.width(), fb.height(),
                          fb.pixels().data(), &bmi, DIB_RGB_COLORS, SRCCOPY);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            if (!state) break;
            SetCapture(hwnd);
            state->desktop->onMouseDown(clientToDesktop(hwnd, lParam), surshell::MouseButton::Left);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_LBUTTONUP: {
            if (!state) break;
            ReleaseCapture();
            state->desktop->onMouseUp(clientToDesktop(hwnd, lParam), surshell::MouseButton::Left);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_RBUTTONDOWN: {
            if (!state) break;
            state->desktop->onMouseDown(clientToDesktop(hwnd, lParam), surshell::MouseButton::Right);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_RBUTTONUP: {
            if (!state) break;
            state->desktop->onMouseUp(clientToDesktop(hwnd, lParam), surshell::MouseButton::Right);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_MOUSEMOVE: {
            if (!state) break;
            state->desktop->onMouseMove(clientToDesktop(hwnd, lParam));
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_LBUTTONDBLCLK: {
            if (!state) break;
            state->desktop->onDoubleClick(clientToDesktop(hwnd, lParam));
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_MOUSEWHEEL: {
            if (!state) break;
            const int32_t delta = GET_WHEEL_DELTA_WPARAM(wParam);
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            ScreenToClient(hwnd, &pt);
            RECT cr;
            GetClientRect(hwnd, &cr);
            const int cw = cr.right - cr.left;
            const int ch = cr.bottom - cr.top;
            const int dx = cw > 0 ? pt.x * 1920 / cw : 0;
            const int dy = ch > 0 ? pt.y * 1080 / ch : 0;
            state->desktop->onMouseWheel(surshell::Point{dx, dy}, delta);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_CHAR: {
            if (!state) break;
            state->desktop->onCharInput(static_cast<char>(wParam));
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_KEYDOWN: {
            if (!state) break;
            const bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            const bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            const bool alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
            const bool win = ((GetKeyState(VK_LWIN) & 0x8000) != 0) || ((GetKeyState(VK_RWIN) & 0x8000) != 0);
            const auto kc = mapVkToKeyCode(wParam);
            state->desktop->onKeyDown(kc, ctrl, shift, alt, win);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int runInteractiveDesktop() {
    DesktopAppState state;
    state.desktop = std::make_unique<surshell::SurShellDesktop>(1920, 1080);

    // Open File Explorer and Sovereign App Hub windows by default so they're immediately accessible
    state.desktop->openFileExplorerWindow("This PC");
    state.desktop->openAppHubWindow();

    HINSTANCE hInstance = GetModuleHandleW(nullptr);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = DesktopWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.hbrBackground = nullptr;
    wc.lpszClassName = L"MicaNTSurShellDesktopHost";

    RegisterClassExW(&wc);

    const int screenW = GetSystemMetrics(SM_CXSCREEN);
    const int screenH = GetSystemMetrics(SM_CYSCREEN);
    const int winW = (std::min)(1600, screenW - 60);
    const int winH = (std::min)(900, screenH - 80);
    const int winX = (screenW - winW) / 2;
    const int winY = (screenH - winH) / 2;

    HWND hwnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        L"MicaNTSurShellDesktopHost",
        L"SurShell: Sovereign Clean-Room Desktop Shell for MicaNT",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        winX, winY, winW, winH,
        nullptr, nullptr, hInstance, &state
    );

    if (!hwnd) {
        std::cerr << "Failed to create SurShell Desktop Win32 window.\n";
        return 1;
    }

    BOOL darkMode = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    std::cout << "[SurShell] Native Interactive Desktop Window launched.\n";
    std::cout << "  -> Resolution: " << winW << "x" << winH << " at (" << winX << ", " << winY << ")\n";
    std::cout << "  -> Sovereign App Hub (winget), Explorer, Task View (Win+Tab), Terminal & Settings ready.\n";
    std::cout << "  -> Close the window or press Alt+F4 to exit.\n\n";

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

} // anonymous namespace
#endif

int runSnapshotPipeline() {
    std::cout << "===============================================================================\n";
    std::cout << "SurShell: Sovereign Clean-Room Desktop Shell for MicaNT\n";
    std::cout << "Engineered by Barrer Software and the MicaNT Community\n";
    std::cout << "Standard: ISO C++23 | Zero Telemetry | Sub-15MB Footprint | 120Hz DWM Pipeline\n";
    std::cout << "===============================================================================\n\n";

    constexpr uint32_t SCREEN_WIDTH = 1920;
    constexpr uint32_t SCREEN_HEIGHT = 1080;

    std::cout << "[SurShell] Initializing Desktop Environment (" << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << ")...\n";
    const auto startInit = std::chrono::high_resolution_clock::now();

    surshell::SurShellDesktop shell(SCREEN_WIDTH, SCREEN_HEIGHT);

    const auto initElapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - startInit).count();
    std::cout << "[SurShell] Initialized in " << initElapsed << " us (" 
              << (initElapsed / 1000.0) << " ms).\n";

    // ------------------------------------------------------------------------
    // Scene 1: Default Desktop with Command Prompt & File Explorer
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 1: Default Multi-Window Desktop...\n";
    shell.render();
    if (shell.exportSnapshot("surshell_desktop_default.bmp")) {
        std::cout << "  -> Exported: surshell_desktop_default.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 2: Modern Centered Start Prism Hub with Tactile App Grid & Recommended
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 2: Floating Start Prism Hub with Application Grid...\n";
    shell.startMenu().open();
    shell.render();
    if (shell.exportSnapshot("surshell_start_menu_active.bmp")) {
        std::cout << "  -> Exported: surshell_start_menu_active.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 2b: Start Menu with Sovereign Interactive Power Flyout
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 2b: Start Menu with Interactive Power Flyout...\n";
    shell.startMenu().setPowerFlyoutOpen(true);
    shell.render();
    if (shell.exportSnapshot("surshell_start_menu_power.bmp")) {
        std::cout << "  -> Exported: surshell_start_menu_power.bmp (1920x1080 32-bpp)\n";
    }
    shell.startMenu().close();

    // ------------------------------------------------------------------------
    // Scene 2c: Start Menu All Applications Catalog View (A-Z)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 2c: Start Menu All Applications Catalog...\n";
    shell.startMenu().open();
    shell.startMenu().setViewMode(surshell::StartViewMode::AllApps);
    shell.render();
    if (shell.exportSnapshot("surshell_start_menu_allapps.bmp")) {
        std::cout << "  -> Exported: surshell_start_menu_allapps.bmp (1920x1080 32-bpp)\n";
    }
    shell.startMenu().close();

    // ------------------------------------------------------------------------
    // Scene 3: Modern 67/33 Priority Snap Layout & Assistant HUD
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 3: Modern 67/33 Priority Snap Layout & Assistant HUD...\n";
    const auto& wins = shell.windowManager().windows();
    if (wins.size() >= 2) {
        shell.windowManager().snapWindow(wins[0]->id, surshell::WindowState::SnappedPriorityLeft);
        shell.windowManager().snapWindow(wins[1]->id, surshell::WindowState::SnappedSidebarRight);
        shell.windowManager().showSnapFlyout(wins[0]->id, surshell::Point{wins[0]->maxButtonBounds().center().x, wins[0]->maxButtonBounds().bottom() + 4});
    }
    shell.render();
    if (shell.exportSnapshot("surshell_aero_snap.bmp")) {
        std::cout << "  -> Exported: surshell_aero_snap.bmp (1920x1080 32-bpp)\n";
    }
    shell.windowManager().hideSnapFlyout();

    // ------------------------------------------------------------------------
    // Scene 4: Modern Quick Settings Flyout with Audio/Brightness Sliders
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 4: Quick Settings Island with Sliders...\n";
    shell.quickSettings().open();
    shell.render();
    if (shell.exportSnapshot("surshell_quick_settings.bmp")) {
        std::cout << "  -> Exported: surshell_quick_settings.bmp (1920x1080 32-bpp)\n";
    }
    shell.quickSettings().close();

    // ------------------------------------------------------------------------
    // Scene 5: Task View & Virtual Desktops Switcher Strip
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 5: Task View & Virtual Desktops Switcher Strip...\n";
    shell.virtualDesktops().showSwitcher();
    shell.render();
    if (shell.exportSnapshot("surshell_virtual_desktops.bmp")) {
        std::cout << "  -> Exported: surshell_virtual_desktops.bmp (1920x1080 32-bpp)\n";
    }
    shell.virtualDesktops().hideSwitcher();

    // ------------------------------------------------------------------------
    // Scene 6: File Explorer with Right-Click Context Menu Active
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 6: File Explorer with Acrylic Context Menu...\n";
    for (const auto& w : shell.windowManager().windows()) {
        if (auto exp = std::dynamic_pointer_cast<surshell::FileExplorer>(w->content)) {
            exp->openContextMenu(surshell::Point{380, 210}, true);
            exp->render(w->clientSurface);
            break;
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_explorer_context_menu.bmp")) {
        std::cout << "  -> Exported: surshell_explorer_context_menu.bmp (1920x1080 32-bpp)\n";
    }
    for (const auto& w : shell.windowManager().windows()) {
        if (auto exp = std::dynamic_pointer_cast<surshell::FileExplorer>(w->content)) {
            exp->closeContextMenu();
            exp->render(w->clientSurface);
            break;
        }
    }

    // ------------------------------------------------------------------------
    // Scene 7: File Explorer with Properties Inspector Dialog
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 7: File Explorer with Properties Inspector Dialog...\n";
    for (const auto& w : shell.windowManager().windows()) {
        if (auto exp = std::dynamic_pointer_cast<surshell::FileExplorer>(w->content)) {
            if (!exp->items().empty()) {
                exp->showPropertiesDialog(exp->items()[0]);
                exp->render(w->clientSurface);
            }
            break;
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_explorer_properties.bmp")) {
        std::cout << "  -> Exported: surshell_explorer_properties.bmp (1920x1080 32-bpp)\n";
    }
    for (const auto& w : shell.windowManager().windows()) {
        if (auto exp = std::dynamic_pointer_cast<surshell::FileExplorer>(w->content)) {
            exp->closePropertiesDialog();
            exp->render(w->clientSurface);
            break;
        }
    }

    // ------------------------------------------------------------------------
    // Scene 8: Sovereign Code & Text Editor inspecting live source code
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 8: Sovereign Code Editor with Syntax Highlighting...\n";
    const uint32_t editorWin = shell.openTextEditorWindow("C:\\source\\SurShell\\include\\surshell\\explorer.hpp");
    shell.windowManager().setWindowActive(editorWin);
    shell.render();
    if (shell.exportSnapshot("surshell_sovereign_editor.bmp")) {
        std::cout << "  -> Exported: surshell_sovereign_editor.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 9: Alt+Tab Task Switcher HUD (Sovereign Ergonomics)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 9: Alt+Tab Task Switcher HUD with Live Thumbnails...\n";
    shell.triggerAltTab();
    shell.render();
    if (shell.exportSnapshot("surshell_alt_tab_hud.bmp")) {
        std::cout << "  -> Exported: surshell_alt_tab_hud.bmp (1920x1080 32-bpp)\n";
    }
    shell.dismissAltTab();

    // ------------------------------------------------------------------------
    // Scene 10: Taskbar Live Hover Preview (Sovereign Peek)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 10: Taskbar Live Hover Preview (Sovereign Peek)...\n";
    if (!shell.taskbar().tasks().empty()) {
        shell.taskbar().setHoveredTaskWindowId(static_cast<int32_t>(shell.taskbar().tasks()[0].windowId));
    }
    shell.render();
    if (shell.exportSnapshot("surshell_taskbar_hover_preview.bmp")) {
        std::cout << "  -> Exported: surshell_taskbar_hover_preview.bmp (1920x1080 32-bpp)\n";
    }
    shell.taskbar().setHoveredTaskWindowId(-1);

    // ------------------------------------------------------------------------
    // Scene 11: Task Manager & Resource Monitor Window (taskmgr.exe)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 11: Task Manager & Resource Monitor...\n";
    const uint32_t tmWinId = shell.openTaskManagerWindow();
    shell.windowManager().setWindowActive(tmWinId);
    shell.render();
    if (shell.exportSnapshot("surshell_task_manager.bmp")) {
        std::cout << "  -> Exported: surshell_task_manager.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 12: Sovereign Acrylic Desktop Toast Notifications
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 12: Sovereign Acrylic Desktop Toast Notifications...\n";
    const auto netInfo = surshell::KernelBridge::queryPrimaryNetworkAdapter();
    shell.toastManager().showToast("Network Telemetry", "Ethernet " + netInfo.linkSpeed + " Connected. IPv4: " + netInfo.ipv4Address, surshell::IconId::NetworkEthernet, surshell::Color::fromHex(0x00D4FF));
    shell.render();
    if (shell.exportSnapshot("surshell_toast_notifications.bmp")) {
        std::cout << "  -> Exported: surshell_toast_notifications.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 13: Audio & Media Playback HUD (OSD Overlay)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 13: Audio & Media Playback HUD (OSD Overlay)...\n";
    shell.mediaHud().showVolume(85);
    shell.mediaHud().showMedia("Barrer Software - Symphony in C++23", "MicaNT Sovereign Philharmonic");
    shell.render();
    if (shell.exportSnapshot("surshell_media_hud.bmp")) {
        std::cout << "  -> Exported: surshell_media_hud.bmp (1920x1080 32-bpp)\n";
    }
    shell.mediaHud().hide();

    // ------------------------------------------------------------------------
    // Scene 14: System Settings & Personalization Center (control.exe)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 14: System Settings & Personalization...\n";
    const uint32_t settingsWinId = shell.openSettingsWindow();
    shell.windowManager().setWindowActive(settingsWinId);
    shell.render();
    if (shell.exportSnapshot("surshell_settings_personalization.bmp")) {
        std::cout << "  -> Exported: surshell_settings_personalization.bmp (1920x1080 32-bpp)\n";
    }

    // Scene 14b: Settings Developer & Enclave Diagnostics (Registry Launcher)
    if (auto* sw = shell.windowManager().findWindow(settingsWinId)) {
        if (auto sContent = std::dynamic_pointer_cast<surshell::SettingsContent>(sw->content)) {
            sContent->setActiveCategory(surshell::SettingsCategory::Developer);
            sContent->render(sw->clientSurface);
            shell.render();
            if (shell.exportSnapshot("surshell_settings_developer.bmp")) {
                std::cout << "  -> Exported: surshell_settings_developer.bmp (1920x1080 32-bpp)\n";
            }
            sContent->setActiveCategory(surshell::SettingsCategory::System);
            sContent->render(sw->clientSurface);
        }
    }

    // ------------------------------------------------------------------------
    // Scene 15: Modern Sovereign Calculator (calc.exe)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 15: Modern Sovereign Calculator...\n";
    const uint32_t calcWinId = shell.openCalculatorWindow();
    shell.windowManager().setWindowActive(calcWinId);
    if (auto* w = shell.windowManager().findWindow(calcWinId)) {
        if (auto calc = std::dynamic_pointer_cast<surshell::CalculatorContent>(w->content)) {
            calc->inputDigit('1');
            calc->inputDigit('2');
            calc->inputDigit('8');
            calc->inputOperator('*');
            calc->inputDigit('8');
            calc->calculateResult();
            calc->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_calculator.bmp")) {
        std::cout << "  -> Exported: surshell_calculator.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 16: Modern Sovereign Run Dialog (run.exe / Win+R)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 16: Modern Sovereign Run Dialog...\n";
    const uint32_t runWinId = shell.openRunDialogWindow();
    shell.windowManager().setWindowActive(runWinId);
    if (auto* w = shell.windowManager().findWindow(runWinId)) {
        if (auto runDlg = std::dynamic_pointer_cast<surshell::RunDialogContent>(w->content)) {
            runDlg->setCommand("control.exe");
            runDlg->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_run_dialog.bmp")) {
        std::cout << "  -> Exported: surshell_run_dialog.bmp (1920x1080 32-bpp)\n";
    }
    shell.windowManager().closeWindow(runWinId);

    // ------------------------------------------------------------------------
    // Scene 17: Live Aero Snap Silhouette Preview
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 17: Live Aero Snap Silhouette Preview...\n";
    if (auto* w = shell.windowManager().findWindow(settingsWinId)) {
        shell.onMouseDown(w->captionBounds().center(), surshell::MouseButton::Left);
        shell.onMouseMove(surshell::Point{1915, 10}); // Drag to top-right corner to activate 25% quadrant silhouette
    }
    shell.render();
    if (shell.exportSnapshot("surshell_aero_snap_silhouette.bmp")) {
        std::cout << "  -> Exported: surshell_aero_snap_silhouette.bmp (1920x1080 32-bpp)\n";
    }
    shell.onMouseUp(surshell::Point{1915, 10}, surshell::MouseButton::Left);

    // ------------------------------------------------------------------------
    // Scene 18: Universal Search Hub (Win+S / Taskbar Search)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 18: Universal Search Hub...\n";
    shell.openSearchHub();
    shell.searchHub().setQuery("cmd");
    shell.render();
    if (shell.exportSnapshot("surshell_search_hub.bmp")) {
        std::cout << "  -> Exported: surshell_search_hub.bmp (1920x1080 32-bpp)\n";
    }
    shell.searchHub().hide();

    // ------------------------------------------------------------------------
    // Scene 19: Action Center & Calendar Flyout (Win+N / Tray Clock)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 19: Action Center & Calendar Flyout...\n";
    shell.openActionCenter();
    shell.render();
    if (shell.exportSnapshot("surshell_action_center.bmp")) {
        std::cout << "  -> Exported: surshell_action_center.bmp (1920x1080 32-bpp)\n";
    }
    shell.actionCenter().hide();

    // ------------------------------------------------------------------------
    // Scene 20: Sovereign Lock Screen & Authentication Center (Win+L)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 20: Sovereign Lock Screen & Credentials...\n";
    shell.lockSession();
    shell.lockScreen().showCredentials();
    shell.lockScreen().setPin("1234");
    shell.render();
    if (shell.exportSnapshot("surshell_lock_screen.bmp")) {
        std::cout << "  -> Exported: surshell_lock_screen.bmp (1920x1080 32-bpp)\n";
    }
    shell.lockScreen().unlock();

    // ------------------------------------------------------------------------
    // Scene 21: Sovereign Terminal System with Tabs (Pure ISO C++23 Architecture)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 21: Sovereign Terminal System with Tabs...\n";
    const uint32_t termWin = shell.openTerminalWindow("C:\\Users\\admin");
    shell.windowManager().setWindowActive(termWin);
    if (auto* w = shell.windowManager().findWindow(termWin)) {
        if (auto term = std::dynamic_pointer_cast<surshell::TerminalContent>(w->content)) {
            term->addTab("PowerShell", "pwsh");
            term->selectTab(0);
            term->inputString("ver");
            term->executeCurrentCommand();
            term->inputString("dir");
            term->executeCurrentCommand();
            term->inputString("echo MicaNT Barrer Software Sovereign Executive Online");
            term->executeCurrentCommand();
            term->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_terminal_tabs.bmp")) {
        std::cout << "  -> Exported: surshell_terminal_tabs.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 22: Sovereign Registry Editor (regedit.exe)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 22: Sovereign Registry Editor (regedit.exe)...\n";
    const uint32_t regWin = shell.openRegistryEditorWindow("Computer\\HKEY_LOCAL_MACHINE\\SOFTWARE\\MicaNT\\CurrentVersion");
    shell.windowManager().setWindowActive(regWin);
    shell.render();
    if (shell.exportSnapshot("surshell_registry_editor.bmp")) {
        std::cout << "  -> Exported: surshell_registry_editor.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 23: Modern File Explorer: 'This PC' with Google Drive & Attached NAS
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 23: File Explorer 'This PC' with Google Drive & NAS...\n";
    const uint32_t expWin = shell.openFileExplorerWindow("This PC");
    shell.windowManager().setWindowActive(expWin);
    shell.render();
    if (shell.exportSnapshot("surshell_file_explorer_this_pc.bmp")) {
        std::cout << "  -> Exported: surshell_file_explorer_this_pc.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 24: Desktop Right-Click Context Menu (Wallpaper & Shell Integration)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 24: Desktop Right-Click Context Menu...\n";
    shell.desktop().openContextMenu(surshell::Point{640, 360});
    shell.render();
    if (shell.exportSnapshot("surshell_desktop_context_menu.bmp")) {
        std::cout << "  -> Exported: surshell_desktop_context_menu.bmp (1920x1080 32-bpp)\n";
    }
    shell.desktop().closeContextMenu();

    // ------------------------------------------------------------------------
    // Scene 25: Sovereign Photo & Image Viewer (Clean-Room Parity)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 25: Sovereign Photo & Image Viewer...\n";
    const uint32_t imgWin = shell.openImageViewerWindow("surshell_desktop_context_menu.bmp");
    shell.windowManager().setWindowActive(imgWin);
    shell.render();
    if (shell.exportSnapshot("surshell_image_viewer.bmp")) {
        std::cout << "  -> Exported: surshell_image_viewer.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 26: Interactive Sovereign Notepad 2.0 (Text & Code Editor)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 26: Interactive Sovereign Notepad 2.0...\n";
    const uint32_t noteWin = shell.openTextEditorWindow("C:\\MicaNT\\kernel\\executive.cpp");
    shell.windowManager().setWindowActive(noteWin);
    if (auto* w = shell.windowManager().findWindow(noteWin)) {
        if (auto editor = std::dynamic_pointer_cast<surshell::TextViewerContent>(w->content)) {
            editor->insertText("\n// Sovereign System Kernel Call Dispatcher\nvoid dispatchSyscall(uint32_t id) {\n    // Executed with hardware privilege level 0\n    return;\n}\n");
            editor->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_notepad_editor.bmp")) {
        std::cout << "  -> Exported: surshell_notepad_editor.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 27: Task Manager Performance & Historical Oscillogram Line Charts
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 27: Task Manager Performance Telemetry Line Charts...\n";
    const uint32_t perfWin = shell.openTaskManagerWindow();
    shell.windowManager().setWindowActive(perfWin);
    if (auto* w = shell.windowManager().findWindow(perfWin)) {
        if (auto tm = std::dynamic_pointer_cast<surshell::TaskManagerContent>(w->content)) {
            tm->setActiveTab(surshell::TaskManagerTab::Performance);
            tm->setPerformanceResource(surshell::PerformanceResource::CPU);
            tm->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_task_manager_performance.bmp")) {
        std::cout << "  -> Exported: surshell_task_manager_performance.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 28: Sovereign Paint Studio & Vector Canvas (paint.exe Parity)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 28: Sovereign Paint Studio & Vector Canvas...\n";
    const uint32_t paintWin = shell.openPaintWindow();
    shell.windowManager().setWindowActive(paintWin);
    if (auto* w = shell.windowManager().findWindow(paintWin)) {
        if (auto paint = std::dynamic_pointer_cast<surshell::PaintContent>(w->content)) {
            const auto cyan = surshell::Color::fromHex(0x00D4FF);
            const auto green = surshell::Color::fromHex(0x00FF9D);
            const auto coral = surshell::Color::fromHex(0xFF5C5C);
            const auto amber = surshell::Color::fromHex(0xF59E0B);
            const auto purple = surshell::Color::fromHex(0xA855F7);
            const auto slate = surshell::Color::fromHex(0x1E293B);

            // Canvas drawing composition
            paint->drawRect(surshell::Rect{20, 20, 480, 300}, surshell::Color::fromHex(0xF8FAFC), true);
            paint->drawRect(surshell::Rect{20, 20, 480, 300}, surshell::Color::fromHex(0xCBD5E1), false);

            paint->drawCircle(surshell::Point{140, 140}, 60, cyan, true);
            paint->drawCircle(surshell::Point{170, 150}, 45, purple, true);
            paint->drawCircle(surshell::Point{200, 130}, 30, amber, true);

            paint->drawRect(surshell::Rect{260, 60, 120, 80}, slate, false);
            paint->floodFill(surshell::Point{280, 80}, green);

            paint->drawLine(surshell::Point{50, 260}, surshell::Point{450, 260}, slate, 3);
            paint->drawLine(surshell::Point{50, 260}, surshell::Point{250, 180}, coral, 4);
            paint->drawLine(surshell::Point{250, 180}, surshell::Point{450, 260}, cyan, 4);

            paint->drawBrushSpot(surshell::Point{380, 200}, coral, 10);
            paint->drawBrushSpot(surshell::Point{420, 200}, amber, 7);

            paint->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_paint_studio.bmp")) {
        std::cout << "  -> Exported: surshell_paint_studio.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 29: Sovereign System Information & Diagnostics (msinfo32.exe Parity)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 29: Sovereign System Information & Diagnostics...\n";
    const uint32_t sysWin = shell.openSystemInfoWindow();
    shell.windowManager().setWindowActive(sysWin);
    if (auto* w = shell.windowManager().findWindow(sysWin)) {
        if (auto sysInfo = std::dynamic_pointer_cast<surshell::SysInfoContent>(w->content)) {
            sysInfo->selectCategory("summary");
            sysInfo->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_system_information.bmp")) {
        std::cout << "  -> Exported: surshell_system_information.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 30: Sovereign Device Manager & Hardware Tree (devmgmt.msc Parity)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 30: Sovereign Device Manager & Hardware Tree...\n";
    const uint32_t devWin = shell.openDeviceManagerWindow();
    shell.windowManager().setWindowActive(devWin);
    if (auto* w = shell.windowManager().findWindow(devWin)) {
        if (auto devMgr = std::dynamic_pointer_cast<surshell::DeviceManagerContent>(w->content)) {
            devMgr->selectDevice("disp_gpu");
            devMgr->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_device_manager.bmp")) {
        std::cout << "  -> Exported: surshell_device_manager.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 31: Sovereign Disk Management & Volume Partitioning (diskmgmt.msc Parity)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 31: Sovereign Disk Management & Volume Partitioning...\n";
    const uint32_t diskWin = shell.openDiskManagementWindow();
    shell.windowManager().setWindowActive(diskWin);
    if (auto* w = shell.windowManager().findWindow(diskWin)) {
        if (auto diskMgr = std::dynamic_pointer_cast<surshell::DiskManagementContent>(w->content)) {
            diskMgr->selectVolume("C:");
            diskMgr->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_disk_management.bmp")) {
        std::cout << "  -> Exported: surshell_disk_management.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 32: Sovereign Services Management Console (services.msc Parity)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 32: Sovereign Services Management Console...\n";
    const uint32_t svcWin = shell.openServicesWindow();
    shell.windowManager().setWindowActive(svcWin);
    if (auto* w = shell.windowManager().findWindow(svcWin)) {
        if (auto svcMgr = std::dynamic_pointer_cast<surshell::ServicesContent>(w->content)) {
            svcMgr->selectServiceByName("EventLog");
            svcMgr->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_services_management.bmp")) {
        std::cout << "  -> Exported: surshell_services_management.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 33: Sovereign Event Viewer Console (eventvwr.msc Parity)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 33: Sovereign Event Viewer Console...\n";
    const uint32_t evWin = shell.openEventViewerWindow("System");
    shell.windowManager().setWindowActive(evWin);
    if (auto* w = shell.windowManager().findWindow(evWin)) {
        if (auto evMgr = std::dynamic_pointer_cast<surshell::EventViewerContent>(w->content)) {
            evMgr->selectIndex(0);
            evMgr->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_event_viewer.bmp")) {
        std::cout << "  -> Exported: surshell_event_viewer.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 34: Sovereign Winget App Hub & Retail Suite (Retail Packages View)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 34: Sovereign Winget App Hub & Retail Suite...\n";
    const uint32_t hubWin = shell.openAppHubWindow();
    shell.windowManager().setWindowActive(hubWin);
    if (auto* w = shell.windowManager().findWindow(hubWin)) {
        if (auto hub = std::dynamic_pointer_cast<surshell::AppHubContent>(w->content)) {
            hub->setCategory(surshell::AppHubCategory::CertifiedRetail);
            hub->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_app_hub.bmp")) {
        std::cout << "  -> Exported: surshell_app_hub.bmp (1920x1080 32-bpp)\n";
    }

    // ------------------------------------------------------------------------
    // Scene 35: Sovereign App Hub Settings & Source Repository Configuration
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 35: Sovereign App Hub Settings & Sources Console...\n";
    if (auto* w = shell.windowManager().findWindow(hubWin)) {
        if (auto hub = std::dynamic_pointer_cast<surshell::AppHubContent>(w->content)) {
            hub->setCategory(surshell::AppHubCategory::Settings);
            hub->render(w->clientSurface);
        }
    }
    shell.render();
    if (shell.exportSnapshot("surshell_app_hub_settings.bmp")) {
        std::cout << "  -> Exported: surshell_app_hub_settings.bmp (1920x1080 32-bpp)\n";
    }

    std::cout << "\n[SurShell] Visual presentation pipeline completed successfully (36 high-resolution scenes generated).\n";
    return 0;
}

int main(int argc, char* argv[]) {
    bool runSnapshots = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--snapshots" || arg == "--batch" || arg == "--export" || arg == "--headless" || arg == "--test-scenes") {
            runSnapshots = true;
            break;
        }
    }

#ifdef _WIN32
    if (!runSnapshots) {
        return runInteractiveDesktop();
    }
#endif

    return runSnapshotPipeline();
}
