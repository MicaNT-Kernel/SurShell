// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/registry_editor.hpp)
//
// Sovereign Clean-Room Registry Editor (regedit.exe).
// Clean-room hierarchical configuration database editor for MicaNT:
// - Tree view hierarchy (Computer, HKCR, HKCU, HKLM, HKU, HKCC)
// - Expand/collapse nodes with procedural vector chevrons and folder icons
// - Values multi-column list view (Name, Type, Data)
// - Support for REG_SZ, REG_DWORD, REG_QWORD, REG_BINARY, REG_MULTI_SZ
// - Address bar with full path display, editable navigation, and Back/Up buttons
// - Classic menu bar (File, Edit, View, Favorites, Help)
// - Status bar with active key path and item counts
// ============================================================================

#pragma once

#include "types.hpp"
#include "window_manager.hpp"
#include "theme.hpp"
#include "icons.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

namespace surshell {

enum class RegType : uint32_t {
    None = 0,
    Sz = 1,                 // String (REG_SZ)
    ExpandSz = 2,           // Expandable String (REG_EXPAND_SZ)
    Binary = 3,             // Free-form binary (REG_BINARY)
    Dword = 4,              // 32-bit unsigned int (REG_DWORD)
    DwordBigEndian = 5,
    Link = 6,
    MultiSz = 7,            // Multi-string (REG_MULTI_SZ)
    Qword = 11              // 64-bit unsigned int (REG_QWORD)
};

inline std::string regTypeToString(RegType t) {
    switch (t) {
        case RegType::Sz: return "REG_SZ";
        case RegType::ExpandSz: return "REG_EXPAND_SZ";
        case RegType::Binary: return "REG_BINARY";
        case RegType::Dword: return "REG_DWORD";
        case RegType::MultiSz: return "REG_MULTI_SZ";
        case RegType::Qword: return "REG_QWORD";
        default: return "REG_NONE";
    }
}

struct RegistryValue {
    std::string name;
    RegType type{RegType::Sz};
    std::string stringData{};
    uint32_t dwordData{0};
    uint64_t qwordData{0};
    std::vector<uint8_t> binaryData{};
    std::vector<std::string> multiSzData{};

    [[nodiscard]] std::string formatDisplayData() const;
};

struct RegistryNode {
    std::string name;
    RegistryNode* parent{nullptr};
    std::vector<std::shared_ptr<RegistryNode>> children{};
    std::vector<RegistryValue> values{};
    bool expanded{false};

    [[nodiscard]] std::string fullPath() const;
    std::shared_ptr<RegistryNode> addSubkey(std::string childName);

    void setValueSz(std::string valName, std::string data);
    void setValueDword(std::string valName, uint32_t data);
    void setValueQword(std::string valName, uint64_t data);
    void setValueBinary(std::string valName, std::vector<uint8_t> data);
    [[nodiscard]] const RegistryValue* findValue(const std::string& valName) const;
};

class RegistryEditorContent : public IWindowContent {
public:
    explicit RegistryEditorContent(std::string initialPath = "Computer\\HKEY_CURRENT_USER\\Software\\MicaNT\\SurShell");

    void navigateToPath(const std::string& path);
    [[nodiscard]] std::string currentPath() const;
    [[nodiscard]] size_t keyCount() const;
    [[nodiscard]] size_t valueCount() const;
    [[nodiscard]] const std::vector<RegistryValue>& currentValues() const;

    using PathChangedCallback = std::function<void(const std::string& path)>;
    using ValueModifiedCallback = std::function<void(const std::string& keyPath, const std::string& valueName)>;
    using CloseCallback = std::function<void()>;

    void setPathChangedCallback(PathChangedCallback cb) { onPathChanged_ = std::move(cb); }
    void setValueModifiedCallback(ValueModifiedCallback cb) { onValueModified_ = std::move(cb); }
    void setCloseCallback(CloseCallback cb) { onClose_ = std::move(cb); }

    // IWindowContent overrides
    void render(Surface& clientSurface) override;
    bool onMouseDown(Point localPt, MouseButton button) override;
    bool onMouseMove(Point localPt) override;
    bool onDoubleClick(Point localPt) override;
    bool onMouseWheel(Point localPt, int32_t delta) override;
    bool onCharInput(char c) override;
    bool onKeyDown(KeyCode key, bool ctrl = false, bool shift = false, bool alt = false) override;

private:
    std::shared_ptr<RegistryNode> rootNode_{};
    RegistryNode* selectedNode_{nullptr};
    std::vector<std::string> pathHistory_{};
    int32_t selectedValIdx_{-1};
    int32_t hoveredValIdx_{-1};
    RegistryNode* hoveredNode_{nullptr};

    int32_t splitX_{280};
    bool draggingSplitter_{false};

    std::string addressBarPath_{};
    bool addressBarFocused_{false};

    // Tree flattening for rendering and hit-testing
    struct VisibleTreeItem {
        RegistryNode* node{nullptr};
        int32_t level{0};
        Rect bounds{};
        Rect chevronBounds{};
    };
    std::vector<VisibleTreeItem> visibleTreeItems_{};

    struct VisibleValueItem {
        size_t index{0};
        Rect bounds{};
    };
    std::vector<VisibleValueItem> visibleValueItems_{};

    // Navigation and hit areas
    Rect btnBackBounds_{};
    Rect btnUpBounds_{};
    Rect btnRefreshBounds_{};
    Rect addressBoxBounds_{};
    Rect splitterBounds_{};

    // Menu bounds
    Rect menuFileBounds_{};
    Rect menuEditBounds_{};
    Rect menuViewBounds_{};
    Rect menuFavoritesBounds_{};
    Rect menuHelpBounds_{};

    // Callbacks
    PathChangedCallback onPathChanged_{};
    ValueModifiedCallback onValueModified_{};
    CloseCallback onClose_{};

    void initDefaultDatabase();
    void flattenTree(RegistryNode* node, int32_t level, int32_t& curY);
    RegistryNode* findNodeByPath(RegistryNode* current, const std::string& path);
    void expandPathToNode(RegistryNode* target);

    void renderMenuBar(Surface& s, const ThemePalette& palette);
    void renderAddressBar(Surface& s, const ThemePalette& palette);
    void renderTreePane(Surface& s, const ThemePalette& palette, Rect paneR);
    void renderValuesPane(Surface& s, const ThemePalette& palette, Rect paneR);
    void renderStatusBar(Surface& s, const ThemePalette& palette, Rect barR);
};

} // namespace surshell
