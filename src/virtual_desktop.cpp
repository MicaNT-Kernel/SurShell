// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/virtual_desktop.cpp)
// ============================================================================

#include "surshell/virtual_desktop.hpp"
#include <algorithm>

namespace surshell {

VirtualDesktopManager::VirtualDesktopManager() {
    desktops_.push_back(VirtualDesktop{
        .id = nextId_++,
        .name = "1: Sovereign NT",
        .windowIds = {}
    });
    desktops_.push_back(VirtualDesktop{
        .id = nextId_++,
        .name = "2: Dev & Tools",
        .windowIds = {}
    });
}

void VirtualDesktopManager::updateLayout(int32_t screenWidth, int32_t screenHeight, int32_t taskbarHeight) {
    const size_t totalSlots = desktops_.size() + 1; // Desktops + "Add" card
    constexpr int32_t cardWidth = 110;
    constexpr int32_t cardHeight = 70;
    constexpr int32_t cardSpacing = 12;
    constexpr int32_t padding = 16;

    const int32_t stripWidth = static_cast<int32_t>(totalSlots * (cardWidth + cardSpacing) - cardSpacing + padding * 2);
    constexpr int32_t stripHeight = 116;

    switcherBounds_ = Rect{
        (screenWidth - stripWidth) / 2,
        screenHeight - taskbarHeight - stripHeight - 16,
        stripWidth,
        stripHeight
    };

    int32_t curX = switcherBounds_.x + padding;
    const int32_t curY = switcherBounds_.y + 32;

    for (size_t i = 0; i < desktops_.size(); ++i) {
        desktops_[i].switcherCardBounds = Rect{curX, curY, cardWidth, cardHeight};
        curX += cardWidth + cardSpacing;
    }

    addDesktopButtonBounds_ = Rect{curX, curY, cardWidth, cardHeight};
}

void VirtualDesktopManager::switchDesktop(size_t index) {
    if (index >= desktops_.size() || index == activeIndex_) return;
    activeIndex_ = index;
    if (onDesktopSwitched_) onDesktopSwitched_(activeIndex_);
}

void VirtualDesktopManager::nextDesktop() {
    if (desktops_.size() <= 1) return;
    switchDesktop((activeIndex_ + 1) % desktops_.size());
}

void VirtualDesktopManager::previousDesktop() {
    if (desktops_.size() <= 1) return;
    switchDesktop((activeIndex_ + desktops_.size() - 1) % desktops_.size());
}

uint32_t VirtualDesktopManager::createDesktop(std::string_view name) {
    const uint32_t id = nextId_++;
    std::string finalName;
    if (name.empty()) {
        finalName = "Desktop " + std::to_string(desktops_.size() + 1);
    } else {
        finalName = std::string(name);
    }

    desktops_.push_back(VirtualDesktop{
        .id = id,
        .name = std::move(finalName),
        .windowIds = {}
    });

    return id;
}

bool VirtualDesktopManager::removeDesktop(size_t index) {
    if (desktops_.size() <= 1 || index >= desktops_.size()) return false;

    // Move windows from removed desktop to active or first desktop
    const size_t targetIdx = (index == 0) ? 1 : 0;
    for (uint32_t wid : desktops_[index].windowIds) {
        desktops_[targetIdx].windowIds.insert(wid);
    }

    desktops_.erase(desktops_.begin() + static_cast<ptrdiff_t>(index));
    if (activeIndex_ >= desktops_.size()) {
        activeIndex_ = desktops_.size() - 1;
    }

    if (onDesktopSwitched_) onDesktopSwitched_(activeIndex_);
    return true;
}

void VirtualDesktopManager::assignWindowToDesktop(uint32_t windowId, size_t desktopIndex) {
    if (desktopIndex >= desktops_.size()) return;
    unassignWindow(windowId);
    desktops_[desktopIndex].windowIds.insert(windowId);
}

void VirtualDesktopManager::unassignWindow(uint32_t windowId) {
    for (auto& d : desktops_) {
        d.windowIds.erase(windowId);
    }
}

void VirtualDesktopManager::pinWindowToAllDesktops(uint32_t windowId, bool pinned) {
    if (pinned) {
        pinnedWindows_.insert(windowId);
    } else {
        pinnedWindows_.erase(windowId);
    }
}

bool VirtualDesktopManager::isWindowVisible(uint32_t windowId) const {
    if (pinnedWindows_.contains(windowId)) return true;
    if (activeIndex_ < desktops_.size()) {
        return desktops_[activeIndex_].windowIds.contains(windowId);
    }
    return true;
}

bool VirtualDesktopManager::onMouseDown(Point pt, MouseButton btn) {
    if (!isSwitcherVisible_ || btn != MouseButton::Left) return false;
    if (!switcherBounds_.contains(pt)) {
        hideSwitcher();
        return false;
    }

    // Check clicking desktop cards
    for (size_t i = 0; i < desktops_.size(); ++i) {
        if (desktops_[i].switcherCardBounds.contains(pt)) {
            switchDesktop(i);
            hideSwitcher();
            return true;
        }
    }

    // Check Add Desktop button
    if (addDesktopButtonBounds_.contains(pt)) {
        createDesktop();
        return true;
    }

    return true;
}

bool VirtualDesktopManager::onMouseMove(Point pt) {
    if (!isSwitcherVisible_) return false;
    return switcherBounds_.contains(pt);
}

void VirtualDesktopManager::renderSwitcher(Surface& surface, const ThemePalette& theme) {
    if (!isSwitcherVisible_) return;

    // 1. Soft Drop Shadow
    surface.drawDropShadow(switcherBounds_, 16, 0.45f);

    // 2. Translucent Mica Acrylic Sub-Surface Blur & Card
    surface.applyAcrylicTint(switcherBounds_, theme.startMenuBg, 8);
    surface.drawRoundedRect(switcherBounds_, 14, theme.startMenuBorder, false);

    // Title Header
    surface.drawString(switcherBounds_.x + 18, switcherBounds_.y + 12, "Task View / Virtual Desktops", theme.textPrimary, 1);

    // 3. Desktop Cards
    for (size_t i = 0; i < desktops_.size(); ++i) {
        const auto& d = desktops_[i];
        const bool isActive = (i == activeIndex_);

        const Color bg = isActive ? theme.startCardHover : theme.startCardBg;
        const Color border = isActive ? theme.accentColor : theme.startCardBorder;

        surface.drawRoundedRect(d.switcherCardBounds, 8, bg, true);
        surface.drawRoundedRect(d.switcherCardBounds, 8, border, false);

        // Desktop Name & Window Count
        surface.drawString(d.switcherCardBounds.x + 8, d.switcherCardBounds.y + 12, d.name, theme.textPrimary, 1);
        const std::string winCountStr = std::to_string(d.windowIds.size()) + " Windows";
        surface.drawString(d.switcherCardBounds.x + 8, d.switcherCardBounds.y + 30, winCountStr, theme.textSecondary, 1);

        // Active indicator pill
        if (isActive) {
            surface.drawRoundedRect(Rect{d.switcherCardBounds.centerX() - 16, d.switcherCardBounds.bottom() - 6, 32, 3}, 2, theme.accentColor, true);
        }
    }

    // 4. Add Desktop Button Card
    surface.drawRoundedRect(addDesktopButtonBounds_, 8, theme.startCardBg, true);
    surface.drawRoundedRect(addDesktopButtonBounds_, 8, theme.startCardBorder, false);
    surface.drawString(addDesktopButtonBounds_.centerX() - 36, addDesktopButtonBounds_.centerY() - 6, "+ New", theme.accentColor, 1);
}

} // namespace surshell
