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

    std::cout << "\n[SurShell] Visual presentation pipeline completed successfully.\n";
    return 0;
}
