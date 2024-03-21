#name: s390 opcode
#objdump: -drw

.*: +file format .*

Disassembly of section .text:

.* <foo>:
.*:	b9 2e 00 69 [	 ]*km	%r6,%r9
.*:	b9 2f 00 69 [	 ]*kmc	%r6,%r9
.*:	b9 3e 00 69 [	 ]*kimd	%r6,%r9
.*:	b9 3f 00 69 [	 ]*klmd	%r6,%r9
.*:	b9 1e 00 69 [	 ]*kmac	%r6,%r9
.*:	eb 69 50 00 80 8f [	 ]*clclu	%r6,%r9,-524288\(%r5\)
.*:	07 07 [ 	]*nopr	%r7
