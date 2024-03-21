#name: AVR (avrtiny) check disassembly if symbolic name present
#as: -mavrtiny
#ld: -mavrtiny
#objdump: -d
#source: lds-tiny.s
#target: avr-*-*

.*:     file format elf32-avr


Disassembly of section .text:

00000000 <main>:
   0:	20 a1       	lds	r18, 0x40	; 0x800040 <myvar1>
   2:	42 a1       	lds	r20, 0x42	; 0x800042 <myvar2\+0x1>
   4:	53 a1       	lds	r21, 0x43	; 0x800043 <_end>
   6:	08 95       	ret

