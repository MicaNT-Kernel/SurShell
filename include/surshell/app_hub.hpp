// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/app_hub.hpp)
//
// Winget Sovereign App Hub & Software Store Console
// Clean-Room ISO C++23 front-end connecting directly to the sovereign winget
// engine and local package repository cache.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "window_manager.hpp"
#include "theme.hpp"
#include "icons.hpp"
#include "winget.hpp"
#include "wsa.hpp"

#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <memory>

namespace surshell {

enum class AppHubCategory {
    All = 0,
    CertifiedRetail,
    AndroidWsa,
    DeveloperTools,
    SystemUtilities,
    MediaDocs,
    Installed,
    Settings
};

struct AppHubCard {
    std::string id;
    std::string name;
    std::string version;
    std::string publisher;
    std::string license;
    std::string description;
    std::string moniker;
    IconId iconId{IconId::AppHub};
    AppHubCategory category{AppHubCategory::SystemUtilities};
    bool isInstalled{false};
    bool isRetailCertified{false};
    bool isAndroidApp{false};
    std::string architecture{"x64"};
    std::string downloadUrl{};
    std::string sha256{};
    Rect cardBounds{};
    Rect actionBtnBounds{};
};

class AppHubContent : public IWindowContent {
public:
    using InstallCallback = std::function<void(const std::string& title, const std::string& message, bool success)>;

    AppHubContent();

    void refresh();

    [[nodiscard]] size_t totalPackagesCount() const noexcept { return allCards_.size(); }
    [[nodiscard]] size_t filteredPackagesCount() const noexcept { return filteredCards_.size(); }
    [[nodiscard]] size_t installedPackagesCount() const noexcept;

    [[nodiscard]] const std::vector<AppHubCard>& filteredCards() const noexcept { return filteredCards_; }
    [[nodiscard]] const std::string& searchQuery() const noexcept { return searchQuery_; }
    [[nodiscard]] AppHubCategory activeCategory() const noexcept { return activeCategory_; }

    void setSearchQuery(std::string query);
    void setCategory(AppHubCategory cat);

    bool installPackage(const std::string& packageId);
    bool uninstallPackage(const std::string& packageId);

    void setInstallCallback(InstallCallback cb) { installCallback_ = std::move(cb); }

    [[nodiscard]] WsaCatalog& wsaCatalog() noexcept { return wsaCatalog_; }
    [[nodiscard]] const WsaSubsystemStatus& wsaStatus() const noexcept { return wsaStatus_; }
    void refreshWsaStatus();
    bool sideloadApk(const std::filesystem::path& apkPath);
    [[nodiscard]] Rect wsaStatusBadgeBounds() const noexcept { return wsaStatusBadgeBounds_; }
    [[nodiscard]] Rect wsaSideloadBtnBounds() const noexcept { return wsaSideloadBtnBounds_; }
    [[nodiscard]] Rect syncWsaRepoBtnBounds() const noexcept { return syncWsaRepoBtnBounds_; }
    [[nodiscard]] Rect micaGToggleBounds() const noexcept { return micaGToggleBounds_; }

    // IWindowContent Interface
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onMouseWheel(Point localPt, int32_t delta) override;
    bool onCharInput(char c) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;

private:
    std::vector<AppHubCard> allCards_{};
    std::vector<AppHubCard> filteredCards_{};

    std::string searchQuery_{};
    AppHubCategory activeCategory_{AppHubCategory::All};

    int32_t scrollY_{0};
    int32_t maxScrollY_{0};

    // UI Regions
    Rect searchBarBounds_{};
    Rect categoryTabsBounds_{};
    Rect catalogAreaBounds_{};

    struct CategoryTab {
        AppHubCategory cat;
        std::string label;
        Rect bounds{};
    };
    std::vector<CategoryTab> categoryTabs_{};

    int32_t hoveredCardIndex_{-1};
    int32_t hoveredActionBtnIndex_{-1};
    int32_t hoveredCategoryTabIndex_{-1};
    bool isSearchHovered_{false};

    InstallCallback installCallback_{};

    // WSA Subsystem integration
    WsaCatalog wsaCatalog_{};
    WsaSubsystemStatus wsaStatus_{};
    Rect wsaStatusBadgeBounds_{};
    Rect wsaSideloadBtnBounds_{};
    Rect syncWsaRepoBtnBounds_{};
    bool isWsaBadgeHovered_{false};
    bool isSideloadBtnHovered_{false};
    bool isSyncWsaRepoHovered_{false};
    Rect micaGToggleBounds_{};
    bool isMicaGToggleHovered_{false};

    // Settings & Sources UI state
    struct RepoSourceCard {
        std::string name;
        std::string argument;
        std::string type;
        bool isActive{false};
        Rect bounds{};
        Rect selectBtnBounds{};
    };
    std::vector<RepoSourceCard> repoSourceCards_{};
    Rect scanRepoBtnBounds_{};
    Rect syncUpstreamBtnBounds_{};
    Rect fipsToggleBounds_{};
    Rect archX64BtnBounds_{};
    Rect archArm64BtnBounds_{};
    Rect resetDefaultsBtnBounds_{};

    int32_t hoveredSourceIndex_{-1};
    int32_t hoveredSourceSelectBtnIndex_{-1};
    bool isScanRepoHovered_{false};
    bool isSyncUpstreamHovered_{false};
    bool isFipsToggleHovered_{false};
    bool isArchX64Hovered_{false};
    bool isArchArm64Hovered_{false};
    bool isResetDefaultsHovered_{false};

    void populateCatalog();
    void updateFilter();
    void updateLayout(int32_t width, int32_t height);
    void renderSettingsView(Surface& clientSurface, int32_t width, int32_t height);
};

} // namespace surshell
