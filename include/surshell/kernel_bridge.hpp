// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/kernel_bridge.hpp)
//
// Clean-Room LPC & Executive Syscall Bridge to MicaNT's SurWin/CSRSS Subsystem.
// Conforms strictly to Microsoft's MIT-licensed win32metadata and Dave Cutler's
// Windows NT 4.0 "SUR" (Shell Update Release) taxonomy.
// ============================================================================

#pragma once

#include "types.hpp"
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <optional>

namespace surshell {

enum class KernelLpcMessageType : uint32_t {
    Connect = 0x0001,
    DispatchInput = 0x0002,
    CreateWindow = 0x0003,
    DestroyWindow = 0x0004,
    SpawnProcess = 0x0005,
    QueryVitals = 0x0006,
    Heartbeat = 0x0007
};

struct KernelProcessInfo {
    uint32_t pid{0};
    std::string name{};
    std::string imagePath{};
    size_t memoryWorkingSetKb{0};
    bool isAlive{true};
};

struct KernelVitals {
    std::string osName{"MicaNT Cutler Edition"};
    std::string osBuild{"Build 26100.1.cutler.2026"};
    size_t totalPhysicalMemoryKb{32 * 1024 * 1024}; // 32 GB
    size_t freePhysicalMemoryKb{28 * 1024 * 1024};  // 28 GB
    uint32_t activeProcessCount{48};
    uint32_t cpuThreadCount{16};
    uint64_t uptimeSeconds{3600};
};

class KernelBridge {
public:
    KernelBridge();
    ~KernelBridge();

    // Connection to MicaNT Executive / SurWin CSRSS LPC Port
    bool connectToExecutive(std::string_view portName = "\\RPC_Control\\SurWinLpc");
    void disconnect();
    [[nodiscard]] bool isConnected() const noexcept { return isConnected_; }

    // Syscall & LPC Dispatching
    bool dispatchInputEvent(Point mousePos, MouseButton mouseBtn, char keyChar = '\0');
    uint32_t registerWindowWithSurWin(std::string_view title, Rect bounds);
    bool unregisterWindowWithSurWin(uint32_t windowHandle);

    // Process Launching & Executive Interaction
    std::optional<KernelProcessInfo> spawnProcess(std::string_view executable, std::string_view args);
    bool terminateProcess(uint32_t pid);
    [[nodiscard]] std::vector<KernelProcessInfo> queryProcesses() const;

    // Vitals Telemetry
    [[nodiscard]] KernelVitals queryVitals() const;

private:
    bool isConnected_{false};
    std::string portName_{};
    uint32_t nextPid_{2000};
    uint32_t nextWindowHandle_{100};
    std::vector<KernelProcessInfo> mockProcesses_;
};

} // namespace surshell
