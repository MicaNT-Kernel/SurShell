// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/image_viewer.hpp)
//
// Sovereign Photo & Image Viewer (Windows Photos / Viewer Parity)
// Clean-room ISO C++23, zero telemetry, native BMP decoder, zoom, pan,
// sibling folder traversal, rotate, and image diagnostics.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "window_manager.hpp"
#include "theme.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <algorithm>

namespace surshell {

struct ImageToolbarButton {
    std::string id;
    std::string tooltip;
    IconId iconId{IconId::FileGeneric};
    Rect bounds{};
    bool isHovered{false};
};

class ImageViewerContent : public IWindowContent {
public:
    explicit ImageViewerContent(std::string imagePath = "");

    void loadImage(const std::string& imagePath);
    void nextImage();
    void prevImage();
    void zoomIn();
    void zoomOut();
    void zoomActual();
    void zoomFit();
    void rotateClockwise();

    [[nodiscard]] const std::string& currentImagePath() const noexcept { return currentPath_; }
    [[nodiscard]] const std::string& currentFileName() const noexcept { return fileName_; }
    [[nodiscard]] float zoom() const noexcept { return zoomScale_; }
    [[nodiscard]] bool hasImage() const noexcept { return imageSurface_.has_value(); }
    [[nodiscard]] uint32_t imageWidth() const noexcept { return imageSurface_ ? imageSurface_->width() : 0; }
    [[nodiscard]] uint32_t imageHeight() const noexcept { return imageSurface_ ? imageSurface_->height() : 0; }
    [[nodiscard]] size_t siblingCount() const noexcept { return siblingFiles_.size(); }
    [[nodiscard]] size_t siblingIndex() const noexcept { return currentSiblingIndex_; }

    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseUp(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onMouseWheel(Point localPt, int32_t delta) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;

private:
    std::string currentPath_{};
    std::string fileName_{"No Image Loaded"};
    std::optional<Surface> imageSurface_{};
    uint64_t fileSizeBytes_{0};
    int32_t rotationAngle_{0}; // 0, 90, 180, 270

    float zoomScale_{1.0f};
    bool isFitMode_{true};
    Point panOffset_{0, 0};

    // Drag-panning
    bool isDragging_{false};
    Point dragStartMouse_{0, 0};
    Point dragStartPan_{0, 0};

    // Sibling folder files for Next/Prev
    std::vector<std::string> siblingFiles_{};
    size_t currentSiblingIndex_{0};

    std::vector<ImageToolbarButton> toolbarButtons_{};
    Rect toolbarBounds_{};

    void discoverSiblingFiles();
    void setupToolbar();
    void recalculateFit(uint32_t viewW, uint32_t viewH);
    void drawCheckerboard(Surface& s, Rect r) const;
};

} // namespace surshell
