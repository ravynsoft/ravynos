#as: -mcpu=archs
#objdump: -dr --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:

[0-9a-f]+ <.text>:
   0:	1a00 0040           	st	r1,\[r2\]
   4:	1a0e 0040           	st	r1,\[r2,14\]
   8:	1a00 0042           	stb	r1,\[r2\]
   c:	1b0e 0048           	st.aw	r1,\[r3,14\]
  10:	1a02 004c           	st[hw]+.aw	r1,\[r2,2\]
  14:	1e00 7040 0000 0384 	st	r1,\[0x384\]
  1c:	1a00 0003           	stb	0,\[r2\]
  20:	1af8 8e01           	st	-8,\[r2,-8\]
  24:	1e00 7080 0000 0000 	st	r2,\[0\]
			28: R_ARC_32_ME	foo
  2c:	1a02 0060           	st.di	r1,\[r2,2\]
  30:	1a03 0068           	st.di.aw	r1,\[r2,3\]
  34:	1a04 006c           	st[hw]+.di.aw	r1,\[r2,4\]
  38:	1c04 1f80 0000 0000 	st	0,\[r12,4\]
			3c: R_ARC_32_ME	.text\+0x40
  40:	212b 0080           	sr	r1,\[r2\]
  44:	216b 0380           	sr	r1,\[aux_irq_ctrl\]
  48:	262b 7040 0000 03e8 	sr	0x3e8,\[r1\]
  50:	262b 7080 0000 0064 	sr	0x64,\[r2\]
  58:	212b 0f80 0000 2710 	sr	r1,\[0x2710\]
  60:	266b 7fc0 0000 0064 	sr	0x64,\[0x3f\]
  68:	26ab 7901 0000 2710 	sr	0x2710,\[vbfdw_build\]
