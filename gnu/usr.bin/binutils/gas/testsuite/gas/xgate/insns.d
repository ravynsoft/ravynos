#objdump: -d --prefix-addresses --reloc
#as: 
#name: insns

# Test handling of basic instructions.

.*: +file format elf32\-xgate

Disassembly of section .text:
0+0000 <\_start> ldl R2, #0x00
			0: R_XGATE_IMM8_LO	.bss
0+0002 <\_start\+0x2> ldh R2, #0x04 Abs\* 0x00000400 <block_end>
			2: R_XGATE_IMM8_HI	.bss
0+0004 <\_start\+0x4> ldl R3, #0x00
			4: R_XGATE_IMM8_LO	.bss
0+0006 <\_start\+0x6> ldh R3, #0x00 Abs\* 0x00000000 <\_start>
			6: R_XGATE_IMM8_HI	.bss
0+0008 <\_start\+0x8> ldl R1, #0x01
0+000a <\_start\+0xa> ldh R1, #0x00 Abs\* 0x00000001 <\_start\+0x1>
0+000c <Loop> bra \*10  Abs\* 0x00000016 <test>
0+000e <Loop\+0x2> nop
0+0010 <Loop\+0x4> bne \*\-4  Abs\* 0x0000000c <Loop>
0+0012 <Stop> subh R5, #0x03
0+0014 <Stop\+0x2> bra \*\-20  Abs\* 0x00000000 <\_start>
0+0016 <test> ldl R5, #0x02
0+0018 <test\+0x2> ldh R5, #0x00 Abs\* 0x00000002 <\_start\+0x2>
0+001a <test\+0x4> bra \*4  Abs\* 0x0000001e <test2>
0+001c <test\+0x6> rts
0+001e <test2> ldl R3, #0x17
0+0020 <test2\+0x2> ldh R3, #0x00 Abs\* 0x00000017 <test\+0x1>
0+0022 <test2\+0x4> stw R4, \(R3, #0x0\)
0+0024 <test2\+0x6> ldl R4, #0xec
			24: R_XGATE_IMM8_LO	.text
0+0026 <test2\+0x8> ldh R4, #0xff Abs\* 0x0000ffec <block_end\+0xfbec>
			26: R_XGATE_IMM8_HI	.text
0+0028 <test2\+0xa> bra \*\-22  Abs\* 0x00000012 <Stop>
0+002a <L1> ldl R1, #0x1e
			2a: R_XGATE_IMM8_LO	.text
0+002c <L1\+0x2> ldh R1, #0x00 Abs\* 0x0000001e <test2>
			2c: R_XGATE_IMM8_HI	.text
0+002e <L1\+0x4> ldl R2, #0x1e
			2e: R_XGATE_IMM8_LO	.text
0+0030 <L1\+0x6> ldh R2, #0x00 Abs\* 0x0000001e <test2>
			30: R_XGATE_IMM8_HI	.text
0+0032 <L1\+0x8> rts
