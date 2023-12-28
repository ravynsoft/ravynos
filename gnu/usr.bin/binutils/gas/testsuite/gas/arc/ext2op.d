#as: -mcpu=arcem
#objdump: -dr -M quarkse_em

.*: +file format .*arc.*


Disassembly of section .text:

[0-9a-f]+ <.text>:
   0:	382f 006d           	dsp_fp_sqrt	r0,r1
   4:	3b2f 372d           	dsp_fp_sqrt	fp,sp
   8:	386f 002d           	dsp_fp_sqrt	r0,0
   c:	392f 0fad ffff ffff 	dsp_fp_sqrt	r1,0xffffffff
  14:	3e2f 70ad           	dsp_fp_sqrt	0,r2
  18:	3c2f 0fad 0000 00ff 	dsp_fp_sqrt	r4,0xff
  20:	3e2f 0fad ffff ff00 	dsp_fp_sqrt	r6,0xffffff00
  28:	382f 1fad 0000 0100 	dsp_fp_sqrt	r8,0x100
  30:	392f 1fad ffff feff 	dsp_fp_sqrt	r9,0xfffffeff
  38:	3b2f 1fad 4242 4242 	dsp_fp_sqrt	r11,0x42424242
  40:	382f 0fad 0000 0000 	dsp_fp_sqrt	r0,0
			44: R_ARC_32_ME	foo
  48:	382f 806d           	dsp_fp_sqrt.f	r0,r1
  4c:	3a6f 806d           	dsp_fp_sqrt.f	r2,0x1
  50:	3e2f f12d           	dsp_fp_sqrt.f	0,r4
  54:	3d2f 8fad 0000 0200 	dsp_fp_sqrt.f	r5,0x200
