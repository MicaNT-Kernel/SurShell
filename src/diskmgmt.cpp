// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/diskmgmt.cpp)
//
// Sovereign Disk Management & Volume Partitioning (diskmgmt.msc Parity)
// ============================================================================

#include "surshell/diskmgmt.hpp"
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

DiskManagementContent::DiskManagementContent() {
    scanDisks();
}

void DiskManagementContent::scanDisks() {
    populateDefaultDisks();
    collectLiveStorageTopology();
    statusMessage_ = "Disk topology updated";
}

void DiskManagementContent::populateDefaultDisks() {
    disks_.clear();

    // ------------------------------------------------------------------------
    // Disk 0: Primary System NVMe Drive (GPT, 1TB)
    // ------------------------------------------------------------------------
    PhysicalDisk disk0{
        .index = 0,
        .name = "Disk 0",
        .model = "Samsung SSD 980 PRO 1TB (NVMe)",
        .type = "Basic",
        .partitionStyle = "GPT",
        .totalMb = 953869, // ~931.51 GB
        .status = "Online",
        .partitions = {
            {
                .name = "EFI System Partition",
                .driveLetter = "",
                .fileSystem = "FAT32",
                .status = "Healthy (EFI System Partition)",
                .capacityMb = 100,
                .freeSpaceMb = 70,
                .kind = PartitionKind::EFI,
                .isBoot = false,
                .isSystem = true
            },
            {
                .name = "MicaNT (C:)",
                .driveLetter = "C:",
                .fileSystem = "NTFS",
                .status = "Healthy (Boot, Page File, Crash Dump, Basic Data)",
                .capacityMb = 952769,
                .freeSpaceMb = 452100,
                .kind = PartitionKind::Primary,
                .isBoot = true,
                .isSystem = true
            },
            {
                .name = "Recovery",
                .driveLetter = "",
                .fileSystem = "NTFS",
                .status = "Healthy (Recovery Partition)",
                .capacityMb = 1000,
                .freeSpaceMb = 480,
                .kind = PartitionKind::Recovery,
                .isBoot = false,
                .isSystem = false
            }
        }
    };

    // ------------------------------------------------------------------------
    // Disk 1: Secondary High-Speed NVMe Storage (GPT, 2TB)
    // ------------------------------------------------------------------------
    PhysicalDisk disk1{
        .index = 1,
        .name = "Disk 1",
        .model = "Crucial CT2000T500SSD8 2TB (NVMe)",
        .type = "Basic",
        .partitionStyle = "GPT",
        .totalMb = 1907729, // ~1863.01 GB
        .status = "Online",
        .partitions = {
            {
                .name = "Data (D:)",
                .driveLetter = "D:",
                .fileSystem = "NTFS",
                .status = "Healthy (Basic Data Partition)",
                .capacityMb = 1907729,
                .freeSpaceMb = 1205300,
                .kind = PartitionKind::Primary,
                .isBoot = false,
                .isSystem = false
            }
        }
    };

    // ------------------------------------------------------------------------
    // Disk 2: Virtual Cloud Storage Drive (Google Drive G:)
    // ------------------------------------------------------------------------
    PhysicalDisk disk2{
        .index = 2,
        .name = "Disk 2",
        .model = "Google Drive Virtual Cloud Stream",
        .type = "Virtual",
        .partitionStyle = "GPT",
        .totalMb = 102400, // 100 GB
        .status = "Online",
        .partitions = {
            {
                .name = "Google Drive (G:)",
                .driveLetter = "G:",
                .fileSystem = "CloudFS",
                .status = "Healthy (Cloud Storage Mount)",
                .capacityMb = 102400,
                .freeSpaceMb = 61440,
                .kind = PartitionKind::Cloud,
                .isBoot = false,
                .isSystem = false
            }
        }
    };

    // ------------------------------------------------------------------------
    // Disk 3: Network Attached Storage Share (\\nas.ash-forge.com\storage)
    // ------------------------------------------------------------------------
    PhysicalDisk disk3{
        .index = 3,
        .name = "Disk 3",
        .model = "MicaNT Sovereign NAS Storage Pool (SMB 3.1.1)",
        .type = "Network",
        .partitionStyle = "GPT",
        .totalMb = 16384000, // 16 TB
        .status = "Online",
        .partitions = {
            {
                .name = "NAS Storage (\\\\nas\\storage)",
                .driveLetter = "\\\\nas",
                .fileSystem = "SMB",
                .status = "Healthy (Network Location)",
                .capacityMb = 16384000,
                .freeSpaceMb = 9216000,
                .kind = PartitionKind::Network,
                .isBoot = false,
                .isSystem = false
            }
        }
    };

    disks_.push_back(std::move(disk0));
    disks_.push_back(std::move(disk1));
    disks_.push_back(std::move(disk2));
    disks_.push_back(std::move(disk3));
}

void DiskManagementContent::collectLiveStorageTopology() {
#if defined(_WIN32)
    // 1. Query Real Physical Disk Models from System Registry
    HKEY hDiskEnum{};
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\disk\\Enum", 0, KEY_READ, &hDiskEnum) == ERROR_SUCCESS) {
        DWORD count = 0;
        DWORD countSize = sizeof(count);
        RegQueryValueExA(hDiskEnum, "Count", nullptr, nullptr, reinterpret_cast<LPBYTE>(&count), &countSize);

        for (DWORD i = 0; i < count && i < disks_.size(); ++i) {
            char instId[512]{};
            DWORD instIdSize = sizeof(instId);
            std::string valName = std::to_string(i);
            if (RegQueryValueExA(hDiskEnum, valName.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(instId), &instIdSize) == ERROR_SUCCESS) {
                std::string enumKeyPath = std::string("SYSTEM\\CurrentControlSet\\Enum\\") + instId;
                HKEY hDevKey{};
                if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, enumKeyPath.c_str(), 0, KEY_READ, &hDevKey) == ERROR_SUCCESS) {
                    char friendly[256]{};
                    DWORD friendlySize = sizeof(friendly);
                    if (RegQueryValueExA(hDevKey, "FriendlyName", nullptr, nullptr, reinterpret_cast<LPBYTE>(friendly), &friendlySize) == ERROR_SUCCESS) {
                        if (friendly[0] != '\0') {
                            disks_[i].model = friendly;
                        }
                    }
                    RegCloseKey(hDevKey);
                }
            }
        }
        RegCloseKey(hDiskEnum);
    }

    // 2. Query Real Drive Volumes and Capacity
    auto queryDrive = [this](char letter, const std::string& targetLetter) {
        std::string root = std::string(1, letter) + ":\\";
        ULARGE_INTEGER freeBytesAvail{}, totalBytes{}, totalFreeBytes{};
        if (GetDiskFreeSpaceExA(root.c_str(), &freeBytesAvail, &totalBytes, &totalFreeBytes)) {
            const uint64_t totMb = totalBytes.QuadPart / (1024 * 1024);
            const uint64_t freeMb = totalFreeBytes.QuadPart / (1024 * 1024);

            char volName[MAX_PATH + 1]{};
            char fsName[MAX_PATH + 1]{};
            DWORD serial = 0, maxComponentLen = 0, flags = 0;
            GetVolumeInformationA(root.c_str(), volName, sizeof(volName), &serial, &maxComponentLen, &flags, fsName, sizeof(fsName));

            for (auto& disk : disks_) {
                for (auto& part : disk.partitions) {
                    if (part.driveLetter == targetLetter) {
                        part.capacityMb = totMb;
                        part.freeSpaceMb = freeMb;
                        if (fsName[0] != '\0') {
                            part.fileSystem = fsName;
                        }
                        if (volName[0] != '\0') {
                            part.name = std::string(volName) + " (" + targetLetter + ")";
                        }
                        if (part.isBoot) {
                            disk.totalMb = totMb + 1350; // Account for EFI (350MB) + Recovery (1000MB)
                        } else if (disk.partitions.size() == 1) {
                            disk.totalMb = totMb;
                        }
                        break;
                    }
                }
            }
        }
    };

    queryDrive('C', "C:");
    queryDrive('D', "D:");
    queryDrive('G', "G:");
#endif
}

size_t DiskManagementContent::volumeCount() const noexcept {
    size_t count = 0;
    for (const auto& d : disks_) {
        for (const auto& p : d.partitions) {
            if (!p.driveLetter.empty() || p.kind == PartitionKind::Primary || p.kind == PartitionKind::EFI || p.kind == PartitionKind::Recovery) {
                count++;
            }
        }
    }
    return count;
}

void DiskManagementContent::selectVolume(const std::string& driveLetter) {
    for (size_t di = 0; di < disks_.size(); ++di) {
        for (size_t pi = 0; pi < disks_[di].partitions.size(); ++pi) {
            if (disks_[di].partitions[pi].driveLetter == driveLetter) {
                selectedDiskIndex_ = static_cast<int32_t>(di);
                selectedPartitionIndex_ = static_cast<int32_t>(pi);
                return;
            }
        }
    }
}

void DiskManagementContent::selectPartition(int32_t diskIndex, size_t partitionIndex) {
    if (diskIndex >= 0 && diskIndex < static_cast<int32_t>(disks_.size())) {
        if (partitionIndex < disks_[static_cast<size_t>(diskIndex)].partitions.size()) {
            selectedDiskIndex_ = diskIndex;
            selectedPartitionIndex_ = static_cast<int32_t>(partitionIndex);
        }
    }
}

const DiskPartition* DiskManagementContent::selectedPartition() const noexcept {
    if (selectedDiskIndex_ >= 0 && selectedDiskIndex_ < static_cast<int32_t>(disks_.size())) {
        const auto& d = disks_[static_cast<size_t>(selectedDiskIndex_)];
        if (selectedPartitionIndex_ >= 0 && selectedPartitionIndex_ < static_cast<int32_t>(d.partitions.size())) {
            return &d.partitions[static_cast<size_t>(selectedPartitionIndex_)];
        }
    }
    return nullptr;
}

bool DiskManagementContent::changeDriveLetter(const std::string& oldLetter, const std::string& newLetter) {
    for (auto& disk : disks_) {
        for (auto& part : disk.partitions) {
            if (part.driveLetter == oldLetter) {
                part.driveLetter = newLetter;
                part.name = part.name.substr(0, part.name.find('(')) + "(" + newLetter + ")";
                statusMessage_ = "Drive letter changed to " + newLetter;
                if (onToast_) {
                    onToast_("Disk Management", statusMessage_, IconId::DiskManagement);
                }
                return true;
            }
        }
    }
    return false;
}

bool DiskManagementContent::shrinkVolume(const std::string& driveLetter, uint64_t shrinkMb) {
    for (auto& disk : disks_) {
        for (size_t i = 0; i < disk.partitions.size(); ++i) {
            auto& part = disk.partitions[i];
            if (part.driveLetter == driveLetter) {
                if (part.freeSpaceMb > shrinkMb + 1024) {
                    part.capacityMb -= shrinkMb;
                    part.freeSpaceMb -= shrinkMb;
                    
                    // Add unallocated space
                    disk.partitions.insert(disk.partitions.begin() + static_cast<ptrdiff_t>(i + 1), DiskPartition{
                        .name = "Unallocated",
                        .driveLetter = "",
                        .fileSystem = "RAW",
                        .status = "Unallocated",
                        .capacityMb = shrinkMb,
                        .freeSpaceMb = shrinkMb,
                        .kind = PartitionKind::Unallocated
                    });
                    statusMessage_ = "Volume " + driveLetter + " shrunk by " + std::to_string(shrinkMb / 1024) + " GB";
                    if (onToast_) {
                        onToast_("Disk Management", statusMessage_, IconId::DiskManagement);
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

bool DiskManagementContent::extendVolume(const std::string& driveLetter, uint64_t extendMb) {
    for (auto& disk : disks_) {
        for (size_t i = 0; i < disk.partitions.size(); ++i) {
            if (disk.partitions[i].driveLetter == driveLetter) {
                if (i + 1 < disk.partitions.size() && disk.partitions[i + 1].kind == PartitionKind::Unallocated) {
                    const uint64_t avail = disk.partitions[i + 1].capacityMb;
                    const uint64_t toAdd = std::min(avail, extendMb);
                    disk.partitions[i].capacityMb += toAdd;
                    disk.partitions[i].freeSpaceMb += toAdd;
                    if (toAdd == avail) {
                        disk.partitions.erase(disk.partitions.begin() + static_cast<ptrdiff_t>(i + 1));
                    } else {
                        disk.partitions[i + 1].capacityMb -= toAdd;
                        disk.partitions[i + 1].freeSpaceMb -= toAdd;
                    }
                    statusMessage_ = "Volume " + driveLetter + " extended by " + std::to_string(toAdd / 1024) + " GB";
                    if (onToast_) {
                        onToast_("Disk Management", statusMessage_, IconId::DiskManagement);
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

void DiskManagementContent::openPropertiesDialog() {
    if (selectedPartition()) {
        showPropertiesModal_ = true;
    }
}

void DiskManagementContent::closePropertiesDialog() {
    showPropertiesModal_ = false;
}

void DiskManagementContent::render(Surface& clientSurface) {
    const int32_t w = clientSurface.width();
    const int32_t h = clientSurface.height();

    // 1. Dark Acrylic Window Background
    clientSurface.fillRect(Rect{0, 0, w, h}, Color::fromHex(0x0C101A));

    // 2. Command Toolbar (Height 42px)
    toolbarRect_ = Rect{0, 0, w, 42};
    clientSurface.fillRect(toolbarRect_, Color::fromHex(0x131A29));
    clientSurface.fillRect(Rect{0, 41, w, 1}, Color::fromHex(0x1E293B));

    int32_t btnX = 10;
    const int32_t btnY = 8;
    const int32_t btnH = 26;

    auto drawToolBtn = [&](Rect& target, const std::string& label, IconId icon, Color iconTint, int32_t btnW) {
        target = Rect{btnX, btnY, btnW, btnH};
        clientSurface.drawRoundedRect(target, 4, Color::fromHex(0x1E293B), true);
        clientSurface.drawRoundedRect(target, 4, Color::fromHex(0x334155), false);
        IconRenderer::draw(clientSurface, icon, Point{btnX + 6, btnY + 5}, 16, iconTint);
        clientSurface.drawString(Point{btnX + 26, btnY + 7}, label, Color::fromHex(0xF8FAFC));
        btnX += btnW + 6;
    };

    drawToolBtn(refreshBtn_, "Refresh", IconId::NavRefresh, Color::fromHex(0x38BDF8), 90);
    drawToolBtn(rescanBtn_, "Rescan Disks", IconId::LocalDisk, Color::fromHex(0x00D4FF), 120);
    drawToolBtn(changeLetterBtn_, "Change Letter", IconId::Rename, Color::fromHex(0xF59E0B), 125);
    drawToolBtn(shrinkBtn_, "Shrink", IconId::Cut, Color::fromHex(0xEF4444), 85);
    drawToolBtn(extendBtn_, "Extend", IconId::ViewGrid, Color::fromHex(0x00FF9D), 85);
    drawToolBtn(propertiesBtn_, "Properties", IconId::Properties, Color::fromHex(0x38BDF8), 105);

    // 3. Top Volume Table Pane (Height 175px: y=42 to 217)
    volumeTablePane_ = Rect{0, 42, w, 175};
    clientSurface.fillRect(volumeTablePane_, Color::fromHex(0x0E1422));

    // Table Column Headers
    const Rect headerR{0, 42, w, 24};
    clientSurface.fillRect(headerR, Color::fromHex(0x161F30));
    clientSurface.fillRect(Rect{0, 65, w, 1}, Color::fromHex(0x1E293B));

    clientSurface.drawString(Point{12, 48}, "Volume", Color::fromHex(0x94A3B8));
    clientSurface.drawString(Point{150, 48}, "Layout", Color::fromHex(0x94A3B8));
    clientSurface.drawString(Point{220, 48}, "Type", Color::fromHex(0x94A3B8));
    clientSurface.drawString(Point{280, 48}, "File System", Color::fromHex(0x94A3B8));
    clientSurface.drawString(Point{380, 48}, "Status", Color::fromHex(0x94A3B8));
    clientSurface.drawString(Point{620, 48}, "Capacity", Color::fromHex(0x94A3B8));
    clientSurface.drawString(Point{710, 48}, "Free Space", Color::fromHex(0x94A3B8));
    clientSurface.drawString(Point{800, 48}, "% Free", Color::fromHex(0x94A3B8));

    // Flatten all volumes for table
    volumeRowBounds_.clear();
    int32_t tableY = 66;
    const int32_t vRowH = 22;

    int32_t flatVolIndex = 0;
    for (size_t di = 0; di < disks_.size(); ++di) {
        for (size_t pi = 0; pi < disks_[di].partitions.size(); ++pi) {
            const auto& part = disks_[di].partitions[pi];
            if (part.kind == PartitionKind::Unallocated) continue;

            const Rect rowR{0, tableY, w, vRowH};
            volumeRowBounds_.push_back(rowR);

            const bool isRowSelected = (selectedDiskIndex_ == static_cast<int32_t>(di) && selectedPartitionIndex_ == static_cast<int32_t>(pi));

            if (isRowSelected) {
                clientSurface.fillRect(rowR, Color::fromRgba(0, 212, 255, 35));
                clientSurface.fillRect(Rect{0, tableY, 3, vRowH}, Color::fromHex(0x00D4FF));
            } else if (flatVolIndex % 2 == 1) {
                clientSurface.fillRect(rowR, Color::fromHex(0x0A0F19));
            }

            // Volume Label & Icon
            IconId vIcon = IconId::LocalDisk;
            if (part.kind == PartitionKind::Cloud) vIcon = IconId::CloudDrive;
            else if (part.kind == PartitionKind::Network) vIcon = IconId::NetworkShare;
            IconRenderer::draw(clientSurface, vIcon, Point{12, tableY + 3}, 16, isRowSelected ? Color::fromHex(0x00FF9D) : Color::fromHex(0x38BDF8));

            clientSurface.drawString(Point{32, tableY + 5}, part.name, isRowSelected ? Color::fromHex(0xF8FAFC) : Color::fromHex(0xCBD5E1));
            clientSurface.drawString(Point{150, tableY + 5}, "Simple", Color::fromHex(0x94A3B8));
            clientSurface.drawString(Point{220, tableY + 5}, disks_[di].type, Color::fromHex(0x94A3B8));
            clientSurface.drawString(Point{280, tableY + 5}, part.fileSystem, Color::fromHex(0xCBD5E1));

            // Status (truncated if wide)
            std::string st = part.status;
            if (st.length() > 28) st = st.substr(0, 26) + "..";
            clientSurface.drawString(Point{380, tableY + 5}, st, Color::fromHex(0x00FF9D));

            // Capacity & Free Space
            auto formatGb = [](uint64_t mb) {
                std::ostringstream ss;
                if (mb >= 1024000) ss << std::fixed << std::setprecision(1) << (static_cast<double>(mb) / 1024000.0) << " TB";
                else if (mb >= 1000) ss << std::fixed << std::setprecision(1) << (static_cast<double>(mb) / 1024.0) << " GB";
                else ss << mb << " MB";
                return ss.str();
            };

            clientSurface.drawString(Point{620, tableY + 5}, formatGb(part.capacityMb), Color::fromHex(0xF8FAFC));
            clientSurface.drawString(Point{710, tableY + 5}, formatGb(part.freeSpaceMb), Color::fromHex(0xCBD5E1));

            const uint64_t pct = (part.capacityMb > 0) ? ((part.freeSpaceMb * 100) / part.capacityMb) : 0;
            clientSurface.drawString(Point{800, tableY + 5}, std::to_string(pct) + "%", Color::fromHex(0x00D4FF));

            tableY += vRowH;
            flatVolIndex++;
        }
    }

    // 4. Splitter Bar (Height 8px: y=217 to 225)
    clientSurface.fillRect(Rect{0, 217, w, 8}, Color::fromHex(0x131A29));
    clientSurface.fillRect(Rect{0, 217, w, 1}, Color::fromHex(0x1E293B));
    clientSurface.fillRect(Rect{0, 224, w, 1}, Color::fromHex(0x1E293B));

    // 5. Bottom Disk Graph Pane (y=225 to h-48)
    diskGraphPane_ = Rect{0, 225, w, h - 225 - 48};
    clientSurface.fillRect(diskGraphPane_, Color::fromHex(0x090D15));

    int32_t diskCardY = diskGraphPane_.y + 8;
    const int32_t diskCardH = 68;

    for (size_t di = 0; di < disks_.size(); ++di) {
        auto& disk = disks_[di];
        if (diskCardY + diskCardH > diskGraphPane_.bottom()) break;

        // A. Left Physical Disk Info Card (width 110px)
        disk.headerBounds = Rect{8, diskCardY, 110, diskCardH};
        clientSurface.drawRoundedRect(disk.headerBounds, 4, Color::fromHex(0x131A29), true);
        clientSurface.drawRoundedRect(disk.headerBounds, 4, Color::fromHex(0x1E293B), false);

        clientSurface.drawString(Point{disk.headerBounds.x + 8, disk.headerBounds.y + 6}, disk.name, Color::fromHex(0xF8FAFC));
        clientSurface.drawString(Point{disk.headerBounds.x + 8, disk.headerBounds.y + 22}, disk.type + " (" + disk.partitionStyle + ")", Color::fromHex(0x94A3B8));

        std::ostringstream ssTot;
        if (disk.totalMb >= 1024000) ssTot << std::fixed << std::setprecision(1) << (static_cast<double>(disk.totalMb) / 1024000.0) << " TB";
        else ssTot << std::fixed << std::setprecision(1) << (static_cast<double>(disk.totalMb) / 1024.0) << " GB";
        clientSurface.drawString(Point{disk.headerBounds.x + 8, disk.headerBounds.y + 36}, ssTot.str(), Color::fromHex(0xCBD5E1));

        clientSurface.drawString(Point{disk.headerBounds.x + 8, disk.headerBounds.y + 50}, disk.status, Color::fromHex(0x00FF9D));

        // B. Right Partition Horizontal Strip (width = w - 132px)
        const int32_t stripX = disk.headerBounds.right() + 6;
        const int32_t stripW = w - stripX - 10;
        disk.stripBounds = Rect{stripX, diskCardY, stripW, diskCardH};

        // Render partition boxes proportionally
        int32_t currPartX = stripX;
        const size_t numParts = disk.partitions.size();

        // Calculate layout widths
        const int32_t minBoxW = 75;
        int32_t remainingW = stripW;
        uint64_t unassignedMb = disk.totalMb;

        for (size_t pi = 0; pi < numParts; ++pi) {
            auto& part = disk.partitions[pi];

            int32_t boxW = 0;
            if (pi == numParts - 1) {
                boxW = stripW - (currPartX - stripX);
            } else {
                double frac = (disk.totalMb > 0) ? (static_cast<double>(part.capacityMb) / static_cast<double>(disk.totalMb)) : (1.0 / numParts);
                boxW = std::max(minBoxW, static_cast<int32_t>(frac * stripW));
            }
            boxW = std::max(minBoxW, std::min(boxW, remainingW));

            part.bounds = Rect{currPartX, diskCardY, boxW, diskCardH};

            const bool isPartSelected = (selectedDiskIndex_ == static_cast<int32_t>(di) && selectedPartitionIndex_ == static_cast<int32_t>(pi));

            // Partition background
            clientSurface.fillRect(part.bounds, isPartSelected ? Color::fromHex(0x101B2E) : Color::fromHex(0x0E1422));

            // Color-Coded Top Stripe (Indicating partition kind)
            Color stripeCol = Color::fromHex(0x2563EB); // Blue Primary
            if (part.kind == PartitionKind::EFI) stripeCol = Color::fromHex(0x00D4FF); // Cyan EFI
            else if (part.kind == PartitionKind::Recovery) stripeCol = Color::fromHex(0xF59E0B); // Amber Recovery
            else if (part.kind == PartitionKind::Cloud) stripeCol = Color::fromHex(0x38BDF8); // Sky Blue Cloud
            else if (part.kind == PartitionKind::Network) stripeCol = Color::fromHex(0xA855F7); // Purple SMB
            else if (part.kind == PartitionKind::Unallocated) stripeCol = Color::fromHex(0x475569); // Slate

            clientSurface.fillRect(Rect{part.bounds.x, part.bounds.y, part.bounds.width, 4}, stripeCol);

            // Inner texts
            std::string titleStr = part.name;
            if (titleStr.length() > 18) titleStr = titleStr.substr(0, 16) + "..";
            clientSurface.drawString(Point{part.bounds.x + 6, part.bounds.y + 10}, titleStr, isPartSelected ? Color::fromHex(0x00D4FF) : Color::fromHex(0xF8FAFC));

            std::ostringstream ssCap;
            if (part.capacityMb >= 1024000) ssCap << std::fixed << std::setprecision(1) << (static_cast<double>(part.capacityMb) / 1024000.0) << " TB ";
            else if (part.capacityMb >= 1000) ssCap << std::fixed << std::setprecision(1) << (static_cast<double>(part.capacityMb) / 1024.0) << " GB ";
            else ssCap << part.capacityMb << " MB ";
            ssCap << part.fileSystem;
            clientSurface.drawString(Point{part.bounds.x + 6, part.bounds.y + 26}, ssCap.str(), Color::fromHex(0xCBD5E1));

            std::string stBrief = (part.kind == PartitionKind::Unallocated) ? "Unallocated" : "Healthy";
            if (part.isBoot) stBrief += " (Boot)";
            clientSurface.drawString(Point{part.bounds.x + 6, part.bounds.y + 42}, stBrief, part.kind == PartitionKind::Unallocated ? Color::fromHex(0x94A3B8) : Color::fromHex(0x00FF9D));

            // Partition outline border
            clientSurface.drawRect(part.bounds, isPartSelected ? Color::fromHex(0x00D4FF) : Color::fromHex(0x1E293B));
            if (isPartSelected) {
                // Secondary glow line
                clientSurface.drawRect(Rect{part.bounds.x + 1, part.bounds.y + 1, part.bounds.width - 2, part.bounds.height - 2}, Color::fromRgba(0, 212, 255, 120));
            }

            currPartX += boxW;
            remainingW = std::max(0, stripW - (currPartX - stripX));
        }

        diskCardY += diskCardH + 10;
    }

    // 6. Legend Strip (Height 24px: y=h-48 to h-24)
    const Rect legendRect{0, h - 48, w, 24};
    clientSurface.fillRect(legendRect, Color::fromHex(0x0E1422));
    clientSurface.fillRect(Rect{0, h - 48, w, 1}, Color::fromHex(0x1E293B));

    int32_t legX = 14;
    auto drawLegend = [&](Color col, const std::string& label) {
        clientSurface.fillRect(Rect{legX, h - 41, 10, 10}, col);
        clientSurface.drawRect(Rect{legX, h - 41, 10, 10}, Color::fromHex(0xF8FAFC));
        clientSurface.drawString(Point{legX + 16, h - 40}, label, Color::fromHex(0x94A3B8));
        legX += static_cast<int32_t>(label.length() * 7) + 30;
    };

    drawLegend(Color::fromHex(0x00D4FF), "EFI System");
    drawLegend(Color::fromHex(0x2563EB), "Primary Partition");
    drawLegend(Color::fromHex(0xF59E0B), "Recovery");
    drawLegend(Color::fromHex(0x38BDF8), "Cloud Storage");
    drawLegend(Color::fromHex(0xA855F7), "Network SMB");
    drawLegend(Color::fromHex(0x475569), "Unallocated");

    // 7. Status Bar (Height 24px)
    const Rect statusBarRect{0, h - 24, w, 24};
    clientSurface.fillRect(statusBarRect, Color::fromHex(0x090D15));
    clientSurface.fillRect(Rect{0, h - 24, w, 1}, Color::fromHex(0x1E293B));

    std::ostringstream statusLeft;
    statusLeft << "Total Disks: " << disks_.size() << " | Volumes: " << volumeCount();
    clientSurface.drawString(Point{10, h - 18}, statusLeft.str(), Color::fromHex(0x94A3B8));

    if (const auto* p = selectedPartition()) {
        std::ostringstream statusRight;
        statusRight << "Selected: " << p->name << " (" << (p->capacityMb / 1024) << " GB " << p->fileSystem << ") - " << p->status;
        clientSurface.drawString(Point{w - 480, h - 18}, statusRight.str(), Color::fromHex(0x00D4FF));
    } else {
        clientSurface.drawString(Point{w - 200, h - 18}, statusMessage_, Color::fromHex(0x00FF9D));
    }

    // 8. Modal Properties Dialog
    if (showPropertiesModal_) {
        const auto* part = selectedPartition();
        if (!part) {
            showPropertiesModal_ = false;
            return;
        }

        // Semi-transparent backdrop dimmer
        clientSurface.fillRect(Rect{0, 0, w, h}, Color::fromRgba(0, 0, 0, 160));

        // Dialog Card (460x340)
        const int32_t dlgW = 460;
        const int32_t dlgH = 340;
        const int32_t dlgX = (w - dlgW) / 2;
        const int32_t dlgY = (h - dlgH) / 2;
        propertiesDialogBounds_ = Rect{dlgX, dlgY, dlgW, dlgH};

        clientSurface.drawRoundedRect(propertiesDialogBounds_, 6, Color::fromHex(0x131A29), true);
        clientSurface.drawRoundedRect(propertiesDialogBounds_, 6, Color::fromHex(0x00D4FF), false);

        // Caption Bar
        const Rect captionR{dlgX, dlgY, dlgW, 32};
        clientSurface.drawRoundedRect(captionR, 6, Color::fromHex(0x1E293B), true);
        IconRenderer::draw(clientSurface, IconId::DiskManagement, Point{dlgX + 10, dlgY + 8}, 16, Color::fromHex(0x00FF9D));
        clientSurface.drawString(Point{dlgX + 32, dlgY + 9}, part->name + " Properties", Color::fromHex(0xF8FAFC));

        // Close Button
        propertiesCloseBtn_ = Rect{dlgX + dlgW - 28, dlgY + 6, 20, 20};
        clientSurface.drawRoundedRect(propertiesCloseBtn_, 3, Color::fromHex(0x334155), true);
        clientSurface.drawString(Point{propertiesCloseBtn_.x + 6, propertiesCloseBtn_.y + 2}, "x", Color::fromHex(0xF8FAFC));

        // Body Content
        int32_t cy = dlgY + 45;
        const int32_t cx = dlgX + 24;

        clientSurface.drawString(Point{cx, cy}, "Type:            Local Fixed Disk Partition", Color::fromHex(0xCBD5E1));
        cy += 22;
        clientSurface.drawString(Point{cx, cy}, "File System:     " + part->fileSystem, Color::fromHex(0xCBD5E1));
        cy += 22;
        clientSurface.drawString(Point{cx, cy}, "Status:          " + part->status, Color::fromHex(0x00FF9D));
        cy += 30;

        // Used / Free Capacity Stats
        const uint64_t usedMb = (part->capacityMb > part->freeSpaceMb) ? (part->capacityMb - part->freeSpaceMb) : 0;
        std::ostringstream ssUsed, ssFree, ssTot;
        ssUsed << (usedMb / 1024) << " GB (" << usedMb << " MB)";
        ssFree << (part->freeSpaceMb / 1024) << " GB (" << part->freeSpaceMb << " MB)";
        ssTot << (part->capacityMb / 1024) << " GB (" << part->capacityMb << " MB)";

        clientSurface.drawString(Point{cx, cy}, "Used space:      " + ssUsed.str(), Color::fromHex(0x38BDF8));
        cy += 22;
        clientSurface.drawString(Point{cx, cy}, "Free space:      " + ssFree.str(), Color::fromHex(0x00FF9D));
        cy += 22;
        clientSurface.drawString(Point{cx, cy}, "Capacity:        " + ssTot.str(), Color::fromHex(0xF8FAFC));
        cy += 30;

        // Visual Capacity Bar
        const int32_t barW = dlgW - 48;
        const Rect barR{cx, cy, barW, 20};
        clientSurface.drawRoundedRect(barR, 4, Color::fromHex(0x0A0E17), true);
        clientSurface.drawRoundedRect(barR, 4, Color::fromHex(0x334155), false);

        if (part->capacityMb > 0) {
            const int32_t usedW = static_cast<int32_t>((static_cast<double>(usedMb) / static_cast<double>(part->capacityMb)) * barW);
            clientSurface.fillRect(Rect{cx, cy, usedW, 20}, Color::fromHex(0x2563EB));
            clientSurface.fillRect(Rect{cx + usedW, cy, barW - usedW, 20}, Color::fromHex(0x00FF9D));
        }

        // OK Button
        propertiesOkBtn_ = Rect{dlgX + dlgW - 90, dlgY + dlgH - 34, 74, 24};
        clientSurface.drawRoundedRect(propertiesOkBtn_, 4, Color::fromHex(0x00D4FF), true);
        clientSurface.drawString(Point{propertiesOkBtn_.centerX() - 8, propertiesOkBtn_.y + 5}, "OK", Color::fromHex(0x040810));
    }
}

bool DiskManagementContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    // 1. If Properties modal is open
    if (showPropertiesModal_) {
        if (propertiesCloseBtn_.contains(localPt) || propertiesOkBtn_.contains(localPt) || !propertiesDialogBounds_.contains(localPt)) {
            closePropertiesDialog();
            return true;
        }
        return true;
    }

    // 2. Toolbar buttons
    if (refreshBtn_.contains(localPt)) {
        refresh();
        return true;
    }
    if (rescanBtn_.contains(localPt)) {
        scanDisks();
        statusMessage_ = "Bus rescan complete";
        if (onToast_) onToast_("Disk Management", statusMessage_, IconId::DiskManagement);
        return true;
    }
    if (changeLetterBtn_.contains(localPt)) {
        if (const auto* p = selectedPartition()) {
            if (!p->driveLetter.empty() && !p->isBoot) {
                const std::string newL = (p->driveLetter == "D:") ? "E:" : "D:";
                changeDriveLetter(p->driveLetter, newL);
                return true;
            }
        }
    }
    if (shrinkBtn_.contains(localPt)) {
        if (const auto* p = selectedPartition()) {
            if (!p->driveLetter.empty()) {
                shrinkVolume(p->driveLetter, 10240); // Shrink 10GB
                return true;
            }
        }
    }
    if (extendBtn_.contains(localPt)) {
        if (const auto* p = selectedPartition()) {
            if (!p->driveLetter.empty()) {
                extendVolume(p->driveLetter, 10240); // Extend 10GB
                return true;
            }
        }
    }
    if (propertiesBtn_.contains(localPt)) {
        openPropertiesDialog();
        return true;
    }

    // 3. Top Volume Table Click
    for (size_t i = 0; i < volumeRowBounds_.size(); ++i) {
        if (volumeRowBounds_[i].contains(localPt)) {
            // Find which disk & partition corresponds to this flat volume row
            size_t count = 0;
            for (size_t di = 0; di < disks_.size(); ++di) {
                for (size_t pi = 0; pi < disks_[di].partitions.size(); ++pi) {
                    if (disks_[di].partitions[pi].kind == PartitionKind::Unallocated) continue;
                    if (count == i) {
                        selectPartition(static_cast<int32_t>(di), pi);
                        return true;
                    }
                    count++;
                }
            }
        }
    }

    // 4. Bottom Partition Graph Click
    for (size_t di = 0; di < disks_.size(); ++di) {
        for (size_t pi = 0; pi < disks_[di].partitions.size(); ++pi) {
            if (disks_[di].partitions[pi].bounds.contains(localPt)) {
                selectPartition(static_cast<int32_t>(di), pi);
                return true;
            }
        }
    }

    return false;
}

bool DiskManagementContent::onMouseUp(Point localPt, MouseButton button) {
    (void)localPt;
    (void)button;
    return false;
}

bool DiskManagementContent::onMouseMove(Point localPt) {
    (void)localPt;
    return false;
}

bool DiskManagementContent::onMouseWheel(Point localPt, int32_t delta) {
    (void)localPt;
    if (delta > 0) {
        topScrollOffset_ = std::max(0, topScrollOffset_ - 1);
    } else if (delta < 0) {
        topScrollOffset_++;
    }
    return true;
}

bool DiskManagementContent::onCharInput(char c) {
    (void)c;
    return false;
}

bool DiskManagementContent::onKeyDown(KeyCode key, bool ctrl, bool shift, bool alt) {
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

    if (key == KeyCode::Up) {
        if (selectedDiskIndex_ > 0) {
            selectedDiskIndex_--;
            selectedPartitionIndex_ = 0;
            return true;
        }
    }
    if (key == KeyCode::Down) {
        if (selectedDiskIndex_ + 1 < static_cast<int32_t>(disks_.size())) {
            selectedDiskIndex_++;
            selectedPartitionIndex_ = 0;
            return true;
        }
    }
    if (key == KeyCode::Left) {
        if (selectedPartitionIndex_ > 0) {
            selectedPartitionIndex_--;
            return true;
        }
    }
    if (key == KeyCode::Right) {
        if (selectedDiskIndex_ >= 0 && selectedDiskIndex_ < static_cast<int32_t>(disks_.size())) {
            if (selectedPartitionIndex_ + 1 < static_cast<int32_t>(disks_[selectedDiskIndex_].partitions.size())) {
                selectedPartitionIndex_++;
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
