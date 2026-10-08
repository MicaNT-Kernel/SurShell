// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/icons.hpp)
//
// Sovereign Procedural Vector Icon Engine (PrismIconPack)
// 100% clean-room mathematical geometry, multi-DPI scalable (16/24/32/48px),
// zero external PNG/ICO dependencies, and sub-microsecond software rasterization.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include <string_view>
#include <optional>

namespace surshell {

enum class IconId {
    // System Shell & Hardware
    StartPrism,
    ThisPC,
    LocalDisk,
    DriveStorage,
    Terminal,
    Settings,
    TaskManager,
    SentinelSec,
    NetBirdMesh,
    FileExplorer,
    TaskView,

    // File System & Documents
    Folder,
    FolderOpen,
    FileGeneric,
    FileText,
    FileCode,
    FileExecutable,
    FileLibrary,
    FileImage,
    FileArchive,

    // Explorer Actions & Navigation
    NavBack,
    NavForward,
    NavUp,
    NavRefresh,
    Search,
    NewFolder,
    NewFile,
    Cut,
    Copy,
    Paste,
    Rename,
    Delete,
    Edit,
    Properties,
    ViewList,
    ViewGrid,
    SortAsc,
    SortDesc,

    // Tray & Status
    VolumeHigh,
    VolumeMute,
    BatteryCharging,
    NetworkOnline,
    Clock,

    // Power & Session
    Power,
    Restart,
    Sleep,
    Lock,
    SignOut,
    User,
    Hibernate,

    // Media & Playback
    MediaPlay,
    MediaPause,
    MediaNext,
    MediaPrev,

    // Notifications & Hardware
    NotificationBell,
    NetworkEthernet,

    // Productivity & System Dialogs
    Calculator,
    RunDialog,
    Display,
    Desktop,
    Personalization,
    Calendar,
    TerminalTab,
    ShieldAdmin,
    SearchCategory,
    Registry,
    ImageViewer,

    // Storage, Cloud & Infrastructure
    CloudDrive,
    NetworkShare,
    OpticalDrive
};

class IconRenderer {
public:
    // Core procedural draw calls
    static void draw(Surface& surface, IconId id, Point pt, int32_t size = 16, std::optional<Color> tintOverride = std::nullopt);
    static void draw(Surface& surface, IconId id, Rect bounds, std::optional<Color> tintOverride = std::nullopt);

    // Helpers to resolve IconId from extensions or app IDs
    [[nodiscard]] static IconId iconForExtension(std::string_view extension, bool isDirectory = false);
    [[nodiscard]] static IconId iconForAppId(std::string_view appId);

private:
    static void drawThisPC(Surface& s, Rect r, std::optional<Color> tint);
    static void drawLocalDisk(Surface& s, Rect r, std::optional<Color> tint);
    static void drawDriveStorage(Surface& s, Rect r, std::optional<Color> tint);
    static void drawTerminal(Surface& s, Rect r, std::optional<Color> tint);
    static void drawSettings(Surface& s, Rect r, std::optional<Color> tint);
    static void drawTaskManager(Surface& s, Rect r, std::optional<Color> tint);
    static void drawSentinelSec(Surface& s, Rect r, std::optional<Color> tint);
    static void drawNetBirdMesh(Surface& s, Rect r, std::optional<Color> tint);
    static void drawTaskView(Surface& s, Rect r, std::optional<Color> tint);
    static void drawFolder(Surface& s, Rect r, bool open, std::optional<Color> tint);
    static void drawDocument(Surface& s, Rect r, IconId docType, std::optional<Color> tint);
    static void drawNavChevron(Surface& s, Rect r, int32_t dir, std::optional<Color> tint); // 0=Back, 1=Forward, 2=Up
    static void drawNavRefresh(Surface& s, Rect r, std::optional<Color> tint);
    static void drawSearch(Surface& s, Rect r, std::optional<Color> tint);
    static void drawNewFolder(Surface& s, Rect r, std::optional<Color> tint);
    static void drawNewFile(Surface& s, Rect r, std::optional<Color> tint);
    static void drawCut(Surface& s, Rect r, std::optional<Color> tint);
    static void drawCopy(Surface& s, Rect r, std::optional<Color> tint);
    static void drawPaste(Surface& s, Rect r, std::optional<Color> tint);
    static void drawRename(Surface& s, Rect r, std::optional<Color> tint);
    static void drawDelete(Surface& s, Rect r, std::optional<Color> tint);
    static void drawProperties(Surface& s, Rect r, std::optional<Color> tint);
    static void drawViewList(Surface& s, Rect r, std::optional<Color> tint);
    static void drawViewGrid(Surface& s, Rect r, std::optional<Color> tint);
    static void drawEdit(Surface& s, Rect r, std::optional<Color> tint);
    static void drawSort(Surface& s, Rect r, bool asc, std::optional<Color> tint);
    static void drawVolume(Surface& s, Rect r, bool mute, std::optional<Color> tint);
    static void drawBattery(Surface& s, Rect r, std::optional<Color> tint);
    static void drawNetwork(Surface& s, Rect r, std::optional<Color> tint);
    static void drawClock(Surface& s, Rect r, std::optional<Color> tint);
    static void drawPower(Surface& s, Rect r, std::optional<Color> tint);
    static void drawRestart(Surface& s, Rect r, std::optional<Color> tint);
    static void drawSleep(Surface& s, Rect r, std::optional<Color> tint);
    static void drawLock(Surface& s, Rect r, std::optional<Color> tint);
    static void drawSignOut(Surface& s, Rect r, std::optional<Color> tint);
    static void drawUser(Surface& s, Rect r, std::optional<Color> tint);
    static void drawHibernate(Surface& s, Rect r, std::optional<Color> tint);
    static void drawMediaPlay(Surface& s, Rect r, std::optional<Color> tint);
    static void drawMediaPause(Surface& s, Rect r, std::optional<Color> tint);
    static void drawMediaNext(Surface& s, Rect r, std::optional<Color> tint);
    static void drawMediaPrev(Surface& s, Rect r, std::optional<Color> tint);
    static void drawNotificationBell(Surface& s, Rect r, std::optional<Color> tint);
    static void drawNetworkEthernet(Surface& s, Rect r, std::optional<Color> tint);
    static void drawCalculator(Surface& s, Rect r, std::optional<Color> tint);
    static void drawRunDialog(Surface& s, Rect r, std::optional<Color> tint);
    static void drawDisplay(Surface& s, Rect r, std::optional<Color> tint);
    static void drawPersonalization(Surface& s, Rect r, std::optional<Color> tint);
    static void drawCalendar(Surface& s, Rect r, std::optional<Color> tint);
    static void drawTerminalTab(Surface& s, Rect r, std::optional<Color> tint);
    static void drawShieldAdmin(Surface& s, Rect r, std::optional<Color> tint);
    static void drawSearchCategory(Surface& s, Rect r, std::optional<Color> tint);
    static void drawRegistry(Surface& s, Rect r, std::optional<Color> tint);
    static void drawImageViewer(Surface& s, Rect r, std::optional<Color> tint);
    static void drawCloudDrive(Surface& s, Rect r, std::optional<Color> tint);
    static void drawNetworkShare(Surface& s, Rect r, std::optional<Color> tint);
    static void drawOpticalDrive(Surface& s, Rect r, std::optional<Color> tint);
};

} // namespace surshell
