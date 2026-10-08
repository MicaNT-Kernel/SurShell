// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/text_viewer.hpp)
//
// Sovereign Notepad & Code Editor 2.0 (notepad.exe)
// Clean-room ISO C++23, zero telemetry, full keyboard navigation and typing,
// mouse cursor positioning, syntax highlighting, live status bar, and file saving.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "window_manager.hpp"
#include "theme.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <functional>

namespace surshell {

struct EditorToolbarButton {
    std::string id;
    std::string label;
    IconId iconId{IconId::Edit};
    Rect bounds{};
    bool isHovered{false};
};

class TextViewerContent : public IWindowContent {
public:
    using FileSavedCallback = std::function<void(const std::string& path, uint64_t bytes)>;
    using TitleChangedCallback = std::function<void(const std::string& newTitle)>;

    explicit TextViewerContent(std::string filePath = "");

    void loadFile(const std::string& filePath);
    bool saveFile(const std::string& targetPath = "");
    void newDocument();

    [[nodiscard]] const std::string& filePath() const noexcept { return filePath_; }
    [[nodiscard]] const std::string& fileName() const noexcept { return fileName_; }
    [[nodiscard]] size_t lineCount() const noexcept { return lines_.size(); }
    [[nodiscard]] int32_t scrollOffset() const noexcept { return scrollOffset_; }
    [[nodiscard]] bool isModified() const noexcept { return isModified_; }
    [[nodiscard]] size_t cursorRow() const noexcept { return cursorRow_; }
    [[nodiscard]] size_t cursorCol() const noexcept { return cursorCol_; }
    [[nodiscard]] uint64_t fileSizeBytes() const noexcept { return fileSizeBytes_; }
    [[nodiscard]] bool wordWrap() const noexcept { return wordWrap_; }

    void setFileSavedCallback(FileSavedCallback cb) { fileSavedCb_ = std::move(cb); }
    void setTitleChangedCallback(TitleChangedCallback cb) { titleChangedCb_ = std::move(cb); }

    // Direct editing operations (for programmatic control and testing)
    void insertChar(char c);
    void insertText(const std::string& text);
    void insertNewline();
    void backspace();
    void deleteForward();
    void setCursor(size_t row, size_t col);
    [[nodiscard]] std::string fullText() const;

    // IWindowContent overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onMouseWheel(Point localPt, int32_t delta) override;
    bool onCharInput(char c) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;

private:
    void updateTitle();
    void ensureCursorVisible(int32_t viewportH, int32_t lineH);

    std::string filePath_{};
    std::string fileName_{"Untitled.txt"};
    std::vector<std::string> lines_{};
    int32_t scrollOffset_{0};
    uint64_t fileSizeBytes_{0};
    bool isModified_{false};

    size_t cursorRow_{0};
    size_t cursorCol_{0};
    bool wordWrap_{false};

    // UI Regions
    Rect toolbarRect_{};
    std::vector<EditorToolbarButton> toolbarButtons_{};
    Rect scrollbarTrack_{};
    Rect scrollbarThumb_{};

    FileSavedCallback fileSavedCb_{};
    TitleChangedCallback titleChangedCb_{};
};

} // namespace surshell
