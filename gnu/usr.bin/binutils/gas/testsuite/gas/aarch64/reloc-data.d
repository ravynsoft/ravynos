#as: -mabi=lp64
#objdump: -dr
#skip: aarch64_be-*-*
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0000000000000000 <.*>:
   0:	d65f03c0 	ret
   4:	ffff005c 	\.word	0xffff005c
   8:	0000005c 	\.word	0x0000005c
   c:	ffffffff 	\.word	0xffffffff
  10:	0000005c 	\.word	0x0000005c
  14:	00000000 	\.word	0x00000000
  18:	ffffffff 	\.word	0xffffffff
  1c:	ffffffff 	\.word	0xffffffff
	\.\.\.
			20: R_AARCH64_ABS64	\.text\+0x12345660
  28:	ffffffff 	\.word	0xffffffff
  2c:	ffffffff 	\.word	0xffffffff
  30:	ffff0000 	\.word	0xffff0000
			30: R_AARCH64_PREL16	global\+0x2c
  34:	00000000 	\.word	0x00000000
			34: R_AARCH64_PREL32	global\+0x30
  38:	ffffffff 	\.word	0xffffffff
  3c:	d503201f 	nop
	\.\.\.
			40: R_AARCH64_PREL64	global\+0x3c
  48:	ffffffff 	\.word	0xffffffff
  4c:	ffffffff 	\.word	0xffffffff
	\.\.\.
			50: R_AARCH64_ABS64	global\+0x12345678
  58:	ffffffff 	\.word	0xffffffff
  5c:	ffffffff 	\.word	0xffffffff
