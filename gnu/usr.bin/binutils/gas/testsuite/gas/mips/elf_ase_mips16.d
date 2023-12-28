# name: ELF MIPS16 ASE markings
# source: empty.s
# objdump: -p
# as: -32 -mips16

.*:.*file format.*mips.*
!private flags = .*mips16.*

MIPS ABI Flags Version: 0

ISA: MIPS.*
GPR size: 32
CPR1 size: 32
CPR2 size: 0
FP ABI: Hard float \(double precision\)
ISA Extension: .*
ASEs:
	None
FLAGS 1: 0000000.
FLAGS 2: 00000000

