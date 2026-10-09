// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/regedit_main.cpp)
//
// Standalone Sovereign Registry Editor Executable (regedit.exe)
// ============================================================================

#include "surshell/registry_editor.hpp"
#include "surshell/compositor.hpp"
#include "surshell/theme.hpp"
#include <iostream>
#include <memory>
#include <chrono>

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
        case VK_F2:     return surshell::KeyCode::F2;
        case VK_F5:     return surshell::KeyCode::F5;
        default:        return surshell::KeyCode::Unknown;
    }
}

struct AppState {
    std::unique_ptr<surshell::Surface> surface;
    std::unique_ptr<surshell::RegistryEditorContent> editor;
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* state = reinterpret_cast<AppState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_CREATE: {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            auto* newState = reinterpret_cast<AppState*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(newState));
            return 0;
        }

        case WM_ERASEBKGND:
            return 1; // Prevent GDI flicker

        case WM_SIZE: {
            if (!state) break;
            const int32_t nw = LOWORD(lParam);
            const int32_t nh = HIWORD(lParam);
            if (nw > 100 && nh > 100) {
                state->surface->resize(static_cast<uint32_t>(nw), static_cast<uint32_t>(nh));
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        }

        case WM_PAINT: {
            if (!state) break;
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            state->editor->render(*state->surface);

            BITMAPINFO bmi{};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = static_cast<LONG>(state->surface->width());
            bmi.bmiHeader.biHeight = -static_cast<LONG>(state->surface->height()); // Top-down
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            SetDIBitsToDevice(hdc, 0, 0, state->surface->width(), state->surface->height(),
                              0, 0, 0, state->surface->height(),
                              state->surface->pixels().data(), &bmi, DIB_RGB_COLORS);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            if (!state) break;
            SetCapture(hwnd);
            const int32_t x = GET_X_LPARAM(lParam);
            const int32_t y = GET_Y_LPARAM(lParam);
            state->editor->onMouseDown(surshell::Point{x, y}, surshell::MouseButton::Left);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_LBUTTONUP: {
            if (!state) break;
            ReleaseCapture();
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_RBUTTONDOWN: {
            if (!state) break;
            const int32_t x = GET_X_LPARAM(lParam);
            const int32_t y = GET_Y_LPARAM(lParam);
            state->editor->onMouseDown(surshell::Point{x, y}, surshell::MouseButton::Right);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_MOUSEMOVE: {
            if (!state) break;
            const int32_t x = GET_X_LPARAM(lParam);
            const int32_t y = GET_Y_LPARAM(lParam);
            state->editor->onMouseMove(surshell::Point{x, y});
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_LBUTTONDBLCLK: {
            if (!state) break;
            const int32_t x = GET_X_LPARAM(lParam);
            const int32_t y = GET_Y_LPARAM(lParam);
            state->editor->onDoubleClick(surshell::Point{x, y});
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_MOUSEWHEEL: {
            if (!state) break;
            const int32_t delta = GET_WHEEL_DELTA_WPARAM(wParam);
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            ScreenToClient(hwnd, &pt);
            state->editor->onMouseWheel(surshell::Point{pt.x, pt.y}, delta);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_CHAR: {
            if (!state) break;
            const char c = static_cast<char>(wParam);
            state->editor->onCharInput(c);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_KEYDOWN: {
            if (!state) break;
            const bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            const bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            const bool alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
            const auto kc = mapVkToKeyCode(wParam);
            state->editor->onKeyDown(kc, ctrl, shift, alt);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // anonymous namespace
#endif

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::cout << "===============================================================================\n";
    std::cout << "MicaNT Sovereign Registry Editor (regedit.exe)\n";
    std::cout << "Standard: ISO C++23 | Clean-Room Provenance | Barrer Software & MicaNT Community\n";
    std::cout << "===============================================================================\n\n";

#ifdef _WIN32
    AppState state;
    state.surface = std::make_unique<surshell::Surface>(960, 620);
    state.editor = std::make_unique<surshell::RegistryEditorContent>("Computer\\HKEY_LOCAL_MACHINE\\SOFTWARE\\MicaNT\\CurrentVersion");

    HINSTANCE hInstance = GetModuleHandleW(nullptr);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.hbrBackground = nullptr;
    wc.lpszClassName = L"MicaNTSovereignRegistryEditor";

    RegisterClassExW(&wc);

    // Center window on primary monitor
    const int screenW = GetSystemMetrics(SM_CXSCREEN);
    const int screenH = GetSystemMetrics(SM_CYSCREEN);
    const int winW = 960;
    const int winH = 620;
    const int winX = (std::max)(0, (screenW - winW) / 2);
    const int winY = (std::max)(0, (screenH - winH) / 2);

    HWND hwnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        L"MicaNTSovereignRegistryEditor",
        L"Registry Editor - MicaNT Sovereign Workstation",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        winX, winY, winW, winH,
        nullptr, nullptr, hInstance, &state
    );

    if (!hwnd) {
        std::cerr << "Failed to create Registry Editor Win32 window.\n";
        return 1;
    }

    // Enable Sovereign Dark Titlebar
    BOOL darkMode = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    std::cout << "[Registry Editor] Native Interactive Win32 window launched.\n";
    std::cout << "  -> Window: " << winW << "x" << winH << " at (" << winX << ", " << winY << ")\n";
    std::cout << "  -> Close the window or press Alt+F4 to exit.\n\n";

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
#else
    // Headless / Linux CI fallback
    std::cout << "[Headless] Initializing Registry Editor for headless test...\n";
    surshell::Surface testSurf(960, 620);
    surshell::RegistryEditorContent testEditor("Computer\\HKEY_LOCAL_MACHINE\\SOFTWARE\\MicaNT\\CurrentVersion");
    testEditor.render(testSurf);
    if (testSurf.exportBmp("surshell_regedit_headless.bmp")) {
        std::cout << "  -> Exported surshell_regedit_headless.bmp successfully.\n";
    }
    return 0;
#endif
}
