#!/bin/bash
# Comprehensive C++ Analysis Script
# Runs static, dynamic, coverage, and performance analysis

set -e

export PATH="$HOME/.local/bin:$PATH"
REPORT_DIR="analysis_reports"
mkdir -p "$REPORT_DIR"

echo "========================================="
echo "C++ Comprehensive Analysis Report"
echo "Generated: $(date)"
echo "========================================="

# Static Analysis
echo ""
echo "=== STATIC ANALYSIS ==="
echo ""

# 1. Clang-format check
echo "[1/8] Running clang-format check..."
find . -name "*.cpp" -o -name "*.hpp" | grep -v ".build" | grep -v ".git" > /tmp/files_to_check.txt
clang-format --dry-run --Werror $(cat /tmp/files_to_check.txt | head -20) 2>&1 | tee "$REPORT_DIR/clang-format.txt" || echo "clang-format found formatting issues (see report)" > "$REPORT_DIR/clang-format.txt"

# 2. Clang-tidy
echo "[2/8] Running clang-tidy..."
if [ -f ".build/compile_commands.json" ]; then
    clang-tidy -p .build $(find . -name "*.cpp" -o -name "*.hpp" | grep -v ".build" | grep -v ".git" | head -30) \
        --config-file=assets/clang-tidy-suppressions.yaml 2>&1 | tee "$REPORT_DIR/clang-tidy.txt" || true
else
    echo "compile_commands.json not found, skipping clang-tidy" > "$REPORT_DIR/clang-tidy.txt"
fi

# 3. Cppcheck
echo "[3/8] Running cppcheck..."
cppcheck --enable=all --suppress=missingIncludeSystem \
    --suppressions-list=assets/cppcheck-suppressions.txt \
    --inline-suppr \
    --xml --xml-version=2 \
    $(find . -name "*.cpp" | grep -v ".build" | grep -v ".git" | head -20) 2>&1 | tee "$REPORT_DIR/cppcheck.xml" || true
cppcheck --enable=all --suppress=missingIncludeSystem \
    --suppressions-list=assets/cppcheck-suppressions.txt \
    --inline-suppr \
    $(find . -name "*.cpp" | grep -v ".build" | grep -v ".git" | head -20) 2>&1 | tee "$REPORT_DIR/cppcheck.txt" || true

# 4. Cpplint
echo "[4/8] Running cpplint..."
cpplint --recursive --extensions=cpp,hpp \
    --filter=-legal/copyright,-readability/todo,-build/include_order \
    include/ src/ tests/ examples/ 2>&1 | tee "$REPORT_DIR/cpplint.txt" || true

echo ""
echo "=== BUILD ==="
echo ""

# 5. Build Debug
echo "[5/8] Building Debug configuration..."
cd .build
make -j$(nproc) 2>&1 | tee "../$REPORT_DIR/build-debug.txt" || echo "Build failed" >> "../$REPORT_DIR/build-debug.txt"
cd ..

# 6. Build Release
echo "[6/8] Building Release configuration..."
cmake -S . -B .build-release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DXTD_BUILD_TESTS=ON -DXTD_BUILD_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-13
cd .build-release
make -j$(nproc) 2>&1 | tee "../$REPORT_DIR/build-release.txt" || echo "Build failed" >> "../$REPORT_DIR/build-release.txt"
cd ..

echo ""
echo "=== DYNAMIC ANALYSIS ==="
echo ""

# 7. Run tests with sanitizers
echo "[7/8] Running tests with sanitizers..."
if [ -f ".build/tests/unit_tests" ]; then
    # AddressSanitizer
    export ASAN_OPTIONS="detect_leaks=1:check_initialization_order=1:strict_init_order=1"
    .build/tests/unit_tests 2>&1 | tee "$REPORT_DIR/asan.txt" || true
    
    # Valgrind
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
        --log-file="$REPORT_DIR/valgrind.txt" \
        .build/tests/unit_tests 2>&1 | tee "$REPORT_DIR/valgrind-output.txt" || true
else
    echo "Tests not built, skipping dynamic analysis" > "$REPORT_DIR/asan.txt"
    echo "Tests not built, skipping dynamic analysis" > "$REPORT_DIR/valgrind.txt"
fi

echo ""
echo "=== COVERAGE ANALYSIS ==="
echo ""

# 8. Coverage
echo "[8/8] Running coverage analysis..."
if [ -f ".build/tests/unit_tests" ]; then
    # Build with coverage flags
    cmake -S . -B .build-coverage -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DXTD_BUILD_TESTS=ON \
        -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++-13 \
        -DCMAKE_CXX_FLAGS="--coverage -fprofile-arcs -ftest-coverage" \
        -DCMAKE_EXE_LINKER_FLAGS="--coverage"
    cd .build-coverage
    make -j$(nproc) tests/unit_tests 2>&1 | tee "../$REPORT_DIR/build-coverage.txt" || true
    if [ -f "tests/unit_tests" ]; then
        ./tests/unit_tests
        lcov --capture --directory . --output-file coverage.info
        lcov --remove coverage.info '/usr/*' --output-file coverage.info
        lcov --list coverage.info > "../$REPORT_DIR/coverage.txt"
        genhtml coverage.info --output-directory "../$REPORT_DIR/coverage_html"
    fi
    cd ..
else
    echo "Tests not built, skipping coverage" > "$REPORT_DIR/coverage.txt"
fi

echo ""
echo "=== PERFORMANCE ANALYSIS ==="
echo ""

# 9. Performance profiling
if [ -f ".build/tests/unit_tests" ]; then
    echo "Running performance profiling..."
    perf record -o "$REPORT_DIR/perf.data" -g .build/tests/unit_tests 2>&1 | tee "$REPORT_DIR/perf-record.txt" || true
    perf report -i "$REPORT_DIR/perf.data" > "$REPORT_DIR/perf-report.txt" 2>&1 || true
    perf stat .build/tests/unit_tests 2>&1 | tee "$REPORT_DIR/perf-stat.txt" || true
fi

echo ""
echo "Analysis complete! Reports saved to $REPORT_DIR/"
echo ""
