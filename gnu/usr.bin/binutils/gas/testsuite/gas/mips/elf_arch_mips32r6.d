# name: ELF MIPS32r5 markings
# source: empty.s
# objdump: -p
# as: -32 -march=mips32r6

.*:.*file format.*elf.*mips.*
private flags = 9.......: .*\[mips32r6\].*

MIPS ABI Flags Version: 0

ISA: MIPS32r6
GPR size: 32
CPR1 size: 64
CPR2 size: 0
FP ABI: Hard float \(32-bit CPU, 64-bit FPU\)
ISA Extension: None
ASEs:
	None
FLAGS 1: 0000000.
FLAGS 2: 00000000

