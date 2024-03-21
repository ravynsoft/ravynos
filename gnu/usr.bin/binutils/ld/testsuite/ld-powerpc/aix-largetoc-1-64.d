#source: aix-largetoc-1.s
#as: -a64
#ld: -b64 -shared -bE:aix-largetoc-1.ex
#objdump: -dr
#target: [is_xcoff_format]

.*

Disassembly of section \.text:

.* <\.foo>:
.*:	3d 22 00 00 	addis   r9,r2,0
.*: R_TOCU	a-.*
.*:	39 29 00 00 	addi    r9,r9,0
.*: R_TOCL	a-.*
.*:	3d 22 00 00 	addis   r9,r2,0
.*: R_TOCU	b-.*
.*:	39 29 00 08 	addi    r9,r9,8
.*: R_TOCL	b-.*
#...
