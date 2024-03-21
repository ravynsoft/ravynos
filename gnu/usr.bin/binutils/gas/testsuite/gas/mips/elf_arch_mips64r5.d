# name: ELF MIPS64r5 markings
# source: empty.s
# objdump: -p
# as: -32 -march=mips64r5

.*:.*file format.*elf.*mips.*
private flags = 8.......: .*\[mips64r2\].*

MIPS ABI Flags Version: 0

ISA: MIPS64r5
GPR size: 32
CPR1 size: 32
CPR2 size: 0
FP ABI: Hard float \(double precision\)
ISA Extension: None
ASEs:
	None
FLAGS 1: 0000000.
FLAGS 2: 00000000

