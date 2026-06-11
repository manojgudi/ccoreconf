cmake_minimum_required(VERSION 3.15)

project(ccoreconf
    VERSION 1.0.0
    DESCRIPTION "CORECONF (CoAP Management Interface) library"
    LANGUAGES C
)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags matching the Makefile
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=strict-prototypes -Werror=format")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=old-style-definition -Werror=cast-align")

# Option to build examples
option(BUILD_EXAMPLES "Build example programs" ON)

# Find nanocbor library
# Users can specify NANOCBOR_INCLUDE and NANOCBOR_BUILD via cmake -D flags
if(DEFINED ENV{NANOCBOR_INCLUDE} OR NANOCBOR_INCLUDE)
    if(NOT NANOCBOR_INCLUDE)
        set(NANOCBOR_INCLUDE $ENV{NANOCBOR_INCLUDE})
    endif()
    include_directories(${NANOCBOR_INCLUDE})
endif()

if(DEFINED ENV{NANOCBOR_BUILD} OR NANOCBOR_BUILD)
    if(NOT NANOCBOR_BUILD)
        set(NANOCBOR_BUILD $ENV{NANOCBOR_BUILD})
    endif()
    link_directories(${NANOCBOR_BUILD})
endif()

# Library source files
set(CCORECONF_SOURCES
    src/coreconfManipulation.c
    src/coreconfTypes.c
    src/hashmap.c
    src/serialization.c
    src/sid.c
    src/sid_handlers.c
)

# Library header files
set(CCORECONF_HEADERS
    include/coreconfManipulation.h
    include/coreconfTypes.h
    include/hashmap.h
    include/serialization.h
    include/sid.h
    include/sid_handlers.h
)

# Create static library
add_library(ccoreconf STATIC ${CCORECONF_SOURCES} ${CCORECONF_HEADERS})

# Set include directories for the library
target_include_directories(ccoreconf
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link nanocbor
target_link_libraries(ccoreconf PUBLIC nanocbor)

# Set output directory
set_target_properties(ccoreconf PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Build examples if requested
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation rules
install(TARGETS ccoreconf
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(FILES ${CCORECONF_HEADERS}
    DESTINATION include/ccoreconf
)

# Print configuration summary
message(STATUS "")
message(STATUS "ccoreconf configuration summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build examples: ${BUILD_EXAMPLES}")
if(NANOCBOR_INCLUDE)
    message(STATUS "  nanocbor include: ${NANOCBOR_INCLUDE}")
endif()
if(NANOCBOR_BUILD)
    message(STATUS "  nanocbor library: ${NANOCBOR_BUILD}")
endif()
message(STATUS "")
