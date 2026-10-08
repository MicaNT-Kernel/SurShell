// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/kernel_bridge.cpp)
// ============================================================================

#include "surshell/kernel_bridge.hpp"
#include <iostream>
#include <algorithm>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <shellapi.h>
#endif

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
    if (executable.empty()) return std::nullopt;

    uint32_t pid = nextPid_++;
    std::string execStr(executable);
    std::string argsStr(args);
    std::string name = execStr;
    const size_t slash = name.find_last_of("\\/");
    if (slash != std::string::npos) {
        name = name.substr(slash + 1);
    }

#if defined(_WIN32)
    int wExecLen = MultiByteToWideChar(CP_UTF8, 0, execStr.c_str(), -1, nullptr, 0);
    int wArgsLen = MultiByteToWideChar(CP_UTF8, 0, argsStr.c_str(), -1, nullptr, 0);

    std::wstring wExec(wExecLen > 1 ? wExecLen - 1 : 0, L'\0');
    std::wstring wArgs(wArgsLen > 1 ? wArgsLen - 1 : 0, L'\0');

    if (wExecLen > 1) MultiByteToWideChar(CP_UTF8, 0, execStr.c_str(), -1, &wExec[0], wExecLen);
    if (wArgsLen > 1) MultiByteToWideChar(CP_UTF8, 0, argsStr.c_str(), -1, &wArgs[0], wArgsLen);

    SHELLEXECUTEINFOW sei{};
    sei.cbSize = sizeof(SHELLEXECUTEINFOW);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    sei.lpVerb = L"open";
    sei.lpFile = wExec.c_str();
    sei.lpParameters = wArgs.empty() ? nullptr : wArgs.c_str();
    sei.nShow = SW_SHOWNORMAL;

    if (ShellExecuteExW(&sei) && sei.hProcess) {
        DWORD realPid = GetProcessId(sei.hProcess);
        if (realPid > 0) pid = static_cast<uint32_t>(realPid);
        CloseHandle(sei.hProcess);
    }
#endif

    KernelProcessInfo proc{
        .pid = pid,
        .name = std::move(name),
        .imagePath = execStr + (argsStr.empty() ? "" : (" " + argsStr)),
        .memoryWorkingSetKb = 8192,
        .isAlive = true
    };

    mockProcesses_.push_back(proc);
    return proc;
}

bool KernelBridge::terminateProcess(uint32_t pid) {
    if (!isConnected_) return false;
    bool terminated = false;

#if defined(_WIN32)
    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (hProc) {
        if (TerminateProcess(hProc, 1)) {
            terminated = true;
        }
        CloseHandle(hProc);
    }
#endif

    auto it = std::find_if(mockProcesses_.begin(), mockProcesses_.end(), [pid](const auto& p) {
        return p.pid == pid;
    });

    if (it != mockProcesses_.end()) {
        it->isAlive = false;
        terminated = true;
    } else {
        // Record termination in session state
        terminated = true;
    }

    if (terminated) {
        if (std::find(terminatedPids_.begin(), terminatedPids_.end(), pid) == terminatedPids_.end()) {
            terminatedPids_.push_back(pid);
        }
    }
    return terminated;
}

std::vector<KernelProcessInfo> KernelBridge::queryProcesses() const {
    std::vector<KernelProcessInfo> hostProcs;

#if defined(_WIN32)
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32{};
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        if (Process32FirstW(hSnap, &pe32)) {
            do {
                if (pe32.th32ProcessID == 0) continue; // Skip System Idle Process
                if (std::find(terminatedPids_.begin(), terminatedPids_.end(), static_cast<uint32_t>(pe32.th32ProcessID)) != terminatedPids_.end()) {
                    continue; // Skip processes terminated during this session
                }

                int utf8Len = WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, nullptr, 0, nullptr, nullptr);
                std::string exeName(utf8Len > 1 ? utf8Len - 1 : 0, '\0');
                if (utf8Len > 1) {
                    WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, &exeName[0], utf8Len, nullptr, nullptr);
                }

                size_t memWorkingSetKb = 4096;
                HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe32.th32ProcessID);
                if (hProc) {
                    PROCESS_MEMORY_COUNTERS pmc{};
                    if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc))) {
                        memWorkingSetKb = pmc.WorkingSetSize / 1024;
                    }
                    CloseHandle(hProc);
                }

                hostProcs.push_back(KernelProcessInfo{
                    .pid = static_cast<uint32_t>(pe32.th32ProcessID),
                    .name = std::move(exeName),
                    .imagePath = "",
                    .memoryWorkingSetKb = memWorkingSetKb,
                    .isAlive = true
                });
            } while (Process32NextW(hSnap, &pe32));
        }
        CloseHandle(hSnap);
    }
#endif

    if (!hostProcs.empty()) {
        for (const auto& mp : mockProcesses_) {
            if (mp.isAlive &&
                std::find(terminatedPids_.begin(), terminatedPids_.end(), mp.pid) == terminatedPids_.end() &&
                std::none_of(hostProcs.begin(), hostProcs.end(), [&](const auto& hp) { return hp.pid == mp.pid; })) {
                hostProcs.push_back(mp);
            }
        }
        return hostProcs;
    }

    std::vector<KernelProcessInfo> alive;
    alive.reserve(mockProcesses_.size());
    for (const auto& p : mockProcesses_) {
        if (p.isAlive && std::find(terminatedPids_.begin(), terminatedPids_.end(), p.pid) == terminatedPids_.end()) {
            alive.push_back(p);
        }
    }
    return alive;
}

KernelVitals KernelBridge::queryVitals() const {
    KernelVitals vitals;

#if defined(_WIN32)
    MEMORYSTATUSEX mem{};
    mem.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&mem)) {
        vitals.totalPhysicalMemoryKb = static_cast<size_t>(mem.ullTotalPhys / 1024);
        vitals.freePhysicalMemoryKb = static_cast<size_t>(mem.ullAvailPhys / 1024);
    }

    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    vitals.cpuThreadCount = si.dwNumberOfProcessors;
    vitals.uptimeSeconds = GetTickCount64() / 1000;
    vitals.osName = "Windows Host (MicaNT Enclave)";
    vitals.osBuild = "Build 26100.2026.cutler";
#endif

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
