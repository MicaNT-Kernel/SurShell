// ============================================================================
// SurShell: Sovereign Clean-Room Modern ISO C++23 Desktop Shell for MicaNT
// (include/surshell/explorer.hpp)
//
// File Cabinet & Navigation Explorer (CabinetWnd), path breadcrumbs,
// virtual file system browser, and file execution dispatch.
// ============================================================================

#pragma once

#include "types.hpp"
#include "compositor.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <optional>

namespace surshell {

struct FileItem {
    std::string name;
    std::string fullPath;
    bool isDirectory{false};
    uint64_t sizeBytes{0};
    std::string iconGlyph{"[F]"};
    Rect bounds{};
    bool selected{false};
};

class FileExplorer {
public:
    using FileExecuteCallback = std::function<void(const std::string& path)>;

    explicit FileExplorer(std::string initialPath = "C:\\");

    void navigateTo(std::string path);
    void navigateUp();
    void navigateBack();
    void navigateForward();

    [[nodiscard]] const std::string& currentPath() const noexcept { return currentPath_; }
    [[nodiscard]] const std::vector<FileItem>& items() const noexcept { return items_; }

    void setExecuteCallback(FileExecuteCallback cb) { executeCallback_ = std::move(cb); }

    void onMouseDown(Point localPt, MouseButton button, Rect clientBounds);
    void onDoubleClick(Point localPt, Rect clientBounds);

    void render(Surface& clientSurface);

private:
    std::string currentPath_{"C:\\"};
    std::vector<std::string> backHistory_{};
    std::vector<std::string> forwardHistory_{};
    std::vector<FileItem> items_{};
    int32_t selectedIndex_{-1};

    FileExecuteCallback executeCallback_{};

    void refreshDirectory();
};

} // namespace surshell
