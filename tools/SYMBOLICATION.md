# Symbolication and Crash Analysis Tools

A comprehensive toolkit for analyzing and symbolicting crashes in ravynOS binaries.

## Tools Overview

### 1. `symbolicate.py` - Address to Symbol Mapping

Maps crash addresses to symbols in compiled binaries.

**Usage:**
```bash
# Single address
./symbolicate.py /path/to/binary 0x1d9660

# Multiple addresses
./symbolicate.py /path/to/binary 0x1d9660 0x1d9700 0x1d9750

# With register context
./symbolicate.py /path/to/binary --rip 0x1d9660 --rax 0x8010000000000007

# Stack trace
./symbolicate.py /path/to/binary --stack-trace 0x1d9660,0x1d9700,0x1d9750

# Search for symbols
./symbolicate.py /path/to/binary --search syscall

# JSON output
./symbolicate.py /path/to/binary 0x1d9660 --json
```

**Features:**
- Binary search through symbol table for fast lookup
- Shows symbol name, section, and offset
- Displays nearby symbols for context
- Supports both exact and substring symbol search
- JSON output for automation
- Handles both global and local symbols

**Example Output:**
```
=== Symbolication Results ===

1. 0x1d9660 -> _exit (TEXT) +0x0 (entry point)
   Size: 0x20 bytes
   Offset: 0x0
   Context: entry point
   Nearby symbols:
     0x1d9640 syscall_wrapper
     0x1d9660 _exit <--
     0x1d9680 exit_trampoline
```

### 2. `crash-analyzer.py` - Comprehensive Crash Analysis

Analyzes crash dumps and classifies crash types with recommendations.

**Usage:**
```bash
# Basic crash analysis with register state
./crash-analyzer.py \
  --binary /path/to/binary \
  --rip 0x1d9660 \
  --rax 0x8010000000000007

# With full register context
./crash-analyzer.py \
  --binary /path/to/binary \
  --rip 0x1d9660 \
  --rax 0x8010000000000007 \
  --rdi 0x000000010ffd0000 \
  --rsi 0x0000000115337c7e \
  --rdx 0x000000011534d833

# With stack trace
./crash-analyzer.py \
  --binary /path/to/binary \
  --rip 0x1d9660 \
  --rax 0x8010000000000007 \
  --stack-trace 0x1d9660,0x1d9700,0x1d9750

# JSON output
./crash-analyzer.py \
  --binary /path/to/binary \
  --rip 0x1d9660 \
  --rax 0x8010000000000007 \
  --json
```

**Crash Types Detected:**
- **Invalid Pointer Dereference** - Bit 63 set or known error patterns
- **Null Pointer Dereference** - RAX = 0x0
- **Syscall Failure** - At syscall instruction
- **Segmentation Fault** - Unaligned or invalid memory access
- **Buffer Overflow** - Stack corruption patterns
- **Use After Free** - Freed memory markers
- **Unknown** - Insufficient information

**Example Output:**
```
======================================================================
  CRASH ANALYSIS REPORT
======================================================================

Crash Type: invalid_pointer_dereference
Confidence: 95%

Description:
  RAX contains invalid pointer: 0x8010000000000007. Bit 63 set indicates 
  kernel space or error marker.

Affected Symbol: dyld::recursiveBind+0x45
Crash Address (RIP): 0x1d9660

RAX: 0x8010000000000007
RDI: 0x000000010ffd0000

Stack Trace (3 frames):
  [0] 0x1d9660
  [1] 0x1d9700
  [2] 0x1d9750

Recommendations:
  1. Check dyld's binding/interposing state at this instruction
  2. Verify no corrupted mach-o load commands
  3. Review recursive dependency loading logic
  4. Check for malformed or misinterpreted bind/lazy-bind records

======================================================================
```

### 3. `analyze-launchd-crash.py` - launchd-Specific Analysis

Specialized analyzer for launchd stub crashes, focusing on syscall entry points and exit paths.

**Usage:**
```bash
# Analyze launchd crash
./analyze-launchd-crash.py /path/to/launchd 0x1d9660

# Analyze exit-related code paths
./analyze-launchd-crash.py /path/to/launchd 0x1d9660 --analyze-exit

# Verbose output
./analyze-launchd-crash.py /path/to/launchd 0x1d9660 -v
```

**Features:**
- Detects syscall instructions at crash location
- Analyzes exit-related code paths
- Shows disassembly context around crash
- Identifies if crash is at binary entry point
- Lists known syscall constants

**Example Output:**
```
=== Launchd Crash Analysis ===

RIP: 0x1d9660
Entry Point: 0x1d9660
  ⚠️  Crash at entry point (very early in execution)

Syscall Entry: YES - syscall
  This suggests a syscall instruction is at the crash location

Disassembly around crash:

0000000000001d50 <_start>:
    1d50: 55                    push   %rbp
    1d51: 48 89 e5              mov    %rsp,%rbp
    ...
    1d60: 0f 05                 syscall
    1d62: ...
```

## Practical Examples

### Example 1: Analyzing the launchd stub issue

Given the crash at `0x1d9660` in the launchd stub which is the `syscall` instruction in `_exit`:

```bash
# First, symbolicate the address
./symbolicate.py /nest/ravynos/BSD/sbin/launchd 0x1d9660

# Then analyze the crash with register state
./crash-analyzer.py \
  --binary /nest/ravynos/BSD/sbin/launchd \
  --rip 0x1d9660 \
  --rax 0x2000001  # SYS_exit

# Get detailed launchd-specific analysis
./analyze-launchd-crash.py /nest/ravynos/BSD/sbin/launchd 0x1d9660 --analyze-exit
```

### Example 2: Analyzing dyld crash with full register state

From CRASH_FIX_ANALYSIS.md:

```bash
./crash-analyzer.py \
  --binary /nest/ravynos/Libraries/dyld/src/dyld \
  --rip 0x00000001154ad2eb \
  --rax 0x8010000000000007 \
  --rdi 0x000000010ffd0000 \
  --stack-trace 0x00000001154ad2eb,0x0000000115337c7e,0x000000011534d833,0x000000011534ac51
```

### Example 3: Searching for symbols

Find all syscall-related symbols:

```bash
./symbolicate.py /path/to/binary --search syscall
```

Find all exit-related symbols:

```bash
./symbolicate.py /path/to/binary --search exit
```

## Building and Testing

### Prerequisites

The tools require standard Unix utilities:
- `nm` - For symbol table extraction
- `objdump` - For disassembly
- `readelf` or `otool` - For binary headers
- Python 3.6+

Install on macOS:
```bash
brew install binutils
```

Install on Linux:
```bash
sudo apt-get install binutils
```

### Quick Test

```bash
# Test symbolicate on a system binary
./symbolicate.py /bin/ls 0x100001000

# Test on ravynOS launchd
./symbolicate.py /nest/ravynos/BSD/sbin/launchd 0x1d9660
```

## Integration with Crash Analysis Workflow

1. **Capture crash** - Kernel or crash reporter provides RIP, RAX, and stack trace
2. **Locate binary** - Find the binary that crashed (often in /nest/ravynos/BSD/sbin/launchd or /nest/ravynos/Libraries/dyld/src/dyld)
3. **Symbolicate** - Use `symbolicate.py` to map addresses to function names
4. **Analyze** - Use `crash-analyzer.py` to classify the crash type
5. **Investigate** - Use `analyze-launchd-crash.py` for launchd-specific patterns

Example workflow script:

```bash
#!/bin/bash

BINARY="$1"
RIP="$2"
RAX="$3"

echo "=== Quick Crash Analysis ==="
echo ""

echo "Step 1: Symbolicate addresses"
./symbolicate.py "$BINARY" "$RIP" --context 3
echo ""

echo "Step 2: Analyze crash"
./crash-analyzer.py --binary "$BINARY" --rip "$RIP" --rax "$RAX"
```

## Output Formats

### Human-Readable (Default)

Formatted for terminal viewing with colors and indentation.

### JSON Output

All tools support `--json` for machine-readable output:

```bash
./symbolicate.py /path/to/binary 0x1d9660 --json
./crash-analyzer.py --binary /path/to/binary --rip 0x1d9660 --json
```

JSON structure:
```json
{
  "address": "0x1d9660",
  "symbol": "_exit",
  "section": "TEXT",
  "offset": "0x0",
  "context": "entry point"
}
```

## Troubleshooting

### "nm command not found"
Install binutils:
- macOS: `brew install binutils`
- Linux: `sudo apt-get install binutils`

### "No symbols found"
Binary may be stripped. Use debug symbols if available:
```bash
./symbolicate.py /path/to/binary.dSYM/Contents/Resources/DWARF/binary 0x1d9660
```

### "Could not get disassembly"
Ensure objdump is installed and binary is accessible.

### Very slow on large binaries
The tools cache symbols after first load. Subsequent queries are fast.

## Advanced Usage

### Analyzing multiple crashes

Create a file `crashes.txt`:
```
0x1d9660 0x8010000000000007
0x1d9700 0xdeadbeef
0x1d9750 0x0000000115337c7e
```

Process all:
```bash
while read rip rax; do
  ./crash-analyzer.py --binary /path/to/binary --rip "$rip" --rax "$rax"
done < crashes.txt
```

### Integration with CI/CD

Export as JSON for processing:
```bash
./crash-analyzer.py \
  --binary /path/to/binary \
  --rip "$RIP" \
  --rax "$RAX" \
  --json > crash_report.json
```

Parse the JSON in your CI pipeline to trigger alerts or create issues.

## Known Limitations

1. **Stripped binaries** - Requires debug symbols
2. **ASLR** - Addresses must be relative to binary's load address
3. **Indirect calls** - Can't determine exact target of indirect calls
4. **Cross-binary analysis** - Each binary requires separate analysis

## Contributing

To add new crash pattern detection:

1. Add pattern to `CrashAnalyzer.INVALID_POINTER_MARKERS` or implement new check
2. Add test case in analysis
3. Update this README with example

## License

Part of ravynOS project. See main LICENSE file.

## See Also

- CRASH_FIX_ANALYSIS.md - Detailed crash analysis from development
- Kernel/xnu/bsd/kern/mach_loader.c - Binary loader implementation
- Libraries/dyld/src/ - Dynamic linker source

