#!/bin/bash
# Comprehensive C++ Analysis Script
# Performs static, dynamic, coverage, and performance analysis

set -e

REPORT_DIR="analysis_reports"
mkdir -p "$REPORT_DIR"

echo "=========================================="
echo "C++ Comprehensive Analysis Framework"
echo "=========================================="
echo "Started: $(date)"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[OK]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# ============================================
# 1. STATIC ANALYSIS
# ============================================
echo "=========================================="
echo "1. STATIC ANALYSIS"
echo "=========================================="

# 1.1 Clang Static Analyzer (scan-build)
log_info "Running Clang Static Analyzer..."
if command -v scan-build &> /dev/null; then
    scan-build -o "$REPORT_DIR/scan-build" \
        --use-analyzer=clang \
        --status-bugs \
        cmake --build .build 2>&1 | tee "$REPORT_DIR/scan-build.log" || true
    if [ -d "$REPORT_DIR/scan-build" ]; then
        log_success "Clang Static Analyzer completed - reports in $REPORT_DIR/scan-build/"
    else
        log_warning "Clang Static Analyzer: No issues directory created (may need successful build)"
    fi
else
    log_warning "scan-build not available"
fi

# 1.2 Clang-tidy
log_info "Running clang-tidy..."
if command -v clang-tidy &> /dev/null; then
    CLANG_TIDY_CMD="clang-tidy"
    CLANG_TIDY_ARGS="--quiet --warnings-as-errors=*"
    
    if [ -f compile_commands.json ]; then
        log_info "Using compile_commands.json"
        find include src tests examples -name "*.cpp" -o -name "*.hpp" | \
            head -50 | while read file; do
            $CLANG_TIDY_CMD "$file" $CLANG_TIDY_ARGS 2>&1 | \
                tee -a "$REPORT_DIR/clang-tidy.log" || true
        done
    else
        log_warning "compile_commands.json not found, using basic analysis"
        find include src tests examples -name "*.cpp" -o -name "*.hpp" | \
            head -50 | while read file; do
            $CLANG_TIDY_CMD "$file" -- -std=c++17 -Iinclude $CLANG_TIDY_ARGS 2>&1 | \
                tee -a "$REPORT_DIR/clang-tidy.log" || true
        done
    fi
    log_success "clang-tidy analysis complete"
else
    log_warning "clang-tidy not available"
fi

# 1.3 Cppcheck
log_info "Running cppcheck..."
if command -v cppcheck &> /dev/null; then
    cppcheck --enable=all \
        --suppress=missingIncludeSystem \
        --suppress=unusedFunction \
        --suppress=unmatchedSuppression \
        --inline-suppr \
        --xml --xml-version=2 \
        --output-file="$REPORT_DIR/cppcheck.xml" \
        include/ src/ tests/ examples/ 2>&1 | tee "$REPORT_DIR/cppcheck.txt" || true
    
    # Count issues
    if [ -f "$REPORT_DIR/cppcheck.xml" ]; then
        ERROR_COUNT=$(grep -c "<error" "$REPORT_DIR/cppcheck.xml" || echo "0")
        log_success "cppcheck found $ERROR_COUNT issues"
    fi
else
    log_warning "cppcheck not available"
fi

# 1.4 Cpplint
log_info "Running cpplint..."
if python3 -c "import cpplint" 2>/dev/null; then
    find include src tests examples -name "*.cpp" -o -name "*.hpp" | \
        while read file; do
        python3 -m cpplint "$file" 2>&1 | tee -a "$REPORT_DIR/cpplint.log" || true
    done
    log_success "cpplint analysis complete"
elif command -v cpplint &> /dev/null; then
    find include src tests examples -name "*.cpp" -o -name "*.hpp" | \
        while read file; do
        cpplint "$file" 2>&1 | tee -a "$REPORT_DIR/cpplint.log" || true
    done
    log_success "cpplint analysis complete"
else
    log_warning "cpplint not available"
fi

# 1.5 Manual Pattern Analysis
log_info "Running pattern-based analysis..."
python3 code_analyzer.py 2>&1 | tee "$REPORT_DIR/pattern_analysis.log"
log_success "Pattern analysis complete"

echo ""

# ============================================
# 2. CODE METRICS
# ============================================
echo "=========================================="
echo "2. CODE METRICS"
echo "=========================================="

log_info "Calculating code metrics..."

cat > "$REPORT_DIR/code_metrics.txt" << EOF
Code Metrics Report
Generated: $(date)

File Statistics:
EOF

find include src tests examples -name "*.cpp" -o -name "*.hpp" | while read file; do
    lines=$(wc -l < "$file" 2>/dev/null || echo "0")
    size=$(wc -c < "$file" 2>/dev/null || echo "0")
    echo "$file: $lines lines, $size bytes" >> "$REPORT_DIR/code_metrics.txt"
done

TOTAL_FILES=$(find include src tests examples -name "*.cpp" -o -name "*.hpp" | wc -l)
TOTAL_LINES=$(find include src tests examples \( -name "*.cpp" -o -name "*.hpp" \) -exec wc -l {} + 2>/dev/null | tail -1 | awk '{print $1}' || echo "0")

echo "" >> "$REPORT_DIR/code_metrics.txt"
echo "Total Files: $TOTAL_FILES" >> "$REPORT_DIR/code_metrics.txt"
echo "Total Lines: $TOTAL_LINES" >> "$REPORT_DIR/code_metrics.txt"

log_success "Code metrics calculated"

echo ""

# ============================================
# 3. DYNAMIC ANALYSIS PREPARATION
# ============================================
echo "=========================================="
echo "3. DYNAMIC ANALYSIS"
echo "=========================================="

log_info "Checking dynamic analysis tools..."

# Valgrind
if command -v valgrind &> /dev/null; then
    log_success "Valgrind available"
    echo "To run: valgrind --leak-check=full --show-leak-kinds=all ./test_executable" >> "$REPORT_DIR/dynamic_analysis_instructions.txt"
else
    log_warning "Valgrind not available"
fi

# Sanitizers
log_info "Sanitizer flags for compilation:"
echo "-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer" >> "$REPORT_DIR/dynamic_analysis_instructions.txt"
log_success "Sanitizer instructions written"

echo ""

# ============================================
# 4. COVERAGE ANALYSIS PREPARATION
# ============================================
echo "=========================================="
echo "4. COVERAGE ANALYSIS"
echo "=========================================="

if command -v gcov &> /dev/null && command -v lcov &> /dev/null; then
    log_success "Coverage tools available"
    cat > "$REPORT_DIR/coverage_instructions.txt" << 'EOF'
To generate coverage:
1. Build with: -fprofile-arcs -ftest-coverage
2. Run all tests
3. lcov --capture --directory . --output-file coverage.info
4. genhtml coverage.info --output-directory coverage_html
EOF
    log_success "Coverage instructions written"
else
    log_warning "Coverage tools not fully available"
fi

echo ""

# ============================================
# 5. PERFORMANCE ANALYSIS PREPARATION
# ============================================
echo "=========================================="
echo "5. PERFORMANCE ANALYSIS"
echo "=========================================="

if command -v perf &> /dev/null; then
    log_success "perf available"
    echo "To profile: perf record ./test_executable && perf report" >> "$REPORT_DIR/performance_instructions.txt"
else
    log_warning "perf not available"
fi

if command -v gprof &> /dev/null; then
    log_success "gprof available"
    echo "Build with: -pg, then run: gprof ./test_executable gmon.out > analysis.txt" >> "$REPORT_DIR/performance_instructions.txt"
else
    log_warning "gprof not available"
fi

echo ""

# ============================================
# 6. GENERATE SUMMARY
# ============================================
echo "=========================================="
echo "6. GENERATING SUMMARY REPORT"
echo "=========================================="

cat > "$REPORT_DIR/ANALYSIS_SUMMARY.md" << EOF
# Comprehensive C++ Analysis Summary

**Generated:** $(date)

## Overview

This report contains the results of comprehensive static, dynamic, coverage, and performance analysis of the C++ codebase.

## Analysis Results

### Static Analysis
- **Clang Static Analyzer**: See \`scan-build/\` directory
- **clang-tidy**: See \`clang-tidy.log\`
- **cppcheck**: See \`cppcheck.xml\` and \`cppcheck.txt\`
- **cpplint**: See \`cpplint.log\`
- **Pattern Analysis**: See \`code_analysis_report.md\`

### Code Metrics
- **Total Files**: $TOTAL_FILES
- **Total Lines**: $TOTAL_LINES
- See \`code_metrics.txt\` for detailed statistics

### Dynamic Analysis
- Instructions in \`dynamic_analysis_instructions.txt\`
- Requires build with sanitizers enabled

### Coverage Analysis
- Instructions in \`coverage_instructions.txt\`
- Requires build with coverage flags

### Performance Analysis
- Instructions in \`performance_instructions.txt\`

## Next Steps

1. Review static analysis reports
2. Build with sanitizers and run dynamic analysis
3. Generate coverage reports
4. Run performance profiling
5. Address identified issues

## Report Files

All analysis reports are located in: \`$REPORT_DIR/\`

EOF

log_success "Summary report generated: $REPORT_DIR/ANALYSIS_SUMMARY.md"

echo ""
echo "=========================================="
echo "Analysis Complete!"
echo "=========================================="
echo "Reports available in: $REPORT_DIR/"
echo "Summary: $REPORT_DIR/ANALYSIS_SUMMARY.md"
echo ""
