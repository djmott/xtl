#!/bin/bash
# Use clang directly for static analysis

REPORT_DIR="analysis_reports"
mkdir -p "$REPORT_DIR"

echo "Running clang-based analysis..."

# Analyze files with clang -fsyntax-only to catch compilation errors
echo "=== Compilation Check ===" > "$REPORT_DIR/clang_syntax_check.txt"
find include src -name "*.cpp" -o -name "*.hpp" | head -30 | while read file; do
    echo "Checking: $file" >> "$REPORT_DIR/clang_syntax_check.txt"
    clang++ -std=c++17 -fsyntax-only -Iinclude -Werror -Wall -Wextra "$file" 2>&1 | \
        tee -a "$REPORT_DIR/clang_syntax_check.txt" || true
    echo "" >> "$REPORT_DIR/clang_syntax_check.txt"
done

# Use clang -Weverything for maximum warnings (on a few files)
echo "=== Maximum Warnings Check ===" > "$REPORT_DIR/clang_max_warnings.txt"
find include -name "*.hpp" | head -10 | while read file; do
    echo "Analyzing: $file" >> "$REPORT_DIR/clang_max_warnings.txt"
    clang++ -std=c++17 -fsyntax-only -Iinclude -Weverything -Wno-c++98-compat -Wno-padded \
        "$file" 2>&1 | tee -a "$REPORT_DIR/clang_max_warnings.txt" || true
    echo "" >> "$REPORT_DIR/clang_max_warnings.txt"
done

echo "Clang analysis complete. See $REPORT_DIR/clang_*.txt"
