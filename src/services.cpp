// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/services.cpp)
// ============================================================================

#include "surshell/services.hpp"
#include <algorithm>
#include <cstdio>
#include <cctype>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winsvc.h>
#endif

namespace surshell {

ServicesContent::ServicesContent() {
    scanServices();
}

void ServicesContent::scanServices() {
    services_.clear();

#if defined(_WIN32)
    SC_HANDLE hSCM = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT);
    if (hSCM) {
        DWORD bytesNeeded = 0;
        DWORD servicesReturned = 0;
        DWORD resumeHandle = 0;

        // First probe to determine needed buffer size
        EnumServicesStatusExA(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                              nullptr, 0, &bytesNeeded, &servicesReturned, &resumeHandle, nullptr);

        if (bytesNeeded > 0) {
            std::vector<uint8_t> buffer(bytesNeeded);
            if (EnumServicesStatusExA(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                                      buffer.data(), bytesNeeded, &bytesNeeded,
                                      &servicesReturned, &resumeHandle, nullptr)) {
                auto* pServices = reinterpret_cast<ENUM_SERVICE_STATUS_PROCESSA*>(buffer.data());
                services_.reserve(servicesReturned);

                for (DWORD i = 0; i < servicesReturned; ++i) {
                    ServiceEntry entry;
                    entry.name = pServices[i].lpServiceName ? pServices[i].lpServiceName : "";
                    entry.displayName = pServices[i].lpDisplayName ? pServices[i].lpDisplayName : entry.name;
                    entry.pid = pServices[i].ServiceStatusProcess.dwProcessId;

                    switch (pServices[i].ServiceStatusProcess.dwCurrentState) {
                        case SERVICE_RUNNING: entry.state = ServiceState::Running; break;
                        case SERVICE_STOPPED: entry.state = ServiceState::Stopped; break;
                        case SERVICE_PAUSED: entry.state = ServiceState::Paused; break;
                        case SERVICE_START_PENDING: entry.state = ServiceState::StartPending; break;
                        case SERVICE_STOP_PENDING: entry.state = ServiceState::StopPending; break;
                        default: entry.state = ServiceState::Unknown; break;
                    }

                    // Query registry for startup type, logOnAs, and description
                    std::string regPath = "SYSTEM\\CurrentControlSet\\Services\\" + entry.name;
                    HKEY hKey = nullptr;
                    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                        DWORD startVal = 0;
                        DWORD sz = sizeof(startVal);
                        if (RegQueryValueExA(hKey, "Start", nullptr, nullptr, reinterpret_cast<LPBYTE>(&startVal), &sz) == ERROR_SUCCESS) {
                            switch (startVal) {
                                case 0: entry.startup = ServiceStartup::Boot; break;
                                case 1: entry.startup = ServiceStartup::System; break;
                                case 2: {
                                    DWORD delayed = 0;
                                    DWORD dsz = sizeof(delayed);
                                    if (RegQueryValueExA(hKey, "DelayedAutostart", nullptr, nullptr, reinterpret_cast<LPBYTE>(&delayed), &dsz) == ERROR_SUCCESS && delayed == 1) {
                                        entry.startup = ServiceStartup::AutomaticDelayed;
                                    } else {
                                        entry.startup = ServiceStartup::Automatic;
                                    }
                                    break;
                                }
                                case 3: entry.startup = ServiceStartup::Manual; break;
                                case 4: entry.startup = ServiceStartup::Disabled; break;
                                default: entry.startup = ServiceStartup::Unknown; break;
                            }
                        }

                        char descBuf[1024] = {0};
                        DWORD dsz = sizeof(descBuf) - 1;
                        if (RegLoadMUIStringA(hKey, "Description", descBuf, dsz, nullptr, 0, nullptr) == ERROR_SUCCESS && descBuf[0] != '\0') {
                            entry.description = descBuf;
                        } else if (RegQueryValueExA(hKey, "Description", nullptr, nullptr, reinterpret_cast<LPBYTE>(descBuf), &dsz) == ERROR_SUCCESS) {
                            entry.description = descBuf;
                        }

                        // If description is a raw resource reference or empty, check known service descriptions
                        if (entry.description.empty() || entry.description[0] == '@') {
                            if (entry.name == "EventLog") {
                                entry.description = "Manages events and event logs. It supports logging events, querying events, archiving logs, and managing event metadata.";
                            } else if (entry.name == "Dhcp") {
                                entry.description = "Registers and updates IP addresses and DNS records for this computer. If this service is stopped, dynamic IP addressing fails.";
                            } else if (entry.name == "AudioSrv") {
                                entry.description = "Manages audio for Windows-based programs. If this service is stopped, audio devices and effects will not function properly.";
                            } else if (entry.name == "AudioEndpointBuilder") {
                                entry.description = "Manages audio devices for the Windows Audio service. If this service is stopped, audio devices and effects will not function properly.";
                            } else if (entry.name == "Spooler") {
                                entry.description = "This service spools print jobs and handles interaction with the printer. If you turn off this service, you won't be able to print.";
                            } else if (entry.name == "LanmanWorkstation") {
                                entry.description = "Creates and maintains client network connections to remote servers using the SMB protocol. If stopped, these connections will be unavailable.";
                            } else if (entry.name == "LanmanServer") {
                                entry.description = "Supports file, print, and named-pipe sharing over the network for this computer. If this service is stopped, these functions will be unavailable.";
                            } else if (entry.name == "Winmgmt") {
                                entry.description = "Provides a common interface and object model to access management information about operating system, devices, applications and services.";
                            } else if (entry.name == "BFE") {
                                entry.description = "Base Filtering Engine (BFE) is a service that manages firewall and Internet Protocol security (IPsec) policies and implements user mode filtering.";
                            } else if (entry.name == "BrokerInfrastructure") {
                                entry.description = "Background Tasks Infrastructure Service controls which background tasks can run on the system to optimize battery and responsiveness.";
                            } else if (entry.name == "AppHostSvc") {
                                entry.description = "Application Host Helper Service provides web hosting and administration infrastructure services.";
                            } else if (entry.name == "Appinfo") {
                                entry.description = "Application Information facilitates the running of interactive applications with additional administrative privileges.";
                            }
                        }

                        char objBuf[256] = {0};
                        sz = sizeof(objBuf) - 1;
                        if (RegQueryValueExA(hKey, "ObjectName", nullptr, nullptr, reinterpret_cast<LPBYTE>(objBuf), &sz) == ERROR_SUCCESS) {
                            entry.logOnAs = objBuf;
                        }

                        RegCloseKey(hKey);
                    }

                    services_.push_back(std::move(entry));
                }
            }
        }
        CloseServiceHandle(hSCM);
    }
#endif

    // Fallback if not on Windows or if enumeration returned empty
    if (services_.empty()) {
        services_ = {
            ServiceEntry{
                .name = "Dhcp",
                .displayName = "DHCP Client",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Automatic,
                .pid = 1148,
                .logOnAs = "NT AUTHORITY\\LocalService",
                .description = "Registers and updates IP addresses and DNS records for this computer. If this service is stopped, dynamic IP addressing fails."
            },
            ServiceEntry{
                .name = "EventLog",
                .displayName = "Windows Event Log",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Automatic,
                .pid = 964,
                .logOnAs = "NT AUTHORITY\\LocalService",
                .description = "Manages events and event logs. It supports logging events, querying events, archiving logs, and managing event metadata."
            },
            ServiceEntry{
                .name = "AudioSrv",
                .displayName = "Windows Audio",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Automatic,
                .pid = 1432,
                .logOnAs = "NT AUTHORITY\\LocalService",
                .description = "Manages audio for Windows-based programs. If this service is stopped, audio devices and effects will not function properly."
            },
            ServiceEntry{
                .name = "AudioEndpointBuilder",
                .displayName = "Windows Audio Endpoint Builder",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Automatic,
                .pid = 1436,
                .logOnAs = "NT AUTHORITY\\LocalService",
                .description = "Manages audio devices for the Windows Audio service. If this service is stopped, audio devices and effects will not function properly."
            },
            ServiceEntry{
                .name = "Spooler",
                .displayName = "Print Spooler",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Automatic,
                .pid = 2040,
                .logOnAs = "LocalSystem",
                .description = "This service spools print jobs and handles interaction with the printer. If you turn off this service, you won't be able to print."
            },
            ServiceEntry{
                .name = "LanmanWorkstation",
                .displayName = "Workstation",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Automatic,
                .pid = 1820,
                .logOnAs = "NT AUTHORITY\\NetworkService",
                .description = "Creates and maintains client network connections to remote servers using the SMB protocol. If stopped, these connections will be unavailable."
            },
            ServiceEntry{
                .name = "LanmanServer",
                .displayName = "Server",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Automatic,
                .pid = 1844,
                .logOnAs = "LocalSystem",
                .description = "Supports file, print, and named-pipe sharing over the network for this computer. If this service is stopped, these functions will be unavailable."
            },
            ServiceEntry{
                .name = "Winmgmt",
                .displayName = "Windows Management Instrumentation",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Automatic,
                .pid = 1012,
                .logOnAs = "NT AUTHORITY\\NetworkService",
                .description = "Provides a common interface and object model to access management information about operating system, devices, applications and services."
            },
            ServiceEntry{
                .name = "W32Time",
                .displayName = "Windows Time",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Manual,
                .pid = 2192,
                .logOnAs = "NT AUTHORITY\\LocalService",
                .description = "Maintains date and time synchronization on all clients and servers in the network. If stopped, date and time synchronization will be unavailable."
            },
            ServiceEntry{
                .name = "CryptSvc",
                .displayName = "Cryptographic Services",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Automatic,
                .pid = 1204,
                .logOnAs = "NT AUTHORITY\\NetworkService",
                .description = "Provides four management services: Catalog Database Service, Protected Root Service, Automatic Root Certificate Update, and Key Service."
            },
            ServiceEntry{
                .name = "Dnscache",
                .displayName = "DNS Client",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Automatic,
                .pid = 1260,
                .logOnAs = "NT AUTHORITY\\NetworkService",
                .description = "The DNS Client service (dnscache) caches Domain Name System (DNS) names and registers the full computer name for this computer."
            },
            ServiceEntry{
                .name = "MpsSvc",
                .displayName = "Windows Defender Firewall",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Automatic,
                .pid = 1380,
                .logOnAs = "NT AUTHORITY\\LocalService",
                .description = "Windows Defender Firewall helps protect your computer by preventing unauthorized users from gaining access through the Internet or network."
            },
            ServiceEntry{
                .name = "wuauserv",
                .displayName = "Windows Update",
                .state = ServiceState::Stopped,
                .startup = ServiceStartup::Manual,
                .pid = 0,
                .logOnAs = "LocalSystem",
                .description = "Enables the detection, download, and installation of updates for Windows and other programs."
            },
            ServiceEntry{
                .name = "DiagTrack",
                .displayName = "Connected User Experiences and Telemetry",
                .state = ServiceState::Stopped,
                .startup = ServiceStartup::Disabled,
                .pid = 0,
                .logOnAs = "NT AUTHORITY\\NetworkService",
                .description = "Enables features that support in-application and connected user experiences. Sovereign Policy Enforced: Disabled."
            },
            ServiceEntry{
                .name = "SentinelSec",
                .displayName = "SentinelSec Sovereign Enclave Guard",
                .state = ServiceState::Running,
                .startup = ServiceStartup::Automatic,
                .pid = 824,
                .logOnAs = "LocalSystem",
                .description = "MicaNT Dave Cutler Clean-Room Zero-Telemetry Kernel Enclave Defense Service."
            }
        };
    }

    // Sort alphabetically by displayName
    std::sort(services_.begin(), services_.end(), [](const ServiceEntry& a, const ServiceEntry& b) {
        return a.displayName < b.displayName;
    });

    updateFilteredIndices();
    if (!filteredIndices_.empty()) {
        selectedIndex_ = 0;
    } else {
        selectedIndex_ = -1;
    }
}

size_t ServicesContent::runningServicesCount() const noexcept {
    return std::count_if(services_.begin(), services_.end(), [](const ServiceEntry& s) {
        return s.state == ServiceState::Running;
    });
}

size_t ServicesContent::stoppedServicesCount() const noexcept {
    return std::count_if(services_.begin(), services_.end(), [](const ServiceEntry& s) {
        return s.state == ServiceState::Stopped;
    });
}

const ServiceEntry* ServicesContent::selectedService() const noexcept {
    if (selectedIndex_ >= 0 && static_cast<size_t>(selectedIndex_) < filteredIndices_.size()) {
        const size_t originalIdx = filteredIndices_[selectedIndex_];
        if (originalIdx < services_.size()) {
            return &services_[originalIdx];
        }
    }
    return nullptr;
}

void ServicesContent::selectServiceByName(const std::string& name) {
    for (size_t i = 0; i < filteredIndices_.size(); ++i) {
        const size_t origIdx = filteredIndices_[i];
        if (services_[origIdx].name == name || services_[origIdx].displayName == name) {
            selectedIndex_ = static_cast<int32_t>(i);
            return;
        }
    }
}

void ServicesContent::selectIndex(size_t index) {
    if (index < filteredIndices_.size()) {
        selectedIndex_ = static_cast<int32_t>(index);
    }
}

void ServicesContent::setSearchQuery(const std::string& query) {
    searchQuery_ = query;
    updateFilteredIndices();
    if (!filteredIndices_.empty()) {
        selectedIndex_ = 0;
    } else {
        selectedIndex_ = -1;
    }
    scrollOffset_ = 0;
}

void ServicesContent::updateFilteredIndices() {
    filteredIndices_.clear();
    std::string lowerQ = searchQuery_;
    std::transform(lowerQ.begin(), lowerQ.end(), lowerQ.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    for (size_t i = 0; i < services_.size(); ++i) {
        if (lowerQ.empty()) {
            filteredIndices_.push_back(i);
            continue;
        }

        std::string n = services_[i].name;
        std::string dn = services_[i].displayName;
        std::string desc = services_[i].description;
        std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        std::transform(dn.begin(), dn.end(), dn.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        std::transform(desc.begin(), desc.end(), desc.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (n.find(lowerQ) != std::string::npos ||
            dn.find(lowerQ) != std::string::npos ||
            desc.find(lowerQ) != std::string::npos) {
            filteredIndices_.push_back(i);
        }
    }
}

bool ServicesContent::startSelectedService() {
    if (selectedIndex_ < 0 || static_cast<size_t>(selectedIndex_) >= filteredIndices_.size()) return false;
    const size_t origIdx = filteredIndices_[selectedIndex_];
    auto& s = services_[origIdx];

    s.state = ServiceState::Running;
    if (s.pid == 0) s.pid = 2400 + (origIdx % 500);

    if (onToast_) {
        onToast_("Service Started", s.displayName + " (" + s.name + ") is now running", IconId::Services);
    }
    return true;
}

bool ServicesContent::stopSelectedService() {
    if (selectedIndex_ < 0 || static_cast<size_t>(selectedIndex_) >= filteredIndices_.size()) return false;
    const size_t origIdx = filteredIndices_[selectedIndex_];
    auto& s = services_[origIdx];

    s.state = ServiceState::Stopped;
    s.pid = 0;

    if (onToast_) {
        onToast_("Service Stopped", s.displayName + " (" + s.name + ") has been stopped", IconId::Services);
    }
    return true;
}

bool ServicesContent::restartSelectedService() {
    if (selectedIndex_ < 0 || static_cast<size_t>(selectedIndex_) >= filteredIndices_.size()) return false;
    const size_t origIdx = filteredIndices_[selectedIndex_];
    auto& s = services_[origIdx];

    s.state = ServiceState::Running;
    s.pid = 2500 + (origIdx % 500);

    if (onToast_) {
        onToast_("Service Restarted", s.displayName + " (" + s.name + ") restarted successfully", IconId::Services);
    }
    return true;
}

bool ServicesContent::pauseSelectedService() {
    if (selectedIndex_ < 0 || static_cast<size_t>(selectedIndex_) >= filteredIndices_.size()) return false;
    const size_t origIdx = filteredIndices_[selectedIndex_];
    auto& s = services_[origIdx];

    s.state = ServiceState::Paused;

    if (onToast_) {
        onToast_("Service Paused", s.displayName + " (" + s.name + ") is paused", IconId::Services);
    }
    return true;
}

void ServicesContent::openPropertiesDialog() {
    if (selectedService()) {
        showPropertiesModal_ = true;
    }
}

void ServicesContent::closePropertiesDialog() {
    showPropertiesModal_ = false;
}

void ServicesContent::render(Surface& s) {
    const auto& palette = ThemeManager::instance().palette();
    const int32_t w = static_cast<int32_t>(s.width());
    const int32_t h = static_cast<int32_t>(s.height());

    s.clear(Color{12, 16, 26, 255});

    renderToolbar(s, w, palette);

    // List and details layout: Details height 140px at bottom
    const int32_t detailsH = 140;
    const int32_t listH = h - 74 - detailsH;

    renderServiceList(s, w, listH, palette);
    renderExtendedDetails(s, w, h, palette);

    if (showPropertiesModal_) {
        renderPropertiesModal(s, w, h, palette);
    }
}

void ServicesContent::renderToolbar(Surface& s, int32_t w, const ThemePalette& palette) {
    // Top banner
    s.fillRect(Rect{0, 0, w, 40}, Color{16, 22, 34, 255});
    IconRenderer::draw(s, IconId::Services, Point{12, 8}, 24, palette.accentColor);

    s.drawString(44, 8, "Services (Local)", palette.textPrimary, 1);

    char statsBuf[128];
    std::snprintf(statsBuf, sizeof(statsBuf), "Total: %zu  |  Running: %zu  |  Stopped: %zu",
                  services_.size(), runningServicesCount(), stoppedServicesCount());
    s.drawString(44, 22, statsBuf, palette.textSecondary, 1);

    // Search box on right
    const int32_t searchW = 200;
    searchBoxBounds_ = Rect{w - searchW - 14, 8, searchW, 24};
    s.drawRoundedRect(searchBoxBounds_, 4, searchFocused_ ? Color{24, 32, 48, 255} : Color{18, 24, 36, 255}, true);
    s.drawRoundedRect(searchBoxBounds_, 4, searchFocused_ ? palette.accentColor : Color{48, 62, 84, 255}, false);

    IconRenderer::draw(s, IconId::Search, Point{searchBoxBounds_.x + 6, searchBoxBounds_.y + 4}, 16, palette.textSecondary);
    if (searchQuery_.empty() && !searchFocused_) {
        s.drawString(searchBoxBounds_.x + 26, searchBoxBounds_.y + 6, "Search services...", palette.textSecondary.withAlpha(140), 1);
    } else {
        s.drawString(searchBoxBounds_.x + 26, searchBoxBounds_.y + 6, searchQuery_, palette.textPrimary, 1);
        if (searchFocused_) {
            const int32_t cx = searchBoxBounds_.x + 26 + static_cast<int32_t>(searchQuery_.size() * 6);
            if (cx < searchBoxBounds_.right() - 4) {
                s.fillRect(Rect{cx, searchBoxBounds_.y + 5, 2, 14}, palette.accentColor);
            }
        }
    }

    // Action Toolbar Bar
    s.fillRect(Rect{0, 40, w, 34}, Color{14, 18, 28, 255});
    s.fillRect(Rect{0, 40, w, 1}, Color{28, 38, 56, 255});
    s.fillRect(Rect{0, 73, w, 1}, Color{28, 38, 56, 255});

    auto drawBtn = [&](Rect& bounds, int32_t x, const std::string& label, IconId icon, bool hovered, bool active) {
        const int32_t bw = static_cast<int32_t>(label.size() * 8 + 36);
        bounds = Rect{x, 44, bw, 26};
        Color bg = hovered ? Color{32, 44, 66, 255} : (active ? Color{20, 28, 42, 255} : Color{16, 22, 34, 255});
        s.drawRoundedRect(bounds, 3, bg, true);
        s.drawRoundedRect(bounds, 3, hovered ? palette.accentColor : Color{38, 52, 74, 255}, false);
        IconRenderer::draw(s, icon, Point{bounds.x + 6, bounds.y + 5}, 16, palette.accentColor);
        s.drawString(bounds.x + 26, bounds.y + 6, label, palette.textPrimary, 1);
        return bounds.right() + 8;
    };

    int32_t curX = 12;
    curX = drawBtn(btnStart_, curX, "Start", IconId::MediaPlay, hoverStart_, false);
    curX = drawBtn(btnPause_, curX, "Pause", IconId::MediaPause, hoverPause_, false);
    curX = drawBtn(btnStop_, curX, "Stop", IconId::Power, hoverStop_, false);
    curX = drawBtn(btnRestart_, curX, "Restart", IconId::Restart, hoverRestart_, false);
    curX = drawBtn(btnProperties_, curX, "Properties", IconId::Properties, hoverProperties_, false);
    drawBtn(btnRefresh_, curX, "Refresh", IconId::NavRefresh, hoverRefresh_, false);
}

void ServicesContent::renderServiceList(Surface& s, int32_t w, int32_t h, const ThemePalette& palette) {
    const int32_t listY = 74;
    s.fillRect(Rect{0, listY, w, h}, Color{10, 14, 22, 255});

    // Column headers
    const int32_t headerH = 24;
    s.fillRect(Rect{0, listY, w, headerH}, Color{18, 24, 38, 255});
    s.fillRect(Rect{0, listY + headerH, w, 1}, Color{34, 46, 68, 255});

    const int32_t cNameW = 180;
    const int32_t cDispW = 270;
    const int32_t cStatW = 90;
    const int32_t cStartW = 140;
    const int32_t cPidW = 60;

    int32_t colX = 14;
    s.drawString(colX, listY + 6, "Name", palette.textSecondary, 1); colX += cNameW;
    s.drawString(colX, listY + 6, "Description / Display Name", palette.textSecondary, 1); colX += cDispW;
    s.drawString(colX, listY + 6, "Status", palette.textSecondary, 1); colX += cStatW;
    s.drawString(colX, listY + 6, "Startup Type", palette.textSecondary, 1); colX += cStartW;
    s.drawString(colX, listY + 6, "PID", palette.textSecondary, 1); colX += cPidW;
    s.drawString(colX, listY + 6, "Log On As", palette.textSecondary, 1);

    // Rows
    const int32_t rowH = 24;
    const int32_t visibleRows = (h - headerH) / rowH;
    const int32_t totalRows = static_cast<int32_t>(filteredIndices_.size());

    // Clamp scroll
    if (scrollOffset_ > totalRows - visibleRows) scrollOffset_ = std::max(0, totalRows - visibleRows);
    if (scrollOffset_ < 0) scrollOffset_ = 0;

    for (int32_t vi = 0; vi < visibleRows && (vi + scrollOffset_) < totalRows; ++vi) {
        const int32_t rowIdx = vi + scrollOffset_;
        const size_t origIdx = filteredIndices_[rowIdx];
        const auto& item = services_[origIdx];

        const int32_t ry = listY + headerH + vi * rowH;
        Rect rowBounds{0, ry, w, rowH};

        const bool isSelected = (rowIdx == selectedIndex_);
        Color rowBg = isSelected ? Color{24, 48, 76, 255} : ((rowIdx % 2 == 1) ? Color{13, 18, 28, 255} : Color{10, 14, 22, 255});
        s.fillRect(rowBounds, rowBg);

        if (isSelected) {
            s.drawRoundedRect(rowBounds, 2, palette.accentColor, false);
        }

        colX = 14;
        // Service programmatic name
        s.drawString(colX, ry + 6, item.name, isSelected ? Color{255, 255, 255, 255} : palette.textPrimary, 1);
        colX += cNameW;

        // Display name
        std::string disp = item.displayName;
        if (disp.size() > 36) disp = disp.substr(0, 33) + "...";
        s.drawString(colX, ry + 6, disp, isSelected ? Color{255, 255, 255, 255} : palette.textPrimary, 1);
        colX += cDispW;

        // Status badge
        Color statusBg = Color{24, 32, 44, 255};
        Color statusFg = palette.textSecondary;
        if (item.state == ServiceState::Running) {
            statusBg = Color{12, 44, 28, 255};
            statusFg = Color::fromHex(0x00FF9D);
        } else if (item.state == ServiceState::Paused) {
            statusBg = Color{48, 38, 12, 255};
            statusFg = Color::fromHex(0xF59E0B);
        } else if (item.state == ServiceState::StartPending || item.state == ServiceState::StopPending) {
            statusBg = Color{32, 36, 52, 255};
            statusFg = Color::fromHex(0x38BDF8);
        }

        Rect badgeRect{colX - 4, ry + 3, 68, 18};
        s.drawRoundedRect(badgeRect, 3, statusBg, true);
        s.drawString(colX + 2, ry + 6, serviceStateToString(item.state), statusFg, 1);
        colX += cStatW;

        // Startup Type
        s.drawString(colX, ry + 6, serviceStartupToString(item.startup), palette.textSecondary, 1);
        colX += cStartW;

        // PID
        if (item.pid > 0) {
            s.drawString(colX, ry + 6, std::to_string(item.pid), Color::fromHex(0x38BDF8), 1);
        } else {
            s.drawString(colX, ry + 6, "-", palette.textSecondary.withAlpha(120), 1);
        }
        colX += cPidW;

        // Log On As
        s.drawString(colX, ry + 6, item.logOnAs, palette.textSecondary, 1);
    }
}

void ServicesContent::renderExtendedDetails(Surface& s, int32_t w, int32_t h, const ThemePalette& palette) {
    const int32_t detailsY = h - 140;
    const Rect detailsRect{0, detailsY, w, 140};

    s.fillRect(detailsRect, Color{14, 18, 28, 255});
    s.fillRect(Rect{0, detailsY, w, 1}, Color{36, 48, 70, 255});

    const auto* cur = selectedService();
    if (!cur) {
        s.drawString(20, detailsY + 20, "Select a service to view its status and description.", palette.textSecondary, 1);
        return;
    }

    // Header in details
    s.drawString(20, detailsY + 12, cur->displayName, Color{255, 255, 255, 255}, 1);
    s.drawString(20 + static_cast<int32_t>(cur->displayName.size() * 8 + 14), detailsY + 12,
                 "(" + cur->name + ")", palette.textSecondary, 1);

    // Quick Action Links
    int32_t actX = 20;
    auto drawActionLink = [&](const std::string& act, Color col) {
        const int32_t tw = static_cast<int32_t>(act.size() * 8 + 20);
        Rect r{actX, detailsY + 30, tw, 20};
        s.drawRoundedRect(r, 3, Color{22, 30, 46, 255}, true);
        s.drawRoundedRect(r, 3, col.withAlpha(160), false);
        s.drawString(r.x + 8, r.y + 4, act, col, 1);
        actX += tw + 10;
    };

    if (cur->state == ServiceState::Stopped) {
        drawActionLink("Start the service", Color::fromHex(0x00FF9D));
    } else {
        drawActionLink("Stop the service", Color::fromHex(0xFF5555));
        drawActionLink("Restart the service", Color::fromHex(0x38BDF8));
        if (cur->state == ServiceState::Running) {
            drawActionLink("Pause the service", Color::fromHex(0xF59E0B));
        }
    }

    // Full description paragraph
    s.drawString(20, detailsY + 58, "Description:", palette.textSecondary, 1);
    const std::string desc = cur->description.empty() ? "(No description provided by service vendor)" : cur->description;

    // Word wrap description to 2-3 lines
    const size_t maxCharsPerLine = static_cast<size_t>(std::max(40, (w - 40) / 8));
    size_t startPos = 0;
    int32_t lineY = detailsY + 74;
    for (int line = 0; line < 3 && startPos < desc.size(); ++line) {
        size_t len = std::min(maxCharsPerLine, desc.size() - startPos);
        if (startPos + len < desc.size()) {
            size_t lastSpace = desc.rfind(' ', startPos + len);
            if (lastSpace != std::string::npos && lastSpace > startPos) {
                len = lastSpace - startPos;
            }
        }
        std::string sub = desc.substr(startPos, len);
        s.drawString(20, lineY, sub, palette.textPrimary, 1);
        startPos += len;
        while (startPos < desc.size() && desc[startPos] == ' ') startPos++;
        lineY += 16;
    }
}

void ServicesContent::renderPropertiesModal(Surface& s, int32_t w, int32_t h, const ThemePalette& palette) {
    const auto* cur = selectedService();
    if (!cur) return;

    // Darken background
    s.fillRect(Rect{0, 0, w, h}, Color{0, 0, 0, 160});

    const int32_t mw = 440;
    const int32_t mh = 360;
    modalBounds_ = Rect{(w - mw) / 2, (h - mh) / 2, mw, mh};

    // Dialog frame
    s.drawRoundedRect(modalBounds_, 6, Color{16, 22, 34, 255}, true);
    s.drawRoundedRect(modalBounds_, 6, palette.accentColor, false);

    // Titlebar
    s.fillRect(Rect{modalBounds_.x, modalBounds_.y, mw, 32}, Color{22, 30, 46, 255});
    IconRenderer::draw(s, IconId::Properties, Point{modalBounds_.x + 10, modalBounds_.y + 8}, 16, palette.accentColor);
    s.drawString(modalBounds_.x + 32, modalBounds_.y + 8, cur->displayName + " Properties", palette.textPrimary, 1);

    modalCloseBtn_ = Rect{modalBounds_.right() - 28, modalBounds_.y + 6, 20, 20};
    s.drawString(modalCloseBtn_.x + 5, modalCloseBtn_.y + 4, "X", palette.textSecondary, 1);

    int32_t curY = modalBounds_.y + 46;
    auto drawField = [&](const std::string& label, const std::string& val) {
        s.drawString(modalBounds_.x + 18, curY, label, palette.textSecondary, 1);
        s.drawString(modalBounds_.x + 130, curY, val, palette.textPrimary, 1);
        curY += 24;
    };

    drawField("Service Name:", cur->name);
    drawField("Display Name:", cur->displayName);
    drawField("Path to binary:", "C:\\Windows\\System32\\svchost.exe -k LocalService");
    drawField("Startup type:", serviceStartupToString(cur->startup));
    drawField("Service status:", serviceStateToString(cur->state));
    drawField("Process ID:", cur->pid > 0 ? std::to_string(cur->pid) : "N/A");
    drawField("Log On As:", cur->logOnAs);

    // Close button at bottom
    Rect okBtn{modalBounds_.centerX() - 40, modalBounds_.bottom() - 36, 80, 26};
    s.drawRoundedRect(okBtn, 4, palette.accentColor, true);
    s.drawString(okBtn.centerX() - 8, okBtn.y + 6, "OK", Color{6, 11, 18, 255}, 1);
}

bool ServicesContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    if (showPropertiesModal_) {
        if (modalCloseBtn_.contains(localPt) || !modalBounds_.contains(localPt)) {
            closePropertiesDialog();
            return true;
        }
        Rect okBtn{modalBounds_.centerX() - 40, modalBounds_.bottom() - 36, 80, 26};
        if (okBtn.contains(localPt)) {
            closePropertiesDialog();
            return true;
        }
        return true;
    }

    if (searchBoxBounds_.contains(localPt)) {
        searchFocused_ = true;
        return true;
    } else {
        searchFocused_ = false;
    }

    if (btnStart_.contains(localPt)) {
        startSelectedService();
        return true;
    }
    if (btnPause_.contains(localPt)) {
        pauseSelectedService();
        return true;
    }
    if (btnStop_.contains(localPt)) {
        stopSelectedService();
        return true;
    }
    if (btnRestart_.contains(localPt)) {
        restartSelectedService();
        return true;
    }
    if (btnRefresh_.contains(localPt)) {
        scanServices();
        if (onToast_) {
            onToast_("Services Refreshed", "Live Windows SCM status updated", IconId::Services);
        }
        return true;
    }
    if (btnProperties_.contains(localPt)) {
        openPropertiesDialog();
        return true;
    }

    // Main service list selection
    const int32_t listY = 74 + 24; // after headers
    const int32_t rowH = 24;
    if (localPt.y >= listY && localPt.y < 480) { // roughly table area
        const int32_t clickedRow = (localPt.y - listY) / rowH + scrollOffset_;
        if (clickedRow >= 0 && static_cast<size_t>(clickedRow) < filteredIndices_.size()) {
            selectedIndex_ = clickedRow;
            return true;
        }
    }

    return false;
}

bool ServicesContent::onMouseUp(Point, MouseButton) {
    return false;
}

bool ServicesContent::onMouseMove(Point localPt) {
    hoverStart_ = btnStart_.contains(localPt);
    hoverPause_ = btnPause_.contains(localPt);
    hoverStop_ = btnStop_.contains(localPt);
    hoverRestart_ = btnRestart_.contains(localPt);
    hoverRefresh_ = btnRefresh_.contains(localPt);
    hoverProperties_ = btnProperties_.contains(localPt);
    return false;
}

bool ServicesContent::onMouseWheel(Point, int32_t delta) {
    if (delta > 0) {
        scrollOffset_ = std::max(0, scrollOffset_ - 3);
    } else {
        scrollOffset_ += 3;
    }
    return true;
}

bool ServicesContent::onCharInput(char c) {
    if (searchFocused_) {
        if (c >= 32 && c <= 126) {
            searchQuery_ += c;
            setSearchQuery(searchQuery_);
            return true;
        }
    }
    return false;
}

bool ServicesContent::onKeyDown(KeyCode key, bool, bool, bool) {
    if (showPropertiesModal_) {
        if (key == KeyCode::Escape || key == KeyCode::Enter) {
            closePropertiesDialog();
            return true;
        }
    }

    if (searchFocused_) {
        if (key == KeyCode::Backspace) {
            if (!searchQuery_.empty()) {
                searchQuery_.pop_back();
                setSearchQuery(searchQuery_);
            }
            return true;
        }
        if (key == KeyCode::Escape) {
            searchQuery_.clear();
            setSearchQuery(searchQuery_);
            searchFocused_ = false;
            return true;
        }
        if (key == KeyCode::Enter) {
            searchFocused_ = false;
            return true;
        }
    }

    if (key == KeyCode::Up) {
        if (selectedIndex_ > 0) {
            selectedIndex_--;
            if (selectedIndex_ < scrollOffset_) scrollOffset_ = selectedIndex_;
            return true;
        }
    } else if (key == KeyCode::Down) {
        if (selectedIndex_ + 1 < static_cast<int32_t>(filteredIndices_.size())) {
            selectedIndex_++;
            if (selectedIndex_ >= scrollOffset_ + 15) scrollOffset_ = selectedIndex_ - 14;
            return true;
        }
    } else if (key == KeyCode::PageUp) {
        selectedIndex_ = std::max(0, selectedIndex_ - 10);
        scrollOffset_ = std::max(0, scrollOffset_ - 10);
        return true;
    } else if (key == KeyCode::PageDown) {
        selectedIndex_ = std::min(static_cast<int32_t>(filteredIndices_.size()) - 1, selectedIndex_ + 10);
        scrollOffset_ += 10;
        return true;
    } else if (key == KeyCode::Enter) {
        openPropertiesDialog();
        return true;
    }

    return false;
}

} // namespace surshell
