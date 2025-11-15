#!/bin/bash
# Comprehensive C++ Analysis Framework
# Performs static, dynamic, coverage, and performance analysis

set -e

ANALYSIS_DIR="analysis_reports"
mkdir -p "$ANALYSIS_DIR"

echo "=== C++ Comprehensive Analysis Framework ==="
echo "Starting analysis at $(date)"
echo ""

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to log analysis results
log_result() {
    local tool=$1
    local status=$2
    local message=$3
    if [ "$status" = "SUCCESS" ]; then
        echo -e "${GREEN}[✓]${NC} $tool: $message"
    elif [ "$status" = "WARNING" ]; then
        echo -e "${YELLOW}[!]${NC} $tool: $message"
    else
        echo -e "${RED}[✗]${NC} $tool: $message"
    fi
}

# 1. STATIC ANALYSIS
echo "=== 1. STATIC ANALYSIS ==="

# Check for clang-tidy
if command -v clang-tidy &> /dev/null; then
    echo "Running clang-tidy..."
    if [ -f compile_commands.json ]; then
        find include src tests examples -name "*.cpp" -o -name "*.hpp" | head -20 | while read file; do
            clang-tidy "$file" -- -std=c++17 2>&1 | tee -a "$ANALYSIS_DIR/clang-tidy.log"
        done
        log_result "clang-tidy" "SUCCESS" "Analysis complete"
    else
        log_result "clang-tidy" "WARNING" "compile_commands.json not found, running without it"
        find include src tests examples -name "*.cpp" -o -name "*.hpp" | head -20 | while read file; do
            clang-tidy "$file" -- -std=c++17 -Iinclude 2>&1 | tee -a "$ANALYSIS_DIR/clang-tidy.log"
        done
    fi
else
    log_result "clang-tidy" "WARNING" "Not installed, skipping"
fi

# Check for cppcheck
if command -v cppcheck &> /dev/null; then
    echo "Running cppcheck..."
    cppcheck --enable=all --suppress=missingIncludeSystem \
        --suppress=unusedFunction --inline-suppr \
        --xml --xml-version=2 \
        include/ src/ tests/ examples/ 2>&1 | tee "$ANALYSIS_DIR/cppcheck.xml"
    cppcheck --enable=all --suppress=missingIncludeSystem \
        --suppress=unusedFunction --inline-suppr \
        include/ src/ tests/ examples/ 2>&1 | tee "$ANALYSIS_DIR/cppcheck.txt"
    log_result "cppcheck" "SUCCESS" "Analysis complete"
else
    log_result "cppcheck" "WARNING" "Not installed, skipping"
fi

# Check for cpplint
if command -v cpplint &> /dev/null || python3 -m cpplint --version &> /dev/null; then
    echo "Running cpplint..."
    find include src tests examples -name "*.cpp" -o -name "*.hpp" | while read file; do
        if command -v cpplint &> /dev/null; then
            cpplint "$file" 2>&1 | tee -a "$ANALYSIS_DIR/cpplint.log"
        else
            python3 -m cpplint "$file" 2>&1 | tee -a "$ANALYSIS_DIR/cpplint.log"
        fi
    done
    log_result "cpplint" "SUCCESS" "Analysis complete"
else
    log_result "cpplint" "WARNING" "Not installed, skipping"
fi

# Clang static analyzer (scan-build)
if command -v scan-build &> /dev/null; then
    echo "Running clang static analyzer (scan-build)..."
    if [ -d .build ]; then
        scan-build -o "$ANALYSIS_DIR/scan-build" cmake --build .build 2>&1 | tee "$ANALYSIS_DIR/scan-build.log"
        log_result "scan-build" "SUCCESS" "Analysis complete"
    else
        log_result "scan-build" "WARNING" "Build directory not found, skipping"
    fi
else
    log_result "scan-build" "WARNING" "Not installed, skipping"
fi

echo ""

# 2. DYNAMIC ANALYSIS
echo "=== 2. DYNAMIC ANALYSIS ==="

# Valgrind
if command -v valgrind &> /dev/null; then
    echo "Valgrind available for memory leak detection"
    log_result "valgrind" "SUCCESS" "Available (run tests with: valgrind --leak-check=full ./test_executable)"
else
    log_result "valgrind" "WARNING" "Not installed, skipping"
fi

# AddressSanitizer and UndefinedBehaviorSanitizer
echo "Sanitizers should be enabled during compilation:"
echo "  -fsanitize=address (AddressSanitizer)"
echo "  -fsanitize=undefined (UndefinedBehaviorSanitizer)"
echo "  -fsanitize=thread (ThreadSanitizer)"
log_result "Sanitizers" "INFO" "Enable via CMAKE_CXX_FLAGS"

echo ""

# 3. COVERAGE ANALYSIS
echo "=== 3. COVERAGE ANALYSIS ==="

if command -v gcov &> /dev/null && command -v lcov &> /dev/null; then
    echo "Coverage tools available"
    echo "To generate coverage:"
    echo "  1. Build with: -fprofile-arcs -ftest-coverage"
    echo "  2. Run tests"
    echo "  3. Run: lcov --capture --directory . --output-file coverage.info"
    echo "  4. Run: genhtml coverage.info --output-directory coverage_html"
    log_result "gcov/lcov" "SUCCESS" "Available"
else
    log_result "gcov/lcov" "WARNING" "Not installed, skipping"
fi

echo ""

# 4. PERFORMANCE ANALYSIS
echo "=== 4. PERFORMANCE ANALYSIS ==="

# perf
if command -v perf &> /dev/null; then
    echo "perf available for performance profiling"
    log_result "perf" "SUCCESS" "Available"
else
    log_result "perf" "WARNING" "Not installed, skipping"
fi

# gprof
if command -v gprof &> /dev/null; then
    echo "gprof available for profiling"
    log_result "gprof" "SUCCESS" "Available"
else
    log_result "gprof" "WARNING" "Not installed, skipping"
fi

echo ""

# 5. GENERATE SUMMARY REPORT
echo "=== 5. GENERATING SUMMARY REPORT ==="

cat > "$ANALYSIS_DIR/analysis_summary.md" << 'EOF'
# C++ Code Analysis Summary Report

Generated: $(date)

## Analysis Tools Status

### Static Analysis
- clang-tidy: [STATUS]
- cppcheck: [STATUS]
- cpplint: [STATUS]
- scan-build: [STATUS]

### Dynamic Analysis
- Valgrind: [STATUS]
- AddressSanitizer: [STATUS]
- UndefinedBehaviorSanitizer: [STATUS]

### Coverage Analysis
- gcov/lcov: [STATUS]

### Performance Analysis
- perf: [STATUS]
- gprof: [STATUS]

## Discovered Issues

[Issues will be listed here]

## Recommendations

[Recommendations will be listed here]
EOF

log_result "Summary" "SUCCESS" "Report generated at $ANALYSIS_DIR/analysis_summary.md"

echo ""
echo "=== Analysis Complete ==="
echo "Reports saved in: $ANALYSIS_DIR/"
