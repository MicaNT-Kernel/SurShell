// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/explorer.cpp)
// ============================================================================

#include "surshell/explorer.hpp"
#include "surshell/theme.hpp"

namespace surshell {

FileExplorer::FileExplorer(std::string initialPath)
    : currentPath_(std::move(initialPath)) {
    refreshDirectory();
}

void FileExplorer::refreshDirectory() {
    items_.clear();

    // Sovereign virtual directory items matching clean-room MicaNT filesystem
    if (currentPath_ == "C:\\" || currentPath_ == "C:") {
        items_.push_back(FileItem{.name = "Windows", .fullPath = "C:\\Windows", .isDirectory = true, .sizeBytes = 0, .iconGlyph = "[D]"});
        items_.push_back(FileItem{.name = "Users", .fullPath = "C:\\Users", .isDirectory = true, .sizeBytes = 0, .iconGlyph = "[D]"});
        items_.push_back(FileItem{.name = "Program Files", .fullPath = "C:\\Program Files", .isDirectory = true, .sizeBytes = 0, .iconGlyph = "[D]"});
        items_.push_back(FileItem{.name = "boot.ini", .fullPath = "C:\\boot.ini", .isDirectory = false, .sizeBytes = 512, .iconGlyph = "[F]"});
        items_.push_back(FileItem{.name = "pagefile.sys", .fullPath = "C:\\pagefile.sys", .isDirectory = false, .sizeBytes = 2147483648, .iconGlyph = "[S]"});
    } else if (currentPath_ == "C:\\Windows") {
        items_.push_back(FileItem{.name = "System32", .fullPath = "C:\\Windows\\System32", .isDirectory = true, .sizeBytes = 0, .iconGlyph = "[D]"});
        items_.push_back(FileItem{.name = "SysWOW64", .fullPath = "C:\\Windows\\SysWOW64", .isDirectory = true, .sizeBytes = 0, .iconGlyph = "[D]"});
        items_.push_back(FileItem{.name = "explorer.exe", .fullPath = "C:\\Windows\\explorer.exe", .isDirectory = false, .sizeBytes = 384000, .iconGlyph = "[X]"});
        items_.push_back(FileItem{.name = "notepad.exe", .fullPath = "C:\\Windows\\notepad.exe", .isDirectory = false, .sizeBytes = 192000, .iconGlyph = "[X]"});
        items_.push_back(FileItem{.name = "win.ini", .fullPath = "C:\\Windows\\win.ini", .isDirectory = false, .sizeBytes = 1024, .iconGlyph = "[F]"});
    } else if (currentPath_ == "C:\\Windows\\System32") {
        items_.push_back(FileItem{.name = "kernel32.dll", .fullPath = "C:\\Windows\\System32\\kernel32.dll", .isDirectory = false, .sizeBytes = 840000, .iconGlyph = "[L]"});
        items_.push_back(FileItem{.name = "user32.dll", .fullPath = "C:\\Windows\\System32\\user32.dll", .isDirectory = false, .sizeBytes = 920000, .iconGlyph = "[L]"});
        items_.push_back(FileItem{.name = "csrss.exe", .fullPath = "C:\\Windows\\System32\\csrss.exe", .isDirectory = false, .sizeBytes = 145000, .iconGlyph = "[X]"});
        items_.push_back(FileItem{.name = "conhost.exe", .fullPath = "C:\\Windows\\System32\\conhost.exe", .isDirectory = false, .sizeBytes = 320000, .iconGlyph = "[X]"});
        items_.push_back(FileItem{.name = "cmd.exe", .fullPath = "C:\\Windows\\System32\\cmd.exe", .isDirectory = false, .sizeBytes = 280000, .iconGlyph = "[X]"});
        items_.push_back(FileItem{.name = "prismx.dll", .fullPath = "C:\\Windows\\System32\\prismx.dll", .isDirectory = false, .sizeBytes = 160000, .iconGlyph = "[L]"});
        items_.push_back(FileItem{.name = "sentinel.dll", .fullPath = "C:\\Windows\\System32\\sentinel.dll", .isDirectory = false, .sizeBytes = 210000, .iconGlyph = "[L]"});
    } else {
        items_.push_back(FileItem{.name = "Desktop", .fullPath = currentPath_ + "\\Desktop", .isDirectory = true, .sizeBytes = 0, .iconGlyph = "[D]"});
        items_.push_back(FileItem{.name = "Documents", .fullPath = currentPath_ + "\\Documents", .isDirectory = true, .sizeBytes = 0, .iconGlyph = "[D]"});
        items_.push_back(FileItem{.name = "Downloads", .fullPath = currentPath_ + "\\Downloads", .isDirectory = true, .sizeBytes = 0, .iconGlyph = "[D]"});
        items_.push_back(FileItem{.name = "source", .fullPath = currentPath_ + "\\source", .isDirectory = true, .sizeBytes = 0, .iconGlyph = "[D]"});
    }
}

void FileExplorer::navigateTo(std::string path) {
    if (path == currentPath_) return;
    backHistory_.push_back(currentPath_);
    forwardHistory_.clear();
    currentPath_ = std::move(path);
    selectedIndex_ = -1;
    refreshDirectory();
}

void FileExplorer::navigateUp() {
    auto lastSlash = currentPath_.find_last_of("\\/");
    if (lastSlash != std::string::npos && lastSlash > 2) {
        navigateTo(currentPath_.substr(0, lastSlash));
    } else if (lastSlash != std::string::npos && lastSlash == 2) {
        navigateTo("C:\\");
    }
}

void FileExplorer::navigateBack() {
    if (backHistory_.empty()) return;
    forwardHistory_.push_back(currentPath_);
    currentPath_ = backHistory_.back();
    backHistory_.pop_back();
    selectedIndex_ = -1;
    refreshDirectory();
}

void FileExplorer::navigateForward() {
    if (forwardHistory_.empty()) return;
    backHistory_.push_back(currentPath_);
    currentPath_ = forwardHistory_.back();
    forwardHistory_.pop_back();
    selectedIndex_ = -1;
    refreshDirectory();
}

void FileExplorer::onMouseDown(Point localPt, MouseButton button, Rect clientBounds) {
    (void)clientBounds;
    if (button != MouseButton::Left) return;

    // Check nav buttons
    if (Rect{8, 6, 26, 24}.contains(localPt)) {
        navigateBack();
        return;
    }
    if (Rect{38, 6, 26, 24}.contains(localPt)) {
        navigateForward();
        return;
    }
    if (Rect{68, 6, 26, 24}.contains(localPt)) {
        navigateUp();
        return;
    }

    // Check items hit test
    selectedIndex_ = -1;
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].bounds.contains(localPt)) {
            selectedIndex_ = static_cast<int32_t>(i);
            break;
        }
    }
}

void FileExplorer::onDoubleClick(Point localPt, Rect clientBounds) {
    (void)clientBounds;
    for (const auto& item : items_) {
        if (item.bounds.contains(localPt)) {
            if (item.isDirectory) {
                navigateTo(item.fullPath);
            } else if (executeCallback_) {
                executeCallback_(item.fullPath);
            }
            break;
        }
    }
}

void FileExplorer::render(Surface& clientSurface) {
    const auto& palette = ThemeManager::instance().palette();
    const int32_t width = static_cast<int32_t>(clientSurface.width());
    const int32_t height = static_cast<int32_t>(clientSurface.height());

    // 1. Top Navigation & Address Bar Strip
    clientSurface.fillRect(Rect{0, 0, width, 36}, Color::fromRgba(18, 26, 42, 240));
    clientSurface.fillRect(Rect{0, 35, width, 1}, palette.startMenuBorder);

    // Nav Buttons: Back [<], Forward [>], Up [^]
    clientSurface.drawRoundedRect(Rect{8, 6, 26, 24}, 4, Color::fromRgba(30, 42, 65, 200), true);
    clientSurface.drawString(16, 12, "<", palette.textPrimary, 1);

    clientSurface.drawRoundedRect(Rect{38, 6, 26, 24}, 4, Color::fromRgba(30, 42, 65, 200), true);
    clientSurface.drawString(46, 12, ">", palette.textPrimary, 1);

    clientSurface.drawRoundedRect(Rect{68, 6, 26, 24}, 4, Color::fromRgba(30, 42, 65, 200), true);
    clientSurface.drawString(76, 12, "^", palette.textPrimary, 1);

    // Breadcrumb Address Bar
    Rect addressBar{104, 6, width - 116, 24};
    clientSurface.drawRoundedRect(addressBar, 4, Color::fromRgba(25, 36, 56, 240), true);
    clientSurface.drawRoundedRect(addressBar, 4, Color::fromRgba(60, 85, 125, 140), false);
    clientSurface.drawString(addressBar.x + 8, addressBar.y + 7, currentPath_, palette.textPrimary, 1);

    // 2. Left Quick Access Sidebar
    const int32_t sidebarW = 140;
    clientSurface.fillRect(Rect{0, 36, sidebarW, height - 60}, Color::fromRgba(14, 20, 32, 230));
    clientSurface.fillRect(Rect{sidebarW - 1, 36, 1, height - 60}, palette.startMenuBorder);

    clientSurface.drawString(12, 48, "QUICK ACCESS", palette.accentColor, 1);
    const char* quickItems[] = {"[P] This PC", "[D] C: (MicaNT)", "[D] D: (Data)", "[*] Desktop", "[F] Documents"};
    for (int32_t q = 0; q < 5; ++q) {
        clientSurface.drawString(12, 70 + q * 24, quickItems[q], palette.textSecondary, 1);
    }

    // 3. Right File & Directory Item Grid
    const int32_t contentX = sidebarW + 12;
    const int32_t contentY = 46;
    const int32_t itemRowH = 28;

    for (size_t i = 0; i < items_.size(); ++i) {
        auto& item = items_[i];
        item.bounds = Rect{contentX, contentY + static_cast<int32_t>(i * itemRowH), width - contentX - 16, itemRowH - 4};

        if (static_cast<int32_t>(i) == selectedIndex_) {
            clientSurface.drawRoundedRect(item.bounds, 4, Color::fromRgba(0, 212, 255, 45), true);
            clientSurface.drawRoundedRect(item.bounds, 4, Color::fromRgba(0, 212, 255, 160), false);
        }

        // Icon Glyph
        Color glyphColor = item.isDirectory ? Color::fromHex(0xFFD700) : palette.accentColor;
        clientSurface.drawString(item.bounds.x + 8, item.bounds.y + 7, item.iconGlyph, glyphColor, 1);

        // Filename
        clientSurface.drawString(item.bounds.x + 36, item.bounds.y + 7, item.name, palette.textPrimary, 1);

        // Size or type
        std::string sizeStr = item.isDirectory ? "<DIR>" : (std::to_string(item.sizeBytes / 1024) + " KB");
        clientSurface.drawString(item.bounds.right() - 90, item.bounds.y + 7, sizeStr, palette.textSecondary, 1);
    }

    // 4. Bottom Status Bar
    Rect statusBar{0, height - 24, width, 24};
    clientSurface.fillRect(statusBar, Color::fromRgba(16, 22, 34, 255));
    clientSurface.fillRect(Rect{0, statusBar.y, width, 1}, palette.startMenuBorder);

    std::string statusText = std::to_string(items_.size()) + " items  |  MicaNT Sovereign File System (EmeraldFS)";
    clientSurface.drawString(12, statusBar.y + 7, statusText, palette.textSecondary, 1);
}

} // namespace surshell
