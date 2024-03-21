#readelf: -Ah
#name: LSI 4010 processor ISA level
#source: empty.s
#as: -m4010
#ld: -r

ELF Header:
#...
  Flags:                             0x1082[01]000, 4010(?:, o32)?, mips2
#...

MIPS ABI Flags Version: 0

ISA: MIPS2
GPR size: 32
CPR1 size: 32
CPR2 size: 0
FP ABI: .*
ISA Extension: LSI R4010
ASEs:
	None
FLAGS 1: .*
FLAGS 2: .*
