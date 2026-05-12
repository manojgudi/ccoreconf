# CMake Build Instructions

This document explains how to build the ccoreconf library using CMake.

## Prerequisites

- CMake 3.15 or higher
- GCC or compatible C compiler
- nanocbor library

## Basic Build

### 1. Configure the build

```bash
mkdir build
cd build
cmake ..
```

### 2. Build the library

```bash
cmake --build .
```

This will create:
- `build/libccoreconf.a` - The static library
- `build/examples/` - Example executables (if enabled)

## Specifying nanocbor Location

If nanocbor is installed in a non-standard location, you can specify its path:

### Using environment variables

```bash
export NANOCBOR_INCLUDE=/path/to/nanocbor/include
export NANOCBOR_BUILD=/path/to/nanocbor/lib
mkdir build && cd build
cmake ..
cmake --build .
```

### Using CMake variables

```bash
mkdir build && cd build
cmake -DNANOCBOR_INCLUDE=/path/to/nanocbor/include \
      -DNANOCBOR_BUILD=/path/to/nanocbor/lib \
      ..
cmake --build .
```

## Build Options

### Build without examples

```bash
cmake -DBUILD_EXAMPLES=OFF ..
cmake --build .
```

### Specify build type

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

## Installation

To install the library and headers to your system:

```bash
cd build
sudo cmake --install .
```

By default, this installs:
- Library to `/usr/local/lib/libccoreconf.a`
- Headers to `/usr/local/include/ccoreconf/`
- Examples to `/usr/local/bin/examples/` (if built)

To change the installation prefix:

```bash
cmake -DCMAKE_INSTALL_PREFIX=/your/custom/path ..
cmake --build .
cmake --install .
```

## Running Examples

After building with examples enabled:

```bash
# From the build directory
./examples/demo_functionalities_coreconf
./examples/coreconf_types_example
./examples/example_memory_tests
./examples/example_nanocbor
```

## Cleaning the Build

To clean and rebuild:

```bash
# Remove the build directory
rm -rf build

# Or use CMake's clean target
cd build
cmake --build . --target clean
```

## Integration with Other CMake Projects

To use ccoreconf in your CMake project:

```cmake
# Add ccoreconf subdirectory
add_subdirectory(path/to/ccoreconf)

# Link your target with ccoreconf
target_link_libraries(your_target PRIVATE ccoreconf)
```

Or if ccoreconf is installed system-wide:

```cmake
find_library(CCORECONF_LIB ccoreconf)
target_link_libraries(your_target PRIVATE ${CCORECONF_LIB})
target_include_directories(your_target PRIVATE /usr/local/include/ccoreconf)
```

## Troubleshooting

### nanocbor not found

If CMake cannot find nanocbor, ensure:
1. nanocbor is properly installed
2. `NANOCBOR_INCLUDE` and `NANOCBOR_BUILD` are set correctly
3. The nanocbor library file is named `libnanocbor.a` or `libnanocbor.so`

### Compiler errors

Make sure you're using a C11-compatible compiler:
```bash
cmake -DCMAKE_C_COMPILER=gcc ..
```

## Comparison with Make

The CMake build provides the same functionality as the Makefile:

| Makefile | CMake Equivalent |
|----------|------------------|
| `make` | `cmake --build .` |
| `make clean` | `cmake --build . --target clean` |
| `make ccoreconf.a` | `cmake --build . --target ccoreconf` |
| `NANOCBOR_INCLUDE=...` | `cmake -DNANOCBOR_INCLUDE=...` |

Both build systems can coexist in the repository.
