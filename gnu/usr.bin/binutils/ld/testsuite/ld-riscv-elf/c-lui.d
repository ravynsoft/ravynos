#name: lui to c.lui relaxation
#source: c-lui.s
#as: -march=rv32ic
#ld: -m[riscv_choose_ilp32_emul]
#objdump: -d -M no-aliases,numeric

.*:     file format .*


Disassembly of section \.text:

.* <_start>:
.*:	6085                	c.lui	x1,0x1
.*:	000000b7          	lui	x1,0x0
.*:	00001037          	lui	x0,0x1
.*:	00001137          	lui	x2,0x1
#pass
