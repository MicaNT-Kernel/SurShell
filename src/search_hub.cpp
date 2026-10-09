// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/search_hub.cpp)
// ============================================================================

#include "surshell/search_hub.hpp"
#include <algorithm>

namespace surshell {

SearchHub::SearchHub() {
    // Populate Search Catalog
    allCatalog_ = {
        // Apps
        {"app_calc",     "Calculator",        "Standard arithmetic and function modifiers", SearchCategoryType::Apps, IconId::Calculator,   "calc",     ""},
        {"app_cmd",      "Command Prompt",    "Sovereign Windows Terminal and command CLI", SearchCategoryType::Apps, IconId::Terminal,     "cmd",      ""},
        {"app_explorer", "File Explorer",     "Browse sovereign directories, libraries & drives", SearchCategoryType::Apps, IconId::FileExplorer, "explorer", "C:\\Users\\admin"},
        {"app_taskmgr",  "Task Manager",      "System vitals, memory graphs & process monitor", SearchCategoryType::Apps, IconId::TaskManager,  "taskmgr",  ""},
        {"app_settings", "Settings",          "System personalization, themes & hardware info", SearchCategoryType::Apps, IconId::Settings,     "settings", ""},
        {"app_run",      "Run...",            "Dispatch system executables and file paths", SearchCategoryType::Apps, IconId::RunDialog,    "run",      ""},
        {"app_editor",   "Sovereign Editor",  "Pure C++ code editor with syntax highlighting", SearchCategoryType::Apps, IconId::FileCode,     "editor",   ""},
        {"app_regedit",  "Registry Editor",   "MicaNT sovereign configuration tree and keys", SearchCategoryType::Apps, IconId::Registry,     "regedit",  ""},
        {"app_photos",   "Photos",            "Sovereign image viewer, zoom, rotate & BMP inspection", SearchCategoryType::Apps, IconId::ImageViewer, "photos", ""},
        {"app_paint",    "Paint",             "Sovereign vector canvas, brushes, shapes & BMP studio", SearchCategoryType::Apps, IconId::Paint,       "paint",  ""},
        {"app_sysinfo",  "System Information","Hardware topology, storage, CPU & diagnostics",        SearchCategoryType::Apps, IconId::SystemInfo,  "sysinfo", ""},

        // Settings
        {"set_theme",    "Personalization",   "Themes, accent color palette & wallpaper style", SearchCategoryType::Settings, IconId::Personalization, "settings", "personalize"},
        {"set_taskbar",  "Taskbar & Dock",    "Center dock alignment and floating island styles", SearchCategoryType::Settings, IconId::TaskView,        "settings", "taskbar"},
        {"set_network",  "Network Telemetry", "Gigabit Ethernet and zero-telemetry policy", SearchCategoryType::Settings, IconId::NetworkEthernet, "settings", "network"},
        {"set_display",  "Display & Graphics","1920x1080 120Hz VSync PrismX software compositor", SearchCategoryType::Settings, IconId::Display,         "settings", "display"},

        // Documents & System Files
        {"doc_kernel",   "ntoskrnl.exe",      "C:\\Windows\\System32\\ntoskrnl.exe", SearchCategoryType::Documents, IconId::FileExecutable, "explorer", "C:\\Windows\\System32"},
        {"doc_surwin",   "surwin.sys",        "C:\\Windows\\System32\\drivers\\surwin.sys", SearchCategoryType::Documents, IconId::FileLibrary, "explorer", "C:\\Windows\\System32"},
        {"doc_k32",      "kernel32.dll",      "C:\\Windows\\System32\\kernel32.dll", SearchCategoryType::Documents, IconId::FileLibrary, "explorer", "C:\\Windows\\System32"},
        {"doc_regedit",  "regedit.exe",       "C:\\Windows\\regedit.exe", SearchCategoryType::Documents, IconId::Registry, "regedit", ""}
    };

    updateFilter();
}

void SearchHub::populateHostApplications(const std::vector<ShellAppEntry>& apps) {
    for (const auto& a : apps) {
        if (a.id.rfind("host_", 0) == 0) {
            allCatalog_.push_back(SearchItem{
                .id = a.id,
                .title = a.title,
                .subtitle = a.subtitle,
                .category = SearchCategoryType::Apps,
                .icon = IconRenderer::iconForAppId(a.title),
                .targetApp = a.executablePath,
                .args = a.arguments,
                .rowBounds = Rect{}
            });
        }
    }
    updateFilter();
}

void SearchHub::setQuery(const std::string& q) {
    query_ = q;
    updateFilter();
}

void SearchHub::updateFilter() {
    filteredItems_.clear();
    std::string lowerQ = query_;
    std::transform(lowerQ.begin(), lowerQ.end(), lowerQ.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    for (const auto& item : allCatalog_) {
        // 1. Check category filter
        if (activeFilter_ != SearchCategoryType::All && item.category != activeFilter_) {
            continue;
        }

        // 2. Check query match
        if (lowerQ.empty()) {
            filteredItems_.push_back(item);
        } else {
            std::string lowerTitle = item.title;
            std::string lowerSub = item.subtitle;
            std::string lowerTarget = item.targetApp;
            std::string lowerId = item.id;
            std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            std::transform(lowerSub.begin(), lowerSub.end(), lowerSub.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            std::transform(lowerId.begin(), lowerId.end(), lowerId.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (lowerTitle.find(lowerQ) != std::string::npos ||
                lowerSub.find(lowerQ) != std::string::npos ||
                lowerTarget.find(lowerQ) != std::string::npos ||
                lowerId.find(lowerQ) != std::string::npos) {
                filteredItems_.push_back(item);
            }
        }
    }

    if (selectedIndex_ >= static_cast<int32_t>(filteredItems_.size())) {
        selectedIndex_ = std::max(0, static_cast<int32_t>(filteredItems_.size()) - 1);
    }
}

void SearchHub::executeSelected(bool asAdmin) {
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int32_t>(filteredItems_.size())) {
        const auto& item = filteredItems_[selectedIndex_];
        if (onExecute_) {
            onExecute_(item.targetApp, item.args, asAdmin);
        }
        hide();
    }
}

void SearchHub::render(Surface& s, int32_t screenW, int32_t screenH) {
    if (!visible_) return;

    const int32_t hubW = 700;
    const int32_t hubH = 500;
    const int32_t hubX = (screenW - hubW) / 2;
    const int32_t hubY = 110;
    bounds_ = Rect{hubX, hubY, hubW, hubH};

    const auto& palette = ThemeManager::instance().palette();

    // 1. Drop shadow
    s.drawRoundedRect(bounds_.inflate(8, 8), 16, Color::fromRgba(0, 0, 0, 95), true);

    // 2. Acrylic container backdrop
    s.drawRoundedRect(bounds_, 10, Color::fromRgba(14, 18, 28, 245), true);
    s.drawRoundedRect(bounds_, 10, Color::fromRgba(45, 60, 85, 200), false);

    // 3. Search Box Input Field
    searchBoxBounds_ = Rect{hubX + 20, hubY + 16, hubW - 40, 38};
    s.drawRoundedRect(searchBoxBounds_, 8, Color::fromHex(0x0C121C), true);
    s.drawRoundedRect(searchBoxBounds_, 8, palette.accentColor, false);

    IconRenderer::draw(s, IconId::Search, Point{searchBoxBounds_.x + 12, searchBoxBounds_.y + 11}, 16, palette.accentColor);

    if (query_.empty()) {
        s.drawString(searchBoxBounds_.x + 36, searchBoxBounds_.y + 11,
                     "Type here to search apps, settings, and files...", Color::fromHex(0x64748B), 1);
    } else {
        s.drawString(searchBoxBounds_.x + 36, searchBoxBounds_.y + 11, query_, Color::fromHex(0xFFFFFF), 1);

        // Blinking cursor
        const int32_t cursorX = searchBoxBounds_.x + 36 + static_cast<int32_t>(query_.size()) * 8;
        s.fillRect(Rect{cursorX, searchBoxBounds_.y + 10, 2, 16}, palette.accentColor);

        // Clear [x] button
        btnClearBounds_ = Rect{searchBoxBounds_.right() - 28, searchBoxBounds_.y + 10, 18, 18};
        s.drawString(btnClearBounds_.x + 5, btnClearBounds_.y + 1, "x", Color::fromHex(0x94A3B8), 1);
    }

    // 4. Horizontal Category Filter Tabs
    const int32_t tabY = hubY + 62;
    tabAll_ = Rect{hubX + 24, tabY, 50, 24};
    tabApps_ = Rect{hubX + 80, tabY, 55, 24};
    tabSettings_ = Rect{hubX + 141, tabY, 70, 24};
    tabDocs_ = Rect{hubX + 217, tabY, 80, 24};

    auto drawTab = [&](const Rect& rect, const std::string& label, SearchCategoryType cat) {
        const bool active = (activeFilter_ == cat);
        s.drawString(rect.x + 6, rect.y + 4, label,
                     active ? palette.accentColor : Color::fromHex(0x94A3B8), 1);
        if (active) {
            s.fillRect(Rect{rect.x + 6, rect.bottom() - 2, rect.width - 12, 2}, palette.accentColor);
        }
    };

    drawTab(tabAll_, "All", SearchCategoryType::All);
    drawTab(tabApps_, "Apps", SearchCategoryType::Apps);
    drawTab(tabSettings_, "Settings", SearchCategoryType::Settings);
    drawTab(tabDocs_, "Documents", SearchCategoryType::Documents);

    // Separator line
    s.fillRect(Rect{hubX + 20, tabY + 28, hubW - 40, 1}, Color::fromHex(0x223044));

    // 5. Left Results List
    const int32_t listX = hubX + 20;
    const int32_t listY = tabY + 36;
    const int32_t listW = 390;
    const int32_t itemH = 46;
    const int32_t maxItems = 7;

    for (size_t i = 0; i < filteredItems_.size() && i < static_cast<size_t>(maxItems); ++i) {
        auto& item = filteredItems_[i];
        item.rowBounds = Rect{listX, listY + static_cast<int32_t>(i) * (itemH + 4), listW, itemH};

        const bool isSelected = (selectedIndex_ == static_cast<int32_t>(i));
        const bool isHover = (hoveredIndex_ == static_cast<int32_t>(i));

        if (isSelected) {
            s.drawRoundedRect(item.rowBounds, 6, Color::fromRgba(0, 212, 255, 45), true);
            s.drawRoundedRect(item.rowBounds, 6, palette.accentColor, false);
            // Left accent bar
            s.fillRect(Rect{item.rowBounds.x + 2, item.rowBounds.y + 8, 3, item.rowBounds.height - 16}, palette.accentColor);
        } else if (isHover) {
            s.drawRoundedRect(item.rowBounds, 6, Color::fromHex(0x182232), true);
        }

        // Icon
        IconRenderer::draw(s, item.icon, Point{item.rowBounds.x + 12, item.rowBounds.y + 11}, 24,
                           isSelected ? std::make_optional(palette.accentColor) : std::nullopt);

        // Title and Subtitle
        s.drawString(item.rowBounds.x + 44, item.rowBounds.y + 8, item.title,
                     isSelected ? Color::fromHex(0xFFFFFF) : palette.textPrimary, 1);

        std::string dispSub = item.subtitle;
        if (dispSub.size() > 40) dispSub = dispSub.substr(0, 37) + "...";
        s.drawString(item.rowBounds.x + 44, item.rowBounds.y + 24, dispSub, palette.textSecondary, 1);
    }

    if (filteredItems_.empty()) {
        s.drawString(listX + 20, listY + 30, "No results found matching '" + query_ + "'", palette.textSecondary, 1);
        s.drawString(listX + 20, listY + 50, "Check spelling or search for another term.", Color::fromHex(0x64748B), 1);
    }

    // Vertical Divider
    s.fillRect(Rect{hubX + 420, listY, 1, hubH - (listY - hubY) - 20}, Color::fromHex(0x223044));

    // 6. Right Details Preview Pane
    const int32_t prevX = hubX + 436;
    const int32_t prevY = listY;
    const int32_t prevW = hubW - (prevX - hubX) - 20;

    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int32_t>(filteredItems_.size())) {
        const auto& sel = filteredItems_[selectedIndex_];

        // Large Preview Icon
        IconRenderer::draw(s, sel.icon, Point{prevX + prevW / 2 - 24, prevY + 12}, 48, palette.accentColor);

        // Title & Category Badge
        s.drawString(prevX + 10, prevY + 70, sel.title, palette.textPrimary, 1);

        std::string catBadge = (sel.category == SearchCategoryType::Apps) ? "Application" :
                               ((sel.category == SearchCategoryType::Settings) ? "System Setting" : "Document");
        s.drawString(prevX + 10, prevY + 88, catBadge, Color::fromHex(0x00FF9D), 1);

        // Description box (wrapped cleanly across up to 2 lines to fit within prevW)
        std::string line1 = sel.subtitle;
        std::string line2;
        if (line1.size() > 28) {
            size_t splitPos = line1.rfind(' ', 28);
            if (splitPos != std::string::npos && splitPos > 10) {
                line2 = line1.substr(splitPos + 1);
                line1 = line1.substr(0, splitPos);
            } else {
                line2 = line1.substr(28);
                line1 = line1.substr(0, 28);
            }
            if (line2.size() > 28) line2 = line2.substr(0, 25) + "...";
        }
        s.drawString(prevX + 10, prevY + 110, line1, palette.textSecondary, 1);
        if (!line2.empty()) {
            s.drawString(prevX + 10, prevY + 126, line2, palette.textSecondary, 1);
        }

        // Action Buttons
        const int32_t btnH = 30;
        int32_t curBtnY = prevY + 148;

        btnOpen_ = Rect{prevX + 10, curBtnY, prevW - 20, btnH};
        s.drawRoundedRect(btnOpen_, 6, hoverOpen_ ? Color::fromHex(0x1E2B3E) : Color::fromHex(0x141E2C), true);
        s.drawRoundedRect(btnOpen_, 6, hoverOpen_ ? palette.accentColor : Color::fromHex(0x354765), false);
        IconRenderer::draw(s, IconId::MediaPlay, Point{btnOpen_.x + 10, btnOpen_.y + 8}, 14, palette.accentColor);
        s.drawString(btnOpen_.x + 32, btnOpen_.y + 9, "Open", Color::fromHex(0xFFFFFF), 1);

        curBtnY += btnH + 10;
        btnAdmin_ = Rect{prevX + 10, curBtnY, prevW - 20, btnH};
        s.drawRoundedRect(btnAdmin_, 6, hoverAdmin_ ? Color::fromHex(0x1E2B3E) : Color::fromHex(0x141E2C), true);
        s.drawRoundedRect(btnAdmin_, 6, hoverAdmin_ ? Color::fromHex(0xFFB703) : Color::fromHex(0x354765), false);
        IconRenderer::draw(s, IconId::ShieldAdmin, Point{btnAdmin_.x + 10, btnAdmin_.y + 8}, 14, Color::fromHex(0xFFB703));
        s.drawString(btnAdmin_.x + 32, btnAdmin_.y + 9, "Run as Administrator", Color::fromHex(0xFFFFFF), 1);

        curBtnY += btnH + 10;
        btnLoc_ = Rect{prevX + 10, curBtnY, prevW - 20, btnH};
        s.drawRoundedRect(btnLoc_, 6, hoverLoc_ ? Color::fromHex(0x1E2B3E) : Color::fromHex(0x141E2C), true);
        s.drawRoundedRect(btnLoc_, 6, hoverLoc_ ? palette.accentColor : Color::fromHex(0x354765), false);
        IconRenderer::draw(s, IconId::FolderOpen, Point{btnLoc_.x + 10, btnLoc_.y + 8}, 14, Color::fromHex(0x00FF9D));
        s.drawString(btnLoc_.x + 32, btnLoc_.y + 9, "Open File Location", Color::fromHex(0xFFFFFF), 1);
    }
}

bool SearchHub::onMouseDown(Point pt, MouseButton button) {
    if (!visible_ || button != MouseButton::Left) return false;

    if (!bounds_.contains(pt)) {
        hide();
        return true;
    }

    // Check Clear Button
    if (!query_.empty() && btnClearBounds_.contains(pt)) {
        setQuery("");
        return true;
    }

    // Check Filter Tabs
    if (tabAll_.contains(pt)) { activeFilter_ = SearchCategoryType::All; updateFilter(); return true; }
    if (tabApps_.contains(pt)) { activeFilter_ = SearchCategoryType::Apps; updateFilter(); return true; }
    if (tabSettings_.contains(pt)) { activeFilter_ = SearchCategoryType::Settings; updateFilter(); return true; }
    if (tabDocs_.contains(pt)) { activeFilter_ = SearchCategoryType::Documents; updateFilter(); return true; }

    // Check Result Items
    for (size_t i = 0; i < filteredItems_.size(); ++i) {
        if (filteredItems_[i].rowBounds.contains(pt)) {
            selectedIndex_ = static_cast<int32_t>(i);
            return true;
        }
    }

    // Check Action Buttons
    if (btnOpen_.contains(pt)) {
        executeSelected(false);
        return true;
    }
    if (btnAdmin_.contains(pt)) {
        executeSelected(true);
        return true;
    }
    if (btnLoc_.contains(pt)) {
        executeSelected(false);
        return true;
    }

    return true; // Click inside modal consumed
}

bool SearchHub::onMouseMove(Point pt) {
    if (!visible_) return false;

    hoverOpen_ = btnOpen_.contains(pt);
    hoverAdmin_ = btnAdmin_.contains(pt);
    hoverLoc_ = btnLoc_.contains(pt);

    hoveredIndex_ = -1;
    for (size_t i = 0; i < filteredItems_.size(); ++i) {
        if (filteredItems_[i].rowBounds.contains(pt)) {
            hoveredIndex_ = static_cast<int32_t>(i);
            break;
        }
    }

    return bounds_.contains(pt);
}

bool SearchHub::onKeyDown(KeyCode key) {
    if (!visible_) return false;

    if (key == KeyCode::Escape) {
        hide();
        return true;
    } else if (key == KeyCode::Enter) {
        executeSelected(false);
        return true;
    } else if (key == KeyCode::Up) {
        if (selectedIndex_ > 0) {
            selectedIndex_--;
            return true;
        }
    } else if (key == KeyCode::Down) {
        if (selectedIndex_ < static_cast<int32_t>(filteredItems_.size()) - 1) {
            selectedIndex_++;
            return true;
        }
    } else if (key == KeyCode::Backspace) {
        if (!query_.empty()) {
            query_.pop_back();
            updateFilter();
            return true;
        }
    }
    return false;
}

bool SearchHub::onCharInput(char c) {
    if (!visible_) return false;
    if (c >= 32 && c <= 126) {
        query_.push_back(c);
        updateFilter();
        return true;
    }
    return false;
}

} // namespace surshell
