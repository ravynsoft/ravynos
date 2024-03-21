# name: ELF MIPS1 markings
# source: empty.s
# objdump: -p
# as: -32 -march=mips1

.*:.*file format.*elf.*mips.*
# Note: objdump omits leading zeros, so must check for the fact that
# flags are _not_ 8 chars long.
private flags = (.......|......|.....|....|...|..|.): .*\[mips1\].*

MIPS ABI Flags Version: 0

ISA: MIPS1
GPR size: 32
CPR1 size: 32
CPR2 size: 0
FP ABI: Hard float \(double precision\)
ISA Extension: None
ASEs:
	None
FLAGS 1: 00000000
FLAGS 2: 00000000

