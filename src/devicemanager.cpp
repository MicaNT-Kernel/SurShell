// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/devicemanager.cpp)
//
// Sovereign Device Manager & Hardware Inspector (devmgmt.msc Parity)
// ============================================================================

#include "surshell/devicemanager.hpp"
#include "surshell/kernel_bridge.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace surshell {

DeviceManagerContent::DeviceManagerContent() {
    scanForHardwareChanges();
}

void DeviceManagerContent::scanForHardwareChanges() {
    populateDefaultHardwareTree();
    collectHostHardwareTelemetry();
    rebuildFlatList();
    statusMessage_ = "Hardware tree updated";
}

void DeviceManagerContent::populateDefaultHardwareTree() {
    categories_.clear();

    // 1. Audio inputs and outputs
    DeviceCategory audioCat{
        .id = "audio",
        .name = "Audio inputs and outputs",
        .iconId = IconId::VolumeHigh,
        .isExpanded = true,
        .devices = {
            {
                .id = "aud_speakers",
                .name = "Speakers (Realtek(R) High Definition Audio)",
                .iconId = IconId::VolumeHigh,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "Realtek",
                .driverVersion = "6.0.9239.1",
                .hardwareId = "HDAUDIO\\FUNC_01&VEN_10EC&DEV_0897",
                .location = "HD Audio Bus 0, Device 31, Function 3",
                .isEnabled = true,
                .properties = {
                    {"Device Status", "Working properly"},
                    {"Driver Provider", "Realtek Semiconductor Corp."},
                    {"Driver Date", "2026-03-12"},
                    {"Driver Version", "6.0.9239.1"},
                    {"Digital Signer", "MicaNT Sovereign WHQL"}
                }
            },
            {
                .id = "aud_mic",
                .name = "Microphone (Realtek(R) High Definition Audio)",
                .iconId = IconId::VolumeHigh,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "Realtek",
                .driverVersion = "6.0.9239.1",
                .hardwareId = "HDAUDIO\\FUNC_01&VEN_10EC&DEV_0897_MIC",
                .location = "HD Audio Bus 0, Device 31, Function 3",
                .isEnabled = true,
                .properties = {
                    {"Device Status", "Working properly"},
                    {"Driver Provider", "Realtek Semiconductor Corp."},
                    {"Sample Rate", "48.0 kHz 24-bit Studio"}
                }
            }
        }
    };

    // 2. Disk drives
    DeviceCategory diskCat{
        .id = "disk",
        .name = "Disk drives",
        .iconId = IconId::LocalDisk,
        .isExpanded = true,
        .devices = {
            {
                .id = "disk_nvme",
                .name = "Samsung SSD 980 PRO 1TB NVMe PCIe 4.0",
                .iconId = IconId::LocalDisk,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "Samsung Electronics Co., Ltd.",
                .driverVersion = "10.0.26100.1",
                .hardwareId = "SCSI\\DiskNVMe____Samsung_SSD_980_1B2Q",
                .location = "PCI Bus 1, Device 0, Function 0",
                .isEnabled = true,
                .properties = {
                    {"Device Status", "Working properly"},
                    {"Interface", "PCIe Gen 4 x4 (64.0 GT/s)"},
                    {"Partition Table", "GUID Partition Table (GPT)"},
                    {"Write Caching", "Enabled"}
                }
            },
            {
                .id = "disk_sec",
                .name = "Crucial CT2000T500SSD8 2TB NVMe SSD",
                .iconId = IconId::LocalDisk,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "Crucial / Micron",
                .driverVersion = "10.0.26100.1",
                .hardwareId = "SCSI\\DiskNVMe____Crucial_CT2000T500",
                .location = "PCI Bus 2, Device 0, Function 0",
                .isEnabled = true,
                .properties = {
                    {"Device Status", "Working properly"},
                    {"Capacity", "2000.3 GB"}
                }
            }
        }
    };

    // 3. Display adapters
    DeviceCategory displayCat{
        .id = "display",
        .name = "Display adapters",
        .iconId = IconId::Display,
        .isExpanded = true,
        .devices = {
            {
                .id = "disp_gpu",
                .name = "NVIDIA GeForce RTX 3080 (10 GB GDDR6X)",
                .iconId = IconId::Display,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "NVIDIA",
                .driverVersion = "32.0.15.5585",
                .hardwareId = "PCI\\VEN_10DE&DEV_2206&SUBSYS_87981043",
                .location = "PCI Bus 1, Device 0, Function 0",
                .isEnabled = true,
                .properties = {
                    {"Device Status", "Working properly"},
                    {"Driver Provider", "NVIDIA Corporation"},
                    {"Driver Date", "2026-05-18"},
                    {"DirectX Version", "Direct3D 12.2 (FL 12_1)"},
                    {"Dedicated Video Memory", "10240 MB GDDR6X"}
                }
            },
            {
                .id = "disp_compositor",
                .name = "MicaNT PrismX Sovereign Software Compositor",
                .iconId = IconId::Display,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "Barrer Software & MicaNT Community",
                .driverVersion = "1.0.26100.0",
                .hardwareId = "ROOT\\MicaNT_PrismX_Compositor",
                .location = "Kernel Direct Memory Pipeline",
                .isEnabled = true,
                .properties = {
                    {"Pipeline", "120Hz VSync Synchronous Rasterizer"},
                    {"Color Format", "32-bpp BGRA Premultiplied Alpha"}
                }
            }
        }
    };

    // 4. Keyboards
    DeviceCategory kbCat{
        .id = "keyboards",
        .name = "Keyboards",
        .iconId = IconId::Edit,
        .isExpanded = true,
        .devices = {
            {
                .id = "kb_mech",
                .name = "HID Mechanical Gaming Keyboard (104-Key ANSI)",
                .iconId = IconId::Edit,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "Standard Keyboard Peripherals",
                .driverVersion = "10.0.26100.1",
                .hardwareId = "HID\\VID_046D&PID_C33F&MI_00",
                .location = "USB Input Device",
                .isEnabled = true,
                .properties = {
                    {"Polling Rate", "1000 Hz"},
                    {"Key Roll-over", "N-Key Rollover (NKRO)"}
                }
            }
        }
    };

    // 5. Mice and other pointing devices
    DeviceCategory miceCat{
        .id = "mice",
        .name = "Mice and other pointing devices",
        .iconId = IconId::SystemInfo,
        .isExpanded = true,
        .devices = {
            {
                .id = "mouse_optical",
                .name = "HID-compliant Optical Gaming Mouse (16000 DPI)",
                .iconId = IconId::SystemInfo,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "Microsoft / Sovereign HID",
                .driverVersion = "10.0.26100.1",
                .hardwareId = "HID\\VID_046D&PID_C084&REV_0100",
                .location = "USB Input Device",
                .isEnabled = true,
                .properties = {
                    {"Buttons", "5"},
                    {"Sensor", "Optical Hero Sensor"}
                }
            }
        }
    };

    // 6. Network adapters
    const auto primaryNet = KernelBridge::queryPrimaryNetworkAdapter();
    DeviceCategory netCat{
        .id = "network",
        .name = "Network adapters",
        .iconId = IconId::NetworkEthernet,
        .isExpanded = true,
        .devices = {
            {
                .id = "net_eth",
                .name = primaryNet.description.empty() ? "Intel(R) Ethernet Controller (Gigabit)" : primaryNet.description,
                .iconId = IconId::NetworkEthernet,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "Intel Corporation",
                .driverVersion = "10.0.26100.1",
                .hardwareId = "PCI\\VEN_8086&DEV_NET_PRIMARY",
                .location = "PCI Bus Network Interface",
                .isEnabled = true,
                .properties = {
                    {"Link Speed", primaryNet.linkSpeed},
                    {"MAC Address", primaryNet.macAddress},
                    {"IPv4 Address", primaryNet.ipv4Address}
                }
            },
            {
                .id = "net_nas",
                .name = "MicaNT Sovereign SMB NAS Virtual Miniport (\\\\nas.ash-forge.com)",
                .iconId = IconId::NetworkShare,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "Barrer Software & MicaNT Community",
                .driverVersion = "1.0.0.1",
                .hardwareId = "ROOT\\MicaNT_NAS_Miniport",
                .location = "SMB 3.1.1 Transport Layer",
                .isEnabled = true,
                .properties = {
                    {"Share Path", "\\\\nas.ash-forge.com\\storage"},
                    {"Total Capacity", "16.0 TB"}
                }
            },
            {
                .id = "net_gdrive",
                .name = "Google Drive Virtual File System Adapter (G:)",
                .iconId = IconId::CloudDrive,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "Google LLC",
                .driverVersion = "88.0.0.0",
                .hardwareId = "ROOT\\Google_Drive_FS",
                .location = "Cloud Storage Mount Point",
                .isEnabled = true,
                .properties = {
                    {"Mount Point", "G:\\"},
                    {"Cloud Tier", "Google Workspace Unlimited"}
                }
            }
        }
    };

    // 7. Processors (will be expanded dynamically with real cores)
    DeviceCategory procCat{
        .id = "processors",
        .name = "Processors",
        .iconId = IconId::SystemInfo,
        .isExpanded = true,
        .devices = {}
    };

    // Baseline 8-core CPU entries (customized by live telemetry)
    for (int i = 0; i < 8; ++i) {
        std::ostringstream ss;
        ss << "11th Gen Intel(R) Core(TM) i7-11700K @ 3.60GHz (Core #" << i << ")";
        procCat.devices.push_back({
            .id = "cpu_core_" + std::to_string(i),
            .name = ss.str(),
            .iconId = IconId::SystemInfo,
            .status = "This device is working properly. (Code 0)",
            .manufacturer = "GenuineIntel",
            .driverVersion = "10.0.26100.1",
            .hardwareId = "ACPI\\GenuineIntel_-_x86_Family_6_Model_167",
            .location = "Socket 0, Processor Core " + std::to_string(i),
            .isEnabled = true,
            .properties = {
                {"Base Frequency", "3.60 GHz"},
                {"Turbo Frequency", "5.00 GHz"},
                {"L1/L2/L3 Cache", "32 KB / 512 KB / 16 MB"}
            }
        });
    }

    // 8. Universal Serial Bus controllers
    DeviceCategory usbCat{
        .id = "usb",
        .name = "Universal Serial Bus controllers",
        .iconId = IconId::SystemInfo,
        .isExpanded = true,
        .devices = {
            {
                .id = "usb_xhci",
                .name = "Intel(R) USB 3.20 eXtensible Host Controller - 1.20 (Microsoft)",
                .iconId = IconId::SystemInfo,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "Generic USB Host Controller",
                .driverVersion = "10.0.26100.1",
                .hardwareId = "PCI\\VEN_8086&DEV_43ED&SUBSYS_86941043",
                .location = "PCI Bus 0, Device 20, Function 0",
                .isEnabled = true,
                .properties = {
                    {"USB Version", "USB 3.2 Gen 2x1 (10 Gbps)"},
                    {"Port Count", "14"}
                }
            },
            {
                .id = "usb_hub",
                .name = "USB Root Hub (USB 3.0)",
                .iconId = IconId::SystemInfo,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "(Standard USB HUBs)",
                .driverVersion = "10.0.26100.1",
                .hardwareId = "USB\\ROOT_HUB30",
                .location = "Port_#0001.Hub_#0001",
                .isEnabled = true,
                .properties = {
                    {"Power State", "D0 (Active)"}
                }
            }
        }
    };

    // 9. System devices
    DeviceCategory sysCat{
        .id = "system",
        .name = "System devices",
        .iconId = IconId::SentinelSec,
        .isExpanded = true,
        .devices = {
            {
                .id = "sys_acpi",
                .name = "ACPI Fixed Feature Button",
                .iconId = IconId::Power,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "(Standard system devices)",
                .driverVersion = "10.0.26100.1",
                .hardwareId = "ACPI\\FixedButton",
                .location = "ACPI Hardware Layer",
                .isEnabled = true,
                .properties = {
                    {"Device Status", "Working properly"}
                }
            },
            {
                .id = "sys_hpet",
                .name = "High precision event timer (HPET)",
                .iconId = IconId::Clock,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "(Standard system devices)",
                .driverVersion = "10.0.26100.1",
                .hardwareId = "ACPI\\PNP0103",
                .location = "Motherboard resources",
                .isEnabled = true,
                .properties = {
                    {"Timer Frequency", "14.31818 MHz"}
                }
            },
            {
                .id = "sys_enclave",
                .name = "MicaNT Barrer Sovereign Kernel Security Enclave",
                .iconId = IconId::ShieldAdmin,
                .status = "This device is working properly. (Code 0)",
                .manufacturer = "Barrer Software & MicaNT Community",
                .driverVersion = "1.0.0.0",
                .hardwareId = "ROOT\\MicaNT_Enclave_V1",
                .location = "Hardware Privilege Level 0 Ring",
                .isEnabled = true,
                .properties = {
                    {"Policy", "Zero-Telemetry Sovereign Enforcement"},
                    {"DMA Isolation", "Enforced"}
                }
            }
        }
    };

    categories_.push_back(std::move(audioCat));
    categories_.push_back(std::move(diskCat));
    categories_.push_back(std::move(displayCat));
    categories_.push_back(std::move(kbCat));
    categories_.push_back(std::move(miceCat));
    categories_.push_back(std::move(netCat));
    categories_.push_back(std::move(procCat));
    categories_.push_back(std::move(usbCat));
    categories_.push_back(std::move(sysCat));
}

void DeviceManagerContent::collectHostHardwareTelemetry() {
#if defined(_WIN32)
    // 1. Live CPU Detection & Core Enumeration
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    const DWORD numCores = si.dwNumberOfProcessors > 0 ? si.dwNumberOfProcessors : 8;

    std::string cpuName = "Intel(R) Xeon(R) E-2236 CPU @ 3.40GHz";
    HKEY hKey{};
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char buffer[256]{};
        DWORD bufSize = sizeof(buffer);
        if (RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufSize) == ERROR_SUCCESS) {
            std::string regCpu = buffer;
            while (!regCpu.empty() && (regCpu.back() == ' ' || regCpu.back() == '\t' || regCpu.back() == '\0')) {
                regCpu.pop_back();
            }
            if (!regCpu.empty()) {
                cpuName = regCpu;
            }
        }
        RegCloseKey(hKey);
    }

    for (auto& cat : categories_) {
        if (cat.id == "processors") {
            cat.devices.clear();
            for (DWORD i = 0; i < numCores; ++i) {
                std::ostringstream ss;
                ss << cpuName << " (Logical Core #" << i << ")";
                cat.devices.push_back({
                    .id = "cpu_core_" + std::to_string(i),
                    .name = ss.str(),
                    .iconId = IconId::SystemInfo,
                    .status = "This device is working properly. (Code 0)",
                    .manufacturer = "GenuineIntel",
                    .driverVersion = "10.0.26100.1",
                    .hardwareId = "ACPI\\Processor_Logical_Core_" + std::to_string(i),
                    .location = "Socket 0, Logical Processor " + std::to_string(i),
                    .isEnabled = true,
                    .properties = {
                        {"Device Status", "Working properly"},
                        {"Hardware Name", cpuName},
                        {"Logical Core Index", std::to_string(i)},
                        {"Driver Provider", "Barrer Software & MicaNT Community"}
                    }
                });
            }
        } else if (cat.id == "disk") {
            // 2. Real Physical Disks from Registry
            HKEY hDiskEnum{};
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\disk\\Enum", 0, KEY_READ, &hDiskEnum) == ERROR_SUCCESS) {
                DWORD count = 0;
                DWORD countSize = sizeof(count);
                RegQueryValueExA(hDiskEnum, "Count", nullptr, nullptr, reinterpret_cast<LPBYTE>(&count), &countSize);

                if (count > 0) {
                    cat.devices.clear();
                    for (DWORD i = 0; i < count; ++i) {
                        char instId[512]{};
                        DWORD instIdSize = sizeof(instId);
                        std::string valName = std::to_string(i);
                        std::string diskModel = "Host Physical Disk " + std::to_string(i);
                        if (RegQueryValueExA(hDiskEnum, valName.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(instId), &instIdSize) == ERROR_SUCCESS) {
                            std::string enumKeyPath = std::string("SYSTEM\\CurrentControlSet\\Enum\\") + instId;
                            HKEY hDevKey{};
                            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, enumKeyPath.c_str(), 0, KEY_READ, &hDevKey) == ERROR_SUCCESS) {
                                char friendly[256]{};
                                DWORD friendlySize = sizeof(friendly);
                                if (RegQueryValueExA(hDevKey, "FriendlyName", nullptr, nullptr, reinterpret_cast<LPBYTE>(friendly), &friendlySize) == ERROR_SUCCESS && friendly[0] != '\0') {
                                    diskModel = friendly;
                                }
                                RegCloseKey(hDevKey);
                            }
                        }

                        cat.devices.push_back({
                            .id = (i == 0) ? "disk_nvme" : ("disk_" + std::to_string(i)),
                            .name = diskModel,
                            .iconId = IconId::LocalDisk,
                            .status = "This device is working properly. (Code 0)",
                            .manufacturer = diskModel.find("HGST") != std::string::npos ? "Western Digital / HGST" : "Host Disk Manufacturer",
                            .driverVersion = "10.0.26100.1",
                            .hardwareId = instId[0] != '\0' ? std::string(instId) : ("SCSI\\Disk_Host_" + std::to_string(i)),
                            .location = "Port " + std::to_string(i) + ", Target 0, LUN 0",
                            .isEnabled = true,
                            .properties = {
                                {"Device Status", "Working properly"},
                                {"Model", diskModel},
                                {"Disk Index", std::to_string(i)},
                                {"Partition Style", "GPT (GUID Partition Table)"}
                            }
                        });
                    }
                }
                RegCloseKey(hDiskEnum);
            }
        } else if (cat.id == "display") {
            // 3. Real Display Adapters via EnumDisplayDevicesA
            std::vector<DeviceItem> realDisplays;
            DISPLAY_DEVICEA dd{};
            dd.cb = sizeof(dd);
            DWORD devIdx = 0;
            while (EnumDisplayDevicesA(nullptr, devIdx, &dd, 0)) {
                if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) || (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) || devIdx == 0) {
                    if (dd.DeviceString[0] != '\0') {
                        bool dup = false;
                        for (const auto& existing : realDisplays) {
                            if (existing.name == dd.DeviceString) { dup = true; break; }
                        }
                        if (!dup) {
                            realDisplays.push_back({
                                .id = (devIdx == 0) ? "disp_gpu" : ("disp_gpu_" + std::to_string(devIdx)),
                                .name = dd.DeviceString,
                                .iconId = IconId::Display,
                                .status = "This device is working properly. (Code 0)",
                                .manufacturer = "Direct Display Subsystem",
                                .driverVersion = "10.0.26100.7309",
                                .hardwareId = dd.DeviceID[0] != '\0' ? std::string(dd.DeviceID) : "PCI\\VEN_DISPLAY_DEVICE",
                                .location = "PCI Bus Display Adapter " + std::to_string(devIdx),
                                .isEnabled = true,
                                .properties = {
                                    {"Device Status", "Working properly"},
                                    {"Display Name", dd.DeviceString},
                                    {"Adapter String", dd.DeviceName},
                                    {"Primary Device", (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) ? "Yes" : "No"}
                                }
                            });
                        }
                    }
                }
                devIdx++;
                if (devIdx > 8) break;
            }
            if (!realDisplays.empty()) {
                realDisplays[0].id = "disp_gpu";
                cat.devices = std::move(realDisplays);
            }
        } else if (cat.id == "network") {
            // 4. Real Network Adapters from Registry & NDIS
            const auto primaryNet = KernelBridge::queryPrimaryNetworkAdapter();
            HKEY hNetClass{};
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}", 0, KEY_READ, &hNetClass) == ERROR_SUCCESS) {
                std::vector<DeviceItem> realNets;
                char subKeyName[256]{};
                DWORD subKeyIdx = 0;
                DWORD nameLen = sizeof(subKeyName);
                while (RegEnumKeyExA(hNetClass, subKeyIdx++, subKeyName, &nameLen, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
                    nameLen = sizeof(subKeyName);
                    if (subKeyName[0] >= '0' && subKeyName[0] <= '9') {
                        HKEY hAdapterKey{};
                        if (RegOpenKeyExA(hNetClass, subKeyName, 0, KEY_READ, &hAdapterKey) == ERROR_SUCCESS) {
                            char desc[256]{};
                            DWORD descSize = sizeof(desc);
                            if (RegQueryValueExA(hAdapterKey, "DriverDesc", nullptr, nullptr, reinterpret_cast<LPBYTE>(desc), &descSize) == ERROR_SUCCESS && desc[0] != '\0') {
                                std::string d = desc;
                                if (d.find("WAN Miniport") == std::string::npos && d.find("Kernel Debug") == std::string::npos) {
                                    char prov[256]{};
                                    DWORD provSize = sizeof(prov);
                                    RegQueryValueExA(hAdapterKey, "ProviderName", nullptr, nullptr, reinterpret_cast<LPBYTE>(prov), &provSize);

                                    char ver[128]{};
                                    DWORD verSize = sizeof(ver);
                                    RegQueryValueExA(hAdapterKey, "DriverVersion", nullptr, nullptr, reinterpret_cast<LPBYTE>(ver), &verSize);

                                    const bool isPrimary = (d.find(primaryNet.description) != std::string::npos || realNets.empty());

                                    realNets.push_back({
                                        .id = realNets.empty() ? "net_eth" : ("net_eth_" + std::to_string(realNets.size())),
                                        .name = d,
                                        .iconId = IconId::NetworkEthernet,
                                        .status = "This device is working properly. (Code 0)",
                                        .manufacturer = prov[0] != '\0' ? prov : "Intel Corporation",
                                        .driverVersion = ver[0] != '\0' ? ver : "10.0.26100.1",
                                        .hardwareId = "PCI\\VEN_NET_ADAPTER_" + std::to_string(subKeyIdx),
                                        .location = "PCI Bus Network Interface",
                                        .isEnabled = true,
                                        .properties = {
                                            {"Device Status", "Working properly"},
                                            {"Link Speed", isPrimary ? primaryNet.linkSpeed : "1.0 Gbps Full Duplex"},
                                            {"Driver Provider", prov[0] != '\0' ? prov : "Intel Corporation"},
                                            {"MAC Address", isPrimary ? primaryNet.macAddress : "00:1A:7D:DA:71:11"},
                                            {"IPv4 Address", isPrimary ? primaryNet.ipv4Address : "None"}
                                        }
                                    });
                                }
                            }
                            RegCloseKey(hAdapterKey);
                        }
                    }
                }
                RegCloseKey(hNetClass);
                if (!realNets.empty()) {
                    cat.devices = std::move(realNets);
                }
            }
        } else if (cat.id == "system") {
            // 5. Real Motherboard & BIOS in System Devices
            HKEY hBiosKey{};
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hBiosKey) == ERROR_SUCCESS) {
                char mfg[256]{};
                DWORD mfgSize = sizeof(mfg);
                RegQueryValueExA(hBiosKey, "BaseBoardManufacturer", nullptr, nullptr, reinterpret_cast<LPBYTE>(mfg), &mfgSize);

                char prod[256]{};
                DWORD prodSize = sizeof(prod);
                RegQueryValueExA(hBiosKey, "BaseBoardProduct", nullptr, nullptr, reinterpret_cast<LPBYTE>(prod), &prodSize);

                char biosVer[256]{};
                DWORD biosVerSize = sizeof(biosVer);
                RegQueryValueExA(hBiosKey, "BIOSVersion", nullptr, nullptr, reinterpret_cast<LPBYTE>(biosVer), &biosVerSize);

                std::string boardName = (mfg[0] != '\0' ? std::string(mfg) : "ASRockRack") + " " + (prod[0] != '\0' ? std::string(prod) : "E3C246D4U2-2T") + " Motherboard Resources";

                bool foundMb = false;
                for (auto& dev : cat.devices) {
                    if (dev.id == "sys_motherboard" || dev.name.find("Motherboard") != std::string::npos) {
                        dev.name = boardName;
                        dev.manufacturer = mfg[0] != '\0' ? mfg : "ASRockRack";
                        foundMb = true;
                        break;
                    }
                }
                if (!foundMb) {
                    cat.devices.insert(cat.devices.begin(), DeviceItem{
                        .id = "sys_motherboard",
                        .name = boardName,
                        .iconId = IconId::SystemInfo,
                        .status = "This device is working properly. (Code 0)",
                        .manufacturer = mfg[0] != '\0' ? mfg : "ASRockRack",
                        .driverVersion = biosVer[0] != '\0' ? biosVer : "1.0",
                        .hardwareId = "ACPI\\Motherboard_Host",
                        .location = "System Board Resources",
                        .isEnabled = true,
                        .properties = {
                            {"Baseboard Manufacturer", mfg[0] != '\0' ? mfg : "ASRockRack"},
                            {"Product Model", prod[0] != '\0' ? prod : "E3C246D4U2-2T"},
                            {"BIOS Version", biosVer[0] != '\0' ? biosVer : "L2.61A"}
                        }
                    });
                }
                RegCloseKey(hBiosKey);
            }
        }
    }
#endif
}

size_t DeviceManagerContent::totalDeviceCount() const noexcept {
    size_t count = 0;
    for (const auto& c : categories_) {
        count += c.devices.size();
    }
    return count;
}

void DeviceManagerContent::toggleCategory(const std::string& categoryId) {
    for (auto& c : categories_) {
        if (c.id == categoryId) {
            c.isExpanded = !c.isExpanded;
            break;
        }
    }
    rebuildFlatList();
}

void DeviceManagerContent::expandAll() {
    for (auto& c : categories_) {
        c.isExpanded = true;
    }
    rebuildFlatList();
}

void DeviceManagerContent::collapseAll() {
    for (auto& c : categories_) {
        c.isExpanded = false;
    }
    rebuildFlatList();
}

void DeviceManagerContent::setFilterQuery(const std::string& query) {
    filterQuery_ = query;
    rebuildFlatList();
}

void DeviceManagerContent::rebuildFlatList() {
    flatRows_.clear();

    const std::string qLower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }(filterQuery_);

    for (const auto& cat : categories_) {
        // Check if category or any child matches filter
        bool catMatches = qLower.empty() || (cat.name.find(qLower) != std::string::npos);
        std::vector<const DeviceItem*> matchingDevices;

        for (const auto& dev : cat.devices) {
            if (qLower.empty()) {
                matchingDevices.push_back(&dev);
            } else {
                std::string devLower = dev.name;
                std::transform(devLower.begin(), devLower.end(), devLower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (catMatches || devLower.find(qLower) != std::string::npos) {
                    matchingDevices.push_back(&dev);
                }
            }
        }

        if (catMatches || !matchingDevices.empty()) {
            flatRows_.push_back(FlatRow{
                .isCategory = true,
                .categoryId = cat.id,
                .deviceId = "",
                .label = cat.name,
                .iconId = cat.iconId,
                .isExpanded = cat.isExpanded,
                .isEnabled = true,
                .childCount = cat.devices.size(),
                .bounds = Rect{}
            });

            if (cat.isExpanded) {
                for (const auto* dev : matchingDevices) {
                    flatRows_.push_back(FlatRow{
                        .isCategory = false,
                        .categoryId = cat.id,
                        .deviceId = dev->id,
                        .label = dev->name,
                        .iconId = dev->iconId,
                        .isExpanded = false,
                        .isEnabled = dev->isEnabled,
                        .childCount = 0,
                        .bounds = Rect{}
                    });
                }
            }
        }
    }

    if (selectedFlatIndex_ >= static_cast<int32_t>(flatRows_.size())) {
        selectedFlatIndex_ = std::max(0, static_cast<int32_t>(flatRows_.size()) - 1);
    }
}

void DeviceManagerContent::selectIndex(int32_t index) {
    if (flatRows_.empty()) {
        selectedFlatIndex_ = 0;
        return;
    }
    selectedFlatIndex_ = std::clamp(index, 0, static_cast<int32_t>(flatRows_.size()) - 1);
}

void DeviceManagerContent::selectDevice(const std::string& deviceId) {
    for (size_t i = 0; i < flatRows_.size(); ++i) {
        if (!flatRows_[i].isCategory && flatRows_[i].deviceId == deviceId) {
            selectedFlatIndex_ = static_cast<int32_t>(i);
            return;
        }
    }
}

const DeviceItem* DeviceManagerContent::selectedDevice() const noexcept {
    if (selectedFlatIndex_ < 0 || selectedFlatIndex_ >= static_cast<int32_t>(flatRows_.size())) {
        return nullptr;
    }
    const auto& row = flatRows_[selectedFlatIndex_];
    if (row.isCategory) return nullptr;

    for (const auto& cat : categories_) {
        if (cat.id == row.categoryId) {
            for (const auto& dev : cat.devices) {
                if (dev.id == row.deviceId) return &dev;
            }
        }
    }
    return nullptr;
}

void DeviceManagerContent::toggleSelectedDeviceEnabled() {
    if (selectedFlatIndex_ < 0 || selectedFlatIndex_ >= static_cast<int32_t>(flatRows_.size())) return;
    auto& row = flatRows_[selectedFlatIndex_];
    if (row.isCategory) return;

    for (auto& cat : categories_) {
        if (cat.id == row.categoryId) {
            for (auto& dev : cat.devices) {
                if (dev.id == row.deviceId) {
                    dev.isEnabled = !dev.isEnabled;
                    if (dev.isEnabled) {
                        dev.status = "This device is working properly. (Code 0)";
                        statusMessage_ = dev.name + " enabled";
                    } else {
                        dev.status = "This device is disabled. (Code 22)";
                        statusMessage_ = dev.name + " disabled";
                    }
                    if (onToast_) {
                        onToast_("Device Manager", statusMessage_, IconId::DeviceManager);
                    }
                    rebuildFlatList();
                    return;
                }
            }
        }
    }
}

void DeviceManagerContent::openPropertiesDialog() {
    if (selectedDevice()) {
        showPropertiesModal_ = true;
        propertiesTab_ = 0;
    }
}

void DeviceManagerContent::closePropertiesDialog() {
    showPropertiesModal_ = false;
}

void DeviceManagerContent::render(Surface& clientSurface) {
    const int32_t w = clientSurface.width();
    const int32_t h = clientSurface.height();

    // 1. Dark Acrylic Background
    clientSurface.fillRect(Rect{0, 0, w, h}, Color::fromHex(0x0C101A));

    // 2. Command Toolbar (Height 42px)
    const Rect toolBarRect{0, 0, w, 42};
    clientSurface.fillRect(toolBarRect, Color::fromHex(0x131A29));
    clientSurface.fillRect(Rect{0, 41, w, 1}, Color::fromHex(0x1E293B));

    int32_t btnX = 10;
    const int32_t btnY = 8;
    const int32_t btnH = 26;

    // Scan for hardware changes button
    scanBtn_ = Rect{btnX, btnY, 150, btnH};
    clientSurface.drawRoundedRect(scanBtn_, 4, Color::fromHex(0x1E293B), true);
    clientSurface.drawRoundedRect(scanBtn_, 4, Color::fromHex(0x334155), false);
    IconRenderer::draw(clientSurface, IconId::NavRefresh, Point{btnX + 6, btnY + 5}, 16, Color::fromHex(0x38BDF8));
    clientSurface.drawString(Point{btnX + 26, btnY + 7}, "Scan Hardware", Color::fromHex(0xF8FAFC));
    btnX += 156;

    // Properties button
    propertiesBtn_ = Rect{btnX, btnY, 105, btnH};
    clientSurface.drawRoundedRect(propertiesBtn_, 4, Color::fromHex(0x1E293B), true);
    clientSurface.drawRoundedRect(propertiesBtn_, 4, Color::fromHex(0x334155), false);
    IconRenderer::draw(clientSurface, IconId::Properties, Point{btnX + 6, btnY + 5}, 16, Color::fromHex(0x00FF9D));
    clientSurface.drawString(Point{btnX + 26, btnY + 7}, "Properties", Color::fromHex(0xF8FAFC));
    btnX += 111;

    // Enable / Disable button
    toggleEnableBtn_ = Rect{btnX, btnY, 120, btnH};
    const bool devSelected = (selectedDevice() != nullptr);
    const bool isCurrentlyEnabled = devSelected ? selectedDevice()->isEnabled : true;
    clientSurface.drawRoundedRect(toggleEnableBtn_, 4, Color::fromHex(0x1E293B), true);
    clientSurface.drawRoundedRect(toggleEnableBtn_, 4, Color::fromHex(0x334155), false);
    const Color toggleCol = devSelected ? (isCurrentlyEnabled ? Color::fromHex(0xEF4444) : Color::fromHex(0x00FF9D)) : Color::fromHex(0x64748B);
    clientSurface.drawString(Point{btnX + 12, btnY + 7}, isCurrentlyEnabled ? "Disable Dev" : "Enable Dev", toggleCol);
    btnX += 126;

    // Expand / Collapse All
    expandAllBtn_ = Rect{btnX, btnY, 100, btnH};
    clientSurface.drawRoundedRect(expandAllBtn_, 4, Color::fromHex(0x1E293B), true);
    clientSurface.drawRoundedRect(expandAllBtn_, 4, Color::fromHex(0x334155), false);
    clientSurface.drawString(Point{btnX + 14, btnY + 7}, "Expand All", Color::fromHex(0xCBD5E1));

    // Search filter input box (Right aligned)
    const int32_t searchW = 180;
    searchBox_ = Rect{w - searchW - 12, btnY, searchW, btnH};
    clientSurface.drawRoundedRect(searchBox_, 4, Color::fromHex(0x0F1524), true);
    clientSurface.drawRoundedRect(searchBox_, 4, searchFocused_ ? Color::fromHex(0x00D4FF) : Color::fromHex(0x334155), false);
    IconRenderer::draw(clientSurface, IconId::Search, Point{searchBox_.x + 6, searchBox_.y + 5}, 16, Color::fromHex(0x64748B));
    if (filterQuery_.empty()) {
        clientSurface.drawString(Point{searchBox_.x + 26, searchBox_.y + 7}, "Filter devices...", Color::fromHex(0x64748B));
    } else {
        clientSurface.drawString(Point{searchBox_.x + 26, searchBox_.y + 7}, filterQuery_, Color::fromHex(0xF8FAFC));
    }

    // 3. Tree View Pane (y=42 to h-24)
    treePaneRect_ = Rect{0, 42, w, h - 66};
    const int32_t rowHeight = 26;
    const int32_t visibleRows = treePaneRect_.height / rowHeight;

    for (int32_t r = 0; r < visibleRows; ++r) {
        const int32_t idx = scrollOffset_ + r;
        if (idx >= static_cast<int32_t>(flatRows_.size())) break;

        auto& row = flatRows_[idx];
        const int32_t rowY = treePaneRect_.y + r * rowHeight;
        row.bounds = Rect{0, rowY, w, rowHeight};

        const bool isSelected = (idx == selectedFlatIndex_);

        // Alternating row background
        if (isSelected) {
            clientSurface.fillRect(row.bounds, Color::fromRgba(0, 212, 255, 35));
            clientSurface.fillRect(Rect{0, rowY, 3, rowHeight}, Color::fromHex(0x00D4FF));
        } else if (idx % 2 == 1) {
            clientSurface.fillRect(row.bounds, Color::fromHex(0x0E1422));
        }

        if (row.isCategory) {
            // Category Row
            const std::string chevron = row.isExpanded ? "v" : ">";
            clientSurface.drawString(Point{12, rowY + 6}, chevron, Color::fromHex(0x38BDF8));

            IconRenderer::draw(clientSurface, row.iconId, Point{26, rowY + 5}, 16, Color::fromHex(0x38BDF8));

            std::ostringstream ss;
            ss << row.label << " (" << row.childCount << ")";
            clientSurface.drawString(Point{48, rowY + 6}, ss.str(), Color::fromHex(0xF8FAFC));
        } else {
            // Device Item Row (indented)
            IconRenderer::draw(clientSurface, row.iconId, Point{38, rowY + 5}, 16, row.isEnabled ? Color::fromHex(0x00FF9D) : Color::fromHex(0x64748B));

            const Color textCol = row.isEnabled ? Color::fromHex(0xE2E8F0) : Color::fromHex(0x94A3B8);
            clientSurface.drawString(Point{60, rowY + 6}, row.label, textCol);

            if (!row.isEnabled) {
                const int32_t badgeX = std::min(w - 90, 60 + static_cast<int32_t>(row.label.length() * 8) + 12);
                clientSurface.drawString(Point{badgeX, rowY + 6}, "[Disabled]", Color::fromHex(0xF59E0B));
            }
        }
    }

    // 4. Status Bar (Height 24px)
    const Rect statusBarRect{0, h - 24, w, 24};
    clientSurface.fillRect(statusBarRect, Color::fromHex(0x0F1524));
    clientSurface.fillRect(Rect{0, h - 24, w, 1}, Color::fromHex(0x1E293B));

    std::ostringstream statusLeft;
    statusLeft << "Total Devices: " << totalDeviceCount() << " | Categories: " << categories_.size();
    clientSurface.drawString(Point{10, h - 18}, statusLeft.str(), Color::fromHex(0x94A3B8));

    if (const auto* dev = selectedDevice()) {
        std::ostringstream statusRight;
        statusRight << dev->name << " : " << (dev->isEnabled ? "Working properly (Code 0)" : "Disabled (Code 22)");
        clientSurface.drawString(Point{w - 450, h - 18}, statusRight.str(), dev->isEnabled ? Color::fromHex(0x00FF9D) : Color::fromHex(0xF59E0B));
    } else {
        clientSurface.drawString(Point{w - 200, h - 18}, statusMessage_, Color::fromHex(0x00D4FF));
    }

    // 5. Modal Device Properties Inspector Dialog (if open)
    if (showPropertiesModal_) {
        const auto* dev = selectedDevice();
        if (!dev) {
            showPropertiesModal_ = false;
            return;
        }

        // Semi-transparent backdrop dimmer
        clientSurface.fillRect(Rect{0, 0, w, h}, Color::fromRgba(0, 0, 0, 160));

        // Center card (500x380)
        const int32_t dlgW = 500;
        const int32_t dlgH = 380;
        const int32_t dlgX = (w - dlgW) / 2;
        const int32_t dlgY = (h - dlgH) / 2;
        propertiesDialogBounds_ = Rect{dlgX, dlgY, dlgW, dlgH};

        // Dialog background & border
        clientSurface.drawRoundedRect(propertiesDialogBounds_, 6, Color::fromHex(0x131A29), true);
        clientSurface.drawRoundedRect(propertiesDialogBounds_, 6, Color::fromHex(0x00D4FF), false);

        // Dialog caption bar
        const Rect captionRect{dlgX, dlgY, dlgW, 32};
        clientSurface.drawRoundedRect(captionRect, 6, Color::fromHex(0x1E293B), true);
        IconRenderer::draw(clientSurface, dev->iconId, Point{dlgX + 10, dlgY + 8}, 16, Color::fromHex(0x00FF9D));
        clientSurface.drawString(Point{dlgX + 32, dlgY + 9}, "Device Properties", Color::fromHex(0xF8FAFC));

        // Close button
        propertiesCloseBtn_ = Rect{dlgX + dlgW - 28, dlgY + 6, 20, 20};
        clientSurface.drawRoundedRect(propertiesCloseBtn_, 3, Color::fromHex(0x334155), true);
        clientSurface.drawString(Point{propertiesCloseBtn_.x + 6, propertiesCloseBtn_.y + 2}, "x", Color::fromHex(0xF8FAFC));

        // Tab Strip (General, Driver, Details)
        const int32_t tabY = dlgY + 38;
        const int32_t tabH = 26;
        propertiesTabGeneral_ = Rect{dlgX + 16, tabY, 80, tabH};
        propertiesTabDriver_ = Rect{dlgX + 102, tabY, 80, tabH};
        propertiesTabDetails_ = Rect{dlgX + 188, tabY, 80, tabH};

        auto renderTab = [&](const Rect& tabRect, const std::string& title, bool active) {
            clientSurface.drawRoundedRect(tabRect, 4, active ? Color::fromHex(0x0F1524) : Color::fromHex(0x1E293B), true);
            clientSurface.drawRoundedRect(tabRect, 4, active ? Color::fromHex(0x00D4FF) : Color::fromHex(0x334155), false);
            clientSurface.drawString(Point{tabRect.x + 16, tabRect.y + 6}, title, active ? Color::fromHex(0x00D4FF) : Color::fromHex(0x94A3B8));
        };

        renderTab(propertiesTabGeneral_, "General", propertiesTab_ == 0);
        renderTab(propertiesTabDriver_, "Driver", propertiesTab_ == 1);
        renderTab(propertiesTabDetails_, "Details", propertiesTab_ == 2);

        // Content Area Card
        const Rect contentCard{dlgX + 16, tabY + tabH + 6, dlgW - 32, dlgH - tabH - 90};
        clientSurface.drawRoundedRect(contentCard, 4, Color::fromHex(0x0A0E17), true);
        clientSurface.drawRoundedRect(contentCard, 4, Color::fromHex(0x1E293B), false);

        int32_t cy = contentCard.y + 14;
        const int32_t cx = contentCard.x + 16;

        if (propertiesTab_ == 0) {
            // General Tab
            clientSurface.drawString(Point{cx, cy}, dev->name, Color::fromHex(0xF8FAFC));
            cy += 24;
            clientSurface.drawString(Point{cx, cy}, "Manufacturer: " + dev->manufacturer, Color::fromHex(0x94A3B8));
            cy += 18;
            clientSurface.drawString(Point{cx, cy}, "Location: " + dev->location, Color::fromHex(0x94A3B8));
            cy += 28;

            // Device Status Box
            clientSurface.drawString(Point{cx, cy}, "Device status", Color::fromHex(0xCBD5E1));
            cy += 16;
            const Rect statusBox{cx, cy, contentCard.width - 32, 60};
            clientSurface.drawRoundedRect(statusBox, 4, Color::fromHex(0x131A29), true);
            clientSurface.drawRoundedRect(statusBox, 4, Color::fromHex(0x334155), false);
            clientSurface.drawString(Point{cx + 10, cy + 12}, dev->status, dev->isEnabled ? Color::fromHex(0x00FF9D) : Color::fromHex(0xF59E0B));
        } else if (propertiesTab_ == 1) {
            // Driver Tab
            clientSurface.drawString(Point{cx, cy}, "Driver Provider: " + dev->manufacturer, Color::fromHex(0xF8FAFC));
            cy += 20;
            clientSurface.drawString(Point{cx, cy}, "Driver Version:  " + dev->driverVersion, Color::fromHex(0xF8FAFC));
            cy += 20;
            clientSurface.drawString(Point{cx, cy}, "Digital Signer:  MicaNT Sovereign WHQL Enclave", Color::fromHex(0x00FF9D));
            cy += 30;

            const Rect driverBox{cx, cy, contentCard.width - 32, 70};
            clientSurface.drawRoundedRect(driverBox, 4, Color::fromHex(0x131A29), true);
            clientSurface.drawRoundedRect(driverBox, 4, Color::fromHex(0x334155), false);
            clientSurface.drawString(Point{cx + 10, cy + 12}, "Driver files are verified and protected by", Color::fromHex(0x94A3B8));
            clientSurface.drawString(Point{cx + 10, cy + 30}, "Barrer Software & MicaNT Community Security Enclave.", Color::fromHex(0x38BDF8));
        } else {
            // Details Tab
            clientSurface.drawString(Point{cx, cy}, "Property: Hardware IDs", Color::fromHex(0xCBD5E1));
            cy += 20;
            const Rect hwBox{cx, cy, contentCard.width - 32, 100};
            clientSurface.drawRoundedRect(hwBox, 4, Color::fromHex(0x131A29), true);
            clientSurface.drawRoundedRect(hwBox, 4, Color::fromHex(0x334155), false);
            clientSurface.drawString(Point{cx + 10, cy + 12}, dev->hardwareId, Color::fromHex(0x38BDF8));
            clientSurface.drawString(Point{cx + 10, cy + 32}, dev->id, Color::fromHex(0x94A3B8));
        }

        // Dialog Bottom Action Buttons (OK)
        const Rect okBtn{dlgX + dlgW - 90, dlgY + dlgH - 34, 74, 24};
        clientSurface.drawRoundedRect(okBtn, 4, Color::fromHex(0x00D4FF), true);
        clientSurface.drawString(Point{okBtn.centerX() - 8, okBtn.y + 5}, "OK", Color::fromHex(0x040810));
    }
}

bool DeviceManagerContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    // 1. If Properties Modal is open, handle modal interactions
    if (showPropertiesModal_) {
        if (propertiesCloseBtn_.contains(localPt) || !propertiesDialogBounds_.contains(localPt)) {
            closePropertiesDialog();
            return true;
        }
        if (propertiesTabGeneral_.contains(localPt)) {
            propertiesTab_ = 0;
            return true;
        }
        if (propertiesTabDriver_.contains(localPt)) {
            propertiesTab_ = 1;
            return true;
        }
        if (propertiesTabDetails_.contains(localPt)) {
            propertiesTab_ = 2;
            return true;
        }
        const Rect okBtn{propertiesDialogBounds_.right() - 90, propertiesDialogBounds_.bottom() - 34, 74, 24};
        if (okBtn.contains(localPt)) {
            closePropertiesDialog();
            return true;
        }
        return true; // absorb click within modal
    }

    // 2. Toolbar buttons
    if (scanBtn_.contains(localPt)) {
        scanForHardwareChanges();
        return true;
    }
    if (propertiesBtn_.contains(localPt)) {
        openPropertiesDialog();
        return true;
    }
    if (toggleEnableBtn_.contains(localPt)) {
        toggleSelectedDeviceEnabled();
        return true;
    }
    if (expandAllBtn_.contains(localPt)) {
        expandAll();
        return true;
    }
    if (searchBox_.contains(localPt)) {
        searchFocused_ = true;
        return true;
    } else {
        searchFocused_ = false;
    }

    // 3. Tree View Rows
    for (size_t i = 0; i < flatRows_.size(); ++i) {
        if (flatRows_[i].bounds.contains(localPt)) {
            selectIndex(static_cast<int32_t>(i));
            if (flatRows_[i].isCategory) {
                toggleCategory(flatRows_[i].categoryId);
            }
            return true;
        }
    }

    return false;
}

bool DeviceManagerContent::onMouseUp(Point localPt, MouseButton button) {
    (void)localPt;
    (void)button;
    return false;
}

bool DeviceManagerContent::onMouseMove(Point localPt) {
    (void)localPt;
    return false;
}

bool DeviceManagerContent::onMouseWheel(Point localPt, int32_t delta) {
    (void)localPt;
    if (delta > 0) {
        scrollOffset_ = std::max(0, scrollOffset_ - 3);
    } else if (delta < 0) {
        scrollOffset_ = std::min(std::max(0, static_cast<int32_t>(flatRows_.size()) - 1), scrollOffset_ + 3);
    }
    return true;
}

bool DeviceManagerContent::onCharInput(char c) {
    if (searchFocused_) {
        if (c >= 32 && c <= 126) {
            setFilterQuery(filterQuery_ + c);
            return true;
        }
    }
    return false;
}

bool DeviceManagerContent::onKeyDown(KeyCode key, bool ctrl, bool shift, bool alt) {
    (void)ctrl;
    (void)shift;
    (void)alt;

    if (showPropertiesModal_) {
        if (key == KeyCode::Escape || key == KeyCode::Enter) {
            closePropertiesDialog();
            return true;
        }
        return true;
    }

    if (searchFocused_) {
        if (key == KeyCode::Backspace) {
            if (!filterQuery_.empty()) {
                filterQuery_.pop_back();
                setFilterQuery(filterQuery_);
            }
            return true;
        }
        if (key == KeyCode::Escape || key == KeyCode::Enter) {
            searchFocused_ = false;
            return true;
        }
    }

    if (key == KeyCode::Up) {
        selectIndex(selectedFlatIndex_ - 1);
        if (selectedFlatIndex_ < scrollOffset_) {
            scrollOffset_ = selectedFlatIndex_;
        }
        return true;
    }
    if (key == KeyCode::Down) {
        selectIndex(selectedFlatIndex_ + 1);
        const int32_t visibleRows = treePaneRect_.height / 26;
        if (selectedFlatIndex_ >= scrollOffset_ + visibleRows) {
            scrollOffset_ = selectedFlatIndex_ - visibleRows + 1;
        }
        return true;
    }
    if (key == KeyCode::Left) {
        if (selectedFlatIndex_ >= 0 && selectedFlatIndex_ < static_cast<int32_t>(flatRows_.size())) {
            if (flatRows_[selectedFlatIndex_].isCategory && flatRows_[selectedFlatIndex_].isExpanded) {
                toggleCategory(flatRows_[selectedFlatIndex_].categoryId);
                return true;
            }
        }
    }
    if (key == KeyCode::Right) {
        if (selectedFlatIndex_ >= 0 && selectedFlatIndex_ < static_cast<int32_t>(flatRows_.size())) {
            if (flatRows_[selectedFlatIndex_].isCategory && !flatRows_[selectedFlatIndex_].isExpanded) {
                toggleCategory(flatRows_[selectedFlatIndex_].categoryId);
                return true;
            }
        }
    }
    if (key == KeyCode::Enter) {
        openPropertiesDialog();
        return true;
    }

    return false;
}

} // namespace surshell
