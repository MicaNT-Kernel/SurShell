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
        { AppHubCategory::DeveloperTools, "Developer Tools", Rect{} },
        { AppHubCategory::SystemUtilities, "System Utilities", Rect{} },
        { AppHubCategory::MediaDocs, "Media & Docs", Rect{} },
        { AppHubCategory::Installed, "Installed", Rect{} }
    };

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

    std::string q = searchQuery_;
    std::transform(q.begin(), q.end(), q.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    for (const auto& card : allCards_) {
        // Category filtering
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

    // Layout Category Tabs
    int32_t tabX = categoryTabsBounds_.x;
    for (auto& tab : categoryTabs_) {
        const int32_t tabW = static_cast<int32_t>(tab.label.size()) * 8 + 20;
        tab.bounds = Rect{tabX, categoryTabsBounds_.y, tabW, 26};
        tabX += tabW + 8;
    }

    // Layout Cards Grid in Catalog Area
    constexpr int32_t cardGap = 12;
    constexpr int32_t cardH = 96;
    const int32_t cols = (catalogAreaBounds_.width > 700) ? 2 : 1;
    const int32_t cardW = (catalogAreaBounds_.width - (cols - 1) * cardGap) / cols;

    for (size_t i = 0; i < filteredCards_.size(); ++i) {
        const int32_t row = static_cast<int32_t>(i) / cols;
        const int32_t col = static_cast<int32_t>(i) % cols;

        const int32_t cx = catalogAreaBounds_.x + col * (cardW + cardGap);
        const int32_t cy = catalogAreaBounds_.y + row * (cardH + cardGap) - scrollY_;

        auto& card = filteredCards_[i];
        card.cardBounds = Rect{cx, cy, cardW, cardH};

        // Action button bounds (bottom-right of card)
        constexpr int32_t btnW = 96;
        constexpr int32_t btnH = 26;
        card.actionBtnBounds = Rect{cx + cardW - btnW - 12, cy + cardH - btnH - 12, btnW, btnH};
    }

    const int32_t totalRows = (static_cast<int32_t>(filteredCards_.size()) + cols - 1) / cols;
    const int32_t contentTotalH = totalRows * (cardH + cardGap);
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

    const Rect verBadge{330, 16, 110, 20};
    clientSurface.drawRoundedRect(verBadge, 4, Color::fromHex(0x132238), true);
    clientSurface.drawRoundedRect(verBadge, 4, Color::fromHex(0x284266), false);
    clientSurface.drawString(verBadge.x + 8, verBadge.y + 4, "winget v1.6.0", Color::fromHex(0x7DD3FC), 1);

    // Sovereign Source Status Badge (top right)
    const std::string mirrorText = "Clean-Room FIPS 180-4  |  Sovereign Mirror";
    const int32_t mirrorW = static_cast<int32_t>(mirrorText.size()) * 8;
    clientSurface.drawString(width - mirrorW - 18, 18, mirrorText, Color::fromHex(0x38BDF8), 1);

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

        // 100% NT Retail Badge
        if (card.isRetailCertified) {
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
        } else {
            // Not installed -> Show Install Button
            const Color btnBg = isBtnHov ? Color::fromHex(0x48CAE4) : Color::fromHex(0x00B4D8);
            clientSurface.drawRoundedRect(card.actionBtnBounds, 6, btnBg, true);

            const std::string btnLabel = "Install";
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

bool AppHubContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    // Check Category Tabs
    for (const auto& tab : categoryTabs_) {
        if (tab.bounds.contains(localPt)) {
            setCategory(tab.cat);
            return true;
        }
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

    hoveredCategoryTabIndex_ = -1;
    for (size_t i = 0; i < categoryTabs_.size(); ++i) {
        if (categoryTabs_[i].bounds.contains(localPt)) {
            hoveredCategoryTabIndex_ = static_cast<int32_t>(i);
            break;
        }
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
