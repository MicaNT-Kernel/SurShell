// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/task_manager.hpp)
//
// Modern Task Manager & Resource Monitor (taskmgr.exe).
// Provides live CPU/RAM sparkline telemetry, active process tree inspection,
// historical performance line graphs, and process lifecycle termination.
// ============================================================================

#pragma once

#include "types.hpp"
#include "window_manager.hpp"
#include "kernel_bridge.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <deque>
#include <memory>

namespace surshell {

enum class TaskManagerTab {
    Processes,
    Performance,
    Details
};

enum class PerformanceResource {
    CPU,
    Memory,
    Disk,
    Network
};

class TaskManagerContent : public IWindowContent {
public:
    explicit TaskManagerContent(KernelBridge* bridge = nullptr);

    void refresh();
    bool endSelectedTask();

    [[nodiscard]] std::optional<uint32_t> selectedPid() const noexcept { return selectedPid_; }
    void selectPid(uint32_t pid) noexcept { selectedPid_ = pid; }

    [[nodiscard]] size_t processCount() const noexcept { return processes_.size(); }
    [[nodiscard]] const std::vector<KernelProcessInfo>& processes() const noexcept { return processes_; }
    [[nodiscard]] TaskManagerTab activeTab() const noexcept { return activeTab_; }
    void setActiveTab(TaskManagerTab tab) noexcept { activeTab_ = tab; }

    [[nodiscard]] PerformanceResource performanceResource() const noexcept { return selectedResource_; }
    void setPerformanceResource(PerformanceResource res) noexcept { selectedResource_ = res; }

    [[nodiscard]] const std::deque<float>& cpuHistory() const noexcept { return cpuHistory_; }
    [[nodiscard]] const std::deque<float>& memHistory() const noexcept { return memHistory_; }
    [[nodiscard]] const std::deque<float>& diskHistory() const noexcept { return diskHistory_; }
    [[nodiscard]] const std::deque<float>& netHistory() const noexcept { return netHistory_; }

    void addCpuSample(float val);
    void addMemSample(float val);
    void addDiskSample(float val);
    void addNetSample(float val);

    // IWindowContent overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;

    using TaskTerminatedCallback = std::function<void(uint32_t pid, const std::string& name)>;
    void setTerminatedCallback(TaskTerminatedCallback cb) { onTaskTerminated_ = std::move(cb); }

private:
    void renderProcessesTab(Surface& clientSurface, int32_t w, int32_t h, int32_t tabH);
    void renderPerformanceTab(Surface& clientSurface, int32_t w, int32_t h, int32_t tabH);
    void renderDetailsTab(Surface& clientSurface, int32_t w, int32_t h, int32_t tabH);

    KernelBridge* bridge_{nullptr};
    std::unique_ptr<KernelBridge> fallbackBridge_{};
    std::vector<KernelProcessInfo> processes_{};
    KernelVitals vitals_{};

    // Telemetry History
    std::deque<float> cpuHistory_{};  // 32-60 samples (%)
    std::deque<float> memHistory_{};  // 32-60 samples (%)
    std::deque<float> diskHistory_{}; // 32-60 samples (KB/s or %)
    std::deque<float> netHistory_{};  // 32-60 samples (Kbps or %)

    std::optional<uint32_t> selectedPid_{std::nullopt};
    std::optional<uint32_t> hoveredPid_{std::nullopt};
    TaskManagerTab activeTab_{TaskManagerTab::Processes};
    PerformanceResource selectedResource_{PerformanceResource::CPU};

    // UI Regions
    Rect tabProcessesBounds_{};
    Rect tabPerformanceBounds_{};
    Rect tabDetailsBounds_{};
    Rect endTaskButtonBounds_{};
    Rect refreshButtonBounds_{};
    Rect processListBounds_{};

    // Performance Side Cards
    Rect perfCardCpu_{};
    Rect perfCardMem_{};
    Rect perfCardDisk_{};
    Rect perfCardNet_{};

    int32_t scrollOffset_{0};
    TaskTerminatedCallback onTaskTerminated_{};

    void updateVitals();
};

} // namespace surshell
