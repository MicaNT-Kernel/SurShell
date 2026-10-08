// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/start_menu.cpp)
// ============================================================================

#include "surshell/start_menu.hpp"
#include "surshell/theme.hpp"

namespace surshell {

StartMenu::StartMenu() {
    // Populate default sovereign applications for MicaNT
    registerApp(ShellAppEntry{
        .id = "cmd",
        .title = "Command Prompt",
        .executablePath = "C:\\Windows\\System32\\cmd.exe",
        .arguments = "",
        .iconGlyph = ">_",
        .category = AppCategory::SystemTools,
        .pinnedToTaskbar = true,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "explorer",
        .title = "File Explorer",
        .executablePath = "C:\\Windows\\explorer.exe",
        .arguments = "",
        .iconGlyph = "[E]",
        .category = AppCategory::SystemTools,
        .pinnedToTaskbar = true,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "taskmgr",
        .title = "Task Manager",
        .executablePath = "C:\\Windows\\System32\\taskmgr.exe",
        .arguments = "",
        .iconGlyph = "[T]",
        .category = AppCategory::SystemTools,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "sentinel",
        .title = "Sentinel Security",
        .executablePath = "C:\\Program Files\\Sentinel\\sentinel.exe",
        .arguments = "",
        .iconGlyph = "[S]",
        .category = AppCategory::Utilities,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "prismx",
        .title = "PrismX 3D Visualizer",
        .executablePath = "C:\\Windows\\System32\\prismx_demo.exe",
        .arguments = "",
        .iconGlyph = "[P]",
        .category = AppCategory::Multimedia,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "winget",
        .title = "Windows Package Manager",
        .executablePath = "C:\\Windows\\System32\\winget.exe",
        .arguments = "",
        .iconGlyph = "[W]",
        .category = AppCategory::Development,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "settings",
        .title = "System Settings",
        .executablePath = "C:\\Windows\\System32\\control.exe",
        .arguments = "",
        .iconGlyph = "[*]",
        .category = AppCategory::Settings,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    refreshFilter();
}

void StartMenu::registerApp(ShellAppEntry app) {
    allApps_.push_back(std::move(app));
    refreshFilter();
}

void StartMenu::unregisterApp(std::string_view appId) {
    std::erase_if(allApps_, [&](const ShellAppEntry& a) { return a.id == appId; });
    refreshFilter();
}

void StartMenu::open() noexcept {
    isOpen_ = true;
    searchQuery_.clear();
    refreshFilter();
}

void StartMenu::close() noexcept {
    isOpen_ = false;
    searchQuery_.clear();
}

void StartMenu::toggle() noexcept {
    if (isOpen_) close();
    else open();
}

void StartMenu::setSearchQuery(std::string query) {
    searchQuery_ = std::move(query);
    refreshFilter();
}

void StartMenu::handleCharInput(char c) {
    if (c >= 32 && c <= 126) {
        searchQuery_.push_back(c);
        refreshFilter();
    }
}

void StartMenu::handleBackspace() {
    if (!searchQuery_.empty()) {
        searchQuery_.pop_back();
        refreshFilter();
    }
}

void StartMenu::refreshFilter() {
    filteredApps_.clear();
    if (searchQuery_.empty()) {
        filteredApps_ = allApps_;
        return;
    }

    std::string lowerQuery = searchQuery_;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    for (const auto& app : allApps_) {
        std::string lowerTitle = app.title;
        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        std::string lowerId = app.id;
        std::transform(lowerId.begin(), lowerId.end(), lowerId.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        std::string lowerExe = app.executablePath;
        std::transform(lowerExe.begin(), lowerExe.end(), lowerExe.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        if (lowerTitle.find(lowerQuery) != std::string::npos ||
            lowerId.find(lowerQuery) != std::string::npos ||
            lowerExe.find(lowerQuery) != std::string::npos) {
            filteredApps_.push_back(app);
        }
    }
}

Rect StartMenu::calculateBounds(uint32_t screenWidth, uint32_t screenHeight, int32_t taskbarHeight) const noexcept {
    (void)screenWidth;
    const auto& metrics = ThemeManager::instance().metrics();
    const int32_t startX = 12;
    const int32_t startY = static_cast<int32_t>(screenHeight) - taskbarHeight - metrics.startMenuHeight - 8;
    return Rect{startX, startY, metrics.startMenuWidth, metrics.startMenuHeight};
}

void StartMenu::onMouseMove(Point pt, Rect menuBounds) {
    if (!isOpen_ || !menuBounds.contains(pt)) {
        hoveredAppIndex_ = -1;
        hoveredPowerIndex_ = -1;
        return;
    }

    // Check apps list hit test
    const int32_t listStartY = menuBounds.y + 70;
    const int32_t itemHeight = 36;
    hoveredAppIndex_ = -1;

    for (size_t i = 0; i < filteredApps_.size(); ++i) {
        Rect itemRect{menuBounds.x + 16, listStartY + static_cast<int32_t>(i * itemHeight), menuBounds.width - 32, itemHeight - 4};
        if (itemRect.contains(pt)) {
            hoveredAppIndex_ = static_cast<int32_t>(i);
            break;
        }
    }

    // Check power buttons bar hit test
    const int32_t powerY = menuBounds.bottom() - 44;
    hoveredPowerIndex_ = -1;
    for (int32_t p = 0; p < 4; ++p) {
        Rect pRect{menuBounds.right() - 170 + p * 38, powerY, 32, 32};
        if (pRect.contains(pt)) {
            hoveredPowerIndex_ = p;
            break;
        }
    }
}

void StartMenu::onMouseDown(Point pt, MouseButton button, Rect menuBounds) {
    if (!isOpen_ || button != MouseButton::Left) return;

    if (!menuBounds.contains(pt)) {
        close();
        return;
    }

    // App item click
    if (hoveredAppIndex_ >= 0 && hoveredAppIndex_ < static_cast<int32_t>(filteredApps_.size())) {
        if (launchCallback_) {
            launchCallback_(filteredApps_[static_cast<size_t>(hoveredAppIndex_)]);
        }
        close();
        return;
    }

    // Power item click
    if (hoveredPowerIndex_ >= 0 && hoveredPowerIndex_ < 4) {
        if (powerCallback_) {
            powerCallback_(static_cast<PowerAction>(hoveredPowerIndex_));
        }
        close();
    }
}

void StartMenu::render(Surface& surface, Rect menuBounds) {
    if (!isOpen_) return;

    const auto& palette = ThemeManager::instance().palette();
    const auto& metrics = ThemeManager::instance().metrics();

    // 1. Drop shadow behind Start Menu
    surface.drawDropShadow(menuBounds, metrics.shadowRadius, metrics.shadowOpacity);

    // 2. Translucent Mica Acrylic Container
    surface.drawRoundedRect(menuBounds, metrics.windowCornerRadius, palette.startMenuBg, true);
    surface.drawRoundedRect(menuBounds, metrics.windowCornerRadius, palette.startMenuBorder, false);

    // 3. Search Bar
    Rect searchBox{menuBounds.x + 16, menuBounds.y + 16, menuBounds.width - 32, 36};
    surface.drawRoundedRect(searchBox, 6, palette.startMenuSearchBg, true);
    surface.drawRoundedRect(searchBox, 6, palette.startMenuSearchBorder, false);

    if (searchQuery_.empty()) {
        surface.drawString(searchBox.x + 12, searchBox.y + 14, "Type here to search...", palette.textDisabled, 1);
    } else {
        std::string displayQuery = searchQuery_ + "|";
        surface.drawString(searchBox.x + 12, searchBox.y + 14, displayQuery, palette.textPrimary, 1);
    }

    // 4. Section Label
    surface.drawString(menuBounds.x + 18, menuBounds.y + 60, "PINNED APPLICATIONS", palette.accentColor, 1);

    // 5. App List
    const int32_t listStartY = menuBounds.y + 80;
    const int32_t itemHeight = 36;
    const size_t maxDisplay = std::min(filteredApps_.size(), static_cast<size_t>(9));

    for (size_t i = 0; i < maxDisplay; ++i) {
        const auto& app = filteredApps_[i];
        Rect itemRect{menuBounds.x + 16, listStartY + static_cast<int32_t>(i * itemHeight), menuBounds.width - 32, itemHeight - 4};

        if (static_cast<int32_t>(i) == hoveredAppIndex_) {
            surface.drawRoundedRect(itemRect, 6, Color::fromRgba(255, 255, 255, 20), true);
            surface.drawRoundedRect(itemRect, 6, Color::fromRgba(0, 212, 255, 120), false);
        }

        // Icon glyph badge
        Rect glyphBox{itemRect.x + 6, itemRect.y + 3, 24, 24};
        surface.drawRoundedRect(glyphBox, 4, Color::fromRgba(30, 42, 65, 220), true);
        surface.drawString(glyphBox.x + 4, glyphBox.y + 8, app.iconGlyph, palette.accentColor, 1);

        // App Title
        surface.drawString(itemRect.x + 38, itemRect.y + 11, app.title, palette.textPrimary, 1);
    }

    // 6. Bottom User & Power Footer
    const int32_t footerY = menuBounds.bottom() - 52;
    surface.fillRect(Rect{menuBounds.x, footerY, menuBounds.width, 1}, palette.startMenuBorder);

    // User Avatar & Name
    Rect avatarBox{menuBounds.x + 16, footerY + 12, 28, 28};
    surface.drawRoundedRect(avatarBox, 14, palette.accentColor, true);
    surface.drawString(avatarBox.x + 10, avatarBox.y + 10, "A", Color::fromHex(0x000000), 1);
    surface.drawString(avatarBox.right() + 10, avatarBox.y + 10, "Administrator", palette.textPrimary, 1);

    // Power Buttons: Lock, Sleep, Restart, Shutdown
    const char* powerLabels[] = {"[L]", "[Z]", "[R]", "[X]"};
    const int32_t pStartX = menuBounds.right() - 160;

    for (int32_t p = 0; p < 4; ++p) {
        Rect pRect{pStartX + p * 36, footerY + 10, 30, 30};
        Color pBg = (hoveredPowerIndex_ == p) ? Color::fromRgba(255, 255, 255, 40) : Color::fromRgba(25, 35, 55, 180);
        surface.drawRoundedRect(pRect, 4, pBg, true);
        surface.drawString(pRect.x + 4, pRect.y + 11, powerLabels[p], palette.textSecondary, 1);
    }
}

} // namespace surshell
