#as: --underscore --em=criself --march=v32
#source: rd-bkw4.s
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
      10:	c64a .*
      12:	c44a .*
      14:	c24a .*
      16:	c04a .*
      18:	b64a .*
      1a:	b44a .*
      1c:	ba4a .*
      1e:	b84a .*
      20:	ae4a .*
      22:	ac4a .*
      24:	b24a .*
      26:	b04a .*
      28:	a64a .*
      2a:	a44a .*
      2c:	aa4a .*
      2e:	a84a .*
      30:	9e4a .*
      32:	9c4a .*
      34:	a24a .*
      36:	984a .*
      38:	9e4a .*
	\.\.\.
    4ac6:	0000                	bcc \.
    4ac8:	0ee0                	ba 4ad6 <x\+0x4ad6>
    4aca:	b005                	nop 
    4acc:	b005                	nop 
    4ace:	ffed 4635           	ba 8014 <x\+0x8014>
    4ad2:	b005                	nop 
    4ad4:	b005                	nop 
    4ad6:	6f9e 0000 0000      	move.d 0 <x>,r9
			4ad8: R_CRIS_32	x336
    4adc:	bfbd 0000 0000      	jsr 0 <x>
			4ade: R_CRIS_32	y
	\.\.\.
    8012:	0000                	bcc \.
    8014:	b005                	nop 
	\.\.\.
