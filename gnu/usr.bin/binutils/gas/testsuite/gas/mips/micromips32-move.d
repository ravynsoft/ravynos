#objdump: -dr -m mips:micromips -M reg-names=numeric
#name: microMIPS insn32 move test
#source: micromips32-move.s

# Check objdump's disassembly of the move menomic for addu, daddu and or.

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	001f 6a90 	move	\$13,\$31
   4:	001f 6950 	move	\$13,\$31
   8:	581f 6950 	move	\$13,\$31
   c:	001f 6a90 	move	\$13,\$31
