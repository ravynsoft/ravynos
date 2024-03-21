# name: ELF MIPS64r6 markings
# source: empty.s
# objdump: -p
# as: -32 -march=mips64r6

.*:.*file format.*elf.*mips.*
private flags = a.......: .*\[mips64r6\].*

MIPS ABI Flags Version: 0

ISA: MIPS64r6
GPR size: 32
CPR1 size: 64
CPR2 size: 0
FP ABI: Hard float \(32-bit CPU, 64-bit FPU\)
ISA Extension: None
ASEs:
	None
FLAGS 1: 0000000.
FLAGS 2: 00000000

