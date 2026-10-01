cmake_minimum_required(VERSION 3.16)
project(schc_demo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Don't rebuild the library's own examples/ from inside this demo.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# The ccoreconf library lives two levels up: multi_sid -> examples -> ccoreconf.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/ccoreconf-build)

add_executable(schc_demo
    main.c
    include_model/test-schc.c
)

target_include_directories(schc_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include_model)
target_compile_definitions(schc_demo PRIVATE CCORECONF_ENABLE_SID_MACROS)

# ccoreconf publishes its own include/ dir and links nanocbor PUBLIC, so both
# reach main.c transitively.
target_link_libraries(schc_demo PRIVATE ccoreconf)
