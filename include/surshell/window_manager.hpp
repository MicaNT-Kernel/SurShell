// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/window_manager.hpp)
//
// Window Manager & Frame Decorator (DWM client), Aero Snap, interactive dragging,
// window state transitions, and Z-order compositor.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <functional>

namespace surshell {

class IWindowContent {
public:
    virtual ~IWindowContent() = default;
    virtual void render(Surface& clientSurface) = 0;
    virtual bool onMouseDown(Point localPt, MouseButton button) { (void)localPt; (void)button; return false; }
    virtual bool onMouseUp(Point localPt, MouseButton button) { (void)localPt; (void)button; return false; }
    virtual bool onMouseMove(Point localPt) { (void)localPt; return false; }
    virtual bool onDoubleClick(Point localPt) { (void)localPt; return false; }
    virtual bool onMouseWheel(Point localPt, int32_t delta) { (void)localPt; (void)delta; return false; }
    virtual bool onCharInput(char c) { (void)c; return false; }
};

struct WindowFrame {
    uint32_t id{0};
    std::string title;
    std::string iconGlyph{"[W]"};
    IconId iconId{IconId::FileGeneric};
    Rect normalBounds{100, 100, 640, 420};
    Rect currentBounds{100, 100, 640, 420};
    WindowState state{WindowState::Normal};
    bool isActive{false};
    bool isVisible{true};
    bool hasMicaEffect{true};

    // Client area surface & attached content
    Surface clientSurface{640, 388, Color{10, 14, 22, 255}};
    std::shared_ptr<IWindowContent> content{};

    [[nodiscard]] Rect captionBounds() const noexcept;
    [[nodiscard]] Rect minButtonBounds() const noexcept;
    [[nodiscard]] Rect maxButtonBounds() const noexcept;
    [[nodiscard]] Rect closeButtonBounds() const noexcept;
    [[nodiscard]] Rect clientAreaBounds() const noexcept;

    [[nodiscard]] HitTestResult hitTest(Point pt) const noexcept;
};

struct SnapZone {
    Rect bounds{};
    WindowState targetState{WindowState::Normal};
    bool isHovered{false};
};

class WindowManager {
public:
    using WindowStateChangedCallback = std::function<void(uint32_t windowId, WindowState state, bool active)>;
    using WindowClosedCallback = std::function<void(uint32_t windowId)>;

    WindowManager(uint32_t screenWidth, uint32_t screenHeight, int32_t taskbarHeight);

    void setScreenSize(uint32_t width, uint32_t height, int32_t taskbarHeight);

    uint32_t createWindow(std::string title, Rect bounds, std::string glyph = "[W]", IconId iconId = IconId::FileGeneric);
    void closeWindow(uint32_t windowId);
    void setWindowActive(uint32_t windowId);
    void setWindowState(uint32_t windowId, WindowState state);
    void toggleMinimize(uint32_t windowId);
    void toggleMaximize(uint32_t windowId);
    void snapWindow(uint32_t windowId, WindowState snapState);

    [[nodiscard]] WindowFrame* findWindow(uint32_t windowId) noexcept;
    [[nodiscard]] const WindowFrame* findWindow(uint32_t windowId) const noexcept;
    [[nodiscard]] const std::vector<std::unique_ptr<WindowFrame>>& windows() const noexcept { return windows_; }
    [[nodiscard]] std::optional<uint32_t> activeWindowId() const noexcept { return activeWindowId_; }

    [[nodiscard]] bool isSnapFlyoutVisible() const noexcept { return isSnapFlyoutVisible_; }
    void showSnapFlyout(uint32_t windowId, Point triggerPt);
    void hideSnapFlyout() noexcept;

    void setCallbacks(WindowStateChangedCallback stateCb, WindowClosedCallback closeCb) {
        stateChangedCb_ = std::move(stateCb);
        closedCb_ = std::move(closeCb);
    }

    // Input Handling
    bool onMouseDown(Point pt, MouseButton button);
    bool onMouseUp(Point pt, MouseButton button);
    bool onMouseMove(Point pt);
    bool onDoubleClick(Point pt);
    bool onMouseWheel(Point pt, int32_t delta);
    bool onCharInput(char c);

    // Compositing
    void render(Surface& surface);

private:
    uint32_t screenWidth_{1920};
    uint32_t screenHeight_{1080};
    int32_t taskbarHeight_{40};
    uint32_t nextWindowId_{1000};
    std::optional<uint32_t> activeWindowId_{};

    std::vector<std::unique_ptr<WindowFrame>> windows_{}; // In Z-order (back to front)

    // Modern Snap Layout Assistant
    bool isSnapFlyoutVisible_{false};
    uint32_t snapFlyoutWindowId_{0};
    Rect snapFlyoutBounds_{};
    std::vector<SnapZone> snapZones_{};

    // Interactive Drag / Resize tracking
    bool isDragging_{false};
    bool isResizing_{false};
    uint32_t interactingWindowId_{0};
    HitTestResult resizeEdge_{HitTestResult::None};
    Point dragStartMouse_{0, 0};
    Rect dragStartWindowBounds_{0, 0, 0, 0};

    WindowStateChangedCallback stateChangedCb_{};
    WindowClosedCallback closedCb_{};

    [[nodiscard]] Rect availableWorkspace() const noexcept;
    void bringToFront(uint32_t windowId);
    void buildSnapZones(Point anchor);
    void renderSnapFlyout(Surface& surface);
};

} // namespace surshell
