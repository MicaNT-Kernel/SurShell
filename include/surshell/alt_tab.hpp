// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/alt_tab.hpp)
//
// Alt+Tab Task Switcher HUD with Live Thumbnail Carousel, Vector Badges,
// and Keyboard / Mouse Navigation.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <algorithm>

namespace surshell {

struct AltTabItem {
    uint32_t windowId{0};
    std::string title;
    std::string iconGlyph{"[W]"};
    IconId iconId{IconId::FileGeneric};
    bool isActive{false};
    bool isMinimized{false};
    const Surface* previewSurface{nullptr};
};

class AltTabSwitcher {
public:
    AltTabSwitcher() = default;

    void show(std::vector<AltTabItem> items, size_t initialIndex = 1);
    void dismiss() noexcept;
    [[nodiscard]] std::optional<uint32_t> confirm() noexcept;

    void next() noexcept;
    void previous() noexcept;
    void selectIndex(size_t index) noexcept;

    [[nodiscard]] bool isActive() const noexcept { return isVisible_; }
    [[nodiscard]] size_t selectedIndex() const noexcept { return selectedIndex_; }
    [[nodiscard]] size_t itemCount() const noexcept { return items_.size(); }
    [[nodiscard]] const std::vector<AltTabItem>& items() const noexcept { return items_; }
    [[nodiscard]] const AltTabItem* selectedItem() const noexcept;

    [[nodiscard]] Rect calculateHudBounds(uint32_t screenWidth, uint32_t screenHeight) const noexcept;
    [[nodiscard]] Rect calculateCardBounds(size_t index, Rect hudBounds) const noexcept;

    // Input Handling
    std::optional<uint32_t> onMouseDown(Point pt, MouseButton button, uint32_t screenWidth, uint32_t screenHeight);
    bool onMouseMove(Point pt, uint32_t screenWidth, uint32_t screenHeight);

    // Rendering
    void render(Surface& target, uint32_t screenWidth, uint32_t screenHeight) const;

private:
    bool isVisible_{false};
    size_t selectedIndex_{0};
    int32_t hoveredIndex_{-1};
    std::vector<AltTabItem> items_{};

    static constexpr int32_t CARD_WIDTH = 220;
    static constexpr int32_t CARD_HEIGHT = 160;
    static constexpr int32_t CARD_SPACING = 14;
    static constexpr int32_t PAD_X = 20;
    static constexpr int32_t PAD_Y = 24;
    static constexpr int32_t HEADER_HEIGHT = 30;
};

} // namespace surshell
