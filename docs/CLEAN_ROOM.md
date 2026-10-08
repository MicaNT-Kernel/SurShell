# SurShell Clean-Room Provenance & Compliance

## 1. Clean-Room Methodology

**SurShell** is an original, clean-room implementation of a sovereign desktop shell environment designed for modern NT-compatible systems (specifically **MicaNT**). 

To ensure complete legal, ethical, and architectural integrity:

1. **No Proprietary Code Inspection**: At no point was any leaked, stolen, decompiled, or non-public Microsoft proprietary source code (e.g. Windows Research Kernel, leaked NT4/Win2k/Win10 source code, or internal symbol databases) referenced, inspected, or utilized in the development of SurShell.
2. **Black-Box Functional Alignment**: SurShell is architected exclusively from:
   - Officially published MSDN and Microsoft Learn technical documentation.
   - Public standard specifications (`win32metadata`, ISO/IEC 14882:2023 C++ standard).
   - Historical literature chronicling Dave Cutler's design philosophy (e.g., G. Pascal Zachary's *Showstopper!*, Helen Custer's *Inside Windows NT*).
   - Mathematical and computer graphics algorithms for 2D software rendering (Porter-Duff compositing, Bresenham line rasterization, bilinear interpolation).
3. **Freestanding Independence**: SurShell contains its own self-contained rasterizer, math types, event dispatching, and windowing abstractions without relying on proprietary platform SDK runtime libraries or undisclosed DLL entry points.

---

## 2. Taxonomy Alignment with Dave Cutler's Heritage

As documented in `MicaNT/docs/SOVEREIGN_TAXONOMY.md`:

| Component | Historical Cutler NT Codename | SurShell Clean-Room Equivalent | Description |
| :--- | :--- | :--- | :--- |
| **Shell & Desktop** | `SUR` / `SurWin` (Shell Update Release) | **SurShell** | Modern C++23 sovereign desktop shell and compositor |
| **Taskbar** | `Shell_TrayWnd` | `surshell::Taskbar` | Application anchor, clock, running task indicators |
| **Notification Area** | `TrayNotifyWnd` | `surshell::SystemTray` | Status indicators (mesh, security, volume, clock) |
| **Desktop Surface** | `Progman` / `WorkerW` | `surshell::DesktopManager` | Desktop canvas, shortcut grid, marquee selector |
| **Cabinet Browser** | `Explorer.exe` | `surshell::FileExplorer` | Sovereign file browser with breadcrumb navigation |
| **Window Frame** | Non-Client Manager | `surshell::WindowManager` | Non-client hit testing, Z-order stack, Aero Snap |

---

## 3. Privacy & Telemetry Guardrails

Modern commercial desktop shells are burdened with:
- Telemetry daemons transmitting user interactions to remote servers.
- Integrated advertising networks in search menus and taskbar widgets.
- Heavy web runtimes consuming hundreds of megabytes of RAM.

**SurShell Sovereign Guarantee**:
- **Zero Telemetry**: Not a single telemetry beacon or diagnostic payload exists in SurShell.
- **Offline First**: All search, file browsing, and application launching operations run 100% locally with zero external network pings.
- **Transparent Codebase**: The entire repository is published under the open-source MIT License for peer verification.
