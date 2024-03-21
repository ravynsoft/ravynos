#readelf: -Ah
#name: ELF p6600 markings
#as: -64 -march=p6600
#source: empty.s

ELF Header:
#...
  Flags: +0xa......., .*mips64r6.*
#...

MIPS ABI Flags Version: 0

ISA: MIPS64r6
GPR size: 64
CPR1 size: 128
CPR2 size: 0
FP ABI: .*
ISA Extension: None
ASEs:
	VZ ASE
	MSA ASE
FLAGS 1: .*
FLAGS 2: .*
