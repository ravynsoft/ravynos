#as: -mcpu=arc700 -mnps400
#objdump: -dr

.*: +file format .*arc.*

Disassembly of section .text:

[0-9a-f]+ <.*>:
   0:	5988 0000           	xldb	r12,\[0x57f00000\]
   4:	5ae8 ffff           	xldb	r23,\[0x57f0ffff\]
   8:	5868 0000           	xldb	r3,\[0x57f00000\]
   c:	5968 ffff           	xldb	r11,\[0x57f0ffff\]
  10:	5a88 0000           	xldb	r20,\[0x57f00000\]
			10: R_ARC_NPS_CMEM16	foo
  14:	5828 0000           	xldb	r1,\[0x57f00000\]
			14: R_ARC_NPS_CMEM16	foo\+0x20
  18:	5989 0000           	xldw	r12,\[0x57f00000\]
  1c:	5ae9 ffff           	xldw	r23,\[0x57f0ffff\]
  20:	5869 0000           	xldw	r3,\[0x57f00000\]
  24:	5969 ffff           	xldw	r11,\[0x57f0ffff\]
  28:	5a89 0000           	xldw	r20,\[0x57f00000\]
			28: R_ARC_NPS_CMEM16	foo
  2c:	5829 0000           	xldw	r1,\[0x57f00000\]
			2c: R_ARC_NPS_CMEM16	foo\+0x20
  30:	598a 0000           	xld	r12,\[0x57f00000\]
  34:	5aea ffff           	xld	r23,\[0x57f0ffff\]
  38:	586a 0000           	xld	r3,\[0x57f00000\]
  3c:	596a ffff           	xld	r11,\[0x57f0ffff\]
  40:	5a8a 0000           	xld	r20,\[0x57f00000\]
			40: R_ARC_NPS_CMEM16	foo
  44:	582a 0000           	xld	r1,\[0x57f00000\]
			44: R_ARC_NPS_CMEM16	foo\+0x20
  48:	598c 0000           	xstb	r12,\[0x57f00000\]
  4c:	5aec ffff           	xstb	r23,\[0x57f0ffff\]
  50:	586c 0000           	xstb	r3,\[0x57f00000\]
  54:	596c ffff           	xstb	r11,\[0x57f0ffff\]
  58:	5a8c 0000           	xstb	r20,\[0x57f00000\]
			58: R_ARC_NPS_CMEM16	foo
  5c:	582c 0000           	xstb	r1,\[0x57f00000\]
			5c: R_ARC_NPS_CMEM16	foo\+0x20
  60:	598d 0000           	xstw	r12,\[0x57f00000\]
  64:	5aed ffff           	xstw	r23,\[0x57f0ffff\]
  68:	586d 0000           	xstw	r3,\[0x57f00000\]
  6c:	596d ffff           	xstw	r11,\[0x57f0ffff\]
  70:	5a8d 0000           	xstw	r20,\[0x57f00000\]
			70: R_ARC_NPS_CMEM16	foo
  74:	582d 0000           	xstw	r1,\[0x57f00000\]
			74: R_ARC_NPS_CMEM16	foo\+0x20
  78:	598e 0000           	xst	r12,\[0x57f00000\]
  7c:	5aee ffff           	xst	r23,\[0x57f0ffff\]
  80:	586e 0000           	xst	r3,\[0x57f00000\]
  84:	596e ffff           	xst	r11,\[0x57f0ffff\]
  88:	5a8e 0000           	xst	r20,\[0x57f00000\]
			88: R_ARC_NPS_CMEM16	foo
  8c:	582e 0000           	xst	r1,\[0x57f00000\]
			8c: R_ARC_NPS_CMEM16	foo\+0x20
