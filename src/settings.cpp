// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/settings.cpp)
// ============================================================================

#include "surshell/settings.hpp"
#include <algorithm>

namespace surshell {

SettingsContent::SettingsContent() {
    accentColors_ = {
        {"Cutler Cyan",   Color::fromHex(0x00D4FF), {}},
        {"Emerald Neon",  Color::fromHex(0x00FF9D), {}},
        {"Solar Amber",   Color::fromHex(0xFFB703), {}},
        {"Crimson Coral", Color::fromHex(0xFF4D6D), {}},
        {"DEC Purple",    Color::fromHex(0x9C27B0), {}},
        {"Royal Cobalt",  Color::fromHex(0x2A69FF), {}}
    };

    wallpapers_ = {
        {"Mica Grid",       WallpaperStyle::MicaGrid,       {}},
        {"Aurora Borealis", WallpaperStyle::AuroraBorealis, {}},
        {"Sovereign Slate", WallpaperStyle::SovereignSlate, {}},
        {"Midnight Nebula", WallpaperStyle::MidnightNebula, {}}
    };
}

void SettingsContent::render(Surface& s) {
    const auto& palette = ThemeManager::instance().palette();
    s.clear(Color{12, 16, 24, 255});

    renderSidebar(s, palette);

    const Rect contentR{190, 0, static_cast<int32_t>(s.width()) - 190, static_cast<int32_t>(s.height())};
    switch (activeCategory_) {
        case SettingsCategory::System:
            renderSystemPage(s, palette, contentR);
            break;
        case SettingsCategory::Personalization:
            renderPersonalizationPage(s, palette, contentR);
            break;
        case SettingsCategory::TaskbarDock:
            renderTaskbarPage(s, palette, contentR);
            break;
        case SettingsCategory::Network:
            renderNetworkPage(s, palette, contentR);
            break;
        case SettingsCategory::About:
            renderAboutPage(s, palette, contentR);
            break;
    }
}

void SettingsContent::renderSidebar(Surface& s, const ThemePalette& palette) {
    const int32_t sidebarW = 190;
    const int32_t h = static_cast<int32_t>(s.height());

    // Sidebar backdrop and separator
    s.fillRect(Rect{0, 0, sidebarW, h}, Color::fromHex(0x101520));
    s.fillRect(Rect{sidebarW - 1, 0, 1, h}, Color::fromHex(0x243248));

    // Header
    IconRenderer::draw(s, IconId::Settings, Point{16, 14}, 20, palette.accentColor);
    s.drawString(42, 16, "Settings", palette.textPrimary, 1);
    s.drawString(115, 18, "v2026.1", palette.textSecondary, 1);

    // Categories
    struct CatItem {
        SettingsCategory cat;
        std::string label;
        IconId icon;
    };
    const CatItem items[] = {
        {SettingsCategory::System,          "System",         IconId::ThisPC},
        {SettingsCategory::Personalization, "Personalization",IconId::Personalization},
        {SettingsCategory::TaskbarDock,     "Taskbar & Dock", IconId::Settings},
        {SettingsCategory::Network,         "Network",        IconId::NetworkOnline},
        {SettingsCategory::About,           "About MicaNT",   IconId::StartPrism}
    };

    categoryBounds_.clear();
    for (int32_t i = 0; i < 5; ++i) {
        const Rect itemR{8, 52 + i * 38, sidebarW - 16, 32};
        categoryBounds_.push_back(itemR);

        const bool isActive = (activeCategory_ == items[i].cat);
        const bool isHover = (hoveredCategory_ == i);

        if (isActive) {
            s.drawRoundedRect(itemR, 6, Color::fromRgba(0, 212, 255, 35), true);
            s.drawRoundedRect(itemR, 6, Color::fromRgba(0, 212, 255, 140), false);
            // Indicator pill on left
            s.fillRect(Rect{itemR.x + 2, itemR.y + 6, 3, itemR.height - 12}, palette.accentColor);
        } else if (isHover) {
            s.drawRoundedRect(itemR, 6, Color::fromRgba(255, 255, 255, 15), true);
        }

        IconRenderer::draw(s, items[i].icon, Point{itemR.x + 10, itemR.y + 7}, 16,
                           isActive ? std::make_optional(palette.accentColor) : std::nullopt);
        s.drawString(itemR.x + 32, itemR.y + 9, items[i].label,
                     isActive ? palette.textPrimary : palette.textSecondary, 1);
    }
}

void SettingsContent::renderSystemPage(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t startX = r.x + 24;
    int32_t curY = r.y + 20;

    s.drawString(startX, curY, "System Specifications & Hardware", palette.textPrimary, 1);
    curY += 28;

    // Device Summary Card
    const Rect cardR{startX, curY, r.width - 48, 136};
    s.drawRoundedRect(cardR, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(cardR, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::ThisPC, Point{cardR.x + 16, cardR.y + 16}, 32, palette.accentColor);
    s.drawString(cardR.x + 60, cardR.y + 16, "MICANT-WORKSTATION", palette.textPrimary, 1);
    s.drawString(cardR.x + 60, cardR.y + 34, "MicaNT Enterprise 64-Bit | PASSIVE_LEVEL", Color::fromHex(0x00FF9D), 1);

    s.drawString(cardR.x + 16, cardR.y + 60,  "Processor:    MicaNT Sovereign vCPU @ 3.80 GHz (16C/32T)", palette.textSecondary, 1);
    s.drawString(cardR.x + 16, cardR.y + 78,  "Memory:       32.0 GB Sovereign RAM (0 Page Faults)", palette.textSecondary, 1);
    s.drawString(cardR.x + 16, cardR.y + 96,  "Display:      1920 x 1080 @ 120Hz VSync (PrismX DWM)", palette.textSecondary, 1);
    s.drawString(cardR.x + 16, cardR.y + 114, "Architecture: Clean-Room ISO C++23 Native Executive", palette.textSecondary, 1);

    curY += 152;

    // System Vitals & Security Card
    const Rect secR{startX, curY, r.width - 48, 110};
    s.drawRoundedRect(secR, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(secR, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::SentinelSec, Point{secR.x + 16, secR.y + 16}, 24, Color::fromHex(0x00FF9D));
    s.drawString(secR.x + 50, secR.y + 18, "Kernel Security & Isolation Status", palette.textPrimary, 1);
    s.drawString(secR.x + 16, secR.y + 48, "SentinelSec Guard: Active  |  Enclave Isolation: Enabled", palette.textSecondary, 1);
    s.drawString(secR.x + 16, secR.y + 66, "Cloud Telemetry:   PURGED  |  Diagnostic Outbound: 0 B", palette.textSecondary, 1);
    s.drawString(secR.x + 16, secR.y + 84, "Kernel LPC Port:   \\RPC_Control\\SurWinLpc (CSRSS Parity)", Color::fromHex(0x00D4FF), 1);
}

void SettingsContent::renderPersonalizationPage(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t startX = r.x + 24;
    int32_t curY = r.y + 18;

    s.drawString(startX, curY, "Personalization & Visual Theming", palette.textPrimary, 1);
    curY += 26;

    // 1. Theme Mode
    s.drawString(startX, curY, "Theme Mode", palette.textSecondary, 1);
    curY += 18;

    const int32_t btnW = 100;
    const int32_t btnH = 28;
    btnDarkTheme_ = Rect{startX, curY, btnW, btnH};
    btnLightTheme_ = Rect{startX + btnW + 10, curY, btnW, btnH};
    btnCarbonTheme_ = Rect{startX + (btnW + 10) * 2, curY, btnW + 20, btnH};

    auto drawPill = [&](const Rect& rect, const std::string& label, bool active) {
        s.drawRoundedRect(rect, 6, active ? Color::fromRgba(0, 212, 255, 45) : Color::fromHex(0x182436), true);
        s.drawRoundedRect(rect, 6, active ? palette.accentColor : Color::fromHex(0x354765), false);
        s.drawString(rect.centerX() - static_cast<int32_t>(label.size() * 3), rect.y + 8, label,
                     active ? Color::fromHex(0xFFFFFF) : palette.textSecondary, 1);
    };

    drawPill(btnDarkTheme_, "Dark", currentThemeMode_ == ThemeMode::Dark);
    drawPill(btnLightTheme_, "Light", currentThemeMode_ == ThemeMode::Light);
    drawPill(btnCarbonTheme_, "Carbon Slate", currentThemeMode_ == ThemeMode::CarbonSlate);

    curY += 40;

    // 2. Accent Color Palette
    s.drawString(startX, curY, "System Accent Color", palette.textSecondary, 1);
    curY += 18;

    const int32_t swatchSize = 28;
    const int32_t swatchSpacing = 12;
    for (size_t i = 0; i < accentColors_.size(); ++i) {
        accentColors_[i].bounds = Rect{startX + static_cast<int32_t>(i) * (swatchSize + swatchSpacing), curY, swatchSize, swatchSize};
        const bool isSelected = (accentColors_[i].color.toRgba() == palette.accentColor.toRgba());

        s.drawRoundedRect(accentColors_[i].bounds, swatchSize / 2, accentColors_[i].color, true);
        if (isSelected) {
            s.drawRoundedRect(accentColors_[i].bounds.inflate(3, 3), swatchSize / 2 + 3, Color::fromHex(0xFFFFFF), false);
        } else {
            s.drawRoundedRect(accentColors_[i].bounds, swatchSize / 2, Color::fromHex(0x203045), false);
        }
    }

    curY += 46;

    // 3. Desktop Wallpaper
    s.drawString(startX, curY, "Desktop Wallpaper Style", palette.textSecondary, 1);
    curY += 18;

    const int32_t wpW = 120;
    const int32_t wpH = 64;
    for (size_t i = 0; i < wallpapers_.size(); ++i) {
        wallpapers_[i].bounds = Rect{startX + static_cast<int32_t>(i) * (wpW + 12), curY, wpW, wpH};
        const bool isSelected = (wallpapers_[i].style == currentWallpaper_);

        s.drawRoundedRect(wallpapers_[i].bounds, 6, isSelected ? Color::fromRgba(0, 212, 255, 40) : Color::fromHex(0x182436), true);
        s.drawRoundedRect(wallpapers_[i].bounds, 6, isSelected ? palette.accentColor : Color::fromHex(0x354765), false);

        // Wallpaper mini preview swatch inside
        Rect previewR{wallpapers_[i].bounds.x + 8, wallpapers_[i].bounds.y + 8, wpW - 16, 28};
        if (wallpapers_[i].style == WallpaperStyle::MicaGrid) {
            s.drawVerticalGradient(previewR, Color::fromHex(0x0E1420), Color::fromHex(0x06090F));
        } else if (wallpapers_[i].style == WallpaperStyle::AuroraBorealis) {
            s.drawVerticalGradient(previewR, Color::fromHex(0x061224), Color::fromHex(0x03060C));
            s.fillRect(Rect{previewR.x, previewR.y + 12, previewR.width, 2}, Color::fromHex(0x00FF9D));
        } else if (wallpapers_[i].style == WallpaperStyle::SovereignSlate) {
            s.drawVerticalGradient(previewR, Color::fromHex(0x1A2332), Color::fromHex(0x0A0F16));
        } else {
            s.drawVerticalGradient(previewR, Color::fromHex(0x1B0E28), Color::fromHex(0x06030A));
            s.putPixel(previewR.centerX(), previewR.centerY(), Color::fromHex(0xFFFFFF));
        }

        s.drawString(wallpapers_[i].bounds.x + 8, wallpapers_[i].bounds.y + 44, wallpapers_[i].name,
                     isSelected ? Color::fromHex(0xFFFFFF) : palette.textSecondary, 1);
    }
}

void SettingsContent::renderTaskbarPage(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t startX = r.x + 24;
    int32_t curY = r.y + 18;

    s.drawString(startX, curY, "Taskbar & Sovereign Dock Configuration", palette.textPrimary, 1);
    curY += 28;

    // 1. Taskbar Alignment
    s.drawString(startX, curY, "Taskbar Alignment", palette.textSecondary, 1);
    curY += 18;

    const int32_t btnW = 120;
    const int32_t btnH = 30;
    btnAlignCenter_ = Rect{startX, curY, btnW, btnH};
    btnAlignLeft_ = Rect{startX + btnW + 12, curY, btnW, btnH};

    auto drawPill = [&](const Rect& rect, const std::string& label, bool active) {
        s.drawRoundedRect(rect, 6, active ? Color::fromRgba(0, 212, 255, 45) : Color::fromHex(0x182436), true);
        s.drawRoundedRect(rect, 6, active ? palette.accentColor : Color::fromHex(0x354765), false);
        s.drawString(rect.centerX() - static_cast<int32_t>(label.size() * 3), rect.y + 9, label,
                     active ? Color::fromHex(0xFFFFFF) : palette.textSecondary, 1);
    };

    drawPill(btnAlignCenter_, "Center Island", currentTaskbarAlignment_ == TaskbarAlignment::Center);
    drawPill(btnAlignLeft_, "Left Classic", currentTaskbarAlignment_ == TaskbarAlignment::Left);

    curY += 46;

    // 2. Taskbar Style
    s.drawString(startX, curY, "Taskbar Style", palette.textSecondary, 1);
    curY += 18;

    btnStyleIsland_ = Rect{startX, curY, btnW, btnH};
    btnStyleDock_ = Rect{startX + btnW + 12, curY, btnW, btnH};

    drawPill(btnStyleIsland_, "Floating Island", currentTaskbarStyle_ == TaskbarStyle::FloatingIsland);
    drawPill(btnStyleDock_, "Edge-to-Edge", currentTaskbarStyle_ == TaskbarStyle::EdgeToEdge);

    curY += 46;

    // 3. Top Diagnostic Header Bar
    s.drawString(startX, curY, "Architectural Diagnostic Bar (Cutler IRQL 0 HUD)", palette.textSecondary, 1);
    curY += 18;

    btnTopBar_ = Rect{startX, curY, 210, btnH};
    drawPill(btnTopBar_, topBarEnabled_ ? "Top Bar: Enabled [ON]" : "Top Bar: Disabled [OFF]", topBarEnabled_);
}

void SettingsContent::renderNetworkPage(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t startX = r.x + 24;
    int32_t curY = r.y + 18;

    s.drawString(startX, curY, "Network Connections & Adapter Telemetry", palette.textPrimary, 1);
    curY += 28;

    const Rect cardR{startX, curY, r.width - 48, 180};
    s.drawRoundedRect(cardR, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(cardR, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::NetworkEthernet, Point{cardR.x + 16, cardR.y + 16}, 32, Color::fromHex(0x00FF9D));
    s.drawString(cardR.x + 60, cardR.y + 18, "Gigabit Ethernet (Clean-Room Realtek Driver)", palette.textPrimary, 1);
    s.drawString(cardR.x + 60, cardR.y + 36, "Status: Connected | 1000/1000 Mbps Full Duplex", Color::fromHex(0x00FF9D), 1);

    s.drawString(cardR.x + 16, cardR.y + 68,  "IPv4 Address:       192.168.1.105 / 24 (Static Local)", palette.textSecondary, 1);
    s.drawString(cardR.x + 16, cardR.y + 88,  "Subnet Mask:        255.255.255.0", palette.textSecondary, 1);
    s.drawString(cardR.x + 16, cardR.y + 108, "Default Gateway:    192.168.1.1", palette.textSecondary, 1);
    s.drawString(cardR.x + 16, cardR.y + 128, "DNS Resolver:       1.1.1.1 (Sovereign DNS Resolver)", palette.textSecondary, 1);
    s.drawString(cardR.x + 16, cardR.y + 148, "External Telemetry: 0 Bytes Transmitted (Zero Collection)", Color::fromHex(0x00D4FF), 1);
}

void SettingsContent::renderAboutPage(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t startX = r.x + 24;
    int32_t curY = r.y + 18;

    s.drawString(startX, curY, "About MicaNT Sovereign Workstation", palette.textPrimary, 1);
    curY += 28;

    const Rect bannerR{startX, curY, r.width - 48, 180};
    s.drawRoundedRect(bannerR, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(bannerR, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::StartPrism, Point{bannerR.x + 16, bannerR.y + 16}, 32, palette.accentColor);
    s.drawString(bannerR.x + 60, bannerR.y + 18, "MicaNT Sovereign Desktop Shell (SurShell)", palette.textPrimary, 1);
    s.drawString(bannerR.x + 60, bannerR.y + 36, "Clean-Room Sovereign Architecture | Release 2026.1", Color::fromHex(0x00D4FF), 1);

    s.drawString(bannerR.x + 16, bannerR.y + 68,  "Build Identifier:   10.0.26100.1-SOVEREIGN (x86_64)", palette.textSecondary, 1);
    s.drawString(bannerR.x + 16, bannerR.y + 88,  "Programming Spec:   ISO C++23 Pure Native Implementation", palette.textSecondary, 1);
    s.drawString(bannerR.x + 16, bannerR.y + 108, "Compositor:         PrismX DWM Composition (Mica/Acrylic)", palette.textSecondary, 1);
    s.drawString(bannerR.x + 16, bannerR.y + 128, "License Provenance: Pure Clean-Room Independent Authoring", palette.textSecondary, 1);
    s.drawString(bannerR.x + 16, bannerR.y + 148, "Kernel Protocol:    Dave Cutler Specification (SurWin LPC)", Color::fromHex(0x00FF9D), 1);
}

bool SettingsContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    // 1. Check sidebar categories
    for (size_t i = 0; i < categoryBounds_.size(); ++i) {
        if (categoryBounds_[i].contains(localPt)) {
            activeCategory_ = static_cast<SettingsCategory>(i);
            return true;
        }
    }

    // 2. Check category pages
    if (activeCategory_ == SettingsCategory::Personalization) {
        if (btnDarkTheme_.contains(localPt)) {
            currentThemeMode_ = ThemeMode::Dark;
            if (onThemeMode_) onThemeMode_(ThemeMode::Dark);
            return true;
        }
        if (btnLightTheme_.contains(localPt)) {
            currentThemeMode_ = ThemeMode::Light;
            if (onThemeMode_) onThemeMode_(ThemeMode::Light);
            return true;
        }
        if (btnCarbonTheme_.contains(localPt)) {
            currentThemeMode_ = ThemeMode::CarbonSlate;
            if (onThemeMode_) onThemeMode_(ThemeMode::CarbonSlate);
            return true;
        }

        for (const auto& swatch : accentColors_) {
            if (swatch.bounds.contains(localPt)) {
                if (onAccentColor_) onAccentColor_(swatch.color);
                return true;
            }
        }

        for (const auto& wp : wallpapers_) {
            if (wp.bounds.contains(localPt)) {
                currentWallpaper_ = wp.style;
                if (onWallpaper_) onWallpaper_(wp.style);
                return true;
            }
        }
    } else if (activeCategory_ == SettingsCategory::TaskbarDock) {
        if (btnAlignCenter_.contains(localPt)) {
            currentTaskbarAlignment_ = TaskbarAlignment::Center;
            if (onTaskbarAlignment_) onTaskbarAlignment_(TaskbarAlignment::Center);
            return true;
        }
        if (btnAlignLeft_.contains(localPt)) {
            currentTaskbarAlignment_ = TaskbarAlignment::Left;
            if (onTaskbarAlignment_) onTaskbarAlignment_(TaskbarAlignment::Left);
            return true;
        }
        if (btnStyleIsland_.contains(localPt)) {
            currentTaskbarStyle_ = TaskbarStyle::FloatingIsland;
            if (onTaskbarStyle_) onTaskbarStyle_(TaskbarStyle::FloatingIsland);
            return true;
        }
        if (btnStyleDock_.contains(localPt)) {
            currentTaskbarStyle_ = TaskbarStyle::EdgeToEdge;
            if (onTaskbarStyle_) onTaskbarStyle_(TaskbarStyle::EdgeToEdge);
            return true;
        }
        if (btnTopBar_.contains(localPt)) {
            topBarEnabled_ = !topBarEnabled_;
            if (onTopBar_) onTopBar_(topBarEnabled_);
            return true;
        }
    }

    return false;
}

bool SettingsContent::onMouseMove(Point localPt) {
    int32_t prevHover = hoveredCategory_;
    hoveredCategory_ = -1;
    for (size_t i = 0; i < categoryBounds_.size(); ++i) {
        if (categoryBounds_[i].contains(localPt)) {
            hoveredCategory_ = static_cast<int32_t>(i);
            break;
        }
    }
    return prevHover != hoveredCategory_;
}

} // namespace surshell
