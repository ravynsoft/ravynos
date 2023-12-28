#name: aarch64-limit-b
#source: limit-b.s
#as:
#ld: -Ttext 0x1000 --section-start .foo=0x8000ffc
#objdump: -dr
#...

Disassembly of section .text:

0+1000 <_start>:
    1000:	15ffffff 	b	8000ffc <bar>
    1004:	d65f03c0 	ret

Disassembly of section .foo:

0+8000ffc <bar>:
 8000ffc:	d65f03c0 	ret
