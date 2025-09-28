#objdump: -dr -M reg-names=numeric
#name: MIPS move disassembly test
#source: move.s

# Check objdump's disassembly of the move menomic for addu, daddu and or.

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	03e08025 	move	\$16,\$31
   4:	03e08021 	move	\$16,\$31
   8:	03e0802d 	move	\$16,\$31
   c:	03e08025 	move	\$16,\$31
