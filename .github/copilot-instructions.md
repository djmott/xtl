# XTL (eXtended Template Library) - Copilot Instructions

## Project Overview

XTL is a C++ template metaprogramming library that extends and supplements the STL with advanced template patterns, idioms, and algorithms. The library is designed to be mostly header-only, emphasizing modern C++11/17 features and cross-platform compatibility.

**Key Features:**
- Template metaprogramming utilities and patterns
- Parser combinators for text parsing and AST generation
- Concurrent data structures (hash maps, queues, stacks with spin locks)
- Network socket abstractions
- String manipulation utilities
- Dynamic library loading
- Natural language processing components
- RFC grammar implementations

## Architecture

### Directory Structure

```
xtl/
├── include/xtd/           # Main header files (header-only components)
│   ├── concurrent/        # Concurrent data structures
│   ├── com/              # COM interfaces (Windows)
│   ├── grammars/         # RFC and standard grammar implementations
│   ├── nlp/              # Natural language processing
│   └── windows/          # Windows-specific utilities
├── src/                  # Compiled runtime components
├── tests/                # Google Test unit and system tests
├── examples/             # Usage examples for each component
├── docs/                 # Documentation and Doxygen configuration
└── CMakeLists.txt        # Build configuration
```

### Build System

- **Build Tool:** CMake (minimum version 3.23)
- **C++ Standard:** C++17
- **Testing Framework:** Google Test (fetched via FetchContent)
- **Documentation:** Doxygen

**Build Process:**
```bash
mkdir build && cd build
cmake .. -DXTD_BUILD_TESTS=ON -DXTD_BUILD_EXAMPLES=ON
make
```

**Configuration:**
- The main header `include/xtd/xtd.hpp` is generated from `xtd.hpp.in` by CMake
- Platform detection and feature detection handled during CMake configuration
- Optional dependencies: libiconv (Unicode on POSIX), libuuid (UUID support on POSIX)

### Testing

Run tests with:
```bash
cd build
make unit_tests
./tests/unit_tests
```

Test files are located in `tests/` with naming pattern `test_*.hpp` and are included conditionally in `tests.cpp` based on CMake configuration flags.

## Coding Conventions

### File Organization

1. **Header Files:**
   - Use `#pragma once` for include guards
   - Start with Doxygen comment block:
     ```cpp
     /** @file
     Brief description
     @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
     @example example_filename.cpp
     */
     ```
   - Include `<xtd/xtd.hpp>` first for platform/compiler configuration
   - All code in `namespace xtd { ... }`

2. **Naming Conventions:**
   - Types and classes: `lower_case_with_underscores`
   - Template parameters: `_suffix_t` (e.g., `_ChT`, `_return_t`, `_arg_ts`)
   - Private/internal namespaces: Use nested `namespace _` for implementation details
   - Macros: `UPPER_CASE` (e.g., `STRING_()`, `CHARACTERS_()`)
   - Member variables: Prefix with `_` when needed for clarity (e.g., `_super_t`)

3. **Template Style:**
   - Heavy use of template metaprogramming
   - Type traits and SFINAE patterns
   - Variadic templates for flexible interfaces
   - Use `#if (!DOXY_INVOKED)` to hide implementation details from Doxygen

### Code Style

- **Indentation:** 2 spaces (no tabs)
- **Braces:** K&R style (opening brace on same line)
- **Comments:** Doxygen-style for public APIs
- **Const correctness:** Methods and parameters marked `const` where appropriate
- **Modern C++:** Prefer `auto`, range-based for loops, smart pointers, `nullptr`
- **NOSONAR comments:** Used to suppress SonarQube false positives

### Platform Abstractions

The library uses conditional compilation based on CMake-detected features:
- `XTD_OS_WINDOWS`, `XTD_OS_UNIX`, `XTD_OS_Linux`, etc. for OS detection
- `XTD_COMPILER_MSVC`, `XTD_COMPILER_GCC`, etc. for compiler detection
- `XTD_HAS_CODECVT`, `XTD_HAS_FILESYSTEM`, etc. for feature detection

Always check `xtd.hpp` for available platform macros when writing platform-specific code.

## Component Patterns

### Header-Only Components

Most components are header-only templates. Pattern:
```cpp
#pragma once
#include <xtd/xtd.hpp>
// other includes...

namespace xtd {
  template <typename T>
  class my_component {
    // Implementation directly in header
  };
}
```

### Parser Combinators (parse.hpp)

The library includes a powerful parser combinator framework:
- Use macros like `STRING_()`, `CHARACTER_()`, `REGEX()` to define terminals
- Combine with operators: `|` (or), `>>` (sequence), `*` (zero or more), `+` (one or more)
- See `docs/Parsing.md` and examples in `examples/example_parse*.cpp`

### Callback Pattern (callback.hpp)

Single-producer, multiple-subscriber callback system:
```cpp
xtd::callback<void(int)> my_callback;
my_callback.connect([](int x) { /* handle event */ });
my_callback(42);  // Invoke all subscribers
```

### Concurrent Components

Thread-safe data structures using spin locks:
- `xtd::concurrent::spin_lock` - Basic spin lock
- `xtd::concurrent::rw_lock` - Reader-writer lock
- `xtd::concurrent::hash_map` - Concurrent hash map
- `xtd::concurrent::queue`, `stack` - Thread-safe containers

## Testing Guidelines

### Test Structure

Tests use Google Test framework:
```cpp
TEST(test_component_name, test_case_name) {
  // Arrange
  xtd::my_component obj;
  
  // Act & Assert
  ASSERT_NO_THROW(obj.method());
  ASSERT_EQ(expected, obj.value());
}
```

### Test Coverage Requirements

Contributions should aim for:
- ~80% code coverage
- Pass SonarQube quality gateway
- Pass Valgrind memcheck with no leaks
- Include tests for edge cases and error conditions

## Integration Points

### Adding New Components

1. Create header file in appropriate `include/xtd/` subdirectory
2. Add Doxygen comments with `@file`, `@copyright`, and `@example` tags
3. Update `CMakeLists.txt` to include in appropriate header list variable
4. Create example in `examples/` directory
5. Create test file in `tests/` directory with pattern `test_*.hpp`
6. Add conditional include in `tests/tests.cpp`

### External Dependencies

- **Minimal external dependencies** by design
- Optional: libiconv, libuuid, platform-specific libraries
- Test dependencies fetched via CMake FetchContent
- Prefer standard library facilities when available

## Documentation

- **Inline Documentation:** Use Doxygen comments (`/** ... */`)
- **Examples:** Each component should have a corresponding `examples/example_*.cpp` file
- **Markdown Docs:** Additional documentation in `docs/*.md` for complex topics
- **Generate Docs:** Run `make docs` after CMake configuration

### Doxygen Comment Style

```cpp
/** Brief description
 * 
 * Detailed description paragraph.
 * 
 * @tparam T Template parameter description
 * @param arg Parameter description
 * @return Return value description
 * @throws exception_type When this exception is thrown
 */
```

## Common Patterns

### Type Traits

Use template specialization and SFINAE for compile-time type checking:
```cpp
template <typename T, typename = void>
struct is_valid : std::false_type {};

template <typename T>
struct is_valid<T, std::void_t<decltype(T::value)>> : std::true_type {};
```

### String Operations

The `xtd::xstring` class extends `std::basic_string`:
```cpp
xtd::string str = "hello";
bool ends = str.ends_with("lo");  // true
auto formatted = xtd::string::format(/* args */);
```

### Resource Management

Follow RAII principles:
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- Destructors marked `= default` or `override`
- Move semantics for expensive-to-copy types

## CI/CD and Quality

### Continuous Integration

Travis CI configuration in `.travis.yml`:
- Automated builds for GCC on Ubuntu
- Static analysis: cppcheck, vera++, rats
- Dynamic analysis: Valgrind memcheck
- Code coverage: gcov/lcov with Coveralls integration
- SonarQube analysis for code quality

### Code Quality Tools

Before submitting:
- Run cppcheck with project suppressions (`assets/cppcheck-suppressions.txt`)
- Ensure no memory leaks with Valgrind
- Check SonarQube dashboard for issues

## License

All contributions must be compatible with the Boost Software License Version 1.0. Include the license header in new files:
```cpp
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
```

## Tips for AI Coding Agents

1. **Check Platform Macros:** Always verify platform-specific code paths using macros from `xtd.hpp`
2. **Template Errors:** Template compilation errors can be verbose; focus on the first error in the chain
3. **Header-Only Design:** Keep implementations in headers unless there's a strong reason for a .cpp file
4. **Minimal Dependencies:** Avoid adding external dependencies; prefer STL or header-only solutions
5. **Documentation First:** Write Doxygen comments as you code, not as an afterthought
6. **Example-Driven:** If adding a component, create a working example to validate the API design
7. **Test Early:** Write tests alongside implementation to catch template instantiation issues
8. **CMake Integration:** Update `CMakeLists.txt` header lists when adding files so they're installed correctly
9. **Cross-Platform:** Test assumptions about types, sizes, and platform features
10. **Consult Docs:** Review `docs/*.md` for architectural patterns specific to parsing, sockets, and TMP techniques
