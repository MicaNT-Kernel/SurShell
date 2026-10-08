// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/text_viewer.hpp)
//
// Sovereign Text & Code Viewer (Sovereign Notepad / Text Editor Content)
// Clean-room ISO C++23, zero telemetry, syntax-aware line rendering,
// mouse wheel scrolling, and status bar.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "window_manager.hpp"
#include "theme.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

namespace surshell {

class TextViewerContent : public IWindowContent {
public:
    explicit TextViewerContent(std::string filePath = "");

    void loadFile(const std::string& filePath);

    [[nodiscard]] const std::string& filePath() const noexcept { return filePath_; }
    [[nodiscard]] size_t lineCount() const noexcept { return lines_.size(); }
    [[nodiscard]] int32_t scrollOffset() const noexcept { return scrollOffset_; }

    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseWheel(Point localPt, int32_t delta) override;
    bool onCharInput(char c) override;

private:
    std::string filePath_{};
    std::string fileName_{"Untitled.txt"};
    std::vector<std::string> lines_{};
    int32_t scrollOffset_{0};
    uint64_t fileSizeBytes_{0};

    Rect scrollbarTrack_{};
    Rect scrollbarThumb_{};
};

} // namespace surshell
