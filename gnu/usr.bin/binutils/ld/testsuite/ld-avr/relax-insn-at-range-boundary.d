#name: AVR relaxation, jump to symbol at ends of pc-relative range boundary
#as: -mlink-relax -mavr51
#ld: --relax -mavr51
#source: relax-insn-at-range-boundary.s
#objdump: -d
#target: avr-*-*

#...
00000000.*
	...
     ffc:	00 00       	nop
     ffe:	00 c8       	rjmp	.-4096   	; 0x0 .*
    1000:	ff c7       	rjmp	.+4094   	; 0x2000 <forward_target>
	...

00002000 <forward_target>:
#...
