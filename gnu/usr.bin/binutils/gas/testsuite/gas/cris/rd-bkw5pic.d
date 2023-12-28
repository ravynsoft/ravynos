#as: --pic --underscore --em=criself --defsym extra=-819
#objdump: -dr
#source: rd-bkw5.s

.*:     file format .*-cris

Disassembly of section \.text:

0+ <x>:
       0:	f67f .*
#...
    1996:	0800 .*
    1998:	ffed 6266           	ba 7ffe <after>
    199c:	0f05                	nop 
    199e:	6ffd f0ff 0000 3f0e 	move \[pc=pc\+fff0 <after\+0x7ff2>\],p0
    19a6:	6ffd e6ff 0000 3f0e 	move \[pc=pc\+ffe6 <after\+0x7fe8>\],p0
#...
    7ff6:	6ffd 0280 0000 3f0e 	move \[pc=pc\+8002 <after\+0x4>\],p0

00007ffe <after>:
	...
    fffe:	0f05                	nop 
#...
   11994:	0f05                	nop 
   11996:	6fae 0000 0000      	move.d 0 <x>,r10
			11998: R_CRIS_32	esymbol

