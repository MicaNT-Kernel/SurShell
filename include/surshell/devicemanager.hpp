// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/devicemanager.hpp)
//
// Sovereign Device Manager & Hardware Inspector (devmgmt.msc Parity)
// Clean-room ISO C++23, zero telemetry, live host hardware tree,
// processor topology, storage volumes, network controllers & driver vitals.
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

struct DeviceProperty {
    std::string name;
    std::string value;
};

struct DeviceItem {
    std::string id;
    std::string name;
    IconId iconId{IconId::SystemInfo};
    std::string status{"This device is working properly. (Code 0)"};
    std::string manufacturer{"MicaNT Sovereign Hardware"};
    std::string driverVersion{"10.0.26100.1"};
    std::string hardwareId{"ROOT\\MicaNT_HW_0001"};
    std::string location{"PCI Bus 0, Device 0, Function 0"};
    bool isEnabled{true};
    std::vector<DeviceProperty> properties{};
};

struct DeviceCategory {
    std::string id;
    std::string name;
    IconId iconId{IconId::Folder};
    bool isExpanded{true};
    std::vector<DeviceItem> devices{};
};

class DeviceManagerContent : public IWindowContent {
public:
    DeviceManagerContent();

    void scanForHardwareChanges();
    void refresh() { scanForHardwareChanges(); }

    void toggleCategory(const std::string& categoryId);
    void expandAll();
    void collapseAll();

    void setFilterQuery(const std::string& query);
    [[nodiscard]] const std::string& filterQuery() const noexcept { return filterQuery_; }

    [[nodiscard]] size_t totalDeviceCount() const noexcept;
    [[nodiscard]] size_t categoryCount() const noexcept { return categories_.size(); }
    [[nodiscard]] const std::vector<DeviceCategory>& categories() const noexcept { return categories_; }

    [[nodiscard]] int32_t selectedIndex() const noexcept { return selectedFlatIndex_; }
    void selectIndex(int32_t index);
    void selectDevice(const std::string& deviceId);

    void toggleSelectedDeviceEnabled();
    [[nodiscard]] const DeviceItem* selectedDevice() const noexcept;

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
    void populateDefaultHardwareTree();
    void collectHostHardwareTelemetry();
    void rebuildFlatList();

    struct FlatRow {
        bool isCategory{false};
        std::string categoryId;
        std::string deviceId;
        std::string label;
        IconId iconId{IconId::SystemInfo};
        bool isExpanded{true};
        bool isEnabled{true};
        size_t childCount{0};
        Rect bounds{};
    };

    std::vector<DeviceCategory> categories_{};
    std::vector<FlatRow> flatRows_{};
    int32_t selectedFlatIndex_{0};
    int32_t scrollOffset_{0};
    std::string filterQuery_{};
    bool searchFocused_{false};

    // Modal Properties Dialog State
    bool showPropertiesModal_{false};
    int32_t propertiesTab_{0}; // 0 = General, 1 = Driver, 2 = Details
    Rect propertiesDialogBounds_{};
    Rect propertiesCloseBtn_{};
    Rect propertiesTabGeneral_{};
    Rect propertiesTabDriver_{};
    Rect propertiesTabDetails_{};

    // Hit-test UI regions
    Rect scanBtn_{};
    Rect propertiesBtn_{};
    Rect toggleEnableBtn_{};
    Rect expandAllBtn_{};
    Rect searchBox_{};
    Rect treePaneRect_{};

    std::string statusMessage_{"Ready"};
    std::function<void(const std::string&, const std::string&, IconId)> onToast_{};
};

} // namespace surshell
