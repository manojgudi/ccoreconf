cmake_minimum_required(VERSION 3.15)

project(ccoreconf
    VERSION 1.0.0
    DESCRIPTION "CORECONF (CoAP Management Interface) library"
    LANGUAGES C
)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(BUILD_EXAMPLES    "Build example programs"               ON)
option(BUILD_SHARED_LIBS "Build ccoreconf as a shared library"   OFF)
option(INSTALL_EXAMPLES  "Install example executables"          OFF)

# ---------------------------------------------------------------------------
# C language settings
# ---------------------------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF) # portable; use strict -std=c11

# Strict warnings, matching the original Makefile. Keep -Werror ON for CI
# builds; allow callers (e.g. distro packagers) to relax it via
# -DCWARN_AS_ERROR=OFF.
option(CWARN_AS_ERROR "Promote compiler warnings to errors" ON)

# ---------------------------------------------------------------------------
# nanocbor discovery
# ---------------------------------------------------------------------------
# Resolution order (first match wins):
#   1. -DNANOCBOR_INCLUDE / -DNANOCBOR_BUILD (or matching env vars) - explicit
#   2. find_package(nanocbor CONFIG) - installed nanocbor with a config file
#   3. find_library / find_path against standard locations
#
# After this block the following variables are guaranteed to be set:
#   NANOCBOR_INCLUDE_DIRS   - include paths (list)
#   NANOCBOR_LIBRARIES      - libraries to link (list)
#   NANOCBOR_FOUND          - TRUE if nanocbor was located

set(NANOCBOR_FOUND FALSE)

# --- 1. Explicit override --------------------------------------------------
if(DEFINED ENV{NANOCBOR_INCLUDE} AND NOT NANOCBOR_INCLUDE)
    set(NANOCBOR_INCLUDE $ENV{NANOCBOR_INCLUDE})
endif()
if(DEFINED ENV{NANOCBOR_BUILD} AND NOT NANOCBOR_BUILD)
    set(NANOCBOR_BUILD $ENV{NANOCBOR_BUILD})
endif()

if(NANOCBOR_INCLUDE AND NANOCBOR_BUILD)
    find_path(NANOCBOR_INCLUDE_DIR
        NAMES nanocbor/nanocbor.h
        HINTS "${NANOCBOR_INCLUDE}"
        NO_DEFAULT_PATH
    )
    find_library(NANOCBOR_LIBRARY
        NAMES nanocbor
        HINTS "${NANOCBOR_BUILD}"
        NO_DEFAULT_PATH
    )
    if(NANOCBOR_INCLUDE_DIR AND NANOCBOR_LIBRARY)
        set(NANOCBOR_FOUND TRUE)
        message(STATUS "nanocbor: using explicit paths "
                       "(include='${NANOCBOR_INCLUDE_DIR}', lib='${NANOCBOR_LIBRARY}')")
    else()
        message(FATAL_ERROR
            "NANOCBOR_INCLUDE / NANOCBOR_BUILD were set but nanocbor was not "
            "found there. include='${NANOCBOR_INCLUDE}', build='${NANOCBOR_BUILD}'.")
    endif()
endif()

# --- 2. find_package(nanocbor CONFIG) -------------------------------------
if(NOT NANOCBOR_FOUND)
    find_package(nanocbor CONFIG QUIET)
    if(nanocbor_FOUND)
        set(NANOCBOR_FOUND TRUE)
        message(STATUS "nanocbor: found via find_package (CONFIG)")
    endif()
endif()

# --- 3. Standard search fallback ------------------------------------------
if(NOT NANOCBOR_FOUND)
    find_path(NANOCBOR_INCLUDE_DIR
        NAMES nanocbor/nanocbor.h
    )
    find_library(NANOCBOR_LIBRARY
        NAMES nanocbor
    )
    if(NANOCBOR_INCLUDE_DIR AND NANOCBOR_LIBRARY)
        set(NANOCBOR_FOUND TRUE)
        message(STATUS "nanocbor: found via standard search "
                       "(include='${NANOCBOR_INCLUDE_DIR}', lib='${NANOCBOR_LIBRARY}')")
    endif()
endif()

if(NOT NANOCBOR_FOUND)
    message(FATAL_ERROR
        "nanocbor was not found. Install it, set -DNANOCBOR_INCLUDE and "
        "-DNANOCBOR_BUILD, or set the NANOCBOR_INCLUDE / NANOCBOR_BUILD "
        "environment variables. See CMAKE_BUILD.md for details.")
endif()

# Normalise to the public variable names used below.
set(NANOCBOR_INCLUDE_DIRS ${NANOCBOR_INCLUDE_DIR})
set(NANOCBOR_LIBRARIES    ${NANOCBOR_LIBRARY})

# ---------------------------------------------------------------------------
# Library target
# ---------------------------------------------------------------------------
set(CCORECONF_SOURCES
    src/coreconfManipulation.c
    src/coreconfModel.c
    src/coreconfTypes.c
    src/hashmap.c
    src/serialization.c
    src/sid.c
    src/sid_handlers.c
)

set(CCORECONF_HEADERS
    include/coreconfManipulation.h
    include/coreconfModel.h
    include/coreconfTypes.h
    include/hashmap.h
    include/serialization.h
    include/sid.h
    include/sid_handlers.h
)

add_library(ccoreconf ${CCORECONF_SOURCES} ${CCORECONF_HEADERS})

add_library(ccoreconf::ccoreconf ALIAS ccoreconf)

target_include_directories(ccoreconf
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${NANOCBOR_INCLUDE_DIRS}
)

target_link_libraries(ccoreconf
    PUBLIC ${NANOCBOR_LIBRARIES}
)

target_compile_features(ccoreconf PUBLIC c_std_11)

set_target_properties(ccoreconf PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# Strict warnings
set(_ccoreconf_warnings
    -Wall -Wextra -pedantic
    -Wstrict-prototypes -Wformat
    -Wold-style-definition -Wcast-align
)
if(CWARN_AS_ERROR)
    list(APPEND _ccoreconf_warnings -Werror)
endif()
target_compile_options(ccoreconf PRIVATE ${_ccoreconf_warnings})

# ---------------------------------------------------------------------------
# Examples
# ---------------------------------------------------------------------------
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ---------------------------------------------------------------------------
# Install + package config
# ---------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/ccoreconfConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ccoreconfConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ccoreconf
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ccoreconfConfigVersion.cmake"
    VERSION       ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(TARGETS ccoreconf
    EXPORT ccoreconfTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ccoreconfTargets
    FILE        ccoreconfTargets.cmake
    NAMESPACE   ccoreconf::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ccoreconf
)

install(FILES ${CCORECONF_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ccoreconf
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ccoreconfConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ccoreconfConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ccoreconf
)

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
message(STATUS "")
message(STATUS "ccoreconf configuration summary:")
message(STATUS "  Version:           ${PROJECT_VERSION}")
message(STATUS "  C compiler:        ${CMAKE_C_COMPILER}")
message(STATUS "  Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared library:    ${BUILD_SHARED_LIBS}")
message(STATUS "  Build examples:    ${BUILD_EXAMPLES}")
message(STATUS "  Install examples:  ${INSTALL_EXAMPLES}")
message(STATUS "  nanocbor include:  ${NANOCBOR_INCLUDE_DIRS}")
message(STATUS "  nanocbor library:  ${NANOCBOR_LIBRARIES}")
message(STATUS "")
