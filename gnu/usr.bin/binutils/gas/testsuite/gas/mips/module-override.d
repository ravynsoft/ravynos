# name: .module command line override
# source: module-override.s
# objdump: -p
# as: -32 -march=mips32r2

.*:.*file format.*elf.*mips.*
private flags = 1.......: .*\[mips2\].*

MIPS ABI Flags Version: 0

ISA: MIPS2
GPR size: 32
CPR1 size: 32
CPR2 size: 0
FP ABI: Hard float \(double precision\)
ISA Extension: None
ASEs:
	None
FLAGS 1: 00000000
FLAGS 2: 00000000
