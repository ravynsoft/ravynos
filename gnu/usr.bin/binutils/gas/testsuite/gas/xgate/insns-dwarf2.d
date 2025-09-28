#objdump: -S -WN
#as: -gdwarf2
#name: Dwarf2 test on insns.s
#source: insns.s

# Test handling of basic instructions.

.*: +file format elf32\-xgate

Disassembly of section .text:

0+0000 <_start>:
	
	.globl _start
	.sect .text

_start:
	ldw R2, #block\+1024
   0:	f2 00       	ldl R2, #0x00
   2:	fa 04       	ldh R2, #0x04 Abs\* 0x400 <block_end>
	ldw R3, #block
   4:	f3 00       	ldl R3, #0x00
   6:	fb 00       	ldh R3, #0x00 Abs\* 0x0 <_start>
	ldw R1, #1
   8:	f1 01       	ldl R1, #0x01
   a:	f9 00       	ldh R1, #0x00 Abs\* 0x1 <_start\+0x1>

0+000c <Loop>:
Loop:	
	bra test
   c:	3c 04       	bra \*10  Abs\* 0x16 <test>
	nop
   e:	01 00       	nop
	bne Loop
  10:	25 fd       	bne \*-4  Abs\* 0xc <Loop>

0+0012 <Stop>:
  12:	cd 03       	subh R5, #0x03
Stop:
	
	.byte 0xcd
	.byte 3	
	bra _start
  14:	3f f5       	bra \*-20  Abs\* 0x0 <_start>

0+0016 <test>:

test:
	ldw R5, #2
  16:	f5 02       	ldl R5, #0x02
  18:	fd 00       	ldh R5, #0x00 Abs\* 0x2 <_start\+0x2>
	bra test2
  1a:	3c 01       	bra \*4  Abs\* 0x1e <test2>
	rts
  1c:	02 00       	rts

0+001e <test2>:

value = 23
		
	.globl test2
test2:
	ldw R3, #value
  1e:	f3 17       	ldl R3, #0x17
  20:	fb 00       	ldh R3, #0x00 Abs\* 0x17 <test\+0x1>
	stw R4, \(R3, #0\)
  22:	5c 60       	stw R4, \(R3, #0x0\)
	ldw R4, #24\+_start\-44
  24:	f4 ec       	ldl R4, #0xec
  26:	fc ff       	ldh R4, #0xff Abs\* 0xffec <block_end\+0xfbec>
	bra Stop
  28:	3f f4       	bra \*-22  Abs\* 0x12 <Stop>

0+002a <L1>:
L1:	
	ldw R1, test2
  2a:	f1 1e       	ldl R1, #0x1e
  2c:	f9 00       	ldh R1, #0x00 Abs\* 0x1e <test2>
	ldw R2, test2
  2e:	f2 1e       	ldl R2, #0x1e
  30:	fa 00       	ldh R2, #0x00 Abs\* 0x1e <test2>
	rts
  32:	02 00       	rts

