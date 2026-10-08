// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/explorer.hpp)
//
// File Cabinet & Navigation Explorer (CabinetWnd) - Modern 2026 Windows Edition.
// Supports multi-tab navigation, real filesystem access (std::filesystem),
// live search filtering, breadcrumbs, column sorting, sidebar quick access,
// and file operation commands.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "theme.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <functional>
#include <optional>
#include <chrono>

namespace surshell {

enum class ExplorerViewMode {
    DetailsList = 0,
    TilesGrid
};

enum class ExplorerSortColumn {
    Name = 0,
    DateModified,
    Type,
    Size
};

struct FileItem {
    std::string name;
    std::string fullPath;
    std::string extension;
    bool isDirectory{false};
    uint64_t sizeBytes{0};
    std::string dateModified{};
    std::string typeDescription{};
    std::string iconGlyph{"[F]"};
    Rect bounds{};
    bool selected{false};
};

struct DriveInfo {
    std::string rootPath;      // e.g. "C:\\"
    std::string label;         // e.g. "Local Disk (C:)"
    uint64_t totalBytes{0};
    uint64_t freeBytes{0};
    Rect bounds{};
};

struct ExplorerTab {
    std::string title{"This PC"};
    std::string currentPath{"This PC"};
    std::vector<std::string> backHistory{};
    std::vector<std::string> forwardHistory{};
    std::vector<FileItem> allItems{};
    std::vector<FileItem> visibleItems{};
    std::string searchQuery{};
    int32_t selectedIndex{-1};
    Rect tabBounds{};
    Rect closeButtonBounds{};
};

class FileExplorer {
public:
    using FileExecuteCallback = std::function<void(const std::string& path)>;
    using PathChangeCallback = std::function<void(const std::string& path)>;

    explicit FileExplorer(std::string initialPath = "C:\\");

    // Tab Management (Windows 11-style multi-tab file explorer)
    [[nodiscard]] size_t tabCount() const noexcept { return tabs_.size(); }
    [[nodiscard]] size_t activeTabIndex() const noexcept { return activeTabIndex_; }
    void addTab(std::string path = "This PC");
    void closeTab(size_t index);
    void switchTab(size_t index);

    // Navigation
    void navigateTo(std::string path);
    void navigateUp();
    void navigateBack();
    void navigateForward();
    void refresh();

    [[nodiscard]] const std::string& currentPath() const noexcept;
    [[nodiscard]] const std::vector<FileItem>& items() const noexcept;
    [[nodiscard]] std::optional<FileItem> selectedItem() const;

    // Search & Filter
    void setSearchQuery(std::string query);
    [[nodiscard]] const std::string& searchQuery() const noexcept;
    void clearSearch();

    // View & Sorting
    void setViewMode(ExplorerViewMode mode) noexcept { viewMode_ = mode; }
    [[nodiscard]] ExplorerViewMode viewMode() const noexcept { return viewMode_; }

    void sortBy(ExplorerSortColumn column, bool toggleDirection = true);
    [[nodiscard]] ExplorerSortColumn sortColumn() const noexcept { return sortColumn_; }
    [[nodiscard]] bool isSortAscending() const noexcept { return sortAscending_; }

    // File Operations
    bool createNewFolder(std::string_view folderName = "New Folder");
    bool deleteSelected();

    // Callbacks
    void setExecuteCallback(FileExecuteCallback cb) { executeCallback_ = std::move(cb); }
    void setPathChangeCallback(PathChangeCallback cb) { pathChangeCallback_ = std::move(cb); }

    // Input Handling
    void onMouseDown(Point localPt, MouseButton button, Rect clientBounds);
    void onDoubleClick(Point localPt, Rect clientBounds);
    void onMouseMove(Point localPt);
    void onCharInput(char c);
    void onBackspace();

    // Rendering
    void render(Surface& clientSurface);

private:
    std::vector<ExplorerTab> tabs_{};
    size_t activeTabIndex_{0};
    ExplorerViewMode viewMode_{ExplorerViewMode::DetailsList};
    ExplorerSortColumn sortColumn_{ExplorerSortColumn::Name};
    bool sortAscending_{true};
    bool searchBoxFocused_{false};

    std::vector<DriveInfo> drives_{};

    // UI Interactive Hit-Test Regions
    Rect navBackBtn_{};
    Rect navFwdBtn_{};
    Rect navUpBtn_{};
    Rect navRefreshBtn_{};
    Rect addressBarBounds_{};
    Rect searchBoxBounds_{};
    Rect newTabButtonBounds_{};

    // Toolbar Command Buttons
    Rect cmdNewFolder_{};
    Rect cmdDelete_{};
    Rect cmdViewToggle_{};

    // Sidebar Quick Access Bounds
    std::vector<std::pair<std::string, Rect>> sidebarQuickPins_{};

    FileExecuteCallback executeCallback_{};
    PathChangeCallback pathChangeCallback_{};

    void refreshCurrentDirectory();
    void refreshDrives();
    void applySearchFilter();
    void sortCurrentItems();

    [[nodiscard]] static std::string formatBytes(uint64_t bytes);
    [[nodiscard]] static std::string getFileIconGlyph(std::string_view extension, bool isDirectory);
};

} // namespace surshell
