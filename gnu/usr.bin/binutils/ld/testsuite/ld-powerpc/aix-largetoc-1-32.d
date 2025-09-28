#source: aix-largetoc-1.s
#as: -a32
#ld: -b32 -shared -bE:aix-largetoc-1.ex
#objdump: -dr
#target: [is_xcoff_format]

.*

Disassembly of section \.text:

.* <\.foo>:
.*:	3d 22 00 00 	cau     r9,r2,0
.*: R_TOCU	a-.*
.*:	39 29 00 00 	cal     r9,0\(r9\)
.*: R_TOCL	a-.*
.*:	3d 22 00 00 	cau     r9,r2,0
.*: R_TOCU	b-.*
.*:	39 29 00 04 	cal     r9,4\(r9\)
.*: R_TOCL	b-.*
#...
