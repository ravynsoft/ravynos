#source: ./bsr.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	39 00 80                      	bsr\.w	0xffff8000
   3:	39 ff 7f                      	bsr\.w	0x8002
   6:	39 00 00                      	bsr\.w	0x6
			7: R_RX_DIR16S_PCREL	foo
   9:	05 00 00 80                   	bsr\.a	0xff800009
   d:	05 ff ff 7f                   	bsr\.a	0x80000c
  11:	05 00 00 00                   	bsr\.a	0x11
			12: R_RX_DIR24S_PCREL	foo
  15:	7f 50                         	bsr\.l	r0
  17:	7f 5f                         	bsr\.l	r15
