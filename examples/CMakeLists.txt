# Examples CMakeLists.txt
#
# Only one example currently ships with ccoreconf:
#   * demo_functionalities_coreconf  - end-to-end demo of CORECONF data
#                                     manipulation, encoding and decoding
#                                     using the public API and nanocbor.

add_executable(demo_functionalities_coreconf
    demo_functionalities_coreconf.c
)

target_link_libraries(demo_functionalities_coreconf
    PRIVATE ccoreconf
)

# Place all example binaries under <build>/examples/
set_target_properties(demo_functionalities_coreconf
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

# Optional install (off by default; enable with -DINSTALL_EXAMPLES=ON)
option(INSTALL_EXAMPLES "Install example executables" OFF)
if(INSTALL_EXAMPLES)
    install(TARGETS demo_functionalities_coreconf
        RUNTIME DESTINATION bin/examples
    )
endif()
