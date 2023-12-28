#as: --underscore --em=criself
#objdump: -dr

.*:     file format .*-cris

Disassembly of section \.text:

0+ <x>:
       0:	f87f .*
#...
    1ffc:	0800 .*
    1ffe:	ffed fc5f           	ba 7ffe <after>
    2002:	0f05                	nop 
    2004:	3f0d fa1f 0100      	jump 11ffa <after\+0x9ffc>
			2006: R_CRIS_32	.text\+0x11ffa
    200a:	3f0d f81f 0100      	jump 11ff8 <after\+0x9ffa>
			200c: R_CRIS_32	.text\+0x11ff8
#...
    7ff8:	3f0d feff 0000      	jump fffe <after\+0x8000>
			7ffa: R_CRIS_32	.text\+0xfffe

00007ffe <after>:
	\.\.\.
    fffe:	0f05                	nop 
#...
   11ffa:	0f05                	nop 
   11ffc:	6fae 0000 0000      	move.d 0 <x>,r10
			11ffe: R_CRIS_32	esymbol
	\.\.\.
