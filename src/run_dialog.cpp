// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/run_dialog.cpp)
// ============================================================================

#include "surshell/run_dialog.hpp"
#include <algorithm>

namespace surshell {

RunDialogContent::RunDialogContent(std::string initialCmd)
    : command_(std::move(initialCmd)) {}

void RunDialogContent::execute() {
    if (onExecute_) {
        onExecute_(command_);
    }
}

void RunDialogContent::render(Surface& s) {
    const auto& palette = ThemeManager::instance().palette();
    const int32_t w = static_cast<int32_t>(s.width());
    const int32_t h = static_cast<int32_t>(s.height());

    s.clear(Color{16, 22, 34, 255});

    // Top Header Icon & Descriptive Text
    IconRenderer::draw(s, IconId::RunDialog, Point{18, 16}, 32, palette.accentColor);
    s.drawString(62, 16, "Type the name of a program, folder, document, or", palette.textPrimary, 1);
    s.drawString(62, 32, "Internet resource, and MicaNT will open it for you.", palette.textSecondary, 1);

    // Command Prompt Input Row
    s.drawString(18, 67, "Open:", palette.textPrimary, 1);

    inputBoxBounds_ = Rect{62, 62, w - 82, 26};
    s.drawRoundedRect(inputBoxBounds_, 4, Color{10, 14, 22, 240}, true);
    s.drawRoundedRect(inputBoxBounds_, 4, palette.accentColor.withAlpha(180), false);

    s.drawString(inputBoxBounds_.x + 8, inputBoxBounds_.y + 6, command_, palette.textPrimary, 1);

    // Blinking cursor
    const int32_t curX = inputBoxBounds_.x + 8 + static_cast<int32_t>(command_.size() * 6);
    if (curX < inputBoxBounds_.right() - 4) {
        s.fillRect(Rect{curX, inputBoxBounds_.y + 5, 2, 14}, palette.accentColor);
    }

    // Button Row (Bottom Right)
    const int32_t btnW = 80;
    const int32_t btnH = 26;
    const int32_t btnY = h - 38;

    btnBrowseBounds_ = Rect{w - 92, btnY, btnW, btnH};
    btnCancelBounds_ = Rect{w - 180, btnY, btnW, btnH};
    btnOkBounds_     = Rect{w - 268, btnY, btnW, btnH};

    auto drawBtn = [&](const Rect& rect, const std::string& label, bool isDefault, bool hovered) {
        Color bgCol = hovered ? Color::fromHex(0x283854) : Color::fromHex(0x182436);
        Color borderCol = isDefault ? palette.accentColor : Color::fromHex(0x354765);
        if (isDefault && hovered) {
            bgCol = palette.accentColor.withAlpha(200);
        }

        s.drawRoundedRect(rect, 4, bgCol, true);
        s.drawRoundedRect(rect, 4, borderCol, false);

        const int32_t txtW = static_cast<int32_t>(label.size() * 6);
        s.drawString(rect.centerX() - txtW / 2, rect.y + 6, label,
                     (isDefault && hovered) ? Color::fromHex(0x060B12) : palette.textPrimary, 1);
    };

    drawBtn(btnOkBounds_, "OK", true, okHovered_);
    drawBtn(btnCancelBounds_, "Cancel", false, cancelHovered_);
    drawBtn(btnBrowseBounds_, "Browse...", false, browseHovered_);
}

bool RunDialogContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    if (btnOkBounds_.contains(localPt)) {
        execute();
        return true;
    }
    if (btnCancelBounds_.contains(localPt)) {
        if (onClose_) onClose_();
        return true;
    }
    if (btnBrowseBounds_.contains(localPt)) {
        if (onBrowse_) onBrowse_();
        return true;
    }
    return false;
}

bool RunDialogContent::onMouseMove(Point localPt) {
    const bool prevOk = okHovered_;
    const bool prevCan = cancelHovered_;
    const bool prevBro = browseHovered_;

    okHovered_ = btnOkBounds_.contains(localPt);
    cancelHovered_ = btnCancelBounds_.contains(localPt);
    browseHovered_ = btnBrowseBounds_.contains(localPt);

    return prevOk != okHovered_ || prevCan != cancelHovered_ || prevBro != browseHovered_;
}

bool RunDialogContent::onKeyDown(KeyCode key, bool, bool, bool) {
    switch (key) {
        case KeyCode::Enter:
            execute();
            return true;
        case KeyCode::Escape:
            if (onClose_) onClose_();
            return true;
        case KeyCode::Backspace:
            if (!command_.empty()) {
                command_.pop_back();
            }
            return true;
        default:
            break;
    }
    return false;
}

bool RunDialogContent::onCharInput(char c) {
    if (c >= 32 && c <= 126) {
        command_ += c;
        return true;
    }
    if (c == '\b') {
        if (!command_.empty()) {
            command_.pop_back();
        }
        return true;
    }
    if (c == '\r' || c == '\n') {
        execute();
        return true;
    }
    return false;
}

} // namespace surshell
