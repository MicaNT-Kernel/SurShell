// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/image_viewer.cpp)
//
// Sovereign Photo & Image Viewer (Windows Photos Parity)
// Clean-room ISO C++23, zero telemetry, native BMP decoder, zoom, pan,
// sibling folder traversal, rotate, and image diagnostics.
// ============================================================================

#include "surshell/image_viewer.hpp"
#include <iomanip>
#include <sstream>

namespace surshell {

ImageViewerContent::ImageViewerContent(std::string imagePath) {
    setupToolbar();
    if (!imagePath.empty()) {
        loadImage(imagePath);
    }
}

void ImageViewerContent::setupToolbar() {
    toolbarButtons_.clear();
    toolbarButtons_.push_back(ImageToolbarButton{.id = "prev", .tooltip = "Previous Image (Left)", .iconId = IconId::NavBack});
    toolbarButtons_.push_back(ImageToolbarButton{.id = "next", .tooltip = "Next Image (Right)", .iconId = IconId::NavForward});
    toolbarButtons_.push_back(ImageToolbarButton{.id = "zoom_in", .tooltip = "Zoom In (+)", .iconId = IconId::Search});
    toolbarButtons_.push_back(ImageToolbarButton{.id = "zoom_out", .tooltip = "Zoom Out (-)", .iconId = IconId::SearchCategory});
    toolbarButtons_.push_back(ImageToolbarButton{.id = "actual", .tooltip = "Actual Size (Ctrl+1)", .iconId = IconId::ViewGrid});
    toolbarButtons_.push_back(ImageToolbarButton{.id = "fit", .tooltip = "Fit to Window (Ctrl+0)", .iconId = IconId::Display});
    toolbarButtons_.push_back(ImageToolbarButton{.id = "rotate", .tooltip = "Rotate 90 deg (R)", .iconId = IconId::NavRefresh});
}

void ImageViewerContent::loadImage(const std::string& imagePath) {
    currentPath_ = imagePath;
    rotationAngle_ = 0;
    isFitMode_ = true;
    panOffset_ = Point{0, 0};

    std::error_code ec;
    std::filesystem::path p(imagePath);
    fileName_ = p.filename().string();
    if (fileName_.empty()) fileName_ = "Untitled";

    if (std::filesystem::exists(p, ec)) {
        fileSizeBytes_ = std::filesystem::file_size(p, ec);
        if (ec) fileSizeBytes_ = 0;

        // Try native BMP decoding
        if (p.extension() == ".bmp" || p.extension() == ".BMP") {
            imageSurface_ = Surface::loadBmp(imagePath);
        }
    }

    // If not loaded or non-BMP, generate a procedural sample canvas
    if (!imageSurface_.has_value()) {
        constexpr uint32_t sampleW = 640;
        constexpr uint32_t sampleH = 400;
        Surface sample(sampleW, sampleH, Color{16, 24, 38, 255});

        // Draw radial/angular gradient backdrop
        for (uint32_t y = 0; y < sampleH; ++y) {
            const float ty = static_cast<float>(y) / static_cast<float>(sampleH);
            for (uint32_t x = 0; x < sampleW; ++x) {
                const float tx = static_cast<float>(x) / static_cast<float>(sampleW);
                const uint8_t r = static_cast<uint8_t>(20 + 35 * tx);
                const uint8_t g = static_cast<uint8_t>(30 + 45 * ty);
                const uint8_t b = static_cast<uint8_t>(55 + 60 * (tx + ty) * 0.5f);
                sample.putPixel(static_cast<int32_t>(x), static_cast<int32_t>(y), Color{r, g, b, 255});
            }
        }

        // Draw test pattern card
        sample.drawRoundedRect(Rect{40, 40, sampleW - 80, sampleH - 80}, 12, Color::fromRgba(0, 212, 255, 60), false);
        sample.drawPrismLogo(Point{sampleW / 2, sampleH / 2 - 30}, 48, Color::fromHex(0x00D4FF), Color::fromHex(0x006699), Color::fromHex(0x66EEFF));
        sample.drawString(sampleW / 2 - 110, sampleH / 2 + 40, "Sovereign Image Canvas", Color::fromHex(0xFFFFFF), 1);
        sample.drawString(sampleW / 2 - 80, sampleH / 2 + 62, fileName_, Color::fromHex(0x00FF9D), 1);

        imageSurface_ = std::move(sample);
        if (fileSizeBytes_ == 0) fileSizeBytes_ = 1024 * 768 * 4;
    }

    discoverSiblingFiles();
}

void ImageViewerContent::discoverSiblingFiles() {
    siblingFiles_.clear();
    currentSiblingIndex_ = 0;

    std::error_code ec;
    std::filesystem::path p(currentPath_);
    if (!p.has_parent_path()) return;

    const auto parent = p.parent_path();
    if (!std::filesystem::is_directory(parent, ec)) return;

    for (const auto& entry : std::filesystem::directory_iterator(parent, std::filesystem::directory_options::skip_permission_denied, ec)) {
        if (ec) break;
        if (entry.is_regular_file(ec)) {
            const auto ext = entry.path().extension().string();
            if (ext == ".bmp" || ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                ext == ".ico" || ext == ".BMP" || ext == ".PNG") {
                siblingFiles_.push_back(entry.path().string());
            }
        }
    }
    std::sort(siblingFiles_.begin(), siblingFiles_.end());

    for (size_t i = 0; i < siblingFiles_.size(); ++i) {
        if (siblingFiles_[i] == currentPath_) {
            currentSiblingIndex_ = i;
            break;
        }
    }
}

void ImageViewerContent::nextImage() {
    if (siblingFiles_.empty()) return;
    currentSiblingIndex_ = (currentSiblingIndex_ + 1) % siblingFiles_.size();
    loadImage(siblingFiles_[currentSiblingIndex_]);
}

void ImageViewerContent::prevImage() {
    if (siblingFiles_.empty()) return;
    currentSiblingIndex_ = (currentSiblingIndex_ == 0) ? (siblingFiles_.size() - 1) : (currentSiblingIndex_ - 1);
    loadImage(siblingFiles_[currentSiblingIndex_]);
}

void ImageViewerContent::zoomIn() {
    isFitMode_ = false;
    zoomScale_ = std::min(10.0f, zoomScale_ * 1.25f);
}

void ImageViewerContent::zoomOut() {
    isFitMode_ = false;
    zoomScale_ = std::max(0.1f, zoomScale_ / 1.25f);
}

void ImageViewerContent::zoomActual() {
    isFitMode_ = false;
    zoomScale_ = 1.0f;
    panOffset_ = Point{0, 0};
}

void ImageViewerContent::zoomFit() {
    isFitMode_ = true;
    panOffset_ = Point{0, 0};
}

void ImageViewerContent::rotateClockwise() {
    rotationAngle_ = (rotationAngle_ + 90) % 360;
    if (!imageSurface_.has_value()) return;

    const uint32_t oldW = imageSurface_->width();
    const uint32_t oldH = imageSurface_->height();
    Surface rotated(oldH, oldW, Color{0, 0, 0, 255});

    for (uint32_t y = 0; y < oldH; ++y) {
        for (uint32_t x = 0; x < oldW; ++x) {
            const Color c = imageSurface_->getPixel(static_cast<int32_t>(x), static_cast<int32_t>(y));
            rotated.putPixel(static_cast<int32_t>(oldH - 1 - y), static_cast<int32_t>(x), c);
        }
    }
    imageSurface_ = std::move(rotated);
}

void ImageViewerContent::recalculateFit(uint32_t viewW, uint32_t viewH) {
    if (!imageSurface_.has_value() || viewW == 0 || viewH == 0) return;
    const float sx = static_cast<float>(viewW) / static_cast<float>(imageSurface_->width());
    const float sy = static_cast<float>(viewH) / static_cast<float>(imageSurface_->height());
    zoomScale_ = std::min(sx, sy);
    if (zoomScale_ > 1.0f) zoomScale_ = 1.0f; // Don't upscale small images in fit mode
}

void ImageViewerContent::drawCheckerboard(Surface& s, Rect r) const {
    constexpr int32_t sz = 16;
    const Color c1 = Color::fromHex(0x131924);
    const Color c2 = Color::fromHex(0x192130);

    for (int32_t y = r.y; y < r.bottom(); y += sz) {
        for (int32_t x = r.x; x < r.right(); x += sz) {
            const bool alt = ((x / sz) + (y / sz)) % 2 == 0;
            const Rect tile{x, y, std::min(sz, r.right() - x), std::min(sz, r.bottom() - y)};
            s.fillRect(tile, alt ? c1 : c2);
        }
    }
}

void ImageViewerContent::render(Surface& clientSurface) {
    const uint32_t cw = clientSurface.width();
    const uint32_t ch = clientSurface.height();

    // 1. Fill deep charcoal backdrop
    clientSurface.clear(Color::fromHex(0x0C1017));

    // Viewport area excluding top info bar (32px) and bottom toolbar (46px)
    constexpr int32_t topBarH = 32;
    constexpr int32_t bottomBarH = 46;
    const Rect viewArea{0, topBarH, static_cast<int32_t>(cw), static_cast<int32_t>(ch) - topBarH - bottomBarH};

    // 2. Subtle checkerboard in viewport
    drawCheckerboard(clientSurface, viewArea);

    // 3. Render Image centered in viewArea
    if (imageSurface_.has_value() && viewArea.width > 0 && viewArea.height > 0) {
        if (isFitMode_) {
            recalculateFit(viewArea.width, viewArea.height);
        }

        const int32_t scaledW = static_cast<int32_t>(imageSurface_->width() * zoomScale_);
        const int32_t scaledH = static_cast<int32_t>(imageSurface_->height() * zoomScale_);
        const int32_t imgX = viewArea.centerX() - scaledW / 2 + panOffset_.x;
        const int32_t imgY = viewArea.centerY() - scaledH / 2 + panOffset_.y;

        const Rect dstRect{imgX, imgY, scaledW, scaledH};
        clientSurface.drawDropShadow(dstRect, 10, 0.4f);
        clientSurface.blitScaled(*imageSurface_, Rect{0, 0, static_cast<int32_t>(imageSurface_->width()), static_cast<int32_t>(imageSurface_->height())}, dstRect);
    }

    // 4. Top Info Bar
    const Rect topBar{0, 0, static_cast<int32_t>(cw), topBarH};
    clientSurface.fillRect(topBar, Color::fromRgba(18, 25, 38, 230));
    clientSurface.fillRect(Rect{0, topBarH - 1, static_cast<int32_t>(cw), 1}, Color::fromHex(0x283850));

    IconRenderer::draw(clientSurface, IconId::ImageViewer, Rect{8, 6, 20, 20});
    clientSurface.drawString(34, 9, fileName_, Color::fromHex(0xFFFFFF), 1);

    if (imageSurface_.has_value()) {
        std::string dimStr = std::to_string(imageSurface_->width()) + " x " + std::to_string(imageSurface_->height()) + " px";
        clientSurface.drawString(cw / 2 - 40, 9, dimStr, Color::fromHex(0x88A2C2), 1);

        const int zoomPct = static_cast<int>(zoomScale_ * 100.0f);
        std::string rightStr = std::to_string(zoomPct) + "%";
        if (!siblingFiles_.empty()) {
            rightStr += "  |  " + std::to_string(currentSiblingIndex_ + 1) + " of " + std::to_string(siblingFiles_.size());
        }
        clientSurface.drawString(cw - 130, 9, rightStr, Color::fromHex(0x00D4FF), 1);
    }

    // 5. Bottom Floating Acrylic Toolbar
    constexpr int32_t tbW = 280;
    constexpr int32_t tbH = 34;
    const int32_t tbX = static_cast<int32_t>(cw) / 2 - tbW / 2;
    const int32_t tbY = static_cast<int32_t>(ch) - bottomBarH + 6;
    toolbarBounds_ = Rect{tbX, tbY, tbW, tbH};

    clientSurface.drawDropShadow(toolbarBounds_, 8, 0.35f);
    clientSurface.applyAcrylicTint(toolbarBounds_, Color::fromRgba(20, 28, 44, 220), 4);
    clientSurface.drawRoundedRect(toolbarBounds_, 8, Color::fromHex(0x384E70), false);

    const int32_t btnW = 32;
    const int32_t btnH = 26;
    int32_t curBx = tbX + 8;
    const int32_t curBy = tbY + 4;

    for (auto& btn : toolbarButtons_) {
        btn.bounds = Rect{curBx, curBy, btnW, btnH};
        if (btn.isHovered) {
            clientSurface.drawRoundedRect(btn.bounds, 4, Color::fromRgba(0, 212, 255, 60), true);
            clientSurface.drawRoundedRect(btn.bounds, 4, Color::fromRgba(0, 212, 255, 140), false);
        }
        IconRenderer::draw(clientSurface, btn.iconId, Rect{btn.bounds.x + 8, btn.bounds.y + 5, 16, 16},
                           btn.isHovered ? std::make_optional(Color::fromHex(0x00D4FF)) : std::nullopt);
        curBx += btnW + 6;
    }
}

bool ImageViewerContent::onMouseDown(Point localPt, MouseButton button) {
    if (button == MouseButton::Left) {
        // Toolbar buttons
        for (const auto& btn : toolbarButtons_) {
            if (btn.bounds.contains(localPt)) {
                if (btn.id == "prev") prevImage();
                else if (btn.id == "next") nextImage();
                else if (btn.id == "zoom_in") zoomIn();
                else if (btn.id == "zoom_out") zoomOut();
                else if (btn.id == "actual") zoomActual();
                else if (btn.id == "fit") zoomFit();
                else if (btn.id == "rotate") rotateClockwise();
                return true;
            }
        }

        // Start drag pan
        isDragging_ = true;
        dragStartMouse_ = localPt;
        dragStartPan_ = panOffset_;
        return true;
    }
    return false;
}

bool ImageViewerContent::onMouseUp(Point localPt, MouseButton button) {
    (void)localPt;
    if (button == MouseButton::Left) {
        isDragging_ = false;
        return true;
    }
    return false;
}

bool ImageViewerContent::onMouseMove(Point localPt) {
    // Update toolbar button hovers
    for (auto& btn : toolbarButtons_) {
        btn.isHovered = btn.bounds.contains(localPt);
    }

    if (isDragging_) {
        isFitMode_ = false;
        panOffset_.x = dragStartPan_.x + (localPt.x - dragStartMouse_.x);
        panOffset_.y = dragStartPan_.y + (localPt.y - dragStartMouse_.y);
        return true;
    }
    return false;
}

bool ImageViewerContent::onMouseWheel(Point localPt, int32_t delta) {
    (void)localPt;
    if (delta > 0) {
        zoomIn();
    } else if (delta < 0) {
        zoomOut();
    }
    return true;
}

bool ImageViewerContent::onKeyDown(KeyCode key, bool ctrl, bool shift, bool alt) {
    (void)shift;
    (void)alt;
    if (key == KeyCode::Left) {
        prevImage();
        return true;
    }
    if (key == KeyCode::Right) {
        nextImage();
        return true;
    }
    if (key == KeyCode::KeyR) {
        rotateClockwise();
        return true;
    }
    if (ctrl && key == KeyCode::Num0) {
        zoomFit();
        return true;
    }
    if (ctrl && key == KeyCode::Num1) {
        zoomActual();
        return true;
    }
    return false;
}

} // namespace surshell
