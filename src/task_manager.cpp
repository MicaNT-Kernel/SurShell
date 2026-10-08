// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/task_manager.cpp)
// ============================================================================

#include "surshell/task_manager.hpp"
#include <iomanip>
#include <sstream>
#include <cmath>

namespace surshell {

TaskManagerContent::TaskManagerContent(KernelBridge* bridge)
    : bridge_(bridge) {
    if (!bridge_) {
        fallbackBridge_ = std::make_unique<KernelBridge>();
        fallbackBridge_->connectToExecutive("\\RPC_Control\\SurWinLpc");
        bridge_ = fallbackBridge_.get();
    }

    // Seed mock CPU history with reasonable idle values (5% - 18%)
    for (size_t i = 0; i < 32; ++i) {
        cpuHistory_.push_back(8.0f + 5.0f * std::sin(static_cast<float>(i) * 0.4f));
    }

    refresh();
}

void TaskManagerContent::updateVitals() {
    if (bridge_) {
        vitals_ = bridge_->queryVitals();
        processes_ = bridge_->queryProcesses();
    }
}

void TaskManagerContent::refresh() {
    updateVitals();

    // Push new simulated CPU utilization sample
    float newSample = 11.5f;
    if (!cpuHistory_.empty()) {
        newSample = std::clamp(cpuHistory_.back() + ((rand() % 7) - 3.0f), 4.0f, 35.0f);
    }
    cpuHistory_.push_back(newSample);
    if (cpuHistory_.size() > 32) {
        cpuHistory_.pop_front();
    }
}

bool TaskManagerContent::endSelectedTask() {
    if (!selectedPid_ || !bridge_) return false;

    const uint32_t pidToKill = *selectedPid_;
    std::string killedName = "Unknown";
    for (const auto& proc : processes_) {
        if (proc.pid == pidToKill) {
            killedName = proc.name;
            break;
        }
    }

    const bool success = bridge_->terminateProcess(pidToKill);
    if (success) {
        if (onTaskTerminated_) {
            onTaskTerminated_(pidToKill, killedName);
        }
        selectedPid_ = std::nullopt;
        refresh();
    }
    return success;
}

bool TaskManagerContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    // 1. Tab Bar Navigation
    if (tabProcessesBounds_.contains(localPt)) {
        activeTab_ = TaskManagerTab::Processes;
        return true;
    }
    if (tabPerformanceBounds_.contains(localPt)) {
        activeTab_ = TaskManagerTab::Performance;
        return true;
    }
    if (tabDetailsBounds_.contains(localPt)) {
        activeTab_ = TaskManagerTab::Details;
        return true;
    }

    // 2. Action Footer Buttons
    if (endTaskButtonBounds_.contains(localPt)) {
        return endSelectedTask();
    }
    if (refreshButtonBounds_.contains(localPt)) {
        refresh();
        return true;
    }

    // 3. Process Table Row Selection
    if (activeTab_ == TaskManagerTab::Processes && processListBounds_.contains(localPt)) {
        constexpr int32_t rowHeight = 26;
        const int32_t relativeY = localPt.y - processListBounds_.y + scrollOffset_;
        const size_t index = static_cast<size_t>(relativeY / rowHeight);
        if (index < processes_.size()) {
            selectedPid_ = processes_[index].pid;
            return true;
        } else {
            selectedPid_ = std::nullopt;
            return true;
        }
    }

    return false;
}

bool TaskManagerContent::onMouseMove(Point localPt) {
    if (activeTab_ == TaskManagerTab::Processes && processListBounds_.contains(localPt)) {
        constexpr int32_t rowHeight = 26;
        const int32_t relativeY = localPt.y - processListBounds_.y + scrollOffset_;
        const size_t index = static_cast<size_t>(relativeY / rowHeight);
        if (index < processes_.size()) {
            hoveredPid_ = processes_[index].pid;
        } else {
            hoveredPid_ = std::nullopt;
        }
        return true;
    }
    hoveredPid_ = std::nullopt;
    return false;
}

bool TaskManagerContent::onKeyDown(KeyCode key, bool ctrl, bool shift, bool alt) {
    (void)ctrl; (void)shift; (void)alt;
    if (key == KeyCode::Delete) {
        return endSelectedTask();
    }
    if (key == KeyCode::F5) {
        refresh();
        return true;
    }
    if (key == KeyCode::Down) {
        if (processes_.empty()) return false;
        if (!selectedPid_) {
            selectedPid_ = processes_.front().pid;
        } else {
            for (size_t i = 0; i < processes_.size(); ++i) {
                if (processes_[i].pid == *selectedPid_ && i + 1 < processes_.size()) {
                    selectedPid_ = processes_[i + 1].pid;
                    break;
                }
            }
        }
        return true;
    }
    if (key == KeyCode::Up) {
        if (processes_.empty()) return false;
        if (selectedPid_) {
            for (size_t i = 0; i < processes_.size(); ++i) {
                if (processes_[i].pid == *selectedPid_ && i > 0) {
                    selectedPid_ = processes_[i - 1].pid;
                    break;
                }
            }
        }
        return true;
    }
    return false;
}

void TaskManagerContent::render(Surface& clientSurface) {
    const int32_t w = clientSurface.width();
    const int32_t h = clientSurface.height();

    // 1. Solid Clean-Room Background
    clientSurface.clear(Color::fromHex(0x0C121D));

    // 2. Tab Bar Header
    constexpr int32_t tabH = 34;
    clientSurface.fillRect(Rect{0, 0, w, tabH}, Color::fromHex(0x101826));
    clientSurface.fillRect(Rect{0, tabH - 1, w, 1}, Color::fromHex(0x24354D));

    tabProcessesBounds_ = Rect{16, 4, 100, tabH - 8};
    tabPerformanceBounds_ = Rect{126, 4, 110, tabH - 8};
    tabDetailsBounds_ = Rect{246, 4, 90, tabH - 8};

    // Draw Tab Buttons
    auto drawTab = [&](const Rect& r, const std::string& label, bool active) {
        if (active) {
            clientSurface.fillRect(r, Color::fromRgba(0, 212, 255, 30));
            clientSurface.drawString(Point{r.x + 12, r.y + 7}, label, Color::fromHex(0x00D4FF));
            clientSurface.fillRect(Rect{r.x, tabH - 2, r.width, 2}, Color::fromHex(0x00D4FF));
        } else {
            clientSurface.drawString(Point{r.x + 12, r.y + 7}, label, Color::fromHex(0x8EA2BE));
        }
    };

    drawTab(tabProcessesBounds_, "Processes", activeTab_ == TaskManagerTab::Processes);
    drawTab(tabPerformanceBounds_, "Performance", activeTab_ == TaskManagerTab::Performance);
    drawTab(tabDetailsBounds_, "Details", activeTab_ == TaskManagerTab::Details);

    // 3. Vitals Ribbon Banner (Live Telemetry Tiles)
    constexpr int32_t vitalsY = tabH + 8;
    constexpr int32_t vitalsH = 54;
    const int32_t tileW = (w - 32 - 16) / 3;

    // Tile 1: CPU Telemetry & Sparkline
    const Rect cpuTile{16, vitalsY, tileW, vitalsH};
    clientSurface.drawRoundedRect(cpuTile, 4, Color::fromHex(0x141F30), true);
    clientSurface.drawRoundedRect(cpuTile, 4, Color::fromHex(0x283E5E), false);
    clientSurface.drawString(Point{cpuTile.x + 8, cpuTile.y + 6}, "CPU UTILIZATION", Color::fromHex(0x607898));
    const float currentCpu = cpuHistory_.empty() ? 12.0f : cpuHistory_.back();
    std::ostringstream cpuSs;
    cpuSs << std::fixed << std::setprecision(1) << currentCpu << "% (3.8 GHz)";
    clientSurface.drawString(Point{cpuTile.x + 8, cpuTile.y + 24}, cpuSs.str(), Color::fromHex(0x00D4FF));

    // Mini CPU sparkline inside tile
    const int32_t sparkX = cpuTile.right() - 80;
    const int32_t sparkY = cpuTile.y + 12;
    const int32_t sparkW = 70;
    const int32_t sparkH = 32;
    clientSurface.fillRect(Rect{sparkX, sparkY, sparkW, sparkH}, Color::fromHex(0x0E1420));
    if (cpuHistory_.size() >= 2) {
        for (size_t i = 0; i < cpuHistory_.size() && i < 20; ++i) {
            const int32_t bx = sparkX + static_cast<int32_t>(i * (sparkW / 20.0f));
            const int32_t barH = std::clamp(static_cast<int32_t>(cpuHistory_[i] / 50.0f * sparkH), 2, sparkH);
            clientSurface.fillRect(Rect{bx, sparkY + sparkH - barH, 2, barH}, Color::fromHex(0x00FF9D));
        }
    }

    // Tile 2: Memory (RAM)
    const Rect memTile{16 + tileW + 8, vitalsY, tileW, vitalsH};
    clientSurface.drawRoundedRect(memTile, 4, Color::fromHex(0x141F30), true);
    clientSurface.drawRoundedRect(memTile, 4, Color::fromHex(0x283E5E), false);
    clientSurface.drawString(Point{memTile.x + 8, memTile.y + 6}, "MEMORY (RAM)", Color::fromHex(0x607898));
    clientSurface.drawString(Point{memTile.x + 8, memTile.y + 24}, "4.2 GB / 32.0 GB (13%)", Color::fromHex(0x00FF9D));
    // Mini memory bar
    clientSurface.fillRect(Rect{memTile.x + 8, memTile.y + 42, memTile.width - 16, 4}, Color::fromHex(0x0E1420));
    clientSurface.fillRect(Rect{memTile.x + 8, memTile.y + 42, (memTile.width - 16) * 13 / 100, 4}, Color::fromHex(0x00FF9D));

    // Tile 3: Processes & Threads
    const Rect procTile{16 + (tileW + 8) * 2, vitalsY, tileW, vitalsH};
    clientSurface.drawRoundedRect(procTile, 4, Color::fromHex(0x141F30), true);
    clientSurface.drawRoundedRect(procTile, 4, Color::fromHex(0x283E5E), false);
    clientSurface.drawString(Point{procTile.x + 8, procTile.y + 6}, "SYSTEM PROCESSES", Color::fromHex(0x607898));
    const std::string procCountStr = std::to_string(processes_.size()) + " Active | 16 Threads";
    clientSurface.drawString(Point{procTile.x + 8, procTile.y + 24}, procCountStr, Color::fromHex(0xFFFFFF));
    clientSurface.drawString(Point{procTile.x + 8, procTile.y + 40}, "Subsystem: SurWin LPC", Color::fromHex(0x607898));

    // 4. Main Process Table
    constexpr int32_t headerY = vitalsY + vitalsH + 8;
    constexpr int32_t headerH = 24;
    clientSurface.fillRect(Rect{16, headerY, w - 32, headerH}, Color::fromHex(0x162235));
    clientSurface.fillRect(Rect{16, headerY + headerH - 1, w - 32, 1}, Color::fromHex(0x24354D));

    clientSurface.drawString(Point{24, headerY + 5}, "Process Name", Color::fromHex(0x8EA2BE));
    clientSurface.drawString(Point{250, headerY + 5}, "PID", Color::fromHex(0x8EA2BE));
    clientSurface.drawString(Point{320, headerY + 5}, "Status", Color::fromHex(0x8EA2BE));
    clientSurface.drawString(Point{420, headerY + 5}, "CPU %", Color::fromHex(0x8EA2BE));
    clientSurface.drawString(Point{490, headerY + 5}, "Memory (Working Set)", Color::fromHex(0x8EA2BE));

    constexpr int32_t footerH = 44;
    processListBounds_ = Rect{16, headerY + headerH, w - 32, h - (headerY + headerH) - footerH};
    clientSurface.fillRect(processListBounds_, Color::fromHex(0x0E1420));
    clientSurface.drawRoundedRect(processListBounds_, 2, Color::fromHex(0x203046), false);

    constexpr int32_t rowHeight = 26;
    int32_t rowY = processListBounds_.y;

    for (size_t i = 0; i < processes_.size() && rowY + rowHeight <= processListBounds_.bottom(); ++i) {
        const auto& proc = processes_[i];
        const bool isSelected = (selectedPid_ && *selectedPid_ == proc.pid);
        const bool isHovered = (hoveredPid_ && *hoveredPid_ == proc.pid);

        // Row background
        if (isSelected) {
            clientSurface.fillRect(Rect{17, rowY, w - 34, rowHeight}, Color::fromRgba(0, 212, 255, 45));
            clientSurface.fillRect(Rect{17, rowY, 3, rowHeight}, Color::fromHex(0x00D4FF)); // Left selection bar
        } else if (isHovered) {
            clientSurface.fillRect(Rect{17, rowY, w - 34, rowHeight}, Color::fromRgba(255, 255, 255, 10));
        } else if (i % 2 == 1) {
            clientSurface.fillRect(Rect{17, rowY, w - 34, rowHeight}, Color::fromRgba(255, 255, 255, 3));
        }

        // App Icon
        IconRenderer::draw(clientSurface, IconRenderer::iconForAppId(proc.name), Point{24, rowY + 5}, 16);

        // Process Name
        clientSurface.drawString(Point{46, rowY + 6}, proc.name, isSelected ? Color::fromHex(0x00D4FF) : Color::fromHex(0xFFFFFF));

        // PID
        clientSurface.drawString(Point{250, rowY + 6}, std::to_string(proc.pid), Color::fromHex(0x8EA2BE));

        // Status
        clientSurface.drawString(Point{320, rowY + 6}, proc.isAlive ? "Running" : "Suspended",
                                Color::fromHex(proc.isAlive ? 0x00FF9D : 0x8EA2BE));

        // Synthetic CPU %
        const float procCpu = (proc.name == "surshell.exe") ? 2.4f : ((i % 3 == 0) ? 0.8f : 0.1f);
        std::ostringstream pss;
        pss << std::fixed << std::setprecision(1) << procCpu << "%";
        clientSurface.drawString(Point{420, rowY + 6}, pss.str(), Color::fromHex(0x8EA2BE));

        // Memory Working Set
        const size_t memMb = (proc.memoryWorkingSetKb > 0) ? (proc.memoryWorkingSetKb / 1024) : 12;
        const std::string memStr = std::to_string(memMb) + " MB";
        clientSurface.drawString(Point{490, rowY + 6}, memStr, Color::fromHex(0x8EA2BE));

        rowY += rowHeight;
    }

    // 5. Action Footer Bar
    const int32_t footerY = h - footerH;
    clientSurface.fillRect(Rect{0, footerY, w, footerH}, Color::fromHex(0x101826));
    clientSurface.fillRect(Rect{0, footerY, w, 1}, Color::fromHex(0x24354D));

    // End Task Button on right
    endTaskButtonBounds_ = Rect{w - 16 - 110, footerY + 8, 110, 28};
    const bool canEnd = selectedPid_.has_value();
    clientSurface.drawRoundedRect(endTaskButtonBounds_, 4,
                                 canEnd ? Color::fromRgba(255, 85, 85, 40) : Color::fromHex(0x182436), true);
    clientSurface.drawRoundedRect(endTaskButtonBounds_, 4,
                                 canEnd ? Color::fromHex(0xFF5555) : Color::fromHex(0x2A3D58), false);
    clientSurface.drawString(Point{endTaskButtonBounds_.x + 22, endTaskButtonBounds_.y + 6}, "End Task",
                            canEnd ? Color::fromHex(0xFF5555) : Color::fromHex(0x506580));

    // Refresh Button next to End Task
    refreshButtonBounds_ = Rect{endTaskButtonBounds_.x - 12 - 90, footerY + 8, 90, 28};
    clientSurface.drawRoundedRect(refreshButtonBounds_, 4, Color::fromHex(0x182436), true);
    clientSurface.drawRoundedRect(refreshButtonBounds_, 4, Color::fromHex(0x2A3D58), false);
    clientSurface.drawString(Point{refreshButtonBounds_.x + 18, refreshButtonBounds_.y + 6}, "Refresh", Color::fromHex(0xD0DCF0));

    // Status on bottom left
    clientSurface.drawString(Point{16, footerY + 12}, "Dave Cutler NT Executive Sandbox | Protected Subsystem", Color::fromHex(0x566B88));
}

} // namespace surshell
