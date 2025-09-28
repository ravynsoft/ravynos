#as: --underscore --em=criself
#objdump: -dr

.*:     file format .*-cris

Disassembly of section \.text:

0+ <x>:
       0:	ce4a .*
       2:	cc4a .*
       4:	ca4a .*
       6:	c84a .*
       8:	c64a .*
       a:	c44a .*
       c:	c24a .*
       e:	c04a .*
      10:	c44a .*
      12:	c24a .*
      14:	c04a .*
      16:	be4a .*
      18:	b64a .*
      1a:	b44a .*
      1c:	b84a .*
      1e:	b64a .*
      20:	ae4a .*
      22:	ac4a .*
      24:	b04a .*
      26:	ae4a .*
      28:	a64a .*
      2a:	a44a .*
      2c:	a84a .*
      2e:	a64a .*
      30:	9e4a .*
      32:	9c4a .*
      34:	a04a .*
      36:	984a .*
      38:	9c4a .*
	\.\.\.
    4ac6:	0000                	bcc \.\+2
    4ac8:	0ae0                	ba 4ad4 <x\+0x4ad4>
    4aca:	0f05                	nop 
    4acc:	0f05                	nop 
    4ace:	ffed 4035           	ba 8012 <x\+0x8012>
    4ad2:	0f05                	nop 
    4ad4:	6f9e 0000 0000      	move\.d 0 <x>,r9
			4ad6: R_CRIS_32	x336
    4ada:	3fbd 0000 0000      	jsr 0 <x>
			4adc: R_CRIS_32	y
	\.\.\.
    8010:	0000                	bcc \.\+2
    8012:	0f05                	nop 
