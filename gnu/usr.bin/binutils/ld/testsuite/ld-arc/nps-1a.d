#source: nps-1.s
#as: -mcpu=arc700 -mnps400
#ld: -defsym=foo=0x57f03000 -T sda-relocs.ld
#objdump: -d

.*: +file format .*arc.*

Disassembly of section .text:

[0-9a-f]+ <.*>:
 *[0-9a-f]+:	5948 3000           	xldb	r10,\[0x57f03000\]
 *[0-9a-f]+:	5949 3000           	xldw	r10,\[0x57f03000\]
 *[0-9a-f]+:	594a 3000           	xld	r10,\[0x57f03000\]
 *[0-9a-f]+:	594c 3000           	xstb	r10,\[0x57f03000\]
 *[0-9a-f]+:	594d 3000           	xstw	r10,\[0x57f03000\]
 *[0-9a-f]+:	594e 3000           	xst	r10,\[0x57f03000\]
