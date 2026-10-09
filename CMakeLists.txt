cmake_minimum_required(VERSION 3.25)
project(SurShell VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core library sources
set(SURSHELL_SOURCES
    src/theme.cpp
    src/compositor.cpp
    src/desktop.cpp
    src/tray.cpp
    src/start_menu.cpp
    src/taskbar.cpp
    src/window_manager.cpp
    src/explorer.cpp
    src/quick_settings.cpp
    src/virtual_desktop.cpp
    src/kernel_bridge.cpp
    src/text_viewer.cpp
    src/icons.cpp
    src/alt_tab.cpp
    src/task_manager.cpp
    src/toast.cpp
    src/media_hud.cpp
    src/settings.cpp
    src/calculator.cpp
    src/run_dialog.cpp
    src/terminal.cpp
    src/action_center.cpp
    src/search_hub.cpp
    src/lock_screen.cpp
    src/registry_editor.cpp
    src/image_viewer.cpp
    src/paint.cpp
    src/sysinfo.cpp
    src/devicemanager.cpp
    src/diskmgmt.cpp
    src/services.cpp
    src/eventviewer.cpp
    src/surshell.cpp
)

# Static library
add_library(surshell STATIC ${SURSHELL_SOURCES})
target_include_directories(surshell PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
if (WIN32)
    target_link_libraries(surshell PUBLIC shell32 psapi advapi32 winmm iphlpapi)
endif()

# Compiler warnings and options
if (MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Main Interactive Shell Executable
add_executable(surshell_app src/main.cpp)
if (WIN32)
    target_link_libraries(surshell_app PRIVATE surshell user32 gdi32 dwmapi)
else()
    target_link_libraries(surshell_app PRIVATE surshell)
endif()

# Standalone Sovereign Registry Editor Executable (regedit.exe)
add_executable(regedit src/regedit_main.cpp)
if (WIN32)
    target_link_libraries(regedit PRIVATE surshell user32 gdi32 dwmapi)
else()
    target_link_libraries(regedit PRIVATE surshell)
endif()

# Standalone Test Suite Executable
add_executable(surshell_tests tests/test_main.cpp)
target_link_libraries(surshell_tests PRIVATE surshell)

enable_testing()
add_test(NAME SurShellCoreTests COMMAND surshell_tests)

# Output binaries to bin/
set_target_properties(
    surshell_app
    regedit
    surshell_tests
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
