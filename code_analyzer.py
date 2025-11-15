#!/usr/bin/env python3
"""
Comprehensive C++ Code Analyzer
Performs static analysis, bug detection, and generates reports
"""

import os
import re
import json
import subprocess
from pathlib import Path
from collections import defaultdict
from datetime import datetime

class CPPCodeAnalyzer:
    def __init__(self, root_dir="."):
        self.root_dir = Path(root_dir)
        self.issues = defaultdict(list)
        self.stats = {
            'files_analyzed': 0,
            'lines_analyzed': 0,
            'issues_found': 0
        }
        
    def find_cpp_files(self):
        """Find all C++ source files"""
        cpp_files = []
        for ext in ['*.cpp', '*.hpp', '*.h', '*.cc', '*.cxx']:
            cpp_files.extend(self.root_dir.rglob(ext))
        return sorted(cpp_files)
    
    def analyze_file(self, filepath):
        """Analyze a single C++ file for common issues"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                lines = content.split('\n')
                self.stats['files_analyzed'] += 1
                self.stats['lines_analyzed'] += len(lines)
                
                # Analyze for common issues
                self.check_memory_issues(filepath, lines)
                self.check_null_pointer_dereference(filepath, lines)
                self.check_buffer_overflows(filepath, lines)
                self.check_uninitialized_variables(filepath, lines)
                self.check_resource_leaks(filepath, lines)
                self.check_exception_safety(filepath, lines)
                self.check_thread_safety(filepath, lines)
                self.check_undefined_behavior(filepath, lines)
                self.check_code_smells(filepath, lines)
                
        except Exception as e:
            self.issues['errors'].append({
                'file': str(filepath),
                'line': 0,
                'severity': 'error',
                'message': f'Failed to analyze file: {str(e)}'
            })
    
    def check_memory_issues(self, filepath, lines):
        """Check for memory-related issues"""
        patterns = [
            (r'delete\s+[^;]+;\s*delete\s+\1', 'DOUBLE_DELETE', 'high'),
            (r'delete\s+\[\]\s*[^;]+;\s*delete\s+\[\]\s*\1', 'DOUBLE_DELETE_ARRAY', 'high'),
            (r'malloc\s*\([^)]+\)', 'USE_MALLOC', 'medium'),  # Prefer new/delete
            (r'free\s*\([^)]+\)', 'USE_FREE', 'medium'),  # Prefer new/delete
        ]
        
        for i, line in enumerate(lines, 1):
            for pattern, issue_type, severity in patterns:
                if re.search(pattern, line):
                    self.issues[issue_type].append({
                        'file': str(filepath),
                        'line': i,
                        'severity': severity,
                        'message': f'Potential memory issue: {issue_type}',
                        'code': line.strip()
                    })
                    self.stats['issues_found'] += 1
    
    def check_null_pointer_dereference(self, filepath, lines):
        """Check for potential null pointer dereferences"""
        # Look for patterns where pointers might be dereferenced without null checks
        for i, line in enumerate(lines, 1):
            # Check for pointer dereference patterns
            deref_match = re.search(r'(\w+)\s*->', line)
            if deref_match:
                ptr_name = deref_match.group(1)
                # Look backwards for null assignment
                for j in range(max(0, i-20), i):
                    if re.search(rf'\b{ptr_name}\s*=\s*(?:nullptr|NULL|0)', lines[j]):
                        # Check if there's a null check between assignment and use
                        has_null_check = False
                        for k in range(j+1, i):
                            if 'if' in lines[k].lower() and ('null' in lines[k].lower() or ptr_name in lines[k]):
                                has_null_check = True
                                break
                        if not has_null_check:
                            self.issues['NULL_POINTER_DEREF'].append({
                                'file': str(filepath),
                                'line': i,
                                'severity': 'high',
                                'message': f'Potential null pointer dereference: {ptr_name} (assigned null at line {j+1})',
                                'code': line.strip()
                            })
                            self.stats['issues_found'] += 1
                            break
    
    def check_buffer_overflows(self, filepath, lines):
        """Check for potential buffer overflows"""
        dangerous_functions = [
            'strcpy', 'strcat', 'sprintf', 'gets', 'scanf'
        ]
        
        for i, line in enumerate(lines, 1):
            for func in dangerous_functions:
                if func in line:
                    self.issues['BUFFER_OVERFLOW'].append({
                        'file': str(filepath),
                        'line': i,
                        'severity': 'high',
                        'message': f'Use of unsafe function: {func}. Consider safer alternatives.',
                        'code': line.strip()
                    })
                    self.stats['issues_found'] += 1
    
    def check_uninitialized_variables(self, filepath, lines):
        """Check for potentially uninitialized variables"""
        for i, line in enumerate(lines, 1):
            # Check for variable declaration followed by use without initialization
            decl_match = re.search(r'(\w+)\s+(\w+)\s*;', line)
            if decl_match:
                var_type, var_name = decl_match.groups()
                # Look for usage without initialization in next few lines
                for j in range(i, min(i+20, len(lines))):
                    if var_name in lines[j] and '=' not in lines[j] and 'if' not in lines[j].lower():
                        if re.search(rf'\b{var_name}\s*[+\-*/%]|\b{var_name}\s*\[', lines[j]):
                            self.issues['UNINITIALIZED_VAR'].append({
                                'file': str(filepath),
                                'line': j+1,
                                'severity': 'medium',
                                'message': f'Potential use of uninitialized variable: {var_name}',
                                'code': lines[j].strip()
                            })
                            self.stats['issues_found'] += 1
                            break
    
    def check_resource_leaks(self, filepath, lines):
        """Check for resource leaks (file handles, locks, etc.)"""
        resource_patterns = [
            (r'fopen\s*\(', 'FILE_LEAK', 'high'),
            (r'open\s*\(', 'FILE_LEAK', 'high'),
            (r'pthread_mutex_lock', 'LOCK_LEAK', 'high'),
            (r'std::lock_guard|std::unique_lock', 'LOCK_SAFE', 'info'),  # Good practice
        ]
        
        for i, line in enumerate(lines, 1):
            for pattern, issue_type, severity in resource_patterns:
                if re.search(pattern, line):
                    # Check if there's a corresponding close/unlock
                    has_close = False
                    for j in range(i, min(i+50, len(lines))):
                        if 'close' in lines[j].lower() or 'unlock' in lines[j].lower() or '}' in lines[j]:
                            has_close = True
                            break
                    
                    if not has_close and severity != 'info':
                        self.issues[issue_type].append({
                            'file': str(filepath),
                            'line': i,
                            'severity': severity,
                            'message': f'Potential resource leak: {issue_type}',
                            'code': line.strip()
                        })
                        self.stats['issues_found'] += 1
    
    def check_exception_safety(self, filepath, lines):
        """Check for exception safety issues"""
        for i, line in enumerate(lines, 1):
            # Check for new without exception handling
            if re.search(r'new\s+\w+', line) and 'try' not in ' '.join(lines[max(0,i-5):i]).lower():
                self.issues['EXCEPTION_SAFETY'].append({
                    'file': str(filepath),
                    'line': i,
                    'severity': 'medium',
                    'message': 'new operator without exception handling',
                    'code': line.strip()
                })
                self.stats['issues_found'] += 1
    
    def check_thread_safety(self, filepath, lines):
        """Check for thread safety issues"""
        for i, line in enumerate(lines, 1):
            # Check for shared variables without synchronization
            if 'static' in line and 'mutex' not in ' '.join(lines[max(0,i-5):i+5]).lower():
                if re.search(r'static\s+\w+\s+\w+\s*=', line):
                    self.issues['THREAD_SAFETY'].append({
                        'file': str(filepath),
                        'line': i,
                        'severity': 'medium',
                        'message': 'Static variable modification may not be thread-safe',
                        'code': line.strip()
                    })
                    self.stats['issues_found'] += 1
    
    def check_undefined_behavior(self, filepath, lines):
        """Check for undefined behavior patterns"""
        for i, line in enumerate(lines, 1):
            # Signed integer overflow - check for ++ ++ pattern
            if re.search(r'\+\+\s*\+\+', line):
                self.issues['UNDEFINED_BEHAVIOR'].append({
                    'file': str(filepath),
                    'line': i,
                    'severity': 'high',
                    'message': 'Multiple increments may cause undefined behavior',
                    'code': line.strip()
                })
                self.stats['issues_found'] += 1
            
            # Division by zero potential
            if '/' in line and '0' in line and 'if' not in line.lower():
                self.issues['UNDEFINED_BEHAVIOR'].append({
                    'file': str(filepath),
                    'line': i,
                    'severity': 'high',
                    'message': 'Potential division by zero',
                    'code': line.strip()
                })
                self.stats['issues_found'] += 1
    
    def check_code_smells(self, filepath, lines):
        """Check for code smells and bad practices"""
        for i, line in enumerate(lines, 1):
            # Magic numbers
            if re.search(r'\b\d{3,}\b', line) and '//' not in line:
                self.issues['CODE_SMELL'].append({
                    'file': str(filepath),
                    'line': i,
                    'severity': 'low',
                    'message': 'Magic number detected, consider using named constant',
                    'code': line.strip()
                })
                self.stats['issues_found'] += 1
            
            # Very long lines
            if len(line) > 120:
                self.issues['CODE_SMELL'].append({
                    'file': str(filepath),
                    'line': i,
                    'severity': 'low',
                    'message': f'Line too long ({len(line)} characters)',
                    'code': line.strip()[:100] + '...'
                })
                self.stats['issues_found'] += 1
    
    def generate_report(self, output_file="analysis_reports/code_analysis_report.json"):
        """Generate JSON report"""
        report = {
            'timestamp': datetime.now().isoformat(),
            'statistics': self.stats,
            'issues': dict(self.issues),
            'summary': {
                'total_files': self.stats['files_analyzed'],
                'total_lines': self.stats['lines_analyzed'],
                'total_issues': self.stats['issues_found'],
                'issues_by_severity': {
                    'high': sum(1 for issues in self.issues.values() for issue in issues if issue.get('severity') == 'high'),
                    'medium': sum(1 for issues in self.issues.values() for issue in issues if issue.get('severity') == 'medium'),
                    'low': sum(1 for issues in self.issues.values() for issue in issues if issue.get('severity') == 'low'),
                }
            }
        }
        
        os.makedirs(os.path.dirname(output_file), exist_ok=True)
        with open(output_file, 'w') as f:
            json.dump(report, f, indent=2)
        
        return report
    
    def generate_markdown_report(self, output_file="analysis_reports/code_analysis_report.md"):
        """Generate Markdown report"""
        os.makedirs(os.path.dirname(output_file), exist_ok=True)
        
        with open(output_file, 'w') as f:
            f.write("# C++ Code Analysis Report\n\n")
            f.write(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n")
            
            f.write("## Statistics\n\n")
            f.write(f"- Files Analyzed: {self.stats['files_analyzed']}\n")
            f.write(f"- Lines Analyzed: {self.stats['lines_analyzed']}\n")
            f.write(f"- Total Issues Found: {self.stats['issues_found']}\n\n")
            
            # Count by severity
            high_count = sum(1 for issues in self.issues.values() for issue in issues if issue.get('severity') == 'high')
            medium_count = sum(1 for issues in self.issues.values() for issue in issues if issue.get('severity') == 'medium')
            low_count = sum(1 for issues in self.issues.values() for issue in issues if issue.get('severity') == 'low')
            
            f.write("## Issues by Severity\n\n")
            f.write(f"- **High**: {high_count}\n")
            f.write(f"- **Medium**: {medium_count}\n")
            f.write(f"- **Low**: {low_count}\n\n")
            
            f.write("## Detailed Issues\n\n")
            
            for issue_type, issues_list in sorted(self.issues.items()):
                if issues_list:
                    f.write(f"### {issue_type}\n\n")
                    for issue in issues_list[:20]:  # Limit to first 20 per type
                        f.write(f"**File**: `{issue['file']}`  \n")
                        f.write(f"**Line**: {issue['line']}  \n")
                        f.write(f"**Severity**: {issue['severity']}  \n")
                        f.write(f"**Message**: {issue['message']}  \n")
                        f.write(f"**Code**: `{issue.get('code', 'N/A')}`  \n\n")
                    if len(issues_list) > 20:
                        f.write(f"*... and {len(issues_list) - 20} more issues of this type*\n\n")
    
    def run_analysis(self):
        """Run complete analysis"""
        print("Starting C++ code analysis...")
        print(f"Scanning directory: {self.root_dir}")
        
        cpp_files = self.find_cpp_files()
        print(f"Found {len(cpp_files)} C++ files")
        
        for filepath in cpp_files:
            if 'build' not in str(filepath) and '.git' not in str(filepath):
                self.analyze_file(filepath)
        
        print(f"\nAnalysis complete!")
        print(f"Files analyzed: {self.stats['files_analyzed']}")
        print(f"Issues found: {self.stats['issues_found']}")
        
        # Generate reports
        self.generate_report()
        self.generate_markdown_report()
        
        print(f"\nReports generated in analysis_reports/")

if __name__ == "__main__":
    analyzer = CPPCodeAnalyzer("/workspace")
    analyzer.run_analysis()
