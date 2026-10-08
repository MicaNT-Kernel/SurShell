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
    src/surshell.cpp
)

# Static library
add_library(surshell STATIC ${SURSHELL_SOURCES})
target_include_directories(surshell PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Compiler warnings and optimization flags
if (MSVC)
    target_compile_options(surshell PRIVATE /W4 /permissive- /O2 /utf-8)
else()
    target_compile_options(surshell PRIVATE -Wall -Wextra -Wpedantic -O2)
endif()

# Main Interactive Shell Executable
add_executable(surshell_app src/main.cpp)
target_link_libraries(surshell_app PRIVATE surshell)

# Standalone Test Suite Executable
add_executable(surshell_tests tests/test_main.cpp)
target_link_libraries(surshell_tests PRIVATE surshell)

enable_testing()
add_test(NAME SurShellCoreTests COMMAND surshell_tests)

# Output binaries to bin/
set_target_properties(
    surshell_app
    surshell_tests
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
