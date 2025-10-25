# Build Instructions

## Overview

This project uses CMake for build configuration. Follow these instructions to build the project correctly.

## Prerequisites

- CMake 3.23 or higher
- C++17 compatible compiler
- (Optional) Ninja build system for faster builds

## Building the Project

### Configuration

Configure the project using CMake:

```bash
# Create a build directory
mkdir build && cd build

# Configure with default generator (Make on Unix, Visual Studio on Windows)
cmake -DXTD_BUILD_TESTS=TRUE ..

# Or configure with Ninja (faster builds)
cmake -G Ninja -DXTD_BUILD_TESTS=TRUE ..
```

### Building

**Important**: Always use `cmake --build` instead of calling the underlying build system directly (e.g., `make` or `ninja`).

✅ **Correct**:
```bash
cmake --build . --parallel
# or with specific target
cmake --build . --target unit_tests --parallel
```

❌ **Incorrect**:
```bash
make -j4        # Don't assume Make is being used
ninja           # Don't assume Ninja is being used
```

### Why use `cmake --build`?

1. **Generator-agnostic**: Works with any CMake generator (Make, Ninja, Visual Studio, Xcode, etc.)
2. **Portable**: Build scripts work across different platforms and environments
3. **Feature-rich**: Supports `--parallel`, `--target`, `--config`, etc.
4. **Standard**: Follows CMake's recommended practices

### Common Build Commands

```bash
# Build everything
cmake --build . --parallel

# Build specific target
cmake --build . --target unit_tests --parallel

# Build in Release mode (for multi-config generators)
cmake --build . --config Release --parallel

# Verbose output
cmake --build . --parallel --verbose
```

### Running Tests

```bash
# Build and run tests
cmake --build . --target unit_tests --parallel
./tests/unit_tests

# Or use CTest (recommended)
ctest
```

## Quick Start

```bash
# Complete build and test workflow
mkdir build && cd build
cmake -G Ninja -DXTD_BUILD_TESTS=TRUE ..
cmake --build . --parallel
ctest
```

## Troubleshooting

### Clean Build
```bash
rm -rf build && mkdir build && cd build
cmake ..
cmake --build . --parallel
```

### Check Generator
```bash
# Check which generator is being used
cd build
cat CMakeCache.txt | grep CMAKE_GENERATOR
```

## Additional Options

### Build Options

- `-DXTD_BUILD_TESTS=TRUE` - Build test suite
- `-DXTD_BUILD_EXAMPLES=TRUE` - Build example programs
- `-DXTD_VERBOSE_BUILD=TRUE` - Enable verbose output
- `-DCMAKE_BUILD_TYPE=Release` - Set build type (single-config generators)

### Useful CMake Variables

- `CMAKE_CXX_STANDARD` - C++ standard version (17 for this project)
- `CMAKE_BUILD_TYPE` - Build type (Debug, Release, etc.)
- `CMAKE_CXX_COMPILER` - Override compiler
- `BUILD_SHARED_LIBS` - Build shared libraries instead of static

## Examples

### Debug Build
```bash
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --parallel
```

### Release Build
```bash
mkdir build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
```

### Cross-Platform Script
```bash
#!/bin/bash
# Works on any platform with any generator

mkdir -p build && cd build
cmake -DXTD_BUILD_TESTS=TRUE ..
cmake --build . --parallel
ctest
```

## See Also

- [CMake Build Command Documentation](https://cmake.org/cmake/help/latest/manual/cmake.1.html#build-tool-mode)
- [CMake Generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)
- [Ninja Build System](https://ninja-build.org/)
