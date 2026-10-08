// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/run_dialog.hpp)
//
// Modern Sovereign Run Dialog (run.exe / Win+R).
// Provides executable / document quick launch prompt with command parsing,
// autocomplete history, and direct process spawning via the Executive.
// ============================================================================

#pragma once

#include "types.hpp"
#include "window_manager.hpp"
#include "theme.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <functional>

namespace surshell {

class RunDialogContent : public IWindowContent {
public:
    explicit RunDialogContent(std::string initialCmd = "cmd");

    [[nodiscard]] std::string command() const noexcept { return command_; }
    void setCommand(std::string cmd) { command_ = std::move(cmd); }

    using ExecuteCallback = std::function<void(const std::string& command)>;
    using CloseCallback = std::function<void()>;
    using BrowseCallback = std::function<void()>;

    void setExecuteCallback(ExecuteCallback cb) { onExecute_ = std::move(cb); }
    void setCloseCallback(CloseCallback cb) { onClose_ = std::move(cb); }
    void setBrowseCallback(BrowseCallback cb) { onBrowse_ = std::move(cb); }

    void execute();

    // IWindowContent overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;
    bool onCharInput(char c) override;

private:
    std::string command_{"cmd"};

    Rect inputBoxBounds_{};
    Rect btnOkBounds_{};
    Rect btnCancelBounds_{};
    Rect btnBrowseBounds_{};

    bool okHovered_{false};
    bool cancelHovered_{false};
    bool browseHovered_{false};

    ExecuteCallback onExecute_{};
    CloseCallback onClose_{};
    BrowseCallback onBrowse_{};
};

} // namespace surshell
