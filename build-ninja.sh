#!/bin/bash
# Build script using Ninja for faster builds

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Debug"
ENABLE_TESTS=0
TARGET=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --tests)
            ENABLE_TESTS=1
            shift
            ;;
        --target)
            TARGET="$2"
            shift 2
            ;;
        --clean)
            echo -e "${YELLOW}Cleaning build directory...${NC}"
            rm -rf .build
            echo -e "${GREEN}Clean complete.${NC}"
            exit 0
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --release      Build in Release mode (default: Debug)"
            echo "  --tests        Enable tests"
            echo "  --target TGT   Build specific target"
            echo "  --clean        Clean build directory"
            echo "  --help         Show this help message"
            echo ""
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Check if Ninja is installed
if ! command -v ninja &> /dev/null; then
    echo -e "${RED}Error: Ninja is not installed.${NC}"
    echo "Install it with: sudo apt-get install ninja-build"
    exit 1
fi

# Create build directory
mkdir -p .build
cd .build

# Configure with CMake
echo -e "${GREEN}Configuring with CMake...${NC}"
CMAKE_ARGS=(-G Ninja -DCMAKE_BUILD_TYPE="$BUILD_TYPE")

if [ $ENABLE_TESTS -eq 1 ]; then
    CMAKE_ARGS+=(-DXTD_BUILD_TESTS=TRUE)
fi

cmake "${CMAKE_ARGS[@]}" ..

# Build
echo -e "${GREEN}Building with Ninja...${NC}"
if [ -n "$TARGET" ]; then
    ninja "$TARGET"
else
    ninja
fi

# Run tests if enabled
if [ $ENABLE_TESTS -eq 1 ] && [ -z "$TARGET" ]; then
    echo -e "${GREEN}Running tests...${NC}"
    ./tests/unit_tests
fi

echo -e "${GREEN}Build complete!${NC}"
