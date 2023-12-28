# name: ELF MIPS32 markings
# source: empty.s
# objdump: -p
# as: -32 -march=mips32

.*:.*file format.*elf.*mips.*
private flags = 5.......: .*\[mips32\].*

MIPS ABI Flags Version: 0

ISA: MIPS32
GPR size: 32
CPR1 size: 32
CPR2 size: 0
FP ABI: Hard float \(double precision\)
ISA Extension: None
ASEs:
	None
FLAGS 1: 0000000.
FLAGS 2: 00000000

