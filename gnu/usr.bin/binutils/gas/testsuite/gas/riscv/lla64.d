#as: -march=rv64i -mabi=lp64
#objdump: -dr

.*:     file format elf64-(little|big)riscv


Disassembly of section .text:

0+000 <.text>:
   0:	0010051b          	addw	a0,zero,1
   4:	00001537          	lui	a0,0x1
   8:	00001537          	lui	a0,0x1
   c:	0015051b          	addw	a0,a0,1 # .*
  10:	00001537          	lui	a0,0x1
  14:	fff5051b          	addw	a0,a0,-1 # .*
  18:	80000537          	lui	a0,0x80000
  1c:	fff5051b          	addw	a0,a0,-1 # .*
  20:	0000051b          	sext.w	a0,zero
  24:	fff0051b          	addw	a0,zero,-1
  28:	80000537          	lui	a0,0x80000
