#as: -Av9v
#objdump: -dr
#name: sparc CRYPTO

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	81 b0 28 00 	md5 
   4:	81 b0 28 20 	sha1 
   8:	81 b0 28 40 	sha256 
   c:	81 b0 28 60 	sha512 
  10:	8d b0 a8 e4 	crc32c  %f2, %f4, %f6
  14:	91 b1 26 06 	aes_kexpand0  %f4, %f6, %f8
  18:	94 c9 8f 08 	aes_kexpand1  %f6, %f8, 0x7, %f10
  1c:	94 c9 8d 08 	aes_kexpand1  %f6, %f8, 0x6, %f10
  20:	99 b2 26 2a 	aes_kexpand2  %f8, %f10, %f12
  24:	a0 ca 9c 0c 	aes_eround01  %f10, %f12, %f14, %f16
  28:	a4 cb 20 2e 	aes_eround23  %f12, %f14, %f16, %f18
  2c:	a8 cb a4 50 	aes_dround01  %f14, %f16, %f18, %f20
  30:	ac cc 28 72 	aes_dround23  %f16, %f18, %f20, %f22
  34:	b0 cc ac 94 	aes_eround01_l  %f18, %f20, %f22, %f24
  38:	b4 cd 30 b6 	aes_eround23_l  %f20, %f22, %f24, %f26
  3c:	b8 cd b4 d8 	aes_dround01_l  %f22, %f24, %f26, %f28
  40:	bc ce 38 fa 	aes_dround23_l  %f24, %f26, %f28, %f30
  44:	87 b0 66 80 	des_ip  %f32, %f34
  48:	8b b0 e6 a0 	des_iip  %f34, %f36
  4c:	8f b1 66 c7 	des_kexpand  %f36, 7, %f38
  50:	9a c9 d7 29 	des_round  %f38, %f40, %f42, %f44
  54:	9f b2 e7 0d 	kasumi_fi_fi  %f42, %f44, %f46
  58:	a6 cb 63 4f 	kasumi_fl_xor  %f44, %f46, %f48, %f50
  5c:	aa cb e7 71 	kasumi_fi_xor  %f46, %f48, %f50, %f52
  60:	af b4 e7 95 	camellia_fl  %f50, %f52, %f54
  64:	b3 b5 67 b7 	camellia_fli  %f52, %f54, %f56
  68:	ba cd f7 99 	camellia_f  %f54, %f56, %f58, %f60
  6c:	81 b0 29 00 	mpmul  0
  70:	81 b0 29 01 	mpmul  1
  74:	81 b0 29 02 	mpmul  2
  78:	81 b0 29 03 	mpmul  3
  7c:	81 b0 29 04 	mpmul  4
  80:	81 b0 29 05 	mpmul  5
  84:	81 b0 29 06 	mpmul  6
  88:	81 b0 29 07 	mpmul  7
  8c:	81 b0 29 08 	mpmul  8
  90:	81 b0 29 09 	mpmul  9
  94:	81 b0 29 0a 	mpmul  0xa
  98:	81 b0 29 0b 	mpmul  0xb
  9c:	81 b0 29 0c 	mpmul  0xc
  a0:	81 b0 29 0d 	mpmul  0xd
  a4:	81 b0 29 0e 	mpmul  0xe
  a8:	81 b0 29 0f 	mpmul  0xf
  ac:	81 b0 29 10 	mpmul  0x10
  b0:	81 b0 29 11 	mpmul  0x11
  b4:	81 b0 29 12 	mpmul  0x12
  b8:	81 b0 29 13 	mpmul  0x13
  bc:	81 b0 29 14 	mpmul  0x14
  c0:	81 b0 29 15 	mpmul  0x15
  c4:	81 b0 29 16 	mpmul  0x16
  c8:	81 b0 29 17 	mpmul  0x17
  cc:	81 b0 29 18 	mpmul  0x18
  d0:	81 b0 29 19 	mpmul  0x19
  d4:	81 b0 29 1a 	mpmul  0x1a
  d8:	81 b0 29 1b 	mpmul  0x1b
  dc:	81 b0 29 1c 	mpmul  0x1c
  e0:	81 b0 29 1d 	mpmul  0x1d
  e4:	81 b0 29 1e 	mpmul  0x1e
  e8:	81 b0 29 1f 	mpmul  0x1f
  ec:	81 b0 29 20 	montmul  0
  f0:	81 b0 29 21 	montmul  1
  f4:	81 b0 29 22 	montmul  2
  f8:	81 b0 29 23 	montmul  3
  fc:	81 b0 29 24 	montmul  4
 100:	81 b0 29 25 	montmul  5
 104:	81 b0 29 26 	montmul  6
 108:	81 b0 29 27 	montmul  7
 10c:	81 b0 29 28 	montmul  8
 110:	81 b0 29 29 	montmul  9
 114:	81 b0 29 2a 	montmul  0xa
 118:	81 b0 29 2b 	montmul  0xb
 11c:	81 b0 29 2c 	montmul  0xc
 120:	81 b0 29 2d 	montmul  0xd
 124:	81 b0 29 2e 	montmul  0xe
 128:	81 b0 29 2f 	montmul  0xf
 12c:	81 b0 29 30 	montmul  0x10
 130:	81 b0 29 31 	montmul  0x11
 134:	81 b0 29 32 	montmul  0x12
 138:	81 b0 29 33 	montmul  0x13
 13c:	81 b0 29 34 	montmul  0x14
 140:	81 b0 29 35 	montmul  0x15
 144:	81 b0 29 36 	montmul  0x16
 148:	81 b0 29 37 	montmul  0x17
 14c:	81 b0 29 38 	montmul  0x18
 150:	81 b0 29 39 	montmul  0x19
 154:	81 b0 29 3a 	montmul  0x1a
 158:	81 b0 29 3b 	montmul  0x1b
 15c:	81 b0 29 3c 	montmul  0x1c
 160:	81 b0 29 3d 	montmul  0x1d
 164:	81 b0 29 3e 	montmul  0x1e
 168:	81 b0 29 3f 	montmul  0x1f
 16c:	81 b0 29 40 	montsqr  0
 170:	81 b0 29 41 	montsqr  1
 174:	81 b0 29 42 	montsqr  2
 178:	81 b0 29 43 	montsqr  3
 17c:	81 b0 29 44 	montsqr  4
 180:	81 b0 29 45 	montsqr  5
 184:	81 b0 29 46 	montsqr  6
 188:	81 b0 29 47 	montsqr  7
 18c:	81 b0 29 48 	montsqr  8
 190:	81 b0 29 49 	montsqr  9
 194:	81 b0 29 4a 	montsqr  0xa
 198:	81 b0 29 4b 	montsqr  0xb
 19c:	81 b0 29 4c 	montsqr  0xc
 1a0:	81 b0 29 4d 	montsqr  0xd
 1a4:	81 b0 29 4e 	montsqr  0xe
 1a8:	81 b0 29 4f 	montsqr  0xf
 1ac:	81 b0 29 50 	montsqr  0x10
 1b0:	81 b0 29 51 	montsqr  0x11
 1b4:	81 b0 29 52 	montsqr  0x12
 1b8:	81 b0 29 53 	montsqr  0x13
 1bc:	81 b0 29 54 	montsqr  0x14
 1c0:	81 b0 29 55 	montsqr  0x15
 1c4:	81 b0 29 56 	montsqr  0x16
 1c8:	81 b0 29 57 	montsqr  0x17
 1cc:	81 b0 29 58 	montsqr  0x18
 1d0:	81 b0 29 59 	montsqr  0x19
 1d4:	81 b0 29 5a 	montsqr  0x1a
 1d8:	81 b0 29 5b 	montsqr  0x1b
 1dc:	81 b0 29 5c 	montsqr  0x1c
 1e0:	81 b0 29 5d 	montsqr  0x1d
 1e4:	81 b0 29 5e 	montsqr  0x1e
 1e8:	81 b0 29 5f 	montsqr  0x1f
