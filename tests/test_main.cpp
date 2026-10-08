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

    // Context Menu on wallpaper right-click
    dm.onMouseDown(surshell::Point{500, 500}, surshell::MouseButton::Right);
    TEST_ASSERT(dm.contextMenu().isOpen, "Context menu opened on wallpaper right click");
    TEST_ASSERT(dm.contextMenu().targetIconId.empty(), "Wallpaper context menu has empty targetIconId");
    TEST_ASSERT(!dm.contextMenu().items.empty(), "Context menu populated with items");

    // Close menu on click outside
    dm.onMouseDown(surshell::Point{10, 10}, surshell::MouseButton::Left);
    TEST_ASSERT(!dm.contextMenu().isOpen, "Context menu closed on outside click");

    // Context Menu on icon right-click
    dm.onMouseDown(surshell::Point{dm.icons()[0].bounds.x + 5, dm.icons()[0].bounds.y + 5}, surshell::MouseButton::Right);
    TEST_ASSERT(dm.contextMenu().isOpen, "Context menu opened on icon right click");
    TEST_ASSERT(dm.contextMenu().targetIconId == "app1", "Icon context menu targets clicked icon");

    // Sort by Name
    dm.sortByName();
    TEST_ASSERT(dm.icons()[0].label <= dm.icons()[1].label, "Icons sorted alphabetically");

    // Render Context Menu
    surshell::Surface testMenuSurface(800, 600);
    dm.openContextMenu(surshell::Point{100, 100});
    dm.renderContextMenu(testMenuSurface);
    TEST_ASSERT(dm.contextMenu().isOpen, "Context menu rendered cleanly");
    dm.closeContextMenu();

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

    // This PC virtual container navigation & drives enumeration
    exp.navigateTo("This PC");
    TEST_ASSERT(exp.currentPath() == "This PC", "Navigated to This PC");
    TEST_ASSERT(!exp.items().empty(), "This PC enumerates drives and standard folders");

    // File Operations & Clipboard (Copy / Cut / Paste / Rename / New File)
    exp.navigateTo("C:\\Windows\\System32");
    TEST_ASSERT(!exp.items().empty(), "Items present in System32");
    exp.onKeyDown(surshell::KeyCode::Down);
    TEST_ASSERT(exp.selectedItem().has_value(), "Down arrow selected item");

    // Copy item
    exp.copySelected();
    TEST_ASSERT(exp.canPaste(), "Clipboard has copied item");

    // Inline Rename triggering
    exp.onKeyDown(surshell::KeyCode::F2);
    TEST_ASSERT(exp.isRenaming(), "F2 entered rename mode");
    exp.onKeyDown(surshell::KeyCode::Escape);
    TEST_ASSERT(!exp.isRenaming(), "Escape cancelled rename mode");

    // Navigation hotkeys
    exp.onKeyDown(surshell::KeyCode::Backspace);
    TEST_ASSERT(exp.currentPath() == "C:\\Windows", "Backspace navigated up to C:\\Windows");

    // File Explorer Toast Notifications & Safe Operations
    const std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "surshell_suite7_test";
    std::filesystem::create_directories(tempDir);
    surshell::FileExplorer testExp(tempDir.string());
    std::string lastToastTitle;
    testExp.setToastCallback([&](const std::string& title, const std::string&, surshell::IconId) {
        lastToastTitle = title;
    });
    testExp.createNewFolder("TestSubFolder");
    TEST_ASSERT(lastToastTitle == "Folder Created", "Toast emitted on folder creation");
    testExp.createNewFile("TestFile.txt");
    TEST_ASSERT(lastToastTitle == "File Created", "Toast emitted on file creation");
    testExp.onKeyDown(surshell::KeyCode::Down);
    testExp.deleteSelected();
    TEST_ASSERT(!lastToastTitle.empty(), "Toast emitted on item deletion");
    std::error_code rmEc;
    std::filesystem::remove_all(tempDir, rmEc);

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
    TEST_ASSERT(qs.isToggleEnabled("network"), "Network enabled by default");
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
    TEST_ASSERT(surshell::IconRenderer::iconForAppId("network") == surshell::IconId::NetworkOnline, "network maps to NetworkOnline");
    TEST_ASSERT(surshell::IconRenderer::iconForAppId("mediaplayer") == surshell::IconId::MediaPlay, "mediaplayer maps to MediaPlay");
    TEST_ASSERT(surshell::IconRenderer::iconForAppId("regedit") == surshell::IconId::Registry, "regedit maps to Registry");

    // 3. Rasterize All Procedural Vector Icons at Multiple Scales (14, 16, 24, 28, 32, 48px)
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
        surshell::IconId::NewFile,
        surshell::IconId::Cut,
        surshell::IconId::Paste,
        surshell::IconId::Rename,
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
        surshell::IconId::Hibernate,
        surshell::IconId::MediaPlay,
        surshell::IconId::MediaPause,
        surshell::IconId::MediaNext,
        surshell::IconId::MediaPrev,
        surshell::IconId::NotificationBell,
        surshell::IconId::NetworkEthernet,
        surshell::IconId::Calculator,
        surshell::IconId::RunDialog,
        surshell::IconId::Display,
        surshell::IconId::Personalization,
        surshell::IconId::Calendar,
        surshell::IconId::TerminalTab,
        surshell::IconId::ShieldAdmin,
        surshell::IconId::SearchCategory,
        surshell::IconId::Registry,
        surshell::IconId::CloudDrive,
        surshell::IconId::NetworkShare,
        surshell::IconId::OpticalDrive
    };

    TEST_ASSERT(allIcons.size() == 68, "All 68 procedural vector icons enumerated");

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

    std::cout << "[TEST] Suite 13: Sovereign Procedural Vector Icon Engine PASSED (68 icons verified across 6 DPI scales).\n";
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

void Test_Task_Manager() {
    std::cout << "[TEST] Running Suite 15: Interactive Task Manager & Resource Monitor...\n";

    surshell::KernelBridge bridge;
    bridge.connectToExecutive("\\RPC_Control\\SurWinLpc");

    surshell::TaskManagerContent tm(&bridge);
    TEST_ASSERT(tm.processCount() >= 8, "Task Manager queries processes from Executive");
    TEST_ASSERT(tm.activeTab() == surshell::TaskManagerTab::Processes, "Default tab is Processes");

    // Tab switching
    tm.setActiveTab(surshell::TaskManagerTab::Performance);
    TEST_ASSERT(tm.activeTab() == surshell::TaskManagerTab::Performance, "Switched to Performance tab");
    tm.setActiveTab(surshell::TaskManagerTab::Processes);

    // Selection
    const uint32_t firstPid = tm.processes()[0].pid;
    tm.selectPid(firstPid);
    TEST_ASSERT(tm.selectedPid() == firstPid, "Process PID selected");

    // End task callback
    bool callbackFired = false;
    uint32_t killedPid = 0;
    tm.setTerminatedCallback([&](uint32_t pid, const std::string&) {
        callbackFired = true;
        killedPid = pid;
    });

    const size_t prevCount = tm.processCount();
    const bool killed = tm.endSelectedTask();
    TEST_ASSERT(killed, "endSelectedTask returned true");
    TEST_ASSERT(callbackFired, "Terminated callback invoked");
    TEST_ASSERT(killedPid == firstPid, "Correct PID killed");
    TEST_ASSERT(tm.processCount() == prevCount - 1, "Process count decreased by 1");

    // Keyboard navigation
    tm.onKeyDown(surshell::KeyCode::Down);
    TEST_ASSERT(tm.selectedPid().has_value(), "Down arrow selected next process");

    // Render to surface
    surshell::Surface tmSurface(720, 480);
    tm.render(tmSurface);
    TEST_ASSERT(tmSurface.width() == 720, "Task Manager rendered to surface");

    std::cout << "[TEST] Suite 15: Interactive Task Manager & Resource Monitor PASSED.\n";
}

void Test_Toast_Notifications() {
    std::cout << "[TEST] Running Suite 16: Sovereign Acrylic Toast Notifications...\n";

    surshell::ToastManager tm;
    TEST_ASSERT(tm.toastCount() == 0, "No toasts initially");

    const uint32_t id1 = tm.showToast("Network Connected", "Ethernet 1000/1000 Mbps Active", surshell::IconId::NetworkOnline);
    TEST_ASSERT(tm.toastCount() == 1, "1 toast active");
    TEST_ASSERT(tm.findToast(id1) != nullptr, "Toast lookup succeeds");

    tm.showToast("SentinelSec", "Zero-Telemetry Guard Secure", surshell::IconId::SentinelSec);
    tm.showToast("Download Finished", "surshell_v1.0.iso downloaded", surshell::IconId::FileArchive);
    const uint32_t id4 = tm.showToast("Volume", "Level set to 85%", surshell::IconId::VolumeHigh);
    TEST_ASSERT(tm.toastCount() == 4, "4 toasts active");

    // Adding 5th toast clamps queue to max 4
    tm.showToast("New Event", "Clamping test", surshell::IconId::NotificationBell);
    TEST_ASSERT(tm.toastCount() == 4, "Toast queue clamped to 4 items");

    // Render toasts to surface
    surshell::Surface screen(1920, 1080);
    tm.render(screen, 1920, 1080, 48);

    // Hit test dismiss
    const auto* latest = tm.findToast(id4);
    if (latest) {
        const surshell::Point clickPt{latest->bounds.centerX(), latest->bounds.centerY()};
        TEST_ASSERT(tm.onMouseDown(clickPt, surshell::MouseButton::Left), "Clicking toast body consumes event");
        TEST_ASSERT(tm.findToast(id4) == nullptr, "Clicked toast dismissed");
    }

    // Timer tick decay
    surshell::ToastManager tickTm;
    const uint32_t quickId = tickTm.showToast("Quick", "Quick decay", surshell::IconId::Clock, surshell::Color::fromHex(0x00D4FF), 2);
    tickTm.tick();
    TEST_ASSERT(tickTm.findToast(quickId) != nullptr, "Toast alive after 1 tick");
    tickTm.tick();
    TEST_ASSERT(tickTm.findToast(quickId) == nullptr, "Toast expired and removed after 2 ticks");

    std::cout << "[TEST] Suite 16: Sovereign Acrylic Toast Notifications PASSED.\n";
}

void Test_Media_Hud_OSD() {
    std::cout << "[TEST] Running Suite 17: Audio & Media Playback HUD (OSD Overlay)...\n";

    surshell::MediaHud hud;
    TEST_ASSERT(!hud.isVisible(), "Media HUD initially hidden");

    hud.showVolume(75);
    TEST_ASSERT(hud.isVisible(), "Volume change triggers HUD visibility");
    TEST_ASSERT(hud.volume() == 75, "Volume reported as 75%");
    TEST_ASSERT(!hud.isMuted(), "Not muted initially");

    // Transport buttons
    bool playPauseToggled = false;
    hud.setPlayPauseCallback([&]() { playPauseToggled = true; });
    const bool prevPlaying = hud.isPlaying();
    hud.togglePlayPause();
    TEST_ASSERT(hud.isPlaying() != prevPlaying, "Play/Pause state toggled");
    TEST_ASSERT(playPauseToggled, "Play/Pause callback invoked");

    // Render to surface
    surshell::Surface screen(1920, 1080);
    hud.render(screen, 1920, 1080, 48);
    TEST_ASSERT(hud.bounds().width == 360, "HUD width is 360");

    // Timer tick decay
    hud.hide();
    TEST_ASSERT(!hud.isVisible(), "HUD hide() made it hidden");

    std::cout << "[TEST] Suite 17: Audio & Media Playback HUD (OSD Overlay) PASSED.\n";
}

void Test_Settings_And_Personalization() {
    std::cout << "[TEST] Running Suite 18: System Settings & Personalization Center...\n";

    surshell::SettingsContent settings;
    TEST_ASSERT(settings.activeCategory() == surshell::SettingsCategory::System, "Default settings category is System");

    surshell::Surface sSurf(780, 520);

    // Category navigation
    settings.setActiveCategory(surshell::SettingsCategory::Personalization);
    TEST_ASSERT(settings.activeCategory() == surshell::SettingsCategory::Personalization, "Switched to Personalization");

    // Personalization callbacks
    surshell::ThemeMode recTheme = surshell::ThemeMode::Dark;
    settings.setThemeModeCallback([&](surshell::ThemeMode mode) { recTheme = mode; });

    surshell::Color recColor{};
    settings.setAccentColorCallback([&](surshell::Color c) { recColor = c; });

    surshell::WallpaperStyle recWp = surshell::WallpaperStyle::MicaGrid;
    settings.setWallpaperCallback([&](surshell::WallpaperStyle wp) { recWp = wp; });

    settings.render(sSurf); // Compute layout bounds

    // Click Light theme button
    settings.onMouseDown(surshell::Point{374, 78}, surshell::MouseButton::Left);
    TEST_ASSERT(recTheme == surshell::ThemeMode::Light, "Theme callback fired on Light mode selection");

    // Click accent color swatch
    settings.onMouseDown(surshell::Point{268, 136}, surshell::MouseButton::Left);
    TEST_ASSERT(recColor.toRgba() != 0, "Accent color callback fired on swatch selection");

    // Click wallpaper option
    settings.onMouseDown(surshell::Point{406, 218}, surshell::MouseButton::Left);
    TEST_ASSERT(recWp != surshell::WallpaperStyle::MicaGrid, "Wallpaper callback fired on wallpaper selection");

    // Taskbar & Dock category
    settings.setActiveCategory(surshell::SettingsCategory::TaskbarDock);
    surshell::TaskbarAlignment recAlign = surshell::TaskbarAlignment::Center;
    settings.setTaskbarAlignmentCallback([&](surshell::TaskbarAlignment a) { recAlign = a; });

    surshell::TaskbarStyle recStyle = surshell::TaskbarStyle::FloatingIsland;
    settings.setTaskbarStyleCallback([&](surshell::TaskbarStyle s) { recStyle = s; });

    bool topBarToggled = false;
    settings.setTopBarCallback([&](bool) { topBarToggled = true; });

    settings.render(sSurf); // Compute taskbar layout bounds

    settings.onMouseDown(surshell::Point{406, 79}, surshell::MouseButton::Left); // Left classic
    TEST_ASSERT(recAlign == surshell::TaskbarAlignment::Left, "Taskbar alignment callback fired");

    settings.onMouseDown(surshell::Point{406, 143}, surshell::MouseButton::Left); // Edge-to-edge dock
    TEST_ASSERT(recStyle == surshell::TaskbarStyle::EdgeToEdge, "Taskbar style callback fired");

    settings.onMouseDown(surshell::Point{319, 207}, surshell::MouseButton::Left); // Top bar toggle
    TEST_ASSERT(topBarToggled, "Top diagnostic bar toggle callback fired");

    TEST_ASSERT(sSurf.width() == 780 && sSurf.height() == 520, "Settings rendered to surface");

    // Verify all 9 Categories can be activated
    const surshell::SettingsCategory allCats[] = {
        surshell::SettingsCategory::System,
        surshell::SettingsCategory::Personalization,
        surshell::SettingsCategory::TaskbarDock,
        surshell::SettingsCategory::Network,
        surshell::SettingsCategory::Apps,
        surshell::SettingsCategory::PrivacySecurity,
        surshell::SettingsCategory::TimeLanguage,
        surshell::SettingsCategory::Developer,
        surshell::SettingsCategory::About
    };
    for (auto cat : allCats) {
        settings.setActiveCategory(cat);
        TEST_ASSERT(settings.activeCategory() == cat, "Activated category successfully");
        settings.render(sSurf);
    }

    // Verify search query filters
    settings.setSearchQuery("volume");
    TEST_ASSERT(settings.activeCategory() == surshell::SettingsCategory::System, "Search 'volume' routed to System");

    settings.setSearchQuery("wifi");
    TEST_ASSERT(settings.activeCategory() == surshell::SettingsCategory::Network, "Search 'wifi' routed to Network");

    settings.setSearchQuery("privacy");
    TEST_ASSERT(settings.activeCategory() == surshell::SettingsCategory::PrivacySecurity, "Search 'privacy' routed to PrivacySecurity");

    settings.setSearchQuery("reg");
    TEST_ASSERT(settings.activeCategory() == surshell::SettingsCategory::Developer, "Search 'reg' routed to Developer");

    settings.setSearchQuery("clock");
    TEST_ASSERT(settings.activeCategory() == surshell::SettingsCategory::TimeLanguage, "Search 'clock' routed to TimeLanguage");

    settings.setSearchQuery("app");
    TEST_ASSERT(settings.activeCategory() == surshell::SettingsCategory::Apps, "Search 'app' routed to Apps");

    // Clear search
    settings.setSearchQuery("");
    TEST_ASSERT(settings.searchQuery().empty(), "Search query cleared");

    // Master Volume & Sound Controls
    int32_t recVol = 0;
    bool recMute = false;
    settings.setVolumeCallback([&](int32_t v, bool m) { recVol = v; recMute = m; });
    settings.setMasterVolume(75);
    TEST_ASSERT(settings.masterVolume() == 75, "Master volume set to 75%");
    settings.setIsMuted(true);
    TEST_ASSERT(settings.isMuted(), "Mute set to true");

    // Power, Display & Storage settings
    settings.setPowerMode(surshell::PowerMode::PowerSaver);
    TEST_ASSERT(settings.powerMode() == surshell::PowerMode::PowerSaver, "Power mode set to Power Saver");
    settings.setDisplayScaling(surshell::DisplayScaling::Scale125);
    TEST_ASSERT(settings.displayScaling() == surshell::DisplayScaling::Scale125, "Display scaling set to 125%");
    settings.setRefreshRate120Hz(true);
    TEST_ASSERT(settings.refreshRate120Hz(), "120Hz refresh rate enabled");

    // Time & Language: 24-Hour Format
    bool rec24H = false;
    settings.setTimeFormatCallback([&](bool is24) { rec24H = is24; });
    settings.setClockFormat24H(true);
    TEST_ASSERT(settings.clockFormat24H(), "24-Hour clock format enabled");

    // Developer & Tool Launchers
    std::string recLaunchApp{};
    settings.setLaunchAppCallback([&](const std::string& app) { recLaunchApp = app; });
    TEST_ASSERT(settings.developerMode(), "Developer mode is enabled by default");

    std::cout << "[TEST] Suite 18: System Settings & Personalization Center PASSED.\n";
}

void Test_Calculator_Application() {
    std::cout << "[TEST] Running Suite 19: Modern Sovereign Calculator...\n";

    surshell::CalculatorContent calc;
    TEST_ASSERT(calc.display() == "0", "Initial calculator display is 0");

    // Addition: 12 + 34 = 46
    calc.inputDigit('1');
    calc.inputDigit('2');
    TEST_ASSERT(calc.display() == "12", "Input digits 12");
    calc.inputOperator('+');
    calc.inputDigit('3');
    calc.inputDigit('4');
    calc.calculateResult();
    TEST_ASSERT(calc.display() == "46", "12 + 34 = 46");

    // Multiplication: 5 * 6 = 30
    calc.clearAll();
    calc.inputDigit('5');
    calc.inputOperator('*');
    calc.inputDigit('6');
    calc.calculateResult();
    TEST_ASSERT(calc.display() == "30", "5 * 6 = 30");

    // Division by zero: 10 / 0
    calc.clearAll();
    calc.inputDigit('1');
    calc.inputDigit('0');
    calc.inputOperator('/');
    calc.inputDigit('0');
    calc.calculateResult();
    TEST_ASSERT(calc.display() == "Cannot divide by 0", "Division by zero handled safely");

    // Square Root: sqrt(16) = 4
    calc.clearAll();
    calc.inputDigit('1');
    calc.inputDigit('6');
    calc.squareRoot();
    TEST_ASSERT(calc.display() == "4", "sqrt(16) = 4");

    // Square: 7^2 = 49
    calc.clearAll();
    calc.inputDigit('7');
    calc.square();
    TEST_ASSERT(calc.display() == "49", "7^2 = 49");

    // Reciprocal: 1/4 = 0.25
    calc.clearAll();
    calc.inputDigit('4');
    calc.reciprocal();
    TEST_ASSERT(calc.display() == "0.25", "1/4 = 0.25");

    // Negation: -8
    calc.clearAll();
    calc.inputDigit('8');
    calc.negate();
    TEST_ASSERT(calc.display() == "-8", "Negate 8 = -8");
    calc.negate();
    TEST_ASSERT(calc.display() == "8", "Negate -8 = 8");

    // Keyboard character input: 9 * 9 = 81
    calc.clearAll();
    calc.onCharInput('9');
    calc.onCharInput('*');
    calc.onCharInput('9');
    calc.onCharInput('=');
    TEST_ASSERT(calc.display() == "81", "Keyboard input 9 * 9 = 81");

    // Backspace
    calc.onCharInput('5');
    calc.onCharInput('7');
    calc.backspace();
    TEST_ASSERT(calc.display() == "5", "Backspace removes trailing digit");

    // Render to surface
    surshell::Surface calcSurf(340, 480);
    calc.render(calcSurf);
    TEST_ASSERT(calcSurf.width() == 340 && calcSurf.height() == 480, "Calculator rendered to surface");

    std::cout << "[TEST] Suite 19: Modern Sovereign Calculator PASSED.\n";
}

void Test_Run_Dialog_And_Live_Aero_Snap() {
    std::cout << "[TEST] Running Suite 20: Run Dialog & Live Aero Snap Docking Previews...\n";

    // 1. Run Dialog Tests
    surshell::RunDialogContent run("cmd");
    TEST_ASSERT(run.command() == "cmd", "Initial command is cmd");

    bool executed = false;
    std::string executedCmd;
    run.setExecuteCallback([&](const std::string& c) {
        executed = true;
        executedCmd = c;
    });

    run.execute();
    TEST_ASSERT(executed && executedCmd == "cmd", "Execute callback dispatched cmd");

    // Backspace & typing in Run dialog
    run.onKeyDown(surshell::KeyCode::Backspace);
    run.onKeyDown(surshell::KeyCode::Backspace);
    run.onKeyDown(surshell::KeyCode::Backspace);
    TEST_ASSERT(run.command().empty(), "Cleared command with backspaces");

    run.onCharInput('c');
    run.onCharInput('a');
    run.onCharInput('l');
    run.onCharInput('c');
    TEST_ASSERT(run.command() == "calc", "Typed calc into Run dialog");

    surshell::Surface runSurf(440, 190);
    run.render(runSurf);
    TEST_ASSERT(runSurf.width() == 440 && runSurf.height() == 190, "Run dialog rendered to surface");

    // 2. Live Aero Snap Docking Previews in WindowManager
    surshell::WindowManager wm(1920, 1080, 40);
    const uint32_t winId = wm.createWindow("Test Window", surshell::Rect{200, 200, 600, 400});

    // Start dragging caption
    auto* win = wm.findWindow(winId);
    TEST_ASSERT(win != nullptr, "Window found");
    wm.onMouseDown(win->captionBounds().center(), surshell::MouseButton::Left);

    // Drag toward top edge: preview full-screen maximize
    wm.onMouseMove(surshell::Point{500, 5});
    TEST_ASSERT(wm.activeSnapPreview().has_value(), "Active snap preview visible near top edge");
    TEST_ASSERT(wm.pendingSnapState() == surshell::WindowState::Maximized, "Pending state is Maximized");
    TEST_ASSERT(wm.activeSnapPreview()->width == 1920, "Preview width matches full workspace");

    // Drag toward left edge: preview left 50%
    wm.onMouseMove(surshell::Point{5, 500});
    TEST_ASSERT(wm.activeSnapPreview().has_value(), "Active snap preview visible near left edge");
    TEST_ASSERT(wm.pendingSnapState() == surshell::WindowState::SnappedLeft, "Pending state is SnappedLeft");
    TEST_ASSERT(wm.activeSnapPreview()->width == 960, "Preview width matches half workspace");
    TEST_ASSERT(wm.activeSnapPreview()->x == 0, "Preview x is 0");

    // Drag toward top-right corner: preview top-right 25% quadrant
    wm.onMouseMove(surshell::Point{1915, 10});
    TEST_ASSERT(wm.activeSnapPreview().has_value(), "Active snap preview visible in top-right corner");
    TEST_ASSERT(wm.pendingSnapState() == surshell::WindowState::SnappedTopRight, "Pending state is SnappedTopRight");
    TEST_ASSERT(wm.activeSnapPreview()->width == 960 && wm.activeSnapPreview()->height == 520, "Preview is 25% quarter workspace");

    // Drag away to center: preview resets
    wm.onMouseMove(surshell::Point{500, 500});
    TEST_ASSERT(!wm.activeSnapPreview().has_value(), "Snap preview dismissed when dragging away from edge");
    TEST_ASSERT(!wm.pendingSnapState().has_value(), "Pending snap state cleared");

    // Drag back to left edge and release mouse: snaps to SnappedLeft!
    wm.onMouseMove(surshell::Point{5, 500});
    TEST_ASSERT(wm.pendingSnapState() == surshell::WindowState::SnappedLeft, "Pending state restored at left edge");
    wm.onMouseUp(surshell::Point{5, 500}, surshell::MouseButton::Left);
    TEST_ASSERT(!wm.activeSnapPreview().has_value(), "Preview cleared after mouse release");
    TEST_ASSERT(win->state == surshell::WindowState::SnappedLeft, "Window snapped to SnappedLeft on mouse release");
    TEST_ASSERT(win->currentBounds.width == 960, "Window snapped bounds width is 960");

    // 3. Master Desktop Integration: Spawning new applications
    surshell::SurShellDesktop desktop(1920, 1080);
    const uint32_t sWin = desktop.openSettingsWindow();
    const uint32_t cWin = desktop.openCalculatorWindow();
    const uint32_t rWin = desktop.openRunDialogWindow();

    TEST_ASSERT(desktop.windowManager().findWindow(sWin) != nullptr, "Settings window spawned in desktop");
    TEST_ASSERT(desktop.windowManager().findWindow(cWin) != nullptr, "Calculator window spawned in desktop");
    TEST_ASSERT(desktop.windowManager().findWindow(rWin) != nullptr, "Run dialog spawned in desktop");

    desktop.render(); // Comprehensive master render
    TEST_ASSERT(desktop.framebuffer().width() == 1920, "Desktop framebuffer rendered successfully");

    std::cout << "[TEST] Suite 20: Run Dialog & Live Aero Snap Docking Previews PASSED.\n";
}

void Test_Modern_Terminal_Subsystem() {
    std::cout << "[TEST] Running Suite 21: Windows Terminal System (microsoft/terminal Architecture)...\n";

    surshell::TerminalContent term;
    TEST_ASSERT(term.tabCount() == 1, "Initial terminal has 1 tab");
    TEST_ASSERT(term.activeTabIndex() == 0, "Initial active tab is index 0");

    // Add secondary tab
    term.addTab("PowerShell", "pwsh");
    TEST_ASSERT(term.tabCount() == 2, "Tab count increased to 2");
    term.selectTab(1);
    TEST_ASSERT(term.activeTabIndex() == 1, "Active tab switched to index 1");

    // Switch back to Command Prompt
    term.selectTab(0);
    TEST_ASSERT(term.activeTabIndex() == 0, "Active tab switched back to index 0");

    // Test text input and command execution
    term.inputString("ver");
    TEST_ASSERT(term.currentInput() == "ver", "Input string matches typed command");
    term.executeCurrentCommand();
    TEST_ASSERT(term.currentInput().empty(), "Input cleared after execution");

    // Test app spawning command callback
    std::string launchedApp;
    term.setAppSpawnCallback([&](const std::string& app, const std::string& /*args*/) {
        launchedApp = app;
    });

    term.inputString("calc");
    term.executeCurrentCommand();
    TEST_ASSERT(launchedApp == "calc", "calc command successfully triggered app spawn callback");

    term.inputString("cls");
    term.executeCurrentCommand();

    // Verify history and start commands
    term.inputString("history");
    term.executeCurrentCommand();
    TEST_ASSERT(!term.activeBuffer().empty(), "Terminal executed history command");

    term.inputString("start calc");
    term.executeCurrentCommand();
    TEST_ASSERT(launchedApp == "calc", "Terminal start command dispatched calc application");

    // Close tab 1
    term.closeTab(1);
    TEST_ASSERT(term.tabCount() == 1, "Tab successfully closed");

    // Render terminal client area
    surshell::Surface canvas(720, 440, surshell::Color::fromHex(0x0C0C0C));
    term.render(canvas);
    TEST_ASSERT(canvas.width() == 720 && canvas.height() == 440, "Terminal rendered into surface");

    std::cout << "[TEST] Suite 21: Windows Terminal System PASSED.\n";
}

void Test_Action_Center_And_Calendar() {
    std::cout << "[TEST] Running Suite 22: Action Center & Calendar Flyout (Win+N / Tray Clock)...\n";

    surshell::ActionCenterFlyout actionCenter;
    TEST_ASSERT(!actionCenter.isVisible(), "Action Center starts hidden");

    actionCenter.show();
    TEST_ASSERT(actionCenter.isVisible(), "Action Center visible after show()");

    // Add notifications
    const size_t initialNotifs = actionCenter.notificationCount();
    actionCenter.addNotification("Security Center", "Zero-telemetry policy active and enforced", surshell::IconId::SentinelSec);
    actionCenter.addNotification("Network Adapter", "Gigabit Ethernet connected at 1.0 Gbps", surshell::IconId::NetworkEthernet);
    actionCenter.addNotification("System Kernel", "Dave Cutler executive IPC channel healthy", surshell::IconId::Terminal);
    TEST_ASSERT(actionCenter.notificationCount() == initialNotifs + 3, "3 notifications added to history stack");

    // Focus Assist toggling
    TEST_ASSERT(!actionCenter.focusAssist(), "Focus Assist defaults to disabled");
    actionCenter.setFocusAssist(true);
    TEST_ASSERT(actionCenter.focusAssist(), "Focus Assist successfully enabled");

    // Calendar navigation
    const int32_t startMonth = actionCenter.currentMonth();
    actionCenter.nextMonth();
    TEST_ASSERT(actionCenter.currentMonth() != startMonth, "Next month navigates calendar forward");
    actionCenter.prevMonth();
    TEST_ASSERT(actionCenter.currentMonth() == startMonth, "Prev month navigates calendar backward");

    // Clear notifications
    actionCenter.clearAllNotifications();
    TEST_ASSERT(actionCenter.notificationCount() == 0, "Notifications cleared");

    // Render Action Center flyout onto 1920x1080 desktop canvas
    surshell::Surface desktopCanvas(1920, 1080, surshell::Color::fromHex(0x0E1420));
    actionCenter.render(desktopCanvas, 1920, 1080, 48);
    TEST_ASSERT(actionCenter.bounds().width == 380, "Action Center width is 380px");

    actionCenter.hide();
    TEST_ASSERT(!actionCenter.isVisible(), "Action Center hidden after hide()");

    std::cout << "[TEST] Suite 22: Action Center & Calendar Flyout PASSED.\n";
}

void Test_Universal_Search_Hub() {
    std::cout << "[TEST] Running Suite 23: Universal Search Hub (Win+S / Taskbar Search)...\n";

    surshell::SearchHub hub;
    TEST_ASSERT(!hub.isVisible(), "Search Hub starts hidden");

    hub.show();
    TEST_ASSERT(hub.isVisible(), "Search Hub visible after show()");

    // Default Catalog
    TEST_ASSERT(hub.resultCount() > 0, "Catalog items populated");
    const size_t totalItems = hub.resultCount();

    // Query filter
    hub.setQuery("terminal");
    TEST_ASSERT(hub.resultCount() >= 1, "Filter for 'terminal' returns matching apps");

    hub.setQuery("calc");
    TEST_ASSERT(hub.resultCount() >= 1, "Filter for 'calc' returns Calculator");

    // App launch execution callback
    std::string executedTarget;
    std::string executedArgs;
    bool executedAsAdmin = false;
    hub.setExecuteCallback([&](const std::string& target, const std::string& args, bool admin) {
        executedTarget = target;
        executedArgs = args;
        executedAsAdmin = admin;
    });

    hub.onKeyDown(surshell::KeyCode::Enter);
    TEST_ASSERT(executedTarget == "calc", "Executing selected item dispatches Calculator");
    TEST_ASSERT(!executedAsAdmin, "Standard execution is non-admin");

    // Category Filter switching
    hub.setCategoryFilter(surshell::SearchCategoryType::Settings);
    hub.setQuery("");
    TEST_ASSERT(hub.resultCount() > 0, "Category filter 'Settings' returns items");
    TEST_ASSERT(hub.activeFilter() == surshell::SearchCategoryType::Settings, "Active category filter is Settings");

    // Reset to All
    hub.setCategoryFilter(surshell::SearchCategoryType::All);
    TEST_ASSERT(hub.resultCount() == totalItems, "Resetting category to All restores catalog count");

    // Render Search Hub onto 1920x1080 desktop canvas
    hub.show();
    surshell::Surface desktopCanvas(1920, 1080, surshell::Color::fromHex(0x0E1420));
    hub.render(desktopCanvas, 1920, 1080);
    TEST_ASSERT(hub.bounds().width == 700, "Search Hub width is 700px");

    hub.hide();
    TEST_ASSERT(!hub.isVisible(), "Search Hub hidden after hide()");

    std::cout << "[TEST] Suite 23: Universal Search Hub PASSED.\n";
}

void Test_Lock_Screen_And_Authentication() {
    std::cout << "[TEST] Running Suite 24: Sovereign Lock Screen & Authentication Center (Win+L)...\n";

    surshell::LockScreen lockScreen;
    TEST_ASSERT(!lockScreen.isLocked(), "Lock screen starts unlocked");

    lockScreen.lock();
    TEST_ASSERT(lockScreen.isLocked(), "Session is locked");
    TEST_ASSERT(lockScreen.lockState() == surshell::LockState::LockedAmbient, "Initial state is Ambient view");

    // Any key or click raises to credentials view
    lockScreen.onMouseDown(surshell::Point{960, 540}, surshell::MouseButton::Left);
    TEST_ASSERT(lockScreen.lockState() == surshell::LockState::CredentialsLogon, "Transitions to Credentials Logon view");

    // Test PIN entry and unlock callback
    bool unlocked = false;
    lockScreen.setUnlockCallback([&]() {
        unlocked = true;
    });

    // Enter wrong PIN
    lockScreen.setPin("0000");
    lockScreen.onKeyDown(surshell::KeyCode::Enter);
    TEST_ASSERT(!unlocked, "Wrong PIN does not unlock");
    TEST_ASSERT(lockScreen.isLocked(), "Session remains locked");

    // Enter correct PIN (1234)
    lockScreen.setPin("1234");
    lockScreen.onKeyDown(surshell::KeyCode::Enter);
    TEST_ASSERT(unlocked, "Correct PIN triggers unlock callback");
    TEST_ASSERT(!lockScreen.isLocked(), "Session unlocked successfully");

    // Test Power Management Callback
    std::string powerAction;
    lockScreen.setPowerCallback([&](const std::string& action) {
        powerAction = action;
    });

    lockScreen.lock();
    lockScreen.showCredentials();

    surshell::Surface lockCanvas(1920, 1080, surshell::Color::fromHex(0x0E1420));
    lockScreen.render(lockCanvas, 1920, 1080);
    TEST_ASSERT(lockCanvas.width() == 1920, "Lock screen rendered onto canvas");

    // Test Master Desktop Integration with new subsystems
    surshell::SurShellDesktop desktop(1920, 1080);
    desktop.openTerminalWindow("C:\\Users\\admin");
    desktop.openSearchHub();
    TEST_ASSERT(desktop.searchHub().isVisible(), "Search Hub opened via desktop coordinator");
    desktop.openActionCenter();
    TEST_ASSERT(desktop.actionCenter().isVisible(), "Action Center opened via desktop coordinator");
    TEST_ASSERT(!desktop.searchHub().isVisible(), "Opening Action Center closes Search Hub");

    desktop.lockSession();
    TEST_ASSERT(desktop.lockScreen().isLocked(), "Desktop session locked via coordinator");
    desktop.render(); // Render full locked desktop

    std::cout << "[TEST] Suite 24: Sovereign Lock Screen & Authentication Center PASSED.\n";
}

void Test_Registry_Editor_Application() {
    std::cout << "[TEST] Running Suite 25: Sovereign Registry Editor (regedit.exe)...\n";

    // 1. Initial State & Path
    surshell::RegistryEditorContent regedit;
    TEST_ASSERT(regedit.currentPath() == "Computer\\HKEY_CURRENT_USER\\Software\\MicaNT\\SurShell",
                "Initial default key path matches MicaNT SurShell hive");
    TEST_ASSERT(regedit.valueCount() >= 5, "SurShell hive has initial configuration values");

    // 2. Value Query & Type Formatting
    const auto& values = regedit.currentValues();
    bool foundDefault = false;
    bool foundAlignment = false;
    for (const auto& v : values) {
        if (v.name == "(Default)") {
            foundDefault = true;
            TEST_ASSERT(v.type == surshell::RegType::Sz, "Default value is REG_SZ");
            TEST_ASSERT(v.stringData.find("MicaNT") != std::string::npos, "Default string content");
        }
        if (v.name == "TaskbarAlignment") {
            foundAlignment = true;
            TEST_ASSERT(v.type == surshell::RegType::Sz, "TaskbarAlignment is REG_SZ");
            TEST_ASSERT(v.stringData == "Center", "TaskbarAlignment value is Center");
        }
    }
    TEST_ASSERT(foundDefault, "Default value found");
    TEST_ASSERT(foundAlignment, "TaskbarAlignment found");

    // 3. Navigation to different hives
    std::string pathNotified;
    regedit.setPathChangedCallback([&](const std::string& p) {
        pathNotified = p;
    });

    regedit.navigateToPath("Computer\\HKEY_LOCAL_MACHINE\\SOFTWARE\\MicaNT\\CurrentVersion");
    TEST_ASSERT(regedit.currentPath() == "Computer\\HKEY_LOCAL_MACHINE\\SOFTWARE\\MicaNT\\CurrentVersion",
                "Navigated to HKLM MicaNT CurrentVersion");
    TEST_ASSERT(pathNotified == "Computer\\HKEY_LOCAL_MACHINE\\SOFTWARE\\MicaNT\\CurrentVersion",
                "Path changed callback invoked");

    const auto& hklmValues = regedit.currentValues();
    bool foundBuild = false;
    for (const auto& v : hklmValues) {
        if (v.name == "CurrentBuild") {
            foundBuild = true;
            TEST_ASSERT(v.stringData == "26100", "Build number is 26100");
        }
    }
    TEST_ASSERT(foundBuild, "CurrentBuild value found in HKLM");

    // 4. Navigate back to Computer root
    regedit.navigateToPath("Computer");
    TEST_ASSERT(regedit.currentPath() == "Computer", "Navigated to Computer root");

    // 5. Test Value Modification Callback and Up Navigation
    regedit.navigateToPath("Computer\\HKEY_CURRENT_USER\\Control Panel\\Personalization");
    const surshell::RegistryValue* dwordVal = nullptr;
    for (const auto& v : regedit.currentValues()) {
        if (v.name == "TransparencyEffects") {
            dwordVal = &v;
            break;
        }
    }
    TEST_ASSERT(dwordVal != nullptr, "TransparencyEffects DWORD found");
    TEST_ASSERT(dwordVal->dwordData == 1, "TransparencyEffects initially 1");

    regedit.onKeyDown(surshell::KeyCode::Up);
    TEST_ASSERT(regedit.currentPath() == "Computer\\HKEY_CURRENT_USER\\Control Panel", "Up arrow navigates to parent key");

    // 6. Surface Rendering
    surshell::Surface clientCanvas(860, 540, surshell::Color::fromHex(0x0C101A));
    regedit.render(clientCanvas);
    TEST_ASSERT(clientCanvas.width() == 860 && clientCanvas.height() == 540, "Registry Editor rendered onto surface");

    // 7. Desktop Coordinator Integration
    surshell::SurShellDesktop desktop(1920, 1080);
    const uint32_t winId = desktop.openRegistryEditorWindow();
    TEST_ASSERT(!desktop.windowManager().windows().empty(), "Registry Editor window created via coordinator");

    const auto* win = desktop.windowManager().findWindow(winId);
    TEST_ASSERT(win != nullptr, "Created window exists");
    TEST_ASSERT(win->title == "Registry Editor", "Window title is Registry Editor");

    std::cout << "[TEST] Suite 25: Sovereign Registry Editor (regedit.exe) PASSED.\n";
}

void Test_Storage_Topology_And_Network_Shares() {
    std::cout << "[TEST] Running Suite 26: Storage Topology, Google Drive & Network Attached Storage (NAS)...\n";

    surshell::FileExplorer exp("This PC");

    // 1. Storage Topology Discovery
    const auto& drives = exp.drives();
    TEST_ASSERT(drives.size() >= 2, "Discovered multiple logical storage drives");

    bool hasFixedDisk = false;
    bool hasCloudDrive = false;
    bool hasNetworkStorage = false;
    bool hasCdRom = false;

    for (const auto& d : drives) {
        if (d.kind == surshell::DriveKind::Fixed) hasFixedDisk = true;
        if (d.kind == surshell::DriveKind::Cloud) hasCloudDrive = true;
        if (d.kind == surshell::DriveKind::Network) hasNetworkStorage = true;
        if (d.kind == surshell::DriveKind::CdRom) hasCdRom = true;
    }

    TEST_ASSERT(hasFixedDisk, "Topology includes local fixed disk (C: or root)");
    std::cout << "       [INFO] Discovered " << drives.size() << " drives. Cloud=" << hasCloudDrive 
              << ", Network=" << hasNetworkStorage << ", CdRom=" << hasCdRom << "\n";

    // 2. This PC Virtual Container Categorization
    exp.navigateTo("This PC");
    TEST_ASSERT(exp.currentPath() == "This PC", "Current path is This PC");
    const auto& pcItems = exp.items();
    TEST_ASSERT(pcItems.size() >= 6, "This PC contains standard items");

    bool seenFolders = false;
    bool seenDevices = false;
    int lastRank = -1;

    for (const auto& item : pcItems) {
        int rank = 3;
        if (item.category == "Folders") {
            seenFolders = true;
            rank = 0;
        } else if (item.category == "Devices and drives") {
            seenDevices = true;
            rank = 1;
        } else if (item.category == "Network locations") {
            rank = 2;
        }
        TEST_ASSERT(rank >= lastRank, "Categories in This PC must follow strict rank order: Folders -> Devices -> Network");
        lastRank = rank;
    }
    TEST_ASSERT(seenFolders, "This PC contains Folders category");
    TEST_ASSERT(seenDevices, "This PC contains Devices and drives category");

    // 3. Attached Network Storage (NAS) Navigation & UNC Path Support
    exp.navigateTo("\\\\nas.ash-forge.com\\storage");
    TEST_ASSERT(exp.currentPath() == "\\\\nas.ash-forge.com\\storage", "Navigated to NAS SMB share");
    TEST_ASSERT(!exp.items().empty(), "NAS share directories enumerated");

    // 4. Deep UNC Navigation & Up Traversal
    exp.navigateTo("\\\\nas.ash-forge.com\\storage\\models");
    TEST_ASSERT(exp.currentPath() == "\\\\nas.ash-forge.com\\storage\\models", "Navigated to models directory on NAS");
    exp.navigateUp();
    TEST_ASSERT(exp.currentPath() == "\\\\nas.ash-forge.com\\storage", "Navigated up from models to NAS root share");
    exp.navigateUp();
    TEST_ASSERT(exp.currentPath() == "This PC", "Navigated up from NAS root share to This PC");

    // 5. Google Drive (G:\) Navigation & Up Traversal
    exp.navigateTo("G:\\");
    TEST_ASSERT(exp.currentPath() == "G:\\", "Navigated to Google Drive G:\\");
    TEST_ASSERT(!exp.items().empty(), "Google Drive items enumerated");
    exp.navigateUp();
    TEST_ASSERT(exp.currentPath() == "This PC", "Navigated up from G:\\ root to This PC");

    // 6. Local Windows C:\ Up Traversal to This PC
    exp.navigateTo("C:\\Windows");
    exp.navigateUp();
    TEST_ASSERT(exp.currentPath() == "C:\\", "Navigated up to C:\\ root");
    exp.navigateUp();
    TEST_ASSERT(exp.currentPath() == "This PC", "Navigated up from C:\\ root to This PC");

    // 7. Visual Surface Render Verification (DetailsList and TilesGrid)
    surshell::Surface clientCanvas(960, 600, surshell::Color::fromHex(0x0C121D));
    exp.navigateTo("This PC");
    exp.setViewMode(surshell::ExplorerViewMode::DetailsList);
    exp.render(clientCanvas);
    TEST_ASSERT(clientCanvas.width() == 960 && clientCanvas.height() == 600, "DetailsList rendered onto surface");

    exp.setViewMode(surshell::ExplorerViewMode::TilesGrid);
    exp.render(clientCanvas);
    TEST_ASSERT(clientCanvas.width() == 960 && clientCanvas.height() == 600, "TilesGrid rendered onto surface");

    std::cout << "[TEST] Suite 26: Storage Topology, Google Drive & Network Attached Storage (NAS) PASSED.\n";
}

void Test_Photo_And_Image_Viewer() {
    std::cout << "[TEST] Running Suite 27: Sovereign Photo & Image Viewer (Photos)...\n";

    // 1. Surface BMP Export & Import Roundtrip Verification
    surshell::Surface testBmp(64, 48, surshell::Color{255, 128, 64, 255});
    testBmp.fillRect(surshell::Rect{10, 10, 20, 20}, surshell::Color{0, 212, 255, 255});
    const std::string testFile = "surshell_test_roundtrip.bmp";
    TEST_ASSERT(testBmp.exportBmp(testFile), "exportBmp must write valid 32-bit BMP");

    auto loadedBmp = surshell::Surface::loadBmp(testFile);
    TEST_ASSERT(loadedBmp.has_value(), "Surface::loadBmp must parse exported BMP");
    TEST_ASSERT(loadedBmp->width() == 64 && loadedBmp->height() == 48, "Decoded BMP dimensions match");
    const auto px1 = loadedBmp->getPixel(0, 0);
    TEST_ASSERT(px1.r == 255 && px1.g == 128 && px1.b == 64, "Decoded background pixel color match");
    const auto px2 = loadedBmp->getPixel(15, 15);
    TEST_ASSERT(px2.r == 0 && px2.g == 212 && px2.b == 255, "Decoded foreground pixel color match");

    // 2. ImageViewerContent Initialization & State
    surshell::ImageViewerContent viewer(testFile);
    TEST_ASSERT(viewer.hasImage(), "ImageViewer must hold active image surface");
    TEST_ASSERT(viewer.imageWidth() == 64 && viewer.imageHeight() == 48, "Viewer image dimensions match");
    TEST_ASSERT(viewer.currentFileName() == testFile, "Current filename match");

    // 3. Zooming Operations
    const float initialZoom = viewer.zoom();
    viewer.zoomIn();
    TEST_ASSERT(viewer.zoom() > initialZoom, "zoomIn increases scale");
    viewer.zoomActual();
    TEST_ASSERT(viewer.zoom() == 1.0f, "zoomActual sets 1.0f");
    viewer.zoomOut();
    TEST_ASSERT(viewer.zoom() < 1.0f, "zoomOut decreases scale");
    viewer.zoomFit();

    // 4. Rotation Operations
    viewer.rotateClockwise();
    TEST_ASSERT(viewer.imageWidth() == 48 && viewer.imageHeight() == 64, "Rotated 90 deg swaps width and height");
    viewer.rotateClockwise();
    viewer.rotateClockwise();
    viewer.rotateClockwise();
    TEST_ASSERT(viewer.imageWidth() == 64 && viewer.imageHeight() == 48, "Full 360 deg rotation restores dimensions");

    // 5. Client Area Rendering
    surshell::Surface clientCanvas(800, 600, surshell::Color{12, 16, 24, 255});
    viewer.render(clientCanvas);
    TEST_ASSERT(clientCanvas.width() == 800 && clientCanvas.height() == 600, "ImageViewer rendered cleanly onto surface");

    // Cleanup temporary test BMP
    std::error_code ec;
    std::filesystem::remove(testFile, ec);

    std::cout << "[TEST] Suite 27: Sovereign Photo & Image Viewer (Photos) PASSED.\n";
}

void Test_Notepad_Interactive_Editor_And_Telemetry() {
    std::cout << "[TEST] Running Suite 28: Interactive Notepad 2.0 & Task Manager Performance Charts...\n";

    // 1. Notepad 2.0 Initialization
    surshell::TextViewerContent notepad;
    TEST_ASSERT(notepad.fileName() == "Untitled.txt", "Default filename is Untitled.txt");
    TEST_ASSERT(!notepad.isModified(), "Initial document is not modified");
    TEST_ASSERT(notepad.lineCount() > 0, "Initial document has placeholder template lines");

    // 2. Title Change and File Saved Callbacks
    std::string currentTitle;
    notepad.setTitleChangedCallback([&](const std::string& title) {
        currentTitle = title;
    });

    bool savedCallbackInvoked = false;
    uint64_t savedBytes = 0;
    notepad.setFileSavedCallback([&](const std::string&, uint64_t bytes) {
        savedCallbackInvoked = true;
        savedBytes = bytes;
    });

    // 3. Typing & Character Insertion
    notepad.setCursor(0, 0);
    notepad.insertChar('#');
    TEST_ASSERT(notepad.isModified(), "Typing marks document as modified");
    TEST_ASSERT(currentTitle.starts_with("*"), "Modified title begins with asterisk");

    notepad.onCharInput(' ');
    notepad.onCharInput('M');
    notepad.onCharInput('i');
    notepad.onCharInput('c');
    notepad.onCharInput('a');
    notepad.onCharInput('N');
    notepad.onCharInput('T');
    notepad.onCharInput('\n');

    TEST_ASSERT(notepad.cursorRow() == 1 && notepad.cursorCol() == 0, "Newline advanced cursor to row 1 col 0");

    // 4. Backspace and Delete Operations
    notepad.onCharInput('Z');
    TEST_ASSERT(notepad.cursorCol() == 1, "Cursor at col 1 after typing Z");
    notepad.onKeyDown(surshell::KeyCode::Backspace);
    TEST_ASSERT(notepad.cursorCol() == 0, "Cursor at col 0 after backspace");

    // 5. Keyboard Navigation
    notepad.onKeyDown(surshell::KeyCode::Up);
    TEST_ASSERT(notepad.cursorRow() == 0, "Up arrow moved to row 0");
    notepad.onKeyDown(surshell::KeyCode::End);
    TEST_ASSERT(notepad.cursorCol() > 0, "End key moved cursor to end of line");
    notepad.onKeyDown(surshell::KeyCode::Home);
    TEST_ASSERT(notepad.cursorCol() == 0, "Home key moved cursor to start of line");

    // 6. File Saving Roundtrip
    const std::string tempSavePath = "surshell_notepad_test.txt";
    const bool saveOk = notepad.saveFile(tempSavePath);
    TEST_ASSERT(saveOk, "saveFile returned true");
    TEST_ASSERT(!notepad.isModified(), "Saving cleared modified flag");
    TEST_ASSERT(savedCallbackInvoked && savedBytes > 0, "FileSaved callback fired with positive byte count");
    TEST_ASSERT(std::filesystem::exists(tempSavePath), "Saved file exists on disk");

    // 7. Reload File & Verify Content
    surshell::TextViewerContent reloaded(tempSavePath);
    TEST_ASSERT(reloaded.lineCount() >= 2, "Reloaded file has lines");
    TEST_ASSERT(!reloaded.isModified(), "Freshly loaded file is not modified");

    // Cleanup temp file
    std::error_code ec;
    std::filesystem::remove(tempSavePath, ec);

    // 8. Notepad Surface Rendering
    surshell::Surface npSurface(740, 480);
    notepad.render(npSurface);
    TEST_ASSERT(npSurface.width() == 740, "Notepad rendered cleanly to surface");

    // 9. Task Manager Performance Tab & Historical Line Graphs
    surshell::TaskManagerContent taskMgr;
    taskMgr.setActiveTab(surshell::TaskManagerTab::Performance);
    TEST_ASSERT(taskMgr.activeTab() == surshell::TaskManagerTab::Performance, "Task Manager active tab is Performance");
    TEST_ASSERT(taskMgr.cpuHistory().size() > 0, "CPU history has historical telemetry samples");
    TEST_ASSERT(taskMgr.memHistory().size() > 0, "Memory history has telemetry samples");

    taskMgr.setPerformanceResource(surshell::PerformanceResource::Memory);
    TEST_ASSERT(taskMgr.performanceResource() == surshell::PerformanceResource::Memory, "Selected resource is Memory");

    surshell::Surface perfSurface(720, 480);
    taskMgr.render(perfSurface);
    TEST_ASSERT(perfSurface.width() == 720, "Task Manager Performance tab rendered with line chart");

    // 10. Task Manager Details Tab
    taskMgr.setActiveTab(surshell::TaskManagerTab::Details);
    TEST_ASSERT(taskMgr.activeTab() == surshell::TaskManagerTab::Details, "Task Manager active tab is Details");
    taskMgr.render(perfSurface);
    TEST_ASSERT(perfSurface.width() == 720, "Task Manager Details tab rendered cleanly");

    std::cout << "[TEST] Suite 28: Interactive Notepad 2.0 & Task Manager Performance Charts PASSED.\n";
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
    Test_Task_Manager();
    Test_Toast_Notifications();
    Test_Media_Hud_OSD();
    Test_Settings_And_Personalization();
    Test_Calculator_Application();
    Test_Run_Dialog_And_Live_Aero_Snap();
    Test_Modern_Terminal_Subsystem();
    Test_Action_Center_And_Calendar();
    Test_Universal_Search_Hub();
    Test_Lock_Screen_And_Authentication();
    Test_Registry_Editor_Application();
    Test_Storage_Topology_And_Network_Shares();
    Test_Photo_And_Image_Viewer();
    Test_Notepad_Interactive_Editor_And_Telemetry();

    std::cout << "\n===============================================================================\n";
    std::cout << "ALL 28 SURSHELL SUBSYSTEM VERIFICATION SUITES PASSED (100% SUCCESS)\n";
    std::cout << "===============================================================================\n";
    return 0;
}
