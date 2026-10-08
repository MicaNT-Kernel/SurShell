// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/kernel_bridge.cpp)
// ============================================================================

#include "surshell/kernel_bridge.hpp"
#include <iostream>
#include <algorithm>

namespace surshell {

KernelBridge::KernelBridge() {
    // Seed initial core sovereign kernel processes
    mockProcesses_ = {
        KernelProcessInfo{.pid = 4, .name = "System", .imagePath = "micant_kernel.exe", .memoryWorkingSetKb = 14200, .isAlive = true},
        KernelProcessInfo{.pid = 240, .name = "smss.exe", .imagePath = "system32\\smss.exe", .memoryWorkingSetKb = 1200, .isAlive = true},
        KernelProcessInfo{.pid = 380, .name = "csrss.exe", .imagePath = "system32\\csrss.exe", .memoryWorkingSetKb = 4600, .isAlive = true},
        KernelProcessInfo{.pid = 490, .name = "wininit.exe", .imagePath = "system32\\wininit.exe", .memoryWorkingSetKb = 3400, .isAlive = true},
        KernelProcessInfo{.pid = 620, .name = "lsass.exe", .imagePath = "system32\\lsass.exe", .memoryWorkingSetKb = 8200, .isAlive = true},
        KernelProcessInfo{.pid = 750, .name = "sentinel.exe", .imagePath = "system32\\sentinel.exe", .memoryWorkingSetKb = 6100, .isAlive = true},
        KernelProcessInfo{.pid = 910, .name = "razzlenet.exe", .imagePath = "system32\\razzlenet.exe", .memoryWorkingSetKb = 5400, .isAlive = true},
        KernelProcessInfo{.pid = 1100, .name = "surshell.exe", .imagePath = "system32\\surshell.exe", .memoryWorkingSetKb = 11800, .isAlive = true}
    };
}

KernelBridge::~KernelBridge() {
    disconnect();
}

bool KernelBridge::connectToExecutive(std::string_view portName) {
    portName_ = std::string(portName);
    // In standalone or host-hosted mode, we initialize the executive LPC session
    isConnected_ = true;
    return true;
}

void KernelBridge::disconnect() {
    isConnected_ = false;
    portName_.clear();
}

bool KernelBridge::dispatchInputEvent(Point /*mousePos*/, MouseButton /*mouseBtn*/, char /*keyChar*/) {
    if (!isConnected_) return false;
    // In live MicaNT, this formats and flushes an LPC message into SurWin
    return true;
}

uint32_t KernelBridge::registerWindowWithSurWin(std::string_view /*title*/, Rect /*bounds*/) {
    if (!isConnected_) return 0;
    return nextWindowHandle_++;
}

bool KernelBridge::unregisterWindowWithSurWin(uint32_t windowHandle) {
    if (!isConnected_ || windowHandle == 0) return false;
    return true;
}

std::optional<KernelProcessInfo> KernelBridge::spawnProcess(std::string_view executable, std::string_view args) {
    if (!isConnected_) return std::nullopt;

    const uint32_t pid = nextPid_++;
    std::string execStr(executable);
    std::string name = execStr;
    const size_t slash = name.find_last_of("\\/");
    if (slash != std::string::npos) {
        name = name.substr(slash + 1);
    }

    KernelProcessInfo proc{
        .pid = pid,
        .name = std::move(name),
        .imagePath = execStr + (args.empty() ? "" : (" " + std::string(args))),
        .memoryWorkingSetKb = 8192,
        .isAlive = true
    };

    mockProcesses_.push_back(proc);
    return proc;
}

bool KernelBridge::terminateProcess(uint32_t pid) {
    if (!isConnected_) return false;
    auto it = std::find_if(mockProcesses_.begin(), mockProcesses_.end(), [pid](const auto& p) {
        return p.pid == pid;
    });

    if (it != mockProcesses_.end()) {
        it->isAlive = false;
        return true;
    }
    return false;
}

std::vector<KernelProcessInfo> KernelBridge::queryProcesses() const {
    std::vector<KernelProcessInfo> alive;
    alive.reserve(mockProcesses_.size());
    for (const auto& p : mockProcesses_) {
        if (p.isAlive) alive.push_back(p);
    }
    return alive;
}

KernelVitals KernelBridge::queryVitals() const {
    KernelVitals vitals;
    vitals.activeProcessCount = static_cast<uint32_t>(queryProcesses().size());
    return vitals;
}

bool KernelBridge::setPowerState(std::string_view state) {
    if (!isConnected_) return false;
    currentPowerState_ = std::string(state);
    return true;
}

std::string KernelBridge::powerState() const {
    return currentPowerState_;
}

} // namespace surshell
