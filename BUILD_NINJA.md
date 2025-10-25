# Building with Ninja

This project supports building with Ninja for faster incremental builds.

## Prerequisites

Install Ninja if not already installed:

### Ubuntu/Debian
```bash
sudo apt-get install ninja-build
```

### macOS
```bash
brew install ninja
```

### Arch Linux
```bash
sudo pacman -S ninja
```

## Building with Ninja

### Basic build
```bash
mkdir -p .build
cd .build
cmake -G Ninja ..
ninja
```

### Build with tests
```bash
mkdir -p .build
cd .build
cmake -G Ninja -DXTD_BUILD_TESTS=TRUE ..
ninja
```

### Run tests
```bash
cd .build
./tests/unit_tests
```

### Clean build
```bash
cd .build
ninja clean
```

### Build specific target
```bash
cd .build
ninja unit_tests
```

## Ninja vs Make

### Advantages of Ninja
- **Faster builds**: Ninja has lower overhead than Make
- **Better parallelism**: More efficient handling of parallel jobs
- **Incremental builds**: Faster subsequent builds
- **Cleaner output**: More concise build status

### Performance Comparison
On typical projects, Ninja builds are 10-30% faster than Make, especially for incremental builds.

## Tips

1. **Automatic dependency tracking**: Ninja automatically tracks dependencies for faster rebuilds
2. **Parallel builds**: Use `ninja -j<cores>` to specify number of parallel jobs (defaults to number of cores)
3. **Verbose output**: Use `ninja -v` for verbose output similar to `make VERBOSE=1`
4. **Time builds**: Use `time ninja` to see build time

## Backward Compatibility

The project still supports traditional Make builds. Simply omit the `-G Ninja` flag:

```bash
cmake ..
make
```
