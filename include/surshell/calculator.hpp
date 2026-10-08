// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/calculator.hpp)
//
// Modern Sovereign Acrylic Calculator (calc.exe).
// Provides arithmetic evaluation, function modifiers (sqrt, reciprocal, square),
// expression history ribbon, and responsive mouse/keyboard keypad interaction.
// ============================================================================

#pragma once

#include "types.hpp"
#include "window_manager.hpp"
#include "theme.hpp"
#include "icons.hpp"
#include <string>
#include <vector>

namespace surshell {

struct CalcButton {
    std::string label;
    std::string opCode;
    Rect bounds{};
    bool isAccent{false};
    bool isHovered{false};
};

class CalculatorContent : public IWindowContent {
public:
    CalculatorContent();

    [[nodiscard]] std::string display() const noexcept { return display_; }
    [[nodiscard]] std::string history() const noexcept { return history_; }

    void inputDigit(char digit);
    void inputDecimal();
    void inputOperator(char op);
    void calculateResult();
    void clearAll();
    void clearEntry();
    void backspace();
    void negate();
    void squareRoot();
    void square();
    void reciprocal();
    void percentage();

    // IWindowContent overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;
    bool onCharInput(char c) override;

private:
    std::string display_{"0"};
    std::string history_{""};
    double accumulator_{0.0};
    char pendingOp_{0};
    bool clearOnNextDigit_{false};

    std::vector<CalcButton> buttons_{};

    void setupButtons(int32_t width, int32_t height);
    void executeOpCode(const std::string& code);
};

} // namespace surshell
