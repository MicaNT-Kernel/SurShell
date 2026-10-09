// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/sysinfo.hpp)
//
// Sovereign System Information & Hardware Diagnostics (msinfo32.exe Parity)
// Clean-room ISO C++23, zero telemetry, hardware topology discovery,
// storage volumes, memory layout, drivers, and diagnostic report export.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "window_manager.hpp"
#include "theme.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace surshell {

struct SysInfoEntry {
    std::string item;
    std::string value;
};

struct SysInfoCategory {
    std::string id;
    std::string title;
    IconId iconId{IconId::SystemInfo};
    std::vector<SysInfoEntry> entries{};
    std::vector<SysInfoCategory> subcategories{};
    bool isExpanded{true};
};

class SysInfoContent : public IWindowContent {
public:
    SysInfoContent();

    void refresh();
    bool exportReport(const std::string& filePath = "msinfo32_report.txt");

    void selectCategory(const std::string& categoryId);
    [[nodiscard]] const std::string& selectedCategoryId() const noexcept { return selectedCategoryId_; }
    [[nodiscard]] size_t currentEntryCount() const noexcept;
    [[nodiscard]] const std::vector<SysInfoEntry>& currentEntries() const noexcept;

    void setFilterQuery(const std::string& query);
    [[nodiscard]] const std::string& filterQuery() const noexcept { return filterQuery_; }

    [[nodiscard]] int32_t selectedRowIndex() const noexcept { return selectedRowIndex_; }
    void selectRow(int32_t index);
    [[nodiscard]] std::string copySelectedRow() const;
    [[nodiscard]] std::string copyAllRows() const;

    // Toast and notification callback
    void setToastCallback(std::function<void(const std::string&, const std::string&, IconId)> cb) {
        onToast_ = std::move(cb);
    }

    // IWindowContent overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseUp(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onMouseWheel(Point localPt, int32_t delta) override;
    bool onCharInput(char c) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;

private:
    void populateDiagnosticTree();
    void collectLiveHostTelemetry();
    [[nodiscard]] SysInfoCategory* findCategory(const std::string& id);
    [[nodiscard]] const SysInfoCategory* findCategory(const std::string& id) const;

    std::vector<SysInfoCategory> categories_{};
    std::string selectedCategoryId_{"summary"};
    int32_t selectedRowIndex_{0};
    int32_t scrollOffset_{0};
    std::string filterQuery_{};
    bool searchFocused_{false};

    // Filtered entries cache
    std::vector<SysInfoEntry> filteredEntries_{};
    void updateFilteredEntries();

    // Hit-test UI regions
    Rect exportBtn_{};
    Rect refreshBtn_{};
    Rect copyAllBtn_{};
    Rect searchBox_{};
    Rect treePaneRect_{};
    Rect tablePaneRect_{};
    std::vector<std::pair<Rect, std::string>> treeItemBounds_{};
    std::vector<Rect> rowBounds_{};

    std::string statusMessage_{"Ready"};
    std::function<void(const std::string&, const std::string&, IconId)> onToast_{};
};

} // namespace surshell
