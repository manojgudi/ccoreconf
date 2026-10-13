cmake_minimum_required(VERSION 3.16)
project(temperature_demo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# The ccoreconf library lives two levels up: temperature_sensor -> examples -> ccoreconf.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/ccoreconf-build)

add_executable(temperature_demo
    main.c
    include_model/test-temperature-sensor.c
)

target_include_directories(temperature_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include_model)
target_compile_definitions(temperature_demo PRIVATE CCORECONF_ENABLE_SID_MACROS)

# ccoreconf publishes its own include/ dir and links nanocbor PUBLIC, so both
# reach main.c transitively.
target_link_libraries(temperature_demo PRIVATE ccoreconf)
