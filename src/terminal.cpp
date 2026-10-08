// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/terminal.cpp)
// ============================================================================

#include "surshell/terminal.hpp"
#include <algorithm>
#include <sstream>

namespace surshell {

TerminalContent::TerminalContent() {
    addTab("Command Prompt", "cmd");
}

void TerminalContent::addTab(const std::string& title, const std::string& profile) {
    TerminalTabItem tab;
    tab.title = title;
    tab.profileName = profile;
    tab.cwd = "C:\\Windows\\System32";

    tab.buffer.push_back({"MicaNT [Version 10.0.26100.1-SOVEREIGN]", Color::fromHex(0x00D4FF), true});
    tab.buffer.push_back({"(c) 2026 Sovereign OS Project. Pure Clean-Room NT Architecture.", Color::fromHex(0x7186A4), false});
    tab.buffer.push_back({"Windows Terminal Modern Host (MIT Architecture Parity)", Color::fromHex(0x00FF9D), false});
    tab.buffer.push_back({"", Color::fromHex(0xCBD5E1), false});
    tab.buffer.push_back({"Type 'help' for built-in commands or launch system apps.", Color::fromHex(0x94A3B8), false});
    tab.buffer.push_back({"", Color::fromHex(0xCBD5E1), false});

    tabs_.push_back(std::move(tab));
    activeTabIndex_ = tabs_.size() - 1;
}

void TerminalContent::closeTab(size_t index) {
    if (index >= tabs_.size()) return;
    tabs_.erase(tabs_.begin() + static_cast<ptrdiff_t>(index));
    if (tabs_.empty()) {
        addTab("Command Prompt", "cmd");
    } else if (activeTabIndex_ >= tabs_.size()) {
        activeTabIndex_ = tabs_.size() - 1;
    }
}

void TerminalContent::selectTab(size_t index) {
    if (index < tabs_.size()) {
        activeTabIndex_ = index;
    }
}

const std::string& TerminalContent::currentInput() const {
    static const std::string empty;
    if (activeTabIndex_ < tabs_.size()) {
        return tabs_[activeTabIndex_].currentInput;
    }
    return empty;
}

void TerminalContent::inputString(const std::string& str) {
    if (activeTabIndex_ >= tabs_.size()) return;
    auto& tab = tabs_[activeTabIndex_];
    for (char c : str) {
        tab.currentInput.insert(tab.currentInput.begin() + tab.cursorPosition, c);
        tab.cursorPosition++;
    }
}

void TerminalContent::clearActiveBuffer() {
    if (activeTabIndex_ < tabs_.size()) {
        tabs_[activeTabIndex_].buffer.clear();
    }
}

void TerminalContent::appendOutput(const std::string& text, Color color, bool bold) {
    if (activeTabIndex_ < tabs_.size()) {
        tabs_[activeTabIndex_].buffer.push_back({text, color, bold});
    }
}

void TerminalContent::executeCurrentCommand() {
    if (activeTabIndex_ >= tabs_.size()) return;
    auto& tab = tabs_[activeTabIndex_];
    const std::string cmd = tab.currentInput;
    tab.currentInput.clear();
    tab.cursorPosition = 0;
    executeCommand(cmd);
}

void TerminalContent::executeCommand(const std::string& rawCmd) {
    if (activeTabIndex_ >= tabs_.size()) return;
    auto& tab = tabs_[activeTabIndex_];

    // Echo command line
    const std::string promptStr = tab.cwd + "> " + rawCmd;
    tab.buffer.push_back({promptStr, Color::fromHex(0x00FF9D), true});

    // Add to history
    if (!rawCmd.empty()) {
        tab.history.push_back(rawCmd);
        tab.historyIndex = static_cast<int32_t>(tab.history.size());
    }

    // Trim whitespace
    std::string trimmed = rawCmd;
    while (!trimmed.empty() && trimmed.front() == ' ') trimmed.erase(trimmed.begin());
    while (!trimmed.empty() && trimmed.back() == ' ') trimmed.pop_back();

    if (trimmed.empty()) return;

    // Split command and arguments
    std::string cmdName;
    std::string args;
    const size_t spacePos = trimmed.find(' ');
    if (spacePos != std::string::npos) {
        cmdName = trimmed.substr(0, spacePos);
        args = trimmed.substr(spacePos + 1);
    } else {
        cmdName = trimmed;
    }

    std::string lowerCmd = cmdName;
    std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (lowerCmd == "help") {
        tab.buffer.push_back({"Sovereign Terminal Built-In Commands:", Color::fromHex(0x00D4FF), true});
        tab.buffer.push_back({"  HELP       Displays this help message", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"  VER        Prints MicaNT Sovereign version", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"  DIR        Lists current directory contents", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"  CLS        Clears terminal display buffer", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"  WHOAMI     Displays current user & credentials", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"  TASKLIST   Displays active running processes", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"  ECHO       Displays message on standard output", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"  CALC       Launches Sovereign Calculator", Color::fromHex(0x00FF9D), false});
        tab.buffer.push_back({"  SETTINGS   Launches Personalization & Control Center", Color::fromHex(0x00FF9D), false});
        tab.buffer.push_back({"  EXPLORER   Launches File Explorer", Color::fromHex(0x00FF9D), false});
        tab.buffer.push_back({"  TASKMGR    Launches Task Manager & Monitor", Color::fromHex(0x00FF9D), false});
        tab.buffer.push_back({"  REGEDIT    Launches Sovereign Registry Editor", Color::fromHex(0x00FF9D), false});
        tab.buffer.push_back({"  EXIT       Closes current terminal tab", Color::fromHex(0xFF4D6D), false});
    } else if (lowerCmd == "ver") {
        tab.buffer.push_back({"MicaNT [Version 10.0.26100.1-SOVEREIGN] - Pure ISO C++23", Color::fromHex(0x00D4FF), true});
        tab.buffer.push_back({"Kernel: Dave Cutler PASSIVE_LEVEL Executive Bridge", Color::fromHex(0x7186A4), false});
    } else if (lowerCmd == "cls") {
        tab.buffer.clear();
    } else if (lowerCmd == "whoami") {
        tab.buffer.push_back({"micant\\admin (Dave Cutler Sovereign Administrator)", Color::fromHex(0x00FF9D), false});
    } else if (lowerCmd == "echo") {
        tab.buffer.push_back({args, Color::fromHex(0xCBD5E1), false});
    } else if (lowerCmd == "dir") {
        tab.buffer.push_back({" Directory of " + tab.cwd, Color::fromHex(0x00D4FF), true});
        tab.buffer.push_back({"", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"2026-10-08  12:00 PM    <DIR>          .", Color::fromHex(0x7186A4), false});
        tab.buffer.push_back({"2026-10-08  12:00 PM    <DIR>          ..", Color::fromHex(0x7186A4), false});
        tab.buffer.push_back({"2026-10-08  08:15 AM           124,416 cmd.exe", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"2026-10-08  08:15 AM           248,832 calc.exe", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"2026-10-08  08:15 AM           512,000 taskmgr.exe", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"2026-10-08  08:15 AM           892,100 explorer.exe", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"2026-10-08  08:15 AM           348,160 regedit.exe", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"2026-10-08  08:15 AM         1,048,576 surshell.exe", Color::fromHex(0x00FF9D), false});
        tab.buffer.push_back({"2026-10-08  08:15 AM           786,432 surwin.sys", Color::fromHex(0x7186A4), false});
        tab.buffer.push_back({"2026-10-08  08:15 AM         2,097,152 ntoskrnl.exe", Color::fromHex(0x00D4FF), false});
        tab.buffer.push_back({"               8 File(s)      6,057,668 bytes", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"               2 Dir(s)   2,453.2 GB free", Color::fromHex(0xCBD5E1), false});
    } else if (lowerCmd == "tasklist") {
        tab.buffer.push_back({"Image Name                     PID Session Name        Mem Usage", Color::fromHex(0x00D4FF), true});
        tab.buffer.push_back({"========================= ======== ================ ============", Color::fromHex(0x405572), false});
        tab.buffer.push_back({"System                           4 Services                   128 K", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"smss.exe                       312 Services                   420 K", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"csrss.exe                      440 Console                  2,140 K", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"lsass.exe                      528 Services                 4,890 K", Color::fromHex(0xCBD5E1), false});
        tab.buffer.push_back({"surshell.exe                  1024 Console                 14,250 K", Color::fromHex(0x00FF9D), true});
        tab.buffer.push_back({"terminal.exe                  1880 Console                  6,300 K", Color::fromHex(0xCBD5E1), false});
    } else if (lowerCmd == "calc" || lowerCmd == "calculator" || lowerCmd == "calc.exe") {
        tab.buffer.push_back({"Launching Sovereign Calculator...", Color::fromHex(0x00FF9D), false});
        if (onSpawnApp_) onSpawnApp_("calc", "");
    } else if (lowerCmd == "settings" || lowerCmd == "control" || lowerCmd == "control.exe") {
        tab.buffer.push_back({"Launching System Settings...", Color::fromHex(0x00FF9D), false});
        if (onSpawnApp_) onSpawnApp_("settings", "");
    } else if (lowerCmd == "explorer" || lowerCmd == "explorer.exe") {
        tab.buffer.push_back({"Launching File Explorer...", Color::fromHex(0x00FF9D), false});
        if (onSpawnApp_) onSpawnApp_("explorer", tab.cwd);
    } else if (lowerCmd == "taskmgr" || lowerCmd == "taskmgr.exe") {
        tab.buffer.push_back({"Launching Task Manager...", Color::fromHex(0x00FF9D), false});
        if (onSpawnApp_) onSpawnApp_("taskmgr", "");
    } else if (lowerCmd == "regedit" || lowerCmd == "regedit.exe") {
        tab.buffer.push_back({"Launching Sovereign Registry Editor...", Color::fromHex(0x00FF9D), false});
        if (onSpawnApp_) onSpawnApp_("regedit", "");
    } else if (lowerCmd == "exit") {
        closeTab(activeTabIndex_);
    } else {
        tab.buffer.push_back({"'" + cmdName + "' is not recognized as an internal or external command,", Color::fromHex(0xFF4D6D), false});
        tab.buffer.push_back({"operable program or batch file. Type 'help' for commands.", Color::fromHex(0xFF4D6D), false});
    }
}

void TerminalContent::render(Surface& s) {
    const auto& palette = ThemeManager::instance().palette();
    s.clear(Color{12, 16, 23, 255}); // Windows Terminal Dark Charcoal

    renderTabBar(s, palette);
    renderBuffer(s, palette);
}

void TerminalContent::renderTabBar(Surface& s, const ThemePalette& palette) {
    const int32_t barH = 34;
    const int32_t w = static_cast<int32_t>(s.width());

    // Tab bar background
    s.fillRect(Rect{0, 0, w, barH}, Color::fromHex(0x080C14));
    s.fillRect(Rect{0, barH - 1, w, 1}, Color::fromHex(0x1E293B));

    int32_t curX = 6;
    const int32_t tabW = 160;
    const int32_t tabH = 28;

    for (size_t i = 0; i < tabs_.size(); ++i) {
        auto& tab = tabs_[i];
        tab.tabBounds = Rect{curX, 4, tabW, tabH};
        tab.closeBounds = Rect{tab.tabBounds.right() - 20, 8, 16, 16};

        const bool isActive = (i == activeTabIndex_);
        const bool isHover = (hoveredTab_ == static_cast<int32_t>(i));

        if (isActive) {
            s.drawRoundedRect(tab.tabBounds, 4, Color::fromHex(0x101622), true);
            // Glowing cyan top bar on active tab
            s.fillRect(Rect{tab.tabBounds.x + 2, tab.tabBounds.y, tab.tabBounds.width - 4, 2}, palette.accentColor);
        } else if (isHover) {
            s.drawRoundedRect(tab.tabBounds, 4, Color::fromHex(0x141C2A), true);
        }

        // Tab icon
        IconRenderer::draw(s, IconId::TerminalTab, Point{tab.tabBounds.x + 8, tab.tabBounds.y + 6}, 16,
                           isActive ? std::make_optional(palette.accentColor) : std::nullopt);

        // Tab title
        std::string dispTitle = tab.title;
        if (dispTitle.size() > 14) dispTitle = dispTitle.substr(0, 11) + "...";
        s.drawString(tab.tabBounds.x + 28, tab.tabBounds.y + 7, dispTitle,
                     isActive ? Color::fromHex(0xFFFFFF) : Color::fromHex(0x94A3B8), 1);

        // Close button
        const bool isCloseHover = (hoveredClose_ == static_cast<int32_t>(i));
        if (isCloseHover) {
            s.drawRoundedRect(tab.closeBounds, 2, Color::fromHex(0xFF4D6D), true);
            s.drawString(tab.closeBounds.x + 4, tab.closeBounds.y + 2, "x", Color::fromHex(0xFFFFFF), 1);
        } else {
            s.drawString(tab.closeBounds.x + 4, tab.closeBounds.y + 2, "x",
                         isActive ? Color::fromHex(0x94A3B8) : Color::fromHex(0x475569), 1);
        }

        curX += tabW + 4;
    }

    // [+] New Tab Button
    newTabBounds_ = Rect{curX + 2, 6, 24, 24};
    if (hoverNewTab_) {
        s.drawRoundedRect(newTabBounds_, 4, Color::fromHex(0x1E293B), true);
    }
    s.drawString(newTabBounds_.x + 8, newTabBounds_.y + 4, "+",
                 hoverNewTab_ ? Color::fromHex(0xFFFFFF) : Color::fromHex(0x94A3B8), 1);
}

void TerminalContent::renderBuffer(Surface& s, const ThemePalette& palette) {
    if (activeTabIndex_ >= tabs_.size()) return;
    auto& tab = tabs_[activeTabIndex_];

    const int32_t startX = 14;
    int32_t startY = 46;
    const int32_t lineH = 16;
    const int32_t maxVisibleLines = (static_cast<int32_t>(s.height()) - startY - 24) / lineH;

    // Calculate lines to display from history
    int32_t lineCount = static_cast<int32_t>(tab.buffer.size());
    int32_t firstLine = std::max(0, lineCount - maxVisibleLines);

    for (int32_t i = firstLine; i < lineCount; ++i) {
        s.drawString(startX, startY, tab.buffer[i].text, tab.buffer[i].color, 1);
        startY += lineH;
    }

    // Draw active prompt line
    const std::string prompt = tab.cwd + "> ";
    s.drawString(startX, startY, prompt, Color::fromHex(0x00FF9D), 1);

    const int32_t promptW = static_cast<int32_t>(prompt.size()) * 8;
    s.drawString(startX + promptW, startY, tab.currentInput, Color::fromHex(0xFFFFFF), 1);

    // Blinking cursor
    const int32_t cursorX = startX + promptW + tab.cursorPosition * 8;
    if (cursorBlink_) {
        s.fillRect(Rect{cursorX, startY + 1, 8, 12}, palette.accentColor);
    }
}

bool TerminalContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    // Check [+] new tab
    if (newTabBounds_.contains(localPt)) {
        addTab("Sovereign Shell", "cmd");
        return true;
    }

    // Check tabs and close buttons
    for (size_t i = 0; i < tabs_.size(); ++i) {
        if (tabs_[i].closeBounds.contains(localPt)) {
            closeTab(i);
            return true;
        }
        if (tabs_[i].tabBounds.contains(localPt)) {
            selectTab(i);
            return true;
        }
    }

    return false;
}

bool TerminalContent::onMouseMove(Point localPt) {
    int32_t prevTab = hoveredTab_;
    int32_t prevClose = hoveredClose_;
    bool prevNew = hoverNewTab_;

    hoveredTab_ = -1;
    hoveredClose_ = -1;
    hoverNewTab_ = newTabBounds_.contains(localPt);

    for (size_t i = 0; i < tabs_.size(); ++i) {
        if (tabs_[i].closeBounds.contains(localPt)) {
            hoveredClose_ = static_cast<int32_t>(i);
        } else if (tabs_[i].tabBounds.contains(localPt)) {
            hoveredTab_ = static_cast<int32_t>(i);
        }
    }

    return (prevTab != hoveredTab_ || prevClose != hoveredClose_ || prevNew != hoverNewTab_);
}

bool TerminalContent::onMouseUp(Point, MouseButton) {
    return false;
}

bool TerminalContent::onKeyDown(KeyCode key, bool ctrl, bool shift, bool alt) {
    (void)ctrl; (void)shift; (void)alt;
    if (activeTabIndex_ >= tabs_.size()) return false;
    auto& tab = tabs_[activeTabIndex_];

    if (key == KeyCode::Enter) {
        executeCurrentCommand();
        return true;
    } else if (key == KeyCode::Backspace) {
        if (tab.cursorPosition > 0) {
            tab.currentInput.erase(tab.currentInput.begin() + tab.cursorPosition - 1);
            tab.cursorPosition--;
            return true;
        }
    } else if (key == KeyCode::Delete) {
        if (tab.cursorPosition < static_cast<int32_t>(tab.currentInput.size())) {
            tab.currentInput.erase(tab.currentInput.begin() + tab.cursorPosition);
            return true;
        }
    } else if (key == KeyCode::Left) {
        if (tab.cursorPosition > 0) {
            tab.cursorPosition--;
            return true;
        }
    } else if (key == KeyCode::Right) {
        if (tab.cursorPosition < static_cast<int32_t>(tab.currentInput.size())) {
            tab.cursorPosition++;
            return true;
        }
    } else if (key == KeyCode::Home) {
        tab.cursorPosition = 0;
        return true;
    } else if (key == KeyCode::End) {
        tab.cursorPosition = static_cast<int32_t>(tab.currentInput.size());
        return true;
    } else if (key == KeyCode::Up) {
        // Navigate history backwards
        if (!tab.history.empty() && tab.historyIndex > 0) {
            tab.historyIndex--;
            tab.currentInput = tab.history[tab.historyIndex];
            tab.cursorPosition = static_cast<int32_t>(tab.currentInput.size());
            return true;
        }
    } else if (key == KeyCode::Down) {
        // Navigate history forwards
        if (!tab.history.empty() && tab.historyIndex < static_cast<int32_t>(tab.history.size()) - 1) {
            tab.historyIndex++;
            tab.currentInput = tab.history[tab.historyIndex];
            tab.cursorPosition = static_cast<int32_t>(tab.currentInput.size());
            return true;
        } else if (tab.historyIndex >= static_cast<int32_t>(tab.history.size()) - 1) {
            tab.historyIndex = static_cast<int32_t>(tab.history.size());
            tab.currentInput.clear();
            tab.cursorPosition = 0;
            return true;
        }
    }
    return false;
}

bool TerminalContent::onCharInput(char c) {
    if (activeTabIndex_ >= tabs_.size()) return false;
    auto& tab = tabs_[activeTabIndex_];
    if (c >= 32 && c <= 126) {
        tab.currentInput.insert(tab.currentInput.begin() + tab.cursorPosition, c);
        tab.cursorPosition++;
        return true;
    }
    return false;
}

} // namespace surshell
