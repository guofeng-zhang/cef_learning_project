# Minimal CEF project that links against an official CEF binary distribution.
# Follows the "Example 2" pattern described in the CEF binary distribution's
# top-level CMakeLists.txt / README.md.

cmake_minimum_required(VERSION 3.21)
set(CMAKE_CONFIGURATION_TYPES Debug Release)
project(cefsimple_demo CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ---------------------------------------------------------------------------
# Point CEF_ROOT at our self-built CEF distribution (produced by build_cef.sh).
# Can be overridden via `-DCEF_ROOT=...` or the CEF_ROOT environment variable,
# e.g. to fall back to the official download.
# ---------------------------------------------------------------------------
if(NOT DEFINED CEF_ROOT AND NOT DEFINED ENV{CEF_ROOT})
    set(CEF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../my_cef"
        CACHE PATH "Path to the CEF binary distribution root")
endif()

if(NOT IS_DIRECTORY "${CEF_ROOT}")
    message(FATAL_ERROR
        "CEF_ROOT is not set to a valid directory: '${CEF_ROOT}'. "
        "Pass -DCEF_ROOT=/path/to/cef_binary_... when configuring.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)

# Build the wrapper static library shipped with the binary distribution.
add_subdirectory("${CEF_LIBCEF_DLL_WRAPPER_PATH}" libcef_dll_wrapper)

# Our own cefsimple executable target.
add_subdirectory(cefsimple)

PRINT_CEF_CONFIG()
