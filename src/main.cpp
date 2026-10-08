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
    // Scene 2: Modern Centered Start Prism Hub with Tactile App Grid
    // ------------------------------------------------------------------------
    std::cout << "[SurShell] Rendering Scene 2: Floating Start Prism Hub with Application Grid...\n";
    shell.startMenu().open();
    shell.render();
    if (shell.exportSnapshot("surshell_start_menu_active.bmp")) {
        std::cout << "  -> Exported: surshell_start_menu_active.bmp (1920x1080 32-bpp)\n";
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

    std::cout << "\n[SurShell] Visual presentation pipeline completed successfully.\n";
    return 0;
}
