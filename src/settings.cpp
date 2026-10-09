// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/settings.cpp)
// ============================================================================

#include "surshell/settings.hpp"
#include "surshell/kernel_bridge.hpp"
#include <algorithm>
#include <sstream>

namespace surshell {

SettingsContent::SettingsContent() {
    deviceName_ = KernelBridge::queryComputerName();

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

void SettingsContent::setSearchQuery(std::string q) {
    searchQuery_ = std::move(q);
    executeSearchFilter();
}

void SettingsContent::executeSearchFilter() {
    if (searchQuery_.empty()) return;

    std::string lowerQ = searchQuery_;
    std::transform(lowerQ.begin(), lowerQ.end(), lowerQ.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (lowerQ.find("vol") != std::string::npos || lowerQ.find("sound") != std::string::npos ||
        lowerQ.find("audio") != std::string::npos || lowerQ.find("disp") != std::string::npos ||
        lowerQ.find("scale") != std::string::npos || lowerQ.find("hdr") != std::string::npos ||
        lowerQ.find("power") != std::string::npos || lowerQ.find("stor") != std::string::npos ||
        lowerQ.find("snap") != std::string::npos || lowerQ.find("cpu") != std::string::npos ||
        lowerQ.find("ram") != std::string::npos) {
        activeCategory_ = SettingsCategory::System;
    } else if (lowerQ.find("theme") != std::string::npos || lowerQ.find("dark") != std::string::npos ||
               lowerQ.find("light") != std::string::npos || lowerQ.find("color") != std::string::npos ||
               lowerQ.find("accent") != std::string::npos || lowerQ.find("wall") != std::string::npos ||
               lowerQ.find("trans") != std::string::npos || lowerQ.find("carbon") != std::string::npos) {
        activeCategory_ = SettingsCategory::Personalization;
    } else if (lowerQ.find("task") != std::string::npos || lowerQ.find("dock") != std::string::npos ||
               lowerQ.find("align") != std::string::npos || lowerQ.find("island") != std::string::npos ||
               lowerQ.find("top") != std::string::npos || lowerQ.find("badge") != std::string::npos ||
               lowerQ.find("auto") != std::string::npos) {
        activeCategory_ = SettingsCategory::TaskbarDock;
    } else if (lowerQ.find("net") != std::string::npos || lowerQ.find("wifi") != std::string::npos ||
               lowerQ.find("dns") != std::string::npos || lowerQ.find("ether") != std::string::npos ||
               lowerQ.find("firewall") != std::string::npos || lowerQ.find("ip") != std::string::npos) {
        activeCategory_ = SettingsCategory::Network;
    } else if (lowerQ.find("app") != std::string::npos || lowerQ.find("term") != std::string::npos ||
               lowerQ.find("start") != std::string::npos || lowerQ.find("calc") != std::string::npos ||
               lowerQ.find("expl") != std::string::npos || lowerQ.find("cmd") != std::string::npos) {
        activeCategory_ = SettingsCategory::Apps;
    } else if (lowerQ.find("sec") != std::string::npos || lowerQ.find("priv") != std::string::npos ||
               lowerQ.find("isol") != std::string::npos || lowerQ.find("dma") != std::string::npos ||
               lowerQ.find("cam") != std::string::npos || lowerQ.find("mic") != std::string::npos ||
               lowerQ.find("telem") != std::string::npos || lowerQ.find("sentinel") != std::string::npos) {
        activeCategory_ = SettingsCategory::PrivacySecurity;
    } else if (lowerQ.find("time") != std::string::npos || lowerQ.find("clock") != std::string::npos ||
               lowerQ.find("date") != std::string::npos || lowerQ.find("zone") != std::string::npos ||
               lowerQ.find("24") != std::string::npos || lowerQ.find("sync") != std::string::npos ||
               lowerQ.find("ntp") != std::string::npos) {
        activeCategory_ = SettingsCategory::TimeLanguage;
    } else if (lowerQ.find("dev") != std::string::npos || lowerQ.find("reg") != std::string::npos ||
               lowerQ.find("lpc") != std::string::npos || lowerQ.find("debug") != std::string::npos ||
               lowerQ.find("taskmgr") != std::string::npos) {
        activeCategory_ = SettingsCategory::Developer;
    } else if (lowerQ.find("about") != std::string::npos || lowerQ.find("spec") != std::string::npos ||
               lowerQ.find("build") != std::string::npos || lowerQ.find("cutler") != std::string::npos ||
               lowerQ.find("rename") != std::string::npos) {
        activeCategory_ = SettingsCategory::About;
    }
}

static void drawPill(Surface& s, const Rect& rect, const std::string& label, bool active, const ThemePalette& palette) {
    s.drawRoundedRect(rect, 6, active ? Color::fromRgba(0, 212, 255, 45) : Color::fromHex(0x182436), true);
    s.drawRoundedRect(rect, 6, active ? palette.accentColor : Color::fromHex(0x354765), false);
    const int32_t textX = rect.centerX() - static_cast<int32_t>(label.size() * 4);
    s.drawString(textX, rect.y + (rect.height - 14) / 2, label,
                 active ? Color::fromHex(0xFFFFFF) : palette.textSecondary, 1);
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
        case SettingsCategory::Apps:
            renderAppsPage(s, palette, contentR);
            break;
        case SettingsCategory::PrivacySecurity:
            renderPrivacyPage(s, palette, contentR);
            break;
        case SettingsCategory::TimeLanguage:
            renderTimePage(s, palette, contentR);
            break;
        case SettingsCategory::Developer:
            renderDeveloperPage(s, palette, contentR);
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
    IconRenderer::draw(s, IconId::Settings, Point{16, 12}, 20, palette.accentColor);
    s.drawString(42, 14, "Settings", palette.textPrimary, 1);
    s.drawString(115, 16, "v2026.1", palette.textSecondary, 1);

    // Search Box
    searchBoxBounds_ = Rect{8, 38, sidebarW - 16, 24};
    s.drawRoundedRect(searchBoxBounds_, 4, searchFocused_ ? Color::fromHex(0x18263A) : Color::fromHex(0x141C2A), true);
    s.drawRoundedRect(searchBoxBounds_, 4, searchFocused_ ? palette.accentColor : Color::fromHex(0x283850), false);
    IconRenderer::draw(s, IconId::Search, Point{searchBoxBounds_.x + 6, searchBoxBounds_.y + 5}, 14, palette.textSecondary);

    if (searchQuery_.empty()) {
        s.drawString(searchBoxBounds_.x + 24, searchBoxBounds_.y + 6, "Search...", Color::fromHex(0x526782), 1);
    } else {
        s.drawString(searchBoxBounds_.x + 24, searchBoxBounds_.y + 6, searchQuery_, palette.textPrimary, 1);
        searchClearBounds_ = Rect{searchBoxBounds_.right() - 18, searchBoxBounds_.y + 4, 14, 16};
        s.drawString(searchClearBounds_.x, searchClearBounds_.y + 1, "x", palette.textSecondary, 1);
    }

    // Categories
    struct CatItem {
        SettingsCategory cat;
        std::string label;
        IconId icon;
    };
    const CatItem items[] = {
        {SettingsCategory::System,          "System",          IconId::ThisPC},
        {SettingsCategory::Personalization, "Personalization", IconId::Personalization},
        {SettingsCategory::TaskbarDock,     "Taskbar & Dock",  IconId::TaskView},
        {SettingsCategory::Network,         "Network",         IconId::NetworkOnline},
        {SettingsCategory::Apps,            "Apps & Features", IconId::TerminalTab},
        {SettingsCategory::PrivacySecurity, "Privacy/Security",IconId::SentinelSec},
        {SettingsCategory::TimeLanguage,    "Time & Language", IconId::Clock},
        {SettingsCategory::Developer,       "Developer/Enclave",IconId::Registry},
        {SettingsCategory::About,           "About MicaNT",    IconId::StartPrism}
    };

    categoryBounds_.clear();
    for (int32_t i = 0; i < 9; ++i) {
        const Rect itemR{8, 68 + i * 34, sidebarW - 16, 30};
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

        IconRenderer::draw(s, items[i].icon, Point{itemR.x + 8, itemR.y + 7}, 16,
                           isActive ? std::make_optional(palette.accentColor) : std::nullopt);
        s.drawString(itemR.x + 30, itemR.y + 8, items[i].label,
                     isActive ? palette.textPrimary : palette.textSecondary, 1);
    }
}

void SettingsContent::renderSystemPage(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t startX = r.x + 24;
    int32_t curY = r.y + 16;

    s.drawString(startX, curY, "System Specifications & Hardware Subsystems", palette.textPrimary, 1);
    curY += 26;

    // 1. Device Summary Card
    const Rect cardR{startX, curY, r.width - 48, 114};
    s.drawRoundedRect(cardR, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(cardR, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::ThisPC, Point{cardR.x + 14, cardR.y + 14}, 28, palette.accentColor);
    s.drawString(cardR.x + 52, cardR.y + 14, deviceName_, palette.textPrimary, 1);
    s.drawString(cardR.x + 52, cardR.y + 30, "MicaNT Enterprise 64-Bit | PASSIVE_LEVEL Native", Color::fromHex(0x00FF9D), 1);

    s.drawString(cardR.x + 14, cardR.y + 52, "Processor:    MicaNT Sovereign vCPU @ 3.80 GHz (16C/32T)", palette.textSecondary, 1);
    s.drawString(cardR.x + 14, cardR.y + 70, "Memory:       32.0 GB Sovereign RAM (0 Page Faults)", palette.textSecondary, 1);
    s.drawString(cardR.x + 14, cardR.y + 88, "Display:      1920 x 1080 @ 120Hz VSync (PrismX DWM)", palette.textSecondary, 1);

    curY += 124;

    // 2. Display & Graphics Card
    const Rect dispCard{startX, curY, (r.width - 48 - 12) / 2, 112};
    s.drawRoundedRect(dispCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(dispCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::Display, Point{dispCard.x + 12, dispCard.y + 12}, 18, palette.accentColor);
    s.drawString(dispCard.x + 36, dispCard.y + 14, "Display & Graphics", palette.textPrimary, 1);

    btnScale100_ = Rect{dispCard.x + 12, dispCard.y + 38, 100, 26};
    btnScale125_ = Rect{dispCard.x + 118, dispCard.y + 38, 70, 26};
    drawPill(s, btnScale100_, "100%", displayScaling_ == DisplayScaling::Scale100, palette);
    drawPill(s, btnScale125_, "125%", displayScaling_ == DisplayScaling::Scale125, palette);

    btnRefresh120_ = Rect{dispCard.x + 12, dispCard.y + 72, 100, 26};
    btnHdrToggle_ = Rect{dispCard.x + 118, dispCard.y + 72, 90, 26};
    drawPill(s, btnRefresh120_, refreshRate120Hz_ ? "120Hz [ON]" : "60Hz", refreshRate120Hz_, palette);
    drawPill(s, btnHdrToggle_, hdrEnabled_ ? "HDR [ON]" : "HDR [OFF]", hdrEnabled_, palette);

    // 3. Sound & Audio Card
    const Rect soundCard{dispCard.right() + 12, curY, (r.width - 48 - 12) / 2, 112};
    s.drawRoundedRect(soundCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(soundCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, isMuted_ ? IconId::VolumeMute : IconId::VolumeHigh,
                       Point{soundCard.x + 12, soundCard.y + 12}, 18,
                       isMuted_ ? Color::fromHex(0xFF4D6D) : palette.accentColor);
    s.drawString(soundCard.x + 36, soundCard.y + 14, "Sound & Audio Engine", palette.textPrimary, 1);

    // Volume track
    volumeSliderTrackR_ = Rect{soundCard.x + 12, soundCard.y + 44, soundCard.width - 80, 8};
    s.drawRoundedRect(volumeSliderTrackR_, 4, Color::fromHex(0x203045), true);
    const int32_t filledW = (masterVolume_ * volumeSliderTrackR_.width) / 100;
    if (filledW > 0) {
        s.drawRoundedRect(Rect{volumeSliderTrackR_.x, volumeSliderTrackR_.y, filledW, 8}, 4,
                          isMuted_ ? Color::fromHex(0x607085) : palette.accentColor, true);
    }
    s.drawString(volumeSliderTrackR_.right() + 10, volumeSliderTrackR_.y - 3,
                 isMuted_ ? "MUTE" : std::to_string(masterVolume_) + "%", palette.textPrimary, 1);

    btnVolumeMute_ = Rect{soundCard.x + 12, soundCard.y + 68, 70, 26};
    btnSpatialAudio_ = Rect{soundCard.x + 88, soundCard.y + 68, 120, 26};
    drawPill(s, btnVolumeMute_, isMuted_ ? "Unmute" : "Mute", isMuted_, palette);
    drawPill(s, btnSpatialAudio_, spatialAudio_ ? "Spatial [ON]" : "Spatial [OFF]", spatialAudio_, palette);

    curY += 122;

    // 4. Power & Battery Management Card
    const Rect powerCard{startX, curY, (r.width - 48 - 12) / 2, 104};
    s.drawRoundedRect(powerCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(powerCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::BatteryCharging, Point{powerCard.x + 12, powerCard.y + 12}, 18, Color::fromHex(0x00FF9D));
    s.drawString(powerCard.x + 36, powerCard.y + 14, "Power & Performance Mode", palette.textPrimary, 1);

    btnPowerPerf_ = Rect{powerCard.x + 12, powerCard.y + 40, 88, 26};
    btnPowerBal_ = Rect{powerCard.x + 106, powerCard.y + 40, 88, 26};
    btnPowerSave_ = Rect{powerCard.x + 200, powerCard.y + 40, 88, 26};
    drawPill(s, btnPowerPerf_, "Max Perf", powerMode_ == PowerMode::BestPerformance, palette);
    drawPill(s, btnPowerBal_, "Balanced", powerMode_ == PowerMode::Balanced, palette);
    drawPill(s, btnPowerSave_, "Saver", powerMode_ == PowerMode::PowerSaver, palette);
    s.drawString(powerCard.x + 12, powerCard.y + 76, "Standby: S0ix Modern Standby", palette.textSecondary, 1);

    // 5. Storage Pools Card
    const Rect storCard{powerCard.right() + 12, curY, (r.width - 48 - 12) / 2, 104};
    s.drawRoundedRect(storCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(storCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::DriveStorage, Point{storCard.x + 12, storCard.y + 12}, 18, palette.accentColor);
    s.drawString(storCard.x + 36, storCard.y + 14, "Storage & Partitions", palette.textPrimary, 1);

    // C: Drive bar
    s.drawString(storCard.x + 12, storCard.y + 38, "C:\\ Sovereign OS (128 GB / 512 GB)", palette.textSecondary, 1);
    const Rect cBar{storCard.x + 12, storCard.y + 54, storCard.width - 24, 6};
    s.drawRoundedRect(cBar, 3, Color::fromHex(0x243248), true);
    s.drawRoundedRect(Rect{cBar.x, cBar.y, cBar.width / 4, 6}, 3, Color::fromHex(0x00FF9D), true);

    btnStorageSense_ = Rect{storCard.x + 12, storCard.y + 68, 150, 24};
    drawPill(s, btnStorageSense_, storageSense_ ? "Storage Sense: ON" : "Storage Sense: OFF", storageSense_, palette);
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

    drawPill(s, btnDarkTheme_, "Dark", currentThemeMode_ == ThemeMode::Dark, palette);
    drawPill(s, btnLightTheme_, "Light", currentThemeMode_ == ThemeMode::Light, palette);
    drawPill(s, btnCarbonTheme_, "Carbon Slate", currentThemeMode_ == ThemeMode::CarbonSlate, palette);

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

    curY += wpH + 24;

    // 4. Transparency Effects
    s.drawString(startX, curY, "Visual Effects & Translucency", palette.textSecondary, 1);
    curY += 18;

    btnTransparency_ = Rect{startX, curY, 260, 28};
    drawPill(s, btnTransparency_,
             transparencyEffects_ ? "Transparency & Mica Glass: [ON]" : "Transparency & Mica Glass: [OFF]",
             transparencyEffects_, palette);
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

    drawPill(s, btnAlignCenter_, "Center Island", currentTaskbarAlignment_ == TaskbarAlignment::Center, palette);
    drawPill(s, btnAlignLeft_, "Left Classic", currentTaskbarAlignment_ == TaskbarAlignment::Left, palette);

    curY += 46;

    // 2. Taskbar Style
    s.drawString(startX, curY, "Taskbar Style", palette.textSecondary, 1);
    curY += 18;

    btnStyleIsland_ = Rect{startX, curY, btnW, btnH};
    btnStyleDock_ = Rect{startX + btnW + 12, curY, btnW, btnH};

    drawPill(s, btnStyleIsland_, "Floating Island", currentTaskbarStyle_ == TaskbarStyle::FloatingIsland, palette);
    drawPill(s, btnStyleDock_, "Edge-to-Edge", currentTaskbarStyle_ == TaskbarStyle::EdgeToEdge, palette);

    curY += 46;

    // 3. Top Diagnostic Header Bar
    s.drawString(startX, curY, "Architectural Diagnostic Bar (Cutler IRQL 0 HUD)", palette.textSecondary, 1);
    curY += 18;

    btnTopBar_ = Rect{startX, curY, 210, btnH};
    drawPill(s, btnTopBar_, topBarEnabled_ ? "Top Bar: Enabled [ON]" : "Top Bar: Disabled [OFF]", topBarEnabled_, palette);

    curY += 46;

    // 4. Taskbar Behaviors
    s.drawString(startX, curY, "Taskbar Behaviors & Indicators", palette.textSecondary, 1);
    curY += 18;

    btnAutoHide_ = Rect{startX, curY, 200, btnH};
    btnBadges_ = Rect{startX + 212, curY, 200, btnH};
    drawPill(s, btnAutoHide_, taskbarAutoHide_ ? "Auto-Hide: [ON]" : "Auto-Hide: [OFF]", taskbarAutoHide_, palette);
    drawPill(s, btnBadges_, taskbarBadges_ ? "App Badges: [ON]" : "App Badges: [OFF]", taskbarBadges_, palette);
}

void SettingsContent::renderNetworkPage(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t startX = r.x + 24;
    int32_t curY = r.y + 18;

    s.drawString(startX, curY, "Network Connections & Adapter Telemetry", palette.textPrimary, 1);
    curY += 28;

    // 1. Ethernet Card
    const auto net = KernelBridge::queryPrimaryNetworkAdapter();
    const Rect cardR{startX, curY, r.width - 48, 140};
    s.drawRoundedRect(cardR, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(cardR, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::NetworkEthernet, Point{cardR.x + 16, cardR.y + 16}, 28, Color::fromHex(0x00FF9D));
    s.drawString(cardR.x + 56, cardR.y + 16, net.description.empty() ? "Gigabit Ethernet Adapter" : net.description, palette.textPrimary, 1);
    s.drawString(cardR.x + 56, cardR.y + 34, "Status: Connected | " + net.linkSpeed, Color::fromHex(0x00FF9D), 1);

    s.drawString(cardR.x + 16, cardR.y + 60,  "IPv4 Address:       " + net.ipv4Address + " / 24" + (net.isDhcp ? " (DHCP Assigned)" : " (Static Local)"), palette.textSecondary, 1);
    s.drawString(cardR.x + 16, cardR.y + 78,  "Subnet Mask:        " + (net.ipv4Mask.empty() ? "255.255.255.0" : net.ipv4Mask), palette.textSecondary, 1);
    s.drawString(cardR.x + 16, cardR.y + 96,  "Default Gateway:    " + (net.defaultGateway.empty() ? "None" : net.defaultGateway), palette.textSecondary, 1);
    s.drawString(cardR.x + 16, cardR.y + 114, "External Telemetry: 0 Bytes Transmitted (Zero Collection)", Color::fromHex(0x00D4FF), 1);

    curY += 152;

    // 2. Wi-Fi Subsystem Card
    const Rect wifiCard{startX, curY, (r.width - 48 - 12) / 2, 110};
    s.drawRoundedRect(wifiCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(wifiCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::NetworkOnline, Point{wifiCard.x + 14, wifiCard.y + 14}, 20,
                       wifiEnabled_ ? Color::fromHex(0x00FF9D) : palette.textSecondary);
    s.drawString(wifiCard.x + 42, wifiCard.y + 16, "Wi-Fi Adapter", palette.textPrimary, 1);

    btnWifiToggle_ = Rect{wifiCard.x + 14, wifiCard.y + 44, 130, 26};
    drawPill(s, btnWifiToggle_, wifiEnabled_ ? "Wi-Fi: [ON]" : "Wi-Fi: [OFF]", wifiEnabled_, palette);
    s.drawString(wifiCard.x + 14, wifiCard.y + 78, "SSID: Sovereign-Mesh-5G", palette.textSecondary, 1);

    // 3. DNS Resolver Card
    const Rect dnsCard{wifiCard.right() + 12, curY, (r.width - 48 - 12) / 2, 110};
    s.drawRoundedRect(dnsCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(dnsCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::SentinelSec, Point{dnsCard.x + 14, dnsCard.y + 14}, 20, palette.accentColor);
    s.drawString(dnsCard.x + 42, dnsCard.y + 16, "Sovereign DNS Resolver", palette.textPrimary, 1);

    btnDnsCloudflare_ = Rect{dnsCard.x + 14, dnsCard.y + 42, 80, 26};
    btnDnsQuad9_ = Rect{dnsCard.x + 100, dnsCard.y + 42, 70, 26};
    btnDnsDhcp_ = Rect{dnsCard.x + 176, dnsCard.y + 42, 70, 26};
    drawPill(s, btnDnsCloudflare_, "1.1.1.1", dnsResolver_ == DnsResolver::CloudflareSovereign, palette);
    drawPill(s, btnDnsQuad9_, "9.9.9.9", dnsResolver_ == DnsResolver::Quad9, palette);
    drawPill(s, btnDnsDhcp_, "DHCP", dnsResolver_ == DnsResolver::LocalGateway, palette);

    btnFirewall_ = Rect{dnsCard.x + 14, dnsCard.y + 74, 170, 24};
    drawPill(s, btnFirewall_, stealthFirewall_ ? "Stealth Firewall: ON" : "Stealth Firewall: OFF", stealthFirewall_, palette);
}

void SettingsContent::renderAppsPage(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t startX = r.x + 24;
    int32_t curY = r.y + 18;

    s.drawString(startX, curY, "Applications, Default Shell & Startup Services", palette.textPrimary, 1);
    curY += 28;

    // 1. Default Terminal & Shell Profile
    const Rect termCard{startX, curY, r.width - 48, 86};
    s.drawRoundedRect(termCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(termCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::Terminal, Point{termCard.x + 16, termCard.y + 16}, 24, palette.accentColor);
    s.drawString(termCard.x + 50, termCard.y + 18, "Default Command Terminal Profile", palette.textPrimary, 1);

    btnTermCmd_ = Rect{termCard.x + 16, termCard.y + 48, 150, 26};
    btnTermPwsh_ = Rect{termCard.x + 174, termCard.y + 48, 140, 26};
    btnTermSurShell_ = Rect{termCard.x + 322, termCard.y + 48, 150, 26};
    drawPill(s, btnTermCmd_, "Command Prompt (cmd)", defaultTerminal_ == DefaultTerminal::Cmd, palette);
    drawPill(s, btnTermPwsh_, "PowerShell (pwsh)", defaultTerminal_ == DefaultTerminal::Pwsh, palette);
    drawPill(s, btnTermSurShell_, "SurShell Native", defaultTerminal_ == DefaultTerminal::SurShell, palette);

    curY += 98;

    // 2. Startup Applications & Daemons
    const Rect startCard{startX, curY, r.width - 48, 90};
    s.drawRoundedRect(startCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(startCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::Settings, Point{startCard.x + 16, startCard.y + 16}, 24, Color::fromHex(0x00FF9D));
    s.drawString(startCard.x + 50, startCard.y + 18, "Startup Applications (Logon Daemons)", palette.textPrimary, 1);

    btnStartupSentinel_ = Rect{startCard.x + 16, startCard.y + 50, 150, 26};
    btnStartupCompositor_ = Rect{startCard.x + 174, startCard.y + 50, 160, 26};
    btnStartupLpc_ = Rect{startCard.x + 342, startCard.y + 50, 160, 26};
    drawPill(s, btnStartupSentinel_, startupSentinel_ ? "SentinelSec: ON" : "SentinelSec: OFF", startupSentinel_, palette);
    drawPill(s, btnStartupCompositor_, startupCompositor_ ? "PrismX DWM: ON" : "PrismX DWM: OFF", startupCompositor_, palette);
    drawPill(s, btnStartupLpc_, startupLpc_ ? "SurWin LPC: ON" : "SurWin LPC: OFF", startupLpc_, palette);

    curY += 102;

    // 3. Installed Sovereign Applications Catalog
    const Rect catCard{startX, curY, r.width - 48, 126};
    s.drawRoundedRect(catCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(catCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::FolderOpen, Point{catCard.x + 16, catCard.y + 14}, 20, palette.accentColor);
    s.drawString(catCard.x + 46, catCard.y + 16, "Installed Sovereign Applications Catalog", palette.textPrimary, 1);

    s.drawString(catCard.x + 16, catCard.y + 44,  "* Windows Terminal System (cmd/pwsh)    | Pure C++23 | 4.2 MB", palette.textSecondary, 1);
    s.drawString(catCard.x + 16, catCard.y + 62,  "* Sovereign File Explorer (Tabs/Nav)    | Pure C++23 | 3.8 MB", palette.textSecondary, 1);
    s.drawString(catCard.x + 16, catCard.y + 80,  "* Modern Sovereign Calculator (Acrylic) | Pure C++23 | 1.4 MB", palette.textSecondary, 1);
    s.drawString(catCard.x + 16, catCard.y + 98,  "* Sovereign Registry Editor (regedit)   | Pure C++23 | 1.8 MB", Color::fromHex(0x00FF9D), 1);
}

void SettingsContent::renderPrivacyPage(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t startX = r.x + 24;
    int32_t curY = r.y + 18;

    s.drawString(startX, curY, "Privacy, SentinelSec Enclave & Isolation Posture", palette.textPrimary, 1);
    curY += 28;

    // 1. SentinelSec Enclave Card
    const Rect secCard{startX, curY, r.width - 48, 110};
    s.drawRoundedRect(secCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(secCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::SentinelSec, Point{secCard.x + 16, secCard.y + 16}, 28, Color::fromHex(0x00FF9D));
    s.drawString(secCard.x + 56, secCard.y + 18, "Kernel Security & Isolation Posture", palette.textPrimary, 1);

    btnCoreIsolation_ = Rect{secCard.x + 16, secCard.y + 56, 170, 26};
    btnDmaProtect_ = Rect{secCard.x + 196, secCard.y + 56, 180, 26};
    drawPill(s, btnCoreIsolation_, coreIsolationHvci_ ? "HVCI Enclave: ACTIVE" : "HVCI: OFF", coreIsolationHvci_, palette);
    drawPill(s, btnDmaProtect_, kernelDmaProtect_ ? "Kernel DMA: ENABLED" : "DMA: OFF", kernelDmaProtect_, palette);
    s.drawString(secCard.x + 16, secCard.y + 88, "Dave Cutler Microkernel Enclave Level 0 | Zero Vulnerabilities", palette.textSecondary, 1);

    curY += 122;

    // 2. Telemetry Purge Card
    const Rect telemCard{startX, curY, r.width - 48, 86};
    s.drawRoundedRect(telemCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(telemCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::ShieldAdmin, Point{telemCard.x + 16, telemCard.y + 16}, 24, palette.accentColor);
    s.drawString(telemCard.x + 50, telemCard.y + 18, "Zero-Telemetry Sovereign Guarantee", palette.textPrimary, 1);
    s.drawString(telemCard.x + 16, telemCard.y + 46, "Cloud Diagnostics: PERMANENTLY PURGED | Outbound Ad-Tracking: 0 B", Color::fromHex(0x00FF9D), 1);
    s.drawString(telemCard.x + 16, telemCard.y + 64, "All operational telemetry resides exclusively in volatile kernel memory.", palette.textSecondary, 1);

    curY += 98;

    // 3. App Sandbox Permissions
    const Rect permCard{startX, curY, r.width - 48, 86};
    s.drawRoundedRect(permCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(permCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::Lock, Point{permCard.x + 16, permCard.y + 16}, 24, palette.accentColor);
    s.drawString(permCard.x + 50, permCard.y + 18, "Application Enclave Permissions", palette.textPrimary, 1);

    btnCameraEnclave_ = Rect{permCard.x + 16, permCard.y + 48, 170, 26};
    btnMicEnclave_ = Rect{permCard.x + 196, permCard.y + 48, 180, 26};
    drawPill(s, btnCameraEnclave_, permCameraPrompt_ ? "Camera: Prompt Guard" : "Camera: Allow", permCameraPrompt_, palette);
    drawPill(s, btnMicEnclave_, permMicPrompt_ ? "Microphone: Prompt Guard" : "Mic: Allow", permMicPrompt_, palette);
}

void SettingsContent::renderTimePage(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t startX = r.x + 24;
    int32_t curY = r.y + 18;

    s.drawString(startX, curY, "Time, Date, Clock Format & Synchronization", palette.textPrimary, 1);
    curY += 28;

    // 1. Clock Format & Synchronization
    const Rect timeCard{startX, curY, r.width - 48, 110};
    s.drawRoundedRect(timeCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(timeCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::Clock, Point{timeCard.x + 16, timeCard.y + 16}, 24, palette.accentColor);
    s.drawString(timeCard.x + 50, timeCard.y + 18, "Clock Display & Format", palette.textPrimary, 1);

    btnClock24H_ = Rect{timeCard.x + 16, timeCard.y + 50, 180, 26};
    btnNtpSync_ = Rect{timeCard.x + 206, timeCard.y + 50, 160, 26};
    btnSyncNow_ = Rect{timeCard.x + 376, timeCard.y + 50, 100, 26};
    drawPill(s, btnClock24H_, clockFormat24H_ ? "24-Hour Military [ON]" : "12-Hour AM/PM", clockFormat24H_, palette);
    drawPill(s, btnNtpSync_, ntpAutoSync_ ? "NTP Auto-Sync: ON" : "NTP: OFF", ntpAutoSync_, palette);
    drawPill(s, btnSyncNow_, "Sync Now", false, palette);
    s.drawString(timeCard.x + 16, timeCard.y + 84, "Current Time: 02:30 PM PDT | NTP Reference: time.cloudflare.com", palette.textSecondary, 1);

    curY += 122;

    // 2. Time Zones Card
    const Rect tzCard{startX, curY, r.width - 48, 90};
    s.drawRoundedRect(tzCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(tzCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::Clock, Point{tzCard.x + 16, tzCard.y + 16}, 20, Color::fromHex(0x00FF9D));
    s.drawString(tzCard.x + 46, tzCard.y + 18, "Time Zone Selection", palette.textPrimary, 1);

    btnTzPacific_ = Rect{tzCard.x + 16, tzCard.y + 46, 120, 26};
    btnTzEastern_ = Rect{tzCard.x + 144, tzCard.y + 46, 120, 26};
    btnTzUtc_ = Rect{tzCard.x + 272, tzCard.y + 46, 80, 26};
    btnTzCet_ = Rect{tzCard.x + 360, tzCard.y + 46, 90, 26};
    drawPill(s, btnTzPacific_, "Pacific (UTC-7)", timeZone_.find("Pacific") != std::string::npos, palette);
    drawPill(s, btnTzEastern_, "Eastern (UTC-4)", timeZone_.find("Eastern") != std::string::npos, palette);
    drawPill(s, btnTzUtc_, "UTC+0", timeZone_.find("UTC+0") != std::string::npos, palette);
    drawPill(s, btnTzCet_, "CET (UTC+1)", timeZone_.find("CET") != std::string::npos, palette);

    curY += 102;

    // 3. Regional Format
    const Rect regCard{startX, curY, r.width - 48, 80};
    s.drawRoundedRect(regCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(regCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::Calendar, Point{regCard.x + 16, regCard.y + 16}, 20, palette.accentColor);
    s.drawString(regCard.x + 46, regCard.y + 18, "Regional Formats & Locale", palette.textPrimary, 1);
    s.drawString(regCard.x + 16, regCard.y + 46, "Regional Format: English (United States) | Calendar: Sunday First", palette.textSecondary, 1);
}

void SettingsContent::renderDeveloperPage(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t startX = r.x + 24;
    int32_t curY = r.y + 18;

    s.drawString(startX, curY, "Developer Mode, Kernel Diagnostics & Tool Launchers", palette.textPrimary, 1);
    curY += 28;

    // 1. Developer Mode Card
    const Rect devCard{startX, curY, r.width - 48, 92};
    s.drawRoundedRect(devCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(devCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::ShieldAdmin, Point{devCard.x + 16, devCard.y + 16}, 24, Color::fromHex(0x00FF9D));
    s.drawString(devCard.x + 50, devCard.y + 18, "Sovereign Developer Mode", palette.textPrimary, 1);

    btnDevMode_ = Rect{devCard.x + 16, devCard.y + 50, 200, 26};
    drawPill(s, btnDevMode_, developerMode_ ? "Developer Mode: ON" : "Developer Mode: OFF", developerMode_, palette);
    s.drawString(devCard.x + 230, devCard.y + 56, "Allows execution of unsigned binaries and LPC hooks.", palette.textSecondary, 1);

    curY += 104;

    // 2. Kernel LPC Subsystem Diagnostics
    const Rect lpcCard{startX, curY, r.width - 48, 86};
    s.drawRoundedRect(lpcCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(lpcCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::TerminalTab, Point{lpcCard.x + 16, lpcCard.y + 16}, 24, palette.accentColor);
    s.drawString(lpcCard.x + 50, lpcCard.y + 18, "Kernel LPC Port Diagnostics (Dave Cutler CSRSS Parity)", palette.textPrimary, 1);
    s.drawString(lpcCard.x + 16, lpcCard.y + 46, "Port: \\RPC_Control\\SurWinLpc  |  Messages Handled: 14,892", palette.textSecondary, 1);
    s.drawString(lpcCard.x + 16, lpcCard.y + 64, "Thread Dispatches: 38,401   |  Avg Dispatch Latency: 0.65 us", Color::fromHex(0x00FF9D), 1);

    curY += 98;

    // 3. Quick Launch Sovereign Tools Card
    const Rect toolsCard{startX, curY, r.width - 48, 92};
    s.drawRoundedRect(toolsCard, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(toolsCard, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::Registry, Point{toolsCard.x + 16, toolsCard.y + 16}, 24, palette.accentColor);
    s.drawString(toolsCard.x + 50, toolsCard.y + 18, "Sovereign Administrative & System Tools", palette.textPrimary, 1);

    btnLaunchTaskMgr_ = Rect{toolsCard.x + 16, toolsCard.y + 48, 140, 28};
    btnLaunchTerminal_ = Rect{toolsCard.x + 166, toolsCard.y + 48, 140, 28};
    btnLaunchRegEdit_ = Rect{toolsCard.x + 316, toolsCard.y + 48, 190, 28};

    drawPill(s, btnLaunchTaskMgr_, "Task Manager", false, palette);
    drawPill(s, btnLaunchTerminal_, "Command Terminal", false, palette);
    drawPill(s, btnLaunchRegEdit_, "Registry Editor", true, palette);
}

void SettingsContent::renderAboutPage(Surface& s, const ThemePalette& palette, Rect r) {
    const int32_t startX = r.x + 24;
    int32_t curY = r.y + 18;

    s.drawString(startX, curY, "About MicaNT Sovereign Workstation", palette.textPrimary, 1);
    curY += 28;

    // 1. Device Summary Banner
    const Rect bannerR{startX, curY, r.width - 48, 136};
    s.drawRoundedRect(bannerR, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(bannerR, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::StartPrism, Point{bannerR.x + 16, bannerR.y + 16}, 32, palette.accentColor);
    s.drawString(bannerR.x + 60, bannerR.y + 18, "MicaNT Sovereign Desktop Shell (SurShell)", palette.textPrimary, 1);
    s.drawString(bannerR.x + 60, bannerR.y + 36, "Clean-Room Sovereign Architecture | Release 2026.1", Color::fromHex(0x00D4FF), 1);

    s.drawString(bannerR.x + 16, bannerR.y + 68,  "Device Name:        " + deviceName_, palette.textPrimary, 1);
    s.drawString(bannerR.x + 16, bannerR.y + 88,  "Build Identifier:   10.0.26100.1-SOVEREIGN (x86_64)", palette.textSecondary, 1);
    s.drawString(bannerR.x + 16, bannerR.y + 108, "Programming Spec:   ISO C++23 Pure Native Implementation", palette.textSecondary, 1);

    btnRenamePc_ = Rect{bannerR.right() - 210, bannerR.y + 16, 95, 26};
    btnCopySpecs_ = Rect{bannerR.right() - 105, bannerR.y + 16, 95, 26};
    drawPill(s, btnRenamePc_, "Rename PC", false, palette);
    drawPill(s, btnCopySpecs_, "Copy Specs", false, palette);

    curY += 148;

    // 2. Provenance Certificate Card
    const Rect certR{startX, curY, r.width - 48, 100};
    s.drawRoundedRect(certR, 8, Color::fromHex(0x161F2E), true);
    s.drawRoundedRect(certR, 8, Color::fromHex(0x283850), false);

    IconRenderer::draw(s, IconId::ShieldAdmin, Point{certR.x + 16, certR.y + 16}, 24, Color::fromHex(0x00FF9D));
    s.drawString(certR.x + 50, certR.y + 18, "Clean-Room Legal Provenance & Architecture Tribute", palette.textPrimary, 1);
    s.drawString(certR.x + 16, certR.y + 48, "Engineered independently with zero proprietary binaries or copyright code.", palette.textSecondary, 1);
    s.drawString(certR.x + 16, certR.y + 68, "Dedicated in honor of Dave Cutler: Architect of VMS and Windows NT.", Color::fromHex(0x00D4FF), 1);
}

bool SettingsContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    // 1. Search Box & Clear Button
    if (searchBoxBounds_.contains(localPt)) {
        searchFocused_ = true;
        if (!searchQuery_.empty() && searchClearBounds_.contains(localPt)) {
            searchQuery_.clear();
            executeSearchFilter();
        }
        return true;
    } else {
        searchFocused_ = false;
    }

    // 2. Sidebar category selection
    for (size_t i = 0; i < categoryBounds_.size(); ++i) {
        if (categoryBounds_[i].contains(localPt)) {
            activeCategory_ = static_cast<SettingsCategory>(i);
            scrollY_ = 0;
            return true;
        }
    }

    // 3. Category Page Interactive Hit Testing
    switch (activeCategory_) {
        case SettingsCategory::System: {
            if (btnScale100_.contains(localPt)) {
                displayScaling_ = DisplayScaling::Scale100;
                if (onToast_) onToast_("Display Scaling", "Display scaling set to 100%", IconId::Display);
                return true;
            }
            if (btnScale125_.contains(localPt)) {
                displayScaling_ = DisplayScaling::Scale125;
                if (onToast_) onToast_("Display Scaling", "Display scaling set to 125%", IconId::Display);
                return true;
            }
            if (btnRefresh120_.contains(localPt)) {
                refreshRate120Hz_ = !refreshRate120Hz_;
                if (onToast_) onToast_("Refresh Rate", refreshRate120Hz_ ? "120Hz VSync Enabled" : "60Hz Standard Enabled", IconId::Display);
                return true;
            }
            if (btnHdrToggle_.contains(localPt)) {
                hdrEnabled_ = !hdrEnabled_;
                if (onToast_) onToast_("HDR Tone Mapping", hdrEnabled_ ? "HDR Display Active" : "HDR Display Inactive", IconId::Display);
                return true;
            }
            if (volumeSliderTrackR_.contains(localPt)) {
                const int32_t clickX = localPt.x - volumeSliderTrackR_.x;
                masterVolume_ = std::clamp(clickX * 100 / volumeSliderTrackR_.width, 0, 100);
                if (onVolume_) onVolume_(masterVolume_, isMuted_);
                return true;
            }
            if (btnVolumeMute_.contains(localPt)) {
                isMuted_ = !isMuted_;
                if (onVolume_) onVolume_(masterVolume_, isMuted_);
                return true;
            }
            if (btnSpatialAudio_.contains(localPt)) {
                spatialAudio_ = !spatialAudio_;
                if (onToast_) onToast_("Spatial Audio", spatialAudio_ ? "Spatial Audio 7.1 Enabled" : "Spatial Audio Disabled", IconId::VolumeHigh);
                return true;
            }
            if (btnPowerPerf_.contains(localPt)) {
                powerMode_ = PowerMode::BestPerformance;
                if (onToast_) onToast_("Power Management", "Best Performance Profile Active", IconId::BatteryCharging);
                return true;
            }
            if (btnPowerBal_.contains(localPt)) {
                powerMode_ = PowerMode::Balanced;
                if (onToast_) onToast_("Power Management", "Balanced Energy Profile Active", IconId::BatteryCharging);
                return true;
            }
            if (btnPowerSave_.contains(localPt)) {
                powerMode_ = PowerMode::PowerSaver;
                if (onToast_) onToast_("Power Management", "Power Saver Profile Active", IconId::BatteryCharging);
                return true;
            }
            if (btnStorageSense_.contains(localPt)) {
                storageSense_ = !storageSense_;
                if (onToast_) onToast_("Storage Sense", storageSense_ ? "Storage Sense: Active" : "Storage Sense: Inactive", IconId::DriveStorage);
                return true;
            }
            break;
        }

        case SettingsCategory::Personalization: {
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

            if (btnTransparency_.contains(localPt)) {
                transparencyEffects_ = !transparencyEffects_;
                if (onToast_) onToast_("Visual Effects", transparencyEffects_ ? "Mica Translucency Enabled" : "Opaque Rendering Enabled", IconId::Personalization);
                return true;
            }
            break;
        }

        case SettingsCategory::TaskbarDock: {
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
            if (btnAutoHide_.contains(localPt)) {
                taskbarAutoHide_ = !taskbarAutoHide_;
                if (onToast_) onToast_("Taskbar", taskbarAutoHide_ ? "Auto-Hide Enabled" : "Auto-Hide Disabled", IconId::TaskView);
                return true;
            }
            if (btnBadges_.contains(localPt)) {
                taskbarBadges_ = !taskbarBadges_;
                if (onToast_) onToast_("Taskbar", taskbarBadges_ ? "App Badges Enabled" : "App Badges Disabled", IconId::TaskView);
                return true;
            }
            break;
        }

        case SettingsCategory::Network: {
            if (btnWifiToggle_.contains(localPt)) {
                wifiEnabled_ = !wifiEnabled_;
                if (onToast_) onToast_("Network", wifiEnabled_ ? "Wi-Fi Enabled" : "Wi-Fi Disabled", IconId::NetworkOnline);
                return true;
            }
            if (btnDnsCloudflare_.contains(localPt)) {
                dnsResolver_ = DnsResolver::CloudflareSovereign;
                if (onToast_) onToast_("Sovereign DNS", "DNS set to Cloudflare (1.1.1.1)", IconId::SentinelSec);
                return true;
            }
            if (btnDnsQuad9_.contains(localPt)) {
                dnsResolver_ = DnsResolver::Quad9;
                if (onToast_) onToast_("Sovereign DNS", "DNS set to Quad9 (9.9.9.9)", IconId::SentinelSec);
                return true;
            }
            if (btnDnsDhcp_.contains(localPt)) {
                dnsResolver_ = DnsResolver::LocalGateway;
                if (onToast_) onToast_("Sovereign DNS", "DNS set to Local DHCP Gateway", IconId::SentinelSec);
                return true;
            }
            if (btnFirewall_.contains(localPt)) {
                stealthFirewall_ = !stealthFirewall_;
                if (onToast_) onToast_("Sovereign Firewall", stealthFirewall_ ? "Stealth Mode Active" : "Stealth Mode Inactive", IconId::SentinelSec);
                return true;
            }
            break;
        }

        case SettingsCategory::Apps: {
            if (btnTermCmd_.contains(localPt)) {
                defaultTerminal_ = DefaultTerminal::Cmd;
                if (onToast_) onToast_("Default Terminal", "Default shell set to Command Prompt (cmd)", IconId::Terminal);
                return true;
            }
            if (btnTermPwsh_.contains(localPt)) {
                defaultTerminal_ = DefaultTerminal::Pwsh;
                if (onToast_) onToast_("Default Terminal", "Default shell set to PowerShell (pwsh)", IconId::Terminal);
                return true;
            }
            if (btnTermSurShell_.contains(localPt)) {
                defaultTerminal_ = DefaultTerminal::SurShell;
                if (onToast_) onToast_("Default Terminal", "Default shell set to SurShell Native", IconId::Terminal);
                return true;
            }
            if (btnStartupSentinel_.contains(localPt)) {
                startupSentinel_ = !startupSentinel_;
                return true;
            }
            if (btnStartupCompositor_.contains(localPt)) {
                startupCompositor_ = !startupCompositor_;
                return true;
            }
            if (btnStartupLpc_.contains(localPt)) {
                startupLpc_ = !startupLpc_;
                return true;
            }
            break;
        }

        case SettingsCategory::PrivacySecurity: {
            if (btnCoreIsolation_.contains(localPt)) {
                coreIsolationHvci_ = !coreIsolationHvci_;
                if (onToast_) onToast_("SentinelSec", coreIsolationHvci_ ? "HVCI Core Isolation Active" : "HVCI Disabled", IconId::SentinelSec);
                return true;
            }
            if (btnDmaProtect_.contains(localPt)) {
                kernelDmaProtect_ = !kernelDmaProtect_;
                if (onToast_) onToast_("Kernel Security", kernelDmaProtect_ ? "Kernel DMA Protection Active" : "Kernel DMA Disabled", IconId::SentinelSec);
                return true;
            }
            if (btnCameraEnclave_.contains(localPt)) {
                permCameraPrompt_ = !permCameraPrompt_;
                return true;
            }
            if (btnMicEnclave_.contains(localPt)) {
                permMicPrompt_ = !permMicPrompt_;
                return true;
            }
            break;
        }

        case SettingsCategory::TimeLanguage: {
            if (btnClock24H_.contains(localPt)) {
                clockFormat24H_ = !clockFormat24H_;
                if (onTimeFormat_) onTimeFormat_(clockFormat24H_);
                return true;
            }
            if (btnNtpSync_.contains(localPt)) {
                ntpAutoSync_ = !ntpAutoSync_;
                return true;
            }
            if (btnSyncNow_.contains(localPt)) {
                if (onToast_) onToast_("Time Synchronization", "Clock synchronized with time.cloudflare.com", IconId::Clock);
                return true;
            }
            if (btnTzPacific_.contains(localPt)) {
                timeZone_ = "UTC-07:00 Pacific Time (US & Canada)";
                if (onToast_) onToast_("Time Zone", "Time zone set to Pacific (UTC-7)", IconId::Clock);
                return true;
            }
            if (btnTzEastern_.contains(localPt)) {
                timeZone_ = "UTC-04:00 Eastern Time (US & Canada)";
                if (onToast_) onToast_("Time Zone", "Time zone set to Eastern (UTC-4)", IconId::Clock);
                return true;
            }
            if (btnTzUtc_.contains(localPt)) {
                timeZone_ = "UTC+00:00 Coordinated Universal Time";
                if (onToast_) onToast_("Time Zone", "Time zone set to UTC+0", IconId::Clock);
                return true;
            }
            if (btnTzCet_.contains(localPt)) {
                timeZone_ = "UTC+01:00 Central European Time";
                if (onToast_) onToast_("Time Zone", "Time zone set to CET (UTC+1)", IconId::Clock);
                return true;
            }
            break;
        }

        case SettingsCategory::Developer: {
            if (btnDevMode_.contains(localPt)) {
                developerMode_ = !developerMode_;
                if (onToast_) onToast_("Developer Mode", developerMode_ ? "Developer Mode Enabled" : "Developer Mode Disabled", IconId::ShieldAdmin);
                return true;
            }
            if (btnLaunchTaskMgr_.contains(localPt)) {
                if (onLaunchApp_) onLaunchApp_("taskmgr");
                return true;
            }
            if (btnLaunchTerminal_.contains(localPt)) {
                if (onLaunchApp_) onLaunchApp_("cmd");
                return true;
            }
            if (btnLaunchRegEdit_.contains(localPt)) {
                if (onLaunchApp_) onLaunchApp_("regedit");
                return true;
            }
            break;
        }

        case SettingsCategory::About: {
            if (btnRenamePc_.contains(localPt)) {
                if (onToast_) onToast_("System Settings", "Renaming PC requires executive reboot.", IconId::ThisPC);
                return true;
            }
            if (btnCopySpecs_.contains(localPt)) {
                if (onToast_) onToast_("Clipboard", "MicaNT Workstation specifications copied.", IconId::StartPrism);
                return true;
            }
            break;
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

bool SettingsContent::onMouseWheel(Point localPt, int32_t delta) {
    (void)localPt;
    (void)delta;
    return true;
}

bool SettingsContent::onCharInput(char c) {
    if (!searchFocused_) return false;
    if (c >= 32 && c <= 126) {
        searchQuery_.push_back(c);
        executeSearchFilter();
        return true;
    }
    return false;
}

bool SettingsContent::onKeyDown(KeyCode key, bool ctrl, bool shift, bool alt) {
    (void)ctrl; (void)shift; (void)alt;
    if (!searchFocused_) return false;

    if (key == KeyCode::Backspace) {
        if (!searchQuery_.empty()) {
            searchQuery_.pop_back();
            executeSearchFilter();
            return true;
        }
    } else if (key == KeyCode::Escape) {
        searchQuery_.clear();
        searchFocused_ = false;
        executeSearchFilter();
        return true;
    }
    return false;
}

} // namespace surshell
