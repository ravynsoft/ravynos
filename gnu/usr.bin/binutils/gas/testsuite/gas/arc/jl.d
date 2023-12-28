#as: -mcpu=archs
#objdump: -dr --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:

[0-9a-f]+ <text_label>:
   0:	2022 0f80 0000 0000 	jl	0
			4: R_ARC_32_ME	.text
   8:	20e3 0042           	jlne.d	\[r1\]
   c:	78e0                	nop_s
   e:	20e2 0f80 0000 0000 	jl	0
			12: R_ARC_32_ME	.text
