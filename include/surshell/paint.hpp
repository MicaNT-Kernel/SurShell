// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/paint.hpp)
//
// Sovereign Paint & Vector Canvas Studio (paint.exe / mspaint Parity)
// Clean-room ISO C++23, zero telemetry, pixel drawing, shape rasterization,
// flood fill, color palette, undo/redo stack, and BMP import/export.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "window_manager.hpp"
#include "theme.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <deque>
#include <optional>
#include <functional>

namespace surshell {

enum class PaintTool {
    Pencil,
    Brush,
    Eraser,
    FloodFill,
    ColorPicker,
    Line,
    Rectangle,
    Circle
};

enum class PaintShapeMode {
    Outline,
    Filled
};

struct PaintToolbarButton {
    std::string id;
    std::string label;
    IconId iconId{IconId::FileGeneric};
    Rect bounds{};
    bool isHovered{false};
    bool isSelected{false};
};

class PaintContent : public IWindowContent {
public:
    explicit PaintContent(std::string filePath = "");

    void newCanvas(uint32_t width = 520, uint32_t height = 340, Color bgColor = Color{255, 255, 255, 255});
    bool loadFromFile(const std::string& filePath);
    bool saveToFile(const std::string& filePath = "");

    void setTool(PaintTool tool) noexcept { activeTool_ = tool; }
    [[nodiscard]] PaintTool tool() const noexcept { return activeTool_; }

    void setPrimaryColor(Color c) noexcept { primaryColor_ = c; }
    [[nodiscard]] Color primaryColor() const noexcept { return primaryColor_; }

    void setSecondaryColor(Color c) noexcept { secondaryColor_ = c; }
    [[nodiscard]] Color secondaryColor() const noexcept { return secondaryColor_; }

    void setStrokeSize(int32_t size) noexcept { strokeSize_ = std::clamp(size, 1, 32); }
    [[nodiscard]] int32_t strokeSize() const noexcept { return strokeSize_; }

    void setShapeMode(PaintShapeMode mode) noexcept { shapeMode_ = mode; }
    [[nodiscard]] PaintShapeMode shapeMode() const noexcept { return shapeMode_; }

    void undo();
    void redo();
    void clearCanvas(Color bgColor = Color{255, 255, 255, 255});

    [[nodiscard]] const Surface& canvas() const noexcept { return canvas_; }
    [[nodiscard]] Surface& canvas() noexcept { return canvas_; }
    [[nodiscard]] const std::string& currentFilePath() const noexcept { return currentFilePath_; }
    [[nodiscard]] const std::string& fileName() const noexcept { return fileName_; }
    [[nodiscard]] bool isModified() const noexcept { return isModified_; }
    [[nodiscard]] size_t undoDepth() const noexcept { return undoStack_.size(); }
    [[nodiscard]] size_t redoDepth() const noexcept { return redoStack_.size(); }

    // Direct programmatic drawing operations (for tests and automated tools)
    void drawPencilPoint(Point canvasPt, Color c);
    void drawBrushSpot(Point canvasPt, Color c, int32_t radius);
    void drawLine(Point startPt, Point endPt, Color c, int32_t stroke = 1);
    void drawRect(Rect canvasRect, Color c, bool filled = false);
    void drawCircle(Point centerPt, int32_t radius, Color c, bool filled = false);
    void floodFill(Point seedPt, Color fillColor);

    // Callbacks
    void setTitleChangedCallback(std::function<void(const std::string&)> cb) { onTitleChanged_ = std::move(cb); }
    void setFileSavedCallback(std::function<void(const std::string&, uint64_t)> cb) { onFileSaved_ = std::move(cb); }

    // IWindowContent overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseUp(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onMouseWheel(Point localPt, int32_t delta) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;

private:
    void pushUndo();
    void updateTitle();
    [[nodiscard]] Point clientToCanvas(Point clientPt, int32_t clientW, int32_t clientH) const noexcept;
    [[nodiscard]] bool isCanvasPoint(Point canvasPt) const noexcept;
    [[nodiscard]] Rect canvasBounds(int32_t clientW, int32_t clientH) const noexcept;

    Surface canvas_{520, 340, Color{255, 255, 255, 255}};
    std::string currentFilePath_{};
    std::string fileName_{"Untitled.bmp"};
    bool isModified_{false};

    PaintTool activeTool_{PaintTool::Pencil};
    Color primaryColor_{Color{0, 0, 0, 255}};         // Black
    Color secondaryColor_{Color{255, 255, 255, 255}}; // White
    int32_t strokeSize_{2};
    PaintShapeMode shapeMode_{PaintShapeMode::Outline};

    std::deque<Surface> undoStack_{};
    std::deque<Surface> redoStack_{};
    static constexpr size_t MAX_UNDO = 10;

    // Interactive dragging state
    bool isDrawing_{false};
    MouseButton activeDrawButton_{MouseButton::Left};
    Point lastCanvasPt_{0, 0};
    Point dragStartCanvasPt_{0, 0};
    Point currentCanvasPt_{0, 0};
    Point currentMousePos_{0, 0};

    // UI Layout Rectangles
    Rect toolbarRect_{};
    Rect paletteRect_{};
    Rect statusBarRect_{};
    std::vector<PaintToolbarButton> toolButtons_{};
    std::vector<PaintToolbarButton> actionButtons_{};
    std::vector<PaintToolbarButton> strokeButtons_{};
    std::vector<PaintToolbarButton> shapeModeButtons_{};
    std::vector<std::pair<Rect, Color>> paletteColorSlots_{};
    Rect primarySwatchRect_{};
    Rect secondarySwatchRect_{};

    std::vector<Color> presetColors_{};
    std::string statusMessage_{"Ready"};

    std::function<void(const std::string&)> onTitleChanged_{};
    std::function<void(const std::string&, uint64_t)> onFileSaved_{};
};

} // namespace surshell
