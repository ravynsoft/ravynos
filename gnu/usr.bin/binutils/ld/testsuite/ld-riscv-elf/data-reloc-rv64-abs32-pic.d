#source: data-reloc.s
#as: -march=rv64i -mabi=lp64 -defsym __abs__=1
#ld: -m[riscv_choose_lp64_emul] -Ttext 0x8000 --defsym _start=0x0 --defsym abs=0x100 --defsym abs_local=0x200 -shared
#objdump: -dR

.*:[ 	]+file format .*


Disassembly of section .text:

0+8000 <.text>:
    8000:	00000100          	.word	0x00000100
    8004:	00000200          	.word	0x00000200
