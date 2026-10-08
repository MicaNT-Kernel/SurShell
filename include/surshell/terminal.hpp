// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/terminal.hpp)
//
// Modern tabbed terminal emulator system inspired by Microsoft Windows Terminal
// (MIT License architecture), featuring multi-profile tabs, VT/ANSI styling,
// interactive command execution, scrollback buffer, and shell command dispatch.
// ============================================================================

#pragma once

#include "surshell/types.hpp"
#include "surshell/compositor.hpp"
#include "surshell/window_manager.hpp"
#include "surshell/icons.hpp"
#include "surshell/theme.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace surshell {

struct TerminalLine {
    std::string text;
    Color color{Color::fromHex(0xCBD5E1)};
    bool bold{false};
};

struct TerminalProfile {
    std::string name;
    std::string prompt;
    Color promptColor{Color::fromHex(0x00FF9D)}; // Sovereign Green
};

struct TerminalTabItem {
    std::string title;
    std::string profileName;
    std::string cwd{"C:\\Windows\\System32"};
    std::vector<TerminalLine> buffer;
    std::string currentInput;
    int32_t cursorPosition{0};
    int32_t scrollOffset{0};
    std::vector<std::string> history;
    int32_t historyIndex{-1};
    Rect tabBounds{};
    Rect closeBounds{};
};

using TerminalAppSpawnCallback = std::function<void(const std::string& appName, const std::string& args)>;

class TerminalContent : public IWindowContent {
public:
    TerminalContent();
    ~TerminalContent() override = default;

    void render(Surface& surface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onMouseUp(Point localPt, MouseButton button) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;
    bool onCharInput(char c) override;

    // Terminal tab controls
    size_t activeTabIndex() const noexcept { return activeTabIndex_; }
    size_t tabCount() const noexcept { return tabs_.size(); }
    void addTab(const std::string& title, const std::string& profile = "cmd");
    void closeTab(size_t index);
    void selectTab(size_t index);

    // Command execution
    void executeCurrentCommand();
    void appendOutput(const std::string& text, Color color = Color::fromHex(0xCBD5E1), bool bold = false);
    void clearActiveBuffer();

    // App spawn callback
    void setAppSpawnCallback(TerminalAppSpawnCallback cb) { onSpawnApp_ = std::move(cb); }

    // Direct input programmatic helpers
    void inputString(const std::string& str);
    const std::string& currentInput() const;

private:
    void executeCommand(const std::string& rawCmd);
    void renderTabBar(Surface& s, const ThemePalette& palette);
    void renderBuffer(Surface& s, const ThemePalette& palette);

    std::vector<TerminalTabItem> tabs_;
    size_t activeTabIndex_{0};
    int32_t hoveredTab_{-1};
    int32_t hoveredClose_{-1};
    bool hoverNewTab_{false};
    Rect newTabBounds_{};

    bool cursorBlink_{true};
    uint32_t blinkTimer_{0};

    TerminalAppSpawnCallback onSpawnApp_;
};

} // namespace surshell
