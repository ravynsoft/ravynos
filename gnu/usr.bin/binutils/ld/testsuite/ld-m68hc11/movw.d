#source: movw.s
#as: -m m9s12x
#ld: -mm68hc12elf --relax -defsym gp_max_on=0x1234 -defsym gp_clk=0x5432 -defsym small_off=0x5
#objdump: -m m9s12x -d --prefix-addresses -r

tmpdir/dump:     file format elf32-m68hc12


Disassembly of section .text:
00008000 <_start> movw	0x1234,X, 0x5432,X
00008008 <_start\+0x8> movw	0x22,SP, 0x5432,Y
0000800f <_start\+0xf> movw	0x5432,X, 0x12,SP
00008016 <_start\+0x16> movw	0x1001,X, 0x2002,Y
0000801e <_start\+0x1e> movw	0x5,SP, 0x1234,Y
00008026 <_start\+0x26> tfr	X,Y
00008028 <_start\+0x28> rts
00008029 <_etext> nop

