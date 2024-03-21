#as:
#objdump: -dwMintel
#name: i386 TBM insns (Intel disassembly)
#source: tbm.s

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	8f ea 78 10 1c f2 67 00 00 00[ 	]+bextr  ebx,DWORD PTR \[edx\+esi\*8\],0x67
[ 	]*[a-f0-9]+:	8f ea 78 10 c6 00 00 00 00[ 	]+bextr  eax,esi,0x0
[ 	]*[a-f0-9]+:	8f ea 78 10 f8 ff ff ff 7f[ 	]+bextr  edi,eax,0x7fffffff
[ 	]*[a-f0-9]+:	8f ea 78 10 26 b2 35 00 00[ 	]+bextr  esp,DWORD PTR \[esi\],0x35b2
[ 	]*[a-f0-9]+:	8f ea 78 10 ef 86 9c 00 00[ 	]+bextr  ebp,edi,0x9c86
[ 	]*[a-f0-9]+:	8f ea 78 10 c9 03 00 00 00[ 	]+bextr  ecx,ecx,0x3
[ 	]*[a-f0-9]+:	8f ea 78 10 74 43 fd ee 00 00 00[ 	]+bextr  esi,DWORD PTR \[ebx\+eax\*2-0x3\],0xee
[ 	]*[a-f0-9]+:	8f ea 78 10 23 55 00 00 00[ 	]+bextr  esp,DWORD PTR \[ebx\],0x55
[ 	]*[a-f0-9]+:	8f ea 78 10 12 e8 4e 00 00[ 	]+bextr  edx,DWORD PTR \[edx\],0x4ee8
[ 	]*[a-f0-9]+:	8f ea 78 10 fb 00 00 00 00[ 	]+bextr  edi,ebx,0x0
[ 	]*[a-f0-9]+:	8f ea 78 10 f4 dc 00 00 00[ 	]+bextr  esi,esp,0xdc
[ 	]*[a-f0-9]+:	8f ea 78 10 00 a9 00 00 00[ 	]+bextr  eax,DWORD PTR \[eax\],0xa9
[ 	]*[a-f0-9]+:	8f ea 78 10 ea 89 01 00 00[ 	]+bextr  ebp,edx,0x189
[ 	]*[a-f0-9]+:	8f ea 78 10 0c 41 84 00 00 00[ 	]+bextr  ecx,DWORD PTR \[ecx\+eax\*2\],0x84
[ 	]*[a-f0-9]+:	8f ea 78 10 04 01 fe ca 00 00[ 	]+bextr  eax,DWORD PTR \[ecx\+eax\*1\],0xcafe
[ 	]*[a-f0-9]+:	8f ea 78 10 bc 3e 09 71 00 00 ad de 00 00[ 	]+bextr  edi,DWORD PTR \[esi\+edi\*1\+0x7109\],0xdead
[ 	]*[a-f0-9]+:	8f e9 78 01 09[ 	]+blcfill eax,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	8f e9 40 01 ce[ 	]+blcfill edi,esi
[ 	]*[a-f0-9]+:	8f e9 70 01 c8[ 	]+blcfill ecx,eax
[ 	]*[a-f0-9]+:	8f e9 48 01 cf[ 	]+blcfill esi,edi
[ 	]*[a-f0-9]+:	8f e9 58 01 0e[ 	]+blcfill esp,DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	8f e9 50 01 0b[ 	]+blcfill ebp,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 68 01 8c 03 95 1a 00 00[ 	]+blcfill edx,DWORD PTR \[ebx\+eax\*1\+0x1a95\]
[ 	]*[a-f0-9]+:	8f e9 40 01 0a[ 	]+blcfill edi,DWORD PTR \[edx\]
[ 	]*[a-f0-9]+:	8f e9 40 01 cb[ 	]+blcfill edi,ebx
[ 	]*[a-f0-9]+:	8f e9 78 01 8c 30 ce 00 00 00[ 	]+blcfill eax,DWORD PTR \[eax\+esi\*1\+0xce\]
[ 	]*[a-f0-9]+:	8f e9 78 01 0c 1d 02 35 ff ff[ 	]+blcfill eax,DWORD PTR \[ebx\*1-0xcafe\]
[ 	]*[a-f0-9]+:	8f e9 60 01 0c 05 a1 51 ff ff[ 	]+blcfill ebx,DWORD PTR \[eax\*1-0xae5f\]
[ 	]*[a-f0-9]+:	8f e9 40 01 c9[ 	]+blcfill edi,ecx
[ 	]*[a-f0-9]+:	8f e9 78 01 cc[ 	]+blcfill eax,esp
[ 	]*[a-f0-9]+:	8f e9 40 01 cd[ 	]+blcfill edi,ebp
[ 	]*[a-f0-9]+:	8f e9 78 01 0c 4e[ 	]+blcfill eax,DWORD PTR \[esi\+ecx\*2\]
[ 	]*[a-f0-9]+:	8f e9 70 02 f0[ 	]+blci   ecx,eax
[ 	]*[a-f0-9]+:	8f e9 60 02 f1[ 	]+blci   ebx,ecx
[ 	]*[a-f0-9]+:	8f e9 78 02 34 45 b0 12 00 00[ 	]+blci   eax,DWORD PTR \[eax\*2\+0x12b0\]
[ 	]*[a-f0-9]+:	8f e9 40 02 30[ 	]+blci   edi,DWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	8f e9 48 02 f7[ 	]+blci   esi,edi
[ 	]*[a-f0-9]+:	8f e9 68 02 f4[ 	]+blci   edx,esp
[ 	]*[a-f0-9]+:	8f e9 50 02 f6[ 	]+blci   ebp,esi
[ 	]*[a-f0-9]+:	8f e9 78 02 f2[ 	]+blci   eax,edx
[ 	]*[a-f0-9]+:	8f e9 58 02 b4 83 57 8d ff ff[ 	]+blci   esp,DWORD PTR \[ebx\+eax\*4-0x72a9\]
[ 	]*[a-f0-9]+:	8f e9 60 02 36[ 	]+blci   ebx,DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	8f e9 78 02 34 73[ 	]+blci   eax,DWORD PTR \[ebx\+esi\*2\]
[ 	]*[a-f0-9]+:	8f e9 68 02 33[ 	]+blci   edx,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 78 02 f3[ 	]+blci   eax,ebx
[ 	]*[a-f0-9]+:	8f e9 70 02 b4 93 a2 e0 00 00[ 	]+blci   ecx,DWORD PTR \[ebx\+edx\*4\+0xe0a2\]
[ 	]*[a-f0-9]+:	8f e9 40 02 37[ 	]+blci   edi,DWORD PTR \[edi\]
[ 	]*[a-f0-9]+:	8f e9 78 02 34 45 ff ff ff 3f[ 	]+blci   eax,DWORD PTR \[eax\*2\+0x3fffffff\]
[ 	]*[a-f0-9]+:	8f e9 70 01 ef[ 	]+blcic  ecx,edi
[ 	]*[a-f0-9]+:	8f e9 40 01 e8[ 	]+blcic  edi,eax
[ 	]*[a-f0-9]+:	8f e9 60 01 28[ 	]+blcic  ebx,DWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	8f e9 68 01 e9[ 	]+blcic  edx,ecx
[ 	]*[a-f0-9]+:	8f e9 58 01 ee[ 	]+blcic  esp,esi
[ 	]*[a-f0-9]+:	8f e9 50 01 2c 1d 02 35 ff ff[ 	]+blcic  ebp,DWORD PTR \[ebx\*1-0xcafe\]
[ 	]*[a-f0-9]+:	8f e9 78 01 ed[ 	]+blcic  eax,ebp
[ 	]*[a-f0-9]+:	8f e9 48 01 2e[ 	]+blcic  esi,DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	8f e9 60 01 ec[ 	]+blcic  ebx,esp
[ 	]*[a-f0-9]+:	8f e9 48 01 2c 3f[ 	]+blcic  esi,DWORD PTR \[edi\+edi\*1\]
[ 	]*[a-f0-9]+:	8f e9 50 01 2c 35 01 00 00 c0[ 	]+blcic  ebp,DWORD PTR \[esi\*1-0x3fffffff\]
[ 	]*[a-f0-9]+:	8f e9 40 01 2b[ 	]+blcic  edi,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 78 01 6c c7 08[ 	]+blcic  eax,DWORD PTR \[edi\+eax\*8\+0x8\]
[ 	]*[a-f0-9]+:	8f e9 40 01 a9 d1 4a 57 3a[ 	]+blcic  edi,DWORD PTR \[ecx\+0x3a574ad1\]
[ 	]*[a-f0-9]+:	8f e9 40 01 ec[ 	]+blcic  edi,esp
[ 	]*[a-f0-9]+:	8f e9 40 01 ea[ 	]+blcic  edi,edx
[ 	]*[a-f0-9]+:	8f e9 40 02 48 0c[ 	]+blcmsk edi,DWORD PTR \[eax\+0xc\]
[ 	]*[a-f0-9]+:	8f e9 50 02 0c 16[ 	]+blcmsk ebp,DWORD PTR \[esi\+edx\*1\]
[ 	]*[a-f0-9]+:	8f e9 70 02 8f 00 22 3d e2[ 	]+blcmsk ecx,DWORD PTR \[edi-0x1dc2de00\]
[ 	]*[a-f0-9]+:	8f e9 58 02 c8[ 	]+blcmsk esp,eax
[ 	]*[a-f0-9]+:	8f e9 78 02 0c 57[ 	]+blcmsk eax,DWORD PTR \[edi\+edx\*2\]
[ 	]*[a-f0-9]+:	8f e9 68 02 0b[ 	]+blcmsk edx,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 40 02 0a[ 	]+blcmsk edi,DWORD PTR \[edx\]
[ 	]*[a-f0-9]+:	8f e9 48 02 ce[ 	]+blcmsk esi,esi
[ 	]*[a-f0-9]+:	8f e9 40 02 cc[ 	]+blcmsk edi,esp
[ 	]*[a-f0-9]+:	8f e9 58 02 cf[ 	]+blcmsk esp,edi
[ 	]*[a-f0-9]+:	8f e9 60 02 0c c3[ 	]+blcmsk ebx,DWORD PTR \[ebx\+eax\*8\]
[ 	]*[a-f0-9]+:	8f e9 78 02 0f[ 	]+blcmsk eax,DWORD PTR \[edi\]
[ 	]*[a-f0-9]+:	8f e9 78 02 ca[ 	]+blcmsk eax,edx
[ 	]*[a-f0-9]+:	8f e9 40 02 4c 3b 67[ 	]+blcmsk edi,DWORD PTR \[ebx\+edi\*1\+0x67\]
[ 	]*[a-f0-9]+:	8f e9 40 02 0c 05 a0 d8 12 aa[ 	]+blcmsk edi,DWORD PTR \[eax\*1-0x55ed2760\]
[ 	]*[a-f0-9]+:	8f e9 78 02 0c 05 01 00 00 00[ 	]+blcmsk eax,DWORD PTR \[eax\*1\+0x1\]
[ 	]*[a-f0-9]+:	8f e9 48 01 da[ 	]+blcs   esi,edx
[ 	]*[a-f0-9]+:	8f e9 78 01 1b[ 	]+blcs   eax,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 40 01 d8[ 	]+blcs   edi,eax
[ 	]*[a-f0-9]+:	8f e9 58 01 9c 01 fe ca 00 00[ 	]+blcs   esp,DWORD PTR \[ecx\+eax\*1\+0xcafe\]
[ 	]*[a-f0-9]+:	8f e9 50 01 df[ 	]+blcs   ebp,edi
[ 	]*[a-f0-9]+:	8f e9 70 01 1a[ 	]+blcs   ecx,DWORD PTR \[edx\]
[ 	]*[a-f0-9]+:	8f e9 40 01 1f[ 	]+blcs   edi,DWORD PTR \[edi\]
[ 	]*[a-f0-9]+:	8f e9 60 01 9b 02 35 ff ff[ 	]+blcs   ebx,DWORD PTR \[ebx-0xcafe\]
[ 	]*[a-f0-9]+:	8f e9 70 01 dc[ 	]+blcs   ecx,esp
[ 	]*[a-f0-9]+:	8f e9 68 01 de[ 	]+blcs   edx,esi
[ 	]*[a-f0-9]+:	8f e9 40 01 18[ 	]+blcs   edi,DWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	8f e9 40 01 1c 0d 01 00 00 00[ 	]+blcs   edi,DWORD PTR \[ecx\*1\+0x1\]
[ 	]*[a-f0-9]+:	8f e9 78 01 d9[ 	]+blcs   eax,ecx
[ 	]*[a-f0-9]+:	8f e9 40 01 1c 13[ 	]+blcs   edi,DWORD PTR \[ebx\+edx\*1\]
[ 	]*[a-f0-9]+:	8f e9 78 01 9c 00 53 21 ff ff[ 	]+blcs   eax,DWORD PTR \[eax\+eax\*1-0xdead\]
[ 	]*[a-f0-9]+:	8f e9 40 01 1c 13[ 	]+blcs   edi,DWORD PTR \[ebx\+edx\*1\]
[ 	]*[a-f0-9]+:	8f e9 78 01 d0[ 	]+blsfill eax,eax
[ 	]*[a-f0-9]+:	8f e9 48 01 d1[ 	]+blsfill esi,ecx
[ 	]*[a-f0-9]+:	8f e9 40 01 10[ 	]+blsfill edi,DWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	8f e9 58 01 d3[ 	]+blsfill esp,ebx
[ 	]*[a-f0-9]+:	8f e9 68 01 d2[ 	]+blsfill edx,edx
[ 	]*[a-f0-9]+:	8f e9 70 01 11[ 	]+blsfill ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	8f e9 40 01 d7[ 	]+blsfill edi,edi
[ 	]*[a-f0-9]+:	8f e9 50 01 d5[ 	]+blsfill ebp,ebp
[ 	]*[a-f0-9]+:	8f e9 40 01 17[ 	]+blsfill edi,DWORD PTR \[edi\]
[ 	]*[a-f0-9]+:	8f e9 60 01 13[ 	]+blsfill ebx,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 78 01 16[ 	]+blsfill eax,DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	8f e9 78 01 14 80[ 	]+blsfill eax,DWORD PTR \[eax\+eax\*4\]
[ 	]*[a-f0-9]+:	8f e9 40 01 d6[ 	]+blsfill edi,esi
[ 	]*[a-f0-9]+:	8f e9 40 01 94 18 21 a2 00 00[ 	]+blsfill edi,DWORD PTR \[eax\+ebx\*1\+0xa221\]
[ 	]*[a-f0-9]+:	8f e9 78 01 14 00[ 	]+blsfill eax,DWORD PTR \[eax\+eax\*1\]
[ 	]*[a-f0-9]+:	8f e9 70 01 14 5d f8 ff ff ff[ 	]+blsfill ecx,DWORD PTR \[ebx\*2-0x8\]
[ 	]*[a-f0-9]+:	8f e9 40 01 f0[ 	]+blsic  edi,eax
[ 	]*[a-f0-9]+:	8f e9 60 01 36[ 	]+blsic  ebx,DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	8f e9 50 01 34 5d 00 00 00 00[ 	]+blsic  ebp,DWORD PTR \[ebx\*2\+0x0\]
[ 	]*[a-f0-9]+:	8f e9 78 01 34 41[ 	]+blsic  eax,DWORD PTR \[ecx\+eax\*2\]
[ 	]*[a-f0-9]+:	8f e9 58 01 37[ 	]+blsic  esp,DWORD PTR \[edi\]
[ 	]*[a-f0-9]+:	8f e9 78 01 33[ 	]+blsic  eax,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 70 01 f7[ 	]+blsic  ecx,edi
[ 	]*[a-f0-9]+:	8f e9 40 01 74 18 51[ 	]+blsic  edi,DWORD PTR \[eax\+ebx\*1\+0x51\]
[ 	]*[a-f0-9]+:	8f e9 68 01 f4[ 	]+blsic  edx,esp
[ 	]*[a-f0-9]+:	8f e9 68 01 74 3e 99[ 	]+blsic  edx,DWORD PTR \[esi\+edi\*1-0x67\]
[ 	]*[a-f0-9]+:	8f e9 40 01 31[ 	]+blsic  edi,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	8f e9 48 01 74 8e 67[ 	]+blsic  esi,DWORD PTR \[esi\+ecx\*4\+0x67\]
[ 	]*[a-f0-9]+:	8f e9 40 01 b4 d3 81 00 00 00[ 	]+blsic  edi,DWORD PTR \[ebx\+edx\*8\+0x81\]
[ 	]*[a-f0-9]+:	8f e9 40 01 74 11 0e[ 	]+blsic  edi,DWORD PTR \[ecx\+edx\*1\+0xe\]
[ 	]*[a-f0-9]+:	8f e9 58 01 70 3b[ 	]+blsic  esp,DWORD PTR \[eax\+0x3b\]
[ 	]*[a-f0-9]+:	8f e9 40 01 f1[ 	]+blsic  edi,ecx
[ 	]*[a-f0-9]+:	8f e9 78 01 f8[ 	]+t1mskc eax,eax
[ 	]*[a-f0-9]+:	8f e9 40 01 ff[ 	]+t1mskc edi,edi
[ 	]*[a-f0-9]+:	8f e9 70 01 39[ 	]+t1mskc ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	8f e9 48 01 3c 33[ 	]+t1mskc esi,DWORD PTR \[ebx\+esi\*1\]
[ 	]*[a-f0-9]+:	8f e9 50 01 fa[ 	]+t1mskc ebp,edx
[ 	]*[a-f0-9]+:	8f e9 68 01 3c 0d 00 00 00 00[ 	]+t1mskc edx,DWORD PTR \[ecx\*1\+0x0\]
[ 	]*[a-f0-9]+:	8f e9 58 01 3c b5 00 00 00 00[ 	]+t1mskc esp,DWORD PTR \[esi\*4\+0x0\]
[ 	]*[a-f0-9]+:	8f e9 70 01 fb[ 	]+t1mskc ecx,ebx
[ 	]*[a-f0-9]+:	8f e9 60 01 3b[ 	]+t1mskc ebx,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 40 01 fc[ 	]+t1mskc edi,esp
[ 	]*[a-f0-9]+:	8f e9 40 01 38[ 	]+t1mskc edi,DWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	8f e9 78 01 f9[ 	]+t1mskc eax,ecx
[ 	]*[a-f0-9]+:	8f e9 40 01 b8 ad de 00 00[ 	]+t1mskc edi,DWORD PTR \[eax\+0xdead\]
[ 	]*[a-f0-9]+:	8f e9 68 01 f9[ 	]+t1mskc edx,ecx
[ 	]*[a-f0-9]+:	8f e9 60 01 3c 15 ad de 00 00[ 	]+t1mskc ebx,DWORD PTR \[edx\*1\+0xdead\]
[ 	]*[a-f0-9]+:	8f e9 40 01 3a[ 	]+t1mskc edi,DWORD PTR \[edx\]
[ 	]*[a-f0-9]+:	8f e9 58 01 23[ 	]+tzmsk  esp,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 78 01 e7[ 	]+tzmsk  eax,edi
[ 	]*[a-f0-9]+:	8f e9 48 01 a7 02 35 ff ff[ 	]+tzmsk  esi,DWORD PTR \[edi-0xcafe\]
[ 	]*[a-f0-9]+:	8f e9 68 01 24 3d 00 00 00 00[ 	]+tzmsk  edx,DWORD PTR \[edi\*1\+0x0\]
[ 	]*[a-f0-9]+:	8f e9 50 01 e0[ 	]+tzmsk  ebp,eax
[ 	]*[a-f0-9]+:	8f e9 60 01 e5[ 	]+tzmsk  ebx,ebp
[ 	]*[a-f0-9]+:	8f e9 40 01 26[ 	]+tzmsk  edi,DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	8f e9 70 01 21[ 	]+tzmsk  ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	8f e9 40 01 24 45 00 00 00 00[ 	]+tzmsk  edi,DWORD PTR \[eax\*2\+0x0\]
[ 	]*[a-f0-9]+:	8f e9 40 01 e7[ 	]+tzmsk  edi,edi
[ 	]*[a-f0-9]+:	8f e9 68 01 e4[ 	]+tzmsk  edx,esp
[ 	]*[a-f0-9]+:	8f e9 70 01 20[ 	]+tzmsk  ecx,DWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	8f e9 78 01 24 3a[ 	]+tzmsk  eax,DWORD PTR \[edx\+edi\*1\]
[ 	]*[a-f0-9]+:	8f e9 78 01 23[ 	]+tzmsk  eax,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 78 01 a3 d9 c6 2a 2a[ 	]+tzmsk  eax,DWORD PTR \[ebx\+0x2a2ac6d9\]
[ 	]*[a-f0-9]+:	8f e9 70 01 a4 01 47 e9 ff ff[ 	]+tzmsk  ecx,DWORD PTR \[ecx\+eax\*1-0x16b9\]
#pass
