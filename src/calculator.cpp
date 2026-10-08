// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/calculator.cpp)
// ============================================================================

#include "surshell/calculator.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace surshell {

namespace {

std::string formatNumber(double val) {
    if (std::isnan(val) || std::isinf(val)) {
        return "Error";
    }
    // Check if integer
    if (std::abs(val - std::round(val)) < 1e-9 && std::abs(val) < 1e12) {
        return std::to_string(static_cast<int64_t>(std::round(val)));
    }
    std::ostringstream ss;
    ss << std::setprecision(8) << val;
    return ss.str();
}

} // namespace

CalculatorContent::CalculatorContent() {
    // Initial buttons configuration
    setupButtons(340, 480);
}

void CalculatorContent::setupButtons(int32_t width, int32_t height) {
    buttons_.clear();

    const struct Def {
        std::string label;
        std::string code;
        bool accent;
    } grid[6][4] = {
        {{"%", "%", false}, {"CE", "CE", false}, {"C", "C", false}, {"<", "BS", false}},
        {{"1/x", "RECIP", false}, {"x^2", "SQR", false}, {"v/", "SQRT", false}, {"/", "/", false}},
        {{"7", "7", false}, {"8", "8", false}, {"9", "9", false}, {"*", "*", false}},
        {{"4", "4", false}, {"5", "5", false}, {"6", "6", false}, {"-", "-", false}},
        {{"1", "1", false}, {"2", "2", false}, {"3", "3", false}, {"+", "+", false}},
        {{"/-", "NEG", false}, {"0", "0", false}, {".", ".", false}, {"=", "=", true}}
    };

    const int32_t margin = 12;
    const int32_t displayH = 100;
    const int32_t keyAreaY = displayH + margin;
    const int32_t keyAreaW = width - margin * 2;
    const int32_t keyAreaH = height - keyAreaY - margin;

    const int32_t gap = 6;
    const int32_t btnW = (keyAreaW - gap * 3) / 4;
    const int32_t btnH = (keyAreaH - gap * 5) / 6;

    for (int32_t r = 0; r < 6; ++r) {
        for (int32_t c = 0; c < 4; ++c) {
            buttons_.push_back(CalcButton{
                .label = grid[r][c].label,
                .opCode = grid[r][c].code,
                .bounds = Rect{
                    margin + c * (btnW + gap),
                    keyAreaY + r * (btnH + gap),
                    btnW,
                    btnH
                },
                .isAccent = grid[r][c].accent,
                .isHovered = false
            });
        }
    }
}

void CalculatorContent::inputDigit(char digit) {
    if (clearOnNextDigit_ || display_ == "0" || display_ == "Error") {
        display_ = std::string(1, digit);
        clearOnNextDigit_ = false;
    } else {
        if (display_.size() < 16) {
            display_ += digit;
        }
    }
}

void CalculatorContent::inputDecimal() {
    if (clearOnNextDigit_ || display_ == "Error") {
        display_ = "0.";
        clearOnNextDigit_ = false;
    } else if (display_.find('.') == std::string::npos) {
        display_ += '.';
    }
}

void CalculatorContent::inputOperator(char op) {
    try {
        const double curVal = std::stod(display_);
        if (pendingOp_ != 0 && !clearOnNextDigit_) {
            calculateResult();
        } else {
            accumulator_ = curVal;
        }
        pendingOp_ = op;
        clearOnNextDigit_ = true;
        history_ = formatNumber(accumulator_) + " " + std::string(1, op);
    } catch (...) {
        display_ = "Error";
    }
}

void CalculatorContent::calculateResult() {
    if (pendingOp_ == 0) return;
    try {
        const double curVal = std::stod(display_);
        double result = 0.0;
        switch (pendingOp_) {
            case '+': result = accumulator_ + curVal; break;
            case '-': result = accumulator_ - curVal; break;
            case '*': result = accumulator_ * curVal; break;
            case '/':
                if (std::abs(curVal) < 1e-12) {
                    display_ = "Cannot divide by 0";
                    clearOnNextDigit_ = true;
                    pendingOp_ = 0;
                    history_ = "";
                    return;
                }
                result = accumulator_ / curVal;
                break;
            default: result = curVal; break;
        }
        history_ = formatNumber(accumulator_) + " " + std::string(1, pendingOp_) + " " + formatNumber(curVal) + " =";
        accumulator_ = result;
        display_ = formatNumber(result);
        pendingOp_ = 0;
        clearOnNextDigit_ = true;
    } catch (...) {
        display_ = "Error";
    }
}

void CalculatorContent::clearAll() {
    display_ = "0";
    history_ = "";
    accumulator_ = 0.0;
    pendingOp_ = 0;
    clearOnNextDigit_ = false;
}

void CalculatorContent::clearEntry() {
    display_ = "0";
    clearOnNextDigit_ = false;
}

void CalculatorContent::backspace() {
    if (clearOnNextDigit_) return;
    if (display_.size() > 1 && display_ != "Error") {
        display_.pop_back();
    } else {
        display_ = "0";
    }
}

void CalculatorContent::negate() {
    try {
        double val = std::stod(display_);
        val = -val;
        display_ = formatNumber(val);
    } catch (...) {}
}

void CalculatorContent::squareRoot() {
    try {
        double val = std::stod(display_);
        if (val < 0.0) {
            display_ = "Invalid input";
            clearOnNextDigit_ = true;
            return;
        }
        history_ = "sqrt(" + formatNumber(val) + ")";
        display_ = formatNumber(std::sqrt(val));
        clearOnNextDigit_ = true;
    } catch (...) {}
}

void CalculatorContent::square() {
    try {
        double val = std::stod(display_);
        history_ = "sqr(" + formatNumber(val) + ")";
        display_ = formatNumber(val * val);
        clearOnNextDigit_ = true;
    } catch (...) {}
}

void CalculatorContent::reciprocal() {
    try {
        double val = std::stod(display_);
        if (std::abs(val) < 1e-12) {
            display_ = "Cannot divide by 0";
            clearOnNextDigit_ = true;
            return;
        }
        history_ = "1/(" + formatNumber(val) + ")";
        display_ = formatNumber(1.0 / val);
        clearOnNextDigit_ = true;
    } catch (...) {}
}

void CalculatorContent::percentage() {
    try {
        double val = std::stod(display_);
        if (pendingOp_ != 0) {
            val = (accumulator_ * val) / 100.0;
        } else {
            val = val / 100.0;
        }
        display_ = formatNumber(val);
        clearOnNextDigit_ = true;
    } catch (...) {}
}

void CalculatorContent::executeOpCode(const std::string& code) {
    if (code.size() == 1 && code[0] >= '0' && code[0] <= '9') {
        inputDigit(code[0]);
    } else if (code == ".") {
        inputDecimal();
    } else if (code == "+" || code == "-" || code == "*" || code == "/") {
        inputOperator(code[0]);
    } else if (code == "=") {
        calculateResult();
    } else if (code == "C") {
        clearAll();
    } else if (code == "CE") {
        clearEntry();
    } else if (code == "BS") {
        backspace();
    } else if (code == "NEG") {
        negate();
    } else if (code == "SQRT") {
        squareRoot();
    } else if (code == "SQR") {
        square();
    } else if (code == "RECIP") {
        reciprocal();
    } else if (code == "%") {
        percentage();
    }
}

void CalculatorContent::render(Surface& s) {
    const auto& palette = ThemeManager::instance().palette();
    const int32_t w = static_cast<int32_t>(s.width());
    const int32_t h = static_cast<int32_t>(s.height());

    setupButtons(w, h);

    // Background
    s.clear(Color{14, 18, 28, 255});

    // Top Display Glass Pane
    const Rect dispR{12, 10, w - 24, 88};
    s.drawRoundedRect(dispR, 6, Color{10, 14, 22, 230}, true);
    s.drawRoundedRect(dispR, 6, Color{30, 42, 64, 180}, false);

    // History Ribbon
    if (!history_.empty()) {
        const int32_t histLen = static_cast<int32_t>(history_.size() * 6);
        s.drawString(std::max(dispR.x + 10, dispR.right() - histLen - 12), dispR.y + 12, history_, palette.textSecondary, 1);
    }

    // Main Value Display (large scale 2 if possible, or right-aligned)
    const int32_t valLen = static_cast<int32_t>(display_.size() * 12);
    const int32_t valX = std::max(dispR.x + 10, dispR.right() - valLen - 14);
    s.drawString(valX, dispR.y + 44, display_, palette.textPrimary, 2);

    // Buttons
    for (const auto& btn : buttons_) {
        Color bgCol = Color::fromHex(0x182234);
        Color borderCol = Color::fromHex(0x283850);
        Color txtCol = palette.textPrimary;

        if (btn.isAccent) {
            bgCol = btn.isHovered ? palette.accentColor : palette.accentColor.withAlpha(210);
            borderCol = palette.accentColor;
            txtCol = Color::fromHex(0x060B12);
        } else if (btn.isHovered) {
            bgCol = Color::fromHex(0x283854);
            borderCol = Color::fromHex(0x405578);
        }

        s.drawRoundedRect(btn.bounds, 6, bgCol, true);
        s.drawRoundedRect(btn.bounds, 6, borderCol, false);

        // Centered label
        const int32_t txtW = static_cast<int32_t>(btn.label.size() * 6);
        s.drawString(btn.bounds.centerX() - txtW / 2, btn.bounds.centerY() - 4, btn.label, txtCol, 1);
    }
}

bool CalculatorContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;
    for (const auto& btn : buttons_) {
        if (btn.bounds.contains(localPt)) {
            executeOpCode(btn.opCode);
            return true;
        }
    }
    return false;
}

bool CalculatorContent::onMouseMove(Point localPt) {
    bool changed = false;
    for (auto& btn : buttons_) {
        const bool hov = btn.bounds.contains(localPt);
        if (btn.isHovered != hov) {
            btn.isHovered = hov;
            changed = true;
        }
    }
    return changed;
}

bool CalculatorContent::onKeyDown(KeyCode key, bool, bool, bool) {
    if (key >= KeyCode::Num0 && key <= KeyCode::Num9) {
        inputDigit(static_cast<char>('0' + (static_cast<int>(key) - static_cast<int>(KeyCode::Num0))));
        return true;
    }
    switch (key) {
        case KeyCode::Enter:
            calculateResult();
            return true;
        case KeyCode::Backspace:
            backspace();
            return true;
        case KeyCode::Escape:
            clearAll();
            return true;
        default:
            break;
    }
    return false;
}

bool CalculatorContent::onCharInput(char c) {
    if (c >= '0' && c <= '9') {
        inputDigit(c);
        return true;
    }
    if (c == '+' || c == '-' || c == '*' || c == '/') {
        inputOperator(c);
        return true;
    }
    if (c == '=' || c == '\r' || c == '\n') {
        calculateResult();
        return true;
    }
    if (c == '.') {
        inputDecimal();
        return true;
    }
    if (c == '\b') {
        backspace();
        return true;
    }
    return false;
}

} // namespace surshell
