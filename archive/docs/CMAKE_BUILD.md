# CMake Build Instructions

This document explains how to build the ccoreconf library using CMake.

## Prerequisites

- CMake 3.15 or higher
- GCC or compatible C11 compiler
- [nanocbor](https://github.com/bergzand/NanoCBOR) (headers and library)

## Quick start

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

This produces:

- `build/libccoreconf.a` — static library (or `libccoreconf.so` if `-DBUILD_SHARED_LIBS=ON`)
- `build/examples/demo_functionalities_coreconf` — example executable

## Telling CMake where nanocbor lives

If nanocbor is not in a standard system location, point CMake at it. The
following forms are all equivalent and may be mixed:

```bash
# As -D variables (preferred for CI)
cmake -DNANOCBOR_INCLUDE=/path/to/nanocbor/include \
      -DNANOCBOR_BUILD=/path/to/nanocbor/build ..

# As environment variables
export NANOCBOR_INCLUDE=/path/to/nanocbor/include
export NANOCBOR_BUILD=/path/to/nanocbor/build
cmake ..
```

`NANOCBOR_INCLUDE` is the directory that contains `nanocbor/`
(e.g. `…/nanocbor/include` if `…/nanocbor/include/nanocbor/nanocbor.h` exists).
`NANOCBOR_BUILD` is the directory that contains `libnanocbor.a` or
`libnanocbor.so`.

If neither is set, CMake searches standard system paths automatically.
You can also point at a sysroot containing both via `-DCMAKE_PREFIX_PATH=/path`.

## Build options

| Option               | Default | Description                                              |
| -------------------- | :-----: | -------------------------------------------------------- |
| `BUILD_EXAMPLES`     |   ON    | Build the `examples/` programs                           |
| `INSTALL_EXAMPLES`   |   OFF   | Install example executables during `cmake --install`     |
| `BUILD_SHARED_LIBS`  |   OFF   | Build ccoreconf as a shared library                      |
| `CWARN_AS_ERROR`     |   ON    | Promote compiler warnings to errors                      |
| `CMAKE_BUILD_TYPE`   | (empty) | `Debug` / `Release` / `RelWithDebInfo` / `MinSizeRel`    |

```bash
# Release build, no examples
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF ..

# Shared library, install examples
cmake -DBUILD_SHARED_LIBS=ON -DINSTALL_EXAMPLES=ON ..
```

## Installation

```bash
cmake --install .                     # → /usr/local by default
cmake --install . --prefix /opt/foo   # custom prefix
```

Default layout:

```text
<prefix>/lib/libccoreconf.{a,so}
<prefix>/lib/cmake/ccoreconf/ccoreconfConfig.cmake
<prefix>/lib/cmake/ccoreconf/ccoreconfConfigVersion.cmake
<prefix>/lib/cmake/ccoreconf/ccoreconfTargets.cmake
<prefix>/include/ccoreconf/*.h
```

## Using ccoreconf from another CMake project

After `cmake --install`, downstream projects can do:

```cmake
find_package(ccoreconf REQUIRED)
target_link_libraries(my_app PRIVATE ccoreconf::ccoreconf)
```

This automatically pulls in nanocbor through ccoreconf's `PUBLIC` linkage.

For a tree-out-of-source build, use `add_subdirectory(path/to/ccoreconf)`;
no `find_package` is required in that case.

## Running the example

```bash
./examples/demo_functionalities_coreconf
```

## Cleaning

```bash
rm -rf build                       # nuke everything
cmake --build build --target clean # CMake-only clean (keeps cache)
```

## Troubleshooting

### "nanocbor was not found"

- Verify `NANOCBOR_INCLUDE` points to the directory containing `nanocbor/`.
- Verify `NANOCBOR_BUILD` contains `libnanocbor.a` or `libnanocbor.so`.
- Run `cmake ..` with `-DCMAKE_FIND_DEBUG_MODE=ON` to see exactly where
  CMake looked.

### Strict warnings fail your build

- Set `-DCWARN_AS_ERROR=OFF` to keep warnings non-fatal.

### Linker error for nanocbor when consuming ccoreconf

- Make sure `find_package(ccoreconf)` was used (which auto-pulls nanocbor),
  not raw `-lccoreconf`.
