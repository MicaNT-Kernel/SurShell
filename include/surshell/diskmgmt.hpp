// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/diskmgmt.hpp)
//
// Sovereign Disk Management & Volume Partitioning (diskmgmt.msc Parity)
// Clean-room ISO C++23, zero telemetry, storage partitioning,
// volume geometry, multi-drive discovery (C:, D:, Google Drive, NAS).
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include "window_manager.hpp"
#include "theme.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace surshell {

enum class PartitionKind {
    EFI,
    Primary,
    Extended,
    Recovery,
    Unallocated,
    Cloud,
    Network
};

struct DiskPartition {
    std::string name;
    std::string driveLetter;
    std::string fileSystem{"NTFS"};
    std::string status{"Healthy (Primary Partition)"};
    uint64_t capacityMb{0};
    uint64_t freeSpaceMb{0};
    PartitionKind kind{PartitionKind::Primary};
    bool isBoot{false};
    bool isSystem{false};
    Rect bounds{};
};

struct PhysicalDisk {
    int32_t index{0};
    std::string name{"Disk 0"};
    std::string model{"NVMe Solid State Drive"};
    std::string type{"Basic"};
    std::string partitionStyle{"GPT"};
    uint64_t totalMb{0};
    std::string status{"Online"};
    std::vector<DiskPartition> partitions{};
    Rect headerBounds{};
    Rect stripBounds{};
};

class DiskManagementContent : public IWindowContent {
public:
    DiskManagementContent();

    void scanDisks();
    void refresh() { scanDisks(); }

    [[nodiscard]] size_t diskCount() const noexcept { return disks_.size(); }
    [[nodiscard]] size_t volumeCount() const noexcept;
    [[nodiscard]] const std::vector<PhysicalDisk>& disks() const noexcept { return disks_; }

    // Selection Handling
    void selectVolume(const std::string& driveLetter);
    void selectPartition(int32_t diskIndex, size_t partitionIndex);
    [[nodiscard]] const DiskPartition* selectedPartition() const noexcept;
    [[nodiscard]] int32_t selectedDiskIndex() const noexcept { return selectedDiskIndex_; }
    [[nodiscard]] int32_t selectedPartitionIndex() const noexcept { return selectedPartitionIndex_; }

    // Actions & Power User Operations
    bool changeDriveLetter(const std::string& oldLetter, const std::string& newLetter);
    bool shrinkVolume(const std::string& driveLetter, uint64_t shrinkMb);
    bool extendVolume(const std::string& driveLetter, uint64_t extendMb);

    void openPropertiesDialog();
    void closePropertiesDialog();
    [[nodiscard]] bool isPropertiesDialogOpen() const noexcept { return showPropertiesModal_; }

    // Toast and notification callback
    void setToastCallback(std::function<void(const std::string&, const std::string&, IconId)> cb) {
        onToast_ = std::move(cb);
    }

    // IWindowContent overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseUp(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onMouseWheel(Point localPt, int32_t delta) override;
    bool onCharInput(char c) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;

private:
    void populateDefaultDisks();
    void collectLiveStorageTopology();

    std::vector<PhysicalDisk> disks_{};
    int32_t selectedDiskIndex_{0};
    int32_t selectedPartitionIndex_{1}; // Default to C: (partition 1 on Disk 0)
    int32_t selectedVolumeTableIndex_{0};

    // UI Layout Rectangles
    Rect toolbarRect_{};
    Rect refreshBtn_{};
    Rect rescanBtn_{};
    Rect changeLetterBtn_{};
    Rect shrinkBtn_{};
    Rect extendBtn_{};
    Rect propertiesBtn_{};

    Rect volumeTablePane_{};
    Rect diskGraphPane_{};
    std::vector<Rect> volumeRowBounds_{};

    // Modal Properties Dialog State
    bool showPropertiesModal_{false};
    Rect propertiesDialogBounds_{};
    Rect propertiesCloseBtn_{};
    Rect propertiesOkBtn_{};

    int32_t topScrollOffset_{0};
    int32_t bottomScrollOffset_{0};
    std::string statusMessage_{"Ready"};
    std::function<void(const std::string&, const std::string&, IconId)> onToast_{};
};

} // namespace surshell
