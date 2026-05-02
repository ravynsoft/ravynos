#!/usr/bin/env python3
"""
Comprehensive Crash Analysis Toolkit for ravynOS

Provides integrated analysis of crash dumps, including symbolication,
register state analysis, and pattern matching against known issues.

Usage:
    ./crash-analyzer.py --rip 0x1d9660 --rax 0x8010000000000007 --binary /path/to/binary
    ./crash-analyzer.py --crash-file crash.txt --binary /path/to/binary
    ./crash-analyzer.py --dyld-crash --rip 0x1d9660 --stack-trace 0x...,0x...,0x...
"""

import sys
import os
import subprocess
import re
import argparse
import json
from dataclasses import dataclass, asdict
from typing import List, Optional, Dict, Tuple
from enum import Enum


class CrashType(Enum):
    """Classification of crash types"""
    INVALID_POINTER = "invalid_pointer_dereference"
    SYSCALL_FAILURE = "syscall_failure"
    SEGMENT_VIOLATION = "segmentation_fault"
    ALIGNMENT_FAULT = "alignment_fault"
    NULL_POINTER = "null_pointer_dereference"
    BUFFER_OVERFLOW = "buffer_overflow"
    USE_AFTER_FREE = "use_after_free"
    UNKNOWN = "unknown"


@dataclass
class CrashContext:
    """Context information about a crash"""
    rip: int
    rax: Optional[int] = None
    rdi: Optional[int] = None
    rsi: Optional[int] = None
    rdx: Optional[int] = None
    stack_trace: List[int] = None

    def __post_init__(self):
        if self.stack_trace is None:
            self.stack_trace = []

    def to_dict(self) -> dict:
        """Convert to dictionary"""
        return {
            'rip': hex(self.rip),
            'rax': hex(self.rax) if self.rax else None,
            'rdi': hex(self.rdi) if self.rdi else None,
            'rsi': hex(self.rsi) if self.rsi else None,
            'rdx': hex(self.rdx) if self.rdx else None,
            'stack_trace': [hex(addr) for addr in self.stack_trace],
        }


@dataclass
class CrashAnalysisResult:
    """Result of crash analysis"""
    crash_type: CrashType
    confidence: float  # 0.0 to 1.0
    description: str
    affected_symbol: Optional[str]
    affected_address: int
    recommendations: List[str]

    def to_dict(self) -> dict:
        """Convert to dictionary"""
        return {
            'crash_type': self.crash_type.value,
            'confidence': self.confidence,
            'description': self.description,
            'affected_symbol': self.affected_symbol,
            'affected_address': hex(self.affected_address),
            'recommendations': self.recommendations,
        }


class CrashAnalyzer:
    """Main crash analysis engine"""

    # Known invalid pointer patterns (bit 63 typically indicates kernel/error space)
    INVALID_POINTER_MARKERS = [
        0x8010000000000000,  # Bit 63 set
        0xffff800000000000,  # Kernel space marker
        0xdeadbeef,          # Debug pattern
        0xdeaddeaddeaddead,  # Debug pattern
        0xcccccccccccccccc,  # Debug pattern
    ]

    def __init__(self, binary_path: str, verbose: bool = False):
        self.binary_path = binary_path
        self.verbose = verbose
        self.symbols: Dict[int, str] = {}
        self.otool_path = self._find_otool()
        self._load_symbols()

    def _find_otool(self) -> Optional[str]:
        """Find otool in common locations"""
        possible_paths = [
            'otool',
            '/usr/bin/otool',
            '/usr/local/bin/otool',
            '/nest/build/Developer/Toolchains/Default.xctoolchain/usr/bin/otool',
        ]

        for path in possible_paths:
            try:
                result = subprocess.run([path, '-h', '--version'],
                                      capture_output=True, timeout=2)
                if result.returncode == 0:
                    return path
            except:
                pass

        return None

    def _load_symbols(self):
        """Load symbols from binary"""
        # Try Mach-O first
        if self._is_macho_binary() and self.otool_path:
            self._load_symbols_from_macho()
        else:
            # Fall back to ELF
            self._load_symbols_from_elf()

    def _is_macho_binary(self) -> bool:
        """Check if binary is Mach-O format"""
        try:
            result = subprocess.run(
                ['file', self.binary_path],
                capture_output=True,
                text=True,
                timeout=10
            )
            return 'Mach-O' in result.stdout
        except:
            return False

    def _load_symbols_from_elf(self):
        """Load symbols from ELF binary using nm"""
        try:
            result = subprocess.run(
                ['nm', '-n', self.binary_path],
                capture_output=True,
                text=True,
                timeout=10
            )

            for line in result.stdout.strip().split('\n'):
                parts = line.split()
                if len(parts) >= 3:
                    try:
                        addr = int(parts[0], 16)
                        name = ' '.join(parts[2:])
                        self.symbols[addr] = name
                    except (ValueError, IndexError):
                        pass
        except Exception as e:
            if self.verbose:
                print(f"Warning: Could not load ELF symbols: {e}", file=sys.stderr)

    def _load_symbols_from_macho(self):
        """Load symbols from Mach-O binary using otool"""
        try:
            result = subprocess.run(
                [self.otool_path, '-tv', self.binary_path],
                capture_output=True,
                text=True,
                timeout=10
            )

            current_symbol = None
            for line in result.stdout.strip().split('\n'):
                line = line.strip()

                # Symbol definition (ends with :)
                if line.endswith(':') and not line[0].isdigit():
                    current_symbol = line[:-1]

                # Instruction line with address
                parts = line.split()
                if parts and all(c in '0123456789abcdef' for c in parts[0]):
                    try:
                        addr = int(parts[0], 16)
                        if current_symbol:
                            self.symbols[addr] = current_symbol
                            current_symbol = None  # Clear after first instruction
                    except (ValueError, IndexError):
                        pass
        except Exception as e:
            if self.verbose:
                print(f"Warning: Could not load Mach-O symbols: {e}", file=sys.stderr)

    def analyze(self, crash_context: CrashContext) -> CrashAnalysisResult:
        """Analyze crash and determine type"""

        # Check for known invalid pointer patterns
        if crash_context.rax and self._is_invalid_pointer(crash_context.rax):
            return CrashAnalysisResult(
                crash_type=CrashType.INVALID_POINTER,
                confidence=0.95,
                description=f"RAX contains invalid pointer: {hex(crash_context.rax)}. "
                          f"Bit 63 set indicates kernel space or error marker.",
                affected_symbol=self._find_symbol(crash_context.rip),
                affected_address=crash_context.rip,
                recommendations=[
                    "Check dyld's binding/interposing state at this instruction",
                    "Verify no corrupted mach-o load commands",
                    "Review recursive dependency loading logic",
                    "Check for malformed or misinterpreted bind/lazy-bind records",
                ]
            )

        # Check for null pointer
        if crash_context.rax == 0:
            return CrashAnalysisResult(
                crash_type=CrashType.NULL_POINTER,
                confidence=0.90,
                description="RAX is NULL, likely dereferencing null pointer",
                affected_symbol=self._find_symbol(crash_context.rip),
                affected_address=crash_context.rip,
                recommendations=[
                    "Add null pointer checks before memory operations",
                    "Review control flow that could result in null values",
                ]
            )

        # Check for syscall-related patterns
        if self._is_syscall_location(crash_context.rip):
            return CrashAnalysisResult(
                crash_type=CrashType.SYSCALL_FAILURE,
                confidence=0.85,
                description="Crash at syscall instruction. "
                          "May indicate invalid syscall number or arguments.",
                affected_symbol=self._find_symbol(crash_context.rip),
                affected_address=crash_context.rip,
                recommendations=[
                    "Check syscall number in RAX",
                    "Verify syscall arguments in RDI, RSI, RDX",
                    "Check if syscall is available in this kernel version",
                ]
            )

        # Generic analysis
        return CrashAnalysisResult(
            crash_type=CrashType.UNKNOWN,
            confidence=0.0,
            description="Unable to determine crash cause from available information",
            affected_symbol=self._find_symbol(crash_context.rip),
            affected_address=crash_context.rip,
            recommendations=[
                "Obtain full debug symbols",
                "Run under debugger to capture full register state",
                "Enable crash reporter diagnostics",
            ]
        )

    def _is_invalid_pointer(self, value: int) -> bool:
        """Check if value looks like an invalid pointer"""
        # Check bit 63 (high bit)
        if value & (1 << 63):
            return True

        # Check known patterns
        for pattern in self.INVALID_POINTER_MARKERS:
            if (value & 0xf0f0f0f0f0f0f0f0) == (pattern & 0xf0f0f0f0f0f0f0f0):
                return True

        return False

    def _find_symbol(self, address: int) -> Optional[str]:
        """Find symbol containing or near address"""
        # Look for exact match or closest lower address
        closest_addr = None
        closest_name = None

        for addr in sorted(self.symbols.keys()):
            if addr <= address:
                closest_addr = addr
                closest_name = self.symbols[addr]
            else:
                break

        if closest_name:
            offset = address - closest_addr if closest_addr else 0
            if offset == 0:
                return closest_name
            else:
                return f"{closest_name}+0x{offset:x}"

        return None

    def _is_syscall_location(self, address: int) -> bool:
        """Check if address appears to be at a syscall instruction"""
        try:
            # Try otool first (for Mach-O)
            if self.otool_path:
                result = subprocess.run(
                    [self.otool_path, '-tv', self.binary_path],
                    capture_output=True,
                    text=True,
                    timeout=10
                )

                for line in result.stdout.split('\n'):
                    if f'{address:x}' in line and 'syscall' in line.lower():
                        return True

            # Fall back to objdump (for ELF)
            result = subprocess.run(
                ['objdump', '-d', self.binary_path],
                capture_output=True,
                text=True,
                timeout=10
            )

            for line in result.stdout.split('\n'):
                if f'{address:x}' in line and 'syscall' in line.lower():
                    return True
        except Exception:
            pass

        return False


def parse_register_value(value_str: str) -> int:
    """Parse register value (hex or decimal)"""
    value_str = value_str.strip()
    if value_str.startswith(('0x', '0X')):
        return int(value_str, 16)
    return int(value_str, 16)  # Default to hex for crash analysis


def normalize_code_address(raw_addr: int, slide: int) -> int:
    """Normalize a runtime code address to a file-relative address using slide."""
    if slide < 0:
        raise ValueError("slide must be non-negative")
    if raw_addr < slide:
        raise ValueError(f"address {hex(raw_addr)} is smaller than slide {hex(slide)}")
    return raw_addr - slide


def print_analysis_report(analysis: CrashAnalysisResult, crash_context: CrashContext):
    """Pretty-print crash analysis report"""
    print(f"\n{'='*70}")
    print(f"  CRASH ANALYSIS REPORT")
    print(f"{'='*70}\n")

    print(f"Crash Type: {analysis.crash_type.value}")
    print(f"Confidence: {analysis.confidence*100:.0f}%\n")

    print(f"Description:")
    print(f"  {analysis.description}\n")

    print(f"Affected Symbol: {analysis.affected_symbol or 'Unknown'}")
    print(f"Crash Address (RIP): {hex(crash_context.rip)}\n")

    if crash_context.rax is not None:
        print(f"RAX: {hex(crash_context.rax)}")
    if crash_context.rdi is not None:
        print(f"RDI: {hex(crash_context.rdi)}")
    if crash_context.rsi is not None:
        print(f"RSI: {hex(crash_context.rsi)}")
    if crash_context.rdx is not None:
        print(f"RDX: {hex(crash_context.rdx)}")

    if crash_context.stack_trace:
        print(f"\nStack Trace ({len(crash_context.stack_trace)} frames):")
        for i, addr in enumerate(crash_context.stack_trace):
            print(f"  [{i}] {hex(addr)}")

    print(f"\nRecommendations:")
    for i, rec in enumerate(analysis.recommendations, 1):
        print(f"  {i}. {rec}")

    print(f"\n{'='*70}\n")


def main():
    parser = argparse.ArgumentParser(
        description='Comprehensive crash analysis for ravynOS',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  %(prog)s --rip 0x1d9660 --rax 0x8010000000000007 --binary /path/to/binary
  %(prog)s --rip 0x1d9660 --stack-trace 0x...,0x...,0x... --binary /path/to/binary
  %(prog)s --crash-file crash.txt --binary /path/to/binary
        '''
    )

    parser.add_argument('--binary', required=True, help='Path to the crashed binary')
    parser.add_argument('--rip', type=str, help='RIP register value (crash address)')
    parser.add_argument('--rax', type=str, help='RAX register value')
    parser.add_argument('--rdi', type=str, help='RDI register value')
    parser.add_argument('--rsi', type=str, help='RSI register value')
    parser.add_argument('--rdx', type=str, help='RDX register value')
    parser.add_argument('--stack-trace', type=str, help='Stack trace (comma-separated addresses)')
    parser.add_argument('--slide', type=str, help='ASLR slide to subtract from RIP/stack addresses (e.g. 0x11526d000)')
    parser.add_argument('--crash-file', type=str, help='Read crash context from file')
    parser.add_argument('--json', action='store_true', help='Output as JSON')
    parser.add_argument('-v', '--verbose', action='store_true', help='Verbose output')

    args = parser.parse_args()

    try:
        slide = parse_register_value(args.slide) if args.slide else 0

        # Parse crash context
        if args.crash_file:
            # TODO: Parse crash file format
            print("Error: --crash-file not yet implemented", file=sys.stderr)
            return 1

        if not args.rip:
            parser.print_help()
            return 1

        raw_rip = parse_register_value(args.rip)

        # Build crash context
        context = CrashContext(
            rip=normalize_code_address(raw_rip, slide),
            rax=parse_register_value(args.rax) if args.rax else None,
            rdi=parse_register_value(args.rdi) if args.rdi else None,
            rsi=parse_register_value(args.rsi) if args.rsi else None,
            rdx=parse_register_value(args.rdx) if args.rdx else None,
        )

        # Parse stack trace
        if args.stack_trace:
            context.stack_trace = [
                normalize_code_address(parse_register_value(addr.strip()), slide)
                for addr in args.stack_trace.split(',')
            ]

        if slide and not args.json:
            print(f"Applied slide: {hex(slide)}")
            print(f"RIP normalized: {hex(raw_rip)} -> {hex(context.rip)}")

        # Perform analysis
        analyzer = CrashAnalyzer(args.binary, verbose=args.verbose)
        result = analyzer.analyze(context)

        if args.json:
            output = {
                'crash_context': context.to_dict(),
                'analysis': result.to_dict(),
            }
            print(json.dumps(output, indent=2))
        else:
            print_analysis_report(result, context)

        return 0

    except FileNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    except ValueError as e:
        print(f"Error: Invalid register value: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        if args.verbose:
            import traceback
            traceback.print_exc()
        return 1


if __name__ == '__main__':
    sys.exit(main())




