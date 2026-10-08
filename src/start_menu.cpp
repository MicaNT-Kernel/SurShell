// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/start_menu.cpp)
// ============================================================================

#include "surshell/start_menu.hpp"
#include "surshell/theme.hpp"

namespace surshell {

StartMenu::StartMenu() {
    // Populate default sovereign applications for MicaNT with rich modern metadata
    registerApp(ShellAppEntry{
        .id = "cmd",
        .title = "Command Prompt",
        .subtitle = "Sovereign NT C++23 CLI",
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
        .subtitle = "Cabinet File Manager",
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
        .subtitle = "Vitals & Process Sandbox",
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
        .subtitle = "Zero-Telemetry Guard",
        .executablePath = "C:\\Program Files\\Sentinel\\sentinel.exe",
        .arguments = "",
        .iconGlyph = "[S]",
        .category = AppCategory::Utilities,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "netbird",
        .title = "NetBird Mesh",
        .subtitle = "P2P Sovereign Network",
        .executablePath = "C:\\Program Files\\NetBird\\netbird-ui.exe",
        .arguments = "",
        .iconGlyph = "[N]",
        .category = AppCategory::Utilities,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "prismx",
        .title = "PrismX 3D",
        .subtitle = "Mica Composition Engine",
        .executablePath = "C:\\Windows\\System32\\prismx_demo.exe",
        .arguments = "",
        .iconGlyph = "[P]",
        .category = AppCategory::Multimedia,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "settings",
        .title = "System Settings",
        .subtitle = "MicaNT Configuration",
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
    const auto& metrics = ThemeManager::instance().metrics();
    const int32_t w = metrics.startMenuWidth;
    const int32_t h = metrics.startMenuHeight;
    const int32_t margin = metrics.startMenuFloatingMargin;

    int32_t startX = 14;
    if (metrics.taskbarAlignment == TaskbarAlignment::Center) {
        startX = (static_cast<int32_t>(screenWidth) - w) / 2;
    }
    const int32_t startY = static_cast<int32_t>(screenHeight) - taskbarHeight - h - margin;
    return Rect{startX, startY, w, h};
}

void StartMenu::onMouseMove(Point pt, Rect menuBounds) {
    if (!isOpen_ || !menuBounds.contains(pt)) {
        hoveredAppIndex_ = -1;
        hoveredPowerIndex_ = -1;
        return;
    }

    // Check tactile 2-column app card grid
    const int32_t gridStartX = menuBounds.x + 20;
    const int32_t gridStartY = menuBounds.y + 110;
    const int32_t cardW = (menuBounds.width - 50) / 2;
    const int32_t cardH = 52;
    const int32_t cardGap = 10;
    hoveredAppIndex_ = -1;

    for (size_t i = 0; i < filteredApps_.size(); ++i) {
        const int32_t col = static_cast<int32_t>(i % 2);
        const int32_t row = static_cast<int32_t>(i / 2);
        Rect cardRect{gridStartX + col * (cardW + cardGap), gridStartY + row * (cardH + cardGap), cardW, cardH};
        if (cardRect.contains(pt)) {
            hoveredAppIndex_ = static_cast<int32_t>(i);
            break;
        }
    }

    // Check power buttons bar hit test
    const int32_t footerY = menuBounds.bottom() - 56;
    hoveredPowerIndex_ = -1;
    for (int32_t p = 0; p < 4; ++p) {
        Rect pRect{menuBounds.right() - 170 + p * 38, footerY + 12, 32, 32};
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

    // App card click
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

    // 1. Soft deep drop shadow behind detached Start card
    surface.drawDropShadow(menuBounds, 18, 0.55f);

    // 2. Modern 14px rounded container in Mica slate
    surface.drawRoundedRect(menuBounds, 14, palette.startMenuBg, true);
    surface.drawRoundedRect(menuBounds, 14, palette.startMenuBorder, false);

    // 3. Modern Search Pill at Top
    Rect searchBox{menuBounds.x + 20, menuBounds.y + 18, menuBounds.width - 40, 40};
    surface.drawRoundedRect(searchBox, 10, palette.startMenuSearchBg, true);
    surface.drawRoundedRect(searchBox, 10, palette.startMenuSearchBorder, false);

    // Search Icon glyph
    surface.drawString(searchBox.x + 14, searchBox.y + 16, "?", palette.accentColor, 1);

    if (searchQuery_.empty()) {
        surface.drawString(searchBox.x + 32, searchBox.y + 16, "Search MicaNT apps, settings, commands...", palette.textDisabled, 1);
    } else {
        std::string displayQuery = searchQuery_ + "|";
        surface.drawString(searchBox.x + 32, searchBox.y + 16, displayQuery, palette.textPrimary, 1);
    }

    // 4. Section Label with subtle accent divider
    surface.drawString(menuBounds.x + 24, menuBounds.y + 80, "PINNED SOVEREIGN APPLICATIONS", palette.accentColor, 1);
    surface.fillRect(Rect{menuBounds.x + 24, menuBounds.y + 96, menuBounds.width - 48, 1}, palette.startCardBorder);

    // 5. Tactile 2-Column Application Card Grid
    const int32_t gridStartX = menuBounds.x + 20;
    const int32_t gridStartY = menuBounds.y + 110;
    const int32_t cardW = (menuBounds.width - 50) / 2;
    const int32_t cardH = 54;
    const int32_t cardGap = 10;
    const size_t maxDisplay = std::min(filteredApps_.size(), static_cast<size_t>(6));

    for (size_t i = 0; i < maxDisplay; ++i) {
        const auto& app = filteredApps_[i];
        const int32_t col = static_cast<int32_t>(i % 2);
        const int32_t row = static_cast<int32_t>(i / 2);
        Rect cardRect{gridStartX + col * (cardW + cardGap), gridStartY + row * (cardH + cardGap), cardW, cardH};

        const bool isHovered = (static_cast<int32_t>(i) == hoveredAppIndex_);
        Color cardBg = isHovered ? palette.startCardHover : palette.startCardBg;
        Color borderCol = isHovered ? palette.accentColor : palette.startCardBorder;

        surface.drawRoundedRect(cardRect, 8, cardBg, true);
        surface.drawRoundedRect(cardRect, 8, borderCol, false);

        // Procedural Vector Icon Tile (34x34)
        Rect iconTile{cardRect.x + 8, cardRect.y + 10, 34, 34};
        if (app.id == "explorer") {
            surface.drawVectorFolder(iconTile, Color::fromHex(0x1E88E5), Color::fromHex(0x64B5F6));
        } else if (app.id == "cmd") {
            surface.drawVectorTerminal(iconTile, Color::fromHex(0x10141E), palette.accentColor);
        } else if (app.id == "taskmgr") {
            surface.drawVectorTaskMgr(iconTile, Color::fromHex(0x141A28), Color::fromHex(0x00FF9D));
        } else if (app.id == "sentinel") {
            surface.drawVectorShield(iconTile, Color::fromHex(0x2E7D32), Color::fromHex(0x81C784));
        } else if (app.id == "netbird") {
            surface.drawVectorMesh(iconTile, Color::fromHex(0x00D4FF), Color::fromHex(0x006699));
        } else if (app.id == "settings") {
            surface.drawVectorGear(iconTile, Color::fromHex(0x78909C));
        } else if (app.id == "prismx") {
            surface.drawVectorPrismIcon(iconTile, palette.accentColor);
        } else {
            surface.drawRoundedRect(iconTile, 6, Color::fromRgba(30, 42, 65, 220), true);
            surface.drawString(iconTile.x + 8, iconTile.y + 12, app.iconGlyph, palette.accentColor, 1);
        }

        // App Title
        surface.drawString(cardRect.x + 50, cardRect.y + 14, app.title, palette.textPrimary, 1);

        // App Subtitle / Description
        std::string dispSub = app.subtitle.empty() ? app.executablePath : app.subtitle;
        if (dispSub.size() > 22) dispSub = dispSub.substr(0, 20) + "..";
        surface.drawString(cardRect.x + 50, cardRect.y + 30, dispSub, palette.textSecondary, 1);
    }

    // 6. Recent / Sovereign System Activity Section
    const int32_t recentY = gridStartY + 3 * (cardH + cardGap) + 16;
    surface.drawString(menuBounds.x + 24, recentY, "SOVEREIGN SYSTEM TOOLS", palette.textSecondary, 1);
    surface.fillRect(Rect{menuBounds.x + 24, recentY + 16, menuBounds.width - 48, 1}, palette.startCardBorder);

    // Quick Tool Badges
    const char* quickTools[] = {"[PQ WireGuard]", "[Zero Telemetry]", "[MicaNT DWM 120Hz]"};
    for (int32_t t = 0; t < 3; ++t) {
        Rect toolBadge{menuBounds.x + 24 + t * 156, recentY + 26, 146, 28};
        surface.drawRoundedRect(toolBadge, 6, Color::fromRgba(25, 38, 60, 180), true);
        surface.drawRoundedRect(toolBadge, 6, Color::fromRgba(50, 75, 115, 120), false);
        surface.drawString(toolBadge.x + 10, toolBadge.y + 10, quickTools[t], palette.accentSecondary, 1);
    }

    // 7. Bottom User Profile & Cutler Power Strip
    const int32_t footerY = menuBounds.bottom() - 56;
    surface.fillRect(Rect{menuBounds.x, footerY, menuBounds.width, 1}, palette.startMenuBorder);

    // User Avatar & Name
    Rect avatarBox{menuBounds.x + 20, footerY + 12, 32, 32};
    surface.drawRoundedRect(avatarBox, 16, palette.accentColor, true);
    surface.drawString(avatarBox.x + 10, avatarBox.y + 12, "S", Color::fromHex(0x000000), 1);

    surface.drawString(avatarBox.right() + 12, footerY + 14, "ssfdre38", palette.textPrimary, 1);
    surface.drawString(avatarBox.right() + 12, footerY + 28, "MicaNT Sovereign Executive", palette.textSecondary, 1);

    // Power Buttons: Lock, Sleep, Restart, Shutdown
    const char* powerLabels[] = {"[L]", "[Z]", "[R]", "[X]"};
    const char* powerTips[] = {"Lock", "Sleep", "Restart", "Shutdown"};
    (void)powerTips;
    const int32_t pStartX = menuBounds.right() - 170;

    for (int32_t p = 0; p < 4; ++p) {
        Rect pRect{pStartX + p * 38, footerY + 12, 32, 32};
        Color pBg = (hoveredPowerIndex_ == p) ? Color::fromRgba(255, 255, 255, 40) : Color::fromRgba(25, 35, 55, 180);
        Color pBorder = (hoveredPowerIndex_ == p) ? palette.accentColor : palette.startCardBorder;
        surface.drawRoundedRect(pRect, 6, pBg, true);
        surface.drawRoundedRect(pRect, 6, pBorder, false);
        surface.drawString(pRect.x + 6, pRect.y + 12, powerLabels[p], palette.textPrimary, 1);
    }
}

} // namespace surshell
