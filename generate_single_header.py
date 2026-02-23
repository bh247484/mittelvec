import os
import re

# --- Configuration ---
PROJECT_NAME = "MittelVec"
# The namespace to wrap the final library in.
NAMESPACE_NAME = "MittelVec" 
# The namespace to strip from the source files.
# Can be a regex pattern to catch variations like "namespace MittelVec {"
NAMESPACE_TO_STRIP_PATTERN = r"^\s*namespace\s+" + re.escape(NAMESPACE_NAME) + r"\s*\{.*"
INCLUDE_DIR = "include"
SRC_DIR = "src"
EXCLUDE_FILES = ["main.cpp"] # Don't include main.cpp used to test lib locally.
OUTPUT_FILE = "dist/mittelvec.h"
IMPLEMENTATION_GUARD = "MITTELVEC_IMPLEMENTATION"

# --- End Configuration ---

def get_project_files(directory, extensions):
    """Gets a list of files with given extensions from a directory."""
    files = []
    for root, _, filenames in os.walk(directory):
        for filename in filenames:
            # Check if extension matches AND file is not in exclude list
            if any(filename.endswith(ext) for ext in extensions):
                if filename not in EXCLUDE_FILES:
                    files.append(os.path.join(root, filename))
    
    # Sort files naturally first
    files.sort()
    return files

def get_local_dependencies(filepath):
    """Scans a file for local #include "..." statements."""
    deps = []
    with open(filepath, 'r') as f:
        content = f.read()
        # Match #include "..." or #include "./..."
        matches = re.findall(r'^\s*#include\s*"(.*?)"', content, re.MULTILINE)
        for m in matches:
            deps.append(os.path.basename(m))
    return deps

def sort_headers_topologically(header_files):
    """Sorts header files so that dependencies come before dependents."""
    name_to_path = {os.path.basename(f): f for f in header_files}
    order = []
    visited = set()
    temp_visited = set()

    def visit(n):
        if n in temp_visited:
            # If a cycle is detected, we just return to avoid infinite recursion.
            # In a real C++ project, this shouldn't happen with headers.
            return 
        if n not in visited:
            temp_visited.add(n)
            if n in name_to_path:
                deps = get_local_dependencies(name_to_path[n])
                for d in deps:
                    if d in name_to_path:
                        visit(d)
            temp_visited.remove(n)
            visited.add(n)
            order.append(n)

    for name in sorted(name_to_path.keys()):
        visit(name)

    return [name_to_path[name] for name in order]

def strip_namespace(content):
    """Strips the namespace block from the file content."""
    lines = content.splitlines()
    
    # Find namespace start and end
    start_line_idx = -1
    brace_count = 0
    
    for i, line in enumerate(lines):
        if re.match(NAMESPACE_TO_STRIP_PATTERN, line):
            start_line_idx = i
            brace_count = 1
            break
            
    if start_line_idx == -1:
        return content # No namespace found to strip

    # This is a simple brace counter, might fail with complex macros
    # or comments, but works for this project structure.
    end_line_idx = -1
    for i in range(start_line_idx + 1, len(lines)):
        brace_count += lines[i].count('{')
        brace_count -= lines[i].count('}')
        if brace_count == 0:
            end_line_idx = i
            break

    if end_line_idx != -1:
        # Exclude the namespace lines
        new_lines = lines[:start_line_idx] + lines[start_line_idx+1:end_line_idx] + lines[end_line_idx+1:]
        return "\n".join(new_lines)
        
    return content # Return original if closing brace not found

def create_single_header():
    """Combines project files into a single header library."""
    if not os.path.exists(os.path.dirname(OUTPUT_FILE)):
        os.makedirs(os.path.dirname(OUTPUT_FILE))

    header_files = get_project_files(INCLUDE_DIR, ['.h', '.hpp'])
    header_files = sort_headers_topologically(header_files)
    source_files = get_project_files(SRC_DIR, ['.cpp', '.c'])

    # Matches any #include "..." (local/relative)
    local_include_pattern = re.compile(r'^\s*#include\s*".*?"', re.MULTILINE)

    # Matches any #include <...> (system)
    system_include_pattern = re.compile(r'^\s*#include\s*<.*?>', re.MULTILINE)

    system_includes = set()
    processed_headers = []

    # --- 1. Process Headers ---
    for filepath in header_files:
        with open(filepath, 'r') as infile:
            # Strip the namespace wrapper
            content = strip_namespace(infile.read())
            # Remove #pragma once
            content = re.sub(r'#pragma once\r?\n?', '', content)
            
            lines = content.splitlines()
            cleaned_lines = []
            for line in lines:
                # 1. Hoist system includes
                if system_include_pattern.match(line):
                    system_includes.add(line.strip())
                # 2. STRIP local includes
                elif local_include_pattern.match(line):
                    continue 
                else:
                    cleaned_lines.append(line)
            
            processed_headers.append("\n".join(cleaned_lines))

    # --- 2. Process Source Files (just for hoisting system includes) ---
    processed_sources = []
    for filepath in source_files:
        with open(filepath, 'r') as infile:
            content = strip_namespace(infile.read())
            
            lines = content.splitlines()
            cleaned_lines = []
            for line in lines:
                # 1. Hoist system includes
                if system_include_pattern.match(line):
                    system_includes.add(line.strip())
                # 2. STRIP local includes
                elif local_include_pattern.match(line):
                    continue 
                else:
                    cleaned_lines.append(line)
            processed_sources.append("\n".join(cleaned_lines))

    # --- 3. Write Output ---
    with open(OUTPUT_FILE, 'w') as outfile:
        header_guard = f"{PROJECT_NAME.upper()}_H"
        outfile.write(f"// {PROJECT_NAME} - Single-Header Library\n")
        outfile.write(f"// Generated on {__import__('datetime').date.today().isoformat()}\n\n")
        outfile.write(f"#ifndef {header_guard}\n")
        outfile.write(f"#define {header_guard}\n\n")

        # Write System Includes (Global Scope)
        outfile.write("// System includes \n")
        for inc in sorted(list(system_includes)):
            outfile.write(f"{inc}\n")
        outfile.write("\n")

        # Write miniaudio dependency include
        outfile.write("// NOTE! This lib depends on miniaudio (a single header file audio lib)\n")
        outfile.write("// Ensure miniaudio.h is in your include path\n")
        outfile.write('#include "miniaudio.h"\n\n')

        # Write Declarations
        outfile.write(f"namespace {NAMESPACE_NAME} {{\n\n")
        for content in processed_headers:
            outfile.write(content)
            outfile.write("\n")
        outfile.write(f"}} // namespace {NAMESPACE_NAME}\n\n")

        outfile.write(f"#endif // {header_guard}\n\n")

        # Implementation Guard
        outfile.write(f"#ifdef {IMPLEMENTATION_GUARD}\n\n")

        # miniaudio implementation injection.
        outfile.write("#ifndef MINIAUDIO_IMPLEMENTATION\n")
        outfile.write("    #define MINIAUDIO_IMPLEMENTATION\n")
        outfile.write("#endif\n")
        outfile.write('#include "miniaudio.h"\n\n')

        # Begin namespace.
        outfile.write(f"namespace {NAMESPACE_NAME} {{\n\n")

        # Write Source Implementations
        for content in processed_sources:
            outfile.write(content)
            outfile.write("\n")
            
        outfile.write(f"}} // namespace {NAMESPACE_NAME}\n\n")
        outfile.write(f"#endif // {IMPLEMENTATION_GUARD}\n")

    print(f"Successfully created single-header library at: {OUTPUT_FILE}")

if __name__ == "__main__":
    create_single_header()