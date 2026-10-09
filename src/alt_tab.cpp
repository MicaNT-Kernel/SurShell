// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/alt_tab.cpp)
//
// Alt+Tab Task Switcher HUD with Live Thumbnail Carousel, Vector Badges,
// and Keyboard / Mouse Navigation.
// ============================================================================

#include "surshell/alt_tab.hpp"
#include <algorithm>

namespace surshell {

void AltTabSwitcher::show(std::vector<AltTabItem> items, size_t initialIndex) {
    if (items.empty()) {
        dismiss();
        return;
    }
    items_ = std::move(items);
    isVisible_ = true;
    hoveredIndex_ = -1;

    if (items_.size() == 1) {
        selectedIndex_ = 0;
    } else if (initialIndex < items_.size()) {
        selectedIndex_ = initialIndex;
    } else {
        selectedIndex_ = 0;
    }
}

void AltTabSwitcher::dismiss() noexcept {
    isVisible_ = false;
    items_.clear();
    selectedIndex_ = 0;
    hoveredIndex_ = -1;
}

std::optional<uint32_t> AltTabSwitcher::confirm() noexcept {
    if (!isVisible_ || items_.empty()) {
        return std::nullopt;
    }
    const uint32_t chosenId = items_[selectedIndex_].windowId;
    dismiss();
    return chosenId;
}

void AltTabSwitcher::next() noexcept {
    if (items_.empty()) return;
    selectedIndex_ = (selectedIndex_ + 1) % items_.size();
}

void AltTabSwitcher::previous() noexcept {
    if (items_.empty()) return;
    selectedIndex_ = (selectedIndex_ + items_.size() - 1) % items_.size();
}

void AltTabSwitcher::selectIndex(size_t index) noexcept {
    if (index < items_.size()) {
        selectedIndex_ = index;
    }
}

const AltTabItem* AltTabSwitcher::selectedItem() const noexcept {
    if (!items_.empty() && selectedIndex_ < items_.size()) {
        return &items_[selectedIndex_];
    }
    return nullptr;
}

Rect AltTabSwitcher::calculateHudBounds(uint32_t screenWidth, uint32_t screenHeight) const noexcept {
    const size_t n = std::max(size_t{1}, items_.size());
    const int32_t contentWidth = static_cast<int32_t>(n) * CARD_WIDTH + static_cast<int32_t>(n - 1) * CARD_SPACING;
    const int32_t hudWidth = std::min(static_cast<int32_t>(screenWidth) - 60, contentWidth + PAD_X * 2);
    const int32_t hudHeight = CARD_HEIGHT + PAD_Y * 2 + HEADER_HEIGHT;
    const int32_t hudX = (static_cast<int32_t>(screenWidth) - hudWidth) / 2;
    const int32_t hudY = (static_cast<int32_t>(screenHeight) - hudHeight) / 2;

    return Rect{hudX, hudY, hudWidth, hudHeight};
}

Rect AltTabSwitcher::calculateCardBounds(size_t index, Rect hudBounds) const noexcept {
    const int32_t startX = hudBounds.x + PAD_X;
    const int32_t cardY = hudBounds.y + PAD_Y + HEADER_HEIGHT;
    const int32_t cardX = startX + static_cast<int32_t>(index) * (CARD_WIDTH + CARD_SPACING);

    return Rect{cardX, cardY, CARD_WIDTH, CARD_HEIGHT};
}

std::optional<uint32_t> AltTabSwitcher::onMouseDown(Point pt, MouseButton button, uint32_t screenWidth, uint32_t screenHeight) {
    if (!isVisible_ || button != MouseButton::Left) return std::nullopt;

    const Rect hud = calculateHudBounds(screenWidth, screenHeight);
    if (!hud.contains(pt)) {
        dismiss();
        return std::nullopt;
    }

    for (size_t i = 0; i < items_.size(); ++i) {
        if (calculateCardBounds(i, hud).contains(pt)) {
            selectIndex(i);
            return confirm();
        }
    }

    return std::nullopt;
}

bool AltTabSwitcher::onMouseMove(Point pt, uint32_t screenWidth, uint32_t screenHeight) {
    if (!isVisible_) return false;

    const Rect hud = calculateHudBounds(screenWidth, screenHeight);
    hoveredIndex_ = -1;

    for (size_t i = 0; i < items_.size(); ++i) {
        if (calculateCardBounds(i, hud).contains(pt)) {
            hoveredIndex_ = static_cast<int32_t>(i);
            return true;
        }
    }

    return hud.contains(pt);
}

void AltTabSwitcher::render(Surface& target, uint32_t screenWidth, uint32_t screenHeight) const {
    if (!isVisible_ || items_.empty()) return;

    const Rect hud = calculateHudBounds(screenWidth, screenHeight);

    // 1. Elevated HUD Drop Shadow
    target.drawDropShadow(hud, 24, 0.60f);

    // 2. Acrylic Container Tint & Frosted Edge
    target.applyAcrylicTint(hud, Color::fromRgba(13, 19, 31, 235), 10);
    target.drawRoundedRect(hud, 14, Color::fromRgba(42, 58, 86, 230), false);

    // 3. Header Diagnostic / Shortcut Strip
    target.drawString(hud.x + PAD_X, hud.y + 12, "Task Switcher", Color::fromHex(0x00D4FF), 1);
    target.drawString(hud.x + PAD_X + 130, hud.y + 12,
                      "|  Tab to cycle  |  Release to switch  |  Esc to cancel",
                      Color::fromRgba(150, 170, 200, 240), 1);

    // 4. Render Horizontal Cards
    for (size_t i = 0; i < items_.size(); ++i) {
        const Rect cb = calculateCardBounds(i, hud);
        const bool isSelected = (i == selectedIndex_);
        const bool isHovered = (static_cast<int32_t>(i) == hoveredIndex_);

        // Card Fill
        Color cardBg = isSelected ? Color::fromRgba(25, 38, 62, 240)
                                  : (isHovered ? Color::fromRgba(22, 32, 50, 220)
                                               : Color::fromRgba(18, 25, 40, 200));
        target.drawRoundedRect(cb, 10, cardBg, true);

        // Selection Glowing Ring (Barrer Cyan #00D4FF 2px)
        if (isSelected) {
            target.drawRoundedRect(cb, 10, Color::fromHex(0x00D4FF), false);
            target.drawRoundedRect(cb.inflate(-1, -1), 9, Color::fromHex(0x00D4FF), false);

            // Active underline indicator
            target.drawRoundedRect(Rect{cb.centerX() - 16, cb.bottom() - 4, 32, 2}, 1, Color::fromHex(0x00D4FF), true);
        } else {
            target.drawRoundedRect(cb, 10, Color::fromRgba(40, 54, 78, 160), false);
        }

        // Header: Vector App Icon + Truncated Title
        const Rect iconRect{cb.x + 10, cb.y + 8, 18, 18};
        IconRenderer::draw(target, items_[i].iconId, iconRect, isSelected ? Color::fromHex(0x00D4FF) : Color::fromRgba(200, 215, 235, 255));

        std::string title = items_[i].title;
        if (title.size() > 18) {
            title = title.substr(0, 16) + "..";
        }
        target.drawString(cb.x + 34, cb.y + 13, title,
                          isSelected ? Color::fromHex(0xFFFFFF) : Color::fromRgba(220, 230, 245, 255), 1);

        // Preview Box (downscaled live client surface)
        const Rect previewRect{cb.x + 10, cb.y + 34, cb.width - 20, cb.height - 44};
        target.fillRect(previewRect, Color::fromRgba(8, 12, 18, 255));
        target.drawRoundedRect(previewRect, 4, Color::fromRgba(35, 48, 70, 180), false);

        if (items_[i].previewSurface && items_[i].previewSurface->width() > 0 && items_[i].previewSurface->height() > 0) {
            const Rect innerPrev = previewRect.inflate(-2, -2);
            target.blitScaled(*items_[i].previewSurface,
                              Rect{0, 0, static_cast<int32_t>(items_[i].previewSurface->width()), static_cast<int32_t>(items_[i].previewSurface->height())},
                              innerPrev);
        } else {
            // Elegant placeholder icon
            IconRenderer::draw(target, items_[i].iconId,
                               Rect{previewRect.centerX() - 16, previewRect.centerY() - 16, 32, 32},
                               Color::fromRgba(80, 105, 140, 200));
        }

        // Minimized Indicator Badge
        if (items_[i].isMinimized) {
            const Rect minBadge{previewRect.right() - 42, previewRect.y + 4, 38, 14};
            target.drawRoundedRect(minBadge, 2, Color::fromRgba(25, 35, 50, 220), true);
            target.drawString(minBadge.x + 4, minBadge.y + 3, "MIN", Color::fromRgba(160, 180, 200, 220), 1);
        }
    }
}

} // namespace surshell
