# Examples CMakeLists.txt

# Demo functionalities example
add_executable(demo_functionalities_coreconf
    demo_functionalities_coreconf.c
)

target_link_libraries(demo_functionalities_coreconf
    PRIVATE ccoreconf
)

# CoreConf types example
add_executable(coreconf_types_example
    coreconf_types_example.c
)

target_link_libraries(coreconf_types_example
    PRIVATE ccoreconf
)

# Memory tests example
add_executable(example_memory_tests
    example_memory_tests.c
)

target_link_libraries(example_memory_tests
    PRIVATE ccoreconf
)

# NanoCBOR example
add_executable(example_nanocbor
    example_nanocbor.c
)

target_link_libraries(example_nanocbor
    PRIVATE
    ccoreconf
    jansson
)

# Set output directory for examples
set_target_properties(
    demo_functionalities_coreconf
    coreconf_types_example
    example_memory_tests
    example_nanocbor
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

# Installation (optional)
install(TARGETS
    demo_functionalities_coreconf
    coreconf_types_example
    example_memory_tests
    example_nanocbor
    RUNTIME DESTINATION bin/examples
)
