#name: s390 opcode
#objdump: -drw

.*: +file format .*

Disassembly of section .text:

.* <foo>:
.*:	b9 93 f0 69 [	 ]*troo	%r6,%r9,15
.*:	b9 93 00 69 [	 ]*troo	%r6,%r9
.*:	b9 92 f0 69 [	 ]*trot	%r6,%r9,15
.*:	b9 92 00 69 [	 ]*trot	%r6,%r9
.*:	b9 91 f0 69 [	 ]*trto	%r6,%r9,15
.*:	b9 91 00 69 [	 ]*trto	%r6,%r9
.*:	b9 90 f0 69 [	 ]*trtt	%r6,%r9,15
.*:	b9 90 00 69 [	 ]*trtt	%r6,%r9
.*:	b2 2b 00 69 [	 ]*sske	%r6,%r9
