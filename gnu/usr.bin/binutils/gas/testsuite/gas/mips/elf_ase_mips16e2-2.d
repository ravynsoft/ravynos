#name: ELF MIPS16e2 ASE markings 2
#source: nop.s
#objdump: -p
#as: -32 -mips16 -mips32r2 -mmips16e2

.*:.*file format.*mips.*
private flags = [0-9a-f]*[4-7c-f]......: .*[[,]mips16[],].*

MIPS ABI Flags Version: 0

ISA: MIPS.*
GPR size: 32
CPR1 size: 32
CPR2 size: 0
FP ABI: Hard float \(double precision\)
ISA Extension: .*
ASEs:
	MIPS16 ASE
	MIPS16e2 ASE
FLAGS 1: 0000000.
FLAGS 2: 00000000
