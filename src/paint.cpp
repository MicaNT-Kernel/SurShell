// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/paint.cpp)
//
// Sovereign Paint & Vector Canvas Studio (paint.exe / mspaint Parity)
// Clean-room ISO C++23, zero telemetry, pixel drawing, shape rasterization,
// flood fill, color palette, undo/redo stack, and BMP import/export.
// ============================================================================

#include "surshell/paint.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <filesystem>

namespace surshell {

PaintContent::PaintContent(std::string filePath) {
    presetColors_ = {
        // Row 1: Dark / Primary Spectrum
        Color::fromHex(0x000000), // Black
        Color::fromHex(0x334155), // Dark Slate
        Color::fromHex(0x64748B), // Slate Gray
        Color::fromHex(0x880015), // Deep Crimson
        Color::fromHex(0xEF4444), // Red
        Color::fromHex(0xF97316), // Orange
        Color::fromHex(0xF59E0B), // Amber Gold
        Color::fromHex(0xFACC15), // Sunshine Yellow
        Color::fromHex(0x15803D), // Forest Green
        Color::fromHex(0x00FF9D), // Neon Green
        Color::fromHex(0x00D4FF), // Sovereign Cyan
        Color::fromHex(0x1D4ED8), // Deep Blue

        // Row 2: Light / Secondary Spectrum
        Color::fromHex(0xFFFFFF), // White
        Color::fromHex(0xCBD5E1), // Light Silver
        Color::fromHex(0x94A3B8), // Medium Gray
        Color::fromHex(0x9A3412), // Rust Brown
        Color::fromHex(0xFF5C5C), // Sunset Coral
        Color::fromHex(0xF43F5E), // Rose Pink
        Color::fromHex(0xFEF08A), // Cream Yellow
        Color::fromHex(0x84CC16), // Lime Green
        Color::fromHex(0x0D9488), // Mint Teal
        Color::fromHex(0x38BDF8), // Sky Blue
        Color::fromHex(0x6366F1), // Royal Indigo
        Color::fromHex(0xD946EF)  // Neon Magenta
    };

    if (!filePath.empty()) {
        loadFromFile(filePath);
    } else {
        newCanvas(520, 340, Color{255, 255, 255, 255});
    }
}

void PaintContent::newCanvas(uint32_t width, uint32_t height, Color bgColor) {
    canvas_ = Surface(std::max(16u, width), std::max(16u, height), bgColor);
    undoStack_.clear();
    redoStack_.clear();
    currentFilePath_.clear();
    fileName_ = "Untitled.bmp";
    isModified_ = false;
    statusMessage_ = "New canvas created";
    updateTitle();
}

bool PaintContent::loadFromFile(const std::string& filePath) {
    auto loaded = Surface::loadBmp(filePath);
    if (!loaded) {
        statusMessage_ = "Failed to load BMP";
        return false;
    }

    canvas_ = std::move(*loaded);
    undoStack_.clear();
    redoStack_.clear();
    currentFilePath_ = filePath;

    const size_t slash = filePath.find_last_of("\\/");
    if (slash != std::string::npos && slash + 1 < filePath.length()) {
        fileName_ = filePath.substr(slash + 1);
    } else {
        fileName_ = filePath;
    }

    isModified_ = false;
    statusMessage_ = "Loaded " + fileName_;
    updateTitle();
    return true;
}

bool PaintContent::saveToFile(const std::string& filePath) {
    std::string target = filePath;
    if (target.empty()) {
        if (!currentFilePath_.empty()) {
            target = currentFilePath_;
        } else {
            target = "drawing.bmp";
        }
    }

    if (canvas_.exportBmp(target)) {
        currentFilePath_ = target;
        const size_t slash = target.find_last_of("\\/");
        if (slash != std::string::npos && slash + 1 < target.length()) {
            fileName_ = target.substr(slash + 1);
        } else {
            fileName_ = target;
        }
        isModified_ = false;
        statusMessage_ = "Saved: " + fileName_;
        updateTitle();

        if (onFileSaved_) {
            std::error_code ec;
            const uint64_t sz = std::filesystem::file_size(target, ec);
            onFileSaved_(target, sz);
        }
        return true;
    }

    statusMessage_ = "Save failed: " + target;
    return false;
}

void PaintContent::pushUndo() {
    undoStack_.push_back(canvas_);
    if (undoStack_.size() > MAX_UNDO) {
        undoStack_.pop_front();
    }
    redoStack_.clear();
}

void PaintContent::undo() {
    if (undoStack_.empty()) {
        statusMessage_ = "Nothing to undo";
        return;
    }
    redoStack_.push_back(canvas_);
    canvas_ = undoStack_.back();
    undoStack_.pop_back();
    isModified_ = true;
    statusMessage_ = "Undo performed";
    updateTitle();
}

void PaintContent::redo() {
    if (redoStack_.empty()) {
        statusMessage_ = "Nothing to redo";
        return;
    }
    undoStack_.push_back(canvas_);
    canvas_ = redoStack_.back();
    redoStack_.pop_back();
    isModified_ = true;
    statusMessage_ = "Redo performed";
    updateTitle();
}

void PaintContent::clearCanvas(Color bgColor) {
    pushUndo();
    canvas_.clear(bgColor);
    isModified_ = true;
    statusMessage_ = "Canvas cleared";
    updateTitle();
}

void PaintContent::updateTitle() {
    const std::string title = (isModified_ ? "* " : "") + fileName_ + " - Sovereign Paint";
    if (onTitleChanged_) {
        onTitleChanged_(title);
    }
}

Point PaintContent::clientToCanvas(Point clientPt, int32_t clientW, int32_t clientH) const noexcept {
    const Rect cb = canvasBounds(clientW, clientH);
    return Point{clientPt.x - cb.x, clientPt.y - cb.y};
}

bool PaintContent::isCanvasPoint(Point canvasPt) const noexcept {
    return canvasPt.x >= 0 && canvasPt.x < static_cast<int32_t>(canvas_.width()) &&
           canvasPt.y >= 0 && canvasPt.y < static_cast<int32_t>(canvas_.height());
}

Rect PaintContent::canvasBounds(int32_t clientW, int32_t clientH) const noexcept {
    constexpr int32_t headerH = 68;
    constexpr int32_t statusH = 22;
    const int32_t workspaceH = clientH - headerH - statusH;
    const int32_t cw = static_cast<int32_t>(canvas_.width());
    const int32_t ch = static_cast<int32_t>(canvas_.height());

    const int32_t cx = std::max(8, (clientW - cw) / 2);
    const int32_t cy = std::max(headerH + 4, headerH + (workspaceH - ch) / 2);
    return Rect{cx, cy, cw, ch};
}

void PaintContent::drawPencilPoint(Point canvasPt, Color c) {
    if (isCanvasPoint(canvasPt)) {
        canvas_.putPixel(canvasPt.x, canvasPt.y, c);
    }
}

void PaintContent::drawBrushSpot(Point canvasPt, Color c, int32_t radius) {
    if (radius <= 1) {
        drawPencilPoint(canvasPt, c);
        return;
    }
    const int32_t r2 = radius * radius;
    for (int32_t dy = -radius; dy <= radius; ++dy) {
        for (int32_t dx = -radius; dx <= radius; ++dx) {
            if (dx * dx + dy * dy <= r2) {
                const int32_t px = canvasPt.x + dx;
                const int32_t py = canvasPt.y + dy;
                if (isCanvasPoint(Point{px, py})) {
                    canvas_.putPixel(px, py, c);
                }
            }
        }
    }
}

void PaintContent::drawLine(Point startPt, Point endPt, Color c, int32_t stroke) {
    int32_t x0 = startPt.x;
    int32_t y0 = startPt.y;
    const int32_t x1 = endPt.x;
    const int32_t y1 = endPt.y;

    const int32_t dx = std::abs(x1 - x0);
    const int32_t dy = -std::abs(y1 - y0);
    const int32_t sx = (x0 < x1) ? 1 : -1;
    const int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;

    while (true) {
        if (stroke <= 1) {
            drawPencilPoint(Point{x0, y0}, c);
        } else {
            drawBrushSpot(Point{x0, y0}, c, stroke / 2);
        }
        if (x0 == x1 && y0 == y1) break;
        const int32_t e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void PaintContent::drawRect(Rect canvasRect, Color c, bool filled) {
    if (canvasRect.width <= 0 || canvasRect.height <= 0) return;
    const int32_t cw = static_cast<int32_t>(canvas_.width());
    const int32_t ch = static_cast<int32_t>(canvas_.height());

    const int32_t rx = std::clamp(canvasRect.x, 0, cw);
    const int32_t ry = std::clamp(canvasRect.y, 0, ch);
    const int32_t rw = std::clamp(canvasRect.width, 0, cw - rx);
    const int32_t rh = std::clamp(canvasRect.height, 0, ch - ry);

    if (filled) {
        canvas_.fillRect(Rect{rx, ry, rw, rh}, c);
    } else {
        canvas_.drawRect(Rect{rx, ry, rw, rh}, c);
    }
}

void PaintContent::drawCircle(Point centerPt, int32_t radius, Color c, bool filled) {
    if (radius <= 0) {
        drawPencilPoint(centerPt, c);
        return;
    }

    if (filled) {
        for (int32_t dy = -radius; dy <= radius; ++dy) {
            const int32_t py = centerPt.y + dy;
            if (py < 0 || py >= static_cast<int32_t>(canvas_.height())) continue;
            const int32_t span = static_cast<int32_t>(std::sqrt(radius * radius - dy * dy));
            const int32_t x0 = std::max(0, centerPt.x - span);
            const int32_t x1 = std::min(static_cast<int32_t>(canvas_.width()) - 1, centerPt.x + span);
            for (int32_t px = x0; px <= x1; ++px) {
                canvas_.putPixel(px, py, c);
            }
        }
    } else {
        // Midpoint circle outline
        int32_t x = radius;
        int32_t y = 0;
        int32_t err = 0;

        auto plot8 = [this, centerPt, c](int32_t cx, int32_t cy) {
            drawPencilPoint(Point{centerPt.x + cx, centerPt.y + cy}, c);
            drawPencilPoint(Point{centerPt.x - cx, centerPt.y + cy}, c);
            drawPencilPoint(Point{centerPt.x + cx, centerPt.y - cy}, c);
            drawPencilPoint(Point{centerPt.x - cx, centerPt.y - cy}, c);
            drawPencilPoint(Point{centerPt.x + cy, centerPt.y + cx}, c);
            drawPencilPoint(Point{centerPt.x - cy, centerPt.y + cx}, c);
            drawPencilPoint(Point{centerPt.x + cy, centerPt.y - cx}, c);
            drawPencilPoint(Point{centerPt.x - cy, centerPt.y - cx}, c);
        };

        while (x >= y) {
            plot8(x, y);
            y += 1;
            err += 1 + 2 * y;
            if (2 * (err - x) + 1 > 0) {
                x -= 1;
                err += 1 - 2 * x;
            }
        }
    }
}

void PaintContent::floodFill(Point seedPt, Color fillColor) {
    if (!isCanvasPoint(seedPt)) return;
    const Color targetColor = canvas_.getPixel(seedPt.x, seedPt.y);
    if (targetColor.toHex() == fillColor.toHex()) return;

    const int32_t w = static_cast<int32_t>(canvas_.width());
    const int32_t h = static_cast<int32_t>(canvas_.height());

    std::vector<uint8_t> visited(static_cast<size_t>(w) * h, 0);
    std::queue<Point> q;
    q.push(seedPt);
    visited[static_cast<size_t>(seedPt.y) * w + seedPt.x] = 1;

    const uint32_t targetHex = targetColor.toHex();

    while (!q.empty()) {
        const Point p = q.front();
        q.pop();

        canvas_.putPixel(p.x, p.y, fillColor);

        constexpr Point dirs[4] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
        for (const auto& d : dirs) {
            const int32_t nx = p.x + d.x;
            const int32_t ny = p.y + d.y;
            if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                const size_t idx = static_cast<size_t>(ny) * w + nx;
                if (!visited[idx]) {
                    visited[idx] = 1;
                    if (canvas_.getPixel(nx, ny).toHex() == targetHex) {
                        q.push(Point{nx, ny});
                    }
                }
            }
        }
    }
}

void PaintContent::render(Surface& clientSurface) {
    const auto& palette = ThemeManager::instance().palette();
    const int32_t width = static_cast<int32_t>(clientSurface.width());
    const int32_t height = static_cast<int32_t>(clientSurface.height());

    // 1. Clear Easel Workspace with dark studio slate
    clientSurface.clear(Color{14, 20, 32, 255});

    // 2. Top Header Toolbar (Height: 34px)
    constexpr int32_t row1H = 34;
    toolbarRect_ = Rect{0, 0, width, row1H};
    clientSurface.fillRect(toolbarRect_, Color{20, 28, 42, 255});
    clientSurface.fillRect(Rect{0, row1H - 1, width, 1}, Color{38, 52, 78, 180});

    // Row 1 Action Buttons
    actionButtons_.clear();
    int32_t actX = 8;
    auto addActionBtn = [&](const std::string& id, const std::string& label, IconId icon) {
        const int32_t bw = static_cast<int32_t>(label.length() * 8) + 24;
        const Rect r{actX, 4, bw, 24};
        actionButtons_.push_back(PaintToolbarButton{
            .id = id,
            .label = label,
            .iconId = icon,
            .bounds = r,
            .isHovered = false,
            .isSelected = false
        });
        actX += bw + 4;
    };

    addActionBtn("new", "New", IconId::NewFile);
    addActionBtn("open", "Open", IconId::Folder);
    addActionBtn("save", "Save", IconId::Save);
    addActionBtn("undo", "Undo", IconId::NavBack);
    addActionBtn("redo", "Redo", IconId::NavForward);
    addActionBtn("clear", "Clear", IconId::Delete);

    for (const auto& btn : actionButtons_) {
        clientSurface.drawRoundedRect(btn.bounds, 3, Color{28, 40, 60, 160}, true);
        clientSurface.drawRoundedRect(btn.bounds, 3, Color{45, 65, 95, 200}, false);
        IconRenderer::draw(clientSurface, btn.iconId, Rect{btn.bounds.x + 3, btn.bounds.y + 4, 16, 16}, Color::fromHex(0x00D4FF));
        clientSurface.drawString(btn.bounds.x + 22, btn.bounds.y + 5, btn.label, palette.textPrimary, 1);
    }

    // Divider
    actX += 4;
    clientSurface.fillRect(Rect{actX, 6, 1, 22}, Color{45, 65, 95, 200});
    actX += 8;

    // Tool Selector Buttons
    toolButtons_.clear();
    auto addToolBtn = [&](const std::string& id, const std::string& label, PaintTool tool, IconId icon) {
        const int32_t bw = static_cast<int32_t>(label.length() * 8) + 24;
        const Rect r{actX, 4, bw, 24};
        toolButtons_.push_back(PaintToolbarButton{
            .id = id,
            .label = label,
            .iconId = icon,
            .bounds = r,
            .isHovered = false,
            .isSelected = (activeTool_ == tool)
        });
        actX += bw + 4;
    };

    addToolBtn("pencil", "Pencil", PaintTool::Pencil, IconId::Edit);
    addToolBtn("brush", "Brush", PaintTool::Brush, IconId::Paint);
    addToolBtn("eraser", "Eraser", PaintTool::Eraser, IconId::Cut);
    addToolBtn("fill", "Fill", PaintTool::FloodFill, IconId::Copy);
    addToolBtn("picker", "Eye", PaintTool::ColorPicker, IconId::SearchCategory);
    addToolBtn("line", "Line", PaintTool::Line, IconId::Terminal);
    addToolBtn("rect", "Box", PaintTool::Rectangle, IconId::Display);
    addToolBtn("circle", "Circle", PaintTool::Circle, IconId::OpticalDrive);

    for (const auto& btn : toolButtons_) {
        const Color bg = btn.isSelected ? Color{0, 212, 255, 60} : Color{28, 40, 60, 160};
        const Color border = btn.isSelected ? Color{0, 212, 255, 220} : Color{45, 65, 95, 200};
        clientSurface.drawRoundedRect(btn.bounds, 3, bg, true);
        clientSurface.drawRoundedRect(btn.bounds, 3, border, false);
        IconRenderer::draw(clientSurface, btn.iconId, Rect{btn.bounds.x + 3, btn.bounds.y + 4, 16, 16}, btn.isSelected ? Color::fromHex(0x00FF9D) : Color::fromHex(0x00D4FF));
        clientSurface.drawString(btn.bounds.x + 22, btn.bounds.y + 5, btn.label, btn.isSelected ? Color::fromHex(0x00D4FF) : palette.textPrimary, 1);
    }

    // 3. Second Row Toolbar: Palette, Swatches, Stroke, and Shape Fill Mode (Height: 34px)
    constexpr int32_t row2Y = 34;
    constexpr int32_t row2H = 34;
    paletteRect_ = Rect{0, row2Y, width, row2H};
    clientSurface.fillRect(paletteRect_, Color{16, 22, 34, 255});
    clientSurface.fillRect(Rect{0, row2Y + row2H - 1, width, 1}, Color{38, 52, 78, 180});

    // Swatches: Primary (Color 1) and Secondary (Color 2)
    primarySwatchRect_ = Rect{8, row2Y + 4, 26, 26};
    secondarySwatchRect_ = Rect{40, row2Y + 4, 26, 26};

    // Primary Swatch
    clientSurface.drawRoundedRect(primarySwatchRect_, 2, primaryColor_, true);
    clientSurface.drawRoundedRect(primarySwatchRect_, 2, Color::fromHex(0x00D4FF), false);
    clientSurface.drawString(12, row2Y + 9, "1", (primaryColor_.r + primaryColor_.g + primaryColor_.b > 380) ? Color::fromHex(0x000000) : Color::fromHex(0xFFFFFF), 1);

    // Secondary Swatch
    clientSurface.drawRoundedRect(secondarySwatchRect_, 2, secondaryColor_, true);
    clientSurface.drawRoundedRect(secondarySwatchRect_, 2, Color::fromHex(0x64748B), false);
    clientSurface.drawString(44, row2Y + 9, "2", (secondaryColor_.r + secondaryColor_.g + secondaryColor_.b > 380) ? Color::fromHex(0x000000) : Color::fromHex(0xFFFFFF), 1);

    // Palette Color Grid (2 rows of 12 boxes, 14x14 each)
    paletteColorSlots_.clear();
    const int32_t palStartX = 76;
    for (size_t i = 0; i < presetColors_.size(); ++i) {
        const int32_t col = static_cast<int32_t>(i % 12);
        const int32_t row = static_cast<int32_t>(i / 12);
        const Rect r{palStartX + col * 16, row2Y + 3 + row * 15, 14, 13};
        paletteColorSlots_.push_back({r, presetColors_[i]});
        clientSurface.fillRect(r, presetColors_[i]);
        clientSurface.drawRect(r, Color::fromHex(0x334155));
    }

    // Stroke Size buttons
    int32_t optX = palStartX + 12 * 16 + 12;
    clientSurface.fillRect(Rect{optX - 6, row2Y + 6, 1, 22}, Color{45, 65, 95, 200});

    strokeButtons_.clear();
    auto addStrokeBtn = [&](int32_t sz, const std::string& label) {
        const Rect r{optX, row2Y + 5, 24, 22};
        strokeButtons_.push_back(PaintToolbarButton{
            .id = std::to_string(sz),
            .label = label,
            .iconId = IconId::Edit,
            .bounds = r,
            .isHovered = false,
            .isSelected = (strokeSize_ == sz)
        });
        optX += 28;
    };
    addStrokeBtn(1, "1p");
    addStrokeBtn(2, "2p");
    addStrokeBtn(4, "4p");
    addStrokeBtn(8, "8p");

    for (const auto& btn : strokeButtons_) {
        const Color bg = btn.isSelected ? Color{0, 212, 255, 60} : Color{28, 40, 60, 160};
        const Color border = btn.isSelected ? Color{0, 212, 255, 220} : Color{45, 65, 95, 200};
        clientSurface.drawRoundedRect(btn.bounds, 2, bg, true);
        clientSurface.drawRoundedRect(btn.bounds, 2, border, false);
        clientSurface.drawString(btn.bounds.x + 4, btn.bounds.y + 4, btn.label, btn.isSelected ? Color::fromHex(0x00D4FF) : palette.textPrimary, 1);
    }

    // Shape Fill Mode buttons
    optX += 4;
    clientSurface.fillRect(Rect{optX - 4, row2Y + 6, 1, 22}, Color{45, 65, 95, 200});
    shapeModeButtons_.clear();

    const Rect outlRect{optX, row2Y + 5, 52, 22};
    shapeModeButtons_.push_back(PaintToolbarButton{
        .id = "outline",
        .label = "Line",
        .bounds = outlRect,
        .isSelected = (shapeMode_ == PaintShapeMode::Outline)
    });
    const Rect fillRect{optX + 56, row2Y + 5, 52, 22};
    shapeModeButtons_.push_back(PaintToolbarButton{
        .id = "filled",
        .label = "Fill",
        .bounds = fillRect,
        .isSelected = (shapeMode_ == PaintShapeMode::Filled)
    });

    for (const auto& btn : shapeModeButtons_) {
        const Color bg = btn.isSelected ? Color{0, 255, 157, 50} : Color{28, 40, 60, 160};
        const Color border = btn.isSelected ? Color{0, 255, 157, 220} : Color{45, 65, 95, 200};
        clientSurface.drawRoundedRect(btn.bounds, 2, bg, true);
        clientSurface.drawRoundedRect(btn.bounds, 2, border, false);
        clientSurface.drawString(btn.bounds.x + 8, btn.bounds.y + 4, btn.label, btn.isSelected ? Color::fromHex(0x00FF9D) : palette.textPrimary, 1);
    }

    // Current document info pill on far right
    const std::string docPill = fileName_ + (isModified_ ? " *" : "") + " [" + std::to_string(canvas_.width()) + "x" + std::to_string(canvas_.height()) + "]";
    const int32_t pillW = static_cast<int32_t>(docPill.length() * 8) + 16;
    const Rect pillRect{width - pillW - 10, row2Y + 5, pillW, 22};
    clientSurface.drawRoundedRect(pillRect, 11, isModified_ ? Color{255, 183, 3, 30} : Color{0, 212, 255, 25}, true);
    clientSurface.drawRoundedRect(pillRect, 11, isModified_ ? Color{255, 183, 3, 140} : Color{0, 212, 255, 120}, false);
    clientSurface.drawString(pillRect.x + 8, pillRect.y + 4, docPill, isModified_ ? Color{255, 183, 3, 255} : Color{0, 212, 255, 255}, 1);

    // 4. Easel Workspace & Canvas Surface
    const Rect cb = canvasBounds(width, height);

    // Subtle drop shadow behind the canvas
    clientSurface.drawDropShadow(Rect{cb.x - 2, cb.y - 2, cb.width + 4, cb.height + 4}, 4, 0.45f);

    // Blit internal Canvas
    clientSurface.blit(canvas_, Rect{0, 0, cb.width, cb.height}, Point{cb.x, cb.y});

    // Clean border around canvas
    clientSurface.drawRect(Rect{cb.x - 1, cb.y - 1, cb.width + 2, cb.height + 2}, Color::fromHex(0x475569));

    // 5. Interactive Shape Preview (while dragging Line, Rectangle, Circle)
    if (isDrawing_ && (activeTool_ == PaintTool::Line || activeTool_ == PaintTool::Rectangle || activeTool_ == PaintTool::Circle)) {
        const Color previewCol = (activeDrawButton_ == MouseButton::Right) ? secondaryColor_ : primaryColor_;
        const Point p0{cb.x + dragStartCanvasPt_.x, cb.y + dragStartCanvasPt_.y};
        const Point p1{cb.x + currentCanvasPt_.x, cb.y + currentCanvasPt_.y};

        if (activeTool_ == PaintTool::Line) {
            // Draw preview line clamped to canvas
            int32_t x0 = p0.x, y0 = p0.y, x1 = p1.x, y1 = p1.y;
            const int32_t dx = std::abs(x1 - x0);
            const int32_t dy = -std::abs(y1 - y0);
            const int32_t sx = (x0 < x1) ? 1 : -1;
            const int32_t sy = (y0 < y1) ? 1 : -1;
            int32_t err = dx + dy;
            while (true) {
                if (cb.contains(Point{x0, y0})) {
                    clientSurface.putPixel(x0, y0, previewCol);
                }
                if (x0 == x1 && y0 == y1) break;
                const int32_t e2 = 2 * err;
                if (e2 >= dy) { err += dy; x0 += sx; }
                if (e2 <= dx) { err += dx; y0 += sy; }
            }
        } else if (activeTool_ == PaintTool::Rectangle) {
            const int32_t rx = std::max(cb.x, std::min(p0.x, p1.x));
            const int32_t ry = std::max(cb.y, std::min(p0.y, p1.y));
            const int32_t rw = std::min(cb.right(), std::max(p0.x, p1.x)) - rx;
            const int32_t rh = std::min(cb.bottom(), std::max(p0.y, p1.y)) - ry;
            if (rw > 0 && rh > 0) {
                if (shapeMode_ == PaintShapeMode::Filled) {
                    clientSurface.fillRect(Rect{rx, ry, rw, rh}, previewCol);
                } else {
                    clientSurface.drawRect(Rect{rx, ry, rw, rh}, previewCol);
                }
            }
        } else if (activeTool_ == PaintTool::Circle) {
            const int32_t rad = static_cast<int32_t>(std::hypot(p1.x - p0.x, p1.y - p0.y));
            if (shapeMode_ == PaintShapeMode::Filled) {
                for (int32_t dy = -rad; dy <= rad; ++dy) {
                    const int32_t cy = p0.y + dy;
                    if (cy < cb.y || cy >= cb.bottom()) continue;
                    const int32_t span = static_cast<int32_t>(std::sqrt(rad * rad - dy * dy));
                    const int32_t x0 = std::max(cb.x, p0.x - span);
                    const int32_t x1 = std::min(cb.right() - 1, p0.x + span);
                    for (int32_t px = x0; px <= x1; ++px) {
                        clientSurface.putPixel(px, cy, previewCol);
                    }
                }
            } else {
                for (int32_t deg = 0; deg < 360; deg += 4) {
                    const float rads = static_cast<float>(deg) * 3.14159f / 180.0f;
                    const int32_t px = static_cast<int32_t>(p0.x + std::cos(rads) * rad);
                    const int32_t py = static_cast<int32_t>(p0.y + std::sin(rads) * rad);
                    if (cb.contains(Point{px, py})) {
                        clientSurface.putPixel(px, py, previewCol);
                    }
                }
            }
        }
    }

    // 6. Bottom Status Bar (Height: 22px)
    statusBarRect_ = Rect{0, height - 22, width, 22};
    clientSurface.fillRect(statusBarRect_, Color{16, 22, 34, 255});
    clientSurface.fillRect(Rect{0, height - 22, width, 1}, Color{38, 52, 78, 180});

    const std::string dimStr = "Canvas: " + std::to_string(canvas_.width()) + "x" + std::to_string(canvas_.height()) + " px";
    clientSurface.drawString(10, height - 16, dimStr, Color::fromHex(0x94A3B8), 1);

    const Point curCanvas = clientToCanvas(currentMousePos_, width, height);
    std::string coordStr = "Cursor: --";
    if (isCanvasPoint(curCanvas)) {
        coordStr = "Cursor: " + std::to_string(curCanvas.x) + ", " + std::to_string(curCanvas.y);
    }
    clientSurface.drawString(170, height - 16, coordStr, Color::fromHex(0x94A3B8), 1);

    std::string toolName = "Tool: ";
    switch (activeTool_) {
        case PaintTool::Pencil: toolName += "Pencil (1px)"; break;
        case PaintTool::Brush: toolName += "Brush (" + std::to_string(strokeSize_) + "px)"; break;
        case PaintTool::Eraser: toolName += "Eraser (" + std::to_string(strokeSize_ * 2) + "px)"; break;
        case PaintTool::FloodFill: toolName += "Flood Fill"; break;
        case PaintTool::ColorPicker: toolName += "Eyedropper"; break;
        case PaintTool::Line: toolName += "Line (" + std::to_string(strokeSize_) + "px)"; break;
        case PaintTool::Rectangle: toolName += "Box (" + std::string(shapeMode_ == PaintShapeMode::Filled ? "Filled" : "Outline") + ")"; break;
        case PaintTool::Circle: toolName += "Circle (" + std::string(shapeMode_ == PaintShapeMode::Filled ? "Filled" : "Outline") + ")"; break;
    }
    clientSurface.drawString(330, height - 16, toolName, Color::fromHex(0x00D4FF), 1);

    // Status Message / Notification on right
    clientSurface.drawString(width - static_cast<int32_t>(statusMessage_.length() * 8) - 16, height - 16, statusMessage_, Color::fromHex(0x00FF9D), 1);
}

bool PaintContent::onMouseDown(Point localPt, MouseButton button) {
    currentMousePos_ = localPt;

    // 1. Action Buttons
    for (const auto& btn : actionButtons_) {
        if (btn.bounds.contains(localPt)) {
            if (btn.id == "new") {
                newCanvas();
            } else if (btn.id == "open") {
                if (!currentFilePath_.empty()) {
                    loadFromFile(currentFilePath_);
                }
            } else if (btn.id == "save") {
                saveToFile();
            } else if (btn.id == "undo") {
                undo();
            } else if (btn.id == "redo") {
                redo();
            } else if (btn.id == "clear") {
                clearCanvas(secondaryColor_);
            }
            return true;
        }
    }

    // 2. Tool Buttons
    for (const auto& btn : toolButtons_) {
        if (btn.bounds.contains(localPt)) {
            if (btn.id == "pencil") activeTool_ = PaintTool::Pencil;
            else if (btn.id == "brush") activeTool_ = PaintTool::Brush;
            else if (btn.id == "eraser") activeTool_ = PaintTool::Eraser;
            else if (btn.id == "fill") activeTool_ = PaintTool::FloodFill;
            else if (btn.id == "picker") activeTool_ = PaintTool::ColorPicker;
            else if (btn.id == "line") activeTool_ = PaintTool::Line;
            else if (btn.id == "rect") activeTool_ = PaintTool::Rectangle;
            else if (btn.id == "circle") activeTool_ = PaintTool::Circle;
            statusMessage_ = "Tool selected: " + btn.label;
            return true;
        }
    }

    // 3. Stroke Buttons
    for (const auto& btn : strokeButtons_) {
        if (btn.bounds.contains(localPt)) {
            strokeSize_ = std::stoi(btn.id);
            statusMessage_ = "Stroke: " + btn.id + "px";
            return true;
        }
    }

    // 4. Shape Mode Buttons
    for (const auto& btn : shapeModeButtons_) {
        if (btn.bounds.contains(localPt)) {
            if (btn.id == "outline") shapeMode_ = PaintShapeMode::Outline;
            else if (btn.id == "filled") shapeMode_ = PaintShapeMode::Filled;
            statusMessage_ = "Shape mode: " + btn.label;
            return true;
        }
    }

    // 5. Palette Swatches & Preset Colors
    if (primarySwatchRect_.contains(localPt) || secondarySwatchRect_.contains(localPt)) {
        // Swap primary & secondary
        std::swap(primaryColor_, secondaryColor_);
        statusMessage_ = "Swapped colors";
        return true;
    }

    for (const auto& [slotRect, col] : paletteColorSlots_) {
        if (slotRect.contains(localPt)) {
            if (button == MouseButton::Right) {
                secondaryColor_ = col;
                statusMessage_ = "Color 2 set";
            } else {
                primaryColor_ = col;
                statusMessage_ = "Color 1 set";
            }
            return true;
        }
    }

    // 6. Easel Canvas Drawing
    // We compute canvas coordinates based on typical or last known dimensions
    const Rect cb = canvasBounds(std::max(800, localPt.x + 100), std::max(600, localPt.y + 100));
    const Point canvasPt{localPt.x - cb.x, localPt.y - cb.y};

    if (isCanvasPoint(canvasPt)) {
        activeDrawButton_ = button;
        currentCanvasPt_ = canvasPt;
        lastCanvasPt_ = canvasPt;
        dragStartCanvasPt_ = canvasPt;

        if (activeTool_ == PaintTool::ColorPicker) {
            const Color sampled = canvas_.getPixel(canvasPt.x, canvasPt.y);
            if (button == MouseButton::Right) {
                secondaryColor_ = sampled;
                statusMessage_ = "Color 2 sampled";
            } else {
                primaryColor_ = sampled;
                statusMessage_ = "Color 1 sampled";
            }
            return true;
        }

        if (activeTool_ == PaintTool::FloodFill) {
            pushUndo();
            const Color fillCol = (button == MouseButton::Right) ? secondaryColor_ : primaryColor_;
            floodFill(canvasPt, fillCol);
            isModified_ = true;
            statusMessage_ = "Filled area";
            updateTitle();
            return true;
        }

        if (activeTool_ == PaintTool::Pencil || activeTool_ == PaintTool::Brush || activeTool_ == PaintTool::Eraser) {
            pushUndo();
            isDrawing_ = true;
            const Color drawCol = (button == MouseButton::Right) ? secondaryColor_ : (activeTool_ == PaintTool::Eraser ? secondaryColor_ : primaryColor_);
            const int32_t sz = (activeTool_ == PaintTool::Pencil) ? 1 : (activeTool_ == PaintTool::Eraser ? strokeSize_ * 2 : strokeSize_);
            if (sz <= 1) {
                drawPencilPoint(canvasPt, drawCol);
            } else {
                drawBrushSpot(canvasPt, drawCol, sz / 2);
            }
            isModified_ = true;
            updateTitle();
            return true;
        }

        if (activeTool_ == PaintTool::Line || activeTool_ == PaintTool::Rectangle || activeTool_ == PaintTool::Circle) {
            isDrawing_ = true;
            return true;
        }
    }

    return false;
}

bool PaintContent::onMouseMove(Point localPt) {
    currentMousePos_ = localPt;

    const Rect cb = canvasBounds(std::max(800, localPt.x + 100), std::max(600, localPt.y + 100));
    const Point canvasPt{localPt.x - cb.x, localPt.y - cb.y};

    if (isDrawing_) {
        currentCanvasPt_ = canvasPt;

        if (activeTool_ == PaintTool::Pencil || activeTool_ == PaintTool::Brush || activeTool_ == PaintTool::Eraser) {
            const Color drawCol = (activeDrawButton_ == MouseButton::Right) ? secondaryColor_ : (activeTool_ == PaintTool::Eraser ? secondaryColor_ : primaryColor_);
            const int32_t sz = (activeTool_ == PaintTool::Pencil) ? 1 : (activeTool_ == PaintTool::Eraser ? strokeSize_ * 2 : strokeSize_);
            drawLine(lastCanvasPt_, canvasPt, drawCol, sz);
            lastCanvasPt_ = canvasPt;
            isModified_ = true;
            return true;
        }

        if (activeTool_ == PaintTool::Line || activeTool_ == PaintTool::Rectangle || activeTool_ == PaintTool::Circle) {
            return true; // Interactive preview is updated via currentCanvasPt_
        }
    }

    return false;
}

bool PaintContent::onMouseUp(Point localPt, MouseButton button) {
    (void)button;
    if (isDrawing_) {
        const Rect cb = canvasBounds(std::max(800, localPt.x + 100), std::max(600, localPt.y + 100));
        const Point canvasPt{localPt.x - cb.x, localPt.y - cb.y};

        if (activeTool_ == PaintTool::Line || activeTool_ == PaintTool::Rectangle || activeTool_ == PaintTool::Circle) {
            pushUndo();
            const Color drawCol = (activeDrawButton_ == MouseButton::Right) ? secondaryColor_ : primaryColor_;

            if (activeTool_ == PaintTool::Line) {
                drawLine(dragStartCanvasPt_, canvasPt, drawCol, strokeSize_);
            } else if (activeTool_ == PaintTool::Rectangle) {
                const int32_t rx = std::min(dragStartCanvasPt_.x, canvasPt.x);
                const int32_t ry = std::min(dragStartCanvasPt_.y, canvasPt.y);
                const int32_t rw = std::abs(canvasPt.x - dragStartCanvasPt_.x) + 1;
                const int32_t rh = std::abs(canvasPt.y - dragStartCanvasPt_.y) + 1;
                drawRect(Rect{rx, ry, rw, rh}, drawCol, shapeMode_ == PaintShapeMode::Filled);
            } else if (activeTool_ == PaintTool::Circle) {
                const int32_t rad = static_cast<int32_t>(std::hypot(canvasPt.x - dragStartCanvasPt_.x, canvasPt.y - dragStartCanvasPt_.y));
                drawCircle(dragStartCanvasPt_, rad, drawCol, shapeMode_ == PaintShapeMode::Filled);
            }
            isModified_ = true;
            updateTitle();
        }

        isDrawing_ = false;
        return true;
    }
    return false;
}

bool PaintContent::onMouseWheel(Point localPt, int32_t delta) {
    (void)localPt;
    // Mouse wheel cycles brush stroke size
    if (delta > 0) {
        strokeSize_ = std::min(16, strokeSize_ + 1);
    } else if (delta < 0) {
        strokeSize_ = std::max(1, strokeSize_ - 1);
    }
    statusMessage_ = "Stroke: " + std::to_string(strokeSize_) + "px";
    return true;
}

bool PaintContent::onKeyDown(KeyCode key, bool ctrl, bool shift, bool alt) {
    (void)shift;
    (void)alt;
    if (ctrl) {
        if (key == KeyCode::KeyZ) {
            if (shift) {
                redo();
            } else {
                undo();
            }
            return true;
        }
        if (key == KeyCode::KeyY) {
            redo();
            return true;
        }
        if (key == KeyCode::KeyS) {
            saveToFile();
            return true;
        }
        if (key == KeyCode::KeyN) {
            newCanvas();
            return true;
        }
    }
    return false;
}

} // namespace surshell
