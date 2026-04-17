# cefsimple application target.
# Based on the official tests/cefsimple/CMakeLists.txt from the CEF binary
# distribution, trimmed down to Linux-only since that's what we're learning on.

set(CEF_TARGET "cefsimple")

set(CEFSIMPLE_SRCS
    browser_app.cc
    browser_app.h
    sub_process_app.cc
    sub_process_app.h
    simple_handler.cc
    simple_handler.h
    cefsimple_linux.cc
    simple_handler_linux.cc
)
source_group(cefsimple FILES ${CEFSIMPLE_SRCS})

# Logical target that resolves to Debug/libcef.so or Release/libcef.so
# depending on the build configuration.
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

# CEF_TARGET_OUT_DIR = <build>/cefsimple/<CONFIG> ; runtime files get staged here.
SET_CEF_TARGET_OUT_DIR()

add_executable(${CEF_TARGET} ${CEFSIMPLE_SRCS})
SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
target_link_libraries(${CEF_TARGET}
    libcef_lib
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS}
)

# Let `#include "simple_handler.h"` resolve from this directory.
target_include_directories(${CEF_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Run the executable from its own dir; $ORIGIN rpath finds libcef.so next to it.
set_target_properties(${CEF_TARGET} PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY "${CEF_TARGET_OUT_DIR}"
)

# Stage libcef.so + all other runtime files next to the binary.
COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}"   "${CEF_BINARY_DIR}"   "${CEF_TARGET_OUT_DIR}")
COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
if(EXISTS "${CEF_BINARY_DIR}/libminigbm.so")
    COPY_FILES("${CEF_TARGET}" "libminigbm.so" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
endif()

# Our build_cef.sh uses is_component_ffmpeg=true, so ffmpeg lives in a separate
# libffmpeg.so next to libcef.so. Upstream CEF's CEF_BINARY_FILES does not list
# it (official builds statically link ffmpeg into libcef.so), so stage it here.
if(EXISTS "${CEF_BINARY_DIR}/libffmpeg.so")
    COPY_FILES("${CEF_TARGET}" "libffmpeg.so" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
endif()

# chrome-sandbox must be setuid-root; run it through CEF's helper macro.
SET_LINUX_SUID_PERMISSIONS("${CEF_TARGET}" "${CEF_TARGET_OUT_DIR}/chrome-sandbox")
