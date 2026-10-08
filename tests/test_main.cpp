// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (tests/test_main.cpp)
// ============================================================================

#include "surshell/surshell.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion FAILED: " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

void Test_Geometry_And_Color() {
    std::cout << "[TEST] Running Suite 1: Geometry & Color Math...\n";

    // 1. Color tests
    surshell::Color c1 = surshell::Color::fromRgb(255, 0, 0);
    surshell::Color c2 = surshell::Color::fromRgb(0, 0, 255);
    surshell::Color cMid = surshell::Color::lerp(c1, c2, 0.5f);
    TEST_ASSERT(cMid.r == 127 || cMid.r == 128, "Lerp red component");
    TEST_ASSERT(cMid.b == 127 || cMid.b == 128, "Lerp blue component");

    surshell::Color transparentCyan = surshell::Color::fromRgba(0, 212, 255, 128);
    surshell::Color backgroundBlack = surshell::Color::fromRgb(0, 0, 0);
    surshell::Color blended = transparentCyan.blendOver(backgroundBlack);
    TEST_ASSERT(blended.a == 255, "Alpha blended over opaque background must be 255");
    TEST_ASSERT(blended.g > 0, "Blended green must be positive");

    // 2. Rect tests
    surshell::Rect r1{10, 20, 100, 50};
    TEST_ASSERT(r1.contains(surshell::Point{15, 25}), "Point inside rect");
    TEST_ASSERT(!r1.contains(surshell::Point{5, 5}), "Point outside rect");
    TEST_ASSERT(r1.right() == 110, "Rect right boundary");
    TEST_ASSERT(r1.bottom() == 70, "Rect bottom boundary");

    surshell::Rect r2{60, 40, 100, 50};
    TEST_ASSERT(r1.intersects(r2), "Intersecting rects");
    surshell::Rect rIntersect = r1.intersectWith(r2);
    TEST_ASSERT(rIntersect.x == 60 && rIntersect.y == 40 && rIntersect.width == 50 && rIntersect.height == 30, "Intersection bounds");

    std::cout << "[TEST] Suite 1: Geometry & Color Math PASSED.\n";
}

void Test_Compositor_And_Surface() {
    std::cout << "[TEST] Running Suite 2: Compositor & Surface Rendering...\n";

    surshell::Surface surf(200, 100, surshell::Color::fromRgb(0, 0, 0));
    TEST_ASSERT(surf.width() == 200, "Surface width match");
    TEST_ASSERT(surf.height() == 100, "Surface height match");

    surf.putPixel(10, 10, surshell::Color::fromRgb(255, 255, 255));
    surshell::Color p = surf.getPixel(10, 10);
    TEST_ASSERT(p.r == 255 && p.g == 255 && p.b == 255, "Pixel readback match");

    surf.fillRect(surshell::Rect{20, 20, 40, 30}, surshell::Color::fromRgb(0, 212, 255));
    TEST_ASSERT(surf.getPixel(25, 25).g == 212, "FillRect interior match");

    surshell::Surface src(20, 20, surshell::Color::fromRgb(255, 0, 0));
    surf.blit(src, surshell::Rect{0, 0, 20, 20}, surshell::Point{80, 40});
    TEST_ASSERT(surf.getPixel(85, 45).r == 255, "Blit readback match");

    TEST_ASSERT(surf.exportBmp("test_surface_export.bmp"), "BMP export must succeed");

    std::cout << "[TEST] Suite 2: Compositor & Surface Rendering PASSED.\n";
}

void Test_Desktop_Manager() {
    std::cout << "[TEST] Running Suite 3: Desktop Manager & Icon Grid...\n";

    surshell::DesktopManager dm(1920, 1080);
    dm.addIcon(surshell::DesktopIcon{
        .id = "app1",
        .label = "App One",
        .executable = "app1.exe",
        .arguments = "",
        .iconGlyph = "[1]",
        .gridX = 0,
        .gridY = 0,
        .bounds = surshell::Rect{},
        .selected = false
    });
    dm.addIcon(surshell::DesktopIcon{
        .id = "app2",
        .label = "App Two",
        .executable = "app2.exe",
        .arguments = "",
        .iconGlyph = "[2]",
        .gridX = 0,
        .gridY = 0,
        .bounds = surshell::Rect{},
        .selected = false
    });

    TEST_ASSERT(dm.icons().size() == 2, "Icons registered");
    TEST_ASSERT(!dm.icons()[0].bounds.empty(), "Icon 0 bounds arranged");
    TEST_ASSERT(!dm.icons()[1].bounds.empty(), "Icon 1 bounds arranged");
    TEST_ASSERT(dm.icons()[0].bounds.y != dm.icons()[1].bounds.y, "Icons arranged vertically in grid");

    // Click icon 0
    dm.onMouseDown(surshell::Point{dm.icons()[0].bounds.x + 5, dm.icons()[0].bounds.y + 5}, surshell::MouseButton::Left);
    auto sel = dm.getSelectedIcon();
    TEST_ASSERT(sel.has_value(), "An icon should be selected");
    TEST_ASSERT(sel->get().id == "app1", "app1 is selected");

    // Launch callback on double click
    bool launched = false;
    dm.setLaunchCallback([&](const surshell::DesktopIcon& icon) {
        if (icon.id == "app1") launched = true;
    });
    dm.onDoubleClick(surshell::Point{dm.icons()[0].bounds.x + 5, dm.icons()[0].bounds.y + 5});
    TEST_ASSERT(launched, "Double click triggers launch callback");

    std::cout << "[TEST] Suite 3: Desktop Manager & Icon Grid PASSED.\n";
}

void Test_Taskbar_And_SystemTray() {
    std::cout << "[TEST] Running Suite 4: Taskbar & System Tray...\n";

    surshell::Taskbar tb(1920, 1080);
    surshell::Rect r = tb.bounds();
    TEST_ASSERT(r.x == 0 && r.width == 1920, "Taskbar horizontal span");
    TEST_ASSERT(tb.appIslandBounds().width > 0, "Floating App Island initialized");
    TEST_ASSERT(tb.trayIslandBounds().width > 0, "Floating Tray Island initialized");

    tb.addOrUpdateTask(1001, "Command Prompt", ">_", true, false);
    tb.addOrUpdateTask(1002, "File Explorer", "[E]", false, false);
    TEST_ASSERT(tb.tasks().size() == 2, "Two tasks present");
    TEST_ASSERT(tb.tasks()[0].isActive, "Task 1001 active");
    TEST_ASSERT(!tb.tasks()[1].isActive, "Task 1002 inactive");

    tb.setActiveTask(1002);
    TEST_ASSERT(!tb.tasks()[0].isActive, "Task 1001 inactive after switch");
    TEST_ASSERT(tb.tasks()[1].isActive, "Task 1002 active after switch");

    tb.removeTask(1001);
    TEST_ASSERT(tb.tasks().size() == 1, "Task removed");
    TEST_ASSERT(tb.tasks()[0].windowId == 1002, "Remaining task is 1002");

    // Tray tests
    auto& tray = tb.tray();
    TEST_ASSERT(tray.preferredWidth() > 100, "System tray has positive width");
    tray.setVolumeLevel(95);
    tray.setNetworkOnline(true);
    tray.setTimeOverride("04:20 PM");
    TEST_ASSERT(tray.currentTimeString() == "04:20 PM", "Time override match");

    std::cout << "[TEST] Suite 4: Taskbar & System Tray PASSED.\n";
}

void Test_Start_Menu_And_Filter() {
    std::cout << "[TEST] Running Suite 5: Start Menu & Search Filter...\n";

    surshell::StartMenu sm;
    TEST_ASSERT(!sm.isOpen(), "Start Menu closed initially");
    sm.open();
    TEST_ASSERT(sm.isOpen(), "Start Menu open");

    sm.setSearchQuery("cmd");
    TEST_ASSERT(sm.filteredApps().size() == 1, "Filtered query 'cmd' matches 1 app");
    TEST_ASSERT(sm.filteredApps()[0].id == "cmd", "Matched app is cmd");

    sm.setSearchQuery("explorer");
    TEST_ASSERT(sm.filteredApps().size() == 1, "Filtered query 'explorer' matches 1 app");

    sm.setSearchQuery("nonexistent_app_xyz");
    TEST_ASSERT(sm.filteredApps().empty(), "Nonexistent query yields 0 results");

    sm.setSearchQuery("");
    TEST_ASSERT(sm.filteredApps().size() >= 5, "Empty query shows all pinned apps");

    sm.toggle();
    TEST_ASSERT(!sm.isOpen(), "Toggle closed start menu");

    std::cout << "[TEST] Suite 5: Start Menu & Search Filter PASSED.\n";
}

void Test_Window_Manager_And_Aero_Snap() {
    std::cout << "[TEST] Running Suite 6: Window Manager & Aero Snap...\n";

    surshell::WindowManager wm(1920, 1080, 40);
    const uint32_t w1 = wm.createWindow("Window One", surshell::Rect{100, 100, 600, 400}, "[1]");
    const uint32_t w2 = wm.createWindow("Window Two", surshell::Rect{200, 200, 500, 350}, "[2]");

    TEST_ASSERT(wm.windows().size() == 2, "Two windows created");
    TEST_ASSERT(wm.activeWindowId() == w2, "Top window is active");

    auto* win1 = wm.findWindow(w1);
    auto* win2 = wm.findWindow(w2);
    TEST_ASSERT(win1 != nullptr && win2 != nullptr, "Windows found by id");

    // Hit Testing
    surshell::Point closePt{win2->closeButtonBounds().x + 5, win2->closeButtonBounds().y + 5};
    TEST_ASSERT(win2->hitTest(closePt) == surshell::HitTestResult::CloseButton, "Hit test close button");

    surshell::Point capPt{win2->captionBounds().x + 50, win2->captionBounds().y + 10};
    TEST_ASSERT(win2->hitTest(capPt) == surshell::HitTestResult::Caption, "Hit test caption");

    // Aero Snap Left / Right
    wm.snapWindow(w1, surshell::WindowState::SnappedLeft);
    TEST_ASSERT(win1->state == surshell::WindowState::SnappedLeft, "SnappedLeft state");
    TEST_ASSERT(win1->currentBounds.x == 0, "SnappedLeft x is 0");
    TEST_ASSERT(win1->currentBounds.width == 960, "SnappedLeft half width");
    TEST_ASSERT(win1->currentBounds.height == 1040, "SnappedLeft height excludes taskbar");

    wm.snapWindow(w2, surshell::WindowState::SnappedRight);
    TEST_ASSERT(win2->state == surshell::WindowState::SnappedRight, "SnappedRight state");
    TEST_ASSERT(win2->currentBounds.x == 960, "SnappedRight x starts at 960");
    TEST_ASSERT(win2->currentBounds.width == 960, "SnappedRight width is 960");

    // Modern 67/33 Priority Split Snap
    wm.snapWindow(w1, surshell::WindowState::SnappedPriorityLeft);
    TEST_ASSERT(win1->state == surshell::WindowState::SnappedPriorityLeft, "SnappedPriorityLeft state");
    TEST_ASSERT(win1->currentBounds.width == 1280, "SnappedPriorityLeft 67% width (1280px)");

    wm.snapWindow(w2, surshell::WindowState::SnappedSidebarRight);
    TEST_ASSERT(win2->state == surshell::WindowState::SnappedSidebarRight, "SnappedSidebarRight state");
    TEST_ASSERT(win2->currentBounds.x == 1280, "SnappedSidebarRight begins at 1280px");
    TEST_ASSERT(win2->currentBounds.width == 640, "SnappedSidebarRight 33% width (640px)");

    // Snap Layout Assistant Flyout
    wm.showSnapFlyout(w1, surshell::Point{500, 40});
    TEST_ASSERT(wm.isSnapFlyoutVisible(), "Snap Layout Flyout is visible");
    wm.hideSnapFlyout();
    TEST_ASSERT(!wm.isSnapFlyoutVisible(), "Snap Layout Flyout is hidden");

    // Maximize & Restore
    wm.toggleMaximize(w1);
    TEST_ASSERT(win1->state == surshell::WindowState::Maximized, "Window maximized");
    TEST_ASSERT(win1->currentBounds.width == 1920 && win1->currentBounds.height == 1040, "Maximized covers full workspace");

    wm.toggleMaximize(w1);
    TEST_ASSERT(win1->state == surshell::WindowState::Normal, "Window restored to normal");

    wm.closeWindow(w2);
    TEST_ASSERT(wm.windows().size() == 1, "Window closed");

    std::cout << "[TEST] Suite 6: Window Manager & Aero Snap PASSED.\n";
}

void Test_File_Explorer_Navigation() {
    std::cout << "[TEST] Running Suite 7: File Explorer Navigation...\n";

    surshell::FileExplorer exp("C:\\");
    TEST_ASSERT(exp.currentPath() == "C:\\", "Initial path C:\\");
    TEST_ASSERT(!exp.items().empty(), "C:\\ has items");

    exp.navigateTo("C:\\Windows");
    TEST_ASSERT(exp.currentPath() == "C:\\Windows", "Navigated to C:\\Windows");

    exp.navigateTo("C:\\Windows\\System32");
    TEST_ASSERT(exp.currentPath() == "C:\\Windows\\System32", "Navigated to C:\\Windows\\System32");

    exp.navigateUp();
    TEST_ASSERT(exp.currentPath() == "C:\\Windows", "Navigate up to C:\\Windows");

    exp.navigateBack();
    TEST_ASSERT(exp.currentPath() == "C:\\Windows\\System32", "Navigate back to C:\\Windows\\System32");

    exp.navigateForward();
    TEST_ASSERT(exp.currentPath() == "C:\\Windows", "Navigate forward to C:\\Windows");

    std::cout << "[TEST] Suite 7: File Explorer Navigation PASSED.\n";
}

void Test_Full_Desktop_Integration() {
    std::cout << "[TEST] Running Suite 8: Master Desktop Integration...\n";

    surshell::SurShellDesktop shell(1920, 1080);
    TEST_ASSERT(shell.width() == 1920, "Width match");
    TEST_ASSERT(shell.height() == 1080, "Height match");

    // Click Start button (Mica Prism)
    const surshell::Point startPt = shell.taskbar().startButtonBounds().center();
    shell.onMouseDown(startPt, surshell::MouseButton::Left);
    shell.onMouseUp(startPt, surshell::MouseButton::Left);
    TEST_ASSERT(shell.startMenu().isOpen(), "Start button click opened start menu");

    // Type query
    shell.onCharInput('c');
    shell.onCharInput('m');
    shell.onCharInput('d');
    TEST_ASSERT(shell.startMenu().filteredApps().size() == 1, "Typed 'cmd' filtered 1 app");

    // Render frame
    shell.render();

    // Export test snapshot
    TEST_ASSERT(shell.exportSnapshot("test_full_desktop_render.bmp"), "Full render snapshot must export");

    std::cout << "[TEST] Suite 8: Master Desktop Integration PASSED.\n";
}

void Test_SubSurface_Blur_And_Acrylic() {
    std::cout << "[TEST] Running Suite 9: Sub-Surface Blur & Acrylic Translucency...\n";

    surshell::Surface surf(120, 120, surshell::Color::fromRgb(0, 0, 0));
    // Draw bright white column
    surf.fillRect(surshell::Rect{50, 0, 20, 120}, surshell::Color::fromRgb(255, 255, 255));

    TEST_ASSERT(surf.getPixel(49, 60).r == 0, "Pre-blur adjacent pixel is black");
    TEST_ASSERT(surf.getPixel(50, 60).r == 255, "Pre-blur stripe pixel is white");

    // Apply fast separable box blur
    surf.applyBoxBlur(surshell::Rect{30, 30, 60, 60}, 5);

    // Pixel at (49, 60) should now be blurred to an intermediate gray
    const surshell::Color blurredAdj = surf.getPixel(49, 60);
    TEST_ASSERT(blurredAdj.r > 20 && blurredAdj.r < 240, "Blurred adjacent pixel must be intermediate intensity");

    // Apply acrylic frosted glass tint
    surf.applyAcrylicTint(surshell::Rect{30, 30, 60, 60}, surshell::Color::fromRgba(16, 24, 40, 200), 4);
    const surshell::Color tinted = surf.getPixel(50, 50);
    TEST_ASSERT(tinted.r < 200, "Tinted pixel has darkened acrylic tone");

    std::cout << "[TEST] Suite 9: Sub-Surface Blur & Acrylic Translucency PASSED.\n";
}

void Test_Quick_Settings_Flyout() {
    std::cout << "[TEST] Running Suite 10: Quick Settings Flyout & Audio/Brightness Controls...\n";

    surshell::QuickSettingsFlyout qs;
    qs.updateLayout(1920, 1080, 48);

    TEST_ASSERT(!qs.isOpen(), "Quick settings closed initially");
    qs.open();
    TEST_ASSERT(qs.isOpen(), "Quick settings open");
    TEST_ASSERT(qs.bounds().width == 360, "Flyout width is 360");

    // Check default states
    TEST_ASSERT(qs.isToggleEnabled("mesh"), "RazzleNet Mesh enabled by default");
    TEST_ASSERT(qs.isToggleEnabled("sentinel"), "SentinelSec enabled by default");
    TEST_ASSERT(!qs.isToggleEnabled("nightlight"), "Night Light disabled by default");

    // Toggle Night Light
    qs.setToggleEnabled("nightlight", true);
    TEST_ASSERT(qs.isToggleEnabled("nightlight"), "Night Light toggled on");

    // Sliders
    qs.setVolume(90);
    TEST_ASSERT(qs.volume() == 90, "Volume set to 90%");
    qs.setBrightness(75);
    TEST_ASSERT(qs.brightness() == 75, "Brightness set to 75%");

    // Mouse click inside flyout (toggle first item or slider)
    const surshell::Point insidePt{qs.bounds().centerX(), qs.bounds().centerY()};
    TEST_ASSERT(qs.onMouseDown(insidePt, surshell::MouseButton::Left), "Click inside flyout consumed");

    // Click outside closes
    const surshell::Point outsidePt{10, 10};
    TEST_ASSERT(!qs.onMouseDown(outsidePt, surshell::MouseButton::Left), "Click outside not consumed");
    TEST_ASSERT(!qs.isOpen(), "Click outside closed flyout");

    std::cout << "[TEST] Suite 10: Quick Settings Flyout & Audio/Brightness Controls PASSED.\n";
}

void Test_Virtual_Desktops() {
    std::cout << "[TEST] Running Suite 11: Virtual Desktops & Multi-Workspace Manager...\n";

    surshell::VirtualDesktopManager vdm;
    vdm.updateLayout(1920, 1080, 48);

    TEST_ASSERT(vdm.desktopCount() == 2, "2 default virtual desktops created");
    TEST_ASSERT(vdm.activeIndex() == 0, "Active desktop is 0");

    // Create a 3rd desktop
    const uint32_t d3 = vdm.createDesktop("3: Media & Gaming");
    TEST_ASSERT(vdm.desktopCount() == 3, "3 virtual desktops present");
    TEST_ASSERT(d3 > 0, "Valid desktop ID returned");

    // Assign window 1001 to Desktop 0, window 1002 to Desktop 1
    vdm.assignWindowToDesktop(1001, 0);
    vdm.assignWindowToDesktop(1002, 1);

    // On Desktop 0: 1001 is visible, 1002 is hidden
    TEST_ASSERT(vdm.isWindowVisible(1001), "Window 1001 visible on Desktop 0");
    TEST_ASSERT(!vdm.isWindowVisible(1002), "Window 1002 hidden on Desktop 0");

    // Switch to Desktop 1
    vdm.switchDesktop(1);
    TEST_ASSERT(vdm.activeIndex() == 1, "Active desktop is 1");
    TEST_ASSERT(!vdm.isWindowVisible(1001), "Window 1001 hidden on Desktop 1");
    TEST_ASSERT(vdm.isWindowVisible(1002), "Window 1002 visible on Desktop 1");

    // Pin window 1001 to all desktops
    vdm.pinWindowToAllDesktops(1001, true);
    TEST_ASSERT(vdm.isWindowVisible(1001), "Pinned window 1001 now visible on Desktop 1");

    // Switcher HUD
    TEST_ASSERT(!vdm.isSwitcherVisible(), "Switcher hidden initially");
    vdm.toggleSwitcher();
    TEST_ASSERT(vdm.isSwitcherVisible(), "Switcher toggled open");
    vdm.toggleSwitcher();
    TEST_ASSERT(!vdm.isSwitcherVisible(), "Switcher toggled closed");

    std::cout << "[TEST] Suite 11: Virtual Desktops & Multi-Workspace Manager PASSED.\n";
}

void Test_MicaNT_Kernel_Bridge() {
    std::cout << "[TEST] Running Suite 12: MicaNT Executive LPC Syscall Bridge...\n";

    surshell::KernelBridge bridge;
    TEST_ASSERT(!bridge.isConnected(), "Bridge disconnected initially");

    TEST_ASSERT(bridge.connectToExecutive("\\RPC_Control\\SurWinLpc"), "Connect to SurWin LPC port succeeds");
    TEST_ASSERT(bridge.isConnected(), "Bridge is connected");

    // Spawn process
    auto proc = bridge.spawnProcess("C:\\Windows\\System32\\sentinel_scan.exe", "--deep-heuristic");
    TEST_ASSERT(proc.has_value(), "Process spawned");
    TEST_ASSERT(proc->pid >= 2000, "PID allocated above standard userland range");
    TEST_ASSERT(proc->name == "sentinel_scan.exe", "Process name extracted");

    // Query active processes
    auto procList = bridge.queryProcesses();
    TEST_ASSERT(procList.size() >= 9, "Process list contains core system and spawned processes");

    // Query vitals
    auto vitals = bridge.queryVitals();
    TEST_ASSERT(vitals.activeProcessCount >= 9, "Vitals reports active process count");
    TEST_ASSERT(vitals.totalPhysicalMemoryKb > 0, "Vitals reports physical memory");

    // Register window with SurWin
    uint32_t handle = bridge.registerWindowWithSurWin("Test Window", surshell::Rect{100, 100, 400, 300});
    TEST_ASSERT(handle > 0, "Valid SurWin window handle returned");
    TEST_ASSERT(bridge.unregisterWindowWithSurWin(handle), "Unregister window succeeds");

    std::cout << "[TEST] Suite 12: MicaNT Executive LPC Syscall Bridge PASSED.\n";
}

int main() {
    std::cout << "===============================================================================\n";
    std::cout << "SurShell Test Runner: Sovereign Desktop Shell Verification Suite\n";
    std::cout << "Standard: ISO C++23 | Zero Dependencies | Clean-Room Provenance\n";
    std::cout << "===============================================================================\n\n";

    Test_Geometry_And_Color();
    Test_Compositor_And_Surface();
    Test_Desktop_Manager();
    Test_Taskbar_And_SystemTray();
    Test_Start_Menu_And_Filter();
    Test_Window_Manager_And_Aero_Snap();
    Test_File_Explorer_Navigation();
    Test_Full_Desktop_Integration();
    Test_SubSurface_Blur_And_Acrylic();
    Test_Quick_Settings_Flyout();
    Test_Virtual_Desktops();
    Test_MicaNT_Kernel_Bridge();

    std::cout << "\n===============================================================================\n";
    std::cout << "ALL 12 SURSHELL SUBSYSTEM VERIFICATION SUITES PASSED (100% SUCCESS)\n";
    std::cout << "===============================================================================\n";
    return 0;
}
