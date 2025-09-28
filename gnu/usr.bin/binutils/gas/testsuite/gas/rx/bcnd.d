#source: ./bcnd.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	13                            	beq\.s	0x3
   1:	12                            	beq\.s	0xb
   2:	10                            	beq\.s	0xa
			2: R_RX_DIR3U_PCREL	foo
   3:	1b                            	bne\.s	0x6
   4:	1a                            	bne\.s	0xe
   5:	18                            	bne\.s	0xd
			5: R_RX_DIR3U_PCREL	foo
   6:	22 80                         	bc\.b	0xffffff86
   8:	22 7f                         	bc\.b	0x87
   a:	22 00                         	bc\.b	0xa
			b: R_RX_DIR8S_PCREL	foo
   c:	22 80                         	bc\.b	0xffffff8c
   e:	22 7f                         	bc\.b	0x8d
  10:	22 00                         	bc\.b	0x10
			11: R_RX_DIR8S_PCREL	foo
  12:	20 80                         	beq\.b	0xffffff92
  14:	20 7f                         	beq\.b	0x93
  16:	20 00                         	beq\.b	0x16
			17: R_RX_DIR8S_PCREL	foo
  18:	20 80                         	beq\.b	0xffffff98
  1a:	20 7f                         	beq\.b	0x99
  1c:	20 00                         	beq\.b	0x1c
			1d: R_RX_DIR8S_PCREL	foo
  1e:	24 80                         	bgtu\.b	0xffffff9e
  20:	24 7f                         	bgtu\.b	0x9f
  22:	24 00                         	bgtu\.b	0x22
			23: R_RX_DIR8S_PCREL	foo
  24:	26 80                         	bpz\.b	0xffffffa4
  26:	26 7f                         	bpz\.b	0xa5
  28:	26 00                         	bpz\.b	0x28
			29: R_RX_DIR8S_PCREL	foo
  2a:	28 80                         	bge\.b	0xffffffaa
  2c:	28 7f                         	bge\.b	0xab
  2e:	28 00                         	bge\.b	0x2e
			2f: R_RX_DIR8S_PCREL	foo
  30:	2a 80                         	bgt\.b	0xffffffb0
  32:	2a 7f                         	bgt\.b	0xb1
  34:	2a 00                         	bgt\.b	0x34
			35: R_RX_DIR8S_PCREL	foo
  36:	2c 80                         	bo\.b	0xffffffb6
  38:	2c 7f                         	bo\.b	0xb7
  3a:	2c 00                         	bo\.b	0x3a
			3b: R_RX_DIR8S_PCREL	foo
  3c:	23 80                         	bnc\.b	0xffffffbc
  3e:	23 7f                         	bnc\.b	0xbd
  40:	23 00                         	bnc\.b	0x40
			41: R_RX_DIR8S_PCREL	foo
  42:	23 80                         	bnc\.b	0xffffffc2
  44:	23 7f                         	bnc\.b	0xc3
  46:	23 00                         	bnc\.b	0x46
			47: R_RX_DIR8S_PCREL	foo
  48:	21 80                         	bne\.b	0xffffffc8
  4a:	21 7f                         	bne\.b	0xc9
  4c:	21 00                         	bne\.b	0x4c
			4d: R_RX_DIR8S_PCREL	foo
  4e:	21 80                         	bne\.b	0xffffffce
  50:	21 7f                         	bne\.b	0xcf
  52:	21 00                         	bne\.b	0x52
			53: R_RX_DIR8S_PCREL	foo
  54:	25 80                         	bleu\.b	0xffffffd4
  56:	25 7f                         	bleu\.b	0xd5
  58:	25 00                         	bleu\.b	0x58
			59: R_RX_DIR8S_PCREL	foo
  5a:	27 80                         	bn\.b	0xffffffda
  5c:	27 7f                         	bn\.b	0xdb
  5e:	27 00                         	bn\.b	0x5e
			5f: R_RX_DIR8S_PCREL	foo
  60:	29 80                         	blt\.b	0xffffffe0
  62:	29 7f                         	blt\.b	0xe1
  64:	29 00                         	blt\.b	0x64
			65: R_RX_DIR8S_PCREL	foo
  66:	2b 80                         	ble\.b	0xffffffe6
  68:	2b 7f                         	ble\.b	0xe7
  6a:	2b 00                         	ble\.b	0x6a
			6b: R_RX_DIR8S_PCREL	foo
  6c:	2d 80                         	bno\.b	0xffffffec
  6e:	2d 7f                         	bno\.b	0xed
  70:	2d 00                         	bno\.b	0x70
			71: R_RX_DIR8S_PCREL	foo
  72:	3a 00 80                      	beq\.w	0xffff8072
  75:	3a ff 7f                      	beq\.w	0x8074
  78:	3a 00 00                      	beq\.w	0x78
			79: R_RX_DIR16S_PCREL	foo
  7b:	3b 00 80                      	bne\.w	0xffff807b
  7e:	3b ff 7f                      	bne\.w	0x807d
  81:	3b 00 00                      	bne\.w	0x81
			82: R_RX_DIR16S_PCREL	foo
  84:	3a 00 80                      	beq\.w	0xffff8084
  87:	3a ff 7f                      	beq\.w	0x8086
  8a:	3a 00 00                      	beq\.w	0x8a
			8b: R_RX_DIR16S_PCREL	foo
  8d:	3b 00 80                      	bne\.w	0xffff808d
  90:	3b ff 7f                      	bne\.w	0x808f
  93:	3b 00 00                      	bne\.w	0x93
			94: R_RX_DIR16S_PCREL	foo
