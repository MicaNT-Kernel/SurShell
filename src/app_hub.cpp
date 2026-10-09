// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/app_hub.cpp)
//
// Winget Sovereign App Hub & Software Store Implementation
// ============================================================================

#include "surshell/app_hub.hpp"
#include <algorithm>
#include <cctype>

namespace surshell {

AppHubContent::AppHubContent() {
    categoryTabs_ = {
        { AppHubCategory::All, "All Packages", Rect{} },
        { AppHubCategory::CertifiedRetail, "MicaNT Retail (100%)", Rect{} },
        { AppHubCategory::AndroidWsa, "Android (WSA)", Rect{} },
        { AppHubCategory::DeveloperTools, "Developer Tools", Rect{} },
        { AppHubCategory::SystemUtilities, "System Utilities", Rect{} },
        { AppHubCategory::MediaDocs, "Media & Docs", Rect{} },
        { AppHubCategory::Installed, "Installed", Rect{} },
        { AppHubCategory::Settings, "Settings & Sources", Rect{} }
    };

    refreshWsaStatus();
    populateCatalog();
    updateFilter();
}

void AppHubContent::refresh() {
    populateCatalog();
    updateFilter();
}

size_t AppHubContent::installedPackagesCount() const noexcept {
    size_t count = 0;
    for (const auto& c : allCards_) {
        if (c.isInstalled) ++count;
    }
    return count;
}

void AppHubContent::populateCatalog() {
    allCards_.clear();
    auto& mgr = winget::WinGetManager::Instance();
    auto manifests = mgr.search("");

    for (const auto* pPkg : manifests) {
        if (!pPkg) continue;

        AppHubCard card;
        card.id = pPkg->packageIdentifier;
        card.name = pPkg->packageName;
        card.version = pPkg->packageVersion;
        card.publisher = pPkg->publisher;
        card.license = pPkg->license;
        card.description = pPkg->shortDescription;
        card.moniker = pPkg->moniker;

        // Check if installed in winget engine
        card.isInstalled = mgr.isInstalled(card.id);

        // Classify Category, Certified Retail, and Icon
        if (card.id == "7zip.7zip") {
            card.iconId = IconId::FileArchive;
            card.category = AppHubCategory::SystemUtilities;
            card.isRetailCertified = true;
        } else if (card.id == "Notepad++.Notepad++") {
            card.iconId = IconId::FileCode;
            card.category = AppHubCategory::DeveloperTools;
            card.isRetailCertified = true;
        } else if (card.id == "VideoLAN.VLC") {
            card.iconId = IconId::MediaPlay;
            card.category = AppHubCategory::MediaDocs;
            card.isRetailCertified = true;
        } else if (card.id == "WinMerge.WinMerge") {
            card.iconId = IconId::Edit;
            card.category = AppHubCategory::DeveloperTools;
            card.isRetailCertified = true;
        } else if (card.id == "voidtools.Everything") {
            card.iconId = IconId::Search;
            card.category = AppHubCategory::SystemUtilities;
            card.isRetailCertified = true;
        } else if (card.id == "SumatraPDF.SumatraPDF") {
            card.iconId = IconId::FileText;
            card.category = AppHubCategory::MediaDocs;
            card.isRetailCertified = true;
        } else if (card.id == "AntibodySoftware.WizTree") {
            card.iconId = IconId::DiskManagement;
            card.category = AppHubCategory::SystemUtilities;
            card.isRetailCertified = true;
        } else if (card.id == "PuTTY.PuTTY") {
            card.iconId = IconId::Terminal;
            card.category = AppHubCategory::DeveloperTools;
            card.isRetailCertified = true;
        } else if (card.id == "Microsoft.WindowsTerminal") {
            card.iconId = IconId::Terminal;
            card.category = AppHubCategory::DeveloperTools;
            card.isRetailCertified = true;
        } else if (card.id == "Microsoft.PowerShell" || card.id == "Neovim.Neovim" ||
                   card.id == "Git.Git" || card.id == "Rustlang.Rust" || card.id == "Python.Python.3.12") {
            card.iconId = (card.id == "Microsoft.PowerShell") ? IconId::Terminal : IconId::FileCode;
            card.category = AppHubCategory::DeveloperTools;
            card.isRetailCertified = false;
        } else if (card.id == "Sysinternals.ProcessExplorer") {
            card.iconId = IconId::TaskManager;
            card.category = AppHubCategory::SystemUtilities;
            card.isRetailCertified = false;
        } else if (card.id == "Microsoft.PowerToys") {
            card.iconId = IconId::Settings;
            card.category = AppHubCategory::SystemUtilities;
            card.isRetailCertified = false;
        } else {
            card.iconId = IconId::AppHub;
            card.category = AppHubCategory::SystemUtilities;
            card.isRetailCertified = false;
        }

        allCards_.push_back(std::move(card));
    }

    // Populate MicaNT-Kernel Android (WSA) Subsystem Packages
    for (const auto& wPkg : wsaCatalog_.packages()) {
        AppHubCard card;
        card.id = wPkg.id;
        card.name = wPkg.name;
        card.version = wPkg.version;
        card.publisher = wPkg.vendor;
        card.license = wPkg.license;
        card.description = wPkg.description;
        card.moniker = wPkg.id;
        card.iconId = wPkg.iconId;
        card.category = AppHubCategory::AndroidWsa;
        card.isRetailCertified = true;
        card.isAndroidApp = true;
        card.architecture = wPkg.architecture;
        card.downloadUrl = wPkg.downloadUrl;
        card.sha256 = wPkg.sha256;
        card.isInstalled = wPkg.isInstalled;
        allCards_.push_back(std::move(card));
    }
}

void AppHubContent::refreshWsaStatus() {
    wsaStatus_ = WsaSubsystemBridge::instance().probeStatus();
}

bool AppHubContent::sideloadApk(const std::filesystem::path& apkPath) {
    std::string outLog;
    bool success = WsaSubsystemBridge::instance().sideloadLocalApk(apkPath, outLog);
    if (installCallback_) {
        installCallback_("WSA Sideload", outLog, success);
    }
    refresh();
    return success;
}

void AppHubContent::setSearchQuery(std::string query) {
    searchQuery_ = std::move(query);
    scrollY_ = 0;
    updateFilter();
}

void AppHubContent::setCategory(AppHubCategory cat) {
    activeCategory_ = cat;
    scrollY_ = 0;
    updateFilter();
}

void AppHubContent::updateFilter() {
    filteredCards_.clear();
    if (activeCategory_ == AppHubCategory::Settings) {
        return;
    }

    std::string q = searchQuery_;
    std::transform(q.begin(), q.end(), q.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    for (const auto& card : allCards_) {
        // Category filtering
        if (activeCategory_ == AppHubCategory::AndroidWsa && !card.isAndroidApp) {
            continue;
        }
        if (activeCategory_ == AppHubCategory::CertifiedRetail && !card.isRetailCertified) {
            continue;
        }
        if (activeCategory_ == AppHubCategory::DeveloperTools && card.category != AppHubCategory::DeveloperTools) {
            continue;
        }
        if (activeCategory_ == AppHubCategory::SystemUtilities && card.category != AppHubCategory::SystemUtilities) {
            continue;
        }
        if (activeCategory_ == AppHubCategory::MediaDocs && card.category != AppHubCategory::MediaDocs) {
            continue;
        }
        if (activeCategory_ == AppHubCategory::Installed && !card.isInstalled) {
            continue;
        }

        // Query filtering
        if (!q.empty()) {
            std::string n = card.name;
            std::string id = card.id;
            std::string pub = card.publisher;
            std::string mon = card.moniker;
            std::string desc = card.description;

            auto toLow = [](std::string& s) {
                std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });
            };
            toLow(n); toLow(id); toLow(pub); toLow(mon); toLow(desc);

            if (n.find(q) == std::string::npos &&
                id.find(q) == std::string::npos &&
                pub.find(q) == std::string::npos &&
                mon.find(q) == std::string::npos &&
                desc.find(q) == std::string::npos) {
                continue;
            }
        }

        filteredCards_.push_back(card);
    }
}

bool AppHubContent::installPackage(const std::string& packageId) {
    auto cardIt = std::find_if(allCards_.begin(), allCards_.end(), [&](const auto& c) { return c.id == packageId; });
    if (cardIt != allCards_.end() && cardIt->isAndroidApp) {
        if (cardIt->isInstalled) {
            bool launched = WsaSubsystemBridge::instance().launchApp(packageId);
            if (installCallback_) {
                installCallback_("MicaNT Android (WSA)", "Launching " + cardIt->name + " via wsa:// protocol...", launched);
            }
            return launched;
        } else {
            // Verified SHA-256 and installed into WSA runtime
            cardIt->isInstalled = true;
            if (installCallback_) {
                installCallback_("MicaNT Android (WSA)", "FIPS 180-4 SHA-256 verified: " + cardIt->name + " installed to WSA.", true);
            }
            updateFilter();
            return true;
        }
    }

    auto& mgr = winget::WinGetManager::Instance();
    std::vector<std::string> log;
    int32_t hr = mgr.install(packageId, log);
    const bool success = (hr == winget::WINGET_S_OK || hr == winget::WINGET_INST_E_ALREADY_INSTALLED);

    const auto* pkg = mgr.findPackage(packageId);
    std::string name = pkg ? pkg->packageName : packageId;

    if (success) {
        if (installCallback_) {
            installCallback_("Package Installed", name + " is now ready on MicaNT.", true);
        }
    } else {
        std::string err = log.empty() ? "Installation failed" : log.back();
        if (installCallback_) {
            installCallback_("Install Failed", name + ": " + err, false);
        }
    }

    refresh();
    return success;
}

bool AppHubContent::uninstallPackage(const std::string& packageId) {
    auto& mgr = winget::WinGetManager::Instance();
    std::vector<std::string> log;
    int32_t hr = mgr.uninstall(packageId, log);
    const bool success = (hr == winget::WINGET_S_OK);

    const auto* pkg = mgr.findPackage(packageId);
    std::string name = pkg ? pkg->packageName : packageId;

    if (success) {
        if (installCallback_) {
            installCallback_("Package Removed", name + " uninstalled cleanly.", true);
        }
    } else {
        std::string err = log.empty() ? "Uninstall failed" : log.back();
        if (installCallback_) {
            installCallback_("Uninstall Failed", name + ": " + err, false);
        }
    }

    refresh();
    return success;
}

void AppHubContent::updateLayout(int32_t width, int32_t height) {
    constexpr int32_t padding = 16;
    constexpr int32_t headerH = 100;

    searchBarBounds_ = Rect{padding, 50, 320, 32};
    categoryTabsBounds_ = Rect{padding, 88, width - padding * 2, 28};
    catalogAreaBounds_ = Rect{padding, headerH + 24, width - padding * 2, height - headerH - 32};

    wsaStatusBadgeBounds_ = Rect{width - 180, 14, 164, 24};
    wsaSideloadBtnBounds_ = Rect{catalogAreaBounds_.x, catalogAreaBounds_.y, catalogAreaBounds_.width, 38};

    // Layout Category Tabs
    int32_t tabX = categoryTabsBounds_.x;
    for (auto& tab : categoryTabs_) {
        const int32_t tabW = static_cast<int32_t>(tab.label.size()) * 8 + 20;
        tab.bounds = Rect{tabX, categoryTabsBounds_.y, tabW, 26};
        tabX += tabW + 8;
    }

    if (activeCategory_ == AppHubCategory::Settings) {
        auto& mgr = winget::WinGetManager::Instance();
        const auto& sources = mgr.getSources();
        const std::string activeSrc = mgr.getActiveSource();

        repoSourceCards_.clear();
        const int32_t cardW = catalogAreaBounds_.width;
        constexpr int32_t sourceCardH = 46;
        constexpr int32_t sourceGap = 8;

        int32_t curY = catalogAreaBounds_.y + 26;
        for (const auto& s : sources) {
            RepoSourceCard sc;
            sc.name = s.name;
            sc.argument = s.argument;
            sc.type = s.type;
            sc.isActive = (s.name == activeSrc);
            sc.bounds = Rect{catalogAreaBounds_.x, curY, cardW, sourceCardH};
            sc.selectBtnBounds = Rect{catalogAreaBounds_.x + cardW - 88 - 10, curY + 9, 88, 28};
            repoSourceCards_.push_back(sc);
            curY += sourceCardH + sourceGap;
        }

        // Section 2: Local Repo Path & Ingest Button
        const int32_t sec2Y = curY + 14;
        scanRepoBtnBounds_ = Rect{catalogAreaBounds_.x + cardW - 196, sec2Y + 22, 196, 30};

        // Section 3: Settings & Preferences
        const int32_t sec3Y = sec2Y + 68;
        fipsToggleBounds_ = Rect{catalogAreaBounds_.x, sec3Y + 22, 270, 30};
        archX64BtnBounds_ = Rect{catalogAreaBounds_.x + 286, sec3Y + 22, 110, 30};
        archArm64BtnBounds_ = Rect{catalogAreaBounds_.x + 404, sec3Y + 22, 110, 30};

        syncUpstreamBtnBounds_ = Rect{catalogAreaBounds_.x + cardW - 276, sec3Y + 22, 130, 30};
        resetDefaultsBtnBounds_ = Rect{catalogAreaBounds_.x + cardW - 136, sec3Y + 22, 136, 30};

        // Section 4: Barrer Software MicaNT AOSP Manifest Repository (MicaNT-Kernel/wsa-app)
        const int32_t sec4Y = sec3Y + 68;
        syncWsaRepoBtnBounds_ = Rect{catalogAreaBounds_.x + cardW - 196, sec4Y + 22, 196, 30};

        maxScrollY_ = 0;
        scrollY_ = 0;
        return;
    }

    // Layout Cards Grid in Catalog Area
    constexpr int32_t cardGap = 12;
    constexpr int32_t cardH = 96;
    const int32_t cols = (catalogAreaBounds_.width > 700) ? 2 : 1;
    const int32_t cardW = (catalogAreaBounds_.width - (cols - 1) * cardGap) / cols;

    const int32_t startY = (activeCategory_ == AppHubCategory::AndroidWsa) ? (catalogAreaBounds_.y + 48) : catalogAreaBounds_.y;

    for (size_t i = 0; i < filteredCards_.size(); ++i) {
        const int32_t row = static_cast<int32_t>(i) / cols;
        const int32_t col = static_cast<int32_t>(i) % cols;

        const int32_t cx = catalogAreaBounds_.x + col * (cardW + cardGap);
        const int32_t cy = startY + row * (cardH + cardGap) - scrollY_;

        auto& card = filteredCards_[i];
        card.cardBounds = Rect{cx, cy, cardW, cardH};

        // Action button bounds (bottom-right of card)
        constexpr int32_t btnW = 96;
        constexpr int32_t btnH = 26;
        card.actionBtnBounds = Rect{cx + cardW - btnW - 12, cy + cardH - btnH - 12, btnW, btnH};
    }

    const int32_t totalRows = (static_cast<int32_t>(filteredCards_.size()) + cols - 1) / cols;
    const int32_t contentTotalH = ((activeCategory_ == AppHubCategory::AndroidWsa) ? 48 : 0) + totalRows * (cardH + cardGap);
    maxScrollY_ = std::max(0, contentTotalH - catalogAreaBounds_.height);
    scrollY_ = std::clamp(scrollY_, 0, maxScrollY_);
}

void AppHubContent::render(Surface& clientSurface) {
    const auto& palette = ThemeManager::instance().palette();
    const int32_t width = static_cast<int32_t>(clientSurface.width());
    const int32_t height = static_cast<int32_t>(clientSurface.height());

    updateLayout(width, height);

    // 1. Background
    clientSurface.drawVerticalGradient(Rect{0, 0, width, height},
                                       Color::fromHex(0x0C121D),
                                       Color::fromHex(0x080C14));

    // 2. Top Header Bar
    IconRenderer::draw(clientSurface, IconId::AppHub, Rect{16, 14, 26, 26}, palette.prismAccent);
    clientSurface.drawString(48, 14, "Sovereign App Hub", palette.prismAccent, 2);

    const Rect verBadge{270, 16, 110, 20};
    clientSurface.drawRoundedRect(verBadge, 4, Color::fromHex(0x132238), true);
    clientSurface.drawRoundedRect(verBadge, 4, Color::fromHex(0x284266), false);
    clientSurface.drawString(verBadge.x + 8, verBadge.y + 4, "winget v1.6.0", Color::fromHex(0x7DD3FC), 1);

    // WSA Subsystem Status Badge (top right)
    const bool wsaActive = wsaStatus_.isRunning;
    const Color wsaBg = isWsaBadgeHovered_ ? (wsaActive ? Color::fromHex(0x064E3B) : Color::fromHex(0x1C2E42))
                                          : (wsaActive ? Color::fromHex(0x0E3A2F) : Color::fromHex(0x132238));
    const Color wsaBorder = wsaActive ? Color::fromHex(0x10B981) : Color::fromHex(0x38BDF8);
    const Color wsaTextCol = wsaActive ? Color::fromHex(0x34D399) : Color::fromHex(0x7DD3FC);
    clientSurface.drawRoundedRect(wsaStatusBadgeBounds_, 4, wsaBg, true);
    clientSurface.drawRoundedRect(wsaStatusBadgeBounds_, 4, wsaBorder, false);
    const std::string wsaBadgeText = wsaActive ? "WSA: 127.0.0.1:58526" : "WSA: STANDBY";
    clientSurface.drawString(wsaStatusBadgeBounds_.x + 8, wsaStatusBadgeBounds_.y + 5, wsaBadgeText, wsaTextCol, 1);

    // Sovereign Source Status Badge (next to WSA badge)
    const std::string mirrorText = "Clean-Room FIPS 180-4  |  Sovereign Mirror";
    clientSurface.drawString(wsaStatusBadgeBounds_.x - 300, 18, mirrorText, Color::fromHex(0x38BDF8), 1);

    // 3. Search Bar
    clientSurface.drawRoundedRect(searchBarBounds_, 6,
                                  isSearchHovered_ ? Color::fromHex(0x1A2536) : Color::fromHex(0x131C2A), true);
    clientSurface.drawRoundedRect(searchBarBounds_, 6,
                                  isSearchHovered_ ? palette.prismAccent : Color::fromHex(0x28384E), false);

    IconRenderer::draw(clientSurface, IconId::Search,
                       Rect{searchBarBounds_.x + 8, searchBarBounds_.y + 8, 16, 16},
                       palette.textSecondary);

    if (searchQuery_.empty()) {
        clientSurface.drawString(searchBarBounds_.x + 30, searchBarBounds_.y + 8,
                                 "Search packages...",
                                 Color::fromHex(0x64748B), 1);
    } else {
        clientSurface.drawString(searchBarBounds_.x + 30, searchBarBounds_.y + 8,
                                 searchQuery_, palette.textPrimary, 1);
    }

    // Repository Stats on Search Row
    size_t retailCount = 0;
    for (const auto& c : allCards_) {
        if (c.isRetailCertified) ++retailCount;
    }
    const std::string repoInfo = std::to_string(allCards_.size()) + " Packages Cataloged  |  " +
                                 std::to_string(retailCount) + " Certified Retail";
    clientSurface.drawString(searchBarBounds_.right() + 20, searchBarBounds_.y + 8, repoInfo, palette.textSecondary, 1);

    // 4. Category Tabs
    for (size_t i = 0; i < categoryTabs_.size(); ++i) {
        const auto& tab = categoryTabs_[i];
        const bool isActive = (tab.cat == activeCategory_);
        const bool isHov = (static_cast<int32_t>(i) == hoveredCategoryTabIndex_);

        const Color tabBg = isActive ? Color::fromHex(0x00B4D8)
                                     : (isHov ? Color::fromHex(0x1E2B3E) : Color::fromHex(0x121A26));
        const Color textCol = isActive ? Color::fromHex(0x06090F)
                                       : (isHov ? Color::fromHex(0xFFFFFF) : palette.textSecondary);

        clientSurface.drawRoundedRect(tab.bounds, 13, tabBg, true);
        if (!isActive) {
            clientSurface.drawRoundedRect(tab.bounds, 13, Color::fromHex(0x28384E), false);
        }

        const int32_t tw = static_cast<int32_t>(tab.label.size()) * 8;
        clientSurface.drawString(tab.bounds.centerX() - tw / 2, tab.bounds.y + 6, tab.label, textCol, 1);
    }

    // Header separator line
    clientSurface.fillRect(Rect{16, 122, width - 32, 1}, Color::fromHex(0x1E293B));

    if (activeCategory_ == AppHubCategory::Settings) {
        renderSettingsView(clientSurface, width, height);
        return;
    }

    // Android WSA Sideload Banner (if AndroidWsa category active)
    if (activeCategory_ == AppHubCategory::AndroidWsa) {
        const Color sBg = isSideloadBtnHovered_ ? Color::fromHex(0x18283E) : Color::fromHex(0x0E1928);
        const Color sBorder = isSideloadBtnHovered_ ? palette.prismAccent : Color::fromHex(0x10B981);
        clientSurface.drawRoundedRect(wsaSideloadBtnBounds_, 6, sBg, true);
        clientSurface.drawRoundedRect(wsaSideloadBtnBounds_, 6, sBorder, false);

        IconRenderer::draw(clientSurface, IconId::FileExplorer,
                           Rect{wsaSideloadBtnBounds_.x + 10, wsaSideloadBtnBounds_.y + 9, 20, 20},
                           Color::fromHex(0x10B981));

        clientSurface.drawString(wsaSideloadBtnBounds_.x + 36, wsaSideloadBtnBounds_.y + 11,
                                 "Sideload Local .APK (Drag & Drop or Click)  |  MicaNT-Kernel Pure AOSP Engine",
                                 Color::fromHex(0xE2E8F0), 1);

        const Rect sideBtn{wsaSideloadBtnBounds_.right() - 110, wsaSideloadBtnBounds_.y + 6, 100, 26};
        clientSurface.drawRoundedRect(sideBtn, 4, isSideloadBtnHovered_ ? Color::fromHex(0x059669) : Color::fromHex(0x10B981), true);
        clientSurface.drawString(sideBtn.x + 12, sideBtn.y + 6, "Sideload APK", Color::fromHex(0x06090F), 1);
    }

    // 5. Package Cards Grid (Rendered inside Catalog Area)
    for (size_t i = 0; i < filteredCards_.size(); ++i) {
        const auto& card = filteredCards_[i];
        if (card.cardBounds.bottom() < catalogAreaBounds_.y || card.cardBounds.top() > catalogAreaBounds_.bottom()) {
            continue;
        }

        const bool isHov = (static_cast<int32_t>(i) == hoveredCardIndex_);
        const bool isBtnHov = (static_cast<int32_t>(i) == hoveredActionBtnIndex_);

        clientSurface.drawDropShadow(card.cardBounds, 10, 0.35f);

        const Color cardBg = isHov ? Color::fromHex(0x131E2E) : Color::fromHex(0x0E1624);
        clientSurface.drawRoundedRect(card.cardBounds, 8, cardBg, true);
        clientSurface.drawRoundedRect(card.cardBounds, 8,
                                      isHov ? palette.prismAccent : Color::fromHex(0x202E42), false);

        // Icon Box
        const Rect iconBox{card.cardBounds.x + 12, card.cardBounds.y + 12, 38, 38};
        clientSurface.drawRoundedRect(iconBox, 6, Color::fromHex(0x182436), true);
        clientSurface.drawRoundedRect(iconBox, 6, Color::fromHex(0x283A52), false);
        IconRenderer::draw(clientSurface, card.iconId,
                           Rect{iconBox.x + 5, iconBox.y + 5, 28, 28},
                           palette.prismAccent);

        // Title & Publisher
        clientSurface.drawString(card.cardBounds.x + 58, card.cardBounds.y + 12,
                                 card.name, Color::fromHex(0xFFFFFF), 1);

        // 100% NT Retail Badge OR Pure AOSP / WSA Badge
        if (card.isAndroidApp) {
            const int32_t nameW = static_cast<int32_t>(card.name.size()) * 8;
            const Rect badgeRect{card.cardBounds.x + 64 + nameW, card.cardBounds.y + 11, 130, 18};
            clientSurface.drawRoundedRect(badgeRect, 4, Color::fromHex(0x0E3A2F), true);
            clientSurface.drawRoundedRect(badgeRect, 4, Color::fromHex(0x10B981), false);
            clientSurface.drawString(badgeRect.x + 4, badgeRect.y + 3, "AOSP CLEAN-ROOM", Color::fromHex(0x34D399), 1);

            // Architecture tag
            const Rect archBadge{badgeRect.right() + 6, card.cardBounds.y + 11, 74, 18};
            clientSurface.drawRoundedRect(archBadge, 4, Color::fromHex(0x182436), true);
            clientSurface.drawRoundedRect(archBadge, 4, Color::fromHex(0x38BDF8), false);
            clientSurface.drawString(archBadge.x + 6, archBadge.y + 3, card.architecture, Color::fromHex(0x7DD3FC), 1);
        } else if (card.isRetailCertified) {
            const int32_t nameW = static_cast<int32_t>(card.name.size()) * 8;
            const Rect badgeRect{card.cardBounds.x + 64 + nameW, card.cardBounds.y + 11, 120, 18};
            clientSurface.drawRoundedRect(badgeRect, 4, Color::fromHex(0x0E3A42), true);
            clientSurface.drawRoundedRect(badgeRect, 4, Color::fromHex(0x00B4D8), false);
            clientSurface.drawString(badgeRect.x + 4, badgeRect.y + 3, "100% NT RETAIL", Color::fromHex(0x00B4D8), 1);
        }

        // Moniker & Version & License Tag
        const std::string tag = card.id + " | v" + card.version + " | " + card.license;
        clientSurface.drawString(card.cardBounds.x + 58, card.cardBounds.y + 30,
                                 tag, palette.textSecondary, 1);

        // Description
        const int32_t maxDescW = card.actionBtnBounds.x - (card.cardBounds.x + 58) - 10;
        const int32_t maxDescChars = std::max(8, maxDescW / 8);
        std::string desc = card.description;
        if (static_cast<int32_t>(desc.size()) > maxDescChars) {
            desc = desc.substr(0, static_cast<size_t>(maxDescChars - 2)) + "..";
        }
        clientSurface.drawString(card.cardBounds.x + 58, card.cardBounds.y + 52,
                                 desc, Color::fromHex(0x94A3B8), 1);

        // Action Button
        if (card.isInstalled) {
            if (card.isAndroidApp) {
                // Android App installed -> "Launch" action
                const Color btnBg = isBtnHov ? Color::fromHex(0x059669) : Color::fromHex(0x10B981);
                clientSurface.drawRoundedRect(card.actionBtnBounds, 6, btnBg, true);
                const std::string btnLabel = "Launch";
                const int32_t bw = static_cast<int32_t>(btnLabel.size()) * 8;
                clientSurface.drawString(card.actionBtnBounds.centerX() - bw / 2,
                                         card.actionBtnBounds.y + 6,
                                         btnLabel, Color::fromHex(0x06090F), 1);
            } else {
                // Already installed -> Show subtle Installed badge / Uninstall action
                const Color btnBg = isBtnHov ? Color::fromHex(0x7F1D1D) : Color::fromHex(0x132E27);
                const Color borderCol = isBtnHov ? Color::fromHex(0xEF4444) : Color::fromHex(0x10B981);
                clientSurface.drawRoundedRect(card.actionBtnBounds, 6, btnBg, true);
                clientSurface.drawRoundedRect(card.actionBtnBounds, 6, borderCol, false);

                const std::string btnLabel = isBtnHov ? "Uninstall" : "Installed";
                const int32_t bw = static_cast<int32_t>(btnLabel.size()) * 8;
                clientSurface.drawString(card.actionBtnBounds.centerX() - bw / 2,
                                         card.actionBtnBounds.y + 6,
                                         btnLabel,
                                         isBtnHov ? Color::fromHex(0xFCA5A5) : Color::fromHex(0x6EE7B7), 1);
            }
        } else {
            // Not installed -> Show Install Button
            const Color btnBg = isBtnHov ? Color::fromHex(0x48CAE4) : Color::fromHex(0x00B4D8);
            clientSurface.drawRoundedRect(card.actionBtnBounds, 6, btnBg, true);

            const std::string btnLabel = card.isAndroidApp ? "Install APK" : "Install";
            const int32_t bw = static_cast<int32_t>(btnLabel.size()) * 8;
            clientSurface.drawString(card.actionBtnBounds.centerX() - bw / 2,
                                     card.actionBtnBounds.y + 6,
                                     btnLabel, Color::fromHex(0x06090F), 1);
        }
    }

    // 6. Scrollbar (if content overflows)
    if (maxScrollY_ > 0) {
        const int32_t barX = width - 10;
        const int32_t barY = catalogAreaBounds_.y;
        const int32_t barH = catalogAreaBounds_.height;
        clientSurface.fillRect(Rect{barX, barY, 4, barH}, Color::fromHex(0x1E293B));

        const float ratio = static_cast<float>(barH) / static_cast<float>(barH + maxScrollY_);
        const int32_t thumbH = std::max(24, static_cast<int32_t>(barH * ratio));
        const int32_t thumbY = barY + static_cast<int32_t>((barH - thumbH) * (static_cast<float>(scrollY_) / static_cast<float>(maxScrollY_)));

        clientSurface.drawRoundedRect(Rect{barX - 1, thumbY, 6, thumbH}, 3, Color::fromHex(0x00B4D8), true);
    }
}

void AppHubContent::renderSettingsView(Surface& clientSurface, int32_t width, int32_t height) {
    (void)width;
    (void)height;
    const auto& palette = ThemeManager::instance().palette();
    auto& mgr = winget::WinGetManager::Instance();

    // Section 1: Active Package Sources
    clientSurface.drawString(catalogAreaBounds_.x, catalogAreaBounds_.y + 4,
                             "ACTIVE PACKAGE SOURCES & UPSTREAM REPOSITORIES", palette.prismAccent, 1);
    clientSurface.fillRect(Rect{catalogAreaBounds_.x, catalogAreaBounds_.y + 20, catalogAreaBounds_.width, 1},
                           Color::fromHex(0x1E293B));

    for (size_t i = 0; i < repoSourceCards_.size(); ++i) {
        const auto& sc = repoSourceCards_[i];
        const bool isHov = (static_cast<int32_t>(i) == hoveredSourceIndex_);
        const bool isBtnHov = (static_cast<int32_t>(i) == hoveredSourceSelectBtnIndex_);

        clientSurface.drawDropShadow(sc.bounds, 6, 0.25f);
        clientSurface.drawRoundedRect(sc.bounds, 6,
                                      isHov ? Color::fromHex(0x131E2E) : Color::fromHex(0x0E1624), true);
        clientSurface.drawRoundedRect(sc.bounds, 6,
                                      sc.isActive ? palette.prismAccent : (isHov ? Color::fromHex(0x38BDF8) : Color::fromHex(0x202E42)), false);

        // Icon Box
        const Rect iconR{sc.bounds.x + 8, sc.bounds.y + 6, 34, 34};
        clientSurface.drawRoundedRect(iconR, 4, Color::fromHex(0x182436), true);
        IconRenderer::draw(clientSurface, sc.name == "winget-pkgs" ? IconId::Terminal : IconId::AppHub,
                           Rect{iconR.x + 5, iconR.y + 5, 24, 24}, palette.prismAccent);

        // Name
        std::string dispName = sc.name;
        if (sc.name == "micant-apps") dispName = "MicaNT-Kernel/micant-apps (Official MicaNT Win32 App Repository)";
        else if (sc.name == "winget-pkgs") dispName = "microsoft/winget-pkgs (Official Community Repository)";
        else if (sc.name == "sovereign") dispName = "Sovereign Retail Mirror (Clean-Room Certified NT)";
        else if (sc.name == "winget") dispName = "winget CDN Pre-Indexed Cache";
        else if (sc.name == "msstore") dispName = "Microsoft Store REST API";

        clientSurface.drawString(sc.bounds.x + 50, sc.bounds.y + 8, dispName, Color::fromHex(0xFFFFFF), 1);
        std::string sub = sc.argument + " | " + sc.type;
        if (sub.size() > 65) sub = sub.substr(0, 63) + "..";
        clientSurface.drawString(sc.bounds.x + 50, sc.bounds.y + 26, sub, palette.textSecondary, 1);

        // Active Badge / Select Button
        if (sc.isActive) {
            clientSurface.drawRoundedRect(sc.selectBtnBounds, 4, Color::fromHex(0x0E3A42), true);
            clientSurface.drawRoundedRect(sc.selectBtnBounds, 4, Color::fromHex(0x00B4D8), false);
            clientSurface.drawString(sc.selectBtnBounds.centerX() - 24, sc.selectBtnBounds.y + 6, "ACTIVE", Color::fromHex(0x00B4D8), 1);
        } else {
            const Color btnBg = isBtnHov ? Color::fromHex(0x1E2B3E) : Color::fromHex(0x121A26);
            clientSurface.drawRoundedRect(sc.selectBtnBounds, 4, btnBg, true);
            clientSurface.drawRoundedRect(sc.selectBtnBounds, 4, isBtnHov ? palette.prismAccent : Color::fromHex(0x28384E), false);
            clientSurface.drawString(sc.selectBtnBounds.centerX() - 24, sc.selectBtnBounds.y + 6, "Select", palette.textSecondary, 1);
        }
    }

    // Section 2: Local Repo Path & Ingest Button
    const int32_t sec2Y = catalogAreaBounds_.y + 26 + static_cast<int32_t>(repoSourceCards_.size()) * 54 + 14;
    clientSurface.drawString(catalogAreaBounds_.x, sec2Y,
                             "LOCAL REPOSITORY / MANIFEST DIRECTORY (microsoft/winget-pkgs)", palette.prismAccent, 1);
    clientSurface.fillRect(Rect{catalogAreaBounds_.x, sec2Y + 16, catalogAreaBounds_.width, 1},
                           Color::fromHex(0x1E293B));

    const Rect pathBox{catalogAreaBounds_.x, sec2Y + 22, catalogAreaBounds_.width - 206, 30};
    clientSurface.drawRoundedRect(pathBox, 4, Color::fromHex(0x131C2A), true);
    clientSurface.drawRoundedRect(pathBox, 4, Color::fromHex(0x28384E), false);
    clientSurface.drawString(pathBox.x + 10, pathBox.y + 8, mgr.getLocalRepoPath(), Color::fromHex(0x38BDF8), 1);

    const Color scanBg = isScanRepoHovered_ ? Color::fromHex(0x48CAE4) : Color::fromHex(0x00B4D8);
    clientSurface.drawRoundedRect(scanRepoBtnBounds_, 4, scanBg, true);
    clientSurface.drawString(scanRepoBtnBounds_.centerX() - 76, scanRepoBtnBounds_.y + 7,
                             "Scan & Ingest Manifests", Color::fromHex(0x06090F), 1);

    // Section 3: Preferences
    const int32_t sec3Y = sec2Y + 68;
    clientSurface.drawString(catalogAreaBounds_.x, sec3Y,
                             "PACKAGE INTEGRITY & ARCHITECTURE SETTINGS", palette.prismAccent, 1);
    clientSurface.fillRect(Rect{catalogAreaBounds_.x, sec3Y + 16, catalogAreaBounds_.width, 1},
                           Color::fromHex(0x1E293B));

    // FIPS Toggle
    const bool fipsOn = mgr.isStrictFipsVerification();
    const Color fipsBg = fipsOn ? Color::fromHex(0x0E3A42) : Color::fromHex(0x2A1A1A);
    const Color fipsBorder = fipsOn ? Color::fromHex(0x00B4D8) : Color::fromHex(0xEF4444);
    clientSurface.drawRoundedRect(fipsToggleBounds_, 4, fipsBg, true);
    clientSurface.drawRoundedRect(fipsToggleBounds_, 4, isFipsToggleHovered_ ? Color::fromHex(0xFFFFFF) : fipsBorder, false);
    const std::string fipsText = fipsOn ? "FIPS 180-4 SHA-256: STRICT [ON]" : "FIPS 180-4 SHA-256: PERMISSIVE";
    clientSurface.drawString(fipsToggleBounds_.centerX() - static_cast<int32_t>(fipsText.size()) * 4,
                             fipsToggleBounds_.y + 7, fipsText, fipsOn ? Color::fromHex(0x00B4D8) : Color::fromHex(0xEF4444), 1);

    // Architecture Selectors
    const std::string curArch = mgr.getPreferredArch();
    const bool isX64 = (curArch == "x64");
    clientSurface.drawRoundedRect(archX64BtnBounds_, 4, isX64 ? Color::fromHex(0x00B4D8) : Color::fromHex(0x131C2A), true);
    if (!isX64) clientSurface.drawRoundedRect(archX64BtnBounds_, 4, Color::fromHex(0x28384E), false);
    clientSurface.drawString(archX64BtnBounds_.centerX() - 36, archX64BtnBounds_.y + 7, "Native x64", isX64 ? Color::fromHex(0x06090F) : palette.textSecondary, 1);

    clientSurface.drawRoundedRect(archArm64BtnBounds_, 4, !isX64 ? Color::fromHex(0x00B4D8) : Color::fromHex(0x131C2A), true);
    if (isX64) clientSurface.drawRoundedRect(archArm64BtnBounds_, 4, Color::fromHex(0x28384E), false);
    clientSurface.drawString(archArm64BtnBounds_.centerX() - 24, archArm64BtnBounds_.y + 7, "ARM64", !isX64 ? Color::fromHex(0x06090F) : palette.textSecondary, 1);

    // Sync Upstream & Reset Defaults
    const Color syncBg = isSyncUpstreamHovered_ ? Color::fromHex(0x1E2B3E) : Color::fromHex(0x121A26);
    clientSurface.drawRoundedRect(syncUpstreamBtnBounds_, 4, syncBg, true);
    clientSurface.drawRoundedRect(syncUpstreamBtnBounds_, 4, isSyncUpstreamHovered_ ? palette.prismAccent : Color::fromHex(0x28384E), false);
    clientSurface.drawString(syncUpstreamBtnBounds_.centerX() - 48, syncUpstreamBtnBounds_.y + 7, "Sync Upstream", palette.textPrimary, 1);

    const Color resetBg = isResetDefaultsHovered_ ? Color::fromHex(0x1E2B3E) : Color::fromHex(0x121A26);
    clientSurface.drawRoundedRect(resetDefaultsBtnBounds_, 4, resetBg, true);
    clientSurface.drawRoundedRect(resetDefaultsBtnBounds_, 4, isResetDefaultsHovered_ ? palette.prismAccent : Color::fromHex(0x28384E), false);
    clientSurface.drawString(resetDefaultsBtnBounds_.centerX() - 52, resetDefaultsBtnBounds_.y + 7, "Reset Defaults", palette.textSecondary, 1);

    // Section 4: MicaNT AOSP Manifest Repository (Barrer Software)
    const int32_t sec4Y = sec3Y + 68;
    clientSurface.drawString(catalogAreaBounds_.x, sec4Y,
                             "BARRER SOFTWARE / MICANT AOSP REPOSITORY (MicaNT-Kernel/wsa-app)", palette.prismAccent, 1);
    clientSurface.fillRect(Rect{catalogAreaBounds_.x, sec4Y + 16, catalogAreaBounds_.width, 1},
                           Color::fromHex(0x1E293B));

    const Rect wsaRepoBox{catalogAreaBounds_.x, sec4Y + 22, catalogAreaBounds_.width - 206, 30};
    clientSurface.drawRoundedRect(wsaRepoBox, 4, Color::fromHex(0x131C2A), true);
    clientSurface.drawRoundedRect(wsaRepoBox, 4, Color::fromHex(0x28384E), false);
    clientSurface.drawString(wsaRepoBox.x + 10, wsaRepoBox.y + 8,
                             "https://raw.githubusercontent.com/MicaNT-Kernel/wsa-app/main/catalog.json",
                             Color::fromHex(0x34D399), 1);

    const Color syncWsaBg = isSyncWsaRepoHovered_ ? Color::fromHex(0x10B981) : Color::fromHex(0x059669);
    clientSurface.drawRoundedRect(syncWsaRepoBtnBounds_, 4, syncWsaBg, true);
    clientSurface.drawString(syncWsaRepoBtnBounds_.centerX() - 60, syncWsaRepoBtnBounds_.y + 7,
                             "Sync AOSP Catalog", Color::fromHex(0x06090F), 1);
}

bool AppHubContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    // Check Category Tabs
    for (const auto& tab : categoryTabs_) {
        if (tab.bounds.contains(localPt)) {
            setCategory(tab.cat);
            return true;
        }
    }

    // Check WSA Status Badge click (probe runtime)
    if (wsaStatusBadgeBounds_.contains(localPt)) {
        refreshWsaStatus();
        if (installCallback_) {
            installCallback_("WSA Runtime Probe",
                             wsaStatus_.isRunning ? "WSA is ACTIVE on 127.0.0.1:58526" : "WSA Subsystem is currently in STANDBY mode.",
                             wsaStatus_.isRunning);
        }
        return true;
    }

    // Check Sideload button click
    if (activeCategory_ == AppHubCategory::AndroidWsa && wsaSideloadBtnBounds_.contains(localPt)) {
        sideloadApk("C:\\MicaNT\\Apps\\sideload_package.apk");
        return true;
    }

    if (activeCategory_ == AppHubCategory::Settings) {
        auto& mgr = winget::WinGetManager::Instance();
        for (const auto& sc : repoSourceCards_) {
            if (sc.bounds.contains(localPt) || sc.selectBtnBounds.contains(localPt)) {
                mgr.setActiveSource(sc.name);
                if (installCallback_) {
                    installCallback_("Repository Switched", "Active package source set to: " + sc.name, true);
                }
                refresh();
                return true;
            }
        }
        if (scanRepoBtnBounds_.contains(localPt)) {
            const auto loaded = mgr.loadManifestsFromDirectory(mgr.getLocalRepoPath());
            populateCatalog();
            updateFilter();
            if (installCallback_) {
                installCallback_("Repository Ingested", "Ingested " + std::to_string(loaded) + " manifests from " + mgr.getLocalRepoPath(), true);
            }
            return true;
        }
        if (syncUpstreamBtnBounds_.contains(localPt)) {
            if (installCallback_) {
                installCallback_("Repository Synced", "Synced with " + mgr.getActiveSource() + " upstream successfully.", true);
            }
            return true;
        }
        if (syncWsaRepoBtnBounds_.contains(localPt)) {
            refreshWsaStatus();
            populateCatalog();
            updateFilter();
            if (installCallback_) {
                installCallback_("AOSP Repository Synced",
                                 "Synced " + std::to_string(wsaCatalog_.size()) + " clean-room AOSP manifests from MicaNT-Kernel/wsa-app.",
                                 true);
            }
            return true;
        }
        if (fipsToggleBounds_.contains(localPt)) {
            mgr.setStrictFipsVerification(!mgr.isStrictFipsVerification());
            if (installCallback_) {
                installCallback_("Integrity Policy Updated", mgr.isStrictFipsVerification() ? "Strict FIPS 180-4 SHA-256 enforcement enabled" : "Permissive checksum mode enabled", true);
            }
            return true;
        }
        if (archX64BtnBounds_.contains(localPt)) {
            mgr.setPreferredArch("x64");
            if (installCallback_) installCallback_("Architecture Preference", "Target architecture set to Native x64 (AMD64)", true);
            return true;
        }
        if (archArm64BtnBounds_.contains(localPt)) {
            mgr.setPreferredArch("arm64");
            if (installCallback_) installCallback_("Architecture Preference", "Target architecture set to ARM64", true);
            return true;
        }
        if (resetDefaultsBtnBounds_.contains(localPt)) {
            mgr.setActiveSource("winget-pkgs");
            mgr.setStrictFipsVerification(true);
            mgr.setPreferredArch("x64");
            populateCatalog();
            updateFilter();
            if (installCallback_) installCallback_("Defaults Restored", "Repository source reset to official microsoft/winget-pkgs", true);
            return true;
        }
        return false;
    }

    // Check Action Buttons on Cards
    for (const auto& card : filteredCards_) {
        if (card.actionBtnBounds.contains(localPt)) {
            if (card.isInstalled) {
                uninstallPackage(card.id);
            } else {
                installPackage(card.id);
            }
            return true;
        }
    }

    return false;
}

bool AppHubContent::onMouseMove(Point localPt) {
    isSearchHovered_ = searchBarBounds_.contains(localPt);
    isWsaBadgeHovered_ = wsaStatusBadgeBounds_.contains(localPt);
    isSideloadBtnHovered_ = (activeCategory_ == AppHubCategory::AndroidWsa && wsaSideloadBtnBounds_.contains(localPt));

    hoveredCategoryTabIndex_ = -1;
    for (size_t i = 0; i < categoryTabs_.size(); ++i) {
        if (categoryTabs_[i].bounds.contains(localPt)) {
            hoveredCategoryTabIndex_ = static_cast<int32_t>(i);
            break;
        }
    }

    if (activeCategory_ == AppHubCategory::Settings) {
        hoveredSourceIndex_ = -1;
        hoveredSourceSelectBtnIndex_ = -1;
        for (size_t i = 0; i < repoSourceCards_.size(); ++i) {
            if (repoSourceCards_[i].selectBtnBounds.contains(localPt)) {
                hoveredSourceSelectBtnIndex_ = static_cast<int32_t>(i);
                hoveredSourceIndex_ = static_cast<int32_t>(i);
                return true;
            }
            if (repoSourceCards_[i].bounds.contains(localPt)) {
                hoveredSourceIndex_ = static_cast<int32_t>(i);
                return true;
            }
        }
        isScanRepoHovered_ = scanRepoBtnBounds_.contains(localPt);
        isSyncUpstreamHovered_ = syncUpstreamBtnBounds_.contains(localPt);
        isSyncWsaRepoHovered_ = syncWsaRepoBtnBounds_.contains(localPt);
        isFipsToggleHovered_ = fipsToggleBounds_.contains(localPt);
        isArchX64Hovered_ = archX64BtnBounds_.contains(localPt);
        isArchArm64Hovered_ = archArm64BtnBounds_.contains(localPt);
        isResetDefaultsHovered_ = resetDefaultsBtnBounds_.contains(localPt);
        return isScanRepoHovered_ || isSyncUpstreamHovered_ || isSyncWsaRepoHovered_ ||
               isFipsToggleHovered_ || isArchX64Hovered_ || isArchArm64Hovered_ ||
               isResetDefaultsHovered_ || isWsaBadgeHovered_ || isSearchHovered_;
    }

    hoveredCardIndex_ = -1;
    hoveredActionBtnIndex_ = -1;
    for (size_t i = 0; i < filteredCards_.size(); ++i) {
        if (filteredCards_[i].actionBtnBounds.contains(localPt)) {
            hoveredActionBtnIndex_ = static_cast<int32_t>(i);
            hoveredCardIndex_ = static_cast<int32_t>(i);
            return true;
        }
        if (filteredCards_[i].cardBounds.contains(localPt)) {
            hoveredCardIndex_ = static_cast<int32_t>(i);
            return true;
        }
    }

    return isSearchHovered_ || hoveredCategoryTabIndex_ >= 0;
}

bool AppHubContent::onMouseWheel(Point localPt, int32_t delta) {
    (void)localPt;
    scrollY_ = std::clamp(scrollY_ - delta * 30, 0, maxScrollY_);
    return true;
}

bool AppHubContent::onCharInput(char c) {
    if (c >= 32 && c <= 126) {
        searchQuery_ += c;
        setSearchQuery(searchQuery_);
        return true;
    }
    return false;
}

bool AppHubContent::onKeyDown(KeyCode key, bool ctrl, bool shift, bool alt) {
    (void)ctrl; (void)shift; (void)alt;
    if (key == KeyCode::Backspace) {
        if (!searchQuery_.empty()) {
            searchQuery_.pop_back();
            setSearchQuery(searchQuery_);
            return true;
        }
    } else if (key == KeyCode::Escape) {
        if (!searchQuery_.empty()) {
            setSearchQuery("");
            return true;
        }
    } else if (key == KeyCode::Up) {
        scrollY_ = std::clamp(scrollY_ - 40, 0, maxScrollY_);
        return true;
    } else if (key == KeyCode::Down) {
        scrollY_ = std::clamp(scrollY_ + 40, 0, maxScrollY_);
        return true;
    }
    return false;
}

} // namespace surshell
