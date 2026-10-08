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

    // Seed mock telemetry history
    for (size_t i = 0; i < 36; ++i) {
        cpuHistory_.push_back(8.0f + 6.0f * std::sin(static_cast<float>(i) * 0.35f));
        memHistory_.push_back(13.0f + 0.5f * std::cos(static_cast<float>(i) * 0.2f));
        diskHistory_.push_back(std::max(0.5f, 3.0f + 2.5f * std::sin(static_cast<float>(i) * 0.5f)));
        netHistory_.push_back(std::max(0.2f, 2.0f + 1.8f * std::cos(static_cast<float>(i) * 0.4f)));
    }

    refresh();
}

void TaskManagerContent::updateVitals() {
    if (bridge_) {
        vitals_ = bridge_->queryVitals();
        processes_ = bridge_->queryProcesses();
    }
}

void TaskManagerContent::addCpuSample(float val) {
    cpuHistory_.push_back(std::clamp(val, 0.0f, 100.0f));
    if (cpuHistory_.size() > 40) cpuHistory_.pop_front();
}

void TaskManagerContent::addMemSample(float val) {
    memHistory_.push_back(std::clamp(val, 0.0f, 100.0f));
    if (memHistory_.size() > 40) memHistory_.pop_front();
}

void TaskManagerContent::addDiskSample(float val) {
    diskHistory_.push_back(std::clamp(val, 0.0f, 100.0f));
    if (diskHistory_.size() > 40) diskHistory_.pop_front();
}

void TaskManagerContent::addNetSample(float val) {
    netHistory_.push_back(std::clamp(val, 0.0f, 100.0f));
    if (netHistory_.size() > 40) netHistory_.pop_front();
}

void TaskManagerContent::refresh() {
    updateVitals();

    // Push new simulated telemetry samples
    float cpu = 12.0f;
    if (!cpuHistory_.empty()) {
        cpu = std::clamp(cpuHistory_.back() + ((rand() % 7) - 3.0f), 4.0f, 40.0f);
    }
    addCpuSample(cpu);

    float mem = 13.2f;
    if (!memHistory_.empty()) {
        mem = std::clamp(memHistory_.back() + ((rand() % 3) - 1.0f) * 0.1f, 12.0f, 16.0f);
    }
    addMemSample(mem);

    float disk = 2.4f;
    if (!diskHistory_.empty()) {
        disk = std::clamp(diskHistory_.back() + ((rand() % 5) - 2.0f) * 0.5f, 0.5f, 25.0f);
    }
    addDiskSample(disk);

    float net = 1.6f;
    if (!netHistory_.empty()) {
        net = std::clamp(netHistory_.back() + ((rand() % 5) - 2.0f) * 0.4f, 0.2f, 18.0f);
    }
    addNetSample(net);
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

    // 3. Performance Tab Resource Selector Cards
    if (activeTab_ == TaskManagerTab::Performance) {
        if (perfCardCpu_.contains(localPt)) {
            selectedResource_ = PerformanceResource::CPU;
            return true;
        }
        if (perfCardMem_.contains(localPt)) {
            selectedResource_ = PerformanceResource::Memory;
            return true;
        }
        if (perfCardDisk_.contains(localPt)) {
            selectedResource_ = PerformanceResource::Disk;
            return true;
        }
        if (perfCardNet_.contains(localPt)) {
            selectedResource_ = PerformanceResource::Network;
            return true;
        }
    }

    // 4. Process Table Row Selection (Processes & Details Tabs)
    if ((activeTab_ == TaskManagerTab::Processes || activeTab_ == TaskManagerTab::Details) &&
        processListBounds_.contains(localPt)) {
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
    if ((activeTab_ == TaskManagerTab::Processes || activeTab_ == TaskManagerTab::Details) &&
        processListBounds_.contains(localPt)) {
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

    // 3. Render content based on active tab
    if (activeTab_ == TaskManagerTab::Performance) {
        renderPerformanceTab(clientSurface, w, h, tabH);
    } else if (activeTab_ == TaskManagerTab::Details) {
        renderDetailsTab(clientSurface, w, h, tabH);
    } else {
        renderProcessesTab(clientSurface, w, h, tabH);
    }
}

void TaskManagerContent::renderProcessesTab(Surface& clientSurface, int32_t w, int32_t h, int32_t tabH) {
    // Vitals Ribbon Banner (Live Telemetry Tiles)
    const int32_t vitalsY = tabH + 8;
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

    // Main Process Table
    const int32_t headerY = vitalsY + vitalsH + 8;
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

        if (isSelected) {
            clientSurface.fillRect(Rect{17, rowY, w - 34, rowHeight}, Color::fromRgba(0, 212, 255, 45));
            clientSurface.fillRect(Rect{17, rowY, 3, rowHeight}, Color::fromHex(0x00D4FF));
        } else if (isHovered) {
            clientSurface.fillRect(Rect{17, rowY, w - 34, rowHeight}, Color::fromRgba(255, 255, 255, 10));
        } else if (i % 2 == 1) {
            clientSurface.fillRect(Rect{17, rowY, w - 34, rowHeight}, Color::fromRgba(255, 255, 255, 3));
        }

        IconRenderer::draw(clientSurface, IconRenderer::iconForAppId(proc.name), Point{24, rowY + 5}, 16);
        clientSurface.drawString(Point{46, rowY + 6}, proc.name, isSelected ? Color::fromHex(0x00D4FF) : Color::fromHex(0xFFFFFF));
        clientSurface.drawString(Point{250, rowY + 6}, std::to_string(proc.pid), Color::fromHex(0x8EA2BE));
        clientSurface.drawString(Point{320, rowY + 6}, proc.isAlive ? "Running" : "Suspended",
                                Color::fromHex(proc.isAlive ? 0x00FF9D : 0x8EA2BE));

        const float procCpu = (proc.name == "surshell.exe") ? 2.4f : ((i % 3 == 0) ? 0.8f : 0.1f);
        std::ostringstream pss;
        pss << std::fixed << std::setprecision(1) << procCpu << "%";
        clientSurface.drawString(Point{420, rowY + 6}, pss.str(), Color::fromHex(0x8EA2BE));

        const size_t memMb = (proc.memoryWorkingSetKb > 0) ? (proc.memoryWorkingSetKb / 1024) : 12;
        const std::string memStr = std::to_string(memMb) + " MB";
        clientSurface.drawString(Point{490, rowY + 6}, memStr, Color::fromHex(0x8EA2BE));

        rowY += rowHeight;
    }

    // Action Footer Bar
    const int32_t footerY = h - footerH;
    clientSurface.fillRect(Rect{0, footerY, w, footerH}, Color::fromHex(0x101826));
    clientSurface.fillRect(Rect{0, footerY, w, 1}, Color::fromHex(0x24354D));

    endTaskButtonBounds_ = Rect{w - 16 - 110, footerY + 8, 110, 28};
    const bool canEnd = selectedPid_.has_value();
    clientSurface.drawRoundedRect(endTaskButtonBounds_, 4,
                                 canEnd ? Color::fromRgba(255, 85, 85, 40) : Color::fromHex(0x182436), true);
    clientSurface.drawRoundedRect(endTaskButtonBounds_, 4,
                                 canEnd ? Color::fromHex(0xFF5555) : Color::fromHex(0x2A3D58), false);
    clientSurface.drawString(Point{endTaskButtonBounds_.x + 22, endTaskButtonBounds_.y + 6}, "End Task",
                            canEnd ? Color::fromHex(0xFF5555) : Color::fromHex(0x506580));

    refreshButtonBounds_ = Rect{endTaskButtonBounds_.x - 12 - 90, footerY + 8, 90, 28};
    clientSurface.drawRoundedRect(refreshButtonBounds_, 4, Color::fromHex(0x182436), true);
    clientSurface.drawRoundedRect(refreshButtonBounds_, 4, Color::fromHex(0x2A3D58), false);
    clientSurface.drawString(Point{refreshButtonBounds_.x + 18, refreshButtonBounds_.y + 6}, "Refresh", Color::fromHex(0xD0DCF0));

    clientSurface.drawString(Point{16, footerY + 12}, "Dave Cutler NT Executive Sandbox | Protected Subsystem", Color::fromHex(0x566B88));
}

void TaskManagerContent::renderPerformanceTab(Surface& clientSurface, int32_t w, int32_t h, int32_t tabH) {
    const int32_t topY = tabH + 8;
    constexpr int32_t cardW = 160;
    constexpr int32_t cardH = 62;
    constexpr int32_t cardGap = 6;

    // 1. Left Selector Cards (CPU, Memory, Disk, Network)
    auto drawPerfCard = [&](Rect& cardBounds, int32_t y, PerformanceResource res, const std::string& title,
                            const std::string& valStr, const std::deque<float>& history, Color lineCol) {
        cardBounds = Rect{16, y, cardW, cardH};
        const bool isSel = (selectedResource_ == res);

        clientSurface.drawRoundedRect(cardBounds, 4, isSel ? Color{24, 38, 58, 240} : Color{16, 24, 36, 200}, true);
        clientSurface.drawRoundedRect(cardBounds, 4, isSel ? lineCol : Color{36, 52, 78, 160}, false);

        if (isSel) {
            clientSurface.fillRect(Rect{cardBounds.x, cardBounds.y + 4, 3, cardBounds.height - 8}, lineCol);
        }

        clientSurface.drawString(Point{cardBounds.x + 8, cardBounds.y + 6}, title, isSel ? lineCol : Color::fromHex(0xD0DCF0));
        clientSurface.drawString(Point{cardBounds.x + 8, cardBounds.y + 22}, valStr, Color::fromHex(0x8EA2BE));

        // Mini sparkline in card
        const int32_t spW = 54;
        const int32_t spH = 26;
        const int32_t spX = cardBounds.right() - spW - 8;
        const int32_t spY = cardBounds.y + 18;
        clientSurface.fillRect(Rect{spX, spY, spW, spH}, Color{10, 14, 22, 255});
        clientSurface.drawRoundedRect(Rect{spX, spY, spW, spH}, 1, Color{28, 42, 64, 160}, false);

        if (history.size() >= 2) {
            for (size_t i = 0; i < history.size() && i < 18; ++i) {
                const int32_t bx = spX + static_cast<int32_t>(i * (spW / 18.0f));
                const int32_t barH = std::clamp(static_cast<int32_t>((history[i] / 50.0f) * spH), 1, spH);
                clientSurface.fillRect(Rect{bx, spY + spH - barH, 2, barH}, lineCol);
            }
        }
    };

    const float curCpu = cpuHistory_.empty() ? 12.0f : cpuHistory_.back();
    std::ostringstream cpuVal;
    cpuVal << std::fixed << std::setprecision(1) << curCpu << "%";
    drawPerfCard(perfCardCpu_, topY, PerformanceResource::CPU, "CPU", cpuVal.str() + " 3.8 GHz", cpuHistory_, Color::fromHex(0x00D4FF));

    drawPerfCard(perfCardMem_, topY + cardH + cardGap, PerformanceResource::Memory, "Memory", "4.2/32.0 GB (13%)", memHistory_, Color::fromHex(0x00FF9D));
    drawPerfCard(perfCardDisk_, topY + (cardH + cardGap) * 2, PerformanceResource::Disk, "Disk 0 (C:)", "2.4 MB/s (1%)", diskHistory_, Color::fromHex(0xFFB703));
    drawPerfCard(perfCardNet_, topY + (cardH + cardGap) * 3, PerformanceResource::Network, "Ethernet", "1.2 / 0.4 Mbps", netHistory_, Color::fromHex(0x9D4EDD));

    // 2. Right Main Telemetry View
    const int32_t mainX = 16 + cardW + 14;
    const int32_t mainW = w - mainX - 16;
    if (mainW < 200) return;

    // Header info of chosen resource
    std::string titleStr = "CPU";
    std::string specStr = "% Utilization: " + cpuVal.str();
    Color themeCol = Color::fromHex(0x00D4FF);
    const std::deque<float>* activeHist = &cpuHistory_;

    if (selectedResource_ == PerformanceResource::Memory) {
        titleStr = "Memory - Physical RAM Composition";
        specStr = "In use: 4.2 GB | Available: 27.8 GB | Speed: 6000 MT/s";
        themeCol = Color::fromHex(0x00FF9D);
        activeHist = &memHistory_;
    } else if (selectedResource_ == PerformanceResource::Disk) {
        titleStr = "Disk 0 (C:) - Solid State NVMe Drive";
        specStr = "Active time: 1% | Read speed: 2.1 MB/s | Write speed: 0.3 MB/s";
        themeCol = Color::fromHex(0xFFB703);
        activeHist = &diskHistory_;
    } else if (selectedResource_ == PerformanceResource::Network) {
        titleStr = "Ethernet - Gigabit Network Adapter";
        specStr = "Send: 420 Kbps | Receive: 1.2 Mbps | Zero Telemetry Guard";
        themeCol = Color::fromHex(0x9D4EDD);
        activeHist = &netHistory_;
    } else {
        titleStr = "CPU - 60 Second Telemetry (Historical Graph)";
        specStr = "% Utilization: " + cpuVal.str() + " | Sockets: 1 | Cores: 8 | Threads: 16";
    }

    clientSurface.drawString(Point{mainX, topY}, titleStr, themeCol, 1);
    clientSurface.drawString(Point{mainX, topY + 18}, specStr, Color::fromHex(0x8EA2BE), 1);

    // 3. High-Definition Oscillogram / Line Graph Area
    const int32_t graphY = topY + 36;
    const int32_t graphH = 170;
    const Rect graphBounds{mainX, graphY, mainW, graphH};

    clientSurface.fillRect(graphBounds, Color{10, 15, 24, 255});
    clientSurface.drawRoundedRect(graphBounds, 2, Color{32, 48, 72, 200}, false);

    // Grid lines (Horizontal at 25%, 50%, 75%, 100%)
    for (int g = 1; g <= 4; ++g) {
        const int32_t gy = graphBounds.bottom() - (graphH * g / 4);
        for (int32_t gx = graphBounds.x; gx < graphBounds.right(); gx += 8) {
            clientSurface.fillRect(Rect{gx, gy, 4, 1}, Color{22, 34, 52, 180});
        }
    }
    // Vertical grid lines every 40px
    for (int32_t gx = graphBounds.x + 40; gx < graphBounds.right(); gx += 40) {
        for (int32_t gy = graphBounds.y; gy < graphBounds.bottom(); gy += 8) {
            clientSurface.fillRect(Rect{gx, gy, 1, 4}, Color{22, 34, 52, 180});
        }
    }

    // Grid corner legends
    clientSurface.drawString(Point{graphBounds.x + 4, graphBounds.y + 4}, "100%", Color{60, 85, 120, 180});
    clientSurface.drawString(Point{graphBounds.x + 4, graphBounds.bottom() - 14}, "0%", Color{60, 85, 120, 180});
    clientSurface.drawString(Point{graphBounds.right() - 80, graphBounds.bottom() - 14}, "60 seconds", Color{60, 85, 120, 180});

    // Draw Smooth Line Graph with Gradient Area Shading
    if (activeHist->size() >= 2) {
        const float stepX = static_cast<float>(graphBounds.width) / static_cast<float>(activeHist->size() - 1);

        // Pass 1: Translucent fill under the curve
        for (size_t i = 0; i < activeHist->size(); ++i) {
            const int32_t px = graphBounds.x + static_cast<int32_t>(i * stepX);
            const float val = std::clamp((*activeHist)[i], 0.0f, 100.0f);
            const int32_t py = graphBounds.bottom() - static_cast<int32_t>((val / 100.0f) * (graphH - 4));
            const int32_t fillH = graphBounds.bottom() - py;
            if (fillH > 0) {
                clientSurface.fillRect(Rect{px, py, std::max(2, static_cast<int32_t>(stepX)), fillH},
                                       Color{themeCol.r, themeCol.g, themeCol.b, 35});
            }
        }

        // Pass 2: Connected Bright Line
        for (size_t i = 0; i + 1 < activeHist->size(); ++i) {
            const int32_t x1 = graphBounds.x + static_cast<int32_t>(i * stepX);
            const float v1 = std::clamp((*activeHist)[i], 0.0f, 100.0f);
            const int32_t y1 = graphBounds.bottom() - static_cast<int32_t>((v1 / 100.0f) * (graphH - 4));

            const int32_t x2 = graphBounds.x + static_cast<int32_t>((i + 1) * stepX);
            const float v2 = std::clamp((*activeHist)[i + 1], 0.0f, 100.0f);
            const int32_t y2 = graphBounds.bottom() - static_cast<int32_t>((v2 / 100.0f) * (graphH - 4));

            // Line segment rasterization
            const int32_t dx = std::abs(x2 - x1);
            const int32_t dy = std::abs(y2 - y1);
            const int32_t steps = std::max(dx, dy);
            for (int32_t s = 0; s <= steps; ++s) {
                const float t = (steps > 0) ? (static_cast<float>(s) / steps) : 0.0f;
                const int32_t lx = static_cast<int32_t>(x1 + t * (x2 - x1));
                const int32_t ly = static_cast<int32_t>(y1 + t * (y2 - y1));
                clientSurface.fillRect(Rect{lx, ly - 1, 2, 2}, themeCol);
            }
        }
    }

    // 4. Hardware Vitals Readout Box underneath chart
    const int32_t specsY = graphBounds.bottom() + 10;
    const int32_t specsH = h - specsY - 14;
    if (specsH > 40) {
        const Rect specsRect{mainX, specsY, mainW, specsH};
        clientSurface.drawRoundedRect(specsRect, 4, Color{14, 20, 32, 255}, true);
        clientSurface.drawRoundedRect(specsRect, 4, Color{28, 42, 64, 180}, false);

        const int32_t colW = mainW / 3;
        // Col 1
        clientSurface.drawString(Point{specsRect.x + 12, specsRect.y + 8}, "Utilization: " + cpuVal.str(), Color::fromHex(0xD0DCF0));
        clientSurface.drawString(Point{specsRect.x + 12, specsRect.y + 24}, "Speed: 3.80 GHz", Color::fromHex(0x8EA2BE));
        clientSurface.drawString(Point{specsRect.x + 12, specsRect.y + 40}, "Sockets: 1 (8 Cores)", Color::fromHex(0x8EA2BE));

        // Col 2
        clientSurface.drawString(Point{specsRect.x + colW + 8, specsRect.y + 8}, "Processes: " + std::to_string(processes_.size()), Color::fromHex(0xD0DCF0));
        clientSurface.drawString(Point{specsRect.x + colW + 8, specsRect.y + 24}, "Threads: 1,420", Color::fromHex(0x8EA2BE));
        clientSurface.drawString(Point{specsRect.x + colW + 8, specsRect.y + 40}, "Handles: 42,190", Color::fromHex(0x8EA2BE));

        // Col 3
        clientSurface.drawString(Point{specsRect.x + colW * 2 + 8, specsRect.y + 8}, "Virt: Enabled", Color::fromHex(0x00FF9D));
        clientSurface.drawString(Point{specsRect.x + colW * 2 + 8, specsRect.y + 24}, "L3 Cache: 32 MB", Color::fromHex(0x8EA2BE));
        clientSurface.drawString(Point{specsRect.x + colW * 2 + 8, specsRect.y + 40}, "Up: 0:14:22:08", Color::fromHex(0x8EA2BE));
    }
}

void TaskManagerContent::renderDetailsTab(Surface& clientSurface, int32_t w, int32_t h, int32_t tabH) {
    const int32_t headerY = tabH + 8;
    constexpr int32_t headerH = 24;
    clientSurface.fillRect(Rect{16, headerY, w - 32, headerH}, Color::fromHex(0x162235));
    clientSurface.fillRect(Rect{16, headerY + headerH - 1, w - 32, 1}, Color::fromHex(0x24354D));

    clientSurface.drawString(Point{24, headerY + 5}, "Image Name", Color::fromHex(0x8EA2BE));
    clientSurface.drawString(Point{220, headerY + 5}, "PID", Color::fromHex(0x8EA2BE));
    clientSurface.drawString(Point{280, headerY + 5}, "Status", Color::fromHex(0x8EA2BE));
    clientSurface.drawString(Point{360, headerY + 5}, "User Name", Color::fromHex(0x8EA2BE));
    clientSurface.drawString(Point{480, headerY + 5}, "CPU", Color::fromHex(0x8EA2BE));
    clientSurface.drawString(Point{540, headerY + 5}, "Working Set", Color::fromHex(0x8EA2BE));

    constexpr int32_t footerH = 44;
    processListBounds_ = Rect{16, headerY + headerH, w - 32, h - (headerY + headerH) - footerH};
    clientSurface.fillRect(processListBounds_, Color::fromHex(0x0E1420));
    clientSurface.drawRoundedRect(processListBounds_, 2, Color::fromHex(0x203046), false);

    constexpr int32_t rowHeight = 24;
    int32_t rowY = processListBounds_.y;

    for (size_t i = 0; i < processes_.size() && rowY + rowHeight <= processListBounds_.bottom(); ++i) {
        const auto& proc = processes_[i];
        const bool isSelected = (selectedPid_ && *selectedPid_ == proc.pid);
        const bool isHovered = (hoveredPid_ && *hoveredPid_ == proc.pid);

        if (isSelected) {
            clientSurface.fillRect(Rect{17, rowY, w - 34, rowHeight}, Color::fromRgba(0, 212, 255, 45));
            clientSurface.fillRect(Rect{17, rowY, 3, rowHeight}, Color::fromHex(0x00D4FF));
        } else if (isHovered) {
            clientSurface.fillRect(Rect{17, rowY, w - 34, rowHeight}, Color::fromRgba(255, 255, 255, 10));
        } else if (i % 2 == 1) {
            clientSurface.fillRect(Rect{17, rowY, w - 34, rowHeight}, Color::fromRgba(255, 255, 255, 3));
        }

        IconRenderer::draw(clientSurface, IconRenderer::iconForAppId(proc.name), Point{22, rowY + 4}, 16);
        clientSurface.drawString(Point{44, rowY + 5}, proc.name, isSelected ? Color::fromHex(0x00D4FF) : Color::fromHex(0xFFFFFF));
        clientSurface.drawString(Point{220, rowY + 5}, std::to_string(proc.pid), Color::fromHex(0x8EA2BE));
        clientSurface.drawString(Point{280, rowY + 5}, proc.isAlive ? "Running" : "Suspended",
                                Color::fromHex(proc.isAlive ? 0x00FF9D : 0x8EA2BE));
        clientSurface.drawString(Point{360, rowY + 5}, "MICANT\\admin", Color::fromHex(0x8EA2BE));

        const float procCpu = (proc.name == "surshell.exe") ? 2.4f : ((i % 3 == 0) ? 0.8f : 0.1f);
        std::ostringstream pss;
        pss << std::fixed << std::setprecision(1) << procCpu << "%";
        clientSurface.drawString(Point{480, rowY + 5}, pss.str(), Color::fromHex(0x8EA2BE));

        const size_t memMb = (proc.memoryWorkingSetKb > 0) ? (proc.memoryWorkingSetKb / 1024) : 12;
        clientSurface.drawString(Point{540, rowY + 5}, std::to_string(memMb) + " MB", Color::fromHex(0x8EA2BE));

        rowY += rowHeight;
    }

    // Action Footer Bar
    const int32_t footerY = h - footerH;
    clientSurface.fillRect(Rect{0, footerY, w, footerH}, Color::fromHex(0x101826));
    clientSurface.fillRect(Rect{0, footerY, w, 1}, Color::fromHex(0x24354D));

    endTaskButtonBounds_ = Rect{w - 16 - 110, footerY + 8, 110, 28};
    const bool canEnd = selectedPid_.has_value();
    clientSurface.drawRoundedRect(endTaskButtonBounds_, 4,
                                 canEnd ? Color::fromRgba(255, 85, 85, 40) : Color::fromHex(0x182436), true);
    clientSurface.drawRoundedRect(endTaskButtonBounds_, 4,
                                 canEnd ? Color::fromHex(0xFF5555) : Color::fromHex(0x2A3D58), false);
    clientSurface.drawString(Point{endTaskButtonBounds_.x + 22, endTaskButtonBounds_.y + 6}, "End Task",
                            canEnd ? Color::fromHex(0xFF5555) : Color::fromHex(0x506580));

    refreshButtonBounds_ = Rect{endTaskButtonBounds_.x - 12 - 90, footerY + 8, 90, 28};
    clientSurface.drawRoundedRect(refreshButtonBounds_, 4, Color::fromHex(0x182436), true);
    clientSurface.drawRoundedRect(refreshButtonBounds_, 4, Color::fromHex(0x2A3D58), false);
    clientSurface.drawString(Point{refreshButtonBounds_.x + 18, refreshButtonBounds_.y + 6}, "Refresh", Color::fromHex(0xD0DCF0));

    clientSurface.drawString(Point{16, footerY + 12}, "MicaNT Task Details | 64-Bit Sovereign Hardware Privilege Level 0", Color::fromHex(0x566B88));
}

} // namespace surshell
