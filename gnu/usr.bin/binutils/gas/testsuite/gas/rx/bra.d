#source: ./bra.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	0b                            	bra\.s	0x3
   1:	0a                            	bra\.s	0xb
   2:	08                            	bra\.s	0xa
			2: R_RX_DIR3U_PCREL	foo
   3:	2e 80                         	bra\.b	0xffffff83
   5:	2e 7f                         	bra\.b	0x84
   7:	2e 00                         	bra\.b	0x7
			8: R_RX_DIR8S_PCREL	foo
   9:	38 00 80                      	bra\.w	0xffff8009
   c:	38 ff 7f                      	bra\.w	0x800b
   f:	38 00 00                      	bra\.w	0xf
			10: R_RX_DIR16S_PCREL	foo
  12:	04 00 00 80                   	bra\.a	0xff800012
  16:	04 ff ff 7f                   	bra\.a	0x800015
  1a:	04 00 00 00                   	bra\.a	0x1a
			1b: R_RX_DIR24S_PCREL	foo
  1e:	04 00 00 80                   	bra\.a	0xff80001e
  22:	04 ff ff 7f                   	bra\.a	0x800021
  26:	04 00 00 00                   	bra\.a	0x26
			27: R_RX_DIR24S_PCREL	foo
  2a:	7f 40                         	bra\.l	r0
  2c:	7f 4f                         	bra\.l	r15
