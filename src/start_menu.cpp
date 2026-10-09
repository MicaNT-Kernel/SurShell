// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/start_menu.cpp)
// ============================================================================

#include "surshell/start_menu.hpp"
#include "surshell/theme.hpp"
#include "surshell/icons.hpp"
#include <algorithm>
#include <filesystem>

namespace surshell {

StartMenu::StartMenu() {
    // Populate default sovereign applications for MicaNT
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
        .id = "network",
        .title = "Network Connections",
        .subtitle = "Ethernet & Wi-Fi Control",
        .executablePath = "C:\\Windows\\System32\\ncpa.cpl",
        .arguments = "",
        .iconGlyph = "[NET]",
        .category = AppCategory::SystemTools,
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

    registerApp(ShellAppEntry{
        .id = "notepad",
        .title = "Sovereign Editor",
        .subtitle = "UTF-8 Source & Text Editor",
        .executablePath = "C:\\Windows\\notepad.exe",
        .arguments = "",
        .iconGlyph = "[T]",
        .category = AppCategory::Development,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "calc",
        .title = "Calculator",
        .subtitle = "Scientific & Standard Math",
        .executablePath = "C:\\Windows\\System32\\calc.exe",
        .arguments = "",
        .iconGlyph = "[CALC]",
        .category = AppCategory::Utilities,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "run",
        .title = "Run...",
        .subtitle = "Open Program or Resource",
        .executablePath = "C:\\Windows\\System32\\run.exe",
        .arguments = "",
        .iconGlyph = "[RUN]",
        .category = AppCategory::SystemTools,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "regedit",
        .title = "Registry Editor",
        .subtitle = "MicaNT Sovereign Configuration Hive",
        .executablePath = "C:\\Windows\\regedit.exe",
        .arguments = "",
        .iconGlyph = "[REG]",
        .category = AppCategory::SystemTools,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "photos",
        .title = "Photos",
        .subtitle = "Sovereign Image & Photo Viewer",
        .executablePath = "C:\\Windows\\System32\\photos.exe",
        .arguments = "",
        .iconGlyph = "[P]",
        .category = AppCategory::Multimedia,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "paint",
        .title = "Paint",
        .subtitle = "Sovereign Vector & Pixel Canvas Studio",
        .executablePath = "C:\\Windows\\System32\\mspaint.exe",
        .arguments = "",
        .iconGlyph = "[PNT]",
        .category = AppCategory::Accessories,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "sysinfo",
        .title = "System Information",
        .subtitle = "Hardware Topology & Diagnostics",
        .executablePath = "C:\\Windows\\System32\\msinfo32.exe",
        .arguments = "",
        .iconGlyph = "[SYS]",
        .category = AppCategory::SystemTools,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    registerApp(ShellAppEntry{
        .id = "devmgmt",
        .title = "Device Manager",
        .subtitle = "Hardware Tree & Device Drivers",
        .executablePath = "C:\\Windows\\System32\\devmgmt.msc",
        .arguments = "",
        .iconGlyph = "[DEV]",
        .category = AppCategory::SystemTools,
        .pinnedToTaskbar = false,
        .pinnedToStart = true
    });

    // Windows 11-style Interactive Power Flyout Options
    powerOptions_ = {
        PowerOptionItem{.action = PowerAction::Sleep, .label = "Sleep", .description = "Save session in low power", .iconId = IconId::Sleep},
        PowerOptionItem{.action = PowerAction::Hibernate, .label = "Hibernate", .description = "Save session to disk", .iconId = IconId::Hibernate},
        PowerOptionItem{.action = PowerAction::Restart, .label = "Restart", .description = "Restart sovereign kernel", .iconId = IconId::Restart},
        PowerOptionItem{.action = PowerAction::ShutDown, .label = "Shut down", .description = "Turn off workstation", .iconId = IconId::Power},
        PowerOptionItem{.action = PowerAction::Lock, .label = "Lock", .description = "Lock workstation session", .iconId = IconId::Lock},
        PowerOptionItem{.action = PowerAction::SignOut, .label = "Sign out", .description = "Close apps and sign out", .iconId = IconId::SignOut}
    };

    // Windows 11-style Recommended Recent Activities
    recommendedItems_ = {
        RecommendedItem{.id = "rec1", .title = "explorer.hpp", .subtitle = "Recent C++ Header - Just now", .path = "C:\\source\\SurShell\\include\\surshell\\explorer.hpp", .iconId = IconId::FileCode},
        RecommendedItem{.id = "rec2", .title = "surshell.cpp", .subtitle = "Recent C++ Source - 10m ago", .path = "C:\\source\\SurShell\\src\\surshell.cpp", .iconId = IconId::FileCode},
        RecommendedItem{.id = "rec3", .title = "System32", .subtitle = "Frequently visited folder", .path = "C:\\Windows\\System32", .iconId = IconId::Folder},
        RecommendedItem{.id = "rec4", .title = "sentinel.exe", .subtitle = "Security daemon - 2h ago", .path = "C:\\Program Files\\Sentinel\\sentinel.exe", .iconId = IconId::SentinelSec}
    };

    discoverHostApplications();
    refreshFilter();
}

void StartMenu::discoverHostApplications() {
    std::string userProfile = "C:\\Users\\admin";
    if (const char* envProf = std::getenv("USERPROFILE"); envProf && envProf[0] != '\0') {
        userProfile = envProf;
    }

    std::vector<std::string> searchPaths = {
        "C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs",
        userProfile + "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs",
        userProfile + "\\AppData\\Local\\Programs"
    };

    std::error_code ec;
    size_t addedCount = 0;

    for (const auto& rootPath : searchPaths) {
        if (!std::filesystem::exists(rootPath, ec)) continue;

        for (std::filesystem::recursive_directory_iterator it(rootPath, std::filesystem::directory_options::skip_permission_denied, ec), end;
             it != end; it.increment(ec)) {
            if (ec) {
                ec.clear();
                continue;
            }
            const auto& entry = *it;
            if (!entry.is_regular_file(ec)) continue;

            const auto ext = entry.path().extension().string();
            if (ext != ".lnk" && ext != ".exe") continue;

            std::string stem = entry.path().stem().string();
            if (stem.empty()) continue;

            std::string lowerStem = stem;
            std::transform(lowerStem.begin(), lowerStem.end(), lowerStem.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lowerStem.find("uninstall") != std::string::npos ||
                lowerStem.find("help") != std::string::npos ||
                lowerStem.find("readme") != std::string::npos ||
                lowerStem.find("documentation") != std::string::npos ||
                lowerStem.find("website") != std::string::npos ||
                lowerStem.find("release notes") != std::string::npos ||
                lowerStem.find("command prompt") != std::string::npos ||
                lowerStem == "git cmd" ||
                lowerStem == "cmd" ||
                lowerStem == "file explorer" ||
                lowerStem == "task manager" ||
                lowerStem == "registry editor" ||
                lowerStem == "control panel") {
                continue;
            }

            bool exists = false;
            for (const auto& a : allApps_) {
                if (a.title == stem) {
                    exists = true;
                    break;
                }
            }
            if (exists) continue;

            AppCategory cat = AppCategory::Utilities;
            IconId icon = IconId::FileExecutable;

            if (lowerStem.find("studio") != std::string::npos || lowerStem.find("code") != std::string::npos ||
                lowerStem.find("git") != std::string::npos || lowerStem.find("python") != std::string::npos ||
                lowerStem.find("terminal") != std::string::npos || lowerStem.find("powershell") != std::string::npos) {
                cat = AppCategory::Development;
                icon = (lowerStem.find("terminal") != std::string::npos || lowerStem.find("powershell") != std::string::npos) ? IconId::Terminal : IconId::FileCode;
            } else if (lowerStem.find("chrome") != std::string::npos || lowerStem.find("edge") != std::string::npos ||
                       lowerStem.find("firefox") != std::string::npos || lowerStem.find("browser") != std::string::npos ||
                       lowerStem.find("netbird") != std::string::npos || lowerStem.find("discord") != std::string::npos) {
                cat = AppCategory::Utilities;
                icon = IconId::NetworkOnline;
            } else if (lowerStem.find("vlc") != std::string::npos || lowerStem.find("media") != std::string::npos ||
                       lowerStem.find("obs") != std::string::npos || lowerStem.find("audio") != std::string::npos ||
                       lowerStem.find("video") != std::string::npos || lowerStem.find("player") != std::string::npos ||
                       lowerStem.find("music") != std::string::npos || lowerStem.find("4k") != std::string::npos) {
                cat = AppCategory::Multimedia;
                icon = IconId::MediaPlay;
            } else if (lowerStem.find("word") != std::string::npos || lowerStem.find("excel") != std::string::npos ||
                       lowerStem.find("powerpoint") != std::string::npos || lowerStem.find("outlook") != std::string::npos ||
                       lowerStem.find("onenote") != std::string::npos || lowerStem.find("notepad") != std::string::npos ||
                       lowerStem.find("office") != std::string::npos) {
                cat = AppCategory::Accessories;
                icon = IconId::FileText;
            } else if (lowerStem.find("settings") != std::string::npos || lowerStem.find("control") != std::string::npos ||
                       lowerStem.find("taskmgr") != std::string::npos || lowerStem.find("disk") != std::string::npos ||
                       lowerStem.find("security") != std::string::npos) {
                cat = AppCategory::SystemTools;
                icon = IconId::Settings;
            } else if (lowerStem.find("calc") != std::string::npos) {
                cat = AppCategory::Utilities;
                icon = IconId::Calculator;
            }

            allApps_.push_back(ShellAppEntry{
                .id = "host_" + std::to_string(addedCount++),
                .title = stem,
                .subtitle = "Installed Application",
                .executablePath = entry.path().string(),
                .arguments = "",
                .iconGlyph = "[A]",
                .category = cat,
                .pinnedToTaskbar = false,
                .pinnedToStart = false
            });
        }
    }

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

void StartMenu::addRecommendedItem(RecommendedItem item) {
    recommendedItems_.push_back(std::move(item));
}

void StartMenu::open() noexcept {
    isOpen_ = true;
    viewMode_ = StartViewMode::Pinned;
    isPowerFlyoutOpen_ = false;
    isUserFlyoutOpen_ = false;
    searchQuery_.clear();
    refreshFilter();
}

void StartMenu::close() noexcept {
    isOpen_ = false;
    isPowerFlyoutOpen_ = false;
    isUserFlyoutOpen_ = false;
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

        std::string lowerExeName = lowerExe;
        const size_t slashPos = lowerExe.find_last_of("\\/");
        if (slashPos != std::string::npos) {
            lowerExeName = lowerExe.substr(slashPos + 1);
        }

        if (lowerTitle.find(lowerQuery) != std::string::npos ||
            lowerId.find(lowerQuery) != std::string::npos ||
            lowerExeName.find(lowerQuery) != std::string::npos) {
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

Rect StartMenu::powerButtonBounds(Rect menuBounds) const noexcept {
    const int32_t footerY = menuBounds.bottom() - 56;
    return Rect{menuBounds.right() - 50, footerY + 10, 34, 34};
}

Rect StartMenu::userButtonBounds(Rect menuBounds) const noexcept {
    const int32_t footerY = menuBounds.bottom() - 56;
    return Rect{menuBounds.x + 16, footerY + 8, 220, 38};
}

Rect StartMenu::allAppsButtonBounds(Rect menuBounds) const noexcept {
    return Rect{menuBounds.right() - 110, menuBounds.y + 74, 90, 24};
}

Rect StartMenu::powerFlyoutBounds(Rect menuBounds) const noexcept {
    const int32_t fw = 220;
    const int32_t itemH = 34;
    const int32_t fh = static_cast<int32_t>(powerOptions_.size()) * itemH + 24;
    return Rect{menuBounds.right() - fw - 16, menuBounds.bottom() - 60 - fh, fw, fh};
}

Rect StartMenu::userFlyoutBounds(Rect menuBounds) const noexcept {
    const int32_t fw = 210;
    const int32_t fh = 3 * 34 + 24;
    return Rect{menuBounds.x + 16, menuBounds.bottom() - 60 - fh, fw, fh};
}

void StartMenu::onMouseMove(Point pt, Rect menuBounds) {
    if (!isOpen_) {
        hoveredAppIndex_ = -1;
        hoveredRecommendedIndex_ = -1;
        hoveredPowerFlyoutIndex_ = -1;
        hoveredUserFlyoutIndex_ = -1;
        isAllAppsButtonHovered_ = false;
        isPowerButtonHovered_ = false;
        isUserButtonHovered_ = false;
        return;
    }

    // Check Power Flyout if open
    if (isPowerFlyoutOpen_) {
        const Rect pfb = powerFlyoutBounds(menuBounds);
        if (pfb.contains(pt)) {
            hoveredPowerFlyoutIndex_ = (pt.y - (pfb.y + 12)) / 34;
            if (hoveredPowerFlyoutIndex_ < 0 || hoveredPowerFlyoutIndex_ >= static_cast<int32_t>(powerOptions_.size())) {
                hoveredPowerFlyoutIndex_ = -1;
            }
            return;
        }
        hoveredPowerFlyoutIndex_ = -1;
    }

    // Check User Flyout if open
    if (isUserFlyoutOpen_) {
        const Rect ufb = userFlyoutBounds(menuBounds);
        if (ufb.contains(pt)) {
            hoveredUserFlyoutIndex_ = (pt.y - (ufb.y + 12)) / 34;
            if (hoveredUserFlyoutIndex_ < 0 || hoveredUserFlyoutIndex_ >= 3) {
                hoveredUserFlyoutIndex_ = -1;
            }
            return;
        }
        hoveredUserFlyoutIndex_ = -1;
    }

    if (!menuBounds.contains(pt)) {
        hoveredAppIndex_ = -1;
        hoveredRecommendedIndex_ = -1;
        isAllAppsButtonHovered_ = false;
        isPowerButtonHovered_ = false;
        isUserButtonHovered_ = false;
        return;
    }

    // Footer button hit tests
    isPowerButtonHovered_ = powerButtonBounds(menuBounds).contains(pt);
    isUserButtonHovered_ = userButtonBounds(menuBounds).contains(pt);
    isAllAppsButtonHovered_ = allAppsButtonBounds(menuBounds).contains(pt);

    hoveredAppIndex_ = -1;
    hoveredRecommendedIndex_ = -1;

    if (viewMode_ == StartViewMode::Pinned) {
        // Pinned 2-column app cards
        const int32_t gridStartX = menuBounds.x + 20;
        const int32_t gridStartY = menuBounds.y + 106;
        const int32_t cardW = (menuBounds.width - 50) / 2;
        const int32_t cardH = 50;
        const int32_t cardGap = 8;
        const size_t maxDisplay = std::min(filteredApps_.size(), static_cast<size_t>(6));

        for (size_t i = 0; i < maxDisplay; ++i) {
            const int32_t col = static_cast<int32_t>(i % 2);
            const int32_t row = static_cast<int32_t>(i / 2);
            Rect cardRect{gridStartX + col * (cardW + cardGap), gridStartY + row * (cardH + cardGap), cardW, cardH};
            if (cardRect.contains(pt)) {
                hoveredAppIndex_ = static_cast<int32_t>(i);
                break;
            }
        }

        // Recommended items (2 columns x 2 rows)
        const int32_t recStartY = gridStartY + 3 * (cardH + cardGap) + 38;
        const int32_t recH = 46;
        const size_t maxRec = std::min(recommendedItems_.size(), static_cast<size_t>(4));
        for (size_t i = 0; i < maxRec; ++i) {
            const int32_t col = static_cast<int32_t>(i % 2);
            const int32_t row = static_cast<int32_t>(i / 2);
            Rect recRect{gridStartX + col * (cardW + cardGap), recStartY + row * (recH + cardGap), cardW, recH};
            if (recRect.contains(pt)) {
                hoveredRecommendedIndex_ = static_cast<int32_t>(i);
                break;
            }
        }
    } else {
        // All Apps vertical list
        const int32_t listStartX = menuBounds.x + 20;
        const int32_t listStartY = menuBounds.y + 110;
        const int32_t rowW = menuBounds.width - 40;
        const int32_t rowH = 40;
        const int32_t rowGap = 4;
        const size_t maxDisplay = std::min(filteredApps_.size(), static_cast<size_t>(8));

        for (size_t i = 0; i < maxDisplay; ++i) {
            Rect rowRect{listStartX, listStartY + static_cast<int32_t>(i) * (rowH + rowGap), rowW, rowH};
            if (rowRect.contains(pt)) {
                hoveredAppIndex_ = static_cast<int32_t>(i);
                break;
            }
        }
    }
}

void StartMenu::onMouseDown(Point pt, MouseButton button, Rect menuBounds) {
    if (!isOpen_ || button != MouseButton::Left) return;

    // 1. Power Flyout Click Handling
    if (isPowerFlyoutOpen_) {
        const Rect pfb = powerFlyoutBounds(menuBounds);
        if (pfb.contains(pt)) {
            if (hoveredPowerFlyoutIndex_ >= 0 && hoveredPowerFlyoutIndex_ < static_cast<int32_t>(powerOptions_.size())) {
                const auto selectedAction = powerOptions_[static_cast<size_t>(hoveredPowerFlyoutIndex_)].action;
                if (powerCallback_) {
                    powerCallback_(selectedAction);
                }
                close();
                return;
            }
        }
        setPowerFlyoutOpen(false);
        return;
    }

    // 2. User Flyout Click Handling
    if (isUserFlyoutOpen_) {
        const Rect ufb = userFlyoutBounds(menuBounds);
        if (ufb.contains(pt)) {
            if (hoveredUserFlyoutIndex_ == 0) {
                // Account Settings
                for (const auto& a : allApps_) {
                    if (a.id == "settings") {
                        if (launchCallback_) launchCallback_(a);
                        break;
                    }
                }
            } else if (hoveredUserFlyoutIndex_ == 1) {
                // Lock
                if (powerCallback_) powerCallback_(PowerAction::Lock);
            } else if (hoveredUserFlyoutIndex_ == 2) {
                // Sign Out
                if (powerCallback_) powerCallback_(PowerAction::SignOut);
            }
            close();
            return;
        }
        setUserFlyoutOpen(false);
        return;
    }

    if (!menuBounds.contains(pt)) {
        close();
        return;
    }

    // 3. Power Button Click -> Toggle Power Flyout
    if (powerButtonBounds(menuBounds).contains(pt)) {
        togglePowerFlyout();
        return;
    }

    // 4. User Profile Button Click -> Toggle User Flyout
    if (userButtonBounds(menuBounds).contains(pt)) {
        toggleUserFlyout();
        return;
    }

    // 5. All Apps / Back Button Click
    if (allAppsButtonBounds(menuBounds).contains(pt)) {
        toggleViewMode();
        return;
    }

    // 6. View Mode specific clicks
    if (viewMode_ == StartViewMode::Pinned) {
        // App Card Click
        if (hoveredAppIndex_ >= 0 && hoveredAppIndex_ < static_cast<int32_t>(filteredApps_.size())) {
            if (launchCallback_) {
                launchCallback_(filteredApps_[static_cast<size_t>(hoveredAppIndex_)]);
            }
            close();
            return;
        }

        // Recommended Item Click
        if (hoveredRecommendedIndex_ >= 0 && hoveredRecommendedIndex_ < static_cast<int32_t>(recommendedItems_.size())) {
            const auto& rec = recommendedItems_[static_cast<size_t>(hoveredRecommendedIndex_)];
            if (openItemCallback_) {
                openItemCallback_(rec.path);
            } else {
                // Fallback to launching matching app or file
                for (const auto& a : allApps_) {
                    if (a.executablePath == rec.path) {
                        if (launchCallback_) launchCallback_(a);
                        break;
                    }
                }
            }
            close();
            return;
        }
    } else {
        // All Apps List Row Click
        if (hoveredAppIndex_ >= 0 && hoveredAppIndex_ < static_cast<int32_t>(filteredApps_.size())) {
            if (launchCallback_) {
                launchCallback_(filteredApps_[static_cast<size_t>(hoveredAppIndex_)]);
            }
            close();
            return;
        }
    }
}

void StartMenu::render(Surface& surface, Rect menuBounds) {
    if (!isOpen_) return;

    const auto& palette = ThemeManager::instance().palette();

    // 1. Soft deep drop shadow behind detached Start card
    surface.drawDropShadow(menuBounds, 18, 0.55f);

    // 2. Modern 14px rounded container with translucent Mica Acrylic blur
    surface.applyAcrylicTint(menuBounds, palette.startMenuBg, 10);
    surface.drawRoundedRect(menuBounds, 14, palette.startMenuBorder, false);

    // 3. Modern Search Pill at Top
    Rect searchBox{menuBounds.x + 20, menuBounds.y + 16, menuBounds.width - 40, 38};
    surface.drawRoundedRect(searchBox, 10, palette.startMenuSearchBg, true);
    surface.drawRoundedRect(searchBox, 10, palette.startMenuSearchBorder, false);

    // Procedural Search Icon
    IconRenderer::draw(surface, IconId::Search, Rect{searchBox.x + 12, searchBox.y + 11, 16, 16}, palette.accentColor);

    if (searchQuery_.empty()) {
        surface.drawString(searchBox.x + 36, searchBox.y + 14, "Search apps, settings, documents...", palette.textDisabled, 1);
    } else {
        std::string displayQuery = searchQuery_ + "|";
        surface.drawString(searchBox.x + 36, searchBox.y + 14, displayQuery, palette.textPrimary, 1);
    }

    // 4. Render Main Content Area
    if (viewMode_ == StartViewMode::Pinned) {
        renderPinnedView(surface, menuBounds);
    } else {
        renderAllAppsView(surface, menuBounds);
    }

    // 5. Bottom User Profile & Windows 11 Power Controls Footer
    const int32_t footerY = menuBounds.bottom() - 56;
    surface.fillRect(Rect{menuBounds.x, footerY, menuBounds.width, 1}, palette.startMenuBorder);

    // User Profile Pill Button
    const Rect userBtn = userButtonBounds(menuBounds);
    if (isUserButtonHovered_ || isUserFlyoutOpen_) {
        surface.drawRoundedRect(userBtn, 6, Color::fromRgba(255, 255, 255, 20), true);
    }

    Rect avatarBox{userBtn.x + 4, userBtn.y + 3, 32, 32};
    surface.drawRoundedRect(avatarBox, 16, palette.accentColor, true);
    IconRenderer::draw(surface, IconId::User, Rect{avatarBox.x + 6, avatarBox.y + 6, 20, 20}, Color::fromHex(0x06090F));

    surface.drawString(avatarBox.right() + 10, userBtn.y + 6, "ssfdre38", palette.textPrimary, 1);
    surface.drawString(avatarBox.right() + 10, userBtn.y + 20, "MicaNT Administrator", palette.textSecondary, 1);

    // Dedicated Power Button with Universal IEC 5009 Standby Vector Icon
    const Rect powerBtn = powerButtonBounds(menuBounds);
    const Color powerBg = (isPowerButtonHovered_ || isPowerFlyoutOpen_)
        ? Color::fromRgba(255, 69, 58, 60)
        : Color::fromRgba(35, 48, 72, 140);
    const Color powerBorder = (isPowerButtonHovered_ || isPowerFlyoutOpen_)
        ? Color::fromHex(0xFF453A)
        : palette.startCardBorder;

    surface.drawRoundedRect(powerBtn, 17, powerBg, true);
    surface.drawRoundedRect(powerBtn, 17, powerBorder, false);

    IconRenderer::draw(surface, IconId::Power, Rect{powerBtn.x + 8, powerBtn.y + 8, 18, 18},
                       (isPowerButtonHovered_ || isPowerFlyoutOpen_) ? Color::fromHex(0xFF6B60) : palette.textPrimary);

    // 6. Floating Flyouts (rendered on top of Start Menu)
    if (isPowerFlyoutOpen_) {
        renderPowerFlyout(surface, powerFlyoutBounds(menuBounds));
    }
    if (isUserFlyoutOpen_) {
        renderUserFlyout(surface, userFlyoutBounds(menuBounds));
    }
}

void StartMenu::renderPinnedView(Surface& surface, Rect menuBounds) {
    const auto& palette = ThemeManager::instance().palette();

    // Header: Pinned Title & "All apps >" Button
    surface.drawString(menuBounds.x + 24, menuBounds.y + 76, "PINNED", palette.accentColor, 1);

    const Rect allAppsBtn = allAppsButtonBounds(menuBounds);
    if (isAllAppsButtonHovered_) {
        surface.drawRoundedRect(allAppsBtn, 4, Color::fromRgba(255, 255, 255, 20), true);
    }
    surface.drawString(allAppsBtn.x + 6, allAppsBtn.y + 6, "All apps", palette.textSecondary, 1);
    IconRenderer::draw(surface, IconId::NavForward, Rect{allAppsBtn.right() - 16, allAppsBtn.y + 5, 14, 14}, palette.textSecondary);

    // Pinned 2-Column Application Card Grid
    const int32_t gridStartX = menuBounds.x + 20;
    const int32_t gridStartY = menuBounds.y + 104;
    const int32_t cardW = (menuBounds.width - 50) / 2;
    const int32_t cardH = 50;
    const int32_t cardGap = 8;
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

        // Procedural Vector Icon tile (30x30)
        Rect iconTile{cardRect.x + 8, cardRect.y + 10, 30, 30};
        surface.drawRoundedRect(iconTile, 6, Color::fromRgba(25, 36, 56, 200), true);
        surface.drawRoundedRect(iconTile, 6, Color::fromRgba(48, 68, 104, 140), false);
        const Rect innerIcon{iconTile.x + 3, iconTile.y + 3, 24, 24};
        IconRenderer::draw(surface, IconRenderer::iconForAppId(app.id), innerIcon);

        // App Title
        surface.drawString(cardRect.x + 46, cardRect.y + 12, app.title, palette.textPrimary, 1);

        // App Subtitle / Description
        std::string dispSub = app.subtitle.empty() ? app.executablePath : app.subtitle;
        if (dispSub.size() > 22) dispSub = dispSub.substr(0, 20) + "..";
        surface.drawString(cardRect.x + 46, cardRect.y + 28, dispSub, palette.textSecondary, 1);
    }

    // Recommended Section
    const int32_t recHeaderY = gridStartY + 3 * (cardH + cardGap) + 14;
    surface.drawString(menuBounds.x + 24, recHeaderY, "RECOMMENDED", palette.textSecondary, 1);
    surface.drawString(menuBounds.right() - 80, recHeaderY, "More >", palette.accentSecondary, 1);
    surface.fillRect(Rect{menuBounds.x + 24, recHeaderY + 16, menuBounds.width - 48, 1}, palette.startCardBorder);

    // Recommended items (2 columns x 2 rows)
    const int32_t recStartY = recHeaderY + 24;
    const int32_t recH = 46;
    const size_t maxRec = std::min(recommendedItems_.size(), static_cast<size_t>(4));

    for (size_t i = 0; i < maxRec; ++i) {
        const auto& rec = recommendedItems_[i];
        const int32_t col = static_cast<int32_t>(i % 2);
        const int32_t row = static_cast<int32_t>(i / 2);
        Rect recRect{gridStartX + col * (cardW + cardGap), recStartY + row * (recH + cardGap), cardW, recH};

        const bool isHovered = (static_cast<int32_t>(i) == hoveredRecommendedIndex_);
        Color recBg = isHovered ? palette.startCardHover : Color::fromRgba(20, 28, 44, 160);
        Color borderCol = isHovered ? palette.accentColor : palette.startCardBorder;

        surface.drawRoundedRect(recRect, 6, recBg, true);
        surface.drawRoundedRect(recRect, 6, borderCol, false);

        // Vector Icon for recommended item
        IconRenderer::draw(surface, rec.iconId, Rect{recRect.x + 8, recRect.y + 11, 24, 24});

        // Title and description
        surface.drawString(recRect.x + 40, recRect.y + 10, rec.title, palette.textPrimary, 1);
        std::string dispSub = rec.subtitle;
        if (dispSub.size() > 20) dispSub = dispSub.substr(0, 18) + "..";
        surface.drawString(recRect.x + 40, recRect.y + 26, dispSub, palette.textSecondary, 1);
    }
}

void StartMenu::renderAllAppsView(Surface& surface, Rect menuBounds) {
    const auto& palette = ThemeManager::instance().palette();

    // Header: Back Button & "ALL APPLICATIONS"
    const Rect backBtn = allAppsButtonBounds(menuBounds);
    if (isAllAppsButtonHovered_) {
        surface.drawRoundedRect(backBtn, 4, Color::fromRgba(255, 255, 255, 20), true);
    }
    IconRenderer::draw(surface, IconId::NavBack, Rect{backBtn.x + 8, backBtn.y + 5, 14, 14}, palette.accentColor);
    surface.drawString(backBtn.x + 26, backBtn.y + 6, "Back", palette.accentColor, 1);

    surface.drawString(menuBounds.x + 24, menuBounds.y + 76, "ALL APPLICATIONS (A-Z)", palette.accentColor, 1);
    surface.fillRect(Rect{menuBounds.x + 24, menuBounds.y + 96, menuBounds.width - 48, 1}, palette.startCardBorder);

    // Vertical List of Applications
    const int32_t listStartX = menuBounds.x + 20;
    const int32_t listStartY = menuBounds.y + 106;
    const int32_t rowW = menuBounds.width - 40;
    const int32_t rowH = 42;
    const int32_t rowGap = 4;
    const size_t maxDisplay = std::min(filteredApps_.size(), static_cast<size_t>(8));

    for (size_t i = 0; i < maxDisplay; ++i) {
        const auto& app = filteredApps_[i];
        Rect rowRect{listStartX, listStartY + static_cast<int32_t>(i) * (rowH + rowGap), rowW, rowH};

        const bool isHovered = (static_cast<int32_t>(i) == hoveredAppIndex_);
        Color rowBg = isHovered ? palette.startCardHover : palette.startCardBg;
        Color borderCol = isHovered ? palette.accentColor : palette.startCardBorder;

        surface.drawRoundedRect(rowRect, 6, rowBg, true);
        surface.drawRoundedRect(rowRect, 6, borderCol, false);

        // Vector Icon
        IconRenderer::draw(surface, IconRenderer::iconForAppId(app.id), Rect{rowRect.x + 10, rowRect.y + 9, 24, 24});

        // App Title and Subtitle
        surface.drawString(rowRect.x + 44, rowRect.y + 8, app.title, palette.textPrimary, 1);
        surface.drawString(rowRect.x + 44, rowRect.y + 24, app.subtitle, palette.textSecondary, 1);

        // Category Tag on far right
        const char* catStr = "Tool";
        if (app.category == AppCategory::SystemTools) catStr = "System";
        else if (app.category == AppCategory::Development) catStr = "Dev";
        else if (app.category == AppCategory::Multimedia) catStr = "Media";
        else if (app.category == AppCategory::Settings) catStr = "Config";
        surface.drawString(rowRect.right() - 56, rowRect.y + 16, catStr, palette.accentSecondary, 1);
    }
}

void StartMenu::renderPowerFlyout(Surface& surface, Rect flyoutBounds) {
    const auto& palette = ThemeManager::instance().palette();

    // Drop shadow
    surface.drawDropShadow(flyoutBounds, 16, 0.60f);

    // Frosted Acrylic container
    surface.applyAcrylicTint(flyoutBounds, palette.startMenuBg, 8);
    surface.drawRoundedRect(flyoutBounds, 10, palette.startMenuBorder, false);

    // Header label
    surface.drawString(flyoutBounds.x + 16, flyoutBounds.y + 10, "POWER OPTIONS", palette.accentColor, 1);
    surface.fillRect(Rect{flyoutBounds.x + 14, flyoutBounds.y + 24, flyoutBounds.width - 28, 1}, palette.startCardBorder);

    // List of power items
    const int32_t itemY = flyoutBounds.y + 30;
    const int32_t itemH = 32;

    for (size_t i = 0; i < powerOptions_.size(); ++i) {
        const auto& opt = powerOptions_[i];
        Rect itemRect{flyoutBounds.x + 8, itemY + static_cast<int32_t>(i) * itemH, flyoutBounds.width - 16, itemH - 2};

        const bool isHovered = (static_cast<int32_t>(i) == hoveredPowerFlyoutIndex_);
        if (isHovered) {
            Color hoverCol = (opt.action == PowerAction::ShutDown)
                ? Color::fromRgba(255, 69, 58, 60)
                : Color::fromRgba(255, 255, 255, 30);
            surface.drawRoundedRect(itemRect, 6, hoverCol, true);
            surface.drawRoundedRect(itemRect, 6, palette.accentColor, false);
        }

        // Icon
        Color iconTint = (opt.action == PowerAction::ShutDown) ? Color::fromHex(0xFF453A) : palette.textPrimary;
        IconRenderer::draw(surface, opt.iconId, Rect{itemRect.x + 8, itemRect.y + 6, 18, 18}, iconTint);

        // Label
        surface.drawString(itemRect.x + 36, itemRect.y + 9, opt.label, palette.textPrimary, 1);
    }
}

void StartMenu::renderUserFlyout(Surface& surface, Rect flyoutBounds) {
    const auto& palette = ThemeManager::instance().palette();

    // Drop shadow
    surface.drawDropShadow(flyoutBounds, 16, 0.60f);

    // Frosted Acrylic container
    surface.applyAcrylicTint(flyoutBounds, palette.startMenuBg, 8);
    surface.drawRoundedRect(flyoutBounds, 10, palette.startMenuBorder, false);

    // Header label
    surface.drawString(flyoutBounds.x + 16, flyoutBounds.y + 10, "USER ACCOUNT", palette.accentColor, 1);
    surface.fillRect(Rect{flyoutBounds.x + 14, flyoutBounds.y + 24, flyoutBounds.width - 28, 1}, palette.startCardBorder);

    struct UserOpt {
        const char* label;
        IconId icon;
    };
    UserOpt opts[] = {
        {"Account Settings", IconId::Settings},
        {"Lock Workstation", IconId::Lock},
        {"Sign Out", IconId::SignOut}
    };

    const int32_t itemY = flyoutBounds.y + 30;
    const int32_t itemH = 32;

    for (int32_t i = 0; i < 3; ++i) {
        Rect itemRect{flyoutBounds.x + 8, itemY + i * itemH, flyoutBounds.width - 16, itemH - 2};

        const bool isHovered = (i == hoveredUserFlyoutIndex_);
        if (isHovered) {
            surface.drawRoundedRect(itemRect, 6, Color::fromRgba(255, 255, 255, 30), true);
            surface.drawRoundedRect(itemRect, 6, palette.accentColor, false);
        }

        IconRenderer::draw(surface, opts[i].icon, Rect{itemRect.x + 8, itemRect.y + 6, 18, 18}, palette.textPrimary);
        surface.drawString(itemRect.x + 36, itemRect.y + 9, opts[i].label, palette.textPrimary, 1);
    }
}

} // namespace surshell
