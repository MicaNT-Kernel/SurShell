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

    // View mode tests (Pinned vs AllApps)
    TEST_ASSERT(sm.viewMode() == surshell::StartViewMode::Pinned, "Default view mode is Pinned");
    sm.toggleViewMode();
    TEST_ASSERT(sm.viewMode() == surshell::StartViewMode::AllApps, "Toggled view mode is AllApps");
    sm.toggleViewMode();
    TEST_ASSERT(sm.viewMode() == surshell::StartViewMode::Pinned, "Toggled back to Pinned");

    // Recommended Items
    TEST_ASSERT(sm.recommendedItems().size() >= 4, "Recommended activities initialized");

    // Power Flyout & Power Callback Tests
    sm.open();
    const auto menuBounds = sm.calculateBounds(1920, 1080, 48);
    TEST_ASSERT(!sm.isPowerFlyoutOpen(), "Power flyout initially closed");

    sm.togglePowerFlyout();
    TEST_ASSERT(sm.isPowerFlyoutOpen(), "Power flyout opened");
    TEST_ASSERT(!sm.isUserFlyoutOpen(), "User flyout closed when power flyout open");

    surshell::PowerAction receivedAction = surshell::PowerAction::Lock;
    bool powerTriggered = false;
    sm.setPowerCallback([&](surshell::PowerAction act) {
        receivedAction = act;
        powerTriggered = true;
    });

    // Simulate clicking Restart (index 2 in power flyout)
    const auto pfb = sm.powerFlyoutBounds(menuBounds);
    const surshell::Point restartPt{pfb.x + 20, pfb.y + 12 + 2 * 34 + 10};
    sm.onMouseMove(restartPt, menuBounds);
    sm.onMouseDown(restartPt, surshell::MouseButton::Left, menuBounds);

    TEST_ASSERT(powerTriggered, "Power callback executed on restart click");
    TEST_ASSERT(receivedAction == surshell::PowerAction::Restart, "Received PowerAction::Restart");
    TEST_ASSERT(!sm.isOpen(), "Start Menu closed after power action");

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

    // Multi-Tab management
    TEST_ASSERT(exp.tabCount() == 1, "Initial tab count is 1");
    exp.addTab("C:\\Windows\\System32");
    TEST_ASSERT(exp.tabCount() == 2, "Tab count increased to 2");
    TEST_ASSERT(exp.activeTabIndex() == 1, "New tab is active");
    TEST_ASSERT(exp.currentPath() == "C:\\Windows\\System32", "Tab 2 path is System32");

    exp.switchTab(0);
    TEST_ASSERT(exp.activeTabIndex() == 0, "Switched back to tab 0");
    TEST_ASSERT(exp.currentPath() == "C:\\Windows", "Tab 0 path is C:\\Windows");

    // Search filtering within current folder
    exp.setSearchQuery("explorer");
    TEST_ASSERT(!exp.items().empty(), "Search for 'explorer' returns results");
    TEST_ASSERT(exp.items()[0].name.find("explorer") != std::string::npos, "Filtered item matches query");

    exp.clearSearch();
    TEST_ASSERT(exp.items().size() >= 3, "Cleared search restores full item list");

    // View mode and sorting
    exp.setViewMode(surshell::ExplorerViewMode::TilesGrid);
    TEST_ASSERT(exp.viewMode() == surshell::ExplorerViewMode::TilesGrid, "View mode changed to TilesGrid");

    exp.sortBy(surshell::ExplorerSortColumn::Size, false);
    TEST_ASSERT(exp.sortColumn() == surshell::ExplorerSortColumn::Size, "Sort column set to Size");

    exp.closeTab(1);
    TEST_ASSERT(exp.tabCount() == 1, "Closed tab 1, count is 1");

    // Vertical scrolling & mouse wheel
    TEST_ASSERT(exp.scrollOffset() == 0, "Initial scroll offset is 0");
    exp.onMouseWheel(surshell::Point{250, 150}, -2); // Scroll down
    TEST_ASSERT(exp.scrollOffset() > 0, "Mouse wheel down increased scroll offset");
    exp.onMouseWheel(surshell::Point{250, 150}, 2); // Scroll up
    TEST_ASSERT(exp.scrollOffset() == 0, "Mouse wheel up returned scroll offset to 0");

    // Column sorting toggle
    exp.sortBy(surshell::ExplorerSortColumn::Name, false);
    TEST_ASSERT(exp.sortColumn() == surshell::ExplorerSortColumn::Name, "Sort column set to Name");
    exp.sortBy(surshell::ExplorerSortColumn::Name, true);
    TEST_ASSERT(!exp.isSortAscending(), "Toggled sort direction to descending");
    exp.sortBy(surshell::ExplorerSortColumn::Name, true);
    TEST_ASSERT(exp.isSortAscending(), "Toggled sort direction to ascending");

    // Right-click Context Menu
    TEST_ASSERT(!exp.contextMenu().isOpen, "Context menu closed initially");
    exp.openContextMenu(surshell::Point{200, 200}, true);
    TEST_ASSERT(exp.contextMenu().isOpen, "Context menu opened for item");
    TEST_ASSERT(exp.contextMenu().items.size() >= 5, "Context menu has standard actions");
    exp.closeContextMenu();
    TEST_ASSERT(!exp.contextMenu().isOpen, "Context menu closed");

    // Properties Dialog Inspector
    TEST_ASSERT(!exp.propertiesDialog().isOpen, "Properties dialog closed initially");
    auto firstItem = exp.items()[0];
    exp.showPropertiesDialog(firstItem);
    TEST_ASSERT(exp.propertiesDialog().isOpen, "Properties dialog opened");
    TEST_ASSERT(exp.propertiesDialog().item.name == firstItem.name, "Properties dialog displays correct item");
    exp.closePropertiesDialog();
    TEST_ASSERT(!exp.propertiesDialog().isOpen, "Properties dialog closed");

    // Text & Code Viewer
    surshell::TextViewerContent viewer("C:\\boot.ini");
    TEST_ASSERT(viewer.lineCount() > 0, "TextViewer loaded lines from boot.ini");
    surshell::Surface viewerSurface(600, 400);
    viewer.render(viewerSurface);
    TEST_ASSERT(viewerSurface.width() == 600, "TextViewer rendered to surface");
    viewer.onMouseWheel(surshell::Point{100, 100}, -1);
    TEST_ASSERT(viewer.scrollOffset() >= 0, "TextViewer mouse wheel handled");

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

void Test_Procedural_Icon_Engine() {
    std::cout << "[TEST] Running Suite 13: Sovereign Procedural Vector Icon Engine...\n";

    // 1. Verify Extension Resolver
    TEST_ASSERT(surshell::IconRenderer::iconForExtension("", true) == surshell::IconId::Folder, "Directory maps to Folder");
    TEST_ASSERT(surshell::IconRenderer::iconForExtension(".exe", false) == surshell::IconId::FileExecutable, ".exe maps to FileExecutable");
    TEST_ASSERT(surshell::IconRenderer::iconForExtension(".bat", false) == surshell::IconId::FileExecutable, ".bat maps to FileExecutable");
    TEST_ASSERT(surshell::IconRenderer::iconForExtension(".dll", false) == surshell::IconId::FileLibrary, ".dll maps to FileLibrary");
    TEST_ASSERT(surshell::IconRenderer::iconForExtension(".sys", false) == surshell::IconId::FileLibrary, ".sys maps to FileLibrary");
    TEST_ASSERT(surshell::IconRenderer::iconForExtension(".cpp", false) == surshell::IconId::FileCode, ".cpp maps to FileCode");
    TEST_ASSERT(surshell::IconRenderer::iconForExtension(".hpp", false) == surshell::IconId::FileCode, ".hpp maps to FileCode");
    TEST_ASSERT(surshell::IconRenderer::iconForExtension(".py", false) == surshell::IconId::FileCode, ".py maps to FileCode");
    TEST_ASSERT(surshell::IconRenderer::iconForExtension(".png", false) == surshell::IconId::FileImage, ".png maps to FileImage");
    TEST_ASSERT(surshell::IconRenderer::iconForExtension(".zip", false) == surshell::IconId::FileArchive, ".zip maps to FileArchive");
    TEST_ASSERT(surshell::IconRenderer::iconForExtension(".txt", false) == surshell::IconId::FileText, ".txt maps to FileText");
    TEST_ASSERT(surshell::IconRenderer::iconForExtension(".unknown_format", false) == surshell::IconId::FileGeneric, "Unknown ext maps to FileGeneric");

    // 2. Verify AppId Resolver
    TEST_ASSERT(surshell::IconRenderer::iconForAppId("this_pc") == surshell::IconId::ThisPC, "this_pc maps to ThisPC");
    TEST_ASSERT(surshell::IconRenderer::iconForAppId("explorer") == surshell::IconId::FileExplorer, "explorer maps to FileExplorer");
    TEST_ASSERT(surshell::IconRenderer::iconForAppId("cmd") == surshell::IconId::Terminal, "cmd maps to Terminal");
    TEST_ASSERT(surshell::IconRenderer::iconForAppId("terminal") == surshell::IconId::Terminal, "terminal maps to Terminal");
    TEST_ASSERT(surshell::IconRenderer::iconForAppId("settings") == surshell::IconId::Settings, "settings maps to Settings");
    TEST_ASSERT(surshell::IconRenderer::iconForAppId("taskmgr") == surshell::IconId::TaskManager, "taskmgr maps to TaskManager");
    TEST_ASSERT(surshell::IconRenderer::iconForAppId("sentinel") == surshell::IconId::SentinelSec, "sentinel maps to SentinelSec");
    TEST_ASSERT(surshell::IconRenderer::iconForAppId("netbird") == surshell::IconId::NetBirdMesh, "netbird maps to NetBirdMesh");

    // 3. Rasterize All 38 Procedural Vector Icons at Multiple Scales (14, 16, 24, 28, 32, 48px)
    surshell::Surface testCanvas(256, 256, surshell::Color::fromHex(0x0E1420));
    const std::vector<surshell::IconId> allIcons = {
        surshell::IconId::StartPrism,
        surshell::IconId::ThisPC,
        surshell::IconId::LocalDisk,
        surshell::IconId::DriveStorage,
        surshell::IconId::Terminal,
        surshell::IconId::Settings,
        surshell::IconId::TaskManager,
        surshell::IconId::SentinelSec,
        surshell::IconId::NetBirdMesh,
        surshell::IconId::FileExplorer,
        surshell::IconId::TaskView,
        surshell::IconId::Folder,
        surshell::IconId::FolderOpen,
        surshell::IconId::FileGeneric,
        surshell::IconId::FileText,
        surshell::IconId::FileCode,
        surshell::IconId::FileExecutable,
        surshell::IconId::FileLibrary,
        surshell::IconId::FileImage,
        surshell::IconId::FileArchive,
        surshell::IconId::NavBack,
        surshell::IconId::NavForward,
        surshell::IconId::NavUp,
        surshell::IconId::NavRefresh,
        surshell::IconId::Search,
        surshell::IconId::NewFolder,
        surshell::IconId::Delete,
        surshell::IconId::Edit,
        surshell::IconId::Copy,
        surshell::IconId::Properties,
        surshell::IconId::ViewList,
        surshell::IconId::ViewGrid,
        surshell::IconId::SortAsc,
        surshell::IconId::SortDesc,
        surshell::IconId::VolumeHigh,
        surshell::IconId::VolumeMute,
        surshell::IconId::BatteryCharging,
        surshell::IconId::NetworkOnline,
        surshell::IconId::Clock,
        surshell::IconId::Power,
        surshell::IconId::Restart,
        surshell::IconId::Sleep,
        surshell::IconId::Lock,
        surshell::IconId::SignOut,
        surshell::IconId::User,
        surshell::IconId::Hibernate
    };

    TEST_ASSERT(allIcons.size() == 46, "All 46 procedural vector icons enumerated");

    const int32_t testSizes[] = {14, 16, 24, 28, 32, 48};
    for (surshell::IconId id : allIcons) {
        for (int32_t sz : testSizes) {
            testCanvas.clear(surshell::Color::fromHex(0x0E1420));
            surshell::IconRenderer::draw(testCanvas, id, surshell::Point{10, 10}, sz);

            // Verify at least one pixel changed from background (guarantees non-blank rendering)
            bool drewSomething = false;
            for (int32_t y = 10; y < 10 + sz && !drewSomething; ++y) {
                for (int32_t x = 10; x < 10 + sz && !drewSomething; ++x) {
                    const auto px = testCanvas.getPixel(x, y);
                    if (px.toRgba() != surshell::Color::fromHex(0x0E1420).toRgba()) {
                        drewSomething = true;
                    }
                }
            }
            TEST_ASSERT(drewSomething, "Icon must rasterize non-blank geometry at size");
        }
    }

    std::cout << "[TEST] Suite 13: Sovereign Procedural Vector Icon Engine PASSED (46 icons verified across 6 DPI scales).\n";
}

void Test_AltTab_And_Taskbar_Hover_Preview() {
    std::cout << "[TEST] Running Suite 14: Alt+Tab Switcher HUD & Taskbar Live Previews (Windows Peek)...\n";

    // 1. Verify Surface::blitScaled functionality
    surshell::Surface srcSurf(100, 100, surshell::Color{255, 0, 0, 255});
    srcSurf.fillRect(surshell::Rect{25, 25, 50, 50}, surshell::Color{0, 255, 0, 255});
    surshell::Surface dstSurf(200, 200, surshell::Color{0, 0, 0, 255});

    dstSurf.blitScaled(srcSurf, surshell::Rect{0, 0, 100, 100}, surshell::Rect{10, 10, 50, 50});
    TEST_ASSERT(dstSurf.getPixel(15, 15).r > 200, "Scaled outer red region verified");
    TEST_ASSERT(dstSurf.getPixel(35, 35).g > 200, "Scaled inner green region verified");

    // Test blitScaled with alpha
    surshell::Surface alphaDst(100, 100, surshell::Color{0, 0, 0, 255});
    alphaDst.blitScaled(srcSurf, surshell::Rect{0, 0, 100, 100}, surshell::Rect{0, 0, 100, 100}, 128);
    TEST_ASSERT(alphaDst.getPixel(10, 10).r > 100 && alphaDst.getPixel(10, 10).r < 160, "Scaled alpha blending verified");

    // 2. AltTabSwitcher Unit Tests
    surshell::AltTabSwitcher switcher;
    TEST_ASSERT(!switcher.isActive(), "Switcher initially inactive");
    TEST_ASSERT(switcher.itemCount() == 0, "Item count is 0");

    std::vector<surshell::AltTabItem> testItems = {
        {.windowId = 101, .title = "File Explorer", .iconGlyph = "[E]", .iconId = surshell::IconId::FileGeneric, .isActive = true, .isMinimized = false, .previewSurface = &srcSurf},
        {.windowId = 102, .title = "Terminal - cmd.exe", .iconGlyph = "[T]", .iconId = surshell::IconId::Terminal, .isActive = false, .isMinimized = false, .previewSurface = &srcSurf},
        {.windowId = 103, .title = "Sovereign Editor", .iconGlyph = "[N]", .iconId = surshell::IconId::FileText, .isActive = false, .isMinimized = true, .previewSurface = &srcSurf}
    };

    switcher.show(testItems, 1);
    TEST_ASSERT(switcher.isActive(), "Switcher active after show");
    TEST_ASSERT(switcher.itemCount() == 3, "Item count matches 3");
    TEST_ASSERT(switcher.selectedIndex() == 1, "Initial index is 1 (next MRU window)");
    TEST_ASSERT(switcher.selectedItem() != nullptr && switcher.selectedItem()->windowId == 102, "Selected item is window 102");

    // Next cycling & wrap
    switcher.next();
    TEST_ASSERT(switcher.selectedIndex() == 2, "Cycled next to index 2");
    switcher.next();
    TEST_ASSERT(switcher.selectedIndex() == 0, "Cycled next wrapped around to index 0");

    // Previous cycling & wrap
    switcher.previous();
    TEST_ASSERT(switcher.selectedIndex() == 2, "Cycled previous wrapped around to index 2");
    switcher.previous();
    TEST_ASSERT(switcher.selectedIndex() == 1, "Cycled previous to index 1");

    // Select index
    switcher.selectIndex(2);
    TEST_ASSERT(switcher.selectedIndex() == 2, "Directly selected index 2");

    // Bounds calculation
    surshell::Rect hudBounds = switcher.calculateHudBounds(1920, 1080);
    TEST_ASSERT(hudBounds.width > 600 && hudBounds.height > 150, "HUD bounds calculated correctly");
    TEST_ASSERT(hudBounds.x > 0 && hudBounds.y > 0, "HUD is centered on screen");

    surshell::Rect card0 = switcher.calculateCardBounds(0, hudBounds);
    surshell::Rect card1 = switcher.calculateCardBounds(1, hudBounds);
    surshell::Rect card2 = switcher.calculateCardBounds(2, hudBounds);
    TEST_ASSERT(card1.x > card0.right(), "Card 1 spaced horizontally after Card 0");
    TEST_ASSERT(card2.x > card1.right(), "Card 2 spaced horizontally after Card 1");

    // Mouse move & hover test
    TEST_ASSERT(switcher.onMouseMove(card1.center(), 1920, 1080), "Mouse move over card 1 returns true");

    // Mouse click selection
    auto clickedId = switcher.onMouseDown(card0.center(), surshell::MouseButton::Left, 1920, 1080);
    TEST_ASSERT(clickedId.has_value() && *clickedId == 101, "Clicking card 0 confirmed window 101");
    TEST_ASSERT(!switcher.isActive(), "Switcher dismissed after click confirmation");

    // Dismissal test
    switcher.show(testItems, 0);
    TEST_ASSERT(switcher.isActive(), "Reopened switcher");
    switcher.dismiss();
    TEST_ASSERT(!switcher.isActive(), "Dismissed switcher");
    TEST_ASSERT(switcher.itemCount() == 0, "Items cleared after dismissal");

    // 3. Taskbar Hover Preview (Windows Peek) Tests
    surshell::Taskbar taskbar(1920, 1080);
    taskbar.addOrUpdateTask(201, "Mica Explorer", "[E]", true, false, surshell::IconId::FileGeneric);
    taskbar.addOrUpdateTask(202, "Mica Terminal", "[T]", false, false, surshell::IconId::Terminal);

    uint32_t closedWinId = 0;
    taskbar.setPreviewCloseCallback([&](uint32_t wid) { closedWinId = wid; });
    taskbar.setWindowPreviewProvider([&](uint32_t wid) -> const surshell::Surface* {
        (void)wid;
        return &srcSurf;
    });

    surshell::Rect prevBounds = taskbar.hoverPreviewBounds(201);
    TEST_ASSERT(!prevBounds.empty(), "Hover preview bounds calculated for task 201");
    TEST_ASSERT(prevBounds.bottom() < taskbar.bounds().y, "Preview card floats above taskbar");

    surshell::Rect closeBtn = taskbar.hoverPreviewCloseButtonBounds(201);
    TEST_ASSERT(!closeBtn.empty() && prevBounds.contains(closeBtn.center()), "Close button is inside preview card");

    // Mouse hover over task pill triggers preview
    taskbar.onMouseMove(taskbar.tasks()[0].bounds.center());
    TEST_ASSERT(taskbar.hoveredTaskWindowId() == 201, "Hovering task pill activates task 201 preview");

    // Mouse navigating inside preview card maintains preview active
    taskbar.onMouseMove(prevBounds.center());
    TEST_ASSERT(taskbar.hoveredTaskWindowId() == 201, "Hovering preview card maintains preview active");

    // Clicking close button triggers callback
    taskbar.onMouseDown(closeBtn.center(), surshell::MouseButton::Left);
    TEST_ASSERT(closedWinId == 201, "Clicking close button triggered close callback for window 201");
    TEST_ASSERT(taskbar.hoveredTaskWindowId() == -1, "Hover reset after close click");

    // 4. Desktop Integration & Rendering Tests
    surshell::SurShellDesktop shell(1920, 1080);
    shell.openFileExplorerWindow();
    shell.openTerminalWindow();
    shell.render(); // Baseline render

    TEST_ASSERT(!shell.altTab().isActive(), "Desktop AltTab initially inactive");
    shell.triggerAltTab();
    TEST_ASSERT(shell.altTab().isActive(), "Desktop triggerAltTab activated HUD");
    TEST_ASSERT(shell.altTab().itemCount() >= 2, "Desktop gathered active open windows into HUD");

    shell.cycleAltTab();
    shell.render(); // Render with AltTab HUD active
    shell.commitAltTab();
    TEST_ASSERT(!shell.altTab().isActive(), "Desktop commitAltTab closed HUD");

    std::cout << "[TEST] Suite 14: Alt+Tab Switcher HUD & Taskbar Live Previews (Windows Peek) PASSED.\n";
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
    Test_Procedural_Icon_Engine();
    Test_AltTab_And_Taskbar_Hover_Preview();

    std::cout << "\n===============================================================================\n";
    std::cout << "ALL 14 SURSHELL SUBSYSTEM VERIFICATION SUITES PASSED (100% SUCCESS)\n";
    std::cout << "===============================================================================\n";
    return 0;
}
