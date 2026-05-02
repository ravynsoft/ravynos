#!/usr/bin/env python3
"""
Symbolication Tool for ravynOS Crash Analysis

This tool maps crash addresses to symbols in compiled binaries, helping identify
the exact function and instruction where a crash occurred.

Usage:
    ./symbolicate.py <binary_path> <address> [<address2> ...]
    ./symbolicate.py <binary_path> --rip <address> --rax <address>
    ./symbolicate.py <binary_path> --stack-trace <addr1>,<addr2>,<addr3>

Examples:
    ./symbolicate.py /path/to/launchd 0x1d9660
    ./symbolicate.py /path/to/dyld 0x00000001154ad2eb
    ./symbolicate.py /path/to/dyld --rip 0x1d9660 --rax 0x8010000000000007
    ./symbolicate.py /path/to/dyld --stack-trace 0x1d9660,0x1d9700,0x1d9750
"""

import sys
import os
import subprocess
import re
import argparse
from dataclasses import dataclass
from typing import List, Optional, Dict, Tuple
import json

@dataclass
class Symbol:
    """Represents a symbol in the binary"""
    address: int
    size: int
    name: str
    section: str
    is_function: bool

    def contains(self, addr: int) -> bool:
        """Check if address falls within this symbol's range"""
        return self.address <= addr < (self.address + self.size)

    def offset(self, addr: int) -> int:
        """Get offset from symbol start to given address"""
        return addr - self.address

    def __repr__(self) -> str:
        offset_str = f"+0x{self.offset(addr):x}" if hasattr(self, 'addr') else ""
        return f"{self.name} ({self.section}) @ 0x{self.address:x}"


@dataclass
class SymbolicationResult:
    """Result of symbolication for a single address"""
    address: int
    symbol: Optional[Symbol]
    context: Optional[str]  # e.g., "first instruction", "middle of function"

    def __str__(self) -> str:
        if not self.symbol:
            return f"0x{self.address:x} - Unknown symbol"

        offset = self.offset()
        if offset == 0:
            offset_str = " (entry point)"
        else:
            offset_str = f" +0x{offset:x}"

        return f"0x{self.address:x} -> {self.symbol.name}{offset_str} ({self.symbol.section})"

    def offset(self) -> int:
        if self.symbol:
            return self.symbol.offset(self.address)
        return 0


class Symbolizer:
    """Main symbolication engine"""

    def __init__(self, binary_path: str, verbose: bool = False):
        self.binary_path = binary_path
        self.verbose = verbose
        self.symbols: List[Symbol] = []
        self._symbol_map: Dict[int, Symbol] = {}

        if not os.path.exists(binary_path):
            raise FileNotFoundError(f"Binary not found: {binary_path}")

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
        """Load symbols from the binary using nm or otool"""
        try:
            # First, check if this is a Mach-O binary (ravynOS/macOS)
            if self._is_macho_binary():
                self._load_symbols_from_macho()
            else:
                # Fall back to ELF (nm)
                self._load_symbols_from_elf()

        except FileNotFoundError:
            raise RuntimeError("Symbol tools not found. Install binutils and/or otool.")
        except subprocess.TimeoutExpired:
            raise RuntimeError("Symbol loading timed out")

        if self.verbose:
            print(f"Loaded {len(self.symbols)} symbols", file=sys.stderr)

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
        result = subprocess.run(
            ['nm', '-nSt', self.binary_path],
            capture_output=True,
            text=True,
            timeout=30
        )

        if result.returncode != 0:
            if self.verbose:
                print(f"Warning: nm failed: {result.stderr}", file=sys.stderr)
            return

        self._parse_nm_output(result.stdout)

    def _load_symbols_from_macho(self):
        """Load symbols from Mach-O binary using otool"""
        # Find otool
        otool_path = self._find_otool()
        if not otool_path:
            if self.verbose:
                print(f"Warning: otool not found", file=sys.stderr)
            return

        # Get disassembly with symbols
        result = subprocess.run(
            [otool_path, '-tv', self.binary_path],
            capture_output=True,
            text=True,
            timeout=30
        )

        if result.returncode != 0:
            if self.verbose:
                print(f"Warning: otool -tv failed: {result.stderr}", file=sys.stderr)
            return

        self._parse_otool_output(result.stdout)

    def _parse_nm_output(self, output: str):
        """Parse nm output in format: address size type section name"""
        # Format from 'nm -nSt': address size type section name
        pattern = r'^([0-9a-f]+)\s+([0-9a-f]+)\s+(.)\s+(\S+)\s+(.+)$'

        for line in output.strip().split('\n'):
            if not line.strip():
                continue

            match = re.match(pattern, line)
            if not match:
                continue

            addr_str, size_str, sym_type, section, name = match.groups()

            try:
                address = int(addr_str, 16)
                size = int(size_str, 16)
                is_function = sym_type in 'tT'  # lowercase = local, uppercase = global

                symbol = Symbol(
                    address=address,
                    size=size,
                    name=name,
                    section=section,
                    is_function=is_function
                )

                self.symbols.append(symbol)
                if address not in self._symbol_map:
                    self._symbol_map[address] = symbol

            except (ValueError, IndexError) as e:
                if self.verbose:
                    print(f"Warning: Failed to parse symbol line: {line}", file=sys.stderr)

        # Sort symbols by address for binary search
        self.symbols.sort(key=lambda s: s.address)

    def _parse_otool_output(self, output: str):
        """Parse otool -tv output for Mach-O binaries

        Format example:
        _main:
        0000000100000be0        pushq   %rbp

        Or with actual symbol names at the start
        """
        current_symbol = None
        current_address = None
        max_instruction_address = None

        for line in output.strip().split('\n'):
            line = line.strip()
            if not line:
                continue

            # Check for symbol name (function definition)
            # Format: _functionname: or functionname:
            if line.endswith(':') and not line[0].isdigit():
                symbol_name = line[:-1]
                current_symbol = symbol_name
                current_address = None
                continue

            # Parse instruction lines to get addresses
            # Format: 0000000100000be0        pushq   %rbp
            parts = line.split()
            if len(parts) >= 2 and all(c in '0123456789abcdef' for c in parts[0]):
                try:
                    address = int(parts[0], 16)
                    if max_instruction_address is None or address > max_instruction_address:
                        max_instruction_address = address

                    if current_symbol and current_address is None:
                        # This is the first instruction of a symbol
                        current_address = address

                        # Create symbol entry
                        symbol = Symbol(
                            address=address,
                            size=0,  # We don't know the size from otool -tv alone
                            name=current_symbol,
                            section='__text',  # Usually in text section
                            is_function=True
                        )

                        self.symbols.append(symbol)
                        if address not in self._symbol_map:
                            self._symbol_map[address] = symbol

                        current_symbol = None

                except (ValueError, IndexError):
                    pass

        # Sort symbols and infer synthetic sizes so range lookups work for Mach-O.
        self.symbols.sort(key=lambda s: s.address)
        for i in range(len(self.symbols) - 1):
            next_addr = self.symbols[i + 1].address
            curr_addr = self.symbols[i].address
            inferred = max(1, next_addr - curr_addr)
            if self.symbols[i].size == 0:
                self.symbols[i].size = inferred

        # Keep the final symbol searchable for nearby addresses.
        if self.symbols and self.symbols[-1].size == 0:
            if max_instruction_address is not None and max_instruction_address >= self.symbols[-1].address:
                self.symbols[-1].size = (max_instruction_address - self.symbols[-1].address) + 1
            else:
                self.symbols[-1].size = 1

    def symbolicate(self, address: int) -> SymbolicationResult:
        """Find the symbol containing the given address"""

        # Quick lookup if exact address exists
        if address in self._symbol_map:
            return SymbolicationResult(
                address=address,
                symbol=self._symbol_map[address],
                context="entry point"
            )

        # Binary search for containing symbol
        symbol = self._find_containing_symbol(address)

        if symbol:
            context = None
            offset = symbol.offset(address)

            # Provide context based on offset
            if offset == 0:
                context = "entry point"
            elif offset < 32:  # Typical prologue size
                context = "in prologue"
            elif offset == symbol.size - 1:
                context = "at exit"

            return SymbolicationResult(
                address=address,
                symbol=symbol,
                context=context
            )

        return SymbolicationResult(address=address, symbol=None, context=None)

    def _find_containing_symbol(self, address: int) -> Optional[Symbol]:
        """Binary search to find symbol containing address"""
        left, right = 0, len(self.symbols) - 1

        while left <= right:
            mid = (left + right) // 2
            symbol = self.symbols[mid]

            if symbol.contains(address):
                return symbol
            elif symbol.address > address:
                right = mid - 1
            else:
                left = mid + 1

        # Check the symbol just before the address
        if right >= 0 and right < len(self.symbols):
            symbol = self.symbols[right]
            if symbol.contains(address):
                return symbol

        return None

    def symbolicate_multiple(self, addresses: List[int]) -> List[SymbolicationResult]:
        """Symbolicate multiple addresses"""
        return [self.symbolicate(addr) for addr in addresses]

    def find_symbol_by_name(self, name: str, exact: bool = False) -> List[Symbol]:
        """Find symbols by name (substring or exact match)"""
        results = []
        for symbol in self.symbols:
            if exact:
                if symbol.name == name:
                    results.append(symbol)
            else:
                if name.lower() in symbol.name.lower():
                    results.append(symbol)
        return results

    def get_context_around_address(self, address: int, before: int = 5, after: int = 5) -> List[Symbol]:
        """Get symbols around a given address"""
        idx = self._find_symbol_index(address)
        if idx is None:
            return []

        start = max(0, idx - before)
        end = min(len(self.symbols), idx + after + 1)
        return self.symbols[start:end]

    def _find_symbol_index(self, address: int) -> Optional[int]:
        """Find the index of a symbol containing or near the address"""
        left, right = 0, len(self.symbols) - 1

        while left <= right:
            mid = (left + right) // 2
            symbol = self.symbols[mid]

            if symbol.contains(address):
                return mid
            elif symbol.address > address:
                right = mid - 1
            else:
                left = mid + 1

        return right if right >= 0 else None


def parse_hex_address(addr_str: str) -> int:
    """Parse address string (with or without 0x prefix)"""
    addr_str = addr_str.strip()
    if addr_str.startswith(('0x', '0X')):
        return int(addr_str, 16)
    return int(addr_str, 16)


def format_hex_address(addr: int) -> str:
    """Format address as hex string"""
    return f"0x{addr:x}"


def normalize_code_address(raw_addr: int, slide: int) -> int:
    """Normalize a runtime code address to a file-relative address using slide."""
    if slide < 0:
        raise ValueError("slide must be non-negative")
    if raw_addr < slide:
        raise ValueError(f"address {format_hex_address(raw_addr)} is smaller than slide {format_hex_address(slide)}")
    return raw_addr - slide


def main():
    parser = argparse.ArgumentParser(
        description='Symbolicate crash addresses in ravynOS binaries',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  %(prog)s /path/to/launchd 0x1d9660
  %(prog)s /path/to/dyld 0x1d9660 0x1d9700 0x1d9750
  %(prog)s /path/to/dyld --rip 0x1d9660
  %(prog)s /path/to/dyld --stack-trace 0x1d9660,0x1d9700,0x1d9750
  %(prog)s /path/to/dyld --search syscall
        '''
    )

    parser.add_argument('binary', help='Path to the binary to symbolicate')
    parser.add_argument('addresses', nargs='*', help='Addresses to symbolicate')
    parser.add_argument('--rip', type=str, help='RIP register value (crashed instruction pointer)')
    parser.add_argument('--rax', type=str, help='RAX register value (for context)')
    parser.add_argument('--stack-trace', type=str, help='Stack trace addresses (comma-separated)')
    parser.add_argument('--slide', type=str, help='ASLR slide to subtract from code addresses (e.g. 0x11526d000)')
    parser.add_argument('--search', type=str, help='Search for symbol by name')
    parser.add_argument('--context', type=int, default=5, help='Symbols to show around address (default: 5)')
    parser.add_argument('--json', action='store_true', help='Output as JSON')
    parser.add_argument('-v', '--verbose', action='store_true', help='Verbose output')

    args = parser.parse_args()

    try:
        slide = parse_hex_address(args.slide) if args.slide else 0

        # Initialize symbolizer
        symbolizer = Symbolizer(args.binary, verbose=args.verbose)

        # Handle search mode
        if args.search:
            print(f"\nSearching for symbols matching: {args.search}")
            results = symbolizer.find_symbol_by_name(args.search)
            if results:
                for symbol in results:
                    print(f"  0x{symbol.address:x} {symbol.name} ({symbol.section}) size=0x{symbol.size:x}")
            else:
                print("  No symbols found")
            return 0

        # Collect addresses to symbolicate
        addresses_to_symbolicate = []

        # Add RIP if provided
        if args.rip:
            try:
                raw_rip = parse_hex_address(args.rip)
                normalized_rip = normalize_code_address(raw_rip, slide)
                addresses_to_symbolicate.append(normalized_rip)
                if slide:
                    print(f"RIP: {args.rip} -> {format_hex_address(normalized_rip)} (slide={format_hex_address(slide)})")
                else:
                    print(f"RIP: {args.rip}")
            except ValueError:
                print(f"Error: Invalid RIP address: {args.rip}", file=sys.stderr)
                return 1

        # Add RAX if provided (for reference)
        if args.rax:
            try:
                rax_addr = parse_hex_address(args.rax)
                print(f"RAX: {args.rax} (invalid pointer marker)")
            except ValueError:
                print(f"Error: Invalid RAX address: {args.rax}", file=sys.stderr)

        # Add stack trace addresses
        if args.stack_trace:
            try:
                for addr_str in args.stack_trace.split(','):
                    raw_addr = parse_hex_address(addr_str.strip())
                    addresses_to_symbolicate.append(normalize_code_address(raw_addr, slide))
            except ValueError as e:
                print(f"Error: Invalid stack trace address: {e}", file=sys.stderr)
                return 1

        # Add positional arguments
        for addr_str in args.addresses:
            try:
                raw_addr = parse_hex_address(addr_str)
                addresses_to_symbolicate.append(normalize_code_address(raw_addr, slide))
            except ValueError:
                print(f"Error: Invalid address: {addr_str}", file=sys.stderr)
                return 1

        if not addresses_to_symbolicate:
            parser.print_help()
            return 1

        # Symbolicate
        results = symbolizer.symbolicate_multiple(addresses_to_symbolicate)

        if args.json:
            # JSON output
            json_results = []
            for result in results:
                json_obj = {
                    'address': format_hex_address(result.address),
                    'symbol': result.symbol.name if result.symbol else None,
                    'section': result.symbol.section if result.symbol else None,
                    'offset': f"0x{result.offset():x}" if result.symbol else None,
                    'context': result.context
                }
                json_results.append(json_obj)
            print(json.dumps(json_results, indent=2))
        else:
            # Human-readable output
            print("\n=== Symbolication Results ===\n")

            for i, result in enumerate(results):
                print(f"{i+1}. {result}")

                # Show context
                if result.symbol:
                    print(f"   Size: 0x{result.symbol.size:x} bytes")
                    print(f"   Offset: 0x{result.offset():x}")
                    if result.context:
                        print(f"   Context: {result.context}")

                    # Show nearby symbols
                    if args.context > 0:
                        context_symbols = symbolizer.get_context_around_address(
                            result.address,
                            before=args.context,
                            after=args.context
                        )
                        print(f"   Nearby symbols:")
                        for nearby in context_symbols:
                            marker = " <--" if nearby == result.symbol else ""
                            print(f"     0x{nearby.address:x} {nearby.name}{marker}")

                print()

        return 0

    except FileNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    except RuntimeError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"Unexpected error: {e}", file=sys.stderr)
        if args.verbose:
            import traceback
            traceback.print_exc()
        return 1


if __name__ == '__main__':
    sys.exit(main())








