#objdump: -d --prefix-addresses --reloc
#as: 
#name: hilo

# Test for correct generation of XGATE insns when using the %hi and %lo modifiers.

.*: +file format elf32\-xgate

Disassembly of section .text:
0+0000 <hiTestLo> ldl R2, #0x88
0+0002 <hiTestHi> ldh R2, #0x88 Abs\* 0x00008888 <symValue\+0x8870>
0+0004 <loTestLo> ldl R3, #0x44
0+0006 <loTestHi> ldh R3, #0x44 Abs\* 0x00004444 <symValue\+0x442c>
0+0008 <hiTestLoF> ldl R2, #0xff
0+000a <hiTestHiF> ldh R2, #0xff Abs\* 0x0000ffff <test\+0x77>
0+000c <loTestLoF> ldl R3, #0x88
0+000e <loTestHiF> ldh R3, #0x88 Abs\* 0x00008888 <symValue\+0x8870>
0+0010 <hiTestLoR> ldl R2, #0x00
			10: R_XGATE_IMM8_HI	.text
0+0012 <hiTestHiR> ldh R2, #0x00 Abs\* 0x00000000 <hiTestLo>
			12: R_XGATE_IMM8_HI	.text
0+0014 <loTestLoR> ldl R3, #0x18
			14: R_XGATE_IMM8_LO	.text
0+0016 <loTestHiR> ldh R3, #0x18 Abs\* 0x00001818 <symValue\+0x1800>
			16: R_XGATE_IMM8_LO	.text

