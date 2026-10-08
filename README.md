# SurShell

[![C++23 Standard](https://img.shields.io/badge/C%2B%2B-23-blue.svg?logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/23)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Telemetry: 0%](https://img.shields.io/badge/Telemetry-0%25_Absolute_Zero-brightgreen.svg)]()
[![Target: MicaNT Sovereign](https://img.shields.io/badge/Target-MicaNT_Sovereign_Kernel-00D4FF.svg)](https://github.com/MicaNT-Kernel)
[![Build Status](https://img.shields.io/github/actions/workflow/status/MicaNT-Kernel/SurShell/ci.yml?branch=main&label=CI%20(Windows%20%2B%20Linux))](https://github.com/MicaNT-Kernel/SurShell/actions)

**SurShell** is the sovereign, clean-room modern desktop environment and user shell for the **MicaNT** operating system. Named in tribute to Dave Cutler's historic Windows NT 4.0 **"SUR" (Shell Update Release)** initiative, SurShell delivers an ultra-responsive, zero-telemetry, memory-efficient desktop shell engineered from scratch in pure standard ISO C++23.

SurShell implements the **"Mica Prism" Modern 2026 Sovereign Design Language**: centered floating island docks, detached tactile start hubs, 2D procedural vector iconography, and intelligent Snap Layout multitasking—engineered 100% clean-room with **zero copyright or trademark infringement**.

---

## ⚡ Architectural Vision

```
+-----------------------------------------------------------------------------------------+
|                                    SurShell Desktop                                     |
|                                                                                         |
|       +------------------------------------+                                            |
|       |  [File Explorer: C:\MicaNT]       |                                            |
|       |  +------------------------------+  |                                            |
|       |  |  Breadcrumb / Cabinet Tree   |  |                                            |
|       |  +------------------------------+  |                                            |
|       +------------------------------------+                                            |
|                                                                                         |
|                    +------------------------------------+                               |
|                    |         [START PRISM HUB]          |                               |
|                    |  [  Search MicaNT apps...   ]      |                               |
|                    |                                    |                               |
|                    |  [>_] Terminal    [E] Explorer     |                               |
|                    |  [T]  TaskMgr     [*] Settings     |                               |
|                    |  [S]  Sentinel    [N] NetBird      |                               |
|                    |  --------------------------------  |                               |
|                    |  ssfdre38              [Power v]   |                               |
|                    +------------------------------------+                               |
|                                                                                         |
|             +-------------------------------------------------------------+             |
|             |  [Prism]   [>_]  [E]  [T]   |   [Mesh] [0% Telemetry] 12:00 |             |
|             +-------------------------------------------------------------+             |
+-----------------------------------------------------------------------------------------+
                          Floating Centered Island Taskbar
```

### Why SurShell?
1. **Dave Cutler "SUR" Heritage**: Following Dave Cutler's internal engineering codename for the classic NT shell update, SurShell captures the lean, bulletproof reliability of classic NT while bringing modern 2026 aesthetics (Mica, Acrylic, Cutler Cyan accent).
2. **Absolute Zero Telemetry**: 100% local execution. No telemetry services, no ad injection, no background tracking daemons, no remote beacons.
3. **Extreme Resource Efficiency**: Runs comfortably within **<15 MB RAM** at idle. No Electron, no WebView2, no bloated JavaScript runtime.
4. **Clean-Room Vector Iconography**: Zero proprietary Microsoft icons or Segoe fonts. 100% original procedural vector glyphs (Mica Prism crystal, terminal cards, pulse waveforms, sovereign shields, mesh graphs).
5. **Freestanding ISO C++23**: Built with zero external library dependencies. Compiles with any conforming C++23 compiler (Clang 18+, GCC 13+, MSVC 2022+).
6. **Software & Hardware Compositing**: High-performance 2D software compositor capable of sub-pixel alpha blending, soft drop shadows, and 120Hz frame rates even on low-end framebuffer hardware.

---

## 🌟 Core Components

### 🖥️ Desktop Manager (`surshell::DesktopManager`)
- Implements the sovereign desktop surface (`Progman` / `WorkerW`).
- Auto-arranging icon grid with procedural vector icons.
- Single-click selection, double-click launch handlers, and rubber-band marquee selection.

### 🪟 Window Manager & Snap Layouts (`surshell::WindowManager`)
- Strict Top-to-Bottom Z-order management and activation hierarchy.
- Non-client window frames with caption bars, title text, and minimize / maximize / close buttons.
- Full 8-direction border resize hit-testing (`Top`, `Bottom`, `Left`, `Right`, `TopLeft`, etc.).
- **Snap Layouts Assistant HUD**: Hovering over the Maximize button triggers a floating preset picker:
  - **50 / 50 Dual Split**
  - **67 / 33 Priority Split** (Primary work area + reference sidebar)
  - **2x2 4-Quadrant Quad**

### 📊 Centered Floating Taskbar (`surshell::Taskbar`)
- Dual-island floating dock:
  - **App Island**: Centered horizontally with 12px rounded corners, holding the vector **Mica Prism** start button and running task buttons with Cutler Cyan (`#00D4FF`) active indicator pills.
  - **Tray Island**: Floating pill on the right for NetBird Mesh VPN status, Zero-Telemetry security shield, volume, and clock.
- Configurable alignment: `TaskbarAlignment::Center` (modern 2026) or `TaskbarAlignment::Left` (classic SUR).

### 🚀 Detached Start Prism Hub (`surshell::StartMenu`)
- Centered floating card with 14px rounded corners and 18px soft drop shadow.
- Integrated modern search pill with live fuzzy filtering across app IDs, titles, and executable paths.
- Tactile 2-column card grid with rich subtitles (e.g., "Command Prompt - Sovereign NT C++23 CLI").
- Bottom user profile footer (`ssfdre38` / `Administrator`) and Cutler power action buttons (Lock, Sleep, Restart, Shutdown).

### 📁 Sovereign File Explorer (`surshell::FileExplorer`)
- Freestanding cabinet explorer with breadcrumb address bar.
- Navigation history with Back, Forward, and Parent Directory actions.
- Directory listing with file metadata, extension badges, and size formatting.
- Quick Access sidebar for system root, drives, and user directories.

### ⚙️ Quick Settings & Action Center Island (`surshell::QuickSettingsFlyout`)
- Modern 2026 floating Action Center card anchored to the System Tray Island.
- Tactile toggle grid:
  - **RazzleNet Mesh**: Instant peer mesh status & toggle.
  - **SentinelSec Shield**: Real-time malware scanning & intrusion detection.
  - **Night Light**: 4500K warm color temperature calibration.
  - **Focus Session / DND**: Quiet hours notification filter.
  - **Daytona Eco Mode**: Energy-saving power throttle.
  - **Prism 3D Spatial Audio**: HRTF surround sound engine.
- Interactive volume and brightness sliders with drag and click position tracking.
- Power telemetry: AC power status, battery percent, and Settings shortcut.

### 🗂️ Task View & Virtual Desktops (`surshell::VirtualDesktopManager`)
- Multi-workspace window isolation and switching.
- Centered floating Task View switcher strip with preview cards.
- Pin individual windows to appear across all virtual desktops.
- Hotkey and dock-level switching between isolated workflow environments.

### 🌉 MicaNT Executive LPC Syscall Bridge (`surshell::KernelBridge`)
- Clean-room Win32 / NT executive syscall abstraction conforming to Dave Cutler's `SurWin` (`micant::surwin`, `micant::user32`, `micant::csrss`).
- Connects directly to `\RPC_Control\SurWinLpc` for window station creation and message dispatching.
- Process spawning and lifecycle monitoring for sovereign kernel executables (`micant_kernel.exe`, `sentinel.exe`, `cmd.exe`).
- Seamless freestanding emulation fallback when executing on host developer systems.

### 🔮 Sub-Surface Mica Acrylic Blur Pipeline (`surshell::Surface::applyBoxBlur`)
- Fast, two-pass separable 1D horizontal + 1D vertical box blur running in $O(W \times H)$ time.
- Single-pass sliding accumulator window providing sub-millisecond frosted glass blur.
- Blends Mica dark slate and Cutler Cyan acrylic tints directly over blurred backdrops.

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

# Configure with CMake and C++23
cmake -B build -DCMAKE_BUILD_TYPE=Release -DSURSHELL_BUILD_TESTS=ON -DSURSHELL_BUILD_DEMO=ON

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
- `Compositor.SurfaceRendering`: Framebuffer memory allocation, pixel writing, clear operations, procedural vector shapes.
- `Theme.Palette`: Mica, Acrylic, Carbon Slate, Cutler Cyan accent verification.
- `Desktop.ItemManagement`: Grid layout, selection toggles, marquee bounding box, procedural icons.
- `Tray.ClockAndItems`: Chronometer formatting, system icon registration.
- `StartMenu.FuzzySearch`: Dynamic application filtering and catalog search.
- `WindowManager.ZOrderAndSnap`: Active window elevation, focus management, 50/50 and 67/33 priority snap layouts, Snap Assistant flyout.
- `Explorer.Navigation`: Breadcrumbs, history stack (Back/Forward), directory navigation.

---

## 📜 Clean-Room Provenance & License

SurShell is developed strictly clean-room under the [MIT License](LICENSE). For complete architectural provenance and compliance details, see [docs/CLEAN_ROOM.md](docs/CLEAN_ROOM.md) and [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

> *"The NT kernel and its shell represent an engineering philosophy: simplicity where possible, robustness always, and performance without compromise."* — In memory of Dave Cutler's SUR team.
