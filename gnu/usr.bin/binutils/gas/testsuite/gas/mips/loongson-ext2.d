#as: -mloongson-ext2 -mabi=64
#objdump: -M reg-names=numeric -M loongson-ext2 -dp
#name: Loongson EXT2 tests

.*:     file format .*

private flags = .*

MIPS ABI Flags Version: 0
ISA: .*
GPR size: .*
CPR1 size: .*
CPR2 size: .*
FP ABI: .*
ISA Extension: None
ASEs:
	Loongson EXT ASE
	Loongson EXT2 ASE
FLAGS 1: .*
FLAGS 2: .*

Disassembly of section .text:

[0-9a-f]+ <.text>:
.*:	70801062 	cto	\$2,\$4
.*:	70801022 	ctz	\$2,\$4
.*:	708010e2 	dcto	\$2,\$4
.*:	708010a2 	dctz	\$2,\$4
