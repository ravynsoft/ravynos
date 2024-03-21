#objdump: -d --prefix-addresses --show-raw-insn
#name: RL78: correct display of SP-relative movw instructions

.*: +file format .*rl78.*

Disassembly of section .text:
0x0+000 cb f8 34 12[	 ]+movw	sp, #0x1234
0x0+004 ae f8[	 ]+movw	ax, sp
0x0+006 be f8[	 ]+movw	sp, ax
0x0+008 af f8 ff[	 ]+movw	ax, !0x000ffff8
0x0+00b bf f8 ff[	 ]+movw	!0x000ffff8, ax
0x0+00e fb f8 ff[	 ]+movw	hl, sp
0x0+011 db f8 ff[	 ]+movw	bc, sp
0x0+014 eb f8 ff[	 ]+movw	de, sp
