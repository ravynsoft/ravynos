#readelf: -Ah
#name: ELF interAptiv MR2 markings
#as: -32 -march=interaptiv-mr2
#source: empty.s

ELF Header:
#...
  Flags: +0x..93...., .*interaptiv-mr2.*
#...

MIPS ABI Flags Version: 0

ISA: MIPS32r3
GPR size: 32
CPR1 size: 32
CPR2 size: 0
FP ABI: .*
ISA Extension: Imagination interAptiv MR2
ASEs:
	DSP ASE
	Enhanced VA Scheme
	MT ASE
FLAGS 1: .*
FLAGS 2: .*
