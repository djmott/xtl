# XTL Project - AI Agent Guidelines

This document provides guidelines for AI agents working on the XTL (eXtended Template Library) project.

## CRITICAL: Development Container Requirement

**THE MOST IMPORTANT RULE: All code execution MUST occur in a development container, NEVER on the host system.**

- **NEVER execute code, build commands, or run tests on the host system**
- **NEVER modify system files, install packages, or change host configuration**
- **ONLY source files may be changed on the host** (via mapped drive/volume)
- **ALL builds, tests, and executions MUST happen inside the dev container**
- The host system must remain completely unchanged and protected from manipulation
- This prevents host corruption, security issues, and ensures a clean development environment

**Violation of this rule is a critical error.**

## Project Context

XTL is a C++17 header-only template library that supplements and extends the STL. It provides frequently used components that are otherwise absent from the standard library. The project uses template metaprogramming patterns, idioms, algorithms, and libraries to solve various programming tasks.

## Build System

- **Always use `.build` folder** for CMake builds. Never use `build` or other directory names.
- When configuring CMake, use: `cmake -S . -B .build`
- Always enable `compile_commands.json` for better IDE support: `cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- The project uses CMake 3.22 or higher

## Code Style

- **Follow existing patterns strictly** - maintain consistency with the current codebase
- Match the coding style, naming conventions, and formatting of existing files
- When in doubt, examine similar files in the codebase for reference
- Use the same indentation, brace style, and comment style as existing code

## Documentation

- **Always add Doxygen-style comments** for:
  - New functions and methods
  - New classes and structs
  - Complex logic and algorithms
  - Template parameters and constraints
- Use `@param`, `@return`, `@throws`, `@tparam` tags where appropriate
- Include brief descriptions and detailed explanations for non-trivial code

## Testing

- **Only write or update tests when explicitly requested** by the user
- Do not automatically create tests for new code
- When tests are requested, use the Google Test framework (gtest) as used in the `tests/` directory
- Tests should be placed in the appropriate test file in the `tests/` directory

## Error Handling

- Use **standard C++ exceptions** where appropriate:
  - `std::exception` for base exceptions
  - `std::runtime_error` for runtime errors
  - `std::invalid_argument` for invalid arguments
  - `std::logic_error` for logic errors
  - Other standard exception types as appropriate
- Do not use `xtd::exception` unless the existing code in a file already uses it

## Warning Levels

- Respect the configurable warning level set in CMake (`XTD_WARNING_LEVEL`)
- The warning level can be set from 0-4 (similar to MSVC warning levels)
- Default warning level is 2
- When adding new code, ensure it compiles cleanly at the configured warning level
- Do not suppress warnings unless absolutely necessary and with proper justification

## Compiler Support

Code must work with the following compilers on Windows and Linux:
- **MSVC** (Microsoft Visual C++)
- **Clang** (LLVM Clang)
- **GCC** (GNU Compiler Collection)
- **Intel C++** (Intel C++ Compiler)

When writing code:
- Avoid compiler-specific extensions unless absolutely necessary
- Use standard C++17 features
- Test compatibility considerations for all supported compilers
- Use platform detection macros (`XTD_OS_WINDOWS`, `XTD_OS_Linux`, etc.) when needed

## CMake Configuration

When configuring CMake:
- Always use the `.build` directory: `cmake -S . -B .build`
- Enable compile commands: `cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- Enable tests if needed: `cmake -DXTD_BUILD_TESTS=ON`
- Enable examples if needed: `cmake -DXTD_BUILD_EXAMPLES=ON`
- Set warning level if needed: `cmake -DXTD_WARNING_LEVEL=3`

## File Structure

- Header files: `include/xtd/`
- Source files: `src/`
- Tests: `tests/`
- Examples: `examples/`
- Build output: `.build/`

## Additional Notes

- The project uses C++17 standard (`CMAKE_CXX_STANDARD 17`)
- Most components are header-only
- Some components require linking to runtime components
- The project uses Boost Software License Version 1.0
- Online documentation: http://djmott.github.io/xtl

