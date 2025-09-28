#as: -march=rv32i -mabi=ilp32
#objdump: -dr

.*:     file format elf32-(little|big)riscv


Disassembly of section .text:

0+000 <.text>:
   0:	00100513          	li	a0,1
   4:	00001537          	lui	a0,0x1
   8:	00001537          	lui	a0,0x1
   c:	00150513          	add	a0,a0,1 # 1001 <c>
  10:	00001537          	lui	a0,0x1
  14:	fff50513          	add	a0,a0,-1 # fff <d>
  18:	80000537          	lui	a0,0x80000
  1c:	fff50513          	add	a0,a0,-1 # 7fffffff <e>
  20:	00000513          	li	a0,0
  24:	fff00513          	li	a0,-1
