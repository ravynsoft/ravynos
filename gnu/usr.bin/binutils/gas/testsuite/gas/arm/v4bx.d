# objdump: -dr --prefix-addresses --show-raw-insn
# as: -meabi=4 --fix-v4bx
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
0+00 <[^>]+> e12fff1e 	bx	lr
			0: R_ARM_V4BX	\*ABS\*
