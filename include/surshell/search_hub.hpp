// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/search_hub.hpp)
//
// Modern Windows 11-style Universal Search Hub (Win+S / Taskbar Search),
// featuring live acrylic two-pane search filtering, app launch routing,
// settings deep-linking, and quick action previews.
// ============================================================================

#pragma once

#include "surshell/types.hpp"
#include "surshell/compositor.hpp"
#include "surshell/icons.hpp"
#include "surshell/theme.hpp"
#include <string>
#include <vector>
#include <functional>

namespace surshell {

enum class SearchCategoryType {
    All = 0,
    Apps,
    Settings,
    Documents
};

struct SearchItem {
    std::string id;
    std::string title;
    std::string subtitle;
    SearchCategoryType category{SearchCategoryType::Apps};
    IconId icon{IconId::StartPrism};
    std::string targetApp;
    std::string args;
    Rect rowBounds{};
};

using SearchExecuteCallback = std::function<void(const std::string& targetApp, const std::string& args, bool asAdmin)>;

class SearchHub {
public:
    SearchHub();
    ~SearchHub() = default;

    void show() noexcept { visible_ = true; }
    void hide() noexcept { visible_ = false; }
    void toggle() noexcept { visible_ = !visible_; }
    [[nodiscard]] bool isVisible() const noexcept { return visible_; }

    void render(Surface& surface, int32_t screenWidth, int32_t screenHeight);
    bool onMouseDown(Point pt, MouseButton button);
    bool onMouseMove(Point pt);
    bool onKeyDown(KeyCode key);
    bool onCharInput(char c);

    [[nodiscard]] Rect bounds() const noexcept { return bounds_; }

    void setQuery(const std::string& q);
    const std::string& query() const noexcept { return query_; }

    void setExecuteCallback(SearchExecuteCallback cb) { onExecute_ = std::move(cb); }

    void setCategoryFilter(SearchCategoryType cat) { activeFilter_ = cat; updateFilter(); }
    [[nodiscard]] SearchCategoryType activeFilter() const noexcept { return activeFilter_; }

    size_t resultCount() const noexcept { return filteredItems_.size(); }
    int32_t selectedIndex() const noexcept { return selectedIndex_; }

private:
    void updateFilter();
    void executeSelected(bool asAdmin);

    bool visible_{false};
    Rect bounds_{};

    std::string query_;
    SearchCategoryType activeFilter_{SearchCategoryType::All};

    std::vector<SearchItem> allCatalog_;
    std::vector<SearchItem> filteredItems_;
    int32_t selectedIndex_{0};
    int32_t hoveredIndex_{-1};

    // UI Regions
    Rect searchBoxBounds_{};
    Rect btnClearBounds_{};
    Rect tabAll_{};
    Rect tabApps_{};
    Rect tabSettings_{};
    Rect tabDocs_{};

    // Right Preview Action Buttons
    Rect btnOpen_{};
    Rect btnAdmin_{};
    Rect btnLoc_{};
    bool hoverOpen_{false};
    bool hoverAdmin_{false};
    bool hoverLoc_{false};

    SearchExecuteCallback onExecute_;
};

} // namespace surshell
