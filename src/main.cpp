// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/main.cpp)
// ============================================================================

#include "surshell/surshell.hpp"
#include <iostream>
#include <chrono>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::cout << "===============================================================================\n";
    std::cout << "SurShell: Sovereign Clean-Room Desktop Shell for MicaNT\n";
    std::cout << "Named in tribute to Dave Cutler's Windows NT 4.0 'SUR' (Shell Update Release)\n";
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
    // Scene 2b: Start Menu with Windows 11-Style Interactive Power Flyout
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
    // Scene 9: Alt+Tab Task Switcher HUD (Windows 11 / Aero Ergonomics)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 9: Alt+Tab Task Switcher HUD with Live Thumbnails...\n";
    shell.triggerAltTab();
    shell.render();
    if (shell.exportSnapshot("surshell_alt_tab_hud.bmp")) {
        std::cout << "  -> Exported: surshell_alt_tab_hud.bmp (1920x1080 32-bpp)\n";
    }
    shell.dismissAltTab();

    // ------------------------------------------------------------------------
    // Scene 10: Taskbar Live Hover Preview (Windows Peek)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 10: Taskbar Live Hover Preview (Windows Peek)...\n";
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
    shell.toastManager().showToast("Network Connected", "Gigabit Ethernet (1000/1000 Mbps) Online", surshell::IconId::NetworkEthernet, surshell::Color::fromHex(0x00FF9D));
    shell.toastManager().showToast("SentinelSec Security", "Zero-Telemetry Protection Guard Active", surshell::IconId::SentinelSec, surshell::Color::fromHex(0x00D4FF));
    shell.toastManager().showToast("MicaNT Audio Engine", "3D Spatial Prism HRTF Ready", surshell::IconId::VolumeHigh, surshell::Color::fromHex(0xFFD54F));
    shell.render();
    if (shell.exportSnapshot("surshell_toast_notifications.bmp")) {
        std::cout << "  -> Exported: surshell_toast_notifications.bmp (1920x1080 32-bpp)\n";
    }
    shell.toastManager().clear();

    // ------------------------------------------------------------------------
    // Scene 13: Modern Audio & Media Playback HUD (OSD Overlay)
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 13: Audio & Media Playback HUD (OSD Overlay)...\n";
    shell.mediaHud().showVolume(85);
    shell.mediaHud().showMedia("Dave Cutler - Symphony in C++23", "MicaNT Sovereign Philharmonic");
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

    std::cout << "\n[SurShell] Visual presentation pipeline completed successfully (19 high-resolution scenes generated).\n";
    return 0;
}
