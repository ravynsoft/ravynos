# name: ELF MIPS64 markings
# source: empty.s
# objdump: -p
# as: -32 -march=mips64

.*:.*file format.*elf.*mips.*
private flags = 6.......: .*\[mips64\].*

MIPS ABI Flags Version: 0

ISA: MIPS64
GPR size: 32
CPR1 size: 32
CPR2 size: 0
FP ABI: Hard float \(double precision\)
ISA Extension: None
ASEs:
	None
FLAGS 1: 0000000.
FLAGS 2: 00000000

