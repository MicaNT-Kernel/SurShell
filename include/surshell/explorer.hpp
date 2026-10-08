// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/explorer.hpp)
//
// File Cabinet & Navigation Explorer (CabinetWnd) - Modern 2026 Windows Edition.
// Supports multi-tab navigation, real filesystem access (std::filesystem),
// live search filtering, breadcrumbs, column sorting, sidebar quick access,
// vertical scrolling with scrollbar, right-click context menu, file properties dialog,
// and file execution / editor launching.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "theme.hpp"
#include "window_manager.hpp"
#include "icons.hpp"
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
    IconId iconId{IconId::FileGeneric};
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
    int32_t scrollOffset{0};
    Rect tabBounds{};
    Rect closeButtonBounds{};
};

struct BreadcrumbItem {
    std::string name;
    std::string targetPath;
    Rect bounds{};
};

struct ContextMenuItem {
    std::string id;
    std::string label;
    std::string shortcut;
    std::string iconGlyph;
    IconId iconId{IconId::FileGeneric};
    bool isSeparator{false};
    bool isEnabled{true};
    Rect bounds{};
};

struct ContextMenu {
    bool isOpen{false};
    Point position{0, 0};
    std::vector<ContextMenuItem> items{};
    int32_t hoveredIndex{-1};
    Rect bounds{};
};

struct FilePropertiesDialog {
    bool isOpen{false};
    FileItem item{};
    Rect bounds{};
    Rect closeButtonBounds{};
    Rect okButtonBounds{};
};

class FileExplorer : public IWindowContent {
public:
    using FileExecuteCallback = std::function<void(const std::string& path)>;
    using PathChangeCallback = std::function<void(const std::string& path)>;
    using OpenEditorCallback = std::function<void(const std::string& path)>;
    using OpenTerminalCallback = std::function<void(const std::string& workingDir)>;

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

    // Scrolling
    [[nodiscard]] int32_t scrollOffset() const noexcept;
    void setScrollOffset(int32_t offset);

    // File Operations & Clipboard
    bool createNewFolder(std::string_view folderName = "New Folder");
    bool createNewFile(std::string_view fileName = "New Document.txt");
    void copySelected();
    void cutSelected();
    void pasteToCurrentDirectory();
    bool renameSelected(std::string_view newName);
    void startRename();
    void cancelRename() noexcept { isRenaming_ = false; }
    [[nodiscard]] bool isRenaming() const noexcept { return isRenaming_; }
    [[nodiscard]] bool canPaste() const noexcept { return s_clipboard.has_value(); }
    bool deleteSelected();

    // Context Menu & Properties
    void openContextMenu(Point pt, bool forItem);
    void closeContextMenu();
    [[nodiscard]] const ContextMenu& contextMenu() const noexcept { return contextMenu_; }

    void showPropertiesDialog(const FileItem& item);
    void closePropertiesDialog();
    [[nodiscard]] const FilePropertiesDialog& propertiesDialog() const noexcept { return propertiesDialog_; }

    // Callbacks
    void setExecuteCallback(FileExecuteCallback cb) { executeCallback_ = std::move(cb); }
    void setPathChangeCallback(PathChangeCallback cb) { pathChangeCallback_ = std::move(cb); }
    void setOpenEditorCallback(OpenEditorCallback cb) { openEditorCallback_ = std::move(cb); }
    void setOpenTerminalCallback(OpenTerminalCallback cb) { openTerminalCallback_ = std::move(cb); }

    // IWindowContent Interface Overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseUp(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onDoubleClick(Point localPt) override;
    bool onMouseWheel(Point localPt, int32_t delta) override;
    bool onCharInput(char c) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;

    // Backward-compatible event overloads
    void onMouseDown(Point localPt, MouseButton button, Rect clientBounds);
    void onDoubleClick(Point localPt, Rect clientBounds);
    void onBackspace();

private:
    std::vector<ExplorerTab> tabs_{};
    size_t activeTabIndex_{0};
    ExplorerViewMode viewMode_{ExplorerViewMode::DetailsList};
    ExplorerSortColumn sortColumn_{ExplorerSortColumn::Name};
    bool sortAscending_{true};
    bool searchBoxFocused_{false};

    // Address Bar Direct Edit & Breadcrumbs
    bool addressEditing_{false};
    std::string addressEditText_{};
    std::vector<BreadcrumbItem> breadcrumbs_{};

    // Context Menu & Inspector
    ContextMenu contextMenu_{};
    FilePropertiesDialog propertiesDialog_{};

    std::vector<DriveInfo> drives_{};

    // UI Interactive Hit-Test Regions
    Rect navBackBtn_{};
    Rect navFwdBtn_{};
    Rect navUpBtn_{};
    Rect navRefreshBtn_{};
    Rect addressBarBounds_{};
    Rect searchBoxBounds_{};
    Rect newTabButtonBounds_{};

    // Column Headers
    Rect nameHeaderBounds_{};
    Rect dateHeaderBounds_{};
    Rect typeHeaderBounds_{};
    Rect sizeHeaderBounds_{};

    // Scrollbar Regions
    Rect scrollbarTrack_{};
    Rect scrollbarThumb_{};
    bool isDraggingScrollbar_{false};
    int32_t scrollDragStartMouseY_{0};
    int32_t scrollDragStartOffset_{0};

    // Clipboard & Renaming State
    struct FileClipboard {
        std::string fullPath;
        bool isCut{false};
    };
    static inline std::optional<FileClipboard> s_clipboard{};

    bool isRenaming_{false};
    std::string renameEditText_{};

    // Toolbar Command Buttons
    Rect cmdNewFolder_{};
    Rect cmdNewFile_{};
    Rect cmdCut_{};
    Rect cmdCopy_{};
    Rect cmdPaste_{};
    Rect cmdRename_{};
    Rect cmdDelete_{};
    Rect cmdViewToggle_{};

    // Sidebar Quick Access Bounds
    std::vector<std::pair<std::string, Rect>> sidebarQuickPins_{};

    FileExecuteCallback executeCallback_{};
    PathChangeCallback pathChangeCallback_{};
    OpenEditorCallback openEditorCallback_{};
    OpenTerminalCallback openTerminalCallback_{};

    void ensureSelectionVisible();

    void refreshCurrentDirectory();
    void refreshDrives();
    void applySearchFilter();
    void sortCurrentItems();
    void updateBreadcrumbs();
    void executeItem(const FileItem& item);

    [[nodiscard]] static std::string formatBytes(uint64_t bytes);
    [[nodiscard]] static std::string getFileIconGlyph(std::string_view extension, bool isDirectory);
};

} // namespace surshell
