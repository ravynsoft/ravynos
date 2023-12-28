#as: -mevexlig=256
#objdump: -dwMintel
#name: i386 AVX512 lig256 insns (Intel disassembly)
#source: evex-lig.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 f4    	vaddsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 58 f4    	vaddsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 58 f4    	vaddsd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 58 f4    	vaddsd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 58 f4    	vaddsd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 58 f4    	vaddsd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 31    	vaddsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 b4 f4 c0 1d fe ff 	vaddsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 72 7f 	vaddsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 b2 00 04 00 00 	vaddsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 72 80 	vaddsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 b2 f8 fb ff ff 	vaddsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 f4    	vaddss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 58 f4    	vaddss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 58 f4    	vaddss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 58 f4    	vaddss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 58 f4    	vaddss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 58 f4    	vaddss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 31    	vaddss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 b4 f4 c0 1d fe ff 	vaddss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 72 7f 	vaddss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 b2 00 02 00 00 	vaddss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 72 80 	vaddss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 b2 fc fd ff ff 	vaddss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec ab 	vcmpsd k5\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec ab 	vcmpsd k5\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 7b 	vcmpsd k5\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 7b 	vcmpsd k5\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 7b 	vcmpsd k5\{k7\},xmm5,QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 7b 	vcmpsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 7b 	vcmpsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 7b 	vcmpsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 7b 	vcmpsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 7b 	vcmpsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 00 	vcmpeqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 00 	vcmpeqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 00 	vcmpeqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 00 	vcmpeqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 01 	vcmpltsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 01 	vcmpltsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 01 	vcmpltsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 01 	vcmpltsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 02 	vcmplesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 02 	vcmplesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 02 	vcmplesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 02 	vcmplesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 03 	vcmpunordsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 03 	vcmpunordsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 03 	vcmpunordsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 03 	vcmpunordsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 04 	vcmpneqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 04 	vcmpneqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 04 	vcmpneqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 04 	vcmpneqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 05 	vcmpnltsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 05 	vcmpnltsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 05 	vcmpnltsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 05 	vcmpnltsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 06 	vcmpnlesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 06 	vcmpnlesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 06 	vcmpnlesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 06 	vcmpnlesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 07 	vcmpordsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 07 	vcmpordsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 07 	vcmpordsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 07 	vcmpordsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 08 	vcmpeq_uqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 08 	vcmpeq_uqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 08 	vcmpeq_uqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 08 	vcmpeq_uqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 08 	vcmpeq_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 08 	vcmpeq_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 08 	vcmpeq_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 08 	vcmpeq_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 09 	vcmpngesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 09 	vcmpngesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 09 	vcmpngesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 09 	vcmpngesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0a 	vcmpngtsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0a 	vcmpngtsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0a 	vcmpngtsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0a 	vcmpngtsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0c 	vcmpneq_oqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0c 	vcmpneq_oqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0c 	vcmpneq_oqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0c 	vcmpneq_oqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0c 	vcmpneq_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0c 	vcmpneq_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0c 	vcmpneq_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0c 	vcmpneq_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0d 	vcmpgesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0d 	vcmpgesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0d 	vcmpgesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0d 	vcmpgesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0e 	vcmpgtsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0e 	vcmpgtsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0e 	vcmpgtsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0e 	vcmpgtsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0f 	vcmptruesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0f 	vcmptruesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0f 	vcmptruesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0f 	vcmptruesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 10 	vcmpeq_ossd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 10 	vcmpeq_ossd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 10 	vcmpeq_ossd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 10 	vcmpeq_ossd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 10 	vcmpeq_ossd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 10 	vcmpeq_ossd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 10 	vcmpeq_ossd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 10 	vcmpeq_ossd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 11 	vcmplt_oqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 11 	vcmplt_oqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 11 	vcmplt_oqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 11 	vcmplt_oqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 11 	vcmplt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 11 	vcmplt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 11 	vcmplt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 11 	vcmplt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 12 	vcmple_oqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 12 	vcmple_oqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 12 	vcmple_oqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 12 	vcmple_oqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 12 	vcmple_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 12 	vcmple_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 12 	vcmple_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 12 	vcmple_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 13 	vcmpunord_ssd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 13 	vcmpunord_ssd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 13 	vcmpunord_ssd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 13 	vcmpunord_ssd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 13 	vcmpunord_ssd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 13 	vcmpunord_ssd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 13 	vcmpunord_ssd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 13 	vcmpunord_ssd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 14 	vcmpneq_ussd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 14 	vcmpneq_ussd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 14 	vcmpneq_ussd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 14 	vcmpneq_ussd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 14 	vcmpneq_ussd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 14 	vcmpneq_ussd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 14 	vcmpneq_ussd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 14 	vcmpneq_ussd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 15 	vcmpnlt_uqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 15 	vcmpnlt_uqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 15 	vcmpnlt_uqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 15 	vcmpnlt_uqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 15 	vcmpnlt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 15 	vcmpnlt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 15 	vcmpnlt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 15 	vcmpnlt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 16 	vcmpnle_uqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 16 	vcmpnle_uqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 16 	vcmpnle_uqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 16 	vcmpnle_uqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 16 	vcmpnle_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 16 	vcmpnle_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 16 	vcmpnle_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 16 	vcmpnle_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 17 	vcmpord_ssd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 17 	vcmpord_ssd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 17 	vcmpord_ssd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 17 	vcmpord_ssd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 17 	vcmpord_ssd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 17 	vcmpord_ssd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 17 	vcmpord_ssd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 17 	vcmpord_ssd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 18 	vcmpeq_ussd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 18 	vcmpeq_ussd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 18 	vcmpeq_ussd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 18 	vcmpeq_ussd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 18 	vcmpeq_ussd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 18 	vcmpeq_ussd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 18 	vcmpeq_ussd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 18 	vcmpeq_ussd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 19 	vcmpnge_uqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 19 	vcmpnge_uqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 19 	vcmpnge_uqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 19 	vcmpnge_uqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 19 	vcmpnge_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 19 	vcmpnge_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 19 	vcmpnge_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 19 	vcmpnge_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 1a 	vcmpngt_uqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1a 	vcmpngt_uqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 1a 	vcmpngt_uqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 1a 	vcmpngt_uqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 1a 	vcmpngt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 1a 	vcmpngt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 1a 	vcmpngt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 1a 	vcmpngt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 1b 	vcmpfalse_ossd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1b 	vcmpfalse_ossd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 1b 	vcmpfalse_ossd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 1b 	vcmpfalse_ossd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 1b 	vcmpfalse_ossd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 1b 	vcmpfalse_ossd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 1b 	vcmpfalse_ossd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 1b 	vcmpfalse_ossd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 1c 	vcmpneq_ossd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1c 	vcmpneq_ossd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 1c 	vcmpneq_ossd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 1c 	vcmpneq_ossd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 1c 	vcmpneq_ossd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 1c 	vcmpneq_ossd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 1c 	vcmpneq_ossd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 1c 	vcmpneq_ossd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 1d 	vcmpge_oqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1d 	vcmpge_oqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 1d 	vcmpge_oqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 1d 	vcmpge_oqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 1d 	vcmpge_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 1d 	vcmpge_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 1d 	vcmpge_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 1d 	vcmpge_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 1e 	vcmpgt_oqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1e 	vcmpgt_oqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 1e 	vcmpgt_oqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 1e 	vcmpgt_oqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 1e 	vcmpgt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 1e 	vcmpgt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 1e 	vcmpgt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 1e 	vcmpgt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 1f 	vcmptrue_ussd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1f 	vcmptrue_ussd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 1f 	vcmptrue_ussd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 1f 	vcmptrue_ussd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 1f 	vcmptrue_ussd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 1f 	vcmptrue_ussd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 1f 	vcmptrue_ussd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 1f 	vcmptrue_ussd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec ab 	vcmpss k5\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec ab 	vcmpss k5\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 7b 	vcmpss k5\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 7b 	vcmpss k5\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 7b 	vcmpss k5\{k7\},xmm5,DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 7b 	vcmpss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 7b 	vcmpss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 7b 	vcmpss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 7b 	vcmpss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 7b 	vcmpss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 00 	vcmpeqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 00 	vcmpeqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 00 	vcmpeqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 00 	vcmpeqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 01 	vcmpltss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 01 	vcmpltss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 01 	vcmpltss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 01 	vcmpltss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 02 	vcmpless k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 02 	vcmpless k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 02 	vcmpless k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 02 	vcmpless k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 03 	vcmpunordss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 03 	vcmpunordss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 03 	vcmpunordss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 03 	vcmpunordss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 04 	vcmpneqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 04 	vcmpneqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 04 	vcmpneqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 04 	vcmpneqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 05 	vcmpnltss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 05 	vcmpnltss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 05 	vcmpnltss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 05 	vcmpnltss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 06 	vcmpnless k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 06 	vcmpnless k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 06 	vcmpnless k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 06 	vcmpnless k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 07 	vcmpordss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 07 	vcmpordss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 07 	vcmpordss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 07 	vcmpordss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 08 	vcmpeq_uqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 08 	vcmpeq_uqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 08 	vcmpeq_uqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 08 	vcmpeq_uqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 08 	vcmpeq_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 08 	vcmpeq_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 08 	vcmpeq_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 08 	vcmpeq_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 09 	vcmpngess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 09 	vcmpngess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 09 	vcmpngess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 09 	vcmpngess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0a 	vcmpngtss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0a 	vcmpngtss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0a 	vcmpngtss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0a 	vcmpngtss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0b 	vcmpfalsess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0b 	vcmpfalsess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0b 	vcmpfalsess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0b 	vcmpfalsess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0c 	vcmpneq_oqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0c 	vcmpneq_oqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0c 	vcmpneq_oqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0c 	vcmpneq_oqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0c 	vcmpneq_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0c 	vcmpneq_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0c 	vcmpneq_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0c 	vcmpneq_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0d 	vcmpgess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0d 	vcmpgess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0d 	vcmpgess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0d 	vcmpgess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0e 	vcmpgtss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0e 	vcmpgtss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0e 	vcmpgtss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0e 	vcmpgtss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0f 	vcmptruess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0f 	vcmptruess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0f 	vcmptruess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0f 	vcmptruess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 10 	vcmpeq_osss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 10 	vcmpeq_osss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 10 	vcmpeq_osss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 10 	vcmpeq_osss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 10 	vcmpeq_osss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 10 	vcmpeq_osss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 10 	vcmpeq_osss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 10 	vcmpeq_osss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 11 	vcmplt_oqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 11 	vcmplt_oqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 11 	vcmplt_oqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 11 	vcmplt_oqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 11 	vcmplt_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 11 	vcmplt_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 11 	vcmplt_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 11 	vcmplt_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 12 	vcmple_oqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 12 	vcmple_oqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 12 	vcmple_oqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 12 	vcmple_oqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 12 	vcmple_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 12 	vcmple_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 12 	vcmple_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 12 	vcmple_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 13 	vcmpunord_sss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 13 	vcmpunord_sss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 13 	vcmpunord_sss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 13 	vcmpunord_sss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 13 	vcmpunord_sss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 13 	vcmpunord_sss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 13 	vcmpunord_sss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 13 	vcmpunord_sss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 14 	vcmpneq_usss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 14 	vcmpneq_usss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 14 	vcmpneq_usss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 14 	vcmpneq_usss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 14 	vcmpneq_usss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 14 	vcmpneq_usss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 14 	vcmpneq_usss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 14 	vcmpneq_usss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 15 	vcmpnlt_uqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 15 	vcmpnlt_uqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 15 	vcmpnlt_uqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 15 	vcmpnlt_uqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 15 	vcmpnlt_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 15 	vcmpnlt_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 15 	vcmpnlt_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 15 	vcmpnlt_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 16 	vcmpnle_uqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 16 	vcmpnle_uqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 16 	vcmpnle_uqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 16 	vcmpnle_uqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 16 	vcmpnle_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 16 	vcmpnle_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 16 	vcmpnle_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 16 	vcmpnle_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 17 	vcmpord_sss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 17 	vcmpord_sss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 17 	vcmpord_sss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 17 	vcmpord_sss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 17 	vcmpord_sss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 17 	vcmpord_sss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 17 	vcmpord_sss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 17 	vcmpord_sss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 18 	vcmpeq_usss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 18 	vcmpeq_usss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 18 	vcmpeq_usss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 18 	vcmpeq_usss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 18 	vcmpeq_usss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 18 	vcmpeq_usss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 18 	vcmpeq_usss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 18 	vcmpeq_usss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 19 	vcmpnge_uqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 19 	vcmpnge_uqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 19 	vcmpnge_uqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 19 	vcmpnge_uqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 19 	vcmpnge_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 19 	vcmpnge_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 19 	vcmpnge_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 19 	vcmpnge_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 1a 	vcmpngt_uqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1a 	vcmpngt_uqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 1a 	vcmpngt_uqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 1a 	vcmpngt_uqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 1a 	vcmpngt_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 1a 	vcmpngt_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 1a 	vcmpngt_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 1a 	vcmpngt_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 1b 	vcmpfalse_osss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1b 	vcmpfalse_osss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 1b 	vcmpfalse_osss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 1b 	vcmpfalse_osss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 1b 	vcmpfalse_osss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 1b 	vcmpfalse_osss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 1b 	vcmpfalse_osss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 1b 	vcmpfalse_osss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 1c 	vcmpneq_osss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1c 	vcmpneq_osss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 1c 	vcmpneq_osss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 1c 	vcmpneq_osss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 1c 	vcmpneq_osss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 1c 	vcmpneq_osss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 1c 	vcmpneq_osss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 1c 	vcmpneq_osss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 1d 	vcmpge_oqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1d 	vcmpge_oqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 1d 	vcmpge_oqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 1d 	vcmpge_oqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 1d 	vcmpge_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 1d 	vcmpge_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 1d 	vcmpge_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 1d 	vcmpge_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 1e 	vcmpgt_oqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1e 	vcmpgt_oqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 1e 	vcmpgt_oqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 1e 	vcmpgt_oqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 1e 	vcmpgt_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 1e 	vcmpgt_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 1e 	vcmpgt_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 1e 	vcmpgt_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 1f 	vcmptrue_usss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1f 	vcmptrue_usss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 1f 	vcmptrue_usss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 1f 	vcmptrue_usss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 1f 	vcmptrue_usss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 1f 	vcmptrue_usss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 1f 	vcmptrue_usss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 1f 	vcmptrue_usss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 fd 18 2f f5    	vcomisd xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7c 18 2f f5    	vcomiss xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 18 2d c6    	vcvtsd2si eax,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 58 2d c6    	vcvtsd2si eax,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 38 2d c6    	vcvtsd2si eax,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 78 2d c6    	vcvtsd2si eax,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 18 2d ee    	vcvtsd2si ebp,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 58 2d ee    	vcvtsd2si ebp,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 38 2d ee    	vcvtsd2si ebp,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 78 2d ee    	vcvtsd2si ebp,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a f4    	vcvtsd2ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 5a f4    	vcvtsd2ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5a f4    	vcvtsd2ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 5a f4    	vcvtsd2ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 5a f4    	vcvtsd2ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 5a f4    	vcvtsd2ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a 31    	vcvtsd2ss xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a b4 f4 c0 1d fe ff 	vcvtsd2ss xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a 72 7f 	vcvtsd2ss xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a b2 00 04 00 00 	vcvtsd2ss xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a 72 80 	vcvtsd2ss xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a b2 f8 fb ff ff 	vcvtsd2ss xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 18 2a f0    	vcvtsi2ss xmm6,xmm5,eax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 58 2a f0    	vcvtsi2ss xmm6,xmm5,eax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 38 2a f0    	vcvtsi2ss xmm6,xmm5,eax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 78 2a f0    	vcvtsi2ss xmm6,xmm5,eax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 18 2a f5    	vcvtsi2ss xmm6,xmm5,ebp\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 58 2a f5    	vcvtsi2ss xmm6,xmm5,ebp\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 38 2a f5    	vcvtsi2ss xmm6,xmm5,ebp\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 78 2a f5    	vcvtsi2ss xmm6,xmm5,ebp\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a f4    	vcvtss2sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 5a f4    	vcvtss2sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 5a f4    	vcvtss2sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a 31    	vcvtss2sd xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a b4 f4 c0 1d fe ff 	vcvtss2sd xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a 72 7f 	vcvtss2sd xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a b2 00 02 00 00 	vcvtss2sd xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a 72 80 	vcvtss2sd xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a b2 fc fd ff ff 	vcvtss2sd xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 7e 18 2d c6    	vcvtss2si eax,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 58 2d c6    	vcvtss2si eax,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 38 2d c6    	vcvtss2si eax,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 78 2d c6    	vcvtss2si eax,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 18 2d ee    	vcvtss2si ebp,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 58 2d ee    	vcvtss2si ebp,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 38 2d ee    	vcvtss2si ebp,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 78 2d ee    	vcvtss2si ebp,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 18 2c c6    	vcvttsd2si eax,xmm6\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 18 2c ee    	vcvttsd2si ebp,xmm6\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 18 2c c6    	vcvttss2si eax,xmm6\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 18 2c ee    	vcvttss2si ebp,xmm6\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e f4    	vdivsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 5e f4    	vdivsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5e f4    	vdivsd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 5e f4    	vdivsd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 5e f4    	vdivsd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 5e f4    	vdivsd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e 31    	vdivsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e b4 f4 c0 1d fe ff 	vdivsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e 72 7f 	vdivsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e b2 00 04 00 00 	vdivsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e 72 80 	vdivsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e b2 f8 fb ff ff 	vdivsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e f4    	vdivss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 5e f4    	vdivss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 5e f4    	vdivss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 5e f4    	vdivss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 5e f4    	vdivss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 5e f4    	vdivss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e 31    	vdivss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e b4 f4 c0 1d fe ff 	vdivss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e 72 7f 	vdivss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e b2 00 02 00 00 	vdivss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e 72 80 	vdivss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e b2 fc fd ff ff 	vdivss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 f4    	vfmadd132sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 99 f4    	vfmadd132sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f 99 f4    	vfmadd132sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 99 f4    	vfmadd132sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 99 f4    	vfmadd132sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 99 f4    	vfmadd132sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 31    	vfmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 b4 f4 c0 1d fe ff 	vfmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 72 7f 	vfmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 b2 00 04 00 00 	vfmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 72 80 	vfmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 b2 f8 fb ff ff 	vfmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 f4    	vfmadd132ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 99 f4    	vfmadd132ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f 99 f4    	vfmadd132ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 99 f4    	vfmadd132ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 99 f4    	vfmadd132ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 99 f4    	vfmadd132ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 31    	vfmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 b4 f4 c0 1d fe ff 	vfmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 72 7f 	vfmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 b2 00 02 00 00 	vfmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 72 80 	vfmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 b2 fc fd ff ff 	vfmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 f4    	vfmadd213sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af a9 f4    	vfmadd213sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f a9 f4    	vfmadd213sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f a9 f4    	vfmadd213sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f a9 f4    	vfmadd213sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f a9 f4    	vfmadd213sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 31    	vfmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 b4 f4 c0 1d fe ff 	vfmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 72 7f 	vfmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 b2 00 04 00 00 	vfmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 72 80 	vfmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 b2 f8 fb ff ff 	vfmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 f4    	vfmadd213ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af a9 f4    	vfmadd213ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f a9 f4    	vfmadd213ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f a9 f4    	vfmadd213ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f a9 f4    	vfmadd213ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f a9 f4    	vfmadd213ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 31    	vfmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 b4 f4 c0 1d fe ff 	vfmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 72 7f 	vfmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 b2 00 02 00 00 	vfmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 72 80 	vfmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 b2 fc fd ff ff 	vfmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 f4    	vfmadd231sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af b9 f4    	vfmadd231sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f b9 f4    	vfmadd231sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f b9 f4    	vfmadd231sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f b9 f4    	vfmadd231sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f b9 f4    	vfmadd231sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 31    	vfmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 b4 f4 c0 1d fe ff 	vfmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 72 7f 	vfmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 b2 00 04 00 00 	vfmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 72 80 	vfmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 b2 f8 fb ff ff 	vfmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 f4    	vfmadd231ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af b9 f4    	vfmadd231ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f b9 f4    	vfmadd231ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f b9 f4    	vfmadd231ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f b9 f4    	vfmadd231ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f b9 f4    	vfmadd231ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 31    	vfmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 b4 f4 c0 1d fe ff 	vfmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 72 7f 	vfmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 b2 00 02 00 00 	vfmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 72 80 	vfmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 b2 fc fd ff ff 	vfmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b f4    	vfmsub132sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 9b f4    	vfmsub132sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f 9b f4    	vfmsub132sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 9b f4    	vfmsub132sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 9b f4    	vfmsub132sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 9b f4    	vfmsub132sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b 31    	vfmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b b4 f4 c0 1d fe ff 	vfmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b 72 7f 	vfmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b b2 00 04 00 00 	vfmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b 72 80 	vfmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b b2 f8 fb ff ff 	vfmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b f4    	vfmsub132ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 9b f4    	vfmsub132ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f 9b f4    	vfmsub132ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 9b f4    	vfmsub132ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 9b f4    	vfmsub132ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 9b f4    	vfmsub132ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b 31    	vfmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b b4 f4 c0 1d fe ff 	vfmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b 72 7f 	vfmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b b2 00 02 00 00 	vfmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b 72 80 	vfmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b b2 fc fd ff ff 	vfmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab f4    	vfmsub213sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af ab f4    	vfmsub213sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f ab f4    	vfmsub213sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f ab f4    	vfmsub213sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f ab f4    	vfmsub213sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f ab f4    	vfmsub213sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab 31    	vfmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab b4 f4 c0 1d fe ff 	vfmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab 72 7f 	vfmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab b2 00 04 00 00 	vfmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab 72 80 	vfmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab b2 f8 fb ff ff 	vfmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ab f4    	vfmsub213ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af ab f4    	vfmsub213ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f ab f4    	vfmsub213ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f ab f4    	vfmsub213ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f ab f4    	vfmsub213ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f ab f4    	vfmsub213ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f ab 31    	vfmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ab b4 f4 c0 1d fe ff 	vfmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ab 72 7f 	vfmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ab b2 00 02 00 00 	vfmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ab 72 80 	vfmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ab b2 fc fd ff ff 	vfmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb f4    	vfmsub231sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af bb f4    	vfmsub231sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f bb f4    	vfmsub231sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f bb f4    	vfmsub231sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f bb f4    	vfmsub231sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f bb f4    	vfmsub231sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb 31    	vfmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb b4 f4 c0 1d fe ff 	vfmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb 72 7f 	vfmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb b2 00 04 00 00 	vfmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb 72 80 	vfmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb b2 f8 fb ff ff 	vfmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bb f4    	vfmsub231ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af bb f4    	vfmsub231ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f bb f4    	vfmsub231ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f bb f4    	vfmsub231ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f bb f4    	vfmsub231ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f bb f4    	vfmsub231ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f bb 31    	vfmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bb b4 f4 c0 1d fe ff 	vfmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bb 72 7f 	vfmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bb b2 00 02 00 00 	vfmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bb 72 80 	vfmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bb b2 fc fd ff ff 	vfmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d f4    	vfnmadd132sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 9d f4    	vfnmadd132sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f 9d f4    	vfnmadd132sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 9d f4    	vfnmadd132sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 9d f4    	vfnmadd132sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 9d f4    	vfnmadd132sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d 31    	vfnmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d b4 f4 c0 1d fe ff 	vfnmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d 72 7f 	vfnmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d b2 00 04 00 00 	vfnmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d 72 80 	vfnmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d b2 f8 fb ff ff 	vfnmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d f4    	vfnmadd132ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 9d f4    	vfnmadd132ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f 9d f4    	vfnmadd132ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 9d f4    	vfnmadd132ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 9d f4    	vfnmadd132ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 9d f4    	vfnmadd132ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d 31    	vfnmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d b4 f4 c0 1d fe ff 	vfnmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d 72 7f 	vfnmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d b2 00 02 00 00 	vfnmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d 72 80 	vfnmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d b2 fc fd ff ff 	vfnmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad f4    	vfnmadd213sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af ad f4    	vfnmadd213sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f ad f4    	vfnmadd213sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f ad f4    	vfnmadd213sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f ad f4    	vfnmadd213sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f ad f4    	vfnmadd213sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad 31    	vfnmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad b4 f4 c0 1d fe ff 	vfnmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad 72 7f 	vfnmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad b2 00 04 00 00 	vfnmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad 72 80 	vfnmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad b2 f8 fb ff ff 	vfnmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ad f4    	vfnmadd213ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af ad f4    	vfnmadd213ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f ad f4    	vfnmadd213ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f ad f4    	vfnmadd213ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f ad f4    	vfnmadd213ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f ad f4    	vfnmadd213ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f ad 31    	vfnmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ad b4 f4 c0 1d fe ff 	vfnmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ad 72 7f 	vfnmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ad b2 00 02 00 00 	vfnmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ad 72 80 	vfnmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ad b2 fc fd ff ff 	vfnmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd f4    	vfnmadd231sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af bd f4    	vfnmadd231sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f bd f4    	vfnmadd231sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f bd f4    	vfnmadd231sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f bd f4    	vfnmadd231sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f bd f4    	vfnmadd231sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd 31    	vfnmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd b4 f4 c0 1d fe ff 	vfnmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd 72 7f 	vfnmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd b2 00 04 00 00 	vfnmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd 72 80 	vfnmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd b2 f8 fb ff ff 	vfnmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bd f4    	vfnmadd231ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af bd f4    	vfnmadd231ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f bd f4    	vfnmadd231ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f bd f4    	vfnmadd231ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f bd f4    	vfnmadd231ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f bd f4    	vfnmadd231ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f bd 31    	vfnmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bd b4 f4 c0 1d fe ff 	vfnmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bd 72 7f 	vfnmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bd b2 00 02 00 00 	vfnmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bd 72 80 	vfnmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bd b2 fc fd ff ff 	vfnmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f f4    	vfnmsub132sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 9f f4    	vfnmsub132sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f 9f f4    	vfnmsub132sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 9f f4    	vfnmsub132sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 9f f4    	vfnmsub132sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 9f f4    	vfnmsub132sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f 31    	vfnmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f b4 f4 c0 1d fe ff 	vfnmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f 72 7f 	vfnmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f b2 00 04 00 00 	vfnmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f 72 80 	vfnmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f b2 f8 fb ff ff 	vfnmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f f4    	vfnmsub132ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 9f f4    	vfnmsub132ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f 9f f4    	vfnmsub132ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 9f f4    	vfnmsub132ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 9f f4    	vfnmsub132ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 9f f4    	vfnmsub132ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f 31    	vfnmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f b4 f4 c0 1d fe ff 	vfnmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f 72 7f 	vfnmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f b2 00 02 00 00 	vfnmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f 72 80 	vfnmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f b2 fc fd ff ff 	vfnmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f af f4    	vfnmsub213sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af af f4    	vfnmsub213sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f af f4    	vfnmsub213sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f af f4    	vfnmsub213sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f af f4    	vfnmsub213sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f af f4    	vfnmsub213sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f af 31    	vfnmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f af b4 f4 c0 1d fe ff 	vfnmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f af 72 7f 	vfnmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f af b2 00 04 00 00 	vfnmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f af 72 80 	vfnmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f af b2 f8 fb ff ff 	vfnmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f af f4    	vfnmsub213ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af af f4    	vfnmsub213ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f af f4    	vfnmsub213ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f af f4    	vfnmsub213ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f af f4    	vfnmsub213ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f af f4    	vfnmsub213ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f af 31    	vfnmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f af b4 f4 c0 1d fe ff 	vfnmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f af 72 7f 	vfnmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f af b2 00 02 00 00 	vfnmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f af 72 80 	vfnmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f af b2 fc fd ff ff 	vfnmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf f4    	vfnmsub231sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af bf f4    	vfnmsub231sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f bf f4    	vfnmsub231sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f bf f4    	vfnmsub231sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f bf f4    	vfnmsub231sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f bf f4    	vfnmsub231sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf 31    	vfnmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf b4 f4 c0 1d fe ff 	vfnmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf 72 7f 	vfnmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf b2 00 04 00 00 	vfnmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf 72 80 	vfnmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf b2 f8 fb ff ff 	vfnmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bf f4    	vfnmsub231ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af bf f4    	vfnmsub231ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f bf f4    	vfnmsub231ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f bf f4    	vfnmsub231ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f bf f4    	vfnmsub231ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f bf f4    	vfnmsub231ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f bf 31    	vfnmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bf b4 f4 c0 1d fe ff 	vfnmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bf 72 7f 	vfnmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bf b2 00 02 00 00 	vfnmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bf 72 80 	vfnmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bf b2 fc fd ff ff 	vfnmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 f4    	vgetexpsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 43 f4    	vgetexpsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f 43 f4    	vgetexpsd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 31    	vgetexpsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 b4 f4 c0 1d fe ff 	vgetexpsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 72 7f 	vgetexpsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 b2 00 04 00 00 	vgetexpsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 72 80 	vgetexpsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 b2 f8 fb ff ff 	vgetexpsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 f4    	vgetexpss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 43 f4    	vgetexpss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f 43 f4    	vgetexpss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 31    	vgetexpss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 b4 f4 c0 1d fe ff 	vgetexpss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 72 7f 	vgetexpss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 b2 00 02 00 00 	vgetexpss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 72 80 	vgetexpss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 b2 fc fd ff ff 	vgetexpss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 f4 ab 	vgetmantsd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 d5 af 27 f4 ab 	vgetmantsd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 d5 1f 27 f4 ab 	vgetmantsd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 f4 7b 	vgetmantsd xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 1f 27 f4 7b 	vgetmantsd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 31 7b 	vgetmantsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 b4 f4 c0 1d fe ff 7b 	vgetmantsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 72 7f 7b 	vgetmantsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 b2 00 04 00 00 7b 	vgetmantsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 72 80 7b 	vgetmantsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 b2 f8 fb ff ff 7b 	vgetmantsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 f4 ab 	vgetmantss xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 55 af 27 f4 ab 	vgetmantss xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 55 1f 27 f4 ab 	vgetmantss xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 f4 7b 	vgetmantss xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 55 1f 27 f4 7b 	vgetmantss xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 31 7b 	vgetmantss xmm6\{k7\},xmm5,DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 b4 f4 c0 1d fe ff 7b 	vgetmantss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 72 7f 7b 	vgetmantss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 b2 00 02 00 00 7b 	vgetmantss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 72 80 7b 	vgetmantss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 b2 fc fd ff ff 7b 	vgetmantss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f f4    	vmaxsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 5f f4    	vmaxsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5f f4    	vmaxsd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f 31    	vmaxsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f b4 f4 c0 1d fe ff 	vmaxsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f 72 7f 	vmaxsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f b2 00 04 00 00 	vmaxsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f 72 80 	vmaxsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f b2 f8 fb ff ff 	vmaxsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f f4    	vmaxss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 5f f4    	vmaxss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 5f f4    	vmaxss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f 31    	vmaxss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f b4 f4 c0 1d fe ff 	vmaxss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f 72 7f 	vmaxss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f b2 00 02 00 00 	vmaxss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f 72 80 	vmaxss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f b2 fc fd ff ff 	vmaxss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d f4    	vminsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 5d f4    	vminsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5d f4    	vminsd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d 31    	vminsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d b4 f4 c0 1d fe ff 	vminsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d 72 7f 	vminsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d b2 00 04 00 00 	vminsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d 72 80 	vminsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d b2 f8 fb ff ff 	vminsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d f4    	vminss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 5d f4    	vminss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 5d f4    	vminss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d 31    	vminss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d b4 f4 c0 1d fe ff 	vminss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d 72 7f 	vminss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d b2 00 02 00 00 	vminss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d 72 80 	vminss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d b2 fc fd ff ff 	vminss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 10 31    	vmovsd xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 ff af 10 31    	vmovsd xmm6\{k7\}\{z\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 10 b4 f4 c0 1d fe ff 	vmovsd xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 10 72 7f 	vmovsd xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 10 b2 00 04 00 00 	vmovsd xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 10 72 80 	vmovsd xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 10 b2 f8 fb ff ff 	vmovsd xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 11 31    	vmovsd QWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 ff 2f 11 b4 f4 c0 1d fe ff 	vmovsd QWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 ff 2f 11 72 7f 	vmovsd QWORD PTR \[edx\+0x3f8\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 ff 2f 11 b2 00 04 00 00 	vmovsd QWORD PTR \[edx\+0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 ff 2f 11 72 80 	vmovsd QWORD PTR \[edx-0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 ff 2f 11 b2 f8 fb ff ff 	vmovsd QWORD PTR \[edx-0x408\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 d7 2f 10 f4    	vmovsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 10 f4    	vmovsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 7e 2f 10 31    	vmovss xmm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 7e af 10 31    	vmovss xmm6\{k7\}\{z\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 7e 2f 10 b4 f4 c0 1d fe ff 	vmovss xmm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 7e 2f 10 72 7f 	vmovss xmm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 7e 2f 10 b2 00 02 00 00 	vmovss xmm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 2f 10 72 80 	vmovss xmm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 2f 10 b2 fc fd ff ff 	vmovss xmm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 7e 2f 11 31    	vmovss DWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 2f 11 b4 f4 c0 1d fe ff 	vmovss DWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 2f 11 72 7f 	vmovss DWORD PTR \[edx\+0x1fc\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 2f 11 b2 00 02 00 00 	vmovss DWORD PTR \[edx\+0x200\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 2f 11 72 80 	vmovss DWORD PTR \[edx-0x200\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 2f 11 b2 fc fd ff ff 	vmovss DWORD PTR \[edx-0x204\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 56 2f 10 f4    	vmovss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 10 f4    	vmovss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 f4    	vmulsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 59 f4    	vmulsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 59 f4    	vmulsd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 59 f4    	vmulsd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 59 f4    	vmulsd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 59 f4    	vmulsd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 31    	vmulsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 b4 f4 c0 1d fe ff 	vmulsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 72 7f 	vmulsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 b2 00 04 00 00 	vmulsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 72 80 	vmulsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 b2 f8 fb ff ff 	vmulsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 f4    	vmulss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 59 f4    	vmulss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 59 f4    	vmulss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 59 f4    	vmulss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 59 f4    	vmulss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 59 f4    	vmulss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 31    	vmulss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 b4 f4 c0 1d fe ff 	vmulss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 72 7f 	vmulss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 b2 00 02 00 00 	vmulss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 72 80 	vmulss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 b2 fc fd ff ff 	vmulss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d f4    	vrcp14sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 4d f4    	vrcp14sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d 31    	vrcp14sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d b4 f4 c0 1d fe ff 	vrcp14sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d 72 7f 	vrcp14sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d b2 00 04 00 00 	vrcp14sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d 72 80 	vrcp14sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d b2 f8 fb ff ff 	vrcp14sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d f4    	vrcp14ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 4d f4    	vrcp14ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d 31    	vrcp14ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d b4 f4 c0 1d fe ff 	vrcp14ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d 72 7f 	vrcp14ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d b2 00 02 00 00 	vrcp14ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d 72 80 	vrcp14ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d b2 fc fd ff ff 	vrcp14ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cb f4    	vrcp28ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af cb f4    	vrcp28ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f cb f4    	vrcp28ss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f cb 31    	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cb b4 f4 c0 1d fe ff 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cb 72 7f 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cb b2 00 02 00 00 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cb 72 80 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cb b2 fc fd ff ff 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb f4    	vrcp28sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af cb f4    	vrcp28sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f cb f4    	vrcp28sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb 31    	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb b4 f4 c0 1d fe ff 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb 72 7f 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb b2 00 04 00 00 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb 72 80 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb b2 f8 fb ff ff 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f f4    	vrsqrt14sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 4f f4    	vrsqrt14sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f 31    	vrsqrt14sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f b4 f4 c0 1d fe ff 	vrsqrt14sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f 72 7f 	vrsqrt14sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f b2 00 04 00 00 	vrsqrt14sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f 72 80 	vrsqrt14sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f b2 f8 fb ff ff 	vrsqrt14sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f f4    	vrsqrt14ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 4f f4    	vrsqrt14ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f 31    	vrsqrt14ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f b4 f4 c0 1d fe ff 	vrsqrt14ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f 72 7f 	vrsqrt14ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f b2 00 02 00 00 	vrsqrt14ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f 72 80 	vrsqrt14ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f b2 fc fd ff ff 	vrsqrt14ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cd f4    	vrsqrt28ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af cd f4    	vrsqrt28ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f cd f4    	vrsqrt28ss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f cd 31    	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cd b4 f4 c0 1d fe ff 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cd 72 7f 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cd b2 00 02 00 00 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cd 72 80 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cd b2 fc fd ff ff 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd f4    	vrsqrt28sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af cd f4    	vrsqrt28sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f cd f4    	vrsqrt28sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd 31    	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd b4 f4 c0 1d fe ff 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd 72 7f 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd b2 00 04 00 00 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd 72 80 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd b2 f8 fb ff ff 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 f4    	vsqrtsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 51 f4    	vsqrtsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 51 f4    	vsqrtsd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 51 f4    	vsqrtsd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 51 f4    	vsqrtsd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 51 f4    	vsqrtsd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 31    	vsqrtsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 b4 f4 c0 1d fe ff 	vsqrtsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 72 7f 	vsqrtsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 b2 00 04 00 00 	vsqrtsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 72 80 	vsqrtsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 b2 f8 fb ff ff 	vsqrtsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 f4    	vsqrtss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 51 f4    	vsqrtss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 51 f4    	vsqrtss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 51 f4    	vsqrtss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 51 f4    	vsqrtss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 51 f4    	vsqrtss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 31    	vsqrtss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 b4 f4 c0 1d fe ff 	vsqrtss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 72 7f 	vsqrtss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 b2 00 02 00 00 	vsqrtss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 72 80 	vsqrtss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 b2 fc fd ff ff 	vsqrtss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c f4    	vsubsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 5c f4    	vsubsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5c f4    	vsubsd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 5c f4    	vsubsd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 5c f4    	vsubsd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 5c f4    	vsubsd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c 31    	vsubsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c b4 f4 c0 1d fe ff 	vsubsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c 72 7f 	vsubsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c b2 00 04 00 00 	vsubsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c 72 80 	vsubsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c b2 f8 fb ff ff 	vsubsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c f4    	vsubss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 5c f4    	vsubss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 5c f4    	vsubss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 5c f4    	vsubss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 5c f4    	vsubss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 5c f4    	vsubss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c 31    	vsubss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c b4 f4 c0 1d fe ff 	vsubss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c 72 7f 	vsubss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c b2 00 02 00 00 	vsubss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c 72 80 	vsubss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c b2 fc fd ff ff 	vsubss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 fd 18 2e f5    	vucomisd xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7c 18 2e f5    	vucomiss xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 c6    	vcvtsd2usi eax,xmm6
[ 	]*[a-f0-9]+:	62 f1 7f 18 79 c6    	vcvtsd2usi eax,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 58 79 c6    	vcvtsd2usi eax,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 38 79 c6    	vcvtsd2usi eax,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 78 79 c6    	vcvtsd2usi eax,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 01    	vcvtsd2usi eax,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 84 f4 c0 1d fe ff 	vcvtsd2usi eax,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 42 7f 	vcvtsd2usi eax,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 82 00 04 00 00 	vcvtsd2usi eax,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 42 80 	vcvtsd2usi eax,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 82 f8 fb ff ff 	vcvtsd2usi eax,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 ee    	vcvtsd2usi ebp,xmm6
[ 	]*[a-f0-9]+:	62 f1 7f 18 79 ee    	vcvtsd2usi ebp,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 58 79 ee    	vcvtsd2usi ebp,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 38 79 ee    	vcvtsd2usi ebp,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 78 79 ee    	vcvtsd2usi ebp,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 29    	vcvtsd2usi ebp,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 ac f4 c0 1d fe ff 	vcvtsd2usi ebp,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 6a 7f 	vcvtsd2usi ebp,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 aa 00 04 00 00 	vcvtsd2usi ebp,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 6a 80 	vcvtsd2usi ebp,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 aa f8 fb ff ff 	vcvtsd2usi ebp,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 c6    	vcvtss2usi eax,xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 18 79 c6    	vcvtss2usi eax,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 58 79 c6    	vcvtss2usi eax,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 38 79 c6    	vcvtss2usi eax,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 78 79 c6    	vcvtss2usi eax,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 01    	vcvtss2usi eax,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 84 f4 c0 1d fe ff 	vcvtss2usi eax,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 42 7f 	vcvtss2usi eax,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 82 00 02 00 00 	vcvtss2usi eax,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 42 80 	vcvtss2usi eax,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 82 fc fd ff ff 	vcvtss2usi eax,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 ee    	vcvtss2usi ebp,xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 18 79 ee    	vcvtss2usi ebp,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 58 79 ee    	vcvtss2usi ebp,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 38 79 ee    	vcvtss2usi ebp,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 78 79 ee    	vcvtss2usi ebp,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 29    	vcvtss2usi ebp,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 ac f4 c0 1d fe ff 	vcvtss2usi ebp,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 6a 7f 	vcvtss2usi ebp,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 aa 00 02 00 00 	vcvtss2usi ebp,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 6a 80 	vcvtss2usi ebp,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 aa fc fd ff ff 	vcvtss2usi ebp,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 57 28 7b f0    	vcvtusi2sd xmm6,xmm5,eax
[ 	]*[a-f0-9]+:	62 f1 57 28 7b f5    	vcvtusi2sd xmm6,xmm5,ebp
[ 	]*[a-f0-9]+:	62 f1 57 28 7b 31    	vcvtusi2sd xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 57 28 7b b4 f4 c0 1d fe ff 	vcvtusi2sd xmm6,xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 57 28 7b 72 7f 	vcvtusi2sd xmm6,xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 57 28 7b b2 00 02 00 00 	vcvtusi2sd xmm6,xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 57 28 7b 72 80 	vcvtusi2sd xmm6,xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 57 28 7b b2 fc fd ff ff 	vcvtusi2sd xmm6,xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 28 7b f0    	vcvtusi2ss xmm6,xmm5,eax
[ 	]*[a-f0-9]+:	62 f1 56 18 7b f0    	vcvtusi2ss xmm6,xmm5,eax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 58 7b f0    	vcvtusi2ss xmm6,xmm5,eax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 38 7b f0    	vcvtusi2ss xmm6,xmm5,eax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 78 7b f0    	vcvtusi2ss xmm6,xmm5,eax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 28 7b f5    	vcvtusi2ss xmm6,xmm5,ebp
[ 	]*[a-f0-9]+:	62 f1 56 18 7b f5    	vcvtusi2ss xmm6,xmm5,ebp\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 58 7b f5    	vcvtusi2ss xmm6,xmm5,ebp\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 38 7b f5    	vcvtusi2ss xmm6,xmm5,ebp\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 78 7b f5    	vcvtusi2ss xmm6,xmm5,ebp\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 28 7b 31    	vcvtusi2ss xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 28 7b b4 f4 c0 1d fe ff 	vcvtusi2ss xmm6,xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 28 7b 72 7f 	vcvtusi2ss xmm6,xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 28 7b b2 00 02 00 00 	vcvtusi2ss xmm6,xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 28 7b 72 80 	vcvtusi2ss xmm6,xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 28 7b b2 fc fd ff ff 	vcvtusi2ss xmm6,xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d f4    	vscalefsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 2d f4    	vscalefsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f 2d f4    	vscalefsd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 2d f4    	vscalefsd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 2d f4    	vscalefsd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 2d f4    	vscalefsd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d 31    	vscalefsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d b4 f4 c0 1d fe ff 	vscalefsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d 72 7f 	vscalefsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d b2 00 04 00 00 	vscalefsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d 72 80 	vscalefsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d b2 f8 fb ff ff 	vscalefsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d f4    	vscalefss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 2d f4    	vscalefss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f 2d f4    	vscalefss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 2d f4    	vscalefss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 2d f4    	vscalefss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 2d f4    	vscalefss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d 31    	vscalefss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d b4 f4 c0 1d fe ff 	vscalefss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d 72 7f 	vscalefss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d b2 00 02 00 00 	vscalefss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d 72 80 	vscalefss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d b2 fc fd ff ff 	vscalefss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 f4 ab 	vfixupimmss xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 55 af 55 f4 ab 	vfixupimmss xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 55 1f 55 f4 ab 	vfixupimmss xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 f4 7b 	vfixupimmss xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 55 1f 55 f4 7b 	vfixupimmss xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 31 7b 	vfixupimmss xmm6\{k7\},xmm5,DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 b4 f4 c0 1d fe ff 7b 	vfixupimmss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 72 7f 7b 	vfixupimmss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 b2 00 02 00 00 7b 	vfixupimmss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 72 80 7b 	vfixupimmss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 b2 fc fd ff ff 7b 	vfixupimmss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 f4 ab 	vfixupimmsd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 d5 af 55 f4 ab 	vfixupimmsd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 d5 1f 55 f4 ab 	vfixupimmsd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 f4 7b 	vfixupimmsd xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 1f 55 f4 7b 	vfixupimmsd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 31 7b 	vfixupimmsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 b4 f4 c0 1d fe ff 7b 	vfixupimmsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 72 7f 7b 	vfixupimmsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 b2 00 04 00 00 7b 	vfixupimmsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 72 80 7b 	vfixupimmsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 b2 f8 fb ff ff 7b 	vfixupimmsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b f4 ab 	vrndscalesd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 d5 af 0b f4 ab 	vrndscalesd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 d5 1f 0b f4 ab 	vrndscalesd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b f4 7b 	vrndscalesd xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 1f 0b f4 7b 	vrndscalesd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b 31 7b 	vrndscalesd xmm6\{k7\},xmm5,QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b b4 f4 c0 1d fe ff 7b 	vrndscalesd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b 72 7f 7b 	vrndscalesd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b b2 00 04 00 00 7b 	vrndscalesd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b 72 80 7b 	vrndscalesd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b b2 f8 fb ff ff 7b 	vrndscalesd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a f4 ab 	vrndscaless xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 55 af 0a f4 ab 	vrndscaless xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 55 1f 0a f4 ab 	vrndscaless xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a f4 7b 	vrndscaless xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 55 1f 0a f4 7b 	vrndscaless xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a 31 7b 	vrndscaless xmm6\{k7\},xmm5,DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a b4 f4 c0 1d fe ff 7b 	vrndscaless xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a 72 7f 7b 	vrndscaless xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a b2 00 02 00 00 7b 	vrndscaless xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a 72 80 7b 	vrndscaless xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a b2 fc fd ff ff 7b 	vrndscaless xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 f3 56 28 c2 ec 7b 	vcmpsh k5,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 56 1f c2 ec 7b 	vcmpsh k5\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 56 28 c2 29 7b 	vcmpsh k5,xmm5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 56 2f c2 ac f4 c0 1d fe ff 7b 	vcmpsh k5\{k7\},xmm5,WORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 56 28 c2 69 7f 7b 	vcmpsh k5,xmm5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:	62 f3 56 2f c2 6a 80 7b 	vcmpsh k5\{k7\},xmm5,WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:	62 f3 7c 28 67 ec 7b 	vfpclasssh k5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 7c 28 67 29 7b 	vfpclasssh k5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 7c 2f 67 ac f4 c0 1d fe ff 7b 	vfpclasssh k5\{k7\},WORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 7c 28 67 69 7f 7b 	vfpclasssh k5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:	62 f3 7c 2f 67 6a 80 7b 	vfpclasssh k5\{k7\},WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 f4    	vaddsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 58 f4    	vaddsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 58 f4    	vaddsd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 58 f4    	vaddsd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 58 f4    	vaddsd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 58 f4    	vaddsd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 31    	vaddsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 b4 f4 c0 1d fe ff 	vaddsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 72 7f 	vaddsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 b2 00 04 00 00 	vaddsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 72 80 	vaddsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 58 b2 f8 fb ff ff 	vaddsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 f4    	vaddss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 58 f4    	vaddss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 58 f4    	vaddss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 58 f4    	vaddss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 58 f4    	vaddss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 58 f4    	vaddss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 31    	vaddss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 b4 f4 c0 1d fe ff 	vaddss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 72 7f 	vaddss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 b2 00 02 00 00 	vaddss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 72 80 	vaddss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 58 b2 fc fd ff ff 	vaddss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec ab 	vcmpsd k5\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec ab 	vcmpsd k5\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 7b 	vcmpsd k5\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 7b 	vcmpsd k5\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 7b 	vcmpsd k5\{k7\},xmm5,QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 7b 	vcmpsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 7b 	vcmpsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 7b 	vcmpsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 7b 	vcmpsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 7b 	vcmpsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 00 	vcmpeqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 00 	vcmpeqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 00 	vcmpeqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 00 	vcmpeqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 00 	vcmpeqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 01 	vcmpltsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 01 	vcmpltsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 01 	vcmpltsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 01 	vcmpltsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 01 	vcmpltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 02 	vcmplesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 02 	vcmplesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 02 	vcmplesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 02 	vcmplesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 02 	vcmplesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 03 	vcmpunordsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 03 	vcmpunordsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 03 	vcmpunordsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 03 	vcmpunordsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 03 	vcmpunordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 04 	vcmpneqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 04 	vcmpneqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 04 	vcmpneqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 04 	vcmpneqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 04 	vcmpneqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 05 	vcmpnltsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 05 	vcmpnltsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 05 	vcmpnltsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 05 	vcmpnltsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 05 	vcmpnltsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 06 	vcmpnlesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 06 	vcmpnlesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 06 	vcmpnlesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 06 	vcmpnlesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 06 	vcmpnlesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 07 	vcmpordsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 07 	vcmpordsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 07 	vcmpordsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 07 	vcmpordsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 07 	vcmpordsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 08 	vcmpeq_uqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 08 	vcmpeq_uqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 08 	vcmpeq_uqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 08 	vcmpeq_uqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 08 	vcmpeq_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 08 	vcmpeq_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 08 	vcmpeq_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 08 	vcmpeq_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 09 	vcmpngesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 09 	vcmpngesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 09 	vcmpngesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 09 	vcmpngesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 09 	vcmpngesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0a 	vcmpngtsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0a 	vcmpngtsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0a 	vcmpngtsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0a 	vcmpngtsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0a 	vcmpngtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0b 	vcmpfalsesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0c 	vcmpneq_oqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0c 	vcmpneq_oqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0c 	vcmpneq_oqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0c 	vcmpneq_oqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0c 	vcmpneq_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0c 	vcmpneq_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0c 	vcmpneq_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0c 	vcmpneq_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0d 	vcmpgesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0d 	vcmpgesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0d 	vcmpgesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0d 	vcmpgesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0d 	vcmpgesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0e 	vcmpgtsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0e 	vcmpgtsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0e 	vcmpgtsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0e 	vcmpgtsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0e 	vcmpgtsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0f 	vcmptruesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0f 	vcmptruesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 0f 	vcmptruesd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0f 	vcmptruesd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 0f 	vcmptruesd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 10 	vcmpeq_ossd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 10 	vcmpeq_ossd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 10 	vcmpeq_ossd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 10 	vcmpeq_ossd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 10 	vcmpeq_ossd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 10 	vcmpeq_ossd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 10 	vcmpeq_ossd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 10 	vcmpeq_ossd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 11 	vcmplt_oqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 11 	vcmplt_oqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 11 	vcmplt_oqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 11 	vcmplt_oqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 11 	vcmplt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 11 	vcmplt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 11 	vcmplt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 11 	vcmplt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 12 	vcmple_oqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 12 	vcmple_oqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 12 	vcmple_oqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 12 	vcmple_oqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 12 	vcmple_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 12 	vcmple_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 12 	vcmple_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 12 	vcmple_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 13 	vcmpunord_ssd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 13 	vcmpunord_ssd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 13 	vcmpunord_ssd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 13 	vcmpunord_ssd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 13 	vcmpunord_ssd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 13 	vcmpunord_ssd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 13 	vcmpunord_ssd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 13 	vcmpunord_ssd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 14 	vcmpneq_ussd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 14 	vcmpneq_ussd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 14 	vcmpneq_ussd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 14 	vcmpneq_ussd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 14 	vcmpneq_ussd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 14 	vcmpneq_ussd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 14 	vcmpneq_ussd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 14 	vcmpneq_ussd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 15 	vcmpnlt_uqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 15 	vcmpnlt_uqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 15 	vcmpnlt_uqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 15 	vcmpnlt_uqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 15 	vcmpnlt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 15 	vcmpnlt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 15 	vcmpnlt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 15 	vcmpnlt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 16 	vcmpnle_uqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 16 	vcmpnle_uqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 16 	vcmpnle_uqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 16 	vcmpnle_uqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 16 	vcmpnle_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 16 	vcmpnle_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 16 	vcmpnle_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 16 	vcmpnle_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 17 	vcmpord_ssd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 17 	vcmpord_ssd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 17 	vcmpord_ssd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 17 	vcmpord_ssd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 17 	vcmpord_ssd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 17 	vcmpord_ssd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 17 	vcmpord_ssd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 17 	vcmpord_ssd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 18 	vcmpeq_ussd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 18 	vcmpeq_ussd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 18 	vcmpeq_ussd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 18 	vcmpeq_ussd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 18 	vcmpeq_ussd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 18 	vcmpeq_ussd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 18 	vcmpeq_ussd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 18 	vcmpeq_ussd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 19 	vcmpnge_uqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 19 	vcmpnge_uqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 19 	vcmpnge_uqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 19 	vcmpnge_uqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 19 	vcmpnge_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 19 	vcmpnge_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 19 	vcmpnge_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 19 	vcmpnge_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 1a 	vcmpngt_uqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1a 	vcmpngt_uqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 1a 	vcmpngt_uqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 1a 	vcmpngt_uqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 1a 	vcmpngt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 1a 	vcmpngt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 1a 	vcmpngt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 1a 	vcmpngt_uqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 1b 	vcmpfalse_ossd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1b 	vcmpfalse_ossd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 1b 	vcmpfalse_ossd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 1b 	vcmpfalse_ossd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 1b 	vcmpfalse_ossd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 1b 	vcmpfalse_ossd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 1b 	vcmpfalse_ossd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 1b 	vcmpfalse_ossd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 1c 	vcmpneq_ossd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1c 	vcmpneq_ossd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 1c 	vcmpneq_ossd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 1c 	vcmpneq_ossd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 1c 	vcmpneq_ossd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 1c 	vcmpneq_ossd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 1c 	vcmpneq_ossd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 1c 	vcmpneq_ossd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 1d 	vcmpge_oqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1d 	vcmpge_oqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 1d 	vcmpge_oqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 1d 	vcmpge_oqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 1d 	vcmpge_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 1d 	vcmpge_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 1d 	vcmpge_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 1d 	vcmpge_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 1e 	vcmpgt_oqsd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1e 	vcmpgt_oqsd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 1e 	vcmpgt_oqsd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 1e 	vcmpgt_oqsd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 1e 	vcmpgt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 1e 	vcmpgt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 1e 	vcmpgt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 1e 	vcmpgt_oqsd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ec 1f 	vcmptrue_ussd k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1f 	vcmptrue_ussd k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 29 1f 	vcmptrue_ussd k5\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 ac f4 c0 1d fe ff 1f 	vcmptrue_ussd k5\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 7f 1f 	vcmptrue_ussd k5\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa 00 04 00 00 1f 	vcmptrue_ussd k5\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 6a 80 1f 	vcmptrue_ussd k5\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f c2 aa f8 fb ff ff 1f 	vcmptrue_ussd k5\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec ab 	vcmpss k5\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec ab 	vcmpss k5\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 7b 	vcmpss k5\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 7b 	vcmpss k5\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 7b 	vcmpss k5\{k7\},xmm5,DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 7b 	vcmpss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 7b 	vcmpss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 7b 	vcmpss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 7b 	vcmpss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 7b 	vcmpss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 00 	vcmpeqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 00 	vcmpeqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 00 	vcmpeqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 00 	vcmpeqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 00 	vcmpeqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 01 	vcmpltss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 01 	vcmpltss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 01 	vcmpltss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 01 	vcmpltss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 01 	vcmpltss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 02 	vcmpless k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 02 	vcmpless k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 02 	vcmpless k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 02 	vcmpless k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 02 	vcmpless k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 03 	vcmpunordss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 03 	vcmpunordss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 03 	vcmpunordss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 03 	vcmpunordss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 03 	vcmpunordss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 04 	vcmpneqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 04 	vcmpneqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 04 	vcmpneqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 04 	vcmpneqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 04 	vcmpneqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 05 	vcmpnltss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 05 	vcmpnltss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 05 	vcmpnltss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 05 	vcmpnltss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 05 	vcmpnltss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 06 	vcmpnless k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 06 	vcmpnless k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 06 	vcmpnless k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 06 	vcmpnless k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 06 	vcmpnless k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 07 	vcmpordss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 07 	vcmpordss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 07 	vcmpordss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 07 	vcmpordss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 07 	vcmpordss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 08 	vcmpeq_uqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 08 	vcmpeq_uqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 08 	vcmpeq_uqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 08 	vcmpeq_uqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 08 	vcmpeq_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 08 	vcmpeq_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 08 	vcmpeq_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 08 	vcmpeq_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 09 	vcmpngess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 09 	vcmpngess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 09 	vcmpngess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 09 	vcmpngess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 09 	vcmpngess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0a 	vcmpngtss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0a 	vcmpngtss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0a 	vcmpngtss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0a 	vcmpngtss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0a 	vcmpngtss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0b 	vcmpfalsess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0b 	vcmpfalsess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0b 	vcmpfalsess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0b 	vcmpfalsess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0b 	vcmpfalsess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0c 	vcmpneq_oqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0c 	vcmpneq_oqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0c 	vcmpneq_oqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0c 	vcmpneq_oqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0c 	vcmpneq_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0c 	vcmpneq_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0c 	vcmpneq_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0c 	vcmpneq_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0d 	vcmpgess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0d 	vcmpgess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0d 	vcmpgess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0d 	vcmpgess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0d 	vcmpgess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0e 	vcmpgtss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0e 	vcmpgtss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0e 	vcmpgtss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0e 	vcmpgtss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0e 	vcmpgtss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0f 	vcmptruess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0f 	vcmptruess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 0f 	vcmptruess k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0f 	vcmptruess k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 0f 	vcmptruess k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 10 	vcmpeq_osss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 10 	vcmpeq_osss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 10 	vcmpeq_osss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 10 	vcmpeq_osss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 10 	vcmpeq_osss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 10 	vcmpeq_osss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 10 	vcmpeq_osss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 10 	vcmpeq_osss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 11 	vcmplt_oqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 11 	vcmplt_oqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 11 	vcmplt_oqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 11 	vcmplt_oqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 11 	vcmplt_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 11 	vcmplt_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 11 	vcmplt_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 11 	vcmplt_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 12 	vcmple_oqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 12 	vcmple_oqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 12 	vcmple_oqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 12 	vcmple_oqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 12 	vcmple_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 12 	vcmple_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 12 	vcmple_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 12 	vcmple_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 13 	vcmpunord_sss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 13 	vcmpunord_sss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 13 	vcmpunord_sss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 13 	vcmpunord_sss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 13 	vcmpunord_sss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 13 	vcmpunord_sss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 13 	vcmpunord_sss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 13 	vcmpunord_sss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 14 	vcmpneq_usss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 14 	vcmpneq_usss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 14 	vcmpneq_usss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 14 	vcmpneq_usss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 14 	vcmpneq_usss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 14 	vcmpneq_usss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 14 	vcmpneq_usss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 14 	vcmpneq_usss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 15 	vcmpnlt_uqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 15 	vcmpnlt_uqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 15 	vcmpnlt_uqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 15 	vcmpnlt_uqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 15 	vcmpnlt_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 15 	vcmpnlt_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 15 	vcmpnlt_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 15 	vcmpnlt_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 16 	vcmpnle_uqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 16 	vcmpnle_uqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 16 	vcmpnle_uqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 16 	vcmpnle_uqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 16 	vcmpnle_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 16 	vcmpnle_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 16 	vcmpnle_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 16 	vcmpnle_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 17 	vcmpord_sss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 17 	vcmpord_sss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 17 	vcmpord_sss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 17 	vcmpord_sss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 17 	vcmpord_sss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 17 	vcmpord_sss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 17 	vcmpord_sss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 17 	vcmpord_sss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 18 	vcmpeq_usss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 18 	vcmpeq_usss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 18 	vcmpeq_usss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 18 	vcmpeq_usss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 18 	vcmpeq_usss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 18 	vcmpeq_usss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 18 	vcmpeq_usss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 18 	vcmpeq_usss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 19 	vcmpnge_uqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 19 	vcmpnge_uqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 19 	vcmpnge_uqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 19 	vcmpnge_uqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 19 	vcmpnge_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 19 	vcmpnge_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 19 	vcmpnge_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 19 	vcmpnge_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 1a 	vcmpngt_uqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1a 	vcmpngt_uqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 1a 	vcmpngt_uqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 1a 	vcmpngt_uqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 1a 	vcmpngt_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 1a 	vcmpngt_uqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 1a 	vcmpngt_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 1a 	vcmpngt_uqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 1b 	vcmpfalse_osss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1b 	vcmpfalse_osss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 1b 	vcmpfalse_osss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 1b 	vcmpfalse_osss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 1b 	vcmpfalse_osss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 1b 	vcmpfalse_osss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 1b 	vcmpfalse_osss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 1b 	vcmpfalse_osss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 1c 	vcmpneq_osss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1c 	vcmpneq_osss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 1c 	vcmpneq_osss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 1c 	vcmpneq_osss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 1c 	vcmpneq_osss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 1c 	vcmpneq_osss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 1c 	vcmpneq_osss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 1c 	vcmpneq_osss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 1d 	vcmpge_oqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1d 	vcmpge_oqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 1d 	vcmpge_oqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 1d 	vcmpge_oqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 1d 	vcmpge_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 1d 	vcmpge_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 1d 	vcmpge_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 1d 	vcmpge_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 1e 	vcmpgt_oqss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1e 	vcmpgt_oqss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 1e 	vcmpgt_oqss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 1e 	vcmpgt_oqss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 1e 	vcmpgt_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 1e 	vcmpgt_oqss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 1e 	vcmpgt_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 1e 	vcmpgt_oqss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ec 1f 	vcmptrue_usss k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1f 	vcmptrue_usss k5\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 29 1f 	vcmptrue_usss k5\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 ac f4 c0 1d fe ff 1f 	vcmptrue_usss k5\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 7f 1f 	vcmptrue_usss k5\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa 00 02 00 00 1f 	vcmptrue_usss k5\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 6a 80 1f 	vcmptrue_usss k5\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f c2 aa fc fd ff ff 1f 	vcmptrue_usss k5\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 fd 18 2f f5    	vcomisd xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7c 18 2f f5    	vcomiss xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 18 2d c6    	vcvtsd2si eax,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 58 2d c6    	vcvtsd2si eax,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 38 2d c6    	vcvtsd2si eax,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 78 2d c6    	vcvtsd2si eax,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 18 2d ee    	vcvtsd2si ebp,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 58 2d ee    	vcvtsd2si ebp,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 38 2d ee    	vcvtsd2si ebp,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 78 2d ee    	vcvtsd2si ebp,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a f4    	vcvtsd2ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 5a f4    	vcvtsd2ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5a f4    	vcvtsd2ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 5a f4    	vcvtsd2ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 5a f4    	vcvtsd2ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 5a f4    	vcvtsd2ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a 31    	vcvtsd2ss xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a b4 f4 c0 1d fe ff 	vcvtsd2ss xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a 72 7f 	vcvtsd2ss xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a b2 00 04 00 00 	vcvtsd2ss xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a 72 80 	vcvtsd2ss xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5a b2 f8 fb ff ff 	vcvtsd2ss xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 18 2a f0    	vcvtsi2ss xmm6,xmm5,eax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 58 2a f0    	vcvtsi2ss xmm6,xmm5,eax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 38 2a f0    	vcvtsi2ss xmm6,xmm5,eax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 78 2a f0    	vcvtsi2ss xmm6,xmm5,eax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 18 2a f5    	vcvtsi2ss xmm6,xmm5,ebp\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 58 2a f5    	vcvtsi2ss xmm6,xmm5,ebp\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 38 2a f5    	vcvtsi2ss xmm6,xmm5,ebp\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 78 2a f5    	vcvtsi2ss xmm6,xmm5,ebp\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a f4    	vcvtss2sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 5a f4    	vcvtss2sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 5a f4    	vcvtss2sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a 31    	vcvtss2sd xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a b4 f4 c0 1d fe ff 	vcvtss2sd xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a 72 7f 	vcvtss2sd xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a b2 00 02 00 00 	vcvtss2sd xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a 72 80 	vcvtss2sd xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5a b2 fc fd ff ff 	vcvtss2sd xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 7e 18 2d c6    	vcvtss2si eax,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 58 2d c6    	vcvtss2si eax,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 38 2d c6    	vcvtss2si eax,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 78 2d c6    	vcvtss2si eax,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 18 2d ee    	vcvtss2si ebp,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 58 2d ee    	vcvtss2si ebp,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 38 2d ee    	vcvtss2si ebp,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 78 2d ee    	vcvtss2si ebp,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 18 2c c6    	vcvttsd2si eax,xmm6\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 18 2c ee    	vcvttsd2si ebp,xmm6\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 18 2c c6    	vcvttss2si eax,xmm6\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 18 2c ee    	vcvttss2si ebp,xmm6\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e f4    	vdivsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 5e f4    	vdivsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5e f4    	vdivsd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 5e f4    	vdivsd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 5e f4    	vdivsd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 5e f4    	vdivsd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e 31    	vdivsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e b4 f4 c0 1d fe ff 	vdivsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e 72 7f 	vdivsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e b2 00 04 00 00 	vdivsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e 72 80 	vdivsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5e b2 f8 fb ff ff 	vdivsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e f4    	vdivss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 5e f4    	vdivss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 5e f4    	vdivss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 5e f4    	vdivss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 5e f4    	vdivss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 5e f4    	vdivss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e 31    	vdivss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e b4 f4 c0 1d fe ff 	vdivss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e 72 7f 	vdivss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e b2 00 02 00 00 	vdivss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e 72 80 	vdivss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5e b2 fc fd ff ff 	vdivss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 f4    	vfmadd132sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 99 f4    	vfmadd132sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f 99 f4    	vfmadd132sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 99 f4    	vfmadd132sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 99 f4    	vfmadd132sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 99 f4    	vfmadd132sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 31    	vfmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 b4 f4 c0 1d fe ff 	vfmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 72 7f 	vfmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 b2 00 04 00 00 	vfmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 72 80 	vfmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 99 b2 f8 fb ff ff 	vfmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 f4    	vfmadd132ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 99 f4    	vfmadd132ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f 99 f4    	vfmadd132ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 99 f4    	vfmadd132ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 99 f4    	vfmadd132ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 99 f4    	vfmadd132ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 31    	vfmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 b4 f4 c0 1d fe ff 	vfmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 72 7f 	vfmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 b2 00 02 00 00 	vfmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 72 80 	vfmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 99 b2 fc fd ff ff 	vfmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 f4    	vfmadd213sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af a9 f4    	vfmadd213sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f a9 f4    	vfmadd213sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f a9 f4    	vfmadd213sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f a9 f4    	vfmadd213sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f a9 f4    	vfmadd213sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 31    	vfmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 b4 f4 c0 1d fe ff 	vfmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 72 7f 	vfmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 b2 00 04 00 00 	vfmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 72 80 	vfmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f a9 b2 f8 fb ff ff 	vfmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 f4    	vfmadd213ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af a9 f4    	vfmadd213ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f a9 f4    	vfmadd213ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f a9 f4    	vfmadd213ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f a9 f4    	vfmadd213ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f a9 f4    	vfmadd213ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 31    	vfmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 b4 f4 c0 1d fe ff 	vfmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 72 7f 	vfmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 b2 00 02 00 00 	vfmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 72 80 	vfmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f a9 b2 fc fd ff ff 	vfmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 f4    	vfmadd231sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af b9 f4    	vfmadd231sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f b9 f4    	vfmadd231sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f b9 f4    	vfmadd231sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f b9 f4    	vfmadd231sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f b9 f4    	vfmadd231sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 31    	vfmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 b4 f4 c0 1d fe ff 	vfmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 72 7f 	vfmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 b2 00 04 00 00 	vfmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 72 80 	vfmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f b9 b2 f8 fb ff ff 	vfmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 f4    	vfmadd231ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af b9 f4    	vfmadd231ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f b9 f4    	vfmadd231ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f b9 f4    	vfmadd231ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f b9 f4    	vfmadd231ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f b9 f4    	vfmadd231ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 31    	vfmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 b4 f4 c0 1d fe ff 	vfmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 72 7f 	vfmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 b2 00 02 00 00 	vfmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 72 80 	vfmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f b9 b2 fc fd ff ff 	vfmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b f4    	vfmsub132sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 9b f4    	vfmsub132sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f 9b f4    	vfmsub132sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 9b f4    	vfmsub132sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 9b f4    	vfmsub132sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 9b f4    	vfmsub132sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b 31    	vfmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b b4 f4 c0 1d fe ff 	vfmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b 72 7f 	vfmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b b2 00 04 00 00 	vfmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b 72 80 	vfmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9b b2 f8 fb ff ff 	vfmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b f4    	vfmsub132ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 9b f4    	vfmsub132ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f 9b f4    	vfmsub132ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 9b f4    	vfmsub132ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 9b f4    	vfmsub132ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 9b f4    	vfmsub132ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b 31    	vfmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b b4 f4 c0 1d fe ff 	vfmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b 72 7f 	vfmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b b2 00 02 00 00 	vfmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b 72 80 	vfmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9b b2 fc fd ff ff 	vfmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab f4    	vfmsub213sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af ab f4    	vfmsub213sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f ab f4    	vfmsub213sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f ab f4    	vfmsub213sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f ab f4    	vfmsub213sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f ab f4    	vfmsub213sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab 31    	vfmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab b4 f4 c0 1d fe ff 	vfmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab 72 7f 	vfmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab b2 00 04 00 00 	vfmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab 72 80 	vfmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ab b2 f8 fb ff ff 	vfmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ab f4    	vfmsub213ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af ab f4    	vfmsub213ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f ab f4    	vfmsub213ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f ab f4    	vfmsub213ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f ab f4    	vfmsub213ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f ab f4    	vfmsub213ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f ab 31    	vfmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ab b4 f4 c0 1d fe ff 	vfmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ab 72 7f 	vfmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ab b2 00 02 00 00 	vfmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ab 72 80 	vfmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ab b2 fc fd ff ff 	vfmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb f4    	vfmsub231sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af bb f4    	vfmsub231sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f bb f4    	vfmsub231sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f bb f4    	vfmsub231sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f bb f4    	vfmsub231sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f bb f4    	vfmsub231sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb 31    	vfmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb b4 f4 c0 1d fe ff 	vfmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb 72 7f 	vfmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb b2 00 04 00 00 	vfmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb 72 80 	vfmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bb b2 f8 fb ff ff 	vfmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bb f4    	vfmsub231ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af bb f4    	vfmsub231ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f bb f4    	vfmsub231ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f bb f4    	vfmsub231ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f bb f4    	vfmsub231ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f bb f4    	vfmsub231ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f bb 31    	vfmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bb b4 f4 c0 1d fe ff 	vfmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bb 72 7f 	vfmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bb b2 00 02 00 00 	vfmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bb 72 80 	vfmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bb b2 fc fd ff ff 	vfmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d f4    	vfnmadd132sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 9d f4    	vfnmadd132sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f 9d f4    	vfnmadd132sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 9d f4    	vfnmadd132sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 9d f4    	vfnmadd132sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 9d f4    	vfnmadd132sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d 31    	vfnmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d b4 f4 c0 1d fe ff 	vfnmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d 72 7f 	vfnmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d b2 00 04 00 00 	vfnmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d 72 80 	vfnmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9d b2 f8 fb ff ff 	vfnmadd132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d f4    	vfnmadd132ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 9d f4    	vfnmadd132ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f 9d f4    	vfnmadd132ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 9d f4    	vfnmadd132ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 9d f4    	vfnmadd132ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 9d f4    	vfnmadd132ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d 31    	vfnmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d b4 f4 c0 1d fe ff 	vfnmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d 72 7f 	vfnmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d b2 00 02 00 00 	vfnmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d 72 80 	vfnmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9d b2 fc fd ff ff 	vfnmadd132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad f4    	vfnmadd213sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af ad f4    	vfnmadd213sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f ad f4    	vfnmadd213sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f ad f4    	vfnmadd213sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f ad f4    	vfnmadd213sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f ad f4    	vfnmadd213sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad 31    	vfnmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad b4 f4 c0 1d fe ff 	vfnmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad 72 7f 	vfnmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad b2 00 04 00 00 	vfnmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad 72 80 	vfnmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f ad b2 f8 fb ff ff 	vfnmadd213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ad f4    	vfnmadd213ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af ad f4    	vfnmadd213ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f ad f4    	vfnmadd213ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f ad f4    	vfnmadd213ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f ad f4    	vfnmadd213ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f ad f4    	vfnmadd213ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f ad 31    	vfnmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ad b4 f4 c0 1d fe ff 	vfnmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ad 72 7f 	vfnmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ad b2 00 02 00 00 	vfnmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ad 72 80 	vfnmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f ad b2 fc fd ff ff 	vfnmadd213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd f4    	vfnmadd231sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af bd f4    	vfnmadd231sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f bd f4    	vfnmadd231sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f bd f4    	vfnmadd231sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f bd f4    	vfnmadd231sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f bd f4    	vfnmadd231sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd 31    	vfnmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd b4 f4 c0 1d fe ff 	vfnmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd 72 7f 	vfnmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd b2 00 04 00 00 	vfnmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd 72 80 	vfnmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bd b2 f8 fb ff ff 	vfnmadd231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bd f4    	vfnmadd231ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af bd f4    	vfnmadd231ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f bd f4    	vfnmadd231ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f bd f4    	vfnmadd231ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f bd f4    	vfnmadd231ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f bd f4    	vfnmadd231ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f bd 31    	vfnmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bd b4 f4 c0 1d fe ff 	vfnmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bd 72 7f 	vfnmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bd b2 00 02 00 00 	vfnmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bd 72 80 	vfnmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bd b2 fc fd ff ff 	vfnmadd231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f f4    	vfnmsub132sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 9f f4    	vfnmsub132sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f 9f f4    	vfnmsub132sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 9f f4    	vfnmsub132sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 9f f4    	vfnmsub132sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 9f f4    	vfnmsub132sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f 31    	vfnmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f b4 f4 c0 1d fe ff 	vfnmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f 72 7f 	vfnmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f b2 00 04 00 00 	vfnmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f 72 80 	vfnmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 9f b2 f8 fb ff ff 	vfnmsub132sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f f4    	vfnmsub132ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 9f f4    	vfnmsub132ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f 9f f4    	vfnmsub132ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 9f f4    	vfnmsub132ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 9f f4    	vfnmsub132ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 9f f4    	vfnmsub132ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f 31    	vfnmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f b4 f4 c0 1d fe ff 	vfnmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f 72 7f 	vfnmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f b2 00 02 00 00 	vfnmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f 72 80 	vfnmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 9f b2 fc fd ff ff 	vfnmsub132ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f af f4    	vfnmsub213sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af af f4    	vfnmsub213sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f af f4    	vfnmsub213sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f af f4    	vfnmsub213sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f af f4    	vfnmsub213sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f af f4    	vfnmsub213sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f af 31    	vfnmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f af b4 f4 c0 1d fe ff 	vfnmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f af 72 7f 	vfnmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f af b2 00 04 00 00 	vfnmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f af 72 80 	vfnmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f af b2 f8 fb ff ff 	vfnmsub213sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f af f4    	vfnmsub213ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af af f4    	vfnmsub213ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f af f4    	vfnmsub213ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f af f4    	vfnmsub213ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f af f4    	vfnmsub213ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f af f4    	vfnmsub213ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f af 31    	vfnmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f af b4 f4 c0 1d fe ff 	vfnmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f af 72 7f 	vfnmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f af b2 00 02 00 00 	vfnmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f af 72 80 	vfnmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f af b2 fc fd ff ff 	vfnmsub213ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf f4    	vfnmsub231sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af bf f4    	vfnmsub231sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f bf f4    	vfnmsub231sd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f bf f4    	vfnmsub231sd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f bf f4    	vfnmsub231sd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f bf f4    	vfnmsub231sd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf 31    	vfnmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf b4 f4 c0 1d fe ff 	vfnmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf 72 7f 	vfnmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf b2 00 04 00 00 	vfnmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf 72 80 	vfnmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f bf b2 f8 fb ff ff 	vfnmsub231sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bf f4    	vfnmsub231ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af bf f4    	vfnmsub231ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f bf f4    	vfnmsub231ss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f bf f4    	vfnmsub231ss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f bf f4    	vfnmsub231ss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f bf f4    	vfnmsub231ss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f bf 31    	vfnmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bf b4 f4 c0 1d fe ff 	vfnmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bf 72 7f 	vfnmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bf b2 00 02 00 00 	vfnmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bf 72 80 	vfnmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f bf b2 fc fd ff ff 	vfnmsub231ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 f4    	vgetexpsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 43 f4    	vgetexpsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f 43 f4    	vgetexpsd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 31    	vgetexpsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 b4 f4 c0 1d fe ff 	vgetexpsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 72 7f 	vgetexpsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 b2 00 04 00 00 	vgetexpsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 72 80 	vgetexpsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 43 b2 f8 fb ff ff 	vgetexpsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 f4    	vgetexpss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 43 f4    	vgetexpss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f 43 f4    	vgetexpss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 31    	vgetexpss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 b4 f4 c0 1d fe ff 	vgetexpss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 72 7f 	vgetexpss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 b2 00 02 00 00 	vgetexpss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 72 80 	vgetexpss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 43 b2 fc fd ff ff 	vgetexpss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 f4 ab 	vgetmantsd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 d5 af 27 f4 ab 	vgetmantsd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 d5 1f 27 f4 ab 	vgetmantsd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 f4 7b 	vgetmantsd xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 1f 27 f4 7b 	vgetmantsd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 31 7b 	vgetmantsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 b4 f4 c0 1d fe ff 7b 	vgetmantsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 72 7f 7b 	vgetmantsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 b2 00 04 00 00 7b 	vgetmantsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 72 80 7b 	vgetmantsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 27 b2 f8 fb ff ff 7b 	vgetmantsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 f4 ab 	vgetmantss xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 55 af 27 f4 ab 	vgetmantss xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 55 1f 27 f4 ab 	vgetmantss xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 f4 7b 	vgetmantss xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 55 1f 27 f4 7b 	vgetmantss xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 31 7b 	vgetmantss xmm6\{k7\},xmm5,DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 b4 f4 c0 1d fe ff 7b 	vgetmantss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 72 7f 7b 	vgetmantss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 b2 00 02 00 00 7b 	vgetmantss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 72 80 7b 	vgetmantss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 27 b2 fc fd ff ff 7b 	vgetmantss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f f4    	vmaxsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 5f f4    	vmaxsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5f f4    	vmaxsd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f 31    	vmaxsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f b4 f4 c0 1d fe ff 	vmaxsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f 72 7f 	vmaxsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f b2 00 04 00 00 	vmaxsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f 72 80 	vmaxsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5f b2 f8 fb ff ff 	vmaxsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f f4    	vmaxss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 5f f4    	vmaxss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 5f f4    	vmaxss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f 31    	vmaxss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f b4 f4 c0 1d fe ff 	vmaxss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f 72 7f 	vmaxss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f b2 00 02 00 00 	vmaxss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f 72 80 	vmaxss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5f b2 fc fd ff ff 	vmaxss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d f4    	vminsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 5d f4    	vminsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5d f4    	vminsd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d 31    	vminsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d b4 f4 c0 1d fe ff 	vminsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d 72 7f 	vminsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d b2 00 04 00 00 	vminsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d 72 80 	vminsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5d b2 f8 fb ff ff 	vminsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d f4    	vminss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 5d f4    	vminss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 5d f4    	vminss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d 31    	vminss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d b4 f4 c0 1d fe ff 	vminss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d 72 7f 	vminss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d b2 00 02 00 00 	vminss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d 72 80 	vminss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5d b2 fc fd ff ff 	vminss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 10 31    	vmovsd xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 ff af 10 31    	vmovsd xmm6\{k7\}\{z\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 10 b4 f4 c0 1d fe ff 	vmovsd xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 10 72 7f 	vmovsd xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 10 b2 00 04 00 00 	vmovsd xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 10 72 80 	vmovsd xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 10 b2 f8 fb ff ff 	vmovsd xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 ff 2f 11 31    	vmovsd QWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 ff 2f 11 b4 f4 c0 1d fe ff 	vmovsd QWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 ff 2f 11 72 7f 	vmovsd QWORD PTR \[edx\+0x3f8\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 ff 2f 11 b2 00 04 00 00 	vmovsd QWORD PTR \[edx\+0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 ff 2f 11 72 80 	vmovsd QWORD PTR \[edx-0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 ff 2f 11 b2 f8 fb ff ff 	vmovsd QWORD PTR \[edx-0x408\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 d7 2f 10 f4    	vmovsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 10 f4    	vmovsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 7e 2f 10 31    	vmovss xmm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 7e af 10 31    	vmovss xmm6\{k7\}\{z\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 7e 2f 10 b4 f4 c0 1d fe ff 	vmovss xmm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 7e 2f 10 72 7f 	vmovss xmm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 7e 2f 10 b2 00 02 00 00 	vmovss xmm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 2f 10 72 80 	vmovss xmm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 2f 10 b2 fc fd ff ff 	vmovss xmm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 7e 2f 11 31    	vmovss DWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 2f 11 b4 f4 c0 1d fe ff 	vmovss DWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 2f 11 72 7f 	vmovss DWORD PTR \[edx\+0x1fc\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 2f 11 b2 00 02 00 00 	vmovss DWORD PTR \[edx\+0x200\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 2f 11 72 80 	vmovss DWORD PTR \[edx-0x200\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 2f 11 b2 fc fd ff ff 	vmovss DWORD PTR \[edx-0x204\]\{k7\},xmm6
[ 	]*[a-f0-9]+:	62 f1 56 2f 10 f4    	vmovss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 10 f4    	vmovss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 f4    	vmulsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 59 f4    	vmulsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 59 f4    	vmulsd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 59 f4    	vmulsd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 59 f4    	vmulsd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 59 f4    	vmulsd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 31    	vmulsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 b4 f4 c0 1d fe ff 	vmulsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 72 7f 	vmulsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 b2 00 04 00 00 	vmulsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 72 80 	vmulsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 59 b2 f8 fb ff ff 	vmulsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 f4    	vmulss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 59 f4    	vmulss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 59 f4    	vmulss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 59 f4    	vmulss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 59 f4    	vmulss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 59 f4    	vmulss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 31    	vmulss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 b4 f4 c0 1d fe ff 	vmulss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 72 7f 	vmulss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 b2 00 02 00 00 	vmulss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 72 80 	vmulss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 59 b2 fc fd ff ff 	vmulss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d f4    	vrcp14sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 4d f4    	vrcp14sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d 31    	vrcp14sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d b4 f4 c0 1d fe ff 	vrcp14sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d 72 7f 	vrcp14sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d b2 00 04 00 00 	vrcp14sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d 72 80 	vrcp14sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4d b2 f8 fb ff ff 	vrcp14sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d f4    	vrcp14ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 4d f4    	vrcp14ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d 31    	vrcp14ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d b4 f4 c0 1d fe ff 	vrcp14ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d 72 7f 	vrcp14ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d b2 00 02 00 00 	vrcp14ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d 72 80 	vrcp14ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4d b2 fc fd ff ff 	vrcp14ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cb f4    	vrcp28ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af cb f4    	vrcp28ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f cb f4    	vrcp28ss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f cb 31    	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cb b4 f4 c0 1d fe ff 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cb 72 7f 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cb b2 00 02 00 00 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cb 72 80 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cb b2 fc fd ff ff 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb f4    	vrcp28sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af cb f4    	vrcp28sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f cb f4    	vrcp28sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb 31    	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb b4 f4 c0 1d fe ff 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb 72 7f 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb b2 00 04 00 00 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb 72 80 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cb b2 f8 fb ff ff 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f f4    	vrsqrt14sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 4f f4    	vrsqrt14sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f 31    	vrsqrt14sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f b4 f4 c0 1d fe ff 	vrsqrt14sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f 72 7f 	vrsqrt14sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f b2 00 04 00 00 	vrsqrt14sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f 72 80 	vrsqrt14sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 4f b2 f8 fb ff ff 	vrsqrt14sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f f4    	vrsqrt14ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 4f f4    	vrsqrt14ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f 31    	vrsqrt14ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f b4 f4 c0 1d fe ff 	vrsqrt14ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f 72 7f 	vrsqrt14ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f b2 00 02 00 00 	vrsqrt14ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f 72 80 	vrsqrt14ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 4f b2 fc fd ff ff 	vrsqrt14ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cd f4    	vrsqrt28ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af cd f4    	vrsqrt28ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f cd f4    	vrsqrt28ss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f cd 31    	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cd b4 f4 c0 1d fe ff 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cd 72 7f 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cd b2 00 02 00 00 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cd 72 80 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f cd b2 fc fd ff ff 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd f4    	vrsqrt28sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af cd f4    	vrsqrt28sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f cd f4    	vrsqrt28sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd 31    	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd b4 f4 c0 1d fe ff 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd 72 7f 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd b2 00 04 00 00 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd 72 80 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f cd b2 f8 fb ff ff 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 f4    	vsqrtsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 51 f4    	vsqrtsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 51 f4    	vsqrtsd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 51 f4    	vsqrtsd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 51 f4    	vsqrtsd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 51 f4    	vsqrtsd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 31    	vsqrtsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 b4 f4 c0 1d fe ff 	vsqrtsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 72 7f 	vsqrtsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 b2 00 04 00 00 	vsqrtsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 72 80 	vsqrtsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 51 b2 f8 fb ff ff 	vsqrtsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 f4    	vsqrtss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 51 f4    	vsqrtss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 51 f4    	vsqrtss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 51 f4    	vsqrtss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 51 f4    	vsqrtss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 51 f4    	vsqrtss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 31    	vsqrtss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 b4 f4 c0 1d fe ff 	vsqrtss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 72 7f 	vsqrtss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 b2 00 02 00 00 	vsqrtss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 72 80 	vsqrtss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 51 b2 fc fd ff ff 	vsqrtss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c f4    	vsubsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 af 5c f4    	vsubsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5c f4    	vsubsd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 5c f4    	vsubsd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 5c f4    	vsubsd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 5c f4    	vsubsd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c 31    	vsubsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c b4 f4 c0 1d fe ff 	vsubsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c 72 7f 	vsubsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c b2 00 04 00 00 	vsubsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c 72 80 	vsubsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 d7 2f 5c b2 f8 fb ff ff 	vsubsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c f4    	vsubss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 af 5c f4    	vsubss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f1 56 1f 5c f4    	vsubss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 5c f4    	vsubss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 5c f4    	vsubss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 5c f4    	vsubss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c 31    	vsubss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c b4 f4 c0 1d fe ff 	vsubss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c 72 7f 	vsubss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c b2 00 02 00 00 	vsubss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c 72 80 	vsubss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 2f 5c b2 fc fd ff ff 	vsubss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 fd 18 2e f5    	vucomisd xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7c 18 2e f5    	vucomiss xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 c6    	vcvtsd2usi eax,xmm6
[ 	]*[a-f0-9]+:	62 f1 7f 18 79 c6    	vcvtsd2usi eax,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 58 79 c6    	vcvtsd2usi eax,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 38 79 c6    	vcvtsd2usi eax,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 78 79 c6    	vcvtsd2usi eax,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 01    	vcvtsd2usi eax,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 84 f4 c0 1d fe ff 	vcvtsd2usi eax,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 42 7f 	vcvtsd2usi eax,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 82 00 04 00 00 	vcvtsd2usi eax,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 42 80 	vcvtsd2usi eax,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 82 f8 fb ff ff 	vcvtsd2usi eax,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 ee    	vcvtsd2usi ebp,xmm6
[ 	]*[a-f0-9]+:	62 f1 7f 18 79 ee    	vcvtsd2usi ebp,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 58 79 ee    	vcvtsd2usi ebp,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 38 79 ee    	vcvtsd2usi ebp,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 78 79 ee    	vcvtsd2usi ebp,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 29    	vcvtsd2usi ebp,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 ac f4 c0 1d fe ff 	vcvtsd2usi ebp,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 6a 7f 	vcvtsd2usi ebp,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 aa 00 04 00 00 	vcvtsd2usi ebp,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 6a 80 	vcvtsd2usi ebp,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 28 79 aa f8 fb ff ff 	vcvtsd2usi ebp,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 c6    	vcvtss2usi eax,xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 18 79 c6    	vcvtss2usi eax,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 58 79 c6    	vcvtss2usi eax,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 38 79 c6    	vcvtss2usi eax,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 78 79 c6    	vcvtss2usi eax,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 01    	vcvtss2usi eax,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 84 f4 c0 1d fe ff 	vcvtss2usi eax,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 42 7f 	vcvtss2usi eax,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 82 00 02 00 00 	vcvtss2usi eax,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 42 80 	vcvtss2usi eax,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 82 fc fd ff ff 	vcvtss2usi eax,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 ee    	vcvtss2usi ebp,xmm6
[ 	]*[a-f0-9]+:	62 f1 7e 18 79 ee    	vcvtss2usi ebp,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 58 79 ee    	vcvtss2usi ebp,xmm6\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 38 79 ee    	vcvtss2usi ebp,xmm6\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 78 79 ee    	vcvtss2usi ebp,xmm6\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 29    	vcvtss2usi ebp,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 ac f4 c0 1d fe ff 	vcvtss2usi ebp,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 6a 7f 	vcvtss2usi ebp,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 aa 00 02 00 00 	vcvtss2usi ebp,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 6a 80 	vcvtss2usi ebp,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 28 79 aa fc fd ff ff 	vcvtss2usi ebp,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 57 28 7b f0    	vcvtusi2sd xmm6,xmm5,eax
[ 	]*[a-f0-9]+:	62 f1 57 28 7b f5    	vcvtusi2sd xmm6,xmm5,ebp
[ 	]*[a-f0-9]+:	62 f1 57 28 7b 31    	vcvtusi2sd xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 57 28 7b b4 f4 c0 1d fe ff 	vcvtusi2sd xmm6,xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 57 28 7b 72 7f 	vcvtusi2sd xmm6,xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 57 28 7b b2 00 02 00 00 	vcvtusi2sd xmm6,xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 57 28 7b 72 80 	vcvtusi2sd xmm6,xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 57 28 7b b2 fc fd ff ff 	vcvtusi2sd xmm6,xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f1 56 28 7b f0    	vcvtusi2ss xmm6,xmm5,eax
[ 	]*[a-f0-9]+:	62 f1 56 18 7b f0    	vcvtusi2ss xmm6,xmm5,eax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 58 7b f0    	vcvtusi2ss xmm6,xmm5,eax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 38 7b f0    	vcvtusi2ss xmm6,xmm5,eax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 78 7b f0    	vcvtusi2ss xmm6,xmm5,eax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 28 7b f5    	vcvtusi2ss xmm6,xmm5,ebp
[ 	]*[a-f0-9]+:	62 f1 56 18 7b f5    	vcvtusi2ss xmm6,xmm5,ebp\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 58 7b f5    	vcvtusi2ss xmm6,xmm5,ebp\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 38 7b f5    	vcvtusi2ss xmm6,xmm5,ebp\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 78 7b f5    	vcvtusi2ss xmm6,xmm5,ebp\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 56 28 7b 31    	vcvtusi2ss xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f1 56 28 7b b4 f4 c0 1d fe ff 	vcvtusi2ss xmm6,xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f1 56 28 7b 72 7f 	vcvtusi2ss xmm6,xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 56 28 7b b2 00 02 00 00 	vcvtusi2ss xmm6,xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 28 7b 72 80 	vcvtusi2ss xmm6,xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 56 28 7b b2 fc fd ff ff 	vcvtusi2ss xmm6,xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d f4    	vscalefsd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 af 2d f4    	vscalefsd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f 2d f4    	vscalefsd xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 2d f4    	vscalefsd xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 2d f4    	vscalefsd xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 2d f4    	vscalefsd xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d 31    	vscalefsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d b4 f4 c0 1d fe ff 	vscalefsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d 72 7f 	vscalefsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d b2 00 04 00 00 	vscalefsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d 72 80 	vscalefsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 2f 2d b2 f8 fb ff ff 	vscalefsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d f4    	vscalefss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 af 2d f4    	vscalefss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f 2d f4    	vscalefss xmm6\{k7\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 2d f4    	vscalefss xmm6\{k7\},xmm5,xmm4\{ru-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 2d f4    	vscalefss xmm6\{k7\},xmm5,xmm4\{rd-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 2d f4    	vscalefss xmm6\{k7\},xmm5,xmm4\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d 31    	vscalefss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d b4 f4 c0 1d fe ff 	vscalefss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d 72 7f 	vscalefss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d b2 00 02 00 00 	vscalefss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d 72 80 	vscalefss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 2f 2d b2 fc fd ff ff 	vscalefss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 f4 ab 	vfixupimmss xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 55 af 55 f4 ab 	vfixupimmss xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 55 1f 55 f4 ab 	vfixupimmss xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 f4 7b 	vfixupimmss xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 55 1f 55 f4 7b 	vfixupimmss xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 31 7b 	vfixupimmss xmm6\{k7\},xmm5,DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 b4 f4 c0 1d fe ff 7b 	vfixupimmss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 72 7f 7b 	vfixupimmss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 b2 00 02 00 00 7b 	vfixupimmss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 72 80 7b 	vfixupimmss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 55 b2 fc fd ff ff 7b 	vfixupimmss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 f4 ab 	vfixupimmsd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 d5 af 55 f4 ab 	vfixupimmsd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 d5 1f 55 f4 ab 	vfixupimmsd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 f4 7b 	vfixupimmsd xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 1f 55 f4 7b 	vfixupimmsd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 31 7b 	vfixupimmsd xmm6\{k7\},xmm5,QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 b4 f4 c0 1d fe ff 7b 	vfixupimmsd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 72 7f 7b 	vfixupimmsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 b2 00 04 00 00 7b 	vfixupimmsd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 72 80 7b 	vfixupimmsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 55 b2 f8 fb ff ff 7b 	vfixupimmsd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b f4 ab 	vrndscalesd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 d5 af 0b f4 ab 	vrndscalesd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 d5 1f 0b f4 ab 	vrndscalesd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b f4 7b 	vrndscalesd xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 1f 0b f4 7b 	vrndscalesd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b 31 7b 	vrndscalesd xmm6\{k7\},xmm5,QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b b4 f4 c0 1d fe ff 7b 	vrndscalesd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b 72 7f 7b 	vrndscalesd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b b2 00 04 00 00 7b 	vrndscalesd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b 72 80 7b 	vrndscalesd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f3 d5 2f 0b b2 f8 fb ff ff 7b 	vrndscalesd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a f4 ab 	vrndscaless xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 55 af 0a f4 ab 	vrndscaless xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	62 f3 55 1f 0a f4 ab 	vrndscaless xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a f4 7b 	vrndscaless xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 55 1f 0a f4 7b 	vrndscaless xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a 31 7b 	vrndscaless xmm6\{k7\},xmm5,DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a b4 f4 c0 1d fe ff 7b 	vrndscaless xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a 72 7f 7b 	vrndscaless xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a b2 00 02 00 00 7b 	vrndscaless xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a 72 80 7b 	vrndscaless xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f3 55 2f 0a b2 fc fd ff ff 7b 	vrndscaless xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 f3 56 28 c2 ec 7b 	vcmpsh k5,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 56 1f c2 ec 7b 	vcmpsh k5\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f3 56 28 c2 29 7b 	vcmpsh k5,xmm5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 56 2f c2 ac f4 c0 1d fe ff 7b 	vcmpsh k5\{k7\},xmm5,WORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 56 28 c2 69 7f 7b 	vcmpsh k5,xmm5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:	62 f3 56 2f c2 6a 80 7b 	vcmpsh k5\{k7\},xmm5,WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:	62 f3 7c 28 67 ec 7b 	vfpclasssh k5,xmm4,0x7b
[ 	]*[a-f0-9]+:	62 f3 7c 28 67 29 7b 	vfpclasssh k5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	62 f3 7c 2f 67 ac f4 c0 1d fe ff 7b 	vfpclasssh k5\{k7\},WORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	62 f3 7c 28 67 69 7f 7b 	vfpclasssh k5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:	62 f3 7c 2f 67 6a 80 7b 	vfpclasssh k5\{k7\},WORD PTR \[edx-0x100\],0x7b
#pass
