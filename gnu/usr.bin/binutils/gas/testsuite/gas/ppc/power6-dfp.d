#as: -a32 -mpower6
#objdump: -dr -Mpower6
#name: POWER6 all DFP instructions

.*

Disassembly of section \.text:

0+00 <start>:
   0:	(ee 11 90 04|04 90 11 ee) 	dadd    f16,f17,f18
   4:	(ee 11 90 05|05 90 11 ee) 	dadd\.   f16,f17,f18
   8:	(fe 96 c0 04|04 c0 96 fe) 	daddq   f20,f22,f24
   c:	(fe 96 c0 05|05 c0 96 fe) 	daddq\.  f20,f22,f24
  10:	(ee 11 94 04|04 94 11 ee) 	dsub    f16,f17,f18
  14:	(ee 11 94 05|05 94 11 ee) 	dsub\.   f16,f17,f18
  18:	(fe 96 c4 04|04 c4 96 fe) 	dsubq   f20,f22,f24
  1c:	(fe 96 c4 05|05 c4 96 fe) 	dsubq\.  f20,f22,f24
  20:	(ee 11 90 44|44 90 11 ee) 	dmul    f16,f17,f18
  24:	(ee 11 90 45|45 90 11 ee) 	dmul\.   f16,f17,f18
  28:	(fe 96 c0 44|44 c0 96 fe) 	dmulq   f20,f22,f24
  2c:	(fe 96 c0 45|45 c0 96 fe) 	dmulq\.  f20,f22,f24
  30:	(ee 11 94 44|44 94 11 ee) 	ddiv    f16,f17,f18
  34:	(ee 11 94 45|45 94 11 ee) 	ddiv\.   f16,f17,f18
  38:	(fe 96 c4 44|44 c4 96 fe) 	ddivq   f20,f22,f24
  3c:	(fe 96 c4 45|45 c4 96 fe) 	ddivq\.  f20,f22,f24
  40:	(ec 83 29 04|04 29 83 ec) 	dcmpo   cr1,f3,f5
  44:	(fd 86 21 04|04 21 86 fd) 	dcmpoq  cr3,f6,f4
  48:	(ed 03 2d 04|04 2d 03 ed) 	dcmpu   cr2,f3,f5
  4c:	(fd 06 25 04|04 25 06 fd) 	dcmpuq  cr2,f6,f4
  50:	(ec 01 fd 84|84 fd 01 ec) 	dtstdc  cr0,f1,63
  54:	(fc 02 01 84|84 01 02 fc) 	dtstdcq cr0,f2,0
  58:	(ec 03 81 c4|c4 81 03 ec) 	dtstdg  cr0,f3,32
  5c:	(fc 04 05 c4|c4 05 04 fc) 	dtstdgq cr0,f4,1
  60:	(ef 81 29 44|44 29 81 ef) 	dtstex  cr7,f1,f5
  64:	(ff 02 31 44|44 31 02 ff) 	dtstexq cr6,f2,f6
  68:	(ee 83 3d 44|44 3d 83 ee) 	dtstsf  cr5,f3,f7
  6c:	(fe 04 45 44|44 45 04 fe) 	dtstsfq cr4,f4,f8
  70:	(ec 10 22 86|86 22 10 ec) 	dquai   -16,f0,f4,1
  74:	(ec 10 22 87|87 22 10 ec) 	dquai\.  -16,f0,f4,1
  78:	(fc 4f 36 86|86 36 4f fc) 	dquaiq  15,f2,f6,3
  7c:	(fc 4f 36 87|87 36 4f fc) 	dquaiq\. 15,f2,f6,3
  80:	(ec 22 28 06|06 28 22 ec) 	dqua    f1,f2,f5,0
  84:	(ec 64 32 07|07 32 64 ec) 	dqua\.   f3,f4,f6,1
  88:	(fc 46 24 06|06 24 46 fc) 	dquaq   f2,f6,f4,2
  8c:	(fc 88 16 07|07 16 88 fc) 	dquaq\.  f4,f8,f2,3
  90:	(ec 22 1a 46|46 1a 22 ec) 	drrnd   f1,f2,f3,1
  94:	(ec 44 32 47|47 32 44 ec) 	drrnd\.  f2,f4,f6,1
  98:	(fc 02 24 46|46 24 02 fc) 	drrndq  f0,f2,f4,2
  9c:	(fc 86 44 47|47 44 86 fc) 	drrndq\. f4,f6,f8,2
  a0:	(ec 20 10 c6|c6 10 20 ec) 	drintx  0,f1,f2,0
  a4:	(ec 41 0a c7|c7 0a 41 ec) 	drintx\. 1,f2,f1,1
  a8:	(fc 40 24 c6|c6 24 40 fc) 	drintxq 0,f2,f4,2
  ac:	(fc 81 36 c7|c7 36 81 fc) 	drintxq\. 1,f4,f6,3
  b0:	(ec 21 19 c6|c6 19 21 ec) 	drintn  1,f1,f3,0
  b4:	(ec 80 13 c7|c7 13 80 ec) 	drintn\. 0,f4,f2,1
  b8:	(fc 01 15 c6|c6 15 01 fc) 	drintnq 1,f0,f2,2
  bc:	(fc 80 17 c7|c7 17 80 fc) 	drintnq\. 0,f4,f2,3
  c0:	(ec 40 22 04|04 22 40 ec) 	dctdp   f2,f4
  c4:	(ec 40 22 05|05 22 40 ec) 	dctdp\.  f2,f4
  c8:	(fc 40 22 04|04 22 40 fc) 	dctqpq  f2,f4
  cc:	(fc 40 22 05|05 22 40 fc) 	dctqpq\. f2,f4
  d0:	(ec 40 26 04|04 26 40 ec) 	drsp    f2,f4
  d4:	(ec 40 26 05|05 26 40 ec) 	drsp\.   f2,f4
  d8:	(fc 40 26 04|04 26 40 fc) 	drdpq   f2,f4
  dc:	(fc 40 26 05|05 26 40 fc) 	drdpq\.  f2,f4
  e0:	(fc 40 26 44|44 26 40 fc) 	dcffixq f2,f4
  e4:	(fc 40 26 45|45 26 40 fc) 	dcffixq\. f2,f4
  e8:	(ec 40 22 44|44 22 40 ec) 	dctfix  f2,f4
  ec:	(ec 40 22 45|45 22 40 ec) 	dctfix\. f2,f4
  f0:	(fc 40 22 44|44 22 40 fc) 	dctfixq f2,f4
  f4:	(fc 40 22 45|45 22 40 fc) 	dctfixq\. f2,f4
  f8:	(ec 20 12 84|84 12 20 ec) 	ddedpd  0,f1,f2
  fc:	(ec 08 0a 85|85 0a 08 ec) 	ddedpd\. 1,f0,f1
 100:	(fc 48 22 84|84 22 48 fc) 	ddedpdq 1,f2,f4
 104:	(fc 80 12 85|85 12 80 fc) 	ddedpdq\. 0,f4,f2
 108:	(ec 20 16 84|84 16 20 ec) 	denbcd  0,f1,f2
 10c:	(ec 10 0e 85|85 0e 10 ec) 	denbcd\. 1,f0,f1
 110:	(fc 10 16 84|84 16 10 fc) 	denbcdq 1,f0,f2
 114:	(fc 40 26 85|85 26 40 fc) 	denbcdq\. 0,f2,f4
 118:	(ec 00 0a c4|c4 0a 00 ec) 	dxex    f0,f1
 11c:	(ec 40 1a c5|c5 1a 40 ec) 	dxex\.   f2,f3
 120:	(fc 80 32 c4|c4 32 80 fc) 	dxexq   f4,f6
 124:	(fc 40 02 c5|c5 02 40 fc) 	dxexq\.  f2,f0
 128:	(ec 01 16 c4|c4 16 01 ec) 	diex    f0,f1,f2
 12c:	(ec 64 2e c5|c5 2e 64 ec) 	diex\.   f3,f4,f5
 130:	(fc 02 26 c4|c4 26 02 fc) 	diexq   f0,f2,f4
 134:	(fc c4 16 c5|c5 16 c4 fc) 	diexq\.  f6,f4,f2
 138:	(ec 22 00 84|84 00 22 ec) 	dscli   f1,f2,0
 13c:	(ec 03 fc 85|85 fc 03 ec) 	dscli\.  f0,f3,63
 140:	(fc 48 04 84|84 04 48 fc) 	dscliq  f2,f8,1
 144:	(fc 86 80 85|85 80 86 fc) 	dscliq\. f4,f6,32
 148:	(ec 20 40 c4|c4 40 20 ec) 	dscri   f1,f0,16
 14c:	(ec 62 3c c5|c5 3c 62 ec) 	dscri\.  f3,f2,15
 150:	(fd 00 a8 c4|c4 a8 00 fd) 	dscriq  f8,f0,42
 154:	(fc 86 54 c5|c5 54 86 fc) 	dscriq\. f4,f6,21
