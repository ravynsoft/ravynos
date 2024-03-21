#as: --march=v32 --pic --underscore --em=criself --defsym extra=-819
#objdump: -dr
#source: rd-bkw5.s

.*:     file format .*-cris

Disassembly of section \.text:

0+ <x>:
       0:	f67f .*
#...
    1996:	0800 .*
    1998:	ffed 6666           	ba 7ffe <after>
    199c:	b005                	nop 
    199e:	bf0e f6ff 0000      	ba 11994 <after\+0x9996>
    19a4:	b005                	nop 
#...
    7ff6:	bf0e 0880 0000      	ba fffe <after\+0x8000>
    7ffc:	b005                	nop 

00007ffe <after>:
	...
    fffe:	b005                	nop 
#...
   11994:	b005                	nop 
   11996:	6fae 0000 0000      	move.d 0 <x>,r10
			11998: R_CRIS_32	esymbol

