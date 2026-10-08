// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (src/icons.cpp)
// ============================================================================

#include "surshell/icons.hpp"
#include <algorithm>
#include <cmath>

namespace surshell {

void IconRenderer::draw(Surface& surface, IconId id, Point pt, int32_t size, std::optional<Color> tintOverride) {
    draw(surface, id, Rect{pt.x, pt.y, size, size}, tintOverride);
}

void IconRenderer::draw(Surface& surface, IconId id, Rect bounds, std::optional<Color> tintOverride) {
    if (bounds.empty()) return;

    switch (id) {
        case IconId::StartPrism: {
            const int32_t cx = bounds.centerX();
            const int32_t cy = bounds.centerY();
            const int32_t r = std::min(bounds.width, bounds.height) / 2 - 1;
            surface.drawPrismLogo(Point{cx, cy}, r,
                tintOverride.value_or(Color::fromHex(0x00D4FF)),
                Color::fromHex(0x005580),
                Color::fromHex(0x80EAFF));
            break;
        }
        case IconId::ThisPC:
            drawThisPC(surface, bounds, tintOverride);
            break;
        case IconId::LocalDisk:
            drawLocalDisk(surface, bounds, tintOverride);
            break;
        case IconId::DriveStorage:
            drawDriveStorage(surface, bounds, tintOverride);
            break;
        case IconId::Terminal:
            drawTerminal(surface, bounds, tintOverride);
            break;
        case IconId::Settings:
            drawSettings(surface, bounds, tintOverride);
            break;
        case IconId::TaskManager:
            drawTaskManager(surface, bounds, tintOverride);
            break;
        case IconId::SentinelSec:
            drawSentinelSec(surface, bounds, tintOverride);
            break;
        case IconId::NetBirdMesh:
            drawNetBirdMesh(surface, bounds, tintOverride);
            break;
        case IconId::TaskView:
            drawTaskView(surface, bounds, tintOverride);
            break;
        case IconId::FileExplorer:
        case IconId::Folder:
            drawFolder(surface, bounds, false, tintOverride);
            break;
        case IconId::FolderOpen:
            drawFolder(surface, bounds, true, tintOverride);
            break;
        case IconId::FileGeneric:
        case IconId::FileText:
        case IconId::FileCode:
        case IconId::FileExecutable:
        case IconId::FileLibrary:
        case IconId::FileImage:
        case IconId::FileArchive:
            drawDocument(surface, bounds, id, tintOverride);
            break;
        case IconId::NavBack:
            drawNavChevron(surface, bounds, 0, tintOverride);
            break;
        case IconId::NavForward:
            drawNavChevron(surface, bounds, 1, tintOverride);
            break;
        case IconId::NavUp:
            drawNavChevron(surface, bounds, 2, tintOverride);
            break;
        case IconId::NavRefresh:
            drawNavRefresh(surface, bounds, tintOverride);
            break;
        case IconId::Search:
            drawSearch(surface, bounds, tintOverride);
            break;
        case IconId::NewFolder:
            drawNewFolder(surface, bounds, tintOverride);
            break;
        case IconId::NewFile:
            drawNewFile(surface, bounds, tintOverride);
            break;
        case IconId::Cut:
            drawCut(surface, bounds, tintOverride);
            break;
        case IconId::Paste:
            drawPaste(surface, bounds, tintOverride);
            break;
        case IconId::Rename:
            drawRename(surface, bounds, tintOverride);
            break;
        case IconId::Delete:
            drawDelete(surface, bounds, tintOverride);
            break;
        case IconId::Properties:
            drawProperties(surface, bounds, tintOverride);
            break;
        case IconId::ViewList:
            drawViewList(surface, bounds, tintOverride);
            break;
        case IconId::ViewGrid:
            drawViewGrid(surface, bounds, tintOverride);
            break;
        case IconId::Edit:
            drawEdit(surface, bounds, tintOverride);
            break;
        case IconId::Copy:
            drawCopy(surface, bounds, tintOverride);
            break;
        case IconId::SortAsc:
            drawSort(surface, bounds, true, tintOverride);
            break;
        case IconId::SortDesc:
            drawSort(surface, bounds, false, tintOverride);
            break;
        case IconId::VolumeHigh:
            drawVolume(surface, bounds, false, tintOverride);
            break;
        case IconId::VolumeMute:
            drawVolume(surface, bounds, true, tintOverride);
            break;
        case IconId::BatteryCharging:
            drawBattery(surface, bounds, tintOverride);
            break;
        case IconId::NetworkOnline:
            drawNetwork(surface, bounds, tintOverride);
            break;
        case IconId::Clock:
            drawClock(surface, bounds, tintOverride);
            break;
        case IconId::Power:
            drawPower(surface, bounds, tintOverride);
            break;
        case IconId::Restart:
            drawRestart(surface, bounds, tintOverride);
            break;
        case IconId::Sleep:
            drawSleep(surface, bounds, tintOverride);
            break;
        case IconId::Lock:
            drawLock(surface, bounds, tintOverride);
            break;
        case IconId::SignOut:
            drawSignOut(surface, bounds, tintOverride);
            break;
        case IconId::User:
            drawUser(surface, bounds, tintOverride);
            break;
        case IconId::Hibernate:
            drawHibernate(surface, bounds, tintOverride);
            break;
        case IconId::MediaPlay:
            drawMediaPlay(surface, bounds, tintOverride);
            break;
        case IconId::MediaPause:
            drawMediaPause(surface, bounds, tintOverride);
            break;
        case IconId::MediaNext:
            drawMediaNext(surface, bounds, tintOverride);
            break;
        case IconId::MediaPrev:
            drawMediaPrev(surface, bounds, tintOverride);
            break;
        case IconId::NotificationBell:
            drawNotificationBell(surface, bounds, tintOverride);
            break;
        case IconId::NetworkEthernet:
            drawNetworkEthernet(surface, bounds, tintOverride);
            break;
        case IconId::Calculator:
            drawCalculator(surface, bounds, tintOverride);
            break;
        case IconId::RunDialog:
            drawRunDialog(surface, bounds, tintOverride);
            break;
        case IconId::Display:
        case IconId::Desktop:
            drawDisplay(surface, bounds, tintOverride);
            break;
        case IconId::Personalization:
            drawPersonalization(surface, bounds, tintOverride);
            break;
        case IconId::Calendar:
            drawCalendar(surface, bounds, tintOverride);
            break;
        case IconId::TerminalTab:
            drawTerminalTab(surface, bounds, tintOverride);
            break;
        case IconId::ShieldAdmin:
            drawShieldAdmin(surface, bounds, tintOverride);
            break;
        case IconId::SearchCategory:
            drawSearchCategory(surface, bounds, tintOverride);
            break;
        case IconId::Registry:
            drawRegistry(surface, bounds, tintOverride);
            break;
        case IconId::CloudDrive:
            drawCloudDrive(surface, bounds, tintOverride);
            break;
        case IconId::NetworkShare:
            drawNetworkShare(surface, bounds, tintOverride);
            break;
        case IconId::OpticalDrive:
            drawOpticalDrive(surface, bounds, tintOverride);
            break;
        default:
            drawDocument(surface, bounds, IconId::FileGeneric, tintOverride);
            break;
    }
}

IconId IconRenderer::iconForExtension(std::string_view ext, bool isDirectory) {
    if (isDirectory) return IconId::Folder;
    if (ext == ".exe" || ext == ".bat" || ext == ".cmd") return IconId::FileExecutable;
    if (ext == ".dll" || ext == ".sys") return IconId::FileLibrary;
    if (ext == ".cpp" || ext == ".hpp" || ext == ".c" || ext == ".h" || ext == ".py" || ext == ".rs" || ext == ".asm") return IconId::FileCode;
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".ico") return IconId::FileImage;
    if (ext == ".zip" || ext == ".tar" || ext == ".gz" || ext == ".7z" || ext == ".rar") return IconId::FileArchive;
    if (ext == ".txt" || ext == ".md" || ext == ".log" || ext == ".ini" || ext == ".json" || ext == ".xml") return IconId::FileText;
    if (ext == ".iso" || ext == ".img" || ext == ".vhd") return IconId::OpticalDrive;
    if (ext == ".gdoc" || ext == ".gsheet" || ext == ".gslides") return IconId::CloudDrive;
    return IconId::FileGeneric;
}

IconId IconRenderer::iconForAppId(std::string_view appId) {
    if (appId == "this_pc") return IconId::ThisPC;
    if (appId == "explorer") return IconId::FileExplorer;
    if (appId == "cmd" || appId == "terminal") return IconId::Terminal;
    if (appId == "settings") return IconId::Settings;
    if (appId == "taskmgr") return IconId::TaskManager;
    if (appId == "sentinel") return IconId::SentinelSec;
    if (appId == "network") return IconId::NetworkOnline;
    if (appId == "mediaplayer") return IconId::MediaPlay;
    if (appId == "calc" || appId == "calculator") return IconId::Calculator;
    if (appId == "run" || appId == "rundll32") return IconId::RunDialog;
    if (appId == "display") return IconId::Display;
    if (appId == "personalize" || appId == "themes") return IconId::Personalization;
    if (appId == "calendar" || appId == "clock") return IconId::Calendar;
    if (appId == "search" || appId == "find") return IconId::Search;
    if (appId == "admin") return IconId::ShieldAdmin;
    if (appId == "regedit" || appId == "registry") return IconId::Registry;
    if (appId == "cloud" || appId == "gdrive" || appId == "google_drive") return IconId::CloudDrive;
    if (appId == "nas" || appId == "share" || appId == "storage") return IconId::NetworkShare;
    if (appId == "cd" || appId == "dvd" || appId == "optical") return IconId::OpticalDrive;
    return IconId::StartPrism;
}

// ----------------------------------------------------------------------------
// Procedural Rasterization Routines
// ----------------------------------------------------------------------------

void IconRenderer::drawThisPC(Surface& s, Rect r, std::optional<Color> tint) {
    const int32_t w = r.width;
    const int32_t h = r.height;
    const int32_t standH = std::max(2, h / 5);
    const int32_t screenH = h - standH - 1;

    // Monitor outer frame
    const Color frameCol = tint.value_or(Color::fromHex(0x4A6082));
    s.drawRoundedRect(Rect{r.x, r.y, w, screenH}, std::max(2, w / 8), frameCol, true);

    // Screen interior
    const int32_t bezel = std::max(1, w / 12);
    const Rect screenArea{r.x + bezel, r.y + bezel, w - bezel * 2, screenH - bezel * 2};
    s.fillRect(screenArea, Color::fromHex(0x0C1422));

    // Screen horizon wallpaper glow
    const int32_t glowH = std::max(2, screenArea.height / 3);
    s.fillRect(Rect{screenArea.x, screenArea.bottom() - glowH, screenArea.width, glowH}, Color::fromRgba(0, 212, 255, 90));

    // Stand neck
    const int32_t neckW = std::max(2, w / 6);
    s.fillRect(Rect{r.centerX() - neckW / 2, r.y + screenH, neckW, standH - 1}, frameCol);

    // Stand base
    const int32_t baseW = std::max(4, w / 2);
    s.drawRoundedRect(Rect{r.centerX() - baseW / 2, r.bottom() - 2, baseW, 2}, 1, frameCol, true);
}

void IconRenderer::drawLocalDisk(Surface& s, Rect r, std::optional<Color> tint) {
    const Color chassisCol = tint.value_or(Color::fromHex(0x384964));
    s.drawRoundedRect(r, std::max(2, r.width / 6), chassisCol, true);
    s.drawRoundedRect(r, std::max(2, r.width / 6), Color::fromRgba(255, 255, 255, 60), false);

    // Platter ring
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY() - 1;
    const int32_t rad = std::max(2, r.width / 3);
    s.drawRoundedRect(Rect{cx - rad, cy - rad, rad * 2, rad * 2}, rad, Color::fromHex(0x547094), true);
    s.drawRoundedRect(Rect{cx - rad / 2, cy - rad / 2, rad, rad}, rad / 2, Color::fromHex(0x283548), true);

    // Actuator Arm
    s.fillRect(Rect{cx, cy, std::max(2, rad / 2), 1}, Color::fromHex(0xC0D0E4));

    // LED status dot
    s.putPixel(r.right() - 3, r.bottom() - 3, Color::fromHex(0x00FF9D));
}

void IconRenderer::drawDriveStorage(Surface& s, Rect r, std::optional<Color> tint) {
    const Color bodyCol = tint.value_or(Color::fromHex(0x24334A));
    s.drawRoundedRect(r, std::max(2, r.width / 6), bodyCol, true);
    s.drawRoundedRect(r, std::max(2, r.width / 6), Color::fromHex(0x405B85), false);

    // Dual bus stripes
    const int32_t stripeY = r.centerY() - 2;
    s.fillRect(Rect{r.x + 3, stripeY, r.width - 6, 2}, Color::fromHex(0x00D4FF));
    s.fillRect(Rect{r.x + 3, stripeY + 4, r.width - 6, 2}, Color::fromRgba(0, 212, 255, 120));
}

void IconRenderer::drawTerminal(Surface& s, Rect r, std::optional<Color> tint) {
    const Color bezelCol = tint.value_or(Color::fromHex(0x101622));
    s.drawRoundedRect(r, std::max(2, r.width / 6), bezelCol, true);
    s.drawRoundedRect(r, std::max(2, r.width / 6), Color::fromHex(0x00D4FF), false);

    // Screen area
    const int32_t pad = std::max(1, r.width / 8);
    const Rect scr{r.x + pad, r.y + pad, r.width - pad * 2, r.height - pad * 2};
    s.fillRect(scr, Color::fromHex(0x06090F));

    // Header strip
    s.fillRect(Rect{scr.x, scr.y, scr.width, std::max(2, scr.height / 5)}, Color::fromHex(0x1A2536));

    // Prompt >_
    if (r.width >= 20) {
        s.drawString(scr.x + 2, scr.y + 4, ">_", Color::fromHex(0x00FF9D), 1);
    } else {
        // Pixel prompt for small 16px size
        s.putPixel(scr.x + 2, scr.y + scr.height / 2, Color::fromHex(0x00FF9D));
        s.putPixel(scr.x + 3, scr.y + scr.height / 2 + 1, Color::fromHex(0x00FF9D));
        s.putPixel(scr.x + 2, scr.y + scr.height / 2 + 2, Color::fromHex(0x00FF9D));
        s.fillRect(Rect{scr.x + 5, scr.y + scr.height / 2 + 2, 2, 1}, Color::fromHex(0x00D4FF));
    }
}

void IconRenderer::drawSettings(Surface& s, Rect r, std::optional<Color> tint) {
    const Color gearCol = tint.value_or(Color::fromHex(0x94A8C4));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();
    const int32_t rad = std::max(3, r.width / 2 - 2);

    // Body disc
    s.drawRoundedRect(Rect{cx - rad, cy - rad, rad * 2, rad * 2}, rad, gearCol, true);

    // 4 cardinal teeth
    const int32_t toothW = std::max(2, rad / 2);
    s.fillRect(Rect{cx - toothW / 2, cy - rad - 2, toothW, 3}, gearCol);
    s.fillRect(Rect{cx - toothW / 2, cy + rad - 1, toothW, 3}, gearCol);
    s.fillRect(Rect{cx - rad - 2, cy - toothW / 2, 3, toothW}, gearCol);
    s.fillRect(Rect{cx + rad - 1, cy - toothW / 2, 3, toothW}, gearCol);

    // Central hole
    const int32_t holeR = std::max(1, rad / 2);
    s.drawRoundedRect(Rect{cx - holeR, cy - holeR, holeR * 2, holeR * 2}, holeR, Color::fromHex(0x0E1420), true);
}

void IconRenderer::drawTaskManager(Surface& s, Rect r, std::optional<Color> tint) {
    s.drawRoundedRect(r, std::max(2, r.width / 6), Color::fromHex(0x121A28), true);
    s.drawRoundedRect(r, std::max(2, r.width / 6), Color::fromHex(0x364E72), false);

    // Pulse wave line in Cutler Cyan
    const Color pulseCol = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t midY = r.centerY();
    const int32_t w = r.width - 4;
    const int32_t x0 = r.x + 2;

    for (int32_t i = 0; i < w; ++i) {
        int32_t py = midY;
        if (i == w * 3 / 8) py -= std::max(2, r.height / 4);
        else if (i == w * 4 / 8) py += std::max(2, r.height / 3);
        else if (i == w * 5 / 8) py -= std::max(3, r.height * 2 / 5);
        else if (i == w * 6 / 8) py += std::max(1, r.height / 6);
        s.putPixel(x0 + i, py, pulseCol);
    }
}

void IconRenderer::drawSentinelSec(Surface& s, Rect r, std::optional<Color> tint) {
    const Color shieldCol = tint.value_or(Color::fromHex(0x00FF9D));
    const int32_t cx = r.centerX();
    const int32_t top = r.y + 1;
    const int32_t bottom = r.bottom() - 1;
    const int32_t h = bottom - top;

    for (int32_t y = top; y <= bottom; ++y) {
        const float t = static_cast<float>(y - top) / static_cast<float>(std::max(1, h));
        const int32_t halfW = static_cast<int32_t>(static_cast<float>(r.width / 2 - 1) * (1.0f - t * t * 0.75f));
        for (int32_t x = cx - halfW; x <= cx + halfW; ++x) {
            s.putPixel(x, y, shieldCol);
        }
    }

    // Inner checkmark / core
    s.fillRect(Rect{cx - 1, top + h / 3, 3, std::max(2, h / 3)}, Color::fromHex(0x082618));
}

void IconRenderer::drawNetBirdMesh(Surface& s, Rect r, std::optional<Color> tint) {
    const Color nodeCol = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();
    const int32_t n1x = cx, n1y = r.y + 2;
    const int32_t n2x = r.x + 3, n2y = r.bottom() - 3;
    const int32_t n3x = r.right() - 4, n3y = r.bottom() - 3;

    // Bus links
    for (int32_t i = 0; i <= 8; ++i) {
        const float t = static_cast<float>(i) / 8.0f;
        s.putPixel(static_cast<int32_t>(n1x + (n2x - n1x) * t), static_cast<int32_t>(n1y + (n2y - n1y) * t), Color::fromRgba(0, 212, 255, 120));
        s.putPixel(static_cast<int32_t>(n1x + (n3x - n1x) * t), static_cast<int32_t>(n1y + (n3y - n1y) * t), Color::fromRgba(0, 212, 255, 120));
        s.putPixel(static_cast<int32_t>(n2x + (n3x - n2x) * t), static_cast<int32_t>(n2y + (n3y - n2y) * t), Color::fromRgba(0, 212, 255, 120));
    }

    // Nodes
    const int32_t nSize = std::max(2, r.width / 6);
    s.fillRect(Rect{n1x - nSize / 2, n1y - nSize / 2, nSize, nSize}, nodeCol);
    s.fillRect(Rect{n2x - nSize / 2, n2y - nSize / 2, nSize, nSize}, nodeCol);
    s.fillRect(Rect{n3x - nSize / 2, n3y - nSize / 2, nSize, nSize}, nodeCol);
    s.fillRect(Rect{cx - nSize / 2, cy - nSize / 2, nSize, nSize}, Color::fromHex(0xFFFFFF));
}

void IconRenderer::drawTaskView(Surface& s, Rect r, std::optional<Color> tint) {
    const Color accent = tint.value_or(Color::fromHex(0x00D4FF));
    const Color backCol = Color::fromHex(0x566B88);
    const int32_t mw = std::max(6, r.width * 60 / 100);
    const int32_t mh = std::max(5, r.height * 55 / 100);

    // Back desktop screen (offset top-left)
    const Rect backRect{r.x + 1, r.y + 1, mw, mh};
    s.drawRoundedRect(backRect, 2, Color::fromHex(0x121A28), true);
    s.drawRoundedRect(backRect, 2, backCol, false);

    // Front desktop screen (offset bottom-right)
    const Rect frontRect{r.right() - mw - 1, r.bottom() - mh - 1, mw, mh};
    s.drawRoundedRect(frontRect, 2, Color::fromHex(0x182438), true);
    s.drawRoundedRect(frontRect, 2, accent, false);

    // Inner mini-window wash and dock indicator on front screen
    const int32_t innerW = std::max(2, mw - 4);
    const int32_t innerH = std::max(2, mh - 6);
    s.fillRect(Rect{frontRect.x + 2, frontRect.y + 2, innerW, innerH}, Color::fromRgba(0, 212, 255, 60));
    s.fillRect(Rect{frontRect.x + mw / 4, frontRect.bottom() - 2, mw / 2, 1}, accent);
}

void IconRenderer::drawFolder(Surface& s, Rect r, bool open, std::optional<Color> tint) {
    const Color tabCol = tint.value_or(Color::fromHex(0xE09E05));
    const Color bodyCol = tint.value_or(Color::fromHex(0xF5B418));
    const Color flapCol = tint.value_or(Color::fromHex(0xFFCA28));

    const int32_t tabW = r.width * 2 / 5;
    const int32_t tabH = std::max(2, r.height / 3);

    // 1. Back Tab
    s.drawRoundedRect(Rect{r.x, r.y, tabW, tabH + 2}, 2, tabCol, true);

    // 2. Back Body
    s.drawRoundedRect(Rect{r.x, r.y + tabH - 2, r.width, r.height - tabH + 2}, 3, bodyCol, true);

    // If folder is open, draw an interior document peeking out
    if (open) {
        s.fillRect(Rect{r.x + 3, r.y + 2, r.width - 6, r.height / 2}, Color::fromHex(0xFAFCFF));
        s.fillRect(Rect{r.x + 5, r.y + 4, r.width - 10, 1}, Color::fromHex(0x94A8C4));
        s.fillRect(Rect{r.x + 5, r.y + 7, r.width - 12, 1}, Color::fromHex(0x94A8C4));
    }

    // 3. Front Flap with bevel highlight
    const int32_t flapH = r.height - tabH - 2;
    const Rect flap{r.x, r.y + tabH + 2, r.width, flapH};
    s.drawRoundedRect(flap, 3, flapCol, true);
    s.fillRect(Rect{flap.x + 1, flap.y, flap.width - 2, 1}, Color::fromRgba(255, 255, 255, 140));
}

void IconRenderer::drawDocument(Surface& s, Rect r, IconId docType, std::optional<Color> tint) {
    // Document Base
    Color baseCol = Color::fromHex(0xEAEEF6);
    if (docType == IconId::FileExecutable) baseCol = Color::fromHex(0x141E30);
    else if (docType == IconId::FileLibrary) baseCol = Color::fromHex(0x1B162C);

    s.drawRoundedRect(r, 2, baseCol, true);
    s.drawRoundedRect(r, 2, tint.value_or(Color::fromRgba(255, 255, 255, 50)), false);

    // Dog-ear folded corner at top-right
    const int32_t earSize = std::max(3, r.width / 4);
    const Rect ear{r.right() - earSize, r.y, earSize, earSize};
    s.fillRect(ear, Color::fromHex(0x182438)); // erase corner to background
    s.fillRect(Rect{ear.x, ear.bottom(), earSize, 1}, Color::fromHex(0x90A4BE));
    s.fillRect(Rect{ear.x, ear.y, 1, earSize}, Color::fromHex(0x90A4BE));

    // Content insignia depending on docType
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();

    if (docType == IconId::FileCode) {
        // Emerald < / >
        const Color codeCol = tint.value_or(Color::fromHex(0x00FF9D));
        s.putPixel(cx - 3, cy, codeCol);
        s.putPixel(cx - 2, cy - 1, codeCol);
        s.putPixel(cx - 2, cy + 1, codeCol);

        s.putPixel(cx + 3, cy, codeCol);
        s.putPixel(cx + 2, cy - 1, codeCol);
        s.putPixel(cx + 2, cy + 1, codeCol);

        s.putPixel(cx, cy, Color::fromHex(0x00D4FF));
    } else if (docType == IconId::FileExecutable) {
        // Cyan rocket / diamond chevron
        const Color exeCol = tint.value_or(Color::fromHex(0x00D4FF));
        s.fillRect(Rect{cx - 2, cy - 2, 5, 5}, exeCol);
        s.putPixel(cx, cy - 3, Color::fromHex(0xFFFFFF));
        s.putPixel(cx, cy + 3, Color::fromHex(0xFFFFFF));
    } else if (docType == IconId::FileLibrary) {
        // Violet twin cogs / puzzle
        const Color libCol = tint.value_or(Color::fromHex(0xBA68C8));
        s.fillRect(Rect{cx - 3, cy - 2, 3, 4}, libCol);
        s.fillRect(Rect{cx + 1, cy - 2, 3, 4}, libCol);
        s.putPixel(cx, cy, Color::fromHex(0xFFFFFF));
    } else if (docType == IconId::FileImage) {
        // Sun & mountain
        s.putPixel(r.right() - 5, r.y + 4, Color::fromHex(0xFFD700));
        s.fillRect(Rect{r.x + 3, r.bottom() - 4, r.width - 6, 2}, Color::fromHex(0x4FC3F7));
    } else if (docType == IconId::FileArchive) {
        // Horizontal zipper teeth
        for (int32_t zy = r.y + 4; zy < r.bottom() - 3; zy += 2) {
            s.fillRect(Rect{cx - 2, zy, 4, 1}, Color::fromHex(0xFFA726));
        }
    } else {
        // Standard text lines
        const Color lineCol = tint.value_or(Color::fromHex(0x7088A8));
        const int32_t lY = r.y + earSize + 1;
        s.fillRect(Rect{r.x + 3, lY, r.width - 6, 1}, lineCol);
        s.fillRect(Rect{r.x + 3, lY + 3, r.width - 6, 1}, lineCol);
        s.fillRect(Rect{r.x + 3, lY + 6, r.width - 8, 1}, lineCol);
    }
}

void IconRenderer::drawNavChevron(Surface& s, Rect r, int32_t dir, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0xF5F8FF));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();

    if (dir == 0) {
        // Back <
        for (int32_t i = 0; i <= 3; ++i) {
            s.putPixel(cx - i, cy - i, col);
            s.putPixel(cx - i, cy + i, col);
        }
    } else if (dir == 1) {
        // Forward >
        for (int32_t i = 0; i <= 3; ++i) {
            s.putPixel(cx + i, cy - i, col);
            s.putPixel(cx + i, cy + i, col);
        }
    } else {
        // Up ^
        for (int32_t i = 0; i <= 3; ++i) {
            s.putPixel(cx - i, cy - 2 + i, col);
            s.putPixel(cx + i, cy - 2 + i, col);
        }
    }
}

void IconRenderer::drawNavRefresh(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();
    const int32_t rad = std::max(3, r.width / 3);

    // 270 degree circular arc
    for (int32_t deg = 0; deg < 270; deg += 15) {
        const float rads = static_cast<float>(deg) * 3.14159f / 180.0f;
        const int32_t px = static_cast<int32_t>(cx + std::cos(rads) * rad);
        const int32_t py = static_cast<int32_t>(cy + std::sin(rads) * rad);
        s.putPixel(px, py, col);
    }
    // Arrowhead at top
    s.putPixel(cx, cy - rad - 2, col);
    s.putPixel(cx + 1, cy - rad - 1, col);
    s.putPixel(cx + 2, cy - rad, col);
}

void IconRenderer::drawSearch(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x8EA2BE));
    const int32_t lensR = std::max(2, r.width / 4);
    const int32_t cx = r.x + lensR + 2;
    const int32_t cy = r.y + lensR + 2;

    // Lens circle
    s.drawRoundedRect(Rect{cx - lensR, cy - lensR, lensR * 2, lensR * 2}, lensR, col, false);

    // 45 degree handle
    for (int32_t i = 0; i < lensR + 2; ++i) {
        s.putPixel(cx + lensR - 1 + i, cy + lensR - 1 + i, col);
        s.putPixel(cx + lensR + i, cy + lensR - 1 + i, col);
    }
}

void IconRenderer::drawNewFolder(Surface& s, Rect r, std::optional<Color> tint) {
    drawFolder(s, r, false, tint);
    // Cyan + badge on bottom right
    const Rect badge{r.right() - 6, r.bottom() - 6, 6, 6};
    s.drawRoundedRect(badge, 2, Color::fromHex(0x00D4FF), true);
    s.fillRect(Rect{badge.x + 1, badge.centerY(), 4, 1}, Color::fromHex(0x0E1420));
    s.fillRect(Rect{badge.centerX(), badge.y + 1, 1, 4}, Color::fromHex(0x0E1420));
}

void IconRenderer::drawNewFile(Surface& s, Rect r, std::optional<Color> tint) {
    drawDocument(s, r, IconId::FileGeneric, tint);
    // Green + badge on bottom right
    const Rect badge{r.right() - 6, r.bottom() - 6, 6, 6};
    s.drawRoundedRect(badge, 2, Color::fromHex(0x00FF9D), true);
    s.fillRect(Rect{badge.x + 1, badge.centerY(), 4, 1}, Color::fromHex(0x0E1420));
    s.fillRect(Rect{badge.centerX(), badge.y + 1, 1, 4}, Color::fromHex(0x0E1420));
}

void IconRenderer::drawCut(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();
    const int32_t rad = std::max(2, r.width / 5);

    // Crossed blades
    for (int32_t i = 0; i <= r.width / 3; ++i) {
        s.putPixel(cx - i, cy - i, col);
        s.putPixel(cx + i, cy - i, col);
    }
    // Pivot screw
    s.putPixel(cx, cy, Color::fromHex(0xFFFFFF));

    // Handle loops (left & right)
    s.drawRoundedRect(Rect{cx - rad - 2, cy + 1, rad * 2, rad * 2}, rad, col, false);
    s.drawRoundedRect(Rect{cx + 2, cy + 1, rad * 2, rad * 2}, rad, col, false);
}

void IconRenderer::drawPaste(Surface& s, Rect r, std::optional<Color> tint) {
    const Color accent = tint.value_or(Color::fromHex(0x00D4FF));
    const Color boardCol = Color::fromHex(0x566B88);
    const int32_t bw = std::max(8, r.width * 70 / 100);
    const int32_t bh = std::max(8, r.height * 80 / 100);

    // Clipboard backboard
    const Rect boardRect{r.x + 1, r.y + 3, bw, bh};
    s.drawRoundedRect(boardRect, 2, boardCol, true);

    // Clip at top
    s.fillRect(Rect{boardRect.centerX() - 2, r.y + 1, 5, 3}, Color::fromHex(0xFFD54F));

    // Foreground paper sheet
    const Rect paperRect{boardRect.x + 3, boardRect.y + 3, bw - 4, bh - 4};
    s.drawRoundedRect(paperRect, 1, Color::fromHex(0x182438), true);
    s.drawRoundedRect(paperRect, 1, accent, false);

    // Text lines on paper
    for (int32_t ly = paperRect.y + 3; ly < paperRect.bottom() - 2; ly += 3) {
        s.fillRect(Rect{paperRect.x + 2, ly, paperRect.width - 4, 1}, Color::fromHex(0x8EA2BE));
    }
}

void IconRenderer::drawRename(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    // Text box outline
    s.drawRoundedRect(Rect{r.x + 1, r.y + 2, r.width - 2, r.height - 4}, 2, Color::fromHex(0x364E72), false);

    // I-Beam cursor in center
    const int32_t cx = r.centerX();
    const int32_t top = r.y + 4;
    const int32_t bot = r.bottom() - 4;
    s.fillRect(Rect{cx - 2, top, 5, 1}, col);
    s.fillRect(Rect{cx, top, 1, bot - top}, col);
    s.fillRect(Rect{cx - 2, bot - 1, 5, 1}, col);
}

void IconRenderer::drawDelete(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0xFF6B6B));
    const int32_t w = r.width;
    const int32_t h = r.height;

    // Lid
    s.fillRect(Rect{r.x + 1, r.y + 1, w - 2, 2}, col);
    s.fillRect(Rect{r.centerX() - 1, r.y, 3, 1}, col);

    // Can body
    s.drawRoundedRect(Rect{r.x + 2, r.y + 4, w - 4, h - 5}, 2, col, false);

    // Vertical ribs
    s.fillRect(Rect{r.centerX() - 1, r.y + 6, 1, h - 8}, col);
    s.fillRect(Rect{r.centerX() + 2, r.y + 6, 1, h - 8}, col);
    s.fillRect(Rect{r.centerX() - 3, r.y + 6, 1, h - 8}, col);
}

void IconRenderer::drawProperties(Surface& s, Rect r, std::optional<Color> tint) {
    drawDocument(s, r, IconId::FileGeneric, tint);
    // Blue i badge
    const int32_t badgeR = std::max(2, r.width / 4);
    const Rect badge{r.centerX() - badgeR, r.centerY() - badgeR, badgeR * 2, badgeR * 2};
    s.drawRoundedRect(badge, badgeR, Color::fromHex(0x00D4FF), true);
    s.putPixel(r.centerX(), badge.y + 1, Color::fromHex(0x0E1420));
    s.fillRect(Rect{r.centerX(), badge.y + 3, 1, badge.height - 4}, Color::fromHex(0x0E1420));
}

void IconRenderer::drawViewList(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x8EA2BE));
    const int32_t y0 = r.y + 2;
    const int32_t step = std::max(3, (r.height - 4) / 3);

    for (int32_t i = 0; i < 3; ++i) {
        const int32_t py = y0 + i * step;
        s.fillRect(Rect{r.x + 1, py, 2, 2}, col);
        s.fillRect(Rect{r.x + 5, py, r.width - 6, 2}, col);
    }
}

void IconRenderer::drawViewGrid(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x8EA2BE));
    const int32_t pad = 1;
    const int32_t availableW = r.width - pad * 2;
    const int32_t availableH = r.height - pad * 2;
    const int32_t cellW = std::max(2, (availableW - 4) / 3);
    const int32_t cellH = std::max(2, (availableH - 4) / 3);
    const int32_t stepX = (availableW - cellW) / 2;
    const int32_t stepY = (availableH - cellH) / 2;

    for (int32_t row = 0; row < 3; ++row) {
        for (int32_t colIdx = 0; colIdx < 3; ++colIdx) {
            const int32_t px = r.x + pad + colIdx * stepX;
            const int32_t py = r.y + pad + row * stepY;
            s.fillRect(Rect{px, py, cellW, cellH}, col);
        }
    }
}

void IconRenderer::drawEdit(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t len = std::max(2, std::min(r.width, r.height) - 4);
    const int32_t startX = r.x + 2;
    const int32_t startY = r.bottom() - 3;

    // Angled stylus / pencil
    for (int32_t i = 0; i < len; ++i) {
        s.putPixel(startX + i, startY - i, col);
        s.putPixel(startX + i + 1, startY - i, col);
    }
    // Eraser tip
    s.putPixel(startX + len, startY - len, Color::fromHex(0xFFA726));
    s.putPixel(startX + len + 1, startY - len, Color::fromHex(0xFFA726));
    // Sharp nib at bottom
    s.putPixel(startX, startY, Color::fromHex(0xFFFFFF));
}

void IconRenderer::drawCopy(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const Color bgCol = Color::fromHex(0x6A82A0);
    const int32_t sheetW = std::max(3, r.width - 5);
    const int32_t sheetH = std::max(3, r.height - 5);

    // Back sheet
    s.drawRoundedRect(Rect{r.x + 4, r.y + 1, sheetW, sheetH}, 2, bgCol, false);
    // Front sheet
    s.drawRoundedRect(Rect{r.x + 1, r.y + 4, sheetW, sheetH}, 2, col, true);
    s.drawRoundedRect(Rect{r.x + 1, r.y + 4, sheetW, sheetH}, 2, Color::fromHex(0xFFFFFF), false);
    // Front sheet text lines
    s.fillRect(Rect{r.x + 3, r.y + 7, std::max(1, sheetW - 4), 1}, Color::fromHex(0x0E1420));
    s.fillRect(Rect{r.x + 3, r.y + 10, std::max(1, sheetW - 6), 1}, Color::fromHex(0x0E1420));
}

void IconRenderer::drawSort(Surface& s, Rect r, bool asc, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t barH = 2;

    // Stepped horizontal bars on left
    const int32_t barX = r.x + 2;
    s.fillRect(Rect{barX, r.y + 3, std::max(2, r.width / 2), barH}, col);
    s.fillRect(Rect{barX, r.y + 7, std::max(2, r.width * 3 / 8), barH}, col);
    s.fillRect(Rect{barX, r.y + 11, std::max(2, r.width / 4), barH}, col);

    // Directional arrow on right
    const int32_t ax = r.right() - 4;
    if (asc) {
        // Up arrow ^
        s.fillRect(Rect{ax, r.y + 4, 1, 8}, col);
        s.putPixel(ax - 1, r.y + 5, col);
        s.putPixel(ax + 1, r.y + 5, col);
        s.putPixel(ax - 2, r.y + 6, col);
        s.putPixel(ax + 2, r.y + 6, col);
    } else {
        // Down arrow v
        s.fillRect(Rect{ax, r.y + 4, 1, 8}, col);
        s.putPixel(ax - 1, r.y + 10, col);
        s.putPixel(ax + 1, r.y + 10, col);
        s.putPixel(ax - 2, r.y + 9, col);
        s.putPixel(ax + 2, r.y + 9, col);
    }
}

void IconRenderer::drawVolume(Surface& s, Rect r, bool mute, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0xF5F8FF));
    const int32_t cy = r.centerY();

    // Speaker cone
    s.fillRect(Rect{r.x + 2, cy - 2, 2, 5}, col);
    for (int32_t i = 0; i < 3; ++i) {
        s.fillRect(Rect{r.x + 4 + i, cy - 2 - i, 1, 5 + i * 2}, col);
    }

    if (mute) {
        // X mark
        s.putPixel(r.right() - 4, cy - 2, Color::fromHex(0xFF6B6B));
        s.putPixel(r.right() - 2, cy - 2, Color::fromHex(0xFF6B6B));
        s.putPixel(r.right() - 3, cy - 1, Color::fromHex(0xFF6B6B));
        s.putPixel(r.right() - 4, cy, Color::fromHex(0xFF6B6B));
        s.putPixel(r.right() - 2, cy, Color::fromHex(0xFF6B6B));
    } else {
        // Sound waves
        for (int32_t dy = -3; dy <= 3; ++dy) {
            s.putPixel(r.right() - 4, cy + dy, col);
        }
        for (int32_t dy = -2; dy <= 2; ++dy) {
            s.putPixel(r.right() - 2, cy + dy, Color::fromRgba(col.r, col.g, col.b, 180));
        }
    }
}

void IconRenderer::drawBattery(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00FF9D));
    const int32_t cy = r.centerY();

    // Battery rectangle
    s.drawRoundedRect(Rect{r.x + 1, cy - 4, r.width - 4, 9}, 2, Color::fromHex(0x607898), false);
    // Positive terminal cap
    s.fillRect(Rect{r.right() - 2, cy - 2, 2, 5}, Color::fromHex(0x607898));
    // Interior charge bar
    s.fillRect(Rect{r.x + 3, cy - 2, r.width - 8, 5}, col);
}

void IconRenderer::drawNetwork(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t bottom = r.bottom() - 1;
    const int32_t barW = std::max(1, r.width / 5);

    // 4 stepped signal bars
    s.fillRect(Rect{r.x + 1, bottom - 3, barW, 3}, col);
    s.fillRect(Rect{r.x + 1 + barW + 1, bottom - 6, barW, 6}, col);
    s.fillRect(Rect{r.x + 1 + (barW + 1) * 2, bottom - 9, barW, 9}, col);
    s.fillRect(Rect{r.x + 1 + (barW + 1) * 3, bottom - 12, barW, 12}, col);
}

void IconRenderer::drawClock(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0xF5F8FF));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();
    const int32_t rad = std::max(3, r.width / 2 - 1);

    // Clock outer ring
    s.drawRoundedRect(Rect{cx - rad, cy - rad, rad * 2, rad * 2}, rad, col, false);

    // Center dot
    s.putPixel(cx, cy, col);

    // Hour hand (pointing to 3)
    s.fillRect(Rect{cx, cy, rad / 2 + 1, 1}, col);

    // Minute hand (pointing to 12)
    s.fillRect(Rect{cx, cy - rad * 2 / 3, 1, rad * 2 / 3}, col);
}

void IconRenderer::drawPower(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0xFF453A));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();
    const int32_t rad = std::max(3, r.width / 2 - 2);

    // Broken circular ring (gap at top)
    const int32_t r2Min = (rad - 2) * (rad - 2);
    const int32_t r2Max = rad * rad;
    for (int32_t dy = -rad; dy <= rad; ++dy) {
        for (int32_t dx = -rad; dx <= rad; ++dx) {
            const int32_t d2 = dx * dx + dy * dy;
            if (d2 >= r2Min && d2 <= r2Max) {
                // Gap at the top: if dy < 0 and |dx| <= rad / 2, skip
                if (dy < 0 && std::abs(dx) <= std::max(1, rad / 3)) {
                    continue;
                }
                s.putPixel(cx + dx, cy + dy, col);
            }
        }
    }

    // Vertical power toggle bar in the top gap
    const int32_t barH = std::max(3, rad + 1);
    s.fillRect(Rect{cx - 1, cy - rad, 2, barH}, col);
}

void IconRenderer::drawRestart(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();
    const int32_t rad = std::max(3, r.width / 2 - 2);

    // 3/4 circular arc (clockwise)
    const int32_t r2Min = (rad - 2) * (rad - 2);
    const int32_t r2Max = rad * rad;
    for (int32_t dy = -rad; dy <= rad; ++dy) {
        for (int32_t dx = -rad; dx <= rad; ++dx) {
            const int32_t d2 = dx * dx + dy * dy;
            if (d2 >= r2Min && d2 <= r2Max) {
                // Skip upper-right quadrant gap where arrowhead sits
                if (dx > 0 && dy < 0) continue;
                s.putPixel(cx + dx, cy + dy, col);
            }
        }
    }

    // Arrowhead at top right pointing clockwise
    const int32_t ax = cx + rad - 1;
    const int32_t ay = cy - 2;
    for (int32_t i = 0; i <= std::max(2, rad / 2); ++i) {
        s.fillRect(Rect{ax - i, ay + i - 1, 2, 2}, col);
    }
}

void IconRenderer::drawSleep(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x94A8C4));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();
    const int32_t rad = std::max(3, r.width / 2 - 2);

    // Crescent moon
    const int32_t r2Outer = rad * rad;
    const int32_t cutX = cx + std::max(2, rad / 3);
    const int32_t cutY = cy - std::max(1, rad / 4);
    const int32_t cutRad = std::max(2, rad * 3 / 4);
    const int32_t r2Inner = cutRad * cutRad;

    for (int32_t dy = -rad; dy <= rad; ++dy) {
        for (int32_t dx = -rad; dx <= rad; ++dx) {
            const int32_t d2Outer = dx * dx + dy * dy;
            if (d2Outer <= r2Outer) {
                const int32_t px = cx + dx;
                const int32_t py = cy + dy;
                const int32_t d2Inner = (px - cutX) * (px - cutX) + (py - cutY) * (py - cutY);
                if (d2Inner > r2Inner) {
                    s.putPixel(px, py, col);
                }
            }
        }
    }
}

void IconRenderer::drawLock(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0xFFB900));
    const int32_t cx = r.centerX();
    const int32_t bodyW = std::max(6, r.width * 3 / 4);
    const int32_t bodyH = std::max(5, r.height / 2);
    const int32_t bodyY = r.bottom() - bodyH - 1;
    const int32_t bodyX = cx - bodyW / 2;

    // Padlock body
    s.drawRoundedRect(Rect{bodyX, bodyY, bodyW, bodyH}, 2, col, true);

    // Keyhole (dark)
    s.drawRoundedRect(Rect{cx - 1, bodyY + 2, 2, 2}, 1, Color::fromHex(0x0E1420), true);
    s.fillRect(Rect{cx, bodyY + 3, 1, std::max(2, bodyH / 3)}, Color::fromHex(0x0E1420));

    // Shackle loop (arch on top)
    const int32_t shackleW = bodyW - 4;
    const int32_t shackleH = std::max(4, r.height / 3);
    const int32_t shackleY = bodyY - shackleH + 1;
    const int32_t shackleX = cx - shackleW / 2;

    s.drawRoundedRect(Rect{shackleX, shackleY, shackleW, shackleH + 2}, 2, col, false);
}

void IconRenderer::drawSignOut(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x94A8C4));
    const int32_t cy = r.centerY();

    // Door frame (left side)
    const int32_t doorW = std::max(4, r.width / 2);
    const int32_t doorH = std::max(8, r.height - 4);
    const int32_t doorX = r.x + 2;
    const int32_t doorY = r.y + 2;

    s.fillRect(Rect{doorX, doorY, 2, doorH}, col); // left upright
    s.fillRect(Rect{doorX, doorY, doorW, 2}, col); // top
    s.fillRect(Rect{doorX, doorY + doorH - 2, doorW, 2}, col); // bottom

    // Arrow pointing right
    const int32_t arrowY = cy;
    const int32_t arrowX1 = doorX + doorW / 2;
    const int32_t arrowX2 = r.right() - 2;
    s.fillRect(Rect{arrowX1, arrowY - 1, arrowX2 - arrowX1, 2}, col);
    s.putPixel(arrowX2, arrowY, col);
    s.putPixel(arrowX2 - 1, arrowY - 1, col);
    s.putPixel(arrowX2 - 1, arrowY + 1, col);
    s.putPixel(arrowX2 - 2, arrowY - 2, col);
    s.putPixel(arrowX2 - 2, arrowY + 2, col);
}

void IconRenderer::drawUser(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t cx = r.centerX();
    const int32_t headR = std::max(2, r.width / 5);
    const int32_t headY = r.y + headR + 2;

    // Head circle
    s.drawRoundedRect(Rect{cx - headR, headY - headR, headR * 2, headR * 2}, headR, col, true);

    // Shoulder arch
    const int32_t shoulderW = std::max(6, r.width * 3 / 4);
    const int32_t shoulderH = std::max(3, r.height / 3);
    const int32_t shoulderY = r.bottom() - shoulderH - 1;
    s.drawRoundedRect(Rect{cx - shoulderW / 2, shoulderY, shoulderW, shoulderH * 2}, shoulderH, col, true);
}

void IconRenderer::drawHibernate(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00FF9D));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();
    const int32_t rad = std::max(3, r.width / 2 - 2);

    // Outer circle
    s.drawRoundedRect(Rect{cx - rad, cy - rad, rad * 2, rad * 2}, rad, col, false);

    // Stylized "Z"
    const int32_t zw = std::max(3, rad);
    const int32_t zh = std::max(3, rad);
    s.fillRect(Rect{cx - zw / 2, cy - zh / 2, zw, 1}, col);
    s.putPixel(cx + zw / 4, cy - zh / 4, col);
    s.putPixel(cx, cy, col);
    s.putPixel(cx - zw / 4, cy + zh / 4, col);
    s.fillRect(Rect{cx - zw / 2, cy + zh / 2, zw, 1}, col);
}

void IconRenderer::drawMediaPlay(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t pad = std::max(2, r.width / 5);
    const int32_t x0 = r.x + pad;
    const int32_t x1 = r.right() - pad;
    const int32_t y0 = r.y + pad;
    const int32_t y1 = r.bottom() - pad;
    const int32_t cy = (y0 + y1) / 2;
    const int32_t w = std::max(1, x1 - x0);

    for (int32_t x = x0; x <= x1; ++x) {
        const float t = static_cast<float>(x - x0) / static_cast<float>(w);
        const int32_t halfH = static_cast<int32_t>((1.0f - t) * (cy - y0));
        s.fillRect(Rect{x, cy - halfH, 1, halfH * 2 + 1}, col);
    }
}

void IconRenderer::drawMediaPause(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t pad = std::max(2, r.width / 5);
    const int32_t barW = std::max(2, (r.width - pad * 2) / 3);
    const int32_t h = r.height - pad * 2;
    const int32_t y = r.y + pad;

    // Left bar
    s.drawRoundedRect(Rect{r.x + pad, y, barW, h}, 1, col, true);
    // Right bar
    s.drawRoundedRect(Rect{r.right() - pad - barW, y, barW, h}, 1, col, true);
}

void IconRenderer::drawMediaNext(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t pad = std::max(2, r.width / 6);
    const int32_t barW = std::max(2, r.width / 8);
    // Right vertical stop bar
    s.fillRect(Rect{r.right() - pad - barW, r.y + pad, barW, r.height - pad * 2}, col);
    // Play triangle on the left
    const Rect playR{r.x + pad, r.y + pad, r.width - pad * 2 - barW - 2, r.height - pad * 2};
    drawMediaPlay(s, playR, tint);
}

void IconRenderer::drawMediaPrev(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t pad = std::max(2, r.width / 6);
    const int32_t barW = std::max(2, r.width / 8);
    // Left vertical stop bar
    s.fillRect(Rect{r.x + pad, r.y + pad, barW, r.height - pad * 2}, col);
    // Left-pointing triangle
    const int32_t x0 = r.x + pad + barW + 2;
    const int32_t x1 = r.right() - pad;
    const int32_t y0 = r.y + pad;
    const int32_t y1 = r.bottom() - pad;
    const int32_t cy = (y0 + y1) / 2;
    const int32_t w = std::max(1, x1 - x0);

    for (int32_t x = x0; x <= x1; ++x) {
        const float t = static_cast<float>(x - x0) / static_cast<float>(w);
        const int32_t halfH = static_cast<int32_t>(t * (cy - y0));
        s.fillRect(Rect{x, cy - halfH, 1, halfH * 2 + 1}, col);
    }
}

void IconRenderer::drawNotificationBell(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0xFFD54F));
    const int32_t cx = r.centerX();
    const int32_t top = r.y + std::max(2, r.height / 7);
    const int32_t bottom = r.bottom() - std::max(3, r.height / 5);

    // Top hanger loop
    s.fillRect(Rect{cx - 1, r.y + 1, 3, 2}, col);

    // Flared bell dome
    const int32_t h = bottom - top;
    for (int32_t y = top; y <= bottom; ++y) {
        const float t = static_cast<float>(y - top) / static_cast<float>(std::max(1, h));
        const int32_t halfW = static_cast<int32_t>((2 + t * t * (r.width / 2 - 2)));
        s.fillRect(Rect{cx - halfW, y, halfW * 2 + 1, 1}, col);
    }
    // Bottom flare rim
    s.fillRect(Rect{r.x + 2, bottom, r.width - 4, 2}, col);

    // Clapper circle
    const int32_t clapR = std::max(1, r.width / 10);
    s.drawRoundedRect(Rect{cx - clapR, bottom + 1, clapR * 2 + 1, clapR * 2 + 1}, clapR, col, true);
}

void IconRenderer::drawNetworkEthernet(Surface& s, Rect r, std::optional<Color> tint) {
    const Color col = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t pad = std::max(2, r.width / 6);
    const Rect body{r.x + pad, r.y + pad, r.width - pad * 2, r.height - pad * 2};

    // Outer RJ-45 jack outline
    s.drawRoundedRect(body, 2, col, false);

    // Bottom tab slot
    const int32_t slotW = std::max(4, body.width / 2);
    s.fillRect(Rect{body.centerX() - slotW / 2, body.bottom() - 2, slotW, 2}, Color::fromHex(0x0E1420));
    s.fillRect(Rect{body.centerX() - slotW / 2, body.bottom() - 1, slotW, 1}, col);

    // Contact pins inside
    const int32_t pinY = body.y + 3;
    const int32_t pinH = std::max(2, body.height / 4);
    for (int32_t px = body.x + 3; px < body.right() - 3; px += 2) {
        s.fillRect(Rect{px, pinY, 1, pinH}, Color::fromHex(0xFFD54F));
    }
}

void IconRenderer::drawCalculator(Surface& s, Rect r, std::optional<Color> tint) {
    const Color frameCol = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t pad = std::max(1, r.width / 8);
    const Rect body{r.x + pad, r.y + 1, r.width - pad * 2, r.height - 2};

    // Body chassis
    s.drawRoundedRect(body, 2, Color::fromHex(0x162032), true);
    s.drawRoundedRect(body, 2, frameCol, false);

    // Screen readout at top
    const int32_t screenH = std::max(3, body.height / 4);
    const Rect screenBox{body.x + 2, body.y + 2, body.width - 4, screenH};
    s.fillRect(screenBox, Color::fromHex(0x0A0F1A));
    s.fillRect(Rect{screenBox.right() - std::max(3, screenBox.width / 3), screenBox.y + screenBox.height / 2, std::max(2, screenBox.width / 3) - 1, 1},
               Color::fromHex(0x00FF9D));

    // Numeric keypad grid
    const int32_t keyStartY = screenBox.bottom() + 2;
    const int32_t keyAreaH = body.bottom() - 2 - keyStartY;
    if (keyAreaH >= 4 && body.width >= 8) {
        const int32_t colStep = (body.width - 4) / 3;
        const int32_t rowStep = keyAreaH / 3;
        for (int32_t row = 0; row < 2; ++row) {
            for (int32_t col = 0; col < 3; ++col) {
                const int32_t kx = body.x + 2 + col * colStep + 1;
                const int32_t ky = keyStartY + row * rowStep + 1;
                s.fillRect(Rect{kx, ky, std::max(1, colStep - 2), std::max(1, rowStep - 2)}, Color::fromHex(0x405578));
            }
        }
        // Equals accent button on bottom row
        const int32_t eqX = body.x + 2 + 2 * colStep + 1;
        const int32_t eqY = keyStartY + 2 * rowStep + 1;
        s.fillRect(Rect{eqX, eqY, std::max(1, colStep - 2), std::max(1, rowStep - 2)}, Color::fromHex(0x00D4FF));
    }
}

void IconRenderer::drawRunDialog(Surface& s, Rect r, std::optional<Color> tint) {
    const Color frameCol = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t pad = std::max(1, r.width / 10);
    const Rect winBox{r.x + pad, r.y + pad + 1, r.width - pad * 2, r.height - pad * 2 - 2};

    // Outer window border and background
    s.drawRoundedRect(winBox, 2, Color::fromHex(0x101726), true);
    s.drawRoundedRect(winBox, 2, frameCol, false);

    // Titlebar header
    const int32_t titleH = std::max(2, winBox.height / 4);
    s.fillRect(Rect{winBox.x + 1, winBox.y + 1, winBox.width - 2, titleH}, frameCol);

    // Prompt glyph '>'
    const int32_t cy = winBox.y + titleH + (winBox.height - titleH) / 2;
    const int32_t px = winBox.x + 3;
    s.putPixel(px, cy - 2, Color::fromHex(0x00FF9D));
    s.putPixel(px + 1, cy - 1, Color::fromHex(0x00FF9D));
    s.putPixel(px + 2, cy, Color::fromHex(0x00FF9D));
    s.putPixel(px + 1, cy + 1, Color::fromHex(0x00FF9D));
    s.putPixel(px, cy + 2, Color::fromHex(0x00FF9D));

    // Blinking cursor '_'
    const int32_t curX = px + 4;
    s.fillRect(Rect{curX, cy + 2, std::max(2, (winBox.width - 10) / 2), 1}, Color::fromHex(0xFFFFFF));
}

void IconRenderer::drawDisplay(Surface& s, Rect r, std::optional<Color> tint) {
    const Color frameCol = tint.value_or(Color::fromHex(0x4A6082));
    const int32_t w = r.width;
    const int32_t h = r.height;
    const int32_t standH = std::max(2, h / 5);
    const int32_t screenH = h - standH - 1;

    // Monitor outer frame
    s.drawRoundedRect(Rect{r.x, r.y, w, screenH}, std::max(2, w / 8), frameCol, true);

    // Screen interior
    const int32_t bezel = std::max(1, w / 12);
    const Rect screenArea{r.x + bezel, r.y + bezel, w - bezel * 2, screenH - bezel * 2};
    s.fillRect(screenArea, Color::fromHex(0x0C1422));

    // Wallpaper horizon line
    if (screenArea.height >= 4) {
        s.fillRect(Rect{screenArea.x, screenArea.y + screenArea.height * 2 / 3, screenArea.width, 1},
                   Color::fromHex(0x00D4FF));
    }

    // Stand neck and base
    const int32_t cx = r.centerX();
    s.fillRect(Rect{cx - 1, r.y + screenH, 2, standH}, frameCol);
    const int32_t baseW = std::max(4, w / 2);
    s.fillRect(Rect{cx - baseW / 2, r.bottom() - 1, baseW, 2}, frameCol);
}

void IconRenderer::drawPersonalization(Surface& s, Rect r, std::optional<Color> tint) {
    const Color frameCol = tint.value_or(Color::fromHex(0x283850));
    const int32_t rad = std::max(2, r.width / 3);

    // Artist palette chassis
    s.drawRoundedRect(Rect{r.x + 1, r.y + 1, r.width - 2, r.height - 2}, rad, frameCol, true);
    s.drawRoundedRect(Rect{r.x + 1, r.y + 1, r.width - 2, r.height - 2}, rad, Color::fromHex(0x56729C), false);

    // Thumb hole
    const int32_t thX = r.right() - std::max(3, r.width / 4);
    const int32_t thY = r.bottom() - std::max(3, r.height / 4);
    s.putPixel(thX, thY, Color::fromHex(0x0E1420));
    s.putPixel(thX + 1, thY, Color::fromHex(0x0E1420));
    s.putPixel(thX, thY + 1, Color::fromHex(0x0E1420));

    // Paint splotches (Cyan, Coral, Gold)
    const int32_t spotR = std::max(1, r.width / 8);
    // Cyan
    s.drawRoundedRect(Rect{r.x + std::max(2, r.width / 5), r.y + std::max(2, r.height / 4), spotR * 2, spotR * 2}, spotR, Color::fromHex(0x00D4FF), true);
    // Coral Red
    s.drawRoundedRect(Rect{r.centerX(), r.y + std::max(2, r.height / 6), spotR * 2, spotR * 2}, spotR, Color::fromHex(0xFF4D6D), true);
    // Solar Amber
    s.drawRoundedRect(Rect{r.x + std::max(2, r.width / 4), r.bottom() - std::max(4, r.height / 3), spotR * 2, spotR * 2}, spotR, Color::fromHex(0xFFB703), true);
}

void IconRenderer::drawCalendar(Surface& s, Rect r, std::optional<Color> tint) {
    const Color headerCol = tint.value_or(Color::fromHex(0xFF4D6D)); // Coral red header
    const Color bodyCol = Color::fromHex(0x141C2A);
    const Color borderCol = Color::fromHex(0x405572);

    const int32_t rad = std::max(1, r.width / 8);
    s.drawRoundedRect(Rect{r.x + 1, r.y + 2, r.width - 2, r.height - 3}, rad, bodyCol, true);
    s.drawRoundedRect(Rect{r.x + 1, r.y + 2, r.width - 2, r.height - 3}, rad, borderCol, false);

    // Top month header ribbon
    const int32_t hdrH = std::max(3, r.height / 3);
    s.fillRect(Rect{r.x + 2, r.y + 3, r.width - 4, hdrH}, headerCol);

    // Binder rings
    const int32_t r1 = r.x + r.width / 3;
    const int32_t r2 = r.x + (r.width * 2) / 3;
    s.fillRect(Rect{r1, r.y, 2, 4}, Color::fromHex(0xCBD5E1));
    s.fillRect(Rect{r2, r.y, 2, 4}, Color::fromHex(0xCBD5E1));

    // Calendar day grid dots
    const int32_t bodyY = r.y + hdrH + 4;
    const int32_t cellW = std::max(2, (r.width - 6) / 3);
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 3; ++col) {
            const int32_t dotX = r.x + 4 + col * cellW;
            const int32_t dotY = bodyY + row * std::max(2, (r.height - hdrH - 8) / 2);
            if (dotX < r.right() - 2 && dotY < r.bottom() - 2) {
                const bool isToday = (row == 0 && col == 1);
                s.putPixel(dotX, dotY, isToday ? Color::fromHex(0x00D4FF) : Color::fromHex(0x7186A4));
                if (r.width >= 24) {
                    s.putPixel(dotX + 1, dotY, isToday ? Color::fromHex(0x00D4FF) : Color::fromHex(0x7186A4));
                }
            }
        }
    }
}

void IconRenderer::drawTerminalTab(Surface& s, Rect r, std::optional<Color> tint) {
    const Color frameCol = Color::fromHex(0x101622);
    const Color borderCol = tint.value_or(Color::fromHex(0x3B506E));
    const Color activeTabCol = Color::fromHex(0x00D4FF);

    s.drawRoundedRect(Rect{r.x + 1, r.y + 1, r.width - 2, r.height - 2}, 2, frameCol, true);
    s.drawRoundedRect(Rect{r.x + 1, r.y + 1, r.width - 2, r.height - 2}, 2, borderCol, false);

    // Active tab in top bar
    const int32_t tabW = std::max(5, (r.width - 4) / 2);
    s.fillRect(Rect{r.x + 3, r.y + 2, tabW, 2}, activeTabCol);

    // Prompt glyph: > _
    const int32_t py = r.y + r.height / 2;
    const int32_t px = r.x + std::max(3, r.width / 5);
    // Chevron >
    s.putPixel(px, py - 2, Color::fromHex(0x00FF9D));
    s.putPixel(px + 1, py - 1, Color::fromHex(0x00FF9D));
    s.putPixel(px + 2, py, Color::fromHex(0x00FF9D));
    s.putPixel(px + 1, py + 1, Color::fromHex(0x00FF9D));
    s.putPixel(px, py + 2, Color::fromHex(0x00FF9D));

    // Cursor _
    const int32_t cx = px + 5;
    if (cx < r.right() - 2) {
        s.fillRect(Rect{cx, py + 2, std::max(2, r.width / 6), 1}, Color::fromHex(0xCBD5E1));
    }
}

void IconRenderer::drawShieldAdmin(Surface& s, Rect r, std::optional<Color> tint) {
    const Color shieldCol = Color::fromHex(0x121A28);
    const Color borderCol = tint.value_or(Color::fromHex(0xFFB703)); // Sovereign Gold

    const int32_t w = r.width;
    const int32_t h = r.height;
    const int32_t cx = r.centerX();

    // Shield body
    s.drawRoundedRect(Rect{r.x + 2, r.y + 1, w - 4, h * 3 / 4}, 2, shieldCol, true);
    s.drawRoundedRect(Rect{r.x + 2, r.y + 1, w - 4, h * 3 / 4}, 2, borderCol, false);

    // Taper to bottom tip
    const int32_t botY = r.bottom() - 1;
    for (int y = r.y + h * 3 / 4; y <= botY; ++y) {
        const float t = static_cast<float>(y - (r.y + h * 3 / 4)) / static_cast<float>(std::max(1, botY - (r.y + h * 3 / 4)));
        const int32_t span = static_cast<int32_t>((w / 2 - 2) * (1.0f - t));
        s.fillRect(Rect{cx - span, y, std::max(1, span * 2), 1}, shieldCol);
        s.putPixel(cx - span, y, borderCol);
        s.putPixel(cx + span, y, borderCol);
    }

    // Central sovereign key/star emblem in gold
    const int32_t cy = r.y + h / 3;
    s.drawRoundedRect(Rect{cx - 2, cy - 2, 4, 4}, 1, Color::fromHex(0xFFB703), true);
    s.fillRect(Rect{cx - 1, cy + 2, 2, std::max(2, h / 5)}, Color::fromHex(0xFFB703));
    s.fillRect(Rect{cx, cy + 4, 2, 1}, Color::fromHex(0xFFB703));
}

void IconRenderer::drawSearchCategory(Surface& s, Rect r, std::optional<Color> tint) {
    const Color lensCol = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t lensSize = std::max(6, r.width * 5 / 8);

    // Lens circle
    s.drawRoundedRect(Rect{r.x + 1, r.y + 1, lensSize, lensSize}, lensSize / 2, Color::fromHex(0x101826), true);
    s.drawRoundedRect(Rect{r.x + 1, r.y + 1, lensSize, lensSize}, lensSize / 2, lensCol, false);

    // Lens center highlight
    s.putPixel(r.x + 3, r.y + 3, Color::fromHex(0xFFFFFF));

    // Handle
    const int32_t hx1 = r.x + lensSize - 1;
    const int32_t hy1 = r.y + lensSize - 1;
    const int32_t hx2 = r.right() - 2;
    for (int32_t i = 0; i <= (hx2 - hx1); ++i) {
        s.putPixel(hx1 + i, hy1 + i, Color::fromHex(0x7186A4));
        s.putPixel(hx1 + i + 1, hy1 + i, Color::fromHex(0x7186A4));
    }
}

void IconRenderer::drawRegistry(Surface& s, Rect r, std::optional<Color> tint) {
    const Color mainCol = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();
    const int32_t sz = std::max(6, std::min(r.width, r.height) / 2 - 1);

    // Main base cube / database hierarchy
    const Rect baseCube{cx - sz / 2, cy - sz / 4, sz, sz * 3 / 4};
    s.drawRoundedRect(baseCube, 2, Color::fromHex(0x102038), true);
    s.drawRoundedRect(baseCube, 2, mainCol, false);

    // Inner divider lines representing database tiers
    s.fillRect(Rect{baseCube.x + 2, baseCube.y + baseCube.height / 2, baseCube.width - 4, 1}, Color::fromHex(0x284870));
    s.fillRect(Rect{baseCube.centerX(), baseCube.y + 2, 1, baseCube.height - 4}, Color::fromHex(0x284870));

    // Floating detached registry hive cube on upper right
    const int32_t floatSz = std::max(3, sz / 2);
    const Rect floatCube{baseCube.right() - floatSz / 2, baseCube.y - floatSz - 1, floatSz, floatSz};
    s.drawRoundedRect(floatCube, 1, Color::fromHex(0x00FF9D), true);
    s.drawRoundedRect(floatCube, 1, Color::fromHex(0xFFFFFF), false);
}

void IconRenderer::drawCloudDrive(Surface& s, Rect r, std::optional<Color> tint) {
    const Color cloudCol = tint.value_or(Color::fromHex(0x4285F4));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY();
    const int32_t w = r.width;
    const int32_t h = r.height;

    // Cloud Body lobes
    const int32_t baseH = std::max(3, h / 3);
    const int32_t baseW = std::max(6, w * 3 / 4);
    const Rect baseRect{cx - baseW / 2, cy - baseH / 4, baseW, baseH};
    s.drawRoundedRect(baseRect, baseH / 2, Color::fromHex(0x1B2A4A), true);
    s.drawRoundedRect(baseRect, baseH / 2, cloudCol, false);

    // Left lobe circle
    const int32_t leftR = std::max(2, w / 4);
    const Rect leftLobe{baseRect.x, baseRect.y - leftR / 2, leftR * 2, leftR * 2};
    s.drawRoundedRect(leftLobe, leftR, Color::fromHex(0x233760), true);
    s.drawRoundedRect(leftLobe, leftR, cloudCol, false);

    // Center/Top lobe circle (largest)
    const int32_t topR = std::max(3, w * 5 / 16);
    const Rect topLobe{cx - topR, baseRect.y - topR, topR * 2, topR * 2};
    s.drawRoundedRect(topLobe, topR, Color::fromHex(0x2A4375), true);
    s.drawRoundedRect(topLobe, topR, Color::fromHex(0x60A5FA), false);

    // Sync accent badge (Google Drive triad green/yellow)
    if (w >= 16) {
        s.fillRect(Rect{cx - 1, baseRect.bottom() - 2, 3, 2}, Color::fromHex(0x34A853));
        s.putPixel(cx + 2, baseRect.bottom() - 2, Color::fromHex(0xFBBC05));
    }
}

void IconRenderer::drawNetworkShare(Surface& s, Rect r, std::optional<Color> tint) {
    const Color linkCol = tint.value_or(Color::fromHex(0x00D4FF));
    const int32_t w = r.width;
    const int32_t h = r.height;
    const int32_t serverH = std::max(3, h * 3 / 10);
    const int32_t serverW = std::max(6, w - 4);
    const int32_t sx = r.centerX() - serverW / 2;

    // Top server appliance (NAS Unit 1)
    const Rect topServer{sx, r.y + 1, serverW, serverH};
    s.drawRoundedRect(topServer, 2, Color::fromHex(0x182438), true);
    s.drawRoundedRect(topServer, 2, Color::fromHex(0x405B85), false);
    s.putPixel(topServer.right() - 3, topServer.y + 2, Color::fromHex(0x00FF9D));
    if (topServer.width >= 12) {
        s.fillRect(Rect{topServer.x + 3, topServer.y + 2, topServer.width - 8, 1}, Color::fromHex(0x2A3E5C));
    }

    // Bottom server appliance (NAS Unit 2)
    const Rect botServer{sx, r.bottom() - 1 - serverH, serverW, serverH};
    s.drawRoundedRect(botServer, 2, Color::fromHex(0x182438), true);
    s.drawRoundedRect(botServer, 2, Color::fromHex(0x405B85), false);
    s.putPixel(botServer.right() - 3, botServer.y + 2, Color::fromHex(0x00D4FF));
    if (botServer.width >= 12) {
        s.fillRect(Rect{botServer.x + 3, botServer.y + 2, botServer.width - 8, 1}, Color::fromHex(0x2A3E5C));
    }

    // Central interconnect link spine
    const int32_t cx = r.centerX();
    s.fillRect(Rect{cx, topServer.bottom(), 1, botServer.y - topServer.bottom()}, linkCol);
    const int32_t midY = (topServer.bottom() + botServer.y) / 2;
    s.putPixel(cx - 1, midY, Color::fromHex(0xFFFFFF));
    s.putPixel(cx, midY, Color::fromHex(0xFFFFFF));
    s.putPixel(cx + 1, midY, Color::fromHex(0xFFFFFF));
}

void IconRenderer::drawOpticalDrive(Surface& s, Rect r, std::optional<Color> tint) {
    const Color discCol = tint.value_or(Color::fromHex(0x94A3B8));
    const int32_t cx = r.centerX();
    const int32_t cy = r.centerY() - 1;
    const int32_t rad = std::max(3, std::min(r.width, r.height) / 2 - 2);

    // Optical disc outer platter
    s.drawRoundedRect(Rect{cx - rad, cy - rad, rad * 2, rad * 2}, rad, Color::fromHex(0x2D3748), true);
    s.drawRoundedRect(Rect{cx - rad, cy - rad, rad * 2, rad * 2}, rad, discCol, false);

    // Iridescent reflection highlight arcs
    if (rad >= 4) {
        s.drawRoundedRect(Rect{cx - rad + 2, cy - rad + 2, (rad - 2) * 2, (rad - 2) * 2}, rad - 2, Color::fromHex(0x38BDF8), false);
        const int32_t holeR = std::max(1, rad / 3);
        s.drawRoundedRect(Rect{cx - holeR, cy - holeR, holeR * 2, holeR * 2}, holeR, Color::fromHex(0x0F172A), true);
        s.drawRoundedRect(Rect{cx - holeR, cy - holeR, holeR * 2, holeR * 2}, holeR, Color::fromHex(0xE2E8F0), false);
    }

    // Drive tray slot at bottom
    const int32_t trayH = std::max(2, r.height / 6);
    const Rect trayRect{r.x + 2, r.bottom() - trayH - 1, r.width - 4, trayH};
    s.drawRoundedRect(trayRect, 1, Color::fromHex(0x1E293B), true);
    s.drawRoundedRect(trayRect, 1, Color::fromHex(0x475569), false);
    s.putPixel(trayRect.right() - 2, trayRect.y + 1, Color::fromHex(0x00FF9D));
}

} // namespace surshell

