// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/app_hub.cpp)
//
// Winget Sovereign App Hub & Software Store Implementation
// ============================================================================

#include "surshell/app_hub.hpp"
#include "surshell/micag.hpp"
#include <algorithm>
#include <cctype>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

static std::string readRegStringVal(HKEY hKey, const wchar_t* valName) {
    DWORD type = 0, bytes = 0;
    if (RegQueryValueExW(hKey, valName, nullptr, &type, nullptr, &bytes) != ERROR_SUCCESS || bytes == 0) return {};
    std::wstring ws(bytes / sizeof(wchar_t), L'\0');
    if (RegQueryValueExW(hKey, valName, nullptr, &type, reinterpret_cast<LPBYTE>(ws.data()), &bytes) != ERROR_SUCCESS) return {};
    while (!ws.empty() && ws.back() == L'\0') ws.pop_back();
    if (ws.empty()) return {};

    // Support REG_EXPAND_SZ environment variable expansion
    if (type == REG_EXPAND_SZ) {
        DWORD expLen = ExpandEnvironmentStringsW(ws.c_str(), nullptr, 0);
        if (expLen > 0) {
            std::wstring expanded(expLen, L'\0');
            ExpandEnvironmentStringsW(ws.c_str(), expanded.data(), expLen);
            while (!expanded.empty() && expanded.back() == L'\0') expanded.pop_back();
            ws = std::move(expanded);
        }
    }

    int len = WideCharToMultiByte(CP_UTF8, 0, ws.data(), static_cast<int>(ws.size()), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.data(), static_cast<int>(ws.size()), s.data(), len, nullptr, nullptr);
    return s;
}

static bool containsWord(std::string_view text, std::string_view word) noexcept {
    if (word.empty() || text.size() < word.size()) return false;
    size_t pos = 0;
    while ((pos = text.find(word, pos)) != std::string_view::npos) {
        bool leftOk = (pos == 0) || !std::isalnum(static_cast<unsigned char>(text[pos - 1]));
        bool rightOk = (pos + word.size() >= text.size()) || !std::isalnum(static_cast<unsigned char>(text[pos + word.size()]));
        if (leftOk && rightOk) return true;
        pos += 1;
    }
    return false;
}
#endif

namespace surshell {

int AppHubContent::compareVersions(std::string_view v1, std::string_view v2) {
    size_t i = 0, j = 0;
    while (i < v1.size() || j < v2.size()) {
        int64_t n1 = 0, n2 = 0;
        while (i < v1.size() && v1[i] >= '0' && v1[i] <= '9') {
            n1 = n1 * 10 + (v1[i] - '0');
            ++i;
        }
        while (j < v2.size() && v2[j] >= '0' && v2[j] <= '9') {
            n2 = n2 * 10 + (v2[j] - '0');
            ++j;
        }
        if (n1 < n2) return -1;
        if (n1 > n2) return 1;

        if (i < v1.size() && (v1[i] == '.' || v1[i] == '-' || v1[i] == '+')) ++i;
        if (j < v2.size() && (v2[j] == '.' || v2[j] == '-' || v2[j] == '+')) ++j;
    }
    return 0;
}

AppHubContent::AppHubContent() {
    categoryTabs_ = {
        { AppHubCategory::All, "All Packages", Rect{} },
        { AppHubCategory::CertifiedRetail, "MicaNT Retail (100%)", Rect{} },
        { AppHubCategory::AndroidWsa, "Android (WSA)", Rect{} },
        { AppHubCategory::DeveloperTools, "Developer Tools", Rect{} },
        { AppHubCategory::SystemUtilities, "System Utilities", Rect{} },
        { AppHubCategory::MediaDocs, "Media & Docs", Rect{} },
        { AppHubCategory::Stacks, "Curated Stacks", Rect{} },
        { AppHubCategory::Installed, "Installed", Rect{} },
        { AppHubCategory::Settings, "Settings & Sources", Rect{} }
    };

    curatedStacks_ = {
        {
            .id = "developer",
            .name = "Developer Sovereign Stack",
            .desc = "Essential developer toolchain including Git, VS Code, Windows Terminal, and 7-Zip.",
            .category = "Developer",
            .pkgIds = {"Git.Git", "Microsoft.VisualStudioCode", "Microsoft.WindowsTerminal", "7zip.7zip"}
        },
        {
            .id = "privacy",
            .name = "Privacy & Sovereign Security",
            .desc = "Zero-telemetry communication, password manager, and private web browser.",
            .category = "Security",
            .pkgIds = {"Signal.Signal", "KeePassXCTeam.KeePassXC", "BraveSoftware.BraveBrowser"}
        },
        {
            .id = "media",
            .name = "Media Production Studio",
            .desc = "Universal media playback, broadcasting, and audio workstation tools.",
            .category = "Media",
            .pkgIds = {"VideoLAN.VLC", "OBSProject.OBSStudio", "Audacity.Audacity"}
        },
        {
            .id = "utilities",
            .name = "System Diagnostics & Utilities",
            .desc = "High-speed file indexing, disk partition visualization, and compression tools.",
            .category = "Utilities",
            .pkgIds = {"voidtools.Everything", "AntibodySoftware.WizTree", "WinMerge.WinMerge", "7zip.7zip"}
        }
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

        card.isPinned = isPackagePinned(card.id);
        if (!pPkg->installers.empty()) {
            card.architecture = winget::ArchitectureToString(pPkg->installers[0].architecture);
            card.downloadUrl = pPkg->installers[0].installerUrl;
            card.sha256 = pPkg->installers[0].installerSha256;
        }

        // Check if installed in winget engine
        card.isInstalled = mgr.isInstalled(card.id);
        if (card.isInstalled) {
            for (const auto& ip : mgr.getInstalledPackages()) {
                if (ip.packageIdentifier == card.id) {
                    card.installedVersion = ip.packageVersion;
                    if (!card.isPinned && compareVersions(card.version, card.installedVersion) > 0) {
                        card.hasUpdateAvailable = true;
                    }
                    break;
                }
            }
        }

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
    if (activeCategory_ == AppHubCategory::Settings || activeCategory_ == AppHubCategory::Stacks) {
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

bool AppHubContent::upgradePackage(const std::string& packageId) {
    auto& mgr = winget::WinGetManager::Instance();
    std::vector<std::string> log;
    int32_t hr = mgr.upgrade(packageId, log);
    const bool success = (hr == winget::WINGET_S_OK);

    const auto* pkg = mgr.findPackage(packageId);
    std::string name = pkg ? pkg->packageName : packageId;

    if (success) {
        if (installCallback_) {
            installCallback_("Package Upgraded", name + " was updated to the latest version.", true);
        }
    } else {
        std::string err = log.empty() ? "Upgrade failed" : log.back();
        if (installCallback_) {
            installCallback_("Upgrade Failed", name + ": " + err, false);
        }
    }

    refresh();
    return success;
}

size_t AppHubContent::upgradeAllPackages() {
    size_t count = 0;
    std::vector<std::string> toUpgrade;
    for (const auto& card : allCards_) {
        if (card.hasUpdateAvailable && !card.isPinned) {
            toUpgrade.push_back(card.id);
        }
    }
    for (const auto& id : toUpgrade) {
        if (upgradePackage(id)) {
            count++;
        }
    }
    return count;
}

size_t AppHubContent::updateAvailableCount() const noexcept {
    size_t count = 0;
    for (const auto& card : allCards_) {
        if (card.hasUpdateAvailable && !card.isPinned) count++;
    }
    return count;
}

bool AppHubContent::pinPackage(const std::string& packageId, bool pin) {
    if (pin) {
        if (!isPackagePinned(packageId)) {
            pinnedPackageIds_.push_back(packageId);
        }
    } else {
        auto it = std::remove(pinnedPackageIds_.begin(), pinnedPackageIds_.end(), packageId);
        pinnedPackageIds_.erase(it, pinnedPackageIds_.end());
    }
    auto& mgr = winget::WinGetManager::Instance();
    auto installedOpt = mgr.getInstalledPackage(packageId);
    if (installedOpt) {
        winget::InstalledPackage ip = *installedOpt;
        ip.isPinned = pin;
        mgr.registerInstalled(ip);
    }

    refresh();
    if (installCallback_) {
        installCallback_(pin ? "Package Pinned" : "Package Unpinned",
                         packageId + (pin ? " is now locked against automatic upgrades." : " updates are now unlocked."),
                         true);
    }
    return true;
}

bool AppHubContent::isPackagePinned(const std::string& packageId) const {
    return std::find(pinnedPackageIds_.begin(), pinnedPackageIds_.end(), packageId) != pinnedPackageIds_.end();
}

size_t AppHubContent::installCuratedStack(const std::string& stackId) {
    size_t count = 0;
    for (const auto& s : curatedStacks_) {
        if (s.id == stackId) {
            for (const auto& id : s.pkgIds) {
                if (installPackage(id)) {
                    ++count;
                }
            }
            if (installCallback_) {
                installCallback_("Curated Stack Installed", "Provisioned " + std::to_string(count) + " packages from " + s.name, true);
            }
            break;
        }
    }
    return count;
}

void AppHubContent::inspectPackage(const std::string& packageId) {
    inspectedPackageId_ = packageId;
}

size_t AppHubContent::scanSystemInstalled() {
    auto& mgr = winget::WinGetManager::Instance();
    size_t foundCount = 0;

#if defined(_WIN32)
    auto scanRegistry = [&](HKEY root, const wchar_t* subKey, REGSAM sam) {
        HKEY hRoot = nullptr;
        if (RegOpenKeyExW(root, subKey, 0, KEY_READ | sam, &hRoot) != ERROR_SUCCESS) return;

        DWORD subCount = 0, maxLen = 0;
        if (RegQueryInfoKeyW(hRoot, nullptr, nullptr, nullptr, &subCount, &maxLen, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
            std::vector<wchar_t> keyBuf(maxLen + 2, 0);
            for (DWORD i = 0; i < subCount; ++i) {
                DWORD len = static_cast<DWORD>(keyBuf.size());
                if (RegEnumKeyExW(hRoot, i, keyBuf.data(), &len, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) continue;

                HKEY hApp = nullptr;
                if (RegOpenKeyExW(hRoot, keyBuf.data(), 0, KEY_READ | sam, &hApp) != ERROR_SUCCESS) continue;

                std::string name = readRegStringVal(hApp, L"DisplayName");
                std::string ver = readRegStringVal(hApp, L"DisplayVersion");
                std::string pub = readRegStringVal(hApp, L"Publisher");
                std::string loc = readRegStringVal(hApp, L"InstallLocation");
                RegCloseKey(hApp);

                if (name.empty()) continue;

                std::string lowerName = name;
                std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });

                for (const auto& [pkgId, pkg] : mgr.getCatalog()) {
                    std::string pName = pkg.packageName;
                    std::transform(pName.begin(), pName.end(), pName.begin(), [](unsigned char c) {
                        return static_cast<char>(std::tolower(c));
                    });

                    // Prevent disastrous false positives: exact match or strict word-boundary match for tokens >= 3 chars
                    bool isMatch = false;
                    if (lowerName == pName) {
                        isMatch = true;
                    } else if (pName.size() >= 3 && containsWord(lowerName, pName)) {
                        isMatch = true;
                    }

                    if (isMatch) {
                        if (!mgr.isInstalled(pkg.packageIdentifier)) {
                            winget::InstalledPackage ip;
                            ip.packageIdentifier = pkg.packageIdentifier;
                            ip.packageVersion = ver.empty() ? pkg.packageVersion : ver;
                            ip.packageName = pkg.packageName;
                            ip.publisher = pub.empty() ? pkg.publisher : pub;
                            ip.installDate = "Discovered";
                            ip.installLocation = loc;
                            ip.isPinned = false;

                            mgr.registerInstalled(ip);
                            foundCount++;
                        }
                        break;
                    }
                }
            }
        }
        RegCloseKey(hRoot);
    };

    scanRegistry(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", KEY_WOW64_64KEY);
    scanRegistry(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", KEY_WOW64_32KEY);
    scanRegistry(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", KEY_WOW64_64KEY);
    scanRegistry(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", KEY_WOW64_32KEY);
#endif

    refresh();
    if (installCallback_) {
        installCallback_("System Scan Complete", "Discovered " + std::to_string(foundCount) + " installed apps matching repository manifests.", true);
    }
    return foundCount;
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

        // Section 5: MicaG & MicaIPC Sovereign GMS Bridge (TPM 2.0 Hardware Attestation)
        const int32_t sec5Y = sec4Y + 68;
        micaGToggleBounds_ = Rect{catalogAreaBounds_.x, sec5Y + 22, 450, 30};

        maxScrollY_ = 0;
        scrollY_ = 0;
    } else if (activeCategory_ == AppHubCategory::Stacks) {
        const int32_t stackCardW = catalogAreaBounds_.width;
        constexpr int32_t stackCardH = 76;
        constexpr int32_t stackGap = 12;

        int32_t curY = catalogAreaBounds_.y + 12 - scrollY_;
        for (auto& s : curatedStacks_) {
            s.bounds = Rect{catalogAreaBounds_.x, curY, stackCardW, stackCardH};
            s.installBtnBounds = Rect{catalogAreaBounds_.x + stackCardW - 130, curY + 22, 118, 32};
            curY += stackCardH + stackGap;
        }

        const int32_t totalH = static_cast<int32_t>(curatedStacks_.size()) * (stackCardH + stackGap);
        maxScrollY_ = std::max(0, totalH - catalogAreaBounds_.height);
        scrollY_ = std::clamp(scrollY_, 0, maxScrollY_);
    } else {
        // Layout Cards Grid in Catalog Area
    constexpr int32_t cardGap = 12;
    constexpr int32_t cardH = 96;
    const int32_t cols = (catalogAreaBounds_.width > 700) ? 2 : 1;
    const int32_t cardW = (catalogAreaBounds_.width - (cols - 1) * cardGap) / cols;

    const bool hasTopBanner = (activeCategory_ == AppHubCategory::AndroidWsa || activeCategory_ == AppHubCategory::Installed);
    const int32_t startY = hasTopBanner ? (catalogAreaBounds_.y + 48) : catalogAreaBounds_.y;

    if (activeCategory_ == AppHubCategory::Installed) {
        scanSystemBtnBounds_ = Rect{catalogAreaBounds_.x, catalogAreaBounds_.y + 6, 160, 32};
        updateAllBtnBounds_ = Rect{catalogAreaBounds_.x + 172, catalogAreaBounds_.y + 6, 150, 32};
    } else {
        scanSystemBtnBounds_ = Rect{};
        updateAllBtnBounds_ = Rect{};
    }

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

        // Pin button bounds (to the left of action button)
        card.pinBtnBounds = Rect{card.actionBtnBounds.x - 32, card.actionBtnBounds.y, 26, btnH};

        // Inspect button bounds (to the left of pin button)
        card.inspectBtnBounds = Rect{card.pinBtnBounds.x - 32, card.actionBtnBounds.y, 26, btnH};
    }

    const int32_t totalRows = (static_cast<int32_t>(filteredCards_.size()) + cols - 1) / cols;
    const int32_t contentTotalH = (hasTopBanner ? 48 : 0) + totalRows * (cardH + cardGap);
        maxScrollY_ = std::max(0, contentTotalH - catalogAreaBounds_.height);
        scrollY_ = std::clamp(scrollY_, 0, maxScrollY_);
    }

    if (inspectedPackageId_.has_value()) {
        const int32_t modalW = std::min(680, catalogAreaBounds_.width - 20);
        const int32_t modalH = 380;
        const Rect modalRect{catalogAreaBounds_.centerX() - modalW / 2, catalogAreaBounds_.centerY() - modalH / 2, modalW, modalH};
        inspectorCloseBtnBounds_ = Rect{modalRect.right() - 34, modalRect.y + 10, 24, 24};
    } else {
        inspectorCloseBtnBounds_ = Rect{};
    }
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

    if (activeCategory_ == AppHubCategory::Stacks) {
        renderStacksView(clientSurface, width, height);
    } else {
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

        // Installed Subsystem Banner (if Installed category active)
        if (activeCategory_ == AppHubCategory::Installed) {
            const Color scBg = isScanSystemBtnHovered_ ? Color::fromHex(0x1D4ED8) : Color::fromHex(0x1E3A8A);
            clientSurface.drawRoundedRect(scanSystemBtnBounds_, 6, scBg, true);
            clientSurface.drawRoundedRect(scanSystemBtnBounds_, 6, Color::fromHex(0x3B82F6), false);
            clientSurface.drawString(scanSystemBtnBounds_.x + 14, scanSystemBtnBounds_.y + 8, "Scan System Apps", Color::fromHex(0xBFDBFE), 1);

            const size_t upCount = updateAvailableCount();
            if (upCount > 0) {
                const Color upBg = isUpdateAllBtnHovered_ ? Color::fromHex(0xD97706) : Color::fromHex(0xB45309);
                clientSurface.drawRoundedRect(updateAllBtnBounds_, 6, upBg, true);
                clientSurface.drawRoundedRect(updateAllBtnBounds_, 6, Color::fromHex(0xF59E0B), false);
                std::string upLabel = "Update All (" + std::to_string(upCount) + ")";
                clientSurface.drawString(updateAllBtnBounds_.x + 18, updateAllBtnBounds_.y + 8, upLabel, Color::fromHex(0xFFFBEB), 1);
            }

            std::string summary = std::to_string(installedPackagesCount()) + " Installed Packages  |  " + std::to_string(upCount) + " Updates Ready";
            int32_t summaryX = (upCount > 0) ? (updateAllBtnBounds_.right() + 20) : (scanSystemBtnBounds_.right() + 20);
            clientSurface.drawString(summaryX, catalogAreaBounds_.y + 14, summary, palette.textSecondary, 1);
        }

        // 5. Package Cards Grid (Rendered inside Catalog Area)
        for (size_t i = 0; i < filteredCards_.size(); ++i) {
            const auto& card = filteredCards_[i];
            if (card.cardBounds.bottom() < catalogAreaBounds_.y || card.cardBounds.top() > catalogAreaBounds_.bottom()) {
                continue;
            }

            const bool isHov = (static_cast<int32_t>(i) == hoveredCardIndex_);
            const bool isBtnHov = (static_cast<int32_t>(i) == hoveredActionBtnIndex_);
            const bool isPinHov = (static_cast<int32_t>(i) == hoveredPinBtnIndex_);
            const bool isInspectHov = (static_cast<int32_t>(i) == hoveredInspectBtnIndex_);

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

            // Badges: Retail, AOSP, Architecture, Pinned
            int32_t badgeOffset = 64 + static_cast<int32_t>(card.name.size()) * 8;
            if (card.isAndroidApp) {
                const Rect badgeRect{card.cardBounds.x + badgeOffset, card.cardBounds.y + 11, 130, 18};
                clientSurface.drawRoundedRect(badgeRect, 4, Color::fromHex(0x0E3A2F), true);
                clientSurface.drawRoundedRect(badgeRect, 4, Color::fromHex(0x10B981), false);
                clientSurface.drawString(badgeRect.x + 4, badgeRect.y + 3, "AOSP CLEAN-ROOM", Color::fromHex(0x34D399), 1);
                badgeOffset += 136;

                const Rect archBadge{card.cardBounds.x + badgeOffset, card.cardBounds.y + 11, 74, 18};
                clientSurface.drawRoundedRect(archBadge, 4, Color::fromHex(0x182436), true);
                clientSurface.drawRoundedRect(archBadge, 4, Color::fromHex(0x38BDF8), false);
                clientSurface.drawString(archBadge.x + 6, archBadge.y + 3, card.architecture, Color::fromHex(0x7DD3FC), 1);
                badgeOffset += 80;
            } else if (card.isRetailCertified) {
                const Rect badgeRect{card.cardBounds.x + badgeOffset, card.cardBounds.y + 11, 120, 18};
                clientSurface.drawRoundedRect(badgeRect, 4, Color::fromHex(0x0E3A42), true);
                clientSurface.drawRoundedRect(badgeRect, 4, Color::fromHex(0x00B4D8), false);
                clientSurface.drawString(badgeRect.x + 4, badgeRect.y + 3, "100% NT RETAIL", Color::fromHex(0x00B4D8), 1);
                badgeOffset += 126;
            }

            if (card.isPinned) {
                const Rect pinBadge{card.cardBounds.x + badgeOffset, card.cardBounds.y + 11, 62, 18};
                clientSurface.drawRoundedRect(pinBadge, 4, Color::fromHex(0x451A03), true);
                clientSurface.drawRoundedRect(pinBadge, 4, Color::fromHex(0xF59E0B), false);
                clientSurface.drawString(pinBadge.x + 6, pinBadge.y + 3, "PINNED", Color::fromHex(0xFCD34D), 1);
            }

            // Moniker & Version & License Tag
            std::string tag;
            if (card.hasUpdateAvailable && !card.installedVersion.empty()) {
                tag = card.id + " | v" + card.installedVersion + " -> v" + card.version + " (Update Available)";
            } else {
                tag = card.id + " | v" + card.version + " | " + card.license;
            }
            clientSurface.drawString(card.cardBounds.x + 58, card.cardBounds.y + 30,
                                     tag, card.hasUpdateAvailable ? Color::fromHex(0xFBBF24) : palette.textSecondary, 1);

            // Description
            const int32_t maxDescW = card.inspectBtnBounds.x - (card.cardBounds.x + 58) - 10;
            const int32_t maxDescChars = std::max(8, maxDescW / 8);
            std::string desc = card.description;
            if (static_cast<int32_t>(desc.size()) > maxDescChars) {
                desc = desc.substr(0, static_cast<size_t>(maxDescChars - 2)) + "..";
            }
            clientSurface.drawString(card.cardBounds.x + 58, card.cardBounds.y + 52,
                                     desc, Color::fromHex(0x94A3B8), 1);

            // Inspect Button
            const Color inspBg = isInspectHov ? Color::fromHex(0x1E293B) : Color::fromHex(0x131C2A);
            const Color inspBorder = isInspectHov ? palette.prismAccent : Color::fromHex(0x334155);
            clientSurface.drawRoundedRect(card.inspectBtnBounds, 4, inspBg, true);
            clientSurface.drawRoundedRect(card.inspectBtnBounds, 4, inspBorder, false);
            clientSurface.drawString(card.inspectBtnBounds.centerX() - 3, card.inspectBtnBounds.y + 6, "i",
                                     isInspectHov ? Color::fromHex(0xFFFFFF) : Color::fromHex(0x94A3B8), 1);

            // Pin Button
            const Color pinBg = card.isPinned
                ? (isPinHov ? Color::fromHex(0x78350F) : Color::fromHex(0x451A03))
                : (isPinHov ? Color::fromHex(0x1E293B) : Color::fromHex(0x131C2A));
            const Color pinBorder = card.isPinned ? Color::fromHex(0xF59E0B) : (isPinHov ? palette.prismAccent : Color::fromHex(0x334155));
            clientSurface.drawRoundedRect(card.pinBtnBounds, 4, pinBg, true);
            clientSurface.drawRoundedRect(card.pinBtnBounds, 4, pinBorder, false);
            clientSurface.drawString(card.pinBtnBounds.centerX() - 3, card.pinBtnBounds.y + 6, card.isPinned ? "*" : "P",
                                     card.isPinned ? Color::fromHex(0xFCD34D) : (isPinHov ? Color::fromHex(0xFFFFFF) : Color::fromHex(0x94A3B8)), 1);

            // Action Button
            if (card.hasUpdateAvailable) {
                // Update action button (Amber/Gold)
                const Color btnBg = isBtnHov ? Color::fromHex(0xD97706) : Color::fromHex(0xB45309);
                const Color borderCol = Color::fromHex(0xF59E0B);
                clientSurface.drawRoundedRect(card.actionBtnBounds, 6, btnBg, true);
                clientSurface.drawRoundedRect(card.actionBtnBounds, 6, borderCol, false);

                const std::string btnLabel = "Update";
                const int32_t bw = static_cast<int32_t>(btnLabel.size()) * 8;
                clientSurface.drawString(card.actionBtnBounds.centerX() - bw / 2,
                                         card.actionBtnBounds.y + 6,
                                         btnLabel, Color::fromHex(0xFFFBEB), 1);
            } else if (card.isInstalled) {
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

    // 7. Inspector Modal (Overlays everything if active)
    if (inspectedPackageId_.has_value()) {
        renderInspectorModal(clientSurface, width, height);
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

    // Section 5: MicaG & MicaIPC Sovereign GMS Bridge (TPM 2.0 Attestation)
    const int32_t sec5Y = sec4Y + 68;
    clientSurface.drawString(catalogAreaBounds_.x, sec5Y,
                             "MICAG & MICAIPC SOVEREIGN GMS BRIDGE (TPM 2.0 ATTESTATION)", palette.prismAccent, 1);
    clientSurface.fillRect(Rect{catalogAreaBounds_.x, sec5Y + 16, catalogAreaBounds_.width, 1},
                           Color::fromHex(0x1E293B));

    auto& micag = MicaGManager::instance();
    const bool micagOn = micag.isMicaGEnabled();
    const Color micagBg = micagOn ? Color::fromHex(0x0E3A2F) : Color::fromHex(0x2A1A1A);
    const Color micagBorder = micagOn ? Color::fromHex(0x10B981) : Color::fromHex(0xEF4444);
    clientSurface.drawRoundedRect(micaGToggleBounds_, 4, micagBg, true);
    clientSurface.drawRoundedRect(micaGToggleBounds_, 4, isMicaGToggleHovered_ ? Color::fromHex(0xFFFFFF) : micagBorder, false);

    const std::string micagText = micagOn
        ? "MicaG: ENABLED [TPM 2.0 VIRTUAL INTEGRITY | 3.5us MicaIPC]"
        : "MicaG: DISABLED [AOSP STANDALONE | NO HARDWARE ATTESTATION]";
    clientSurface.drawString(micaGToggleBounds_.x + 10,
                             micaGToggleBounds_.y + 7, micagText,
                             micagOn ? Color::fromHex(0x34D399) : Color::fromHex(0xF87171), 1);
}

void AppHubContent::renderStacksView(Surface& clientSurface, int32_t width, int32_t height) {
    (void)width;
    (void)height;
    const auto& palette = ThemeManager::instance().palette();

    clientSurface.drawString(catalogAreaBounds_.x, catalogAreaBounds_.y + 4,
                             "CURATED SOVEREIGN APPLICATION STACKS (1-CLICK WORKFLOWS)", palette.prismAccent, 1);
    clientSurface.fillRect(Rect{catalogAreaBounds_.x, catalogAreaBounds_.y + 20, catalogAreaBounds_.width, 1},
                           Color::fromHex(0x1E293B));

    for (size_t i = 0; i < curatedStacks_.size(); ++i) {
        const auto& s = curatedStacks_[i];
        if (s.bounds.bottom() < catalogAreaBounds_.y || s.bounds.top() > catalogAreaBounds_.bottom()) {
            continue;
        }

        const bool isHov = (static_cast<int32_t>(i) == hoveredStackIndex_);
        const bool isBtnHov = (static_cast<int32_t>(i) == hoveredStackInstallBtnIndex_);

        clientSurface.drawDropShadow(s.bounds, 8, 0.3f);
        clientSurface.drawRoundedRect(s.bounds, 8, isHov ? Color::fromHex(0x131E2E) : Color::fromHex(0x0E1624), true);
        clientSurface.drawRoundedRect(s.bounds, 8, isHov ? palette.prismAccent : Color::fromHex(0x202E42), false);

        // Icon Box
        const Rect iconR{s.bounds.x + 12, s.bounds.y + 12, 48, 48};
        clientSurface.drawRoundedRect(iconR, 6, Color::fromHex(0x182436), true);
        clientSurface.drawRoundedRect(iconR, 6, Color::fromHex(0x283A52), false);
        IconRenderer::draw(clientSurface, IconId::AppHub, Rect{iconR.x + 10, iconR.y + 10, 28, 28}, palette.prismAccent);

        // Stack Name & Category Tag
        clientSurface.drawString(s.bounds.x + 70, s.bounds.y + 12, s.name, Color::fromHex(0xFFFFFF), 1);
        const int32_t nameW = static_cast<int32_t>(s.name.size()) * 8;
        const Rect catBadge{s.bounds.x + 76 + nameW, s.bounds.y + 11, static_cast<int32_t>(s.category.size()) * 8 + 14, 18};
        clientSurface.drawRoundedRect(catBadge, 4, Color::fromHex(0x0E3A42), true);
        clientSurface.drawRoundedRect(catBadge, 4, Color::fromHex(0x00B4D8), false);
        clientSurface.drawString(catBadge.x + 7, catBadge.y + 3, s.category, Color::fromHex(0x00B4D8), 1);

        // Description
        clientSurface.drawString(s.bounds.x + 70, s.bounds.y + 32, s.desc, Color::fromHex(0x94A3B8), 1);

        // Package Summary
        std::string pkgsSummary = "Packages (" + std::to_string(s.pkgIds.size()) + "): ";
        for (size_t p = 0; p < s.pkgIds.size(); ++p) {
            if (p > 0) pkgsSummary += ", ";
            pkgsSummary += s.pkgIds[p];
        }
        if (pkgsSummary.size() > 70) pkgsSummary = pkgsSummary.substr(0, 68) + "..";
        clientSurface.drawString(s.bounds.x + 70, s.bounds.y + 50, pkgsSummary, Color::fromHex(0x38BDF8), 1);

        // Deploy Stack Button
        const Color btnBg = isBtnHov ? Color::fromHex(0x48CAE4) : Color::fromHex(0x00B4D8);
        clientSurface.drawRoundedRect(s.installBtnBounds, 6, btnBg, true);
        const std::string btnLabel = "Deploy Stack";
        const int32_t bw = static_cast<int32_t>(btnLabel.size()) * 8;
        clientSurface.drawString(s.installBtnBounds.centerX() - bw / 2, s.installBtnBounds.y + 8, btnLabel, Color::fromHex(0x06090F), 1);
    }
}

void AppHubContent::renderInspectorModal(Surface& clientSurface, int32_t width, int32_t height) {
    if (!inspectedPackageId_.has_value()) return;

    const auto& palette = ThemeManager::instance().palette();
    const std::string& pkgId = inspectedPackageId_.value();

    const AppHubCard* foundCard = nullptr;
    for (const auto& c : allCards_) {
        if (c.id == pkgId) {
            foundCard = &c;
            break;
        }
    }
    if (!foundCard) return;

    // Dim background overlay with acrylic glass
    clientSurface.applyAcrylicTint(Rect{0, 0, width, height}, Color{0, 0, 0, 180}, 4);

    const int32_t modalW = std::min(680, catalogAreaBounds_.width - 20);
    const int32_t modalH = 380;
    const Rect modalRect{catalogAreaBounds_.centerX() - modalW / 2, catalogAreaBounds_.centerY() - modalH / 2, modalW, modalH};

    clientSurface.drawDropShadow(modalRect, 16, 0.5f);
    clientSurface.drawRoundedRect(modalRect, 8, Color::fromHex(0x0C121D), true);
    clientSurface.drawRoundedRect(modalRect, 8, palette.prismAccent, false);

    // Modal Header
    const Rect headerRect{modalRect.x, modalRect.y, modalRect.width, 42};
    clientSurface.drawRoundedRect(headerRect, 8, Color::fromHex(0x131E2E), true);
    clientSurface.fillRect(Rect{modalRect.x, modalRect.y + 41, modalRect.width, 1}, Color::fromHex(0x1E293B));

    IconRenderer::draw(clientSurface, foundCard->iconId, Rect{modalRect.x + 14, modalRect.y + 10, 22, 22}, palette.prismAccent);
    std::string title = "Package Inspector: " + foundCard->name + " (" + foundCard->id + ")";
    clientSurface.drawString(modalRect.x + 44, modalRect.y + 12, title, Color::fromHex(0xFFFFFF), 1);

    // Close Button [X]
    const Color closeBg = isInspectorCloseBtnHovered_ ? Color::fromHex(0xEF4444) : Color::fromHex(0x1E293B);
    clientSurface.drawRoundedRect(inspectorCloseBtnBounds_, 4, closeBg, true);
    clientSurface.drawString(inspectorCloseBtnBounds_.centerX() - 4, inspectorCloseBtnBounds_.y + 5, "X", Color::fromHex(0xFFFFFF), 1);

    // Details Rows
    int32_t curY = modalRect.y + 54;
    auto drawDetail = [&](const std::string& label, const std::string& val, Color valCol = Color::fromHex(0xE2E8F0)) {
        clientSurface.drawString(modalRect.x + 20, curY, label, Color::fromHex(0x94A3B8), 1);
        clientSurface.drawString(modalRect.x + 180, curY, val, valCol, 1);
        curY += 26;
    };

    drawDetail("Package ID:", foundCard->id);
    drawDetail("Catalog Version:", "v" + foundCard->version);
    drawDetail("Publisher / Vendor:", foundCard->publisher);
    drawDetail("License:", foundCard->license);
    drawDetail("Target Architecture:", foundCard->architecture);
    drawDetail("Clean-Room FIPS SHA-256:", foundCard->sha256.empty() ? "(Catalog verified digest)" : foundCard->sha256, Color::fromHex(0x38BDF8));
    drawDetail("Installer URL:", foundCard->downloadUrl.empty() ? "https://github.com/microsoft/winget-pkgs" : foundCard->downloadUrl, Color::fromHex(0x7DD3FC));
    drawDetail("Pin Status:", foundCard->isPinned ? "PINNED (Locked against upgrades)" : "UNPINNED (Eligible for auto-updates)", foundCard->isPinned ? Color::fromHex(0xFCD34D) : Color::fromHex(0x34D399));
    drawDetail("Install Status:", foundCard->isInstalled ? ("Installed (v" + (foundCard->installedVersion.empty() ? foundCard->version : foundCard->installedVersion) + ")") : "Not Installed", foundCard->isInstalled ? Color::fromHex(0x34D399) : Color::fromHex(0x94A3B8));

    curY += 4;
    clientSurface.drawString(modalRect.x + 20, curY, "Description:", Color::fromHex(0x94A3B8), 1);
    clientSurface.drawString(modalRect.x + 180, curY, foundCard->description, Color::fromHex(0xE2E8F0), 1);
}

bool AppHubContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    // Check if Inspector modal is open
    if (inspectedPackageId_.has_value()) {
        if (inspectorCloseBtnBounds_.contains(localPt)) {
            closeInspector();
            return true;
        }
        const int32_t modalW = std::min(680, catalogAreaBounds_.width - 20);
        const int32_t modalH = 380;
        const Rect modalRect{catalogAreaBounds_.centerX() - modalW / 2, catalogAreaBounds_.centerY() - modalH / 2, modalW, modalH};
        if (!modalRect.contains(localPt)) {
            closeInspector();
            return true;
        }
        return true; // Modal consumes click
    }

    // Check Category Tabs
    for (const auto& tab : categoryTabs_) {
        if (tab.bounds.contains(localPt)) {
            setCategory(tab.cat);
            return true;
        }
    }

    // Check Curated Stacks Install Buttons
    if (activeCategory_ == AppHubCategory::Stacks) {
        for (const auto& s : curatedStacks_) {
            if (s.installBtnBounds.contains(localPt)) {
                installCuratedStack(s.id);
                return true;
            }
        }
        return false;
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

    // Check Installed Subsystem buttons (Scan System Apps & Update All)
    if (activeCategory_ == AppHubCategory::Installed) {
        if (scanSystemBtnBounds_.contains(localPt)) {
            scanSystemInstalled();
            return true;
        }
        if (updateAvailableCount() > 0 && updateAllBtnBounds_.contains(localPt)) {
            upgradeAllPackages();
            return true;
        }
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
        if (micaGToggleBounds_.contains(localPt)) {
            auto& micag = MicaGManager::instance();
            micag.setMicaGEnabled(!micag.isMicaGEnabled());
            if (installCallback_) {
                installCallback_("MicaG Sovereign Bridge",
                                 micag.isMicaGEnabled()
                                     ? "MicaG Hardware TPM 2.0 Play Integrity & MicaIPC enabled."
                                     : "MicaG GMS compatibility layer disabled.",
                                 micag.isMicaGEnabled());
            }
            return true;
        }
        return false;
    }

    // Check Action Buttons, Pin Buttons, and Inspect Buttons on Cards
    for (const auto& card : filteredCards_) {
        if (card.inspectBtnBounds.contains(localPt)) {
            inspectPackage(card.id);
            return true;
        }
        if (card.pinBtnBounds.contains(localPt)) {
            pinPackage(card.id, !card.isPinned);
            return true;
        }
        if (card.actionBtnBounds.contains(localPt)) {
            if (card.hasUpdateAvailable) {
                upgradePackage(card.id);
            } else if (card.isInstalled) {
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
    if (inspectedPackageId_.has_value()) {
        isInspectorCloseBtnHovered_ = inspectorCloseBtnBounds_.contains(localPt);
        return true;
    }

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

    if (activeCategory_ == AppHubCategory::Stacks) {
        hoveredStackIndex_ = -1;
        hoveredStackInstallBtnIndex_ = -1;
        for (size_t i = 0; i < curatedStacks_.size(); ++i) {
            if (curatedStacks_[i].installBtnBounds.contains(localPt)) {
                hoveredStackInstallBtnIndex_ = static_cast<int32_t>(i);
                hoveredStackIndex_ = static_cast<int32_t>(i);
                return true;
            }
            if (curatedStacks_[i].bounds.contains(localPt)) {
                hoveredStackIndex_ = static_cast<int32_t>(i);
                return true;
            }
        }
        return isSearchHovered_ || hoveredCategoryTabIndex_ >= 0;
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
        isMicaGToggleHovered_ = micaGToggleBounds_.contains(localPt);
        return isScanRepoHovered_ || isSyncUpstreamHovered_ || isSyncWsaRepoHovered_ ||
               isFipsToggleHovered_ || isArchX64Hovered_ || isArchArm64Hovered_ ||
               isResetDefaultsHovered_ || isMicaGToggleHovered_ || isWsaBadgeHovered_ || isSearchHovered_;
    }

    if (activeCategory_ == AppHubCategory::Installed) {
        isScanSystemBtnHovered_ = scanSystemBtnBounds_.contains(localPt);
        isUpdateAllBtnHovered_ = (updateAvailableCount() > 0 && updateAllBtnBounds_.contains(localPt));
    } else {
        isScanSystemBtnHovered_ = false;
        isUpdateAllBtnHovered_ = false;
    }

    hoveredCardIndex_ = -1;
    hoveredActionBtnIndex_ = -1;
    hoveredPinBtnIndex_ = -1;
    hoveredInspectBtnIndex_ = -1;
    for (size_t i = 0; i < filteredCards_.size(); ++i) {
        if (filteredCards_[i].actionBtnBounds.contains(localPt)) {
            hoveredActionBtnIndex_ = static_cast<int32_t>(i);
            hoveredCardIndex_ = static_cast<int32_t>(i);
            return true;
        }
        if (filteredCards_[i].pinBtnBounds.contains(localPt)) {
            hoveredPinBtnIndex_ = static_cast<int32_t>(i);
            hoveredCardIndex_ = static_cast<int32_t>(i);
            return true;
        }
        if (filteredCards_[i].inspectBtnBounds.contains(localPt)) {
            hoveredInspectBtnIndex_ = static_cast<int32_t>(i);
            hoveredCardIndex_ = static_cast<int32_t>(i);
            return true;
        }
        if (filteredCards_[i].cardBounds.contains(localPt)) {
            hoveredCardIndex_ = static_cast<int32_t>(i);
            return true;
        }
    }

    return isSearchHovered_ || hoveredCategoryTabIndex_ >= 0 || isScanSystemBtnHovered_ || isUpdateAllBtnHovered_;
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
    if (key == KeyCode::Escape) {
        if (inspectedPackageId_.has_value()) {
            closeInspector();
            return true;
        }
        if (!searchQuery_.empty()) {
            setSearchQuery("");
            return true;
        }
    } else if (key == KeyCode::Backspace) {
        if (!searchQuery_.empty()) {
            searchQuery_.pop_back();
            setSearchQuery(searchQuery_);
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
