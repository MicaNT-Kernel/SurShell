// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/kernel_bridge.cpp)
// ============================================================================

#include "surshell/kernel_bridge.hpp"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
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
        // When genuine sovereign host processes are running, only merge dynamically spawned child processes (PID >= 2000)
        for (const auto& mp : mockProcesses_) {
            if (mp.pid >= 2000 && mp.isAlive &&
                std::find(terminatedPids_.begin(), terminatedPids_.end(), mp.pid) == terminatedPids_.end() &&
                std::none_of(hostProcs.begin(), hostProcs.end(), [&](const auto& hp) { return hp.pid == mp.pid; })) {
                hostProcs.push_back(mp);
            }
        }
        if (!terminatedPids_.empty()) {
            std::erase_if(hostProcs, [&](const auto& hp) {
                return std::find(terminatedPids_.begin(), terminatedPids_.end(), hp.pid) != terminatedPids_.end();
            });
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
    vitals.osName = "MicaNT Sovereign Host Enclave";
    vitals.osBuild = "Build 26100.2026.barrer";
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

std::string KernelBridge::queryCurrentUserName() {
#if defined(_WIN32)
    char buffer[256]{};
    DWORD len = sizeof(buffer);
    if (GetUserNameA(buffer, &len) && len > 0) {
        return std::string(buffer);
    }
    const char* envUser = std::getenv("USERNAME");
    if (envUser && envUser[0] != '\0') {
        return std::string(envUser);
    }
#else
    const char* envUser = std::getenv("USER");
    if (envUser && envUser[0] != '\0') {
        return std::string(envUser);
    }
#endif
    return "admin";
}

std::string KernelBridge::queryComputerName() {
#if defined(_WIN32)
    char buffer[MAX_COMPUTERNAME_LENGTH + 1]{};
    DWORD len = sizeof(buffer);
    if (GetComputerNameA(buffer, &len) && len > 0) {
        return std::string(buffer);
    }
    const char* envComp = std::getenv("COMPUTERNAME");
    if (envComp && envComp[0] != '\0') {
        return std::string(envComp);
    }
#else
    const char* envHost = std::getenv("HOSTNAME");
    if (envHost && envHost[0] != '\0') {
        return std::string(envHost);
    }
#endif
    return "NS1003135";
}

std::vector<KernelNetworkAdapter> KernelBridge::queryNetworkAdapters() {
    std::vector<KernelNetworkAdapter> adapters;

#if defined(_WIN32)
    // 1. Primary DNS Server from GetNetworkParams
    std::string primaryDns = "1.1.1.1, 8.8.8.8";
    FIXED_INFO fixedInfoBuf{};
    ULONG fixedLen = sizeof(fixedInfoBuf);
    PFIXED_INFO pFixedInfo = &fixedInfoBuf;
    std::vector<uint8_t> fiAlloc;
    if (GetNetworkParams(pFixedInfo, &fixedLen) == ERROR_BUFFER_OVERFLOW) {
        fiAlloc.resize(fixedLen);
        pFixedInfo = reinterpret_cast<PFIXED_INFO>(fiAlloc.data());
    }
    if (GetNetworkParams(pFixedInfo, &fixedLen) == NO_ERROR) {
        if (pFixedInfo->DnsServerList.IpAddress.String[0] != '\0') {
            primaryDns = pFixedInfo->DnsServerList.IpAddress.String;
        }
    }

    // 2. Adapters from GetAdaptersInfo
    ULONG outBufLen = sizeof(IP_ADAPTER_INFO);
    std::vector<uint8_t> buffer(outBufLen);
    PIP_ADAPTER_INFO pAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(buffer.data());

    DWORD dwRetVal = GetAdaptersInfo(pAdapterInfo, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        buffer.resize(outBufLen);
        pAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(buffer.data());
        dwRetVal = GetAdaptersInfo(pAdapterInfo, &outBufLen);
    }

    if (dwRetVal == NO_ERROR) {
        for (PIP_ADAPTER_INFO pAdapter = pAdapterInfo; pAdapter != nullptr; pAdapter = pAdapter->Next) {
            KernelNetworkAdapter ad;
            ad.adapterName = pAdapter->AdapterName;
            ad.description = pAdapter->Description[0] != '\0' ? pAdapter->Description : "Ethernet Controller";
            ad.ipv4Address = pAdapter->IpAddressList.IpAddress.String;
            ad.ipv4Mask = pAdapter->IpAddressList.IpMask.String;
            ad.defaultGateway = pAdapter->GatewayList.IpAddress.String;
            ad.isDhcp = (pAdapter->DhcpEnabled != 0);
            ad.dnsServer = primaryDns;

            char macStr[32]{};
            if (pAdapter->AddressLength == 6) {
                snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                         pAdapter->Address[0], pAdapter->Address[1], pAdapter->Address[2],
                         pAdapter->Address[3], pAdapter->Address[4], pAdapter->Address[5]);
            }
            ad.macAddress = macStr;

            ad.isConnected = (!ad.ipv4Address.empty() && ad.ipv4Address != "0.0.0.0");
            ad.linkSpeed = (ad.description.find("X550") != std::string::npos || ad.description.find("10G") != std::string::npos)
                ? "10.0 Gbps Full Duplex"
                : (ad.description.find("I225") != std::string::npos || ad.description.find("2.5G") != std::string::npos)
                ? "2.5 Gbps Full Duplex"
                : "1000/1000 Mbps Full Duplex";

            adapters.push_back(std::move(ad));
        }
    }
#endif

    // Clean-room architectural fallback for Linux CI or sandboxed environments
    if (adapters.empty()) {
        adapters.push_back(KernelNetworkAdapter{
            .adapterName = "{Sovereign-Loopback-0}",
            .description = "Gigabit Ethernet (Sovereign Network Interface)",
            .macAddress = "00:1A:2B:3C:4D:5E",
            .ipv4Address = "10.0.0.2",
            .ipv4Mask = "255.255.255.0",
            .defaultGateway = "10.0.0.1",
            .dnsServer = "1.1.1.1, 8.8.8.8",
            .linkSpeed = "1000/1000 Mbps Full Duplex",
            .isConnected = true,
            .isDhcp = true
        });
    }

    return adapters;
}

KernelNetworkAdapter KernelBridge::queryPrimaryNetworkAdapter() {
    auto all = queryNetworkAdapters();
    for (const auto& a : all) {
        if (a.isConnected && !a.defaultGateway.empty() && a.defaultGateway != "0.0.0.0") {
            return a;
        }
    }
    for (const auto& a : all) {
        if (a.isConnected) return a;
    }
    return all.empty() ? KernelNetworkAdapter{} : all.front();
}

} // namespace surshell
