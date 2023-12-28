#objdump: -S
#as: -m68hc11 -gdwarf2
#name: Dwarf2 test on insns.s
#source: insns.s

# Test handling of basic instructions.

.*: +file format elf32\-m68hc11

Disassembly of section .text:

00000000 <_start>:
#...
	.globl _start
	.sect .text

_start:
	lds #stack\+1024
   0:	8e 04 00    	lds	#0x400 <stack_end>
	ldx #1
   3:	ce 00 01    	ldx	#0x1 <_start\+0x1>

0+06 <Loop>:
Loop:	
	jsr test
   6:	bd 00 00    	jsr	0x0 <_start>
	dex
   9:	09          	dex
	bne Loop
   a:	26 fa       	bne	0x6 <Loop>

0000000c <Stop>:
   c:	cd 03       	.byte	0xcd, 0x03
Stop:
	
	.byte 0xcd
	.byte 3	
	bra _start
   e:	20 f0       	bra	0x0 <_start>

00000010 <test>:

test:
	ldd #2
  10:	cc 00 02    	ldd	#0x2 <_start\+0x2>
	jsr test2
  13:	bd 00 00    	jsr	0x0 <_start>
	rts
  16:	39          	rts

00000017 <test2>:

D_low = 50
value = 23
		
	.globl test2
test2:
	ldx value,y
  17:	cd ee 17    	ldx	0x17,y
	std value,x
  1a:	ed 17       	std	0x17,x
	ldd ,x
  1c:	ec 00       	ldd	0x0,x
	sty ,y
  1e:	18 ef 00    	sty	0x0,y
	stx ,y
  21:	cd ef 00    	stx	0x0,y
	brclr 6,x,#4,test2
  24:	1f 06 04 ef 	brclr	0x6,x, #0x04, 0x17 <test2>
	brclr 12,x #8 test2
  28:	1f 0c 08 eb 	brclr	0xc,x, #0x08, 0x17 <test2>
	ldd \*ZD1
  2c:	dc 00       	ldd	\*0x0 <_start>
	ldx \*ZD1\+2
  2e:	de 02       	ldx	\*0x2 <_start\+0x2>
	clr \*ZD2
  30:	7f 00 00    	clr	0x0 <_start>
	clr \*ZD2\+1
  33:	7f 00 01    	clr	0x1 <_start\+0x1>
	bne .-4
  36:	26 fc       	bne	0x34 <test2\+0x1d>
	beq .\+2
  38:	27 02       	beq	0x3c <test2\+0x25>
	bclr \*ZD1\+1, #32
  3a:	15 01 20    	bclr	\*0x1 <_start\+0x1>, #0x20
	brclr \*ZD2\+2, #40, test2
  3d:	13 02 28 d6 	brclr	\*0x2 <_start\+0x2>, #0x28, 0x17 <test2>
	ldy #24\+_start-44
  41:	18 ce ff ec 	ldy	#0xffec <stack_end\+0xfbec>
	ldd B_low,y
  45:	18 ec 0c    	ldd	0xc,y
	addd A_low,y
  48:	18 e3 2c    	addd	0x2c,y
	addd D_low,y
  4b:	18 e3 32    	addd	0x32,y
	subd A_low
  4e:	b3 00 2c    	subd	0x2c <test2\+0x15>
	subd #A_low
  51:	83 00 2c    	subd	#0x2c <test2\+0x15>
	jmp Stop
  54:	7e 00 00    	jmp	0x0 <_start>

00000057 <L1>:
L1:	
	anda #%lo\(test2\)
  57:	84 17       	anda	#0x17
	andb #%hi\(test2\)
  59:	c4 00       	andb	#0x0
	ldab #%page\(test2\)	; Check that the relocs are against symbol
  5b:	c6 00       	ldab	#0x0
	ldy  #%addr\(test2\)	; otherwise linker relaxation fails
  5d:	18 ce 00 00 	ldy	#0x0 <_start>
	rts
  61:	39          	rts
