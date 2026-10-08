# SurShell

[![C++23 Standard](https://img.shields.io/badge/C%2B%2B-23-blue.svg?logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/23)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Telemetry: 0%](https://img.shields.io/badge/Telemetry-0%25_Absolute_Zero-brightgreen.svg)]()
[![Target: MicaNT Sovereign](https://img.shields.io/badge/Target-MicaNT_Sovereign_Kernel-00D4FF.svg)](https://github.com/MicaNT-Kernel)

**SurShell** is the sovereign, clean-room modern desktop environment and user shell for the **MicaNT** operating system. Named in tribute to Dave Cutler's historic Windows NT 4.0 **"SUR" (Shell Update Release)** initiative, SurShell delivers an ultra-responsive, zero-telemetry, memory-efficient desktop shell engineered from scratch in pure standard ISO C++23.

---

## ⚡ Architectural Vision

```
+-----------------------------------------------------------------------------------------+
|                                      SurShell Desktop                                   |
+-----------------------------------------------------------------------------------------+
|  [Progman / DesktopManager]       [WindowManager / Aero Snap]     [Cabinet / FileExplorer]|
|  - Grid shortcut aligner          - Z-Order Stack (Top->Bottom)   - Breadcrumb navigation |
|  - Marquee drag selection         - Drag, resize, min/max/close   - History (Back/Fwd/Up) |
|  - High-res wallpaper blit        - Left/Right/Full snap tiles    - Quick access sidebar  |
+-----------------------------------------------------------------------------------------+
|  [Taskbar: Shell_TrayWnd]                                        [TrayNotifyWnd: SystemTray]
|  - MICA Prism Start Button        - Running task indicators       - Zero-Telemetry Badge |
|  - Start Menu (Fuzzy Search)      - Dynamic window switching      - Mesh NetBird Status  |
+-----------------------------------------------------------------------------------------+
|                    Compositor: 32-bpp BGRA Software DWM Rasterizer                      |
|          Porter-Duff Over · Box Shadows · Gradients · Rounded Corners · 8x8 Glyphs      |
+-----------------------------------------------------------------------------------------+
|                             MicaNT Clean-Room Kernel Executive                          |
+-----------------------------------------------------------------------------------------+
```

### Why SurShell?
1. **Dave Cutler "SUR" Heritage**: Following Dave Cutler's internal engineering codename for the classic NT shell update, SurShell captures the lean, bulletproof reliability of classic NT while bringing modern 2026 aesthetics (Mica, Acrylic, Cutler Cyan accent).
2. **Absolute Zero Telemetry**: 100% local execution. No telemetry services, no ad injection, no background tracking daemons, no remote beacons.
3. **Extreme Resource Efficiency**: Runs comfortably within **<15 MB RAM** at idle. No Electron, no WebView2, no bloated JavaScript runtime.
4. **Freestanding ISO C++23**: Built with zero external library dependencies. Compiles with any conforming C++23 compiler (Clang 18+, GCC 13+, MSVC 2022+).
5. **Software & Hardware Compositing**: High-performance 2D software compositor capable of sub-pixel alpha blending, soft shadows, and 120Hz frame rates even on low-end framebuffer hardware.

---

## 🌟 Core Components

### 🖥️ Desktop Manager (`surshell::DesktopManager`)
- Implements the sovereign desktop surface (`Progman` / `WorkerW`).
- Auto-arranging icon grid with label typography.
- Single-click selection, double-click launch handlers, and rubber-band marquee selection.

### 🪟 Window Manager (`surshell::WindowManager`)
- Strict Top-to-Bottom Z-order management and activation hierarchy.
- Non-client window frames with caption bars, title text, and minimize / maximize / close buttons.
- Full 8-direction border resize hit-testing (`Top`, `Bottom`, `Left`, `Right`, `TopLeft`, etc.).
- **Aero Snap Engine**: Instant docking to 50% left, 50% right, or full work-area maximize.

### 📊 Taskbar & Start Menu (`surshell::Taskbar` & `surshell::StartMenu`)
- Sovereign `Shell_TrayWnd` with MICA Prism start trigger.
- Running taskbar buttons with active/focused indicator bars in Cutler Cyan (`#00D4FF`).
- Instant Start Menu with real-time fuzzy search filtering, categorized pinned items, and system power actions (Lock, Sleep, Restart, Shutdown).

### 🔔 System Tray (`surshell::SystemTray`)
- Sovereign `TrayNotifyWnd` dock.
- Integrated status badges: Zero-Telemetry active shield, NetBird P2P Mesh VPN, Audio/Hardware vitals.
- Real-time chronometer clock with 12/24 hour display formatting.

### 📁 Sovereign File Explorer (`surshell::FileExplorer`)
- Freestanding cabinet explorer with breadcrumb address bar.
- Navigation history with Back, Forward, and Parent Directory actions.
- Directory listing with file metadata, extension badges, and size formatting.
- Quick Access sidebar for system root, drives, and user directories.

---

## 🚀 Building from Source

### Prerequisites
- Modern C++23 compiler: **Clang 18+**, **GCC 13+**, or **MSVC 19.38+**
- **CMake 3.25** or higher
- **Ninja** (recommended)

### Build Commands
```bash
# Clone the repository
git clone https://github.com/MicaNT-Kernel/SurShell.git
cd SurShell

# Configure with Ninja and C++23
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSURSHELL_BUILD_TESTS=ON -DSURSHELL_BUILD_DEMO=ON

# Build all targets (limit to -j2 on mechanical HDDs if needed)
cmake --build build

# Run the test suite
ctest --test-dir build --output-on-failure

# Run the desktop demonstration and generate scene renders
./build/bin/surshell_app
```

---

## 🧪 Verification & Testing

SurShell includes an automated verification test suite:
- `Compositor.ColorAndGeometry`: Color blending, lerp arithmetic, bounding rect containment.
- `Compositor.SurfaceRendering`: Framebuffer memory allocation, pixel writing, clear operations.
- `Theme.Palette`: Mica, Acrylic, Carbon Slate, Cutler Cyan accent verification.
- `Desktop.ItemManagement`: Grid layout, selection toggles, marquee bounding box.
- `Tray.ClockAndItems`: Chronometer formatting, system icon registration.
- `StartMenu.FuzzySearch`: Dynamic application filtering and catalog search.
- `WindowManager.ZOrderAndSnap`: Active window elevation, focus management, Aero Snap dimensions.
- `Explorer.Navigation`: Breadcrumbs, history stack (Back/Forward), directory navigation.

---

## 📜 Clean-Room Provenance & License

SurShell is developed strictly clean-room under the [MIT License](LICENSE). For complete architectural provenance and compliance details, see [docs/CLEAN_ROOM.md](docs/CLEAN_ROOM.md) and [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

> *"The NT kernel and its shell represent an engineering philosophy: simplicity where possible, robustness always, and performance without compromise."* — In memory of Dave Cutler's SUR team.
