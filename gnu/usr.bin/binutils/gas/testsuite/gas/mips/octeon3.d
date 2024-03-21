#objdump: -d -r --show-raw-insn -Mgpr-names=numeric
#name: MIPS octeon3 instructions

.*: +file format .*mips.*

Disassembly of section .text:

[0-9a-f]+ <foo>:
.*:	71ec0008 	mtm0	\$15,\$12
.*:	71a40008 	mtm0	\$13,\$4
.*:	7083000c 	mtm1	\$4,\$3
.*:	70e1000c 	mtm1	\$7,\$1
.*:	7022000d 	mtm2	\$1,\$2
.*:	7083000c 	mtm1	\$4,\$3
.*:	70a20009 	mtp0	\$5,\$2
.*:	70c40009 	mtp0	\$6,\$4
.*:	7083000a 	mtp1	\$4,\$3
.*:	70e1000a 	mtp1	\$7,\$1
.*:	7022000b 	mtp2	\$1,\$2
.*:	7083000a 	mtp1	\$4,\$3
