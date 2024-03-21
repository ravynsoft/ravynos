#as: -mabi=32 -mips2 -call_nonpic
#objdump: -pdr

.*
private flags = 10001004: .*

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


Disassembly of section \.text:

0+0 <\.text>:
.* 	lui	t9,0x0
.*: R_MIPS_HI16	foo
.* 	addiu	t9,t9,0
.*: R_MIPS_LO16	foo
.* 	jalr	t9
.* 	nop
