// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/wsa.hpp)
//
// MicaNT Windows Subsystem for Android (WSA / wsa.hpp) Package Management Bridge
//
// Organization: Barrer Software | Ecosystem: MicaNT-Kernel (GitHub)
// Subsystem: Pure AOSP runtime compliance (Apache 2.0 clean-room target)
// Distribution: Public GitHub Manifest Repository (MicaNT-Kernel/micant-apps)
//
// Legal & Clean-Room Guarantees:
//   - Zero proprietary Google Play binaries (Phonesky.apk) or GMS dependencies
//   - Zero scraping or token harvesting against proprietary APIs
//   - Human-readable YAML/JSON manifests with direct upstream official developer URLs
//   - NIST FIPS 180-4 SHA-256 pre-flight cryptographic gate before WSA payload handoff
//   - Direct local .apk drag-and-drop sideloading broker
// ============================================================================

#pragma once

#include "types.hpp"
#include "icons.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <filesystem>
#include <functional>
#include <mutex>
#include <memory>
#include <cstdint>

namespace surshell {

class Sha256FipsEngine {
public:
    static std::string hashString(std::string_view str);
    static std::string hashFile(const std::filesystem::path& path);
};

struct WsaPackageManifest {
    std::string id;            // e.g. "org.videolan.vlc"
    std::string name;          // e.g. "VLC for Android"
    std::string version;       // e.g. "3.5.4"
    std::string vendor;        // e.g. "VideoLAN"
    std::string license;       // e.g. "GPLv3"
    std::string homepage;      // e.g. "https://www.videolan.org"
    std::string downloadUrl;   // Upstream official HTTPS asset URL
    std::string sha256;        // 64-char NIST FIPS 180-4 SHA-256 digest
    std::string architecture;  // "arm64-v8a", "x86_64", "universal"
    std::string category;      // "Multimedia", "Internet", "Utilities", "Productivity", "Games"
    std::string description;   // Human-readable summary
    std::string iconGlyph{"[APK]"};
    IconId iconId{IconId::FileExecutable};
    bool isInstalled{false};
};

struct WsaSubsystemStatus {
    bool isInstalled{false};
    bool isRunning{false};
    std::string ipAddress{"127.0.0.1"};
    uint16_t adbPort{58526};
    std::string androidVersion{"AOSP 13.0 (Tiramisu)"};
    std::string clientPath;
    size_t installedAppCount{0};
};

class WsaCatalog {
public:
    WsaCatalog();

    [[nodiscard]] const std::vector<WsaPackageManifest>& packages() const noexcept { return packages_; }
    [[nodiscard]] size_t size() const noexcept { return packages_.size(); }
    [[nodiscard]] bool empty() const noexcept { return packages_.empty(); }
    [[nodiscard]] std::vector<WsaPackageManifest> search(std::string_view query) const;
    [[nodiscard]] std::vector<WsaPackageManifest> filterByCategory(std::string_view category) const;
    [[nodiscard]] std::vector<WsaPackageManifest> searchByCategory(std::string_view category) const { return filterByCategory(category); }
    [[nodiscard]] std::optional<WsaPackageManifest> findById(std::string_view id) const;
    [[nodiscard]] std::optional<WsaPackageManifest> findPackage(std::string_view id) const { return findById(id); }

    bool loadFromJsonString(std::string_view jsonStr);
    bool loadFromJson(std::string_view jsonStr) { return loadFromJsonString(jsonStr); }
    bool loadFromCatalogFile(const std::filesystem::path& path);
    size_t loadFromManifestDirectory(const std::filesystem::path& dirPath);

    [[nodiscard]] std::string exportToJson(bool minified = false) const;
    [[nodiscard]] std::string exportJson(bool minified = false) const { return exportToJson(minified); }
    void seedDefaultMicaNtApps();

    [[nodiscard]] const std::string& remoteCatalogEndpoint() const noexcept { return remoteCatalogEndpoint_; }
    void setRemoteCatalogEndpoint(std::string endpoint) { remoteCatalogEndpoint_ = std::move(endpoint); }

private:
    std::vector<WsaPackageManifest> packages_{};
    std::string remoteCatalogEndpoint_{"https://raw.githubusercontent.com/MicaNT-Kernel/micant-apps/main/catalog.json"};
    mutable std::mutex mutex_{};
};

class WsaSubsystemBridge {
public:
    static WsaSubsystemBridge& instance();

    WsaSubsystemBridge();

    [[nodiscard]] WsaSubsystemStatus probeStatus();
    [[nodiscard]] bool isAdbAvailable() const;
    bool startSubsystem();

    // Cryptographic pre-flight verification (FIPS 180-4 SHA-256)
    [[nodiscard]] static bool verifyApkSha256(const std::filesystem::path& apkPath, const std::string& expectedSha256);
    [[nodiscard]] static bool verifyApkSha256(const std::filesystem::path& apkPath, const std::string& expectedSha256, std::string& outLog);

    // Structural ZIP / AndroidManifest.xml validation
    [[nodiscard]] static bool validateApkStructure(const std::filesystem::path& apkPath);
    [[nodiscard]] static bool validateApkStructure(const std::filesystem::path& apkPath, std::string& outLog);

    // Package Installer Service
    bool installApk(const std::filesystem::path& apkPath, std::string& outLog);
    bool uninstallApp(const std::string& packageId, std::string& outLog);
    bool launchApp(const std::string& packageId);
    [[nodiscard]] std::vector<std::string> queryInstalledPackages();

    // Sideload broker
    bool sideloadLocalApk(const std::filesystem::path& apkPath, std::string& outLog);

private:
    WsaSubsystemStatus cachedStatus_{};
    std::mutex bridgeMutex_{};
    std::string findWsaClientExecutable() const;
    std::string findAdbExecutable() const;
};

} // namespace surshell
