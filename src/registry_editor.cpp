// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/registry_editor.cpp)
// ============================================================================

#include "surshell/registry_editor.hpp"
#include <cstdio>
#include <sstream>
#include <iomanip>

namespace surshell {

std::string RegistryValue::formatDisplayData() const {
    switch (type) {
        case RegType::Sz:
        case RegType::ExpandSz:
            return "\"" + stringData + "\"";
        case RegType::Dword: {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "0x%08x (%u)", dwordData, dwordData);
            return std::string(buf);
        }
        case RegType::Qword: {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "0x%016llx (%llu)",
                          static_cast<unsigned long long>(qwordData),
                          static_cast<unsigned long long>(qwordData));
            return std::string(buf);
        }
        case RegType::Binary: {
            if (binaryData.empty()) return "(zero-length binary value)";
            std::string s;
            for (size_t i = 0; i < binaryData.size(); ++i) {
                if (i > 0) s += " ";
                char b[8];
                std::snprintf(b, sizeof(b), "%02x", binaryData[i]);
                s += b;
            }
            return s;
        }
        case RegType::MultiSz: {
            std::string s;
            for (size_t i = 0; i < multiSzData.size(); ++i) {
                if (i > 0) s += " ";
                s += multiSzData[i];
            }
            return s;
        }
        default:
            return "(value not set)";
    }
}

std::string RegistryNode::fullPath() const {
    if (!parent) return name;
    std::string p = parent->fullPath();
    if (p.empty() || p == "Computer") return name;
    return p + "\\" + name;
}

std::shared_ptr<RegistryNode> RegistryNode::addSubkey(std::string childName) {
    auto child = std::make_shared<RegistryNode>();
    child->name = std::move(childName);
    child->parent = this;
    children.push_back(child);
    return child;
}

void RegistryNode::setValueSz(std::string valName, std::string data) {
    for (auto& v : values) {
        if (v.name == valName) {
            v.type = RegType::Sz;
            v.stringData = std::move(data);
            return;
        }
    }
    values.push_back(RegistryValue{
        .name = std::move(valName),
        .type = RegType::Sz,
        .stringData = std::move(data)
    });
}

void RegistryNode::setValueDword(std::string valName, uint32_t data) {
    for (auto& v : values) {
        if (v.name == valName) {
            v.type = RegType::Dword;
            v.dwordData = data;
            return;
        }
    }
    values.push_back(RegistryValue{
        .name = std::move(valName),
        .type = RegType::Dword,
        .dwordData = data
    });
}

void RegistryNode::setValueQword(std::string valName, uint64_t data) {
    for (auto& v : values) {
        if (v.name == valName) {
            v.type = RegType::Qword;
            v.qwordData = data;
            return;
        }
    }
    values.push_back(RegistryValue{
        .name = std::move(valName),
        .type = RegType::Qword,
        .qwordData = data
    });
}

void RegistryNode::setValueBinary(std::string valName, std::vector<uint8_t> data) {
    for (auto& v : values) {
        if (v.name == valName) {
            v.type = RegType::Binary;
            v.binaryData = std::move(data);
            return;
        }
    }
    values.push_back(RegistryValue{
        .name = std::move(valName),
        .type = RegType::Binary,
        .binaryData = std::move(data)
    });
}

const RegistryValue* RegistryNode::findValue(const std::string& valName) const {
    for (const auto& v : values) {
        if (v.name == valName) return &v;
    }
    return nullptr;
}

RegistryEditorContent::RegistryEditorContent(std::string initialPath) {
    initDefaultDatabase();
    navigateToPath(initialPath);
}

void RegistryEditorContent::initDefaultDatabase() {
    rootNode_ = std::make_shared<RegistryNode>();
    rootNode_->name = "Computer";
    rootNode_->expanded = true;

    // 1. HKEY_CLASSES_ROOT
    auto hkcr = rootNode_->addSubkey("HKEY_CLASSES_ROOT");
    auto dotBat = hkcr->addSubkey(".bat");
    dotBat->setValueSz("(Default)", "batfile");
    auto dotCmd = hkcr->addSubkey(".cmd");
    dotCmd->setValueSz("(Default)", "cmdfile");
    auto dotCpp = hkcr->addSubkey(".cpp");
    dotCpp->setValueSz("(Default)", "cppfile");
    auto dotExe = hkcr->addSubkey(".exe");
    dotExe->setValueSz("(Default)", "exefile");
    dotExe->setValueSz("Content Type", "application/x-msdownload");
    auto dotReg = hkcr->addSubkey(".reg");
    dotReg->setValueSz("(Default)", "regfile");

    // 2. HKEY_CURRENT_USER
    auto hkcu = rootNode_->addSubkey("HKEY_CURRENT_USER");
    hkcu->expanded = true;

    auto hkcuControl = hkcu->addSubkey("Control Panel");
    hkcuControl->expanded = true;

    auto hkcuDesk = hkcuControl->addSubkey("Desktop");
    hkcuDesk->setValueSz("Wallpaper", "MicaGrid.bmp");
    hkcuDesk->setValueSz("WallpaperStyle", "10");
    hkcuDesk->setValueSz("TileWallpaper", "0");
    hkcuDesk->setValueSz("ScreenSaveActive", "1");
    hkcuDesk->setValueSz("ScreenSaveTimeOut", "900");

    auto hkcuPers = hkcuControl->addSubkey("Personalization");
    hkcuPers->setValueSz("ThemeMode", "Dark");
    hkcuPers->setValueDword("AccentColor", 0x00D4FF);
    hkcuPers->setValueDword("TransparencyEffects", 1);
    hkcuPers->setValueDword("SubtleBorders", 1);

    auto hkcuSoft = hkcu->addSubkey("Software");
    hkcuSoft->expanded = true;

    auto hkcuMica = hkcuSoft->addSubkey("MicaNT");
    hkcuMica->expanded = true;

    auto hkcuShell = hkcuMica->addSubkey("SurShell");
    hkcuShell->expanded = true;
    hkcuShell->setValueSz("(Default)", "MicaNT Sovereign Shell Configuration");
    hkcuShell->setValueSz("TaskbarAlignment", "Center");
    hkcuShell->setValueSz("TaskbarStyle", "FloatingIsland");
    hkcuShell->setValueDword("TopBarEnabled", 0);
    hkcuShell->setValueDword("Volume", 85);
    hkcuShell->setValueDword("ClockFormat24H", 0);
    hkcuShell->setValueSz("Version", "2026.1-SOVEREIGN");

    auto hkcuTerm = hkcuMica->addSubkey("WindowsTerminal");
    hkcuTerm->setValueSz("DefaultProfile", "cmd.exe");
    hkcuTerm->setValueDword("FontSize", 14);
    hkcuTerm->setValueDword("CursorBlink", 1);

    // 3. HKEY_LOCAL_MACHINE
    auto hklm = rootNode_->addSubkey("HKEY_LOCAL_MACHINE");
    auto hklmHw = hklm->addSubkey("HARDWARE");
    auto hklmDesc = hklmHw->addSubkey("DESCRIPTION");
    auto hklmSys = hklmDesc->addSubkey("System");
    auto hklmCpu = hklmSys->addSubkey("CentralProcessor");
    auto hklmCpu0 = hklmCpu->addSubkey("0");
    hklmCpu0->setValueSz("ProcessorNameString", "MicaNT Sovereign vCPU @ 3.80 GHz");
    hklmCpu0->setValueSz("Identifier", "x86 Family 6 Model 158 Stepping 10");
    hklmCpu0->setValueSz("VendorIdentifier", "MicaNT GenuineProcessor");
    hklmCpu0->setValueDword("~MHz", 3800);

    auto hklmSoft = hklm->addSubkey("SOFTWARE");
    auto hklmMica = hklmSoft->addSubkey("MicaNT");
    auto hklmVer = hklmMica->addSubkey("CurrentVersion");
    hklmVer->setValueSz("ProductName", "MicaNT Workstation Pro 64-Bit");
    hklmVer->setValueSz("CurrentBuild", "26100");
    hklmVer->setValueSz("CurrentBuildNumber", "26100.1");
    hklmVer->setValueSz("DaveCutlerProvenance", "Clean-Room ISO C++23 Native Executive");
    hklmVer->setValueSz("EnclaveSecurityLevel", "Ring0-CSRSS-Parity");

    auto hklmSysRoot = hklm->addSubkey("SYSTEM");
    auto hklmCcs = hklmSysRoot->addSubkey("CurrentControlSet");
    auto hklmCtrl = hklmCcs->addSubkey("Control");
    auto hklmSm = hklmCtrl->addSubkey("Session Manager");
    hklmSm->setValueSz("BootExecute", "autocheck autochk *");
    hklmSm->setValueDword("ProtectionMode", 1);

    auto hklmSvc = hklmCcs->addSubkey("Services");
    auto hklmSec = hklmSvc->addSubkey("SentinelSec");
    hklmSec->setValueSz("DisplayName", "SentinelSec Core Enclave Defense");
    hklmSec->setValueSz("ImagePath", "C:\\Windows\\System32\\sentinelsec.sys");
    hklmSec->setValueDword("Start", 2);
    hklmSec->setValueDword("Type", 16);

    auto hklmLpc = hklmSvc->addSubkey("SurWinLpc");
    hklmLpc->setValueSz("DisplayName", "Dave Cutler Sovereign LPC Subsystem");
    hklmLpc->setValueSz("PortName", "\\RPC_Control\\SurWinLpc");
    hklmLpc->setValueDword("Start", 1);

    // 4. HKEY_USERS
    auto hku = rootNode_->addSubkey("HKEY_USERS");
    hku->addSubkey(".DEFAULT");
    auto hkuAdmin = hku->addSubkey("S-1-5-21-38401");
    auto hkuEnv = hkuAdmin->addSubkey("Environment");
    hkuEnv->setValueSz("TEMP", "C:\\Users\\admin\\AppData\\Local\\Temp");
    hkuEnv->setValueSz("PATH", "C:\\Windows\\System32;C:\\Windows");

    // 5. HKEY_CURRENT_CONFIG
    auto hkcc = rootNode_->addSubkey("HKEY_CURRENT_CONFIG");
    auto hkccSys = hkcc->addSubkey("System");
    auto hkccCcs = hkccSys->addSubkey("CurrentControlSet");
    auto hkccCtrl = hkccCcs->addSubkey("Control");
    auto hkccPrint = hkccCtrl->addSubkey("Print");
    hkccPrint->addSubkey("Printers");

    selectedNode_ = hkcuShell.get();
    addressBarPath_ = currentPath();
}

RegistryNode* RegistryEditorContent::findNodeByPath(RegistryNode* current, const std::string& path) {
    if (!current) return nullptr;
    if (path.empty() || path == "Computer") return rootNode_.get();

    // Split path tokens
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string item;
    while (std::getline(ss, item, '\\')) {
        if (!item.empty() && item != "Computer") {
            parts.push_back(item);
        }
    }

    RegistryNode* curr = rootNode_.get();
    for (const auto& part : parts) {
        bool found = false;
        for (const auto& child : curr->children) {
            if (child->name == part) {
                curr = child.get();
                found = true;
                break;
            }
        }
        if (!found) return nullptr;
    }
    return curr;
}

void RegistryEditorContent::expandPathToNode(RegistryNode* target) {
    RegistryNode* curr = target;
    while (curr) {
        curr->expanded = true;
        curr = curr->parent;
    }
}

void RegistryEditorContent::navigateToPath(const std::string& path) {
    auto* found = findNodeByPath(rootNode_.get(), path);
    if (found) {
        expandPathToNode(found);
        selectedNode_ = found;
        addressBarPath_ = currentPath();
        pathHistory_.push_back(addressBarPath_);
        selectedValIdx_ = -1;
        if (onPathChanged_) onPathChanged_(addressBarPath_);
    }
}

std::string RegistryEditorContent::currentPath() const {
    if (!selectedNode_ || selectedNode_ == rootNode_.get() || !selectedNode_->parent) {
        return "Computer";
    }
    return "Computer\\" + selectedNode_->fullPath();
}

size_t RegistryEditorContent::keyCount() const {
    return selectedNode_ ? selectedNode_->children.size() : 0;
}

size_t RegistryEditorContent::valueCount() const {
    return selectedNode_ ? selectedNode_->values.size() : 0;
}

const std::vector<RegistryValue>& RegistryEditorContent::currentValues() const {
    static const std::vector<RegistryValue> emptyValues;
    return selectedNode_ ? selectedNode_->values : emptyValues;
}

void RegistryEditorContent::flattenTree(RegistryNode* node, int32_t level, int32_t& curY) {
    if (!node) return;

    VisibleTreeItem item;
    item.node = node;
    item.level = level;
    item.bounds = Rect{0, curY, splitX_, 22};
    const int32_t indentX = 8 + level * 14;
    item.chevronBounds = Rect{indentX, curY + 3, 12, 16};

    visibleTreeItems_.push_back(item);
    curY += 22;

    if (node->expanded) {
        for (const auto& child : node->children) {
            flattenTree(child.get(), level + 1, curY);
        }
    }
}

void RegistryEditorContent::render(Surface& s) {
    const auto& palette = ThemeManager::instance().palette();
    s.clear(Color{12, 16, 24, 255});

    renderMenuBar(s, palette);
    renderAddressBar(s, palette);

    const int32_t paneY = 54;
    const int32_t statusH = 22;
    const int32_t paneH = static_cast<int32_t>(s.height()) - paneY - statusH;
    const int32_t w = static_cast<int32_t>(s.width());

    const Rect treeR{0, paneY, splitX_, paneH};
    const Rect valuesR{splitX_ + 1, paneY, w - splitX_ - 1, paneH};
    splitterBounds_ = Rect{splitX_ - 2, paneY, 5, paneH};

    renderTreePane(s, palette, treeR);
    renderValuesPane(s, palette, valuesR);
    renderStatusBar(s, palette, Rect{0, static_cast<int32_t>(s.height()) - statusH, w, statusH});
}

void RegistryEditorContent::renderMenuBar(Surface& s, const ThemePalette& palette) {
    const int32_t menuH = 22;
    const int32_t w = static_cast<int32_t>(s.width());

    s.fillRect(Rect{0, 0, w, menuH}, Color::fromHex(0x101520));
    s.fillRect(Rect{0, menuH - 1, w, 1}, Color::fromHex(0x243248));

    struct MenuItem {
        std::string label;
        Rect* bounds;
    };
    MenuItem menus[] = {
        {"File",      &menuFileBounds_},
        {"Edit",      &menuEditBounds_},
        {"View",      &menuViewBounds_},
        {"Favorites", &menuFavoritesBounds_},
        {"Help",      &menuHelpBounds_}
    };

    int32_t curX = 12;
    for (auto& m : menus) {
        const int32_t itemW = static_cast<int32_t>(m.label.size() * 8 + 16);
        *m.bounds = Rect{curX, 2, itemW, 18};
        s.drawString(curX + 8, 4, m.label, palette.textSecondary, 1);
        curX += itemW + 4;
    }
}

void RegistryEditorContent::renderAddressBar(Surface& s, const ThemePalette& palette) {
    const int32_t addrY = 22;
    const int32_t addrH = 32;
    const int32_t w = static_cast<int32_t>(s.width());

    s.fillRect(Rect{0, addrY, w, addrH}, Color::fromHex(0x0C101A));
    s.fillRect(Rect{0, addrY + addrH - 1, w, 1}, Color::fromHex(0x283850));

    btnBackBounds_ = Rect{8, addrY + 5, 22, 22};
    btnUpBounds_   = Rect{34, addrY + 5, 22, 22};
    btnRefreshBounds_ = Rect{w - 30, addrY + 5, 22, 22};

    auto drawNavBtn = [&](const Rect& r, IconId icon) {
        s.drawRoundedRect(r, 4, Color::fromHex(0x162030), true);
        s.drawRoundedRect(r, 4, Color::fromHex(0x30425C), false);
        IconRenderer::draw(s, icon, Point{r.x + 3, r.y + 3}, 16, palette.textSecondary);
    };

    drawNavBtn(btnBackBounds_, IconId::NavBack);
    drawNavBtn(btnUpBounds_, IconId::NavUp);
    drawNavBtn(btnRefreshBounds_, IconId::NavRefresh);

    addressBoxBounds_ = Rect{62, addrY + 4, w - 98, 24};
    s.drawRoundedRect(addressBoxBounds_, 4, addressBarFocused_ ? Color::fromHex(0x182436) : Color::fromHex(0x101724), true);
    s.drawRoundedRect(addressBoxBounds_, 4, addressBarFocused_ ? palette.accentColor : Color::fromHex(0x2D3E56), false);

    IconRenderer::draw(s, IconId::Registry, Point{addressBoxBounds_.x + 6, addressBoxBounds_.y + 4}, 16, palette.accentColor);
    s.drawString(addressBoxBounds_.x + 28, addressBoxBounds_.y + 5, addressBarPath_, palette.textPrimary, 1);
}

void RegistryEditorContent::renderTreePane(Surface& s, const ThemePalette& palette, Rect paneR) {
    s.fillRect(paneR, Color::fromHex(0x101520));
    s.fillRect(Rect{paneR.right() - 1, paneR.y, 1, paneR.height}, Color::fromHex(0x243248));

    visibleTreeItems_.clear();
    int32_t curY = paneR.y + 4;
    flattenTree(rootNode_.get(), 0, curY);

    for (const auto& item : visibleTreeItems_) {
        if (item.bounds.bottom() > paneR.bottom()) break;

        const bool isSelected = (item.node == selectedNode_);
        const bool isHovered = (item.node == hoveredNode_);

        if (isSelected) {
            s.drawRoundedRect(Rect{item.bounds.x + 4, item.bounds.y, item.bounds.width - 8, item.bounds.height}, 4,
                              Color::fromRgba(0, 212, 255, 40), true);
            s.drawRoundedRect(Rect{item.bounds.x + 4, item.bounds.y, item.bounds.width - 8, item.bounds.height}, 4,
                              Color::fromRgba(0, 212, 255, 140), false);
        } else if (isHovered) {
            s.drawRoundedRect(Rect{item.bounds.x + 4, item.bounds.y, item.bounds.width - 8, item.bounds.height}, 4,
                              Color::fromRgba(255, 255, 255, 15), true);
        }

        const int32_t indentX = 8 + item.level * 14;

        // Chevron if node has children
        if (!item.node->children.empty()) {
            s.drawString(indentX, item.bounds.y + 3, item.node->expanded ? "v" : ">", palette.textSecondary, 1);
        }

        // Folder or Computer icon
        IconId icon = IconId::Folder;
        if (item.node == rootNode_.get()) {
            icon = IconId::ThisPC;
        } else if (item.node->expanded) {
            icon = IconId::FolderOpen;
        }
        IconRenderer::draw(s, icon, Point{indentX + 12, item.bounds.y + 3}, 16,
                           isSelected ? std::make_optional(palette.accentColor) : std::nullopt);

        // Node label
        s.drawString(indentX + 32, item.bounds.y + 4, item.node->name,
                     isSelected ? Color::fromHex(0xFFFFFF) : palette.textPrimary, 1);
    }
}

void RegistryEditorContent::renderValuesPane(Surface& s, const ThemePalette& palette, Rect paneR) {
    s.fillRect(paneR, Color::fromHex(0x0C121E));

    // Column Headers
    const int32_t headerH = 24;
    const Rect headerR{paneR.x, paneR.y, paneR.width, headerH};
    s.fillRect(headerR, Color::fromHex(0x141C2A));
    s.fillRect(Rect{paneR.x, paneR.y + headerH - 1, paneR.width, 1}, Color::fromHex(0x283850));

    const int32_t colNameW = 220;
    const int32_t colTypeW = 120;

    s.drawString(paneR.x + 12, paneR.y + 5, "Name", palette.textPrimary, 1);
    s.fillRect(Rect{paneR.x + colNameW, paneR.y + 3, 1, headerH - 6}, Color::fromHex(0x283850));

    s.drawString(paneR.x + colNameW + 12, paneR.y + 5, "Type", palette.textPrimary, 1);
    s.fillRect(Rect{paneR.x + colNameW + colTypeW, paneR.y + 3, 1, headerH - 6}, Color::fromHex(0x283850));

    s.drawString(paneR.x + colNameW + colTypeW + 12, paneR.y + 5, "Data", palette.textPrimary, 1);

    visibleValueItems_.clear();
    if (!selectedNode_) return;

    int32_t curY = paneR.y + headerH + 4;
    for (size_t i = 0; i < selectedNode_->values.size(); ++i) {
        if (curY + 22 > paneR.bottom()) break;

        const auto& val = selectedNode_->values[i];
        const Rect rowR{paneR.x + 4, curY, paneR.width - 8, 22};

        VisibleValueItem vItem;
        vItem.index = i;
        vItem.bounds = rowR;
        visibleValueItems_.push_back(vItem);

        const bool isSelected = (selectedValIdx_ == static_cast<int32_t>(i));
        const bool isHovered = (hoveredValIdx_ == static_cast<int32_t>(i));

        if (isSelected) {
            s.drawRoundedRect(rowR, 4, Color::fromRgba(0, 212, 255, 40), true);
            s.drawRoundedRect(rowR, 4, Color::fromRgba(0, 212, 255, 140), false);
        } else if (isHovered) {
            s.drawRoundedRect(rowR, 4, Color::fromRgba(255, 255, 255, 15), true);
        }

        // Small type glyph: [ab] for string, [01] for binary/dword
        const bool isString = (val.type == RegType::Sz || val.type == RegType::ExpandSz || val.type == RegType::MultiSz);
        IconId valIcon = isString ? IconId::FileText : IconId::FileCode;
        IconRenderer::draw(s, valIcon, Point{rowR.x + 8, rowR.y + 3}, 16,
                           isSelected ? palette.accentColor : (isString ? Color::fromHex(0xFF4D6D) : Color::fromHex(0x00FF9D)));

        // Name with safe column boundary
        std::string dispName = val.name;
        const int32_t maxNameChars = (colNameW - 36) / 8;
        if (maxNameChars > 4 && static_cast<int32_t>(dispName.size()) > maxNameChars) {
            dispName = dispName.substr(0, static_cast<size_t>(maxNameChars - 2)) + "..";
        }
        s.drawString(rowR.x + 28, rowR.y + 4, dispName,
                     isSelected ? Color::fromHex(0xFFFFFF) : palette.textPrimary, 1);

        // Type
        s.drawString(paneR.x + colNameW + 12, rowR.y + 4, regTypeToString(val.type),
                     palette.textSecondary, 1);

        // Data with safe column boundary
        std::string dispData = val.formatDisplayData();
        const int32_t maxDataChars = (paneR.width - colNameW - colTypeW - 20) / 8;
        if (maxDataChars > 4 && static_cast<int32_t>(dispData.size()) > maxDataChars) {
            dispData = dispData.substr(0, static_cast<size_t>(maxDataChars - 2)) + "..";
        }
        s.drawString(paneR.x + colNameW + colTypeW + 12, rowR.y + 4, dispData,
                     isSelected ? Color::fromHex(0x00D4FF) : palette.textPrimary, 1);

        curY += 22;
    }
}

void RegistryEditorContent::renderStatusBar(Surface& s, const ThemePalette& palette, Rect barR) {
    s.fillRect(barR, Color::fromHex(0x0A0E18));
    s.fillRect(Rect{barR.x, barR.y, barR.width, 1}, Color::fromHex(0x1E293B));

    // Full key path on left in Cutler Cyan
    s.drawString(barR.x + 10, barR.y + 4, addressBarPath_, Color::fromHex(0x00D4FF), 1);

    // Value count on right
    std::string countStr = std::to_string(valueCount()) + " value(s)";
    const int32_t textW = static_cast<int32_t>(countStr.size() * 8);
    s.drawString(barR.right() - textW - 14, barR.y + 4, countStr, palette.textSecondary, 1);
}

bool RegistryEditorContent::onMouseDown(Point localPt, MouseButton button) {
    if (button != MouseButton::Left) return false;

    // 1. Navigation buttons
    if (btnBackBounds_.contains(localPt)) {
        if (pathHistory_.size() > 1) {
            pathHistory_.pop_back();
            navigateToPath(pathHistory_.back());
            pathHistory_.pop_back(); // Don't duplicate in history
        }
        return true;
    }
    if (btnUpBounds_.contains(localPt)) {
        if (selectedNode_ && selectedNode_->parent) {
            selectedNode_ = selectedNode_->parent;
            addressBarPath_ = currentPath();
            selectedValIdx_ = -1;
            if (onPathChanged_) onPathChanged_(addressBarPath_);
        }
        return true;
    }
    if (btnRefreshBounds_.contains(localPt)) {
        return true;
    }
    if (addressBoxBounds_.contains(localPt)) {
        addressBarFocused_ = true;
        return true;
    } else {
        addressBarFocused_ = false;
    }

    // 2. Tree Pane Items
    for (const auto& item : visibleTreeItems_) {
        if (item.chevronBounds.contains(localPt)) {
            item.node->expanded = !item.node->expanded;
            return true;
        }
        if (item.bounds.contains(localPt)) {
            selectedNode_ = item.node;
            addressBarPath_ = currentPath();
            selectedValIdx_ = -1;
            if (onPathChanged_) onPathChanged_(addressBarPath_);
            return true;
        }
    }

    // 3. Value Pane Items
    for (const auto& vItem : visibleValueItems_) {
        if (vItem.bounds.contains(localPt)) {
            selectedValIdx_ = static_cast<int32_t>(vItem.index);
            return true;
        }
    }

    // 4. Splitter drag start
    if (splitterBounds_.contains(localPt)) {
        draggingSplitter_ = true;
        return true;
    }

    return false;
}

bool RegistryEditorContent::onMouseMove(Point localPt) {
    if (draggingSplitter_) {
        splitX_ = std::clamp(localPt.x, 150, 480);
        return true;
    }

    hoveredNode_ = nullptr;
    for (const auto& item : visibleTreeItems_) {
        if (item.bounds.contains(localPt)) {
            hoveredNode_ = item.node;
            break;
        }
    }

    hoveredValIdx_ = -1;
    for (const auto& vItem : visibleValueItems_) {
        if (vItem.bounds.contains(localPt)) {
            hoveredValIdx_ = static_cast<int32_t>(vItem.index);
            break;
        }
    }

    return false;
}

bool RegistryEditorContent::onDoubleClick(Point localPt) {
    for (const auto& vItem : visibleValueItems_) {
        if (vItem.bounds.contains(localPt)) {
            if (selectedNode_ && vItem.index < selectedNode_->values.size()) {
                auto& val = selectedNode_->values[vItem.index];
                if (val.type == RegType::Dword) {
                    val.dwordData = val.dwordData ? 0 : 1; // Quick toggle
                    if (onValueModified_) onValueModified_(currentPath(), val.name);
                    return true;
                }
            }
        }
    }
    return false;
}

bool RegistryEditorContent::onMouseWheel(Point localPt, int32_t delta) {
    (void)localPt;
    (void)delta;
    return true;
}

bool RegistryEditorContent::onCharInput(char c) {
    if (!addressBarFocused_) return false;
    if (c >= 32 && c <= 126) {
        addressBarPath_.push_back(c);
        return true;
    }
    return false;
}

bool RegistryEditorContent::onKeyDown(KeyCode key, bool ctrl, bool shift, bool alt) {
    (void)ctrl; (void)shift; (void)alt;
    if (addressBarFocused_) {
        if (key == KeyCode::Backspace && !addressBarPath_.empty()) {
            addressBarPath_.pop_back();
            return true;
        } else if (key == KeyCode::Enter) {
            navigateToPath(addressBarPath_);
            addressBarFocused_ = false;
            return true;
        } else if (key == KeyCode::Escape) {
            addressBarPath_ = currentPath();
            addressBarFocused_ = false;
            return true;
        }
    } else {
        if (key == KeyCode::Up && selectedNode_ && selectedNode_->parent) {
            selectedNode_ = selectedNode_->parent;
            addressBarPath_ = currentPath();
            return true;
        }
    }
    return false;
}

} // namespace surshell
