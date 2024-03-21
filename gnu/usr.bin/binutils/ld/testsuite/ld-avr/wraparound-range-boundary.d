#name: Wraparound works for jump target at pc-relative range boundary
#as: -m avr51
#ld: --pmem-wrap-around=8k -m avr51
#source: wraparound-range-boundary.s
#objdump: -d
#target: avr-*-*

#...
Disassembly of section .text:

00000000 <__ctors_end>:
       0:	00 c8       	rjmp	.\-4096   	; 0xfffff002 <__eeprom_end\+0xff7ef002>
	...

00001002 <target>:
	...

