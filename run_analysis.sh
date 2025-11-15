#!/bin/bash
# Comprehensive C++ Code Analysis Script
# Performs static, dynamic, coverage, and performance analysis

set -e

ANALYSIS_DIR="analysis_reports"
mkdir -p "$ANALYSIS_DIR"

echo "=========================================="
echo "C++ Comprehensive Code Analysis"
echo "=========================================="
echo ""

# Find all C++ source files
echo "Finding C++ source files..."
CPP_FILES=$(find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) ! -path "./.build/*" ! -path "./.git/*" | sort)
echo "Found $(echo "$CPP_FILES" | wc -l) C++ files"
echo ""

# ==========================================
# STATIC ANALYSIS
# ==========================================
echo "=========================================="
echo "1. STATIC ANALYSIS"
echo "=========================================="

# 1.1 Clang-tidy
echo "Running clang-tidy..."
if [ -f .build/compile_commands.json ]; then
    clang-tidy --version > "$ANALYSIS_DIR/clang-tidy_version.txt" 2>&1 || true
    clang-tidy $(echo "$CPP_FILES" | head -50) -- -std=c++17 > "$ANALYSIS_DIR/clang-tidy_report.txt" 2>&1 || true
    echo "  ✓ Clang-tidy report: $ANALYSIS_DIR/clang-tidy_report.txt"
else
    echo "  ⚠ compile_commands.json not found, skipping clang-tidy"
fi
echo ""

# 1.2 Cppcheck
echo "Running cppcheck..."
cppcheck --version > "$ANALYSIS_DIR/cppcheck_version.txt" 2>&1 || true
cppcheck --enable=all --std=c++17 --suppress=missingIncludeSystem \
    --suppress=unusedFunction --suppress=unmatchedSuppression \
    --xml --xml-version=2 . 2> "$ANALYSIS_DIR/cppcheck_report.xml" || true
cppcheck --enable=all --std=c++17 --suppress=missingIncludeSystem \
    --suppress=unusedFunction --suppress=unmatchedSuppression . \
    2> "$ANALYSIS_DIR/cppcheck_report.txt" || true
echo "  ✓ Cppcheck report: $ANALYSIS_DIR/cppcheck_report.txt"
echo ""

# 1.3 Cpplint
echo "Running cpplint..."
cpplint --version > "$ANALYSIS_DIR/cpplint_version.txt" 2>&1 || true
cpplint --extensions=cpp,hpp,h --filter=-legal/copyright,-runtime/references \
    $(echo "$CPP_FILES" | head -100) > "$ANALYSIS_DIR/cpplint_report.txt" 2>&1 || true
echo "  ✓ Cpplint report: $ANALYSIS_DIR/cpplint_report.txt"
echo ""

# ==========================================
# BUILD ANALYSIS
# ==========================================
echo "=========================================="
echo "2. BUILD ANALYSIS"
echo "=========================================="

# Try to build with different configurations
echo "Attempting builds with various configurations..."

# Debug build
echo "  Building Debug configuration..."
cd .build
make clean 2>/dev/null || true
make xtl -j$(nproc) 2>&1 | tee "../$ANALYSIS_DIR/build_debug.txt" || true
cd ..
echo "  ✓ Debug build log: $ANALYSIS_DIR/build_debug.txt"
echo ""

# ==========================================
# COVERAGE ANALYSIS (if build succeeds)
# ==========================================
echo "=========================================="
echo "3. COVERAGE ANALYSIS"
echo "=========================================="

if [ -f .build/src/CMakeFiles/xtl.dir/xtl.cpp.o ]; then
    echo "Building with coverage instrumentation..."
    cd .build
    make clean 2>/dev/null || true
    
    # Reconfigure with coverage
    CC=gcc CXX=g++ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_FLAGS="-g -O0 -fprofile-arcs -ftest-coverage" \
        -DCMAKE_C_FLAGS="-g -O0 -fprofile-arcs -ftest-coverage" \
        -S .. -B . > /dev/null 2>&1 || true
    
    make xtl -j$(nproc) 2>&1 | tee "../$ANALYSIS_DIR/build_coverage.txt" || true
    cd ..
    
    # Find coverage files
    GCNO_FILES=$(find .build -name "*.gcno" 2>/dev/null || true)
    if [ -n "$GCNO_FILES" ]; then
        echo "  Found coverage instrumentation files"
        # Note: Actual coverage requires running tests, which may not compile
    else
        echo "  ⚠ No coverage files generated"
    fi
else
    echo "  ⚠ Skipping coverage (build artifacts not found)"
fi
echo ""

# ==========================================
# DYNAMIC ANALYSIS
# ==========================================
echo "=========================================="
echo "4. DYNAMIC ANALYSIS"
echo "=========================================="

# Check if we have binaries to analyze
if [ -f .build/src/libxtl.a ]; then
    echo "  Found library binary: .build/src/libxtl.a"
    file .build/src/libxtl.a > "$ANALYSIS_DIR/binary_info.txt" 2>&1 || true
    echo "  ✓ Binary info: $ANALYSIS_DIR/binary_info.txt"
else
    echo "  ⚠ No binary found for dynamic analysis"
fi
echo ""

# ==========================================
# CODE METRICS
# ==========================================
echo "=========================================="
echo "5. CODE METRICS"
echo "=========================================="

echo "Calculating code metrics..."
{
    echo "=== File Count ==="
    echo "Total C++ files: $(echo "$CPP_FILES" | wc -l)"
    echo "Header files (.hpp, .h): $(echo "$CPP_FILES" | grep -E '\.(hpp|h)$' | wc -l)"
    echo "Source files (.cpp): $(echo "$CPP_FILES" | grep '\.cpp$' | wc -l)"
    echo ""
    echo "=== Lines of Code ==="
    echo "$CPP_FILES" | xargs wc -l 2>/dev/null | tail -1 || echo "Unable to count lines"
    echo ""
    echo "=== Largest Files ==="
    echo "$CPP_FILES" | xargs wc -l 2>/dev/null | sort -rn | head -10 || true
} > "$ANALYSIS_DIR/code_metrics.txt"
echo "  ✓ Code metrics: $ANALYSIS_DIR/code_metrics.txt"
echo ""

# ==========================================
# BUG SUMMARY
# ==========================================
echo "=========================================="
echo "6. GENERATING BUG SUMMARY"
echo "=========================================="

{
    echo "BUG DISCOVERY REPORT"
    echo "===================="
    echo "Generated: $(date)"
    echo ""
    
    echo "=== COMPILATION ERRORS ==="
    if [ -f "$ANALYSIS_DIR/build_debug.txt" ]; then
        grep -i "error:" "$ANALYSIS_DIR/build_debug.txt" | head -20 || echo "No errors found in build log"
    fi
    echo ""
    
    echo "=== CLANG-TIDY ISSUES ==="
    if [ -f "$ANALYSIS_DIR/clang-tidy_report.txt" ]; then
        grep -E "(warning|error):" "$ANALYSIS_DIR/clang-tidy_report.txt" | head -30 || echo "No clang-tidy issues found"
    fi
    echo ""
    
    echo "=== CPPCHECK ISSUES ==="
    if [ -f "$ANALYSIS_DIR/cppcheck_report.txt" ]; then
        grep -E "\[.*\]" "$ANALYSIS_DIR/cppcheck_report.txt" | head -30 || echo "No cppcheck issues found"
    fi
    echo ""
    
    echo "=== CPPLINT ISSUES ==="
    if [ -f "$ANALYSIS_DIR/cpplint_report.txt" ]; then
        grep -E "^.*:[0-9]+:" "$ANALYSIS_DIR/cpplint_report.txt" | head -30 || echo "No cpplint issues found"
    fi
    echo ""
    
} > "$ANALYSIS_DIR/bug_summary.txt"

echo "  ✓ Bug summary: $ANALYSIS_DIR/bug_summary.txt"
echo ""

echo "=========================================="
echo "Analysis Complete!"
echo "=========================================="
echo "Reports generated in: $ANALYSIS_DIR/"
echo ""
ls -lh "$ANALYSIS_DIR"/
