#readelf: -Ah
#name: ELF R5900 markings
#as: -32 -march=r5900
#source: empty.s

ELF Header:
#...
  Flags: +0x..92...., .*5900.*
#...

MIPS ABI Flags Version: 0

ISA: MIPS3
GPR size: 32
CPR1 size: 32
CPR2 size: 0
FP ABI: .*
ISA Extension: Toshiba R5900
ASEs:
	None
FLAGS 1: .*
FLAGS 2: .*
