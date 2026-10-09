// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/eventviewer.hpp)
//
// Sovereign Event Viewer Console (eventvwr.msc Parity)
// Clean-room ISO C++23, zero telemetry, Windows Event Log integration,
// category navigation (Application, Security, System, Setup), level filtering
// (Critical, Error, Warning, Information, Audit), sub-millisecond search,
// live event details preview, and Event Properties dialog with XML export.
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
#include <memory>

namespace surshell {

enum class EventLevel {
    Critical,
    Error,
    Warning,
    Information,
    AuditSuccess,
    AuditFailure,
    Unknown
};

inline std::string eventLevelToString(EventLevel lvl) {
    switch (lvl) {
        case EventLevel::Critical: return "Critical";
        case EventLevel::Error: return "Error";
        case EventLevel::Warning: return "Warning";
        case EventLevel::Information: return "Information";
        case EventLevel::AuditSuccess: return "Audit Success";
        case EventLevel::AuditFailure: return "Audit Failure";
        default: return "Information";
    }
}

inline Color eventLevelToColor(EventLevel lvl) {
    switch (lvl) {
        case EventLevel::Critical:
        case EventLevel::Error:
            return Color{232, 17, 35, 255}; // Windows red
        case EventLevel::Warning:
            return Color{255, 185, 0, 255}; // Windows amber
        case EventLevel::AuditSuccess:
            return Color{16, 124, 65, 255}; // Windows green
        case EventLevel::AuditFailure:
            return Color{216, 59, 1, 255}; // Windows orange
        case EventLevel::Information:
        default:
            return Color{0, 120, 215, 255}; // Windows blue
    }
}

enum class EventLogCategory {
    Application,
    Security,
    System,
    Setup
};

inline std::string eventLogCategoryToString(EventLogCategory cat) {
    switch (cat) {
        case EventLogCategory::Application: return "Application";
        case EventLogCategory::Security: return "Security";
        case EventLogCategory::System: return "System";
        case EventLogCategory::Setup: return "Setup";
        default: return "System";
    }
}

struct EventRecord {
    uint32_t recordNumber{0};
    EventLevel level{EventLevel::Information};
    uint32_t eventId{0};
    std::string timeGenerated{}; // "YYYY-MM-DD HH:MM:SS"
    uint64_t timestampEpoch{0};
    std::string source{};        // Provider / Source name (e.g. "Service Control Manager")
    std::string taskCategory{"None"};
    std::string computer{"SUR-SERVER25"};
    std::string user{"SYSTEM"};
    std::string message{};       // Formatted human-readable event message
    std::vector<std::string> rawStrings{};
    Rect bounds{};               // Table row layout bounds
};

struct LogCategoryItem {
    EventLogCategory category{EventLogCategory::System};
    std::string name{"System"};
    size_t count{0};
    Rect bounds{};
};

enum class EventLevelFilter {
    All,
    ErrorsAndCritical,
    Warnings,
    Information
};

class EventViewerContent : public IWindowContent {
public:
    explicit EventViewerContent(const std::string& initialLog = "System");

    void scanLog(const std::string& logName);
    void refresh() { scanLog(currentLogName_); }

    [[nodiscard]] const std::string& currentLogName() const noexcept { return currentLogName_; }
    void selectLog(const std::string& logName);
    void selectCategory(EventLogCategory cat);

    [[nodiscard]] const std::vector<LogCategoryItem>& categories() const noexcept { return categories_; }
    [[nodiscard]] size_t totalRecordsCount() const noexcept { return records_.size(); }
    [[nodiscard]] size_t errorCount() const noexcept;
    [[nodiscard]] size_t warningCount() const noexcept;
    [[nodiscard]] size_t infoCount() const noexcept;

    [[nodiscard]] const std::vector<EventRecord>& records() const noexcept { return records_; }
    [[nodiscard]] const EventRecord* selectedRecord() const noexcept;

    // Selection & Navigation
    void selectIndex(size_t index);
    [[nodiscard]] int32_t selectedIndex() const noexcept { return selectedIndex_; }

    // Filtering
    void setLevelFilter(EventLevelFilter filter);
    [[nodiscard]] EventLevelFilter levelFilter() const noexcept { return levelFilter_; }

    void setSearchQuery(const std::string& query);
    [[nodiscard]] const std::string& searchQuery() const noexcept { return searchQuery_; }

    // Clear Log action
    void clearLog();

    // Properties Dialog Modal
    void openPropertiesDialog();
    void closePropertiesDialog();
    [[nodiscard]] bool isPropertiesDialogOpen() const noexcept { return showPropertiesModal_; }

    // XML Export
    [[nodiscard]] std::string selectedRecordToXml() const;

    // Callbacks
    void setToastCallback(std::function<void(const std::string&, const std::string&, IconId)> cb) {
        onToast_ = std::move(cb);
    }

    // IWindowContent overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseUp(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onMouseWheel(Point localPt, int32_t delta) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;
    bool onCharInput(char c) override;
    [[nodiscard]] bool isSearchFocused() const { return searchFocused_; }
    void setSearchFocused(bool focused) { searchFocused_ = focused; }

private:
    void updateFilteredIndices();
    void renderSidebar(Surface& s, const Rect& area);
    void renderToolbar(Surface& s, const Rect& area);
    void renderEventTable(Surface& s, const Rect& area);
    void renderDetailsPane(Surface& s, const Rect& area);
    void renderPropertiesModal(Surface& s, int32_t width, int32_t height);

    std::string currentLogName_{"System"};
    std::vector<LogCategoryItem> categories_{};
    std::vector<EventRecord> records_{};
    std::vector<size_t> filteredIndices_{};

    int32_t selectedIndex_{-1};
    int32_t scrollOffset_{0};
    int32_t detailsScrollOffset_{0};
    EventLevelFilter levelFilter_{EventLevelFilter::All};
    std::string searchQuery_{};
    bool searchFocused_{false};

    // Modal dialog state
    bool showPropertiesModal_{false};
    int32_t modalActiveTab_{0}; // 0 = General, 1 = XML Details

    // Interactive button rects
    Rect searchBoxBounds_{};
    Rect btnFilterAll_{};
    Rect btnFilterErrors_{};
    Rect btnFilterWarnings_{};
    Rect btnFilterInfo_{};
    Rect btnRefresh_{};
    Rect btnClearLog_{};
    Rect btnProperties_{};

    // Modal buttons
    Rect modalBtnGeneralTab_{};
    Rect modalBtnXmlTab_{};
    Rect modalBtnCopy_{};
    Rect modalBtnClose_{};
    Rect modalBtnPrev_{};
    Rect modalBtnNext_{};

    Point mousePos_{0, 0};
    std::function<void(const std::string&, const std::string&, IconId)> onToast_{};
};

} // namespace surshell
