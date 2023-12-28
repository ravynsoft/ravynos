# name: ELF microMIPS ASE markings 2
# source: nop.s
# objdump: -p
# as: -32 -mmicromips

.*:.*file format.*mips.*
private flags = [0-9a-f]*[2367abef]......: .*[[,]micromips[],].*

MIPS ABI Flags Version: 0

ISA: MIPS.*
GPR size: 32
CPR1 size: 32
CPR2 size: 0
FP ABI: Hard float \(double precision\)
ISA Extension: .*
ASEs:
	MICROMIPS ASE
FLAGS 1: 0000000.
FLAGS 2: 00000000

