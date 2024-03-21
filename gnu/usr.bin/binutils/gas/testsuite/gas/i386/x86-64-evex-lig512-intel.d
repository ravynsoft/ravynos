#as: -mevexlig=512
#objdump: -dwMintel
#name: x86_64 AVX512 lig512 insns (Intel disassembly)
#source: x86-64-evex-lig.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 01 97 47 58 f4    	vaddsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 58 f4    	vaddsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 58 f4    	vaddsd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 97 57 58 f4    	vaddsd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 97 37 58 f4    	vaddsd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 97 77 58 f4    	vaddsd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 58 31    	vaddsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 58 b4 f0 23 01 00 00 	vaddsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 97 47 58 72 7f 	vaddsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 58 b2 00 04 00 00 	vaddsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 58 72 80 	vaddsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 58 b2 f8 fb ff ff 	vaddsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 58 f4    	vaddss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 58 f4    	vaddss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 58 f4    	vaddss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 16 57 58 f4    	vaddss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 16 37 58 f4    	vaddss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 16 77 58 f4    	vaddss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 58 31    	vaddss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 58 b4 f0 23 01 00 00 	vaddss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 16 47 58 72 7f 	vaddss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 58 b2 00 02 00 00 	vaddss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 58 72 80 	vaddss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 58 b2 fc fd ff ff 	vaddss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec ab 	vcmpsd k5\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec ab 	vcmpsd k5\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 7b 	vcmpsd k5\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 7b 	vcmpsd k5\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 7b 	vcmpsd k5\{k7\},xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 7b 	vcmpsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 7b 	vcmpsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 7b 	vcmpsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 7b 	vcmpsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 7b 	vcmpsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 00 	vcmpeqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 00 	vcmpeqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 00 	vcmpeqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 00 	vcmpeqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 01 	vcmpltsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 01 	vcmpltsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 01 	vcmpltsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 01 	vcmpltsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 02 	vcmplesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 02 	vcmplesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 02 	vcmplesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 02 	vcmplesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 03 	vcmpunordsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 03 	vcmpunordsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 03 	vcmpunordsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 03 	vcmpunordsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 04 	vcmpneqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 04 	vcmpneqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 04 	vcmpneqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 04 	vcmpneqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 05 	vcmpnltsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 05 	vcmpnltsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 05 	vcmpnltsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 05 	vcmpnltsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 06 	vcmpnlesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 06 	vcmpnlesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 06 	vcmpnlesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 06 	vcmpnlesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 07 	vcmpordsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 07 	vcmpordsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 07 	vcmpordsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 07 	vcmpordsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 08 	vcmpeq_uqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 08 	vcmpeq_uqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 08 	vcmpeq_uqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 08 	vcmpeq_uqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 08 	vcmpeq_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 08 	vcmpeq_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 08 	vcmpeq_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 08 	vcmpeq_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 09 	vcmpngesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 09 	vcmpngesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 09 	vcmpngesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 09 	vcmpngesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0a 	vcmpngtsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0a 	vcmpngtsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0a 	vcmpngtsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0a 	vcmpngtsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0c 	vcmpneq_oqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0c 	vcmpneq_oqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0c 	vcmpneq_oqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0c 	vcmpneq_oqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0c 	vcmpneq_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0c 	vcmpneq_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0c 	vcmpneq_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0c 	vcmpneq_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0d 	vcmpgesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0d 	vcmpgesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0d 	vcmpgesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0d 	vcmpgesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0e 	vcmpgtsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0e 	vcmpgtsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0e 	vcmpgtsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0e 	vcmpgtsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0f 	vcmptruesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0f 	vcmptruesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0f 	vcmptruesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0f 	vcmptruesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 10 	vcmpeq_ossd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 10 	vcmpeq_ossd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 10 	vcmpeq_ossd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 10 	vcmpeq_ossd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 10 	vcmpeq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 10 	vcmpeq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 10 	vcmpeq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 10 	vcmpeq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 11 	vcmplt_oqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 11 	vcmplt_oqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 11 	vcmplt_oqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 11 	vcmplt_oqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 11 	vcmplt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 11 	vcmplt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 11 	vcmplt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 11 	vcmplt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 12 	vcmple_oqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 12 	vcmple_oqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 12 	vcmple_oqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 12 	vcmple_oqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 12 	vcmple_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 12 	vcmple_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 12 	vcmple_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 12 	vcmple_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 13 	vcmpunord_ssd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 13 	vcmpunord_ssd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 13 	vcmpunord_ssd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 13 	vcmpunord_ssd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 13 	vcmpunord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 13 	vcmpunord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 13 	vcmpunord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 13 	vcmpunord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 14 	vcmpneq_ussd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 14 	vcmpneq_ussd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 14 	vcmpneq_ussd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 14 	vcmpneq_ussd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 14 	vcmpneq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 14 	vcmpneq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 14 	vcmpneq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 14 	vcmpneq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 15 	vcmpnlt_uqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 15 	vcmpnlt_uqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 15 	vcmpnlt_uqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 15 	vcmpnlt_uqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 15 	vcmpnlt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 15 	vcmpnlt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 15 	vcmpnlt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 15 	vcmpnlt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 16 	vcmpnle_uqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 16 	vcmpnle_uqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 16 	vcmpnle_uqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 16 	vcmpnle_uqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 16 	vcmpnle_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 16 	vcmpnle_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 16 	vcmpnle_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 16 	vcmpnle_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 17 	vcmpord_ssd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 17 	vcmpord_ssd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 17 	vcmpord_ssd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 17 	vcmpord_ssd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 17 	vcmpord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 17 	vcmpord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 17 	vcmpord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 17 	vcmpord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 18 	vcmpeq_ussd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 18 	vcmpeq_ussd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 18 	vcmpeq_ussd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 18 	vcmpeq_ussd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 18 	vcmpeq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 18 	vcmpeq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 18 	vcmpeq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 18 	vcmpeq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 19 	vcmpnge_uqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 19 	vcmpnge_uqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 19 	vcmpnge_uqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 19 	vcmpnge_uqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 19 	vcmpnge_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 19 	vcmpnge_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 19 	vcmpnge_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 19 	vcmpnge_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1a 	vcmpngt_uqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1a 	vcmpngt_uqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1a 	vcmpngt_uqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 1a 	vcmpngt_uqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1a 	vcmpngt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1a 	vcmpngt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1a 	vcmpngt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1a 	vcmpngt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1b 	vcmpfalse_ossd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1b 	vcmpfalse_ossd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1b 	vcmpfalse_ossd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 1b 	vcmpfalse_ossd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1b 	vcmpfalse_ossd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1b 	vcmpfalse_ossd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1b 	vcmpfalse_ossd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1b 	vcmpfalse_ossd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1c 	vcmpneq_ossd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1c 	vcmpneq_ossd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1c 	vcmpneq_ossd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 1c 	vcmpneq_ossd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1c 	vcmpneq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1c 	vcmpneq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1c 	vcmpneq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1c 	vcmpneq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1d 	vcmpge_oqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1d 	vcmpge_oqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1d 	vcmpge_oqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 1d 	vcmpge_oqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1d 	vcmpge_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1d 	vcmpge_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1d 	vcmpge_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1d 	vcmpge_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1e 	vcmpgt_oqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1e 	vcmpgt_oqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1e 	vcmpgt_oqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 1e 	vcmpgt_oqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1e 	vcmpgt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1e 	vcmpgt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1e 	vcmpgt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1e 	vcmpgt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1f 	vcmptrue_ussd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1f 	vcmptrue_ussd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1f 	vcmptrue_ussd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 1f 	vcmptrue_ussd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1f 	vcmptrue_ussd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1f 	vcmptrue_ussd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1f 	vcmptrue_ussd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1f 	vcmptrue_ussd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec ab 	vcmpss k5\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec ab 	vcmpss k5\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 7b 	vcmpss k5\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 7b 	vcmpss k5\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 7b 	vcmpss k5\{k7\},xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 7b 	vcmpss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 7b 	vcmpss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 7b 	vcmpss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 7b 	vcmpss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 7b 	vcmpss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 00 	vcmpeqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 00 	vcmpeqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 00 	vcmpeqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 00 	vcmpeqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 01 	vcmpltss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 01 	vcmpltss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 01 	vcmpltss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 01 	vcmpltss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 02 	vcmpless k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 02 	vcmpless k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 02 	vcmpless k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 02 	vcmpless k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 03 	vcmpunordss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 03 	vcmpunordss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 03 	vcmpunordss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 03 	vcmpunordss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 04 	vcmpneqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 04 	vcmpneqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 04 	vcmpneqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 04 	vcmpneqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 05 	vcmpnltss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 05 	vcmpnltss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 05 	vcmpnltss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 05 	vcmpnltss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 06 	vcmpnless k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 06 	vcmpnless k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 06 	vcmpnless k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 06 	vcmpnless k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 07 	vcmpordss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 07 	vcmpordss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 07 	vcmpordss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 07 	vcmpordss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 08 	vcmpeq_uqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 08 	vcmpeq_uqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 08 	vcmpeq_uqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 08 	vcmpeq_uqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 08 	vcmpeq_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 08 	vcmpeq_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 08 	vcmpeq_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 08 	vcmpeq_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 09 	vcmpngess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 09 	vcmpngess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 09 	vcmpngess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 09 	vcmpngess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0a 	vcmpngtss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0a 	vcmpngtss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0a 	vcmpngtss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0a 	vcmpngtss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0b 	vcmpfalsess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0b 	vcmpfalsess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0b 	vcmpfalsess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0b 	vcmpfalsess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0c 	vcmpneq_oqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0c 	vcmpneq_oqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0c 	vcmpneq_oqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0c 	vcmpneq_oqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0c 	vcmpneq_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0c 	vcmpneq_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0c 	vcmpneq_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0c 	vcmpneq_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0d 	vcmpgess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0d 	vcmpgess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0d 	vcmpgess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0d 	vcmpgess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0e 	vcmpgtss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0e 	vcmpgtss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0e 	vcmpgtss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0e 	vcmpgtss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0f 	vcmptruess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0f 	vcmptruess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0f 	vcmptruess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0f 	vcmptruess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 10 	vcmpeq_osss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 10 	vcmpeq_osss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 10 	vcmpeq_osss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 10 	vcmpeq_osss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 10 	vcmpeq_osss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 10 	vcmpeq_osss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 10 	vcmpeq_osss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 10 	vcmpeq_osss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 11 	vcmplt_oqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 11 	vcmplt_oqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 11 	vcmplt_oqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 11 	vcmplt_oqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 11 	vcmplt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 11 	vcmplt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 11 	vcmplt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 11 	vcmplt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 12 	vcmple_oqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 12 	vcmple_oqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 12 	vcmple_oqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 12 	vcmple_oqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 12 	vcmple_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 12 	vcmple_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 12 	vcmple_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 12 	vcmple_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 13 	vcmpunord_sss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 13 	vcmpunord_sss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 13 	vcmpunord_sss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 13 	vcmpunord_sss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 13 	vcmpunord_sss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 13 	vcmpunord_sss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 13 	vcmpunord_sss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 13 	vcmpunord_sss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 14 	vcmpneq_usss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 14 	vcmpneq_usss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 14 	vcmpneq_usss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 14 	vcmpneq_usss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 14 	vcmpneq_usss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 14 	vcmpneq_usss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 14 	vcmpneq_usss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 14 	vcmpneq_usss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 15 	vcmpnlt_uqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 15 	vcmpnlt_uqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 15 	vcmpnlt_uqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 15 	vcmpnlt_uqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 15 	vcmpnlt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 15 	vcmpnlt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 15 	vcmpnlt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 15 	vcmpnlt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 16 	vcmpnle_uqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 16 	vcmpnle_uqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 16 	vcmpnle_uqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 16 	vcmpnle_uqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 16 	vcmpnle_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 16 	vcmpnle_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 16 	vcmpnle_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 16 	vcmpnle_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 17 	vcmpord_sss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 17 	vcmpord_sss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 17 	vcmpord_sss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 17 	vcmpord_sss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 17 	vcmpord_sss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 17 	vcmpord_sss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 17 	vcmpord_sss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 17 	vcmpord_sss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 18 	vcmpeq_usss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 18 	vcmpeq_usss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 18 	vcmpeq_usss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 18 	vcmpeq_usss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 18 	vcmpeq_usss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 18 	vcmpeq_usss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 18 	vcmpeq_usss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 18 	vcmpeq_usss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 19 	vcmpnge_uqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 19 	vcmpnge_uqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 19 	vcmpnge_uqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 19 	vcmpnge_uqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 19 	vcmpnge_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 19 	vcmpnge_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 19 	vcmpnge_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 19 	vcmpnge_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1a 	vcmpngt_uqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1a 	vcmpngt_uqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1a 	vcmpngt_uqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 1a 	vcmpngt_uqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1a 	vcmpngt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1a 	vcmpngt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1a 	vcmpngt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1a 	vcmpngt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1b 	vcmpfalse_osss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1b 	vcmpfalse_osss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1b 	vcmpfalse_osss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 1b 	vcmpfalse_osss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1b 	vcmpfalse_osss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1b 	vcmpfalse_osss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1b 	vcmpfalse_osss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1b 	vcmpfalse_osss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1c 	vcmpneq_osss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1c 	vcmpneq_osss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1c 	vcmpneq_osss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 1c 	vcmpneq_osss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1c 	vcmpneq_osss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1c 	vcmpneq_osss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1c 	vcmpneq_osss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1c 	vcmpneq_osss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1d 	vcmpge_oqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1d 	vcmpge_oqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1d 	vcmpge_oqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 1d 	vcmpge_oqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1d 	vcmpge_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1d 	vcmpge_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1d 	vcmpge_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1d 	vcmpge_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1e 	vcmpgt_oqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1e 	vcmpgt_oqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1e 	vcmpgt_oqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 1e 	vcmpgt_oqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1e 	vcmpgt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1e 	vcmpgt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1e 	vcmpgt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1e 	vcmpgt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1f 	vcmptrue_usss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1f 	vcmptrue_usss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1f 	vcmptrue_usss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 1f 	vcmptrue_usss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1f 	vcmptrue_usss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1f 	vcmptrue_usss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1f 	vcmptrue_usss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1f 	vcmptrue_usss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 01 fd 48 2f f5    	vcomisd xmm30,xmm29
[ 	]*[a-f0-9]+:	62 01 fd 18 2f f5    	vcomisd xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:	62 61 fd 48 2f 31    	vcomisd xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 fd 48 2f b4 f0 23 01 00 00 	vcomisd xmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2f 72 7f 	vcomisd xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2f b2 00 04 00 00 	vcomisd xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2f 72 80 	vcomisd xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2f b2 f8 fb ff ff 	vcomisd xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 7c 48 2f f5    	vcomiss xmm30,xmm29
[ 	]*[a-f0-9]+:	62 01 7c 18 2f f5    	vcomiss xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:	62 61 7c 48 2f 31    	vcomiss xmm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 7c 48 2f b4 f0 23 01 00 00 	vcomiss xmm30,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2f 72 7f 	vcomiss xmm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2f b2 00 02 00 00 	vcomiss xmm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2f 72 80 	vcomiss xmm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2f b2 fc fd ff ff 	vcomiss xmm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 7f 18 2d c6    	vcvtsd2si eax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 58 2d c6    	vcvtsd2si eax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 38 2d c6    	vcvtsd2si eax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 78 2d c6    	vcvtsd2si eax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 18 2d ee    	vcvtsd2si ebp,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 58 2d ee    	vcvtsd2si ebp,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 38 2d ee    	vcvtsd2si ebp,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 78 2d ee    	vcvtsd2si ebp,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 18 2d ee    	vcvtsd2si r13d,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 58 2d ee    	vcvtsd2si r13d,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 38 2d ee    	vcvtsd2si r13d,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 78 2d ee    	vcvtsd2si r13d,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 18 2d c6    	vcvtsd2si rax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 58 2d c6    	vcvtsd2si rax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 38 2d c6    	vcvtsd2si rax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 78 2d c6    	vcvtsd2si rax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 18 2d c6    	vcvtsd2si r8,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 58 2d c6    	vcvtsd2si r8,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 38 2d c6    	vcvtsd2si r8,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 78 2d c6    	vcvtsd2si r8,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 01 97 47 5a f4    	vcvtsd2ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 5a f4    	vcvtsd2ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 5a f4    	vcvtsd2ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 97 57 5a f4    	vcvtsd2ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 97 37 5a f4    	vcvtsd2ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 97 77 5a f4    	vcvtsd2ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 5a 31    	vcvtsd2ss xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 5a b4 f0 23 01 00 00 	vcvtsd2ss xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 97 47 5a 72 7f 	vcvtsd2ss xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 5a b2 00 04 00 00 	vcvtsd2ss xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5a 72 80 	vcvtsd2ss xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5a b2 f8 fb ff ff 	vcvtsd2ss xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 61 17 40 2a f0    	vcvtsi2sd xmm30,xmm29,eax
[ 	]*[a-f0-9]+:	62 61 17 40 2a f5    	vcvtsi2sd xmm30,xmm29,ebp
[ 	]*[a-f0-9]+:	62 41 17 40 2a f5    	vcvtsi2sd xmm30,xmm29,r13d
[ 	]*[a-f0-9]+:	62 61 17 40 2a 31    	vcvtsi2sd xmm30,xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 17 40 2a b4 f0 23 01 00 00 	vcvtsi2sd xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 17 40 2a 72 7f 	vcvtsi2sd xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 17 40 2a b2 00 02 00 00 	vcvtsi2sd xmm30,xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 17 40 2a 72 80 	vcvtsi2sd xmm30,xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 17 40 2a b2 fc fd ff ff 	vcvtsi2sd xmm30,xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 97 40 2a f0    	vcvtsi2sd xmm30,xmm29,rax
[ 	]*[a-f0-9]+:	62 61 97 10 2a f0    	vcvtsi2sd xmm30,xmm29,rax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 97 50 2a f0    	vcvtsi2sd xmm30,xmm29,rax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 97 30 2a f0    	vcvtsi2sd xmm30,xmm29,rax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 97 70 2a f0    	vcvtsi2sd xmm30,xmm29,rax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 41 97 40 2a f0    	vcvtsi2sd xmm30,xmm29,r8
[ 	]*[a-f0-9]+:	62 41 97 10 2a f0    	vcvtsi2sd xmm30,xmm29,r8\{rn-sae\}
[ 	]*[a-f0-9]+:	62 41 97 50 2a f0    	vcvtsi2sd xmm30,xmm29,r8\{ru-sae\}
[ 	]*[a-f0-9]+:	62 41 97 30 2a f0    	vcvtsi2sd xmm30,xmm29,r8\{rd-sae\}
[ 	]*[a-f0-9]+:	62 41 97 70 2a f0    	vcvtsi2sd xmm30,xmm29,r8\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 40 2a 31    	vcvtsi2sd xmm30,xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 40 2a b4 f0 23 01 00 00 	vcvtsi2sd xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 97 40 2a 72 7f 	vcvtsi2sd xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 40 2a b2 00 04 00 00 	vcvtsi2sd xmm30,xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 40 2a 72 80 	vcvtsi2sd xmm30,xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 40 2a b2 f8 fb ff ff 	vcvtsi2sd xmm30,xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 61 16 40 2a f0    	vcvtsi2ss xmm30,xmm29,eax
[ 	]*[a-f0-9]+:	62 61 16 10 2a f0    	vcvtsi2ss xmm30,xmm29,eax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 16 50 2a f0    	vcvtsi2ss xmm30,xmm29,eax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 16 30 2a f0    	vcvtsi2ss xmm30,xmm29,eax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 16 70 2a f0    	vcvtsi2ss xmm30,xmm29,eax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 40 2a f5    	vcvtsi2ss xmm30,xmm29,ebp
[ 	]*[a-f0-9]+:	62 61 16 10 2a f5    	vcvtsi2ss xmm30,xmm29,ebp\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 16 50 2a f5    	vcvtsi2ss xmm30,xmm29,ebp\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 16 30 2a f5    	vcvtsi2ss xmm30,xmm29,ebp\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 16 70 2a f5    	vcvtsi2ss xmm30,xmm29,ebp\{rz-sae\}
[ 	]*[a-f0-9]+:	62 41 16 40 2a f5    	vcvtsi2ss xmm30,xmm29,r13d
[ 	]*[a-f0-9]+:	62 41 16 10 2a f5    	vcvtsi2ss xmm30,xmm29,r13d\{rn-sae\}
[ 	]*[a-f0-9]+:	62 41 16 50 2a f5    	vcvtsi2ss xmm30,xmm29,r13d\{ru-sae\}
[ 	]*[a-f0-9]+:	62 41 16 30 2a f5    	vcvtsi2ss xmm30,xmm29,r13d\{rd-sae\}
[ 	]*[a-f0-9]+:	62 41 16 70 2a f5    	vcvtsi2ss xmm30,xmm29,r13d\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 40 2a 31    	vcvtsi2ss xmm30,xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 40 2a b4 f0 23 01 00 00 	vcvtsi2ss xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 16 40 2a 72 7f 	vcvtsi2ss xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 40 2a b2 00 02 00 00 	vcvtsi2ss xmm30,xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 40 2a 72 80 	vcvtsi2ss xmm30,xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 40 2a b2 fc fd ff ff 	vcvtsi2ss xmm30,xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 96 40 2a f0    	vcvtsi2ss xmm30,xmm29,rax
[ 	]*[a-f0-9]+:	62 61 96 10 2a f0    	vcvtsi2ss xmm30,xmm29,rax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 96 50 2a f0    	vcvtsi2ss xmm30,xmm29,rax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 96 30 2a f0    	vcvtsi2ss xmm30,xmm29,rax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 96 70 2a f0    	vcvtsi2ss xmm30,xmm29,rax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 41 96 40 2a f0    	vcvtsi2ss xmm30,xmm29,r8
[ 	]*[a-f0-9]+:	62 41 96 10 2a f0    	vcvtsi2ss xmm30,xmm29,r8\{rn-sae\}
[ 	]*[a-f0-9]+:	62 41 96 50 2a f0    	vcvtsi2ss xmm30,xmm29,r8\{ru-sae\}
[ 	]*[a-f0-9]+:	62 41 96 30 2a f0    	vcvtsi2ss xmm30,xmm29,r8\{rd-sae\}
[ 	]*[a-f0-9]+:	62 41 96 70 2a f0    	vcvtsi2ss xmm30,xmm29,r8\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 96 40 2a 31    	vcvtsi2ss xmm30,xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 96 40 2a b4 f0 23 01 00 00 	vcvtsi2ss xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 96 40 2a 72 7f 	vcvtsi2ss xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 96 40 2a b2 00 04 00 00 	vcvtsi2ss xmm30,xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 96 40 2a 72 80 	vcvtsi2ss xmm30,xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 96 40 2a b2 f8 fb ff ff 	vcvtsi2ss xmm30,xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 5a f4    	vcvtss2sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 5a f4    	vcvtss2sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 5a f4    	vcvtss2sd xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 5a 31    	vcvtss2sd xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 5a b4 f0 23 01 00 00 	vcvtss2sd xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 16 47 5a 72 7f 	vcvtss2sd xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 5a b2 00 02 00 00 	vcvtss2sd xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5a 72 80 	vcvtss2sd xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5a b2 fc fd ff ff 	vcvtss2sd xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 7e 18 2d c6    	vcvtss2si eax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 58 2d c6    	vcvtss2si eax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 38 2d c6    	vcvtss2si eax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 78 2d c6    	vcvtss2si eax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 18 2d ee    	vcvtss2si ebp,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 58 2d ee    	vcvtss2si ebp,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 38 2d ee    	vcvtss2si ebp,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 78 2d ee    	vcvtss2si ebp,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 18 2d ee    	vcvtss2si r13d,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 58 2d ee    	vcvtss2si r13d,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 38 2d ee    	vcvtss2si r13d,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 78 2d ee    	vcvtss2si r13d,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 18 2d c6    	vcvtss2si rax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 58 2d c6    	vcvtss2si rax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 38 2d c6    	vcvtss2si rax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 78 2d c6    	vcvtss2si rax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 18 2d c6    	vcvtss2si r8,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 58 2d c6    	vcvtss2si r8,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 38 2d c6    	vcvtss2si r8,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 78 2d c6    	vcvtss2si r8,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 18 2c c6    	vcvttsd2si eax,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 91 7f 18 2c ee    	vcvttsd2si ebp,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 11 7f 18 2c ee    	vcvttsd2si r13d,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 91 ff 18 2c c6    	vcvttsd2si rax,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 11 ff 18 2c c6    	vcvttsd2si r8,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 91 7e 18 2c c6    	vcvttss2si eax,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 91 7e 18 2c ee    	vcvttss2si ebp,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 11 7e 18 2c ee    	vcvttss2si r13d,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 91 fe 18 2c c6    	vcvttss2si rax,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 11 fe 18 2c c6    	vcvttss2si r8,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 01 97 47 5e f4    	vdivsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 5e f4    	vdivsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 5e f4    	vdivsd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 97 57 5e f4    	vdivsd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 97 37 5e f4    	vdivsd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 97 77 5e f4    	vdivsd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 5e 31    	vdivsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 5e b4 f0 23 01 00 00 	vdivsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 97 47 5e 72 7f 	vdivsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 5e b2 00 04 00 00 	vdivsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5e 72 80 	vdivsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5e b2 f8 fb ff ff 	vdivsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 5e f4    	vdivss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 5e f4    	vdivss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 5e f4    	vdivss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 16 57 5e f4    	vdivss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 16 37 5e f4    	vdivss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 16 77 5e f4    	vdivss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 5e 31    	vdivss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 5e b4 f0 23 01 00 00 	vdivss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 16 47 5e 72 7f 	vdivss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 5e b2 00 02 00 00 	vdivss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5e 72 80 	vdivss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5e b2 fc fd ff ff 	vdivss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 99 f4    	vfmadd132sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 99 f4    	vfmadd132sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 99 f4    	vfmadd132sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 99 f4    	vfmadd132sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 99 f4    	vfmadd132sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 99 f4    	vfmadd132sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 99 31    	vfmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 99 b4 f0 23 01 00 00 	vfmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 99 72 7f 	vfmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 99 b2 00 04 00 00 	vfmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 99 72 80 	vfmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 99 b2 f8 fb ff ff 	vfmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 99 f4    	vfmadd132ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 99 f4    	vfmadd132ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 99 f4    	vfmadd132ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 99 f4    	vfmadd132ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 99 f4    	vfmadd132ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 99 f4    	vfmadd132ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 99 31    	vfmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 99 b4 f0 23 01 00 00 	vfmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 99 72 7f 	vfmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 99 b2 00 02 00 00 	vfmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 99 72 80 	vfmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 99 b2 fc fd ff ff 	vfmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 a9 f4    	vfmadd213sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 a9 f4    	vfmadd213sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 a9 f4    	vfmadd213sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 a9 f4    	vfmadd213sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 a9 f4    	vfmadd213sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 a9 f4    	vfmadd213sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 a9 31    	vfmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 a9 b4 f0 23 01 00 00 	vfmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 a9 72 7f 	vfmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 a9 b2 00 04 00 00 	vfmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 a9 72 80 	vfmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 a9 b2 f8 fb ff ff 	vfmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 a9 f4    	vfmadd213ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 a9 f4    	vfmadd213ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 a9 f4    	vfmadd213ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 a9 f4    	vfmadd213ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 a9 f4    	vfmadd213ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 a9 f4    	vfmadd213ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 a9 31    	vfmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 a9 b4 f0 23 01 00 00 	vfmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 a9 72 7f 	vfmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 a9 b2 00 02 00 00 	vfmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 a9 72 80 	vfmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 a9 b2 fc fd ff ff 	vfmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 b9 f4    	vfmadd231sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 b9 f4    	vfmadd231sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 b9 f4    	vfmadd231sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 b9 f4    	vfmadd231sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 b9 f4    	vfmadd231sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 b9 f4    	vfmadd231sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 b9 31    	vfmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 b9 b4 f0 23 01 00 00 	vfmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 b9 72 7f 	vfmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 b9 b2 00 04 00 00 	vfmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 b9 72 80 	vfmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 b9 b2 f8 fb ff ff 	vfmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 b9 f4    	vfmadd231ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 b9 f4    	vfmadd231ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 b9 f4    	vfmadd231ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 b9 f4    	vfmadd231ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 b9 f4    	vfmadd231ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 b9 f4    	vfmadd231ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 b9 31    	vfmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 b9 b4 f0 23 01 00 00 	vfmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 b9 72 7f 	vfmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 b9 b2 00 02 00 00 	vfmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 b9 72 80 	vfmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 b9 b2 fc fd ff ff 	vfmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 9b f4    	vfmsub132sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 9b f4    	vfmsub132sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 9b f4    	vfmsub132sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 9b f4    	vfmsub132sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 9b f4    	vfmsub132sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 9b f4    	vfmsub132sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 9b 31    	vfmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 9b b4 f0 23 01 00 00 	vfmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 9b 72 7f 	vfmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 9b b2 00 04 00 00 	vfmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 9b 72 80 	vfmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 9b b2 f8 fb ff ff 	vfmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 9b f4    	vfmsub132ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 9b f4    	vfmsub132ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 9b f4    	vfmsub132ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 9b f4    	vfmsub132ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 9b f4    	vfmsub132ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 9b f4    	vfmsub132ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 9b 31    	vfmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 9b b4 f0 23 01 00 00 	vfmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 9b 72 7f 	vfmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 9b b2 00 02 00 00 	vfmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 9b 72 80 	vfmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 9b b2 fc fd ff ff 	vfmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 ab f4    	vfmsub213sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 ab f4    	vfmsub213sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 ab f4    	vfmsub213sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 ab f4    	vfmsub213sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 ab f4    	vfmsub213sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 ab f4    	vfmsub213sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 ab 31    	vfmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 ab b4 f0 23 01 00 00 	vfmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 ab 72 7f 	vfmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 ab b2 00 04 00 00 	vfmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 ab 72 80 	vfmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 ab b2 f8 fb ff ff 	vfmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 ab f4    	vfmsub213ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 ab f4    	vfmsub213ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 ab f4    	vfmsub213ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 ab f4    	vfmsub213ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 ab f4    	vfmsub213ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 ab f4    	vfmsub213ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 ab 31    	vfmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 ab b4 f0 23 01 00 00 	vfmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 ab 72 7f 	vfmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 ab b2 00 02 00 00 	vfmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 ab 72 80 	vfmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 ab b2 fc fd ff ff 	vfmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 bb f4    	vfmsub231sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 bb f4    	vfmsub231sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 bb f4    	vfmsub231sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 bb f4    	vfmsub231sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 bb f4    	vfmsub231sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 bb f4    	vfmsub231sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 bb 31    	vfmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 bb b4 f0 23 01 00 00 	vfmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 bb 72 7f 	vfmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 bb b2 00 04 00 00 	vfmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 bb 72 80 	vfmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 bb b2 f8 fb ff ff 	vfmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 bb f4    	vfmsub231ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 bb f4    	vfmsub231ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 bb f4    	vfmsub231ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 bb f4    	vfmsub231ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 bb f4    	vfmsub231ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 bb f4    	vfmsub231ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 bb 31    	vfmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 bb b4 f0 23 01 00 00 	vfmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 bb 72 7f 	vfmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 bb b2 00 02 00 00 	vfmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 bb 72 80 	vfmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 bb b2 fc fd ff ff 	vfmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 9d f4    	vfnmadd132sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 9d f4    	vfnmadd132sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 9d f4    	vfnmadd132sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 9d f4    	vfnmadd132sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 9d f4    	vfnmadd132sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 9d f4    	vfnmadd132sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 9d 31    	vfnmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 9d b4 f0 23 01 00 00 	vfnmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 9d 72 7f 	vfnmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 9d b2 00 04 00 00 	vfnmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 9d 72 80 	vfnmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 9d b2 f8 fb ff ff 	vfnmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 9d f4    	vfnmadd132ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 9d f4    	vfnmadd132ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 9d f4    	vfnmadd132ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 9d f4    	vfnmadd132ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 9d f4    	vfnmadd132ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 9d f4    	vfnmadd132ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 9d 31    	vfnmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 9d b4 f0 23 01 00 00 	vfnmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 9d 72 7f 	vfnmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 9d b2 00 02 00 00 	vfnmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 9d 72 80 	vfnmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 9d b2 fc fd ff ff 	vfnmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 ad f4    	vfnmadd213sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 ad f4    	vfnmadd213sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 ad f4    	vfnmadd213sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 ad f4    	vfnmadd213sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 ad f4    	vfnmadd213sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 ad f4    	vfnmadd213sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 ad 31    	vfnmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 ad b4 f0 23 01 00 00 	vfnmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 ad 72 7f 	vfnmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 ad b2 00 04 00 00 	vfnmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 ad 72 80 	vfnmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 ad b2 f8 fb ff ff 	vfnmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 ad f4    	vfnmadd213ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 ad f4    	vfnmadd213ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 ad f4    	vfnmadd213ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 ad f4    	vfnmadd213ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 ad f4    	vfnmadd213ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 ad f4    	vfnmadd213ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 ad 31    	vfnmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 ad b4 f0 23 01 00 00 	vfnmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 ad 72 7f 	vfnmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 ad b2 00 02 00 00 	vfnmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 ad 72 80 	vfnmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 ad b2 fc fd ff ff 	vfnmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 bd f4    	vfnmadd231sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 bd f4    	vfnmadd231sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 bd f4    	vfnmadd231sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 bd f4    	vfnmadd231sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 bd f4    	vfnmadd231sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 bd f4    	vfnmadd231sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 bd 31    	vfnmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 bd b4 f0 23 01 00 00 	vfnmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 bd 72 7f 	vfnmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 bd b2 00 04 00 00 	vfnmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 bd 72 80 	vfnmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 bd b2 f8 fb ff ff 	vfnmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 bd f4    	vfnmadd231ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 bd f4    	vfnmadd231ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 bd f4    	vfnmadd231ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 bd f4    	vfnmadd231ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 bd f4    	vfnmadd231ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 bd f4    	vfnmadd231ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 bd 31    	vfnmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 bd b4 f0 23 01 00 00 	vfnmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 bd 72 7f 	vfnmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 bd b2 00 02 00 00 	vfnmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 bd 72 80 	vfnmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 bd b2 fc fd ff ff 	vfnmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 9f f4    	vfnmsub132sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 9f f4    	vfnmsub132sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 9f f4    	vfnmsub132sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 9f f4    	vfnmsub132sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 9f f4    	vfnmsub132sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 9f f4    	vfnmsub132sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 9f 31    	vfnmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 9f b4 f0 23 01 00 00 	vfnmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 9f 72 7f 	vfnmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 9f b2 00 04 00 00 	vfnmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 9f 72 80 	vfnmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 9f b2 f8 fb ff ff 	vfnmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 9f f4    	vfnmsub132ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 9f f4    	vfnmsub132ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 9f f4    	vfnmsub132ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 9f f4    	vfnmsub132ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 9f f4    	vfnmsub132ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 9f f4    	vfnmsub132ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 9f 31    	vfnmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 9f b4 f0 23 01 00 00 	vfnmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 9f 72 7f 	vfnmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 9f b2 00 02 00 00 	vfnmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 9f 72 80 	vfnmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 9f b2 fc fd ff ff 	vfnmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 af f4    	vfnmsub213sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 af f4    	vfnmsub213sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 af f4    	vfnmsub213sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 af f4    	vfnmsub213sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 af f4    	vfnmsub213sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 af f4    	vfnmsub213sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 af 31    	vfnmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 af b4 f0 23 01 00 00 	vfnmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 af 72 7f 	vfnmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 af b2 00 04 00 00 	vfnmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 af 72 80 	vfnmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 af b2 f8 fb ff ff 	vfnmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 af f4    	vfnmsub213ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 af f4    	vfnmsub213ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 af f4    	vfnmsub213ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 af f4    	vfnmsub213ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 af f4    	vfnmsub213ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 af f4    	vfnmsub213ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 af 31    	vfnmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 af b4 f0 23 01 00 00 	vfnmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 af 72 7f 	vfnmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 af b2 00 02 00 00 	vfnmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 af 72 80 	vfnmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 af b2 fc fd ff ff 	vfnmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 bf f4    	vfnmsub231sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 bf f4    	vfnmsub231sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 bf f4    	vfnmsub231sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 bf f4    	vfnmsub231sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 bf f4    	vfnmsub231sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 bf f4    	vfnmsub231sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 bf 31    	vfnmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 bf b4 f0 23 01 00 00 	vfnmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 bf 72 7f 	vfnmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 bf b2 00 04 00 00 	vfnmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 bf 72 80 	vfnmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 bf b2 f8 fb ff ff 	vfnmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 bf f4    	vfnmsub231ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 bf f4    	vfnmsub231ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 bf f4    	vfnmsub231ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 bf f4    	vfnmsub231ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 bf f4    	vfnmsub231ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 bf f4    	vfnmsub231ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 bf 31    	vfnmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 bf b4 f0 23 01 00 00 	vfnmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 bf 72 7f 	vfnmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 bf b2 00 02 00 00 	vfnmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 bf 72 80 	vfnmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 bf b2 fc fd ff ff 	vfnmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 43 f4    	vgetexpsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 43 f4    	vgetexpsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 43 f4    	vgetexpsd xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 43 31    	vgetexpsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 43 b4 f0 23 01 00 00 	vgetexpsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 43 72 7f 	vgetexpsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 43 b2 00 04 00 00 	vgetexpsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 43 72 80 	vgetexpsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 43 b2 f8 fb ff ff 	vgetexpsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 43 f4    	vgetexpss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 43 f4    	vgetexpss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 43 f4    	vgetexpss xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 43 31    	vgetexpss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 43 b4 f0 23 01 00 00 	vgetexpss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 43 72 7f 	vgetexpss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 43 b2 00 02 00 00 	vgetexpss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 43 72 80 	vgetexpss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 43 b2 fc fd ff ff 	vgetexpss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 03 95 47 27 f4 ab 	vgetmantsd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 95 c7 27 f4 ab 	vgetmantsd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 95 17 27 f4 ab 	vgetmantsd xmm30\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 03 95 47 27 f4 7b 	vgetmantsd xmm30\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 03 95 17 27 f4 7b 	vgetmantsd xmm30\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 27 31 7b 	vgetmantsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 23 95 47 27 b4 f0 23 01 00 00 7b 	vgetmantsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 27 72 7f 7b 	vgetmantsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 27 b2 00 04 00 00 7b 	vgetmantsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 27 72 80 7b 	vgetmantsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 27 b2 f8 fb ff ff 7b 	vgetmantsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 03 15 47 27 f4 ab 	vgetmantss xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 15 c7 27 f4 ab 	vgetmantss xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 15 17 27 f4 ab 	vgetmantss xmm30\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 03 15 47 27 f4 7b 	vgetmantss xmm30\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 03 15 17 27 f4 7b 	vgetmantss xmm30\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 27 31 7b 	vgetmantss xmm30\{k7\},xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 23 15 47 27 b4 f0 23 01 00 00 7b 	vgetmantss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 27 72 7f 7b 	vgetmantss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 27 b2 00 02 00 00 7b 	vgetmantss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 27 72 80 7b 	vgetmantss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 27 b2 fc fd ff ff 7b 	vgetmantss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 01 97 47 5f f4    	vmaxsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 5f f4    	vmaxsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 5f f4    	vmaxsd xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 5f 31    	vmaxsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 5f b4 f0 23 01 00 00 	vmaxsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 97 47 5f 72 7f 	vmaxsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 5f b2 00 04 00 00 	vmaxsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5f 72 80 	vmaxsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5f b2 f8 fb ff ff 	vmaxsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 5f f4    	vmaxss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 5f f4    	vmaxss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 5f f4    	vmaxss xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 5f 31    	vmaxss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 5f b4 f0 23 01 00 00 	vmaxss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 16 47 5f 72 7f 	vmaxss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 5f b2 00 02 00 00 	vmaxss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5f 72 80 	vmaxss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5f b2 fc fd ff ff 	vmaxss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 01 97 47 5d f4    	vminsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 5d f4    	vminsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 5d f4    	vminsd xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 5d 31    	vminsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 5d b4 f0 23 01 00 00 	vminsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 97 47 5d 72 7f 	vminsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 5d b2 00 04 00 00 	vminsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5d 72 80 	vminsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5d b2 f8 fb ff ff 	vminsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 5d f4    	vminss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 5d f4    	vminss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 5d f4    	vminss xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 5d 31    	vminss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 5d b4 f0 23 01 00 00 	vminss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 16 47 5d 72 7f 	vminss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 5d b2 00 02 00 00 	vminss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5d 72 80 	vminss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5d b2 fc fd ff ff 	vminss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 31    	vmovsd xmm30\{k7\},QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 61 ff cf 10 31    	vmovsd xmm30\{k7\}\{z\},QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 ff 4f 10 b4 f0 23 01 00 00 	vmovsd xmm30\{k7\},QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 72 7f 	vmovsd xmm30\{k7\},QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 b2 00 04 00 00 	vmovsd xmm30\{k7\},QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 72 80 	vmovsd xmm30\{k7\},QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 b2 f8 fb ff ff 	vmovsd xmm30\{k7\},QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 31    	vmovsd QWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 21 ff 4f 11 b4 f0 23 01 00 00 	vmovsd QWORD PTR \[rax\+r14\*8\+0x123\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 72 7f 	vmovsd QWORD PTR \[rdx\+0x3f8\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 b2 00 04 00 00 	vmovsd QWORD PTR \[rdx\+0x400\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 72 80 	vmovsd QWORD PTR \[rdx-0x400\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 b2 f8 fb ff ff 	vmovsd QWORD PTR \[rdx-0x408\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 01 97 47 10 f4    	vmovsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 10 f4    	vmovsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 31    	vmovss xmm30\{k7\},DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 61 7e cf 10 31    	vmovss xmm30\{k7\}\{z\},DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 7e 4f 10 b4 f0 23 01 00 00 	vmovss xmm30\{k7\},DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 72 7f 	vmovss xmm30\{k7\},DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 b2 00 02 00 00 	vmovss xmm30\{k7\},DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 72 80 	vmovss xmm30\{k7\},DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 b2 fc fd ff ff 	vmovss xmm30\{k7\},DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 31    	vmovss DWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 21 7e 4f 11 b4 f0 23 01 00 00 	vmovss DWORD PTR \[rax\+r14\*8\+0x123\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 72 7f 	vmovss DWORD PTR \[rdx\+0x1fc\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 b2 00 02 00 00 	vmovss DWORD PTR \[rdx\+0x200\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 72 80 	vmovss DWORD PTR \[rdx-0x200\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 b2 fc fd ff ff 	vmovss DWORD PTR \[rdx-0x204\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 01 16 47 10 f4    	vmovss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 10 f4    	vmovss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 47 59 f4    	vmulsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 59 f4    	vmulsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 59 f4    	vmulsd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 97 57 59 f4    	vmulsd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 97 37 59 f4    	vmulsd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 97 77 59 f4    	vmulsd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 59 31    	vmulsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 59 b4 f0 23 01 00 00 	vmulsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 97 47 59 72 7f 	vmulsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 59 b2 00 04 00 00 	vmulsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 59 72 80 	vmulsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 59 b2 f8 fb ff ff 	vmulsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 59 f4    	vmulss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 59 f4    	vmulss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 59 f4    	vmulss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 16 57 59 f4    	vmulss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 16 37 59 f4    	vmulss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 16 77 59 f4    	vmulss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 59 31    	vmulss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 59 b4 f0 23 01 00 00 	vmulss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 16 47 59 72 7f 	vmulss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 59 b2 00 02 00 00 	vmulss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 59 72 80 	vmulss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 59 b2 fc fd ff ff 	vmulss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 4d f4    	vrcp14sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 4d f4    	vrcp14sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 62 95 47 4d 31    	vrcp14sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 4d b4 f0 23 01 00 00 	vrcp14sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 4d 72 7f 	vrcp14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 4d b2 00 04 00 00 	vrcp14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 4d 72 80 	vrcp14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 4d b2 f8 fb ff ff 	vrcp14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 4d f4    	vrcp14ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 4d f4    	vrcp14ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 62 15 47 4d 31    	vrcp14ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 4d b4 f0 23 01 00 00 	vrcp14ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 4d 72 7f 	vrcp14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 4d b2 00 02 00 00 	vrcp14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 4d 72 80 	vrcp14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 4d b2 fc fd ff ff 	vrcp14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 15 47 cb f4    	vrcp28ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 cb f4    	vrcp28ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 cb f4    	vrcp28ss xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 cb 31    	vrcp28ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 cb b4 f0 23 01 00 00 	vrcp28ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 cb 72 7f 	vrcp28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 cb b2 00 02 00 00 	vrcp28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 cb 72 80 	vrcp28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 cb b2 fc fd ff ff 	vrcp28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 cb f4    	vrcp28sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 cb f4    	vrcp28sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 cb f4    	vrcp28sd xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 cb 31    	vrcp28sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 cb b4 f0 23 01 00 00 	vrcp28sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 cb 72 7f 	vrcp28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 cb b2 00 04 00 00 	vrcp28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 cb 72 80 	vrcp28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 cb b2 f8 fb ff ff 	vrcp28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 95 47 4f f4    	vrsqrt14sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 4f f4    	vrsqrt14sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 62 95 47 4f 31    	vrsqrt14sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 4f b4 f0 23 01 00 00 	vrsqrt14sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 4f 72 7f 	vrsqrt14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 4f b2 00 04 00 00 	vrsqrt14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 4f 72 80 	vrsqrt14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 4f b2 f8 fb ff ff 	vrsqrt14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 4f f4    	vrsqrt14ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 4f f4    	vrsqrt14ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 62 15 47 4f 31    	vrsqrt14ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 4f b4 f0 23 01 00 00 	vrsqrt14ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 4f 72 7f 	vrsqrt14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 4f b2 00 02 00 00 	vrsqrt14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 4f 72 80 	vrsqrt14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 4f b2 fc fd ff ff 	vrsqrt14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 15 47 cd f4    	vrsqrt28ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 cd f4    	vrsqrt28ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 cd f4    	vrsqrt28ss xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 cd 31    	vrsqrt28ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 cd b4 f0 23 01 00 00 	vrsqrt28ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 cd 72 7f 	vrsqrt28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 cd b2 00 02 00 00 	vrsqrt28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 cd 72 80 	vrsqrt28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 cd b2 fc fd ff ff 	vrsqrt28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 cd f4    	vrsqrt28sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 cd f4    	vrsqrt28sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 cd f4    	vrsqrt28sd xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 cd 31    	vrsqrt28sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 cd b4 f0 23 01 00 00 	vrsqrt28sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 cd 72 7f 	vrsqrt28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 cd b2 00 04 00 00 	vrsqrt28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 cd 72 80 	vrsqrt28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 cd b2 f8 fb ff ff 	vrsqrt28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 97 47 51 f4    	vsqrtsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 51 f4    	vsqrtsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 51 f4    	vsqrtsd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 97 57 51 f4    	vsqrtsd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 97 37 51 f4    	vsqrtsd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 97 77 51 f4    	vsqrtsd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 51 31    	vsqrtsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 51 b4 f0 23 01 00 00 	vsqrtsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 97 47 51 72 7f 	vsqrtsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 51 b2 00 04 00 00 	vsqrtsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 51 72 80 	vsqrtsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 51 b2 f8 fb ff ff 	vsqrtsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 51 f4    	vsqrtss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 51 f4    	vsqrtss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 51 f4    	vsqrtss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 16 57 51 f4    	vsqrtss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 16 37 51 f4    	vsqrtss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 16 77 51 f4    	vsqrtss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 51 31    	vsqrtss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 51 b4 f0 23 01 00 00 	vsqrtss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 16 47 51 72 7f 	vsqrtss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 51 b2 00 02 00 00 	vsqrtss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 51 72 80 	vsqrtss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 51 b2 fc fd ff ff 	vsqrtss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 01 97 47 5c f4    	vsubsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 5c f4    	vsubsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 5c f4    	vsubsd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 97 57 5c f4    	vsubsd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 97 37 5c f4    	vsubsd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 97 77 5c f4    	vsubsd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 5c 31    	vsubsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 5c b4 f0 23 01 00 00 	vsubsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 97 47 5c 72 7f 	vsubsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 5c b2 00 04 00 00 	vsubsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5c 72 80 	vsubsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5c b2 f8 fb ff ff 	vsubsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 5c f4    	vsubss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 5c f4    	vsubss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 5c f4    	vsubss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 16 57 5c f4    	vsubss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 16 37 5c f4    	vsubss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 16 77 5c f4    	vsubss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 5c 31    	vsubss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 5c b4 f0 23 01 00 00 	vsubss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 16 47 5c 72 7f 	vsubss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 5c b2 00 02 00 00 	vsubss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5c 72 80 	vsubss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5c b2 fc fd ff ff 	vsubss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 01 fd 48 2e f5    	vucomisd xmm30,xmm29
[ 	]*[a-f0-9]+:	62 01 fd 18 2e f5    	vucomisd xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:	62 61 fd 48 2e 31    	vucomisd xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 fd 48 2e b4 f0 23 01 00 00 	vucomisd xmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2e 72 7f 	vucomisd xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2e b2 00 04 00 00 	vucomisd xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2e 72 80 	vucomisd xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2e b2 f8 fb ff ff 	vucomisd xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 7c 48 2e f5    	vucomiss xmm30,xmm29
[ 	]*[a-f0-9]+:	62 01 7c 18 2e f5    	vucomiss xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:	62 61 7c 48 2e 31    	vucomiss xmm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 7c 48 2e b4 f0 23 01 00 00 	vucomiss xmm30,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2e 72 7f 	vucomiss xmm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2e b2 00 02 00 00 	vucomiss xmm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2e 72 80 	vucomiss xmm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2e b2 fc fd ff ff 	vucomiss xmm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 7f 48 79 c6    	vcvtsd2usi eax,xmm30
[ 	]*[a-f0-9]+:	62 91 7f 18 79 c6    	vcvtsd2usi eax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 58 79 c6    	vcvtsd2usi eax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 38 79 c6    	vcvtsd2usi eax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 78 79 c6    	vcvtsd2usi eax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 01    	vcvtsd2usi eax,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 7f 48 79 84 f0 23 01 00 00 	vcvtsd2usi eax,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 42 7f 	vcvtsd2usi eax,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 82 00 04 00 00 	vcvtsd2usi eax,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 42 80 	vcvtsd2usi eax,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 82 f8 fb ff ff 	vcvtsd2usi eax,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 7f 48 79 ee    	vcvtsd2usi ebp,xmm30
[ 	]*[a-f0-9]+:	62 91 7f 18 79 ee    	vcvtsd2usi ebp,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 58 79 ee    	vcvtsd2usi ebp,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 38 79 ee    	vcvtsd2usi ebp,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 78 79 ee    	vcvtsd2usi ebp,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 29    	vcvtsd2usi ebp,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 7f 48 79 ac f0 23 01 00 00 	vcvtsd2usi ebp,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 6a 7f 	vcvtsd2usi ebp,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 aa 00 04 00 00 	vcvtsd2usi ebp,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 6a 80 	vcvtsd2usi ebp,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 aa f8 fb ff ff 	vcvtsd2usi ebp,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 11 7f 48 79 ee    	vcvtsd2usi r13d,xmm30
[ 	]*[a-f0-9]+:	62 11 7f 18 79 ee    	vcvtsd2usi r13d,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 58 79 ee    	vcvtsd2usi r13d,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 38 79 ee    	vcvtsd2usi r13d,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 78 79 ee    	vcvtsd2usi r13d,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 71 7f 48 79 29    	vcvtsd2usi r13d,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 31 7f 48 79 ac f0 23 01 00 00 	vcvtsd2usi r13d,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 71 7f 48 79 6a 7f 	vcvtsd2usi r13d,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 71 7f 48 79 aa 00 04 00 00 	vcvtsd2usi r13d,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 71 7f 48 79 6a 80 	vcvtsd2usi r13d,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 71 7f 48 79 aa f8 fb ff ff 	vcvtsd2usi r13d,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 ff 48 79 c6    	vcvtsd2usi rax,xmm30
[ 	]*[a-f0-9]+:	62 91 ff 18 79 c6    	vcvtsd2usi rax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 58 79 c6    	vcvtsd2usi rax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 38 79 c6    	vcvtsd2usi rax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 78 79 c6    	vcvtsd2usi rax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 01    	vcvtsd2usi rax,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 ff 48 79 84 f0 23 01 00 00 	vcvtsd2usi rax,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 42 7f 	vcvtsd2usi rax,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 82 00 04 00 00 	vcvtsd2usi rax,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 42 80 	vcvtsd2usi rax,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 82 f8 fb ff ff 	vcvtsd2usi rax,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 11 ff 48 79 c6    	vcvtsd2usi r8,xmm30
[ 	]*[a-f0-9]+:	62 11 ff 18 79 c6    	vcvtsd2usi r8,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 58 79 c6    	vcvtsd2usi r8,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 38 79 c6    	vcvtsd2usi r8,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 78 79 c6    	vcvtsd2usi r8,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 71 ff 48 79 01    	vcvtsd2usi r8,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 31 ff 48 79 84 f0 23 01 00 00 	vcvtsd2usi r8,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 71 ff 48 79 42 7f 	vcvtsd2usi r8,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 71 ff 48 79 82 00 04 00 00 	vcvtsd2usi r8,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 71 ff 48 79 42 80 	vcvtsd2usi r8,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 71 ff 48 79 82 f8 fb ff ff 	vcvtsd2usi r8,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 7e 48 79 c6    	vcvtss2usi eax,xmm30
[ 	]*[a-f0-9]+:	62 91 7e 18 79 c6    	vcvtss2usi eax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 58 79 c6    	vcvtss2usi eax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 38 79 c6    	vcvtss2usi eax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 78 79 c6    	vcvtss2usi eax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 01    	vcvtss2usi eax,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 7e 48 79 84 f0 23 01 00 00 	vcvtss2usi eax,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 42 7f 	vcvtss2usi eax,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 82 00 02 00 00 	vcvtss2usi eax,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 42 80 	vcvtss2usi eax,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 82 fc fd ff ff 	vcvtss2usi eax,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 7e 48 79 ee    	vcvtss2usi ebp,xmm30
[ 	]*[a-f0-9]+:	62 91 7e 18 79 ee    	vcvtss2usi ebp,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 58 79 ee    	vcvtss2usi ebp,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 38 79 ee    	vcvtss2usi ebp,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 78 79 ee    	vcvtss2usi ebp,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 29    	vcvtss2usi ebp,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 7e 48 79 ac f0 23 01 00 00 	vcvtss2usi ebp,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 6a 7f 	vcvtss2usi ebp,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 aa 00 02 00 00 	vcvtss2usi ebp,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 6a 80 	vcvtss2usi ebp,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 aa fc fd ff ff 	vcvtss2usi ebp,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 11 7e 48 79 ee    	vcvtss2usi r13d,xmm30
[ 	]*[a-f0-9]+:	62 11 7e 18 79 ee    	vcvtss2usi r13d,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 58 79 ee    	vcvtss2usi r13d,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 38 79 ee    	vcvtss2usi r13d,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 78 79 ee    	vcvtss2usi r13d,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 71 7e 48 79 29    	vcvtss2usi r13d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 31 7e 48 79 ac f0 23 01 00 00 	vcvtss2usi r13d,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 71 7e 48 79 6a 7f 	vcvtss2usi r13d,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 71 7e 48 79 aa 00 02 00 00 	vcvtss2usi r13d,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 71 7e 48 79 6a 80 	vcvtss2usi r13d,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 71 7e 48 79 aa fc fd ff ff 	vcvtss2usi r13d,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 fe 48 79 c6    	vcvtss2usi rax,xmm30
[ 	]*[a-f0-9]+:	62 91 fe 18 79 c6    	vcvtss2usi rax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 58 79 c6    	vcvtss2usi rax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 38 79 c6    	vcvtss2usi rax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 78 79 c6    	vcvtss2usi rax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 01    	vcvtss2usi rax,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 fe 48 79 84 f0 23 01 00 00 	vcvtss2usi rax,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 42 7f 	vcvtss2usi rax,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 82 00 02 00 00 	vcvtss2usi rax,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 42 80 	vcvtss2usi rax,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 82 fc fd ff ff 	vcvtss2usi rax,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 11 fe 48 79 c6    	vcvtss2usi r8,xmm30
[ 	]*[a-f0-9]+:	62 11 fe 18 79 c6    	vcvtss2usi r8,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 58 79 c6    	vcvtss2usi r8,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 38 79 c6    	vcvtss2usi r8,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 78 79 c6    	vcvtss2usi r8,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 71 fe 48 79 01    	vcvtss2usi r8,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 31 fe 48 79 84 f0 23 01 00 00 	vcvtss2usi r8,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 71 fe 48 79 42 7f 	vcvtss2usi r8,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 71 fe 48 79 82 00 02 00 00 	vcvtss2usi r8,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 71 fe 48 79 42 80 	vcvtss2usi r8,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 71 fe 48 79 82 fc fd ff ff 	vcvtss2usi r8,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 17 40 7b f0    	vcvtusi2sd xmm30,xmm29,eax
[ 	]*[a-f0-9]+:	62 61 17 40 7b f5    	vcvtusi2sd xmm30,xmm29,ebp
[ 	]*[a-f0-9]+:	62 41 17 40 7b f5    	vcvtusi2sd xmm30,xmm29,r13d
[ 	]*[a-f0-9]+:	62 61 17 40 7b 31    	vcvtusi2sd xmm30,xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 17 40 7b b4 f0 23 01 00 00 	vcvtusi2sd xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 17 40 7b 72 7f 	vcvtusi2sd xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 17 40 7b b2 00 02 00 00 	vcvtusi2sd xmm30,xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 17 40 7b 72 80 	vcvtusi2sd xmm30,xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 17 40 7b b2 fc fd ff ff 	vcvtusi2sd xmm30,xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 97 40 7b f0    	vcvtusi2sd xmm30,xmm29,rax
[ 	]*[a-f0-9]+:	62 61 97 10 7b f0    	vcvtusi2sd xmm30,xmm29,rax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 97 50 7b f0    	vcvtusi2sd xmm30,xmm29,rax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 97 30 7b f0    	vcvtusi2sd xmm30,xmm29,rax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 97 70 7b f0    	vcvtusi2sd xmm30,xmm29,rax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 41 97 40 7b f0    	vcvtusi2sd xmm30,xmm29,r8
[ 	]*[a-f0-9]+:	62 41 97 10 7b f0    	vcvtusi2sd xmm30,xmm29,r8\{rn-sae\}
[ 	]*[a-f0-9]+:	62 41 97 50 7b f0    	vcvtusi2sd xmm30,xmm29,r8\{ru-sae\}
[ 	]*[a-f0-9]+:	62 41 97 30 7b f0    	vcvtusi2sd xmm30,xmm29,r8\{rd-sae\}
[ 	]*[a-f0-9]+:	62 41 97 70 7b f0    	vcvtusi2sd xmm30,xmm29,r8\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 40 7b 31    	vcvtusi2sd xmm30,xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 40 7b b4 f0 23 01 00 00 	vcvtusi2sd xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 97 40 7b 72 7f 	vcvtusi2sd xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 40 7b b2 00 04 00 00 	vcvtusi2sd xmm30,xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 40 7b 72 80 	vcvtusi2sd xmm30,xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 40 7b b2 f8 fb ff ff 	vcvtusi2sd xmm30,xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 61 16 40 7b f0    	vcvtusi2ss xmm30,xmm29,eax
[ 	]*[a-f0-9]+:	62 61 16 10 7b f0    	vcvtusi2ss xmm30,xmm29,eax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 16 50 7b f0    	vcvtusi2ss xmm30,xmm29,eax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 16 30 7b f0    	vcvtusi2ss xmm30,xmm29,eax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 16 70 7b f0    	vcvtusi2ss xmm30,xmm29,eax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 40 7b f5    	vcvtusi2ss xmm30,xmm29,ebp
[ 	]*[a-f0-9]+:	62 61 16 10 7b f5    	vcvtusi2ss xmm30,xmm29,ebp\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 16 50 7b f5    	vcvtusi2ss xmm30,xmm29,ebp\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 16 30 7b f5    	vcvtusi2ss xmm30,xmm29,ebp\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 16 70 7b f5    	vcvtusi2ss xmm30,xmm29,ebp\{rz-sae\}
[ 	]*[a-f0-9]+:	62 41 16 40 7b f5    	vcvtusi2ss xmm30,xmm29,r13d
[ 	]*[a-f0-9]+:	62 41 16 10 7b f5    	vcvtusi2ss xmm30,xmm29,r13d\{rn-sae\}
[ 	]*[a-f0-9]+:	62 41 16 50 7b f5    	vcvtusi2ss xmm30,xmm29,r13d\{ru-sae\}
[ 	]*[a-f0-9]+:	62 41 16 30 7b f5    	vcvtusi2ss xmm30,xmm29,r13d\{rd-sae\}
[ 	]*[a-f0-9]+:	62 41 16 70 7b f5    	vcvtusi2ss xmm30,xmm29,r13d\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 40 7b 31    	vcvtusi2ss xmm30,xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 40 7b b4 f0 23 01 00 00 	vcvtusi2ss xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 16 40 7b 72 7f 	vcvtusi2ss xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 40 7b b2 00 02 00 00 	vcvtusi2ss xmm30,xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 40 7b 72 80 	vcvtusi2ss xmm30,xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 40 7b b2 fc fd ff ff 	vcvtusi2ss xmm30,xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 96 40 7b f0    	vcvtusi2ss xmm30,xmm29,rax
[ 	]*[a-f0-9]+:	62 61 96 10 7b f0    	vcvtusi2ss xmm30,xmm29,rax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 96 50 7b f0    	vcvtusi2ss xmm30,xmm29,rax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 96 30 7b f0    	vcvtusi2ss xmm30,xmm29,rax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 96 70 7b f0    	vcvtusi2ss xmm30,xmm29,rax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 41 96 40 7b f0    	vcvtusi2ss xmm30,xmm29,r8
[ 	]*[a-f0-9]+:	62 41 96 10 7b f0    	vcvtusi2ss xmm30,xmm29,r8\{rn-sae\}
[ 	]*[a-f0-9]+:	62 41 96 50 7b f0    	vcvtusi2ss xmm30,xmm29,r8\{ru-sae\}
[ 	]*[a-f0-9]+:	62 41 96 30 7b f0    	vcvtusi2ss xmm30,xmm29,r8\{rd-sae\}
[ 	]*[a-f0-9]+:	62 41 96 70 7b f0    	vcvtusi2ss xmm30,xmm29,r8\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 96 40 7b 31    	vcvtusi2ss xmm30,xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 96 40 7b b4 f0 23 01 00 00 	vcvtusi2ss xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 61 96 40 7b 72 7f 	vcvtusi2ss xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 96 40 7b b2 00 04 00 00 	vcvtusi2ss xmm30,xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 96 40 7b 72 80 	vcvtusi2ss xmm30,xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 96 40 7b b2 f8 fb ff ff 	vcvtusi2ss xmm30,xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 95 47 2d f4    	vscalefsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 2d f4    	vscalefsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 2d f4    	vscalefsd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 2d f4    	vscalefsd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 2d f4    	vscalefsd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 2d f4    	vscalefsd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 2d 31    	vscalefsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 2d b4 f0 23 01 00 00 	vscalefsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 95 47 2d 72 7f 	vscalefsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 2d b2 00 04 00 00 	vscalefsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 2d 72 80 	vscalefsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 2d b2 f8 fb ff ff 	vscalefsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 2d f4    	vscalefss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 2d f4    	vscalefss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 2d f4    	vscalefss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 2d f4    	vscalefss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 2d f4    	vscalefss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 2d f4    	vscalefss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 2d 31    	vscalefss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 2d b4 f0 23 01 00 00 	vscalefss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 15 47 2d 72 7f 	vscalefss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 2d b2 00 02 00 00 	vscalefss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 2d 72 80 	vscalefss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 2d b2 fc fd ff ff 	vscalefss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 03 15 47 55 f4 ab 	vfixupimmss xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 15 c7 55 f4 ab 	vfixupimmss xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 15 17 55 f4 ab 	vfixupimmss xmm30\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 03 15 47 55 f4 7b 	vfixupimmss xmm30\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 03 15 17 55 f4 7b 	vfixupimmss xmm30\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 55 31 7b 	vfixupimmss xmm30\{k7\},xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 23 15 47 55 b4 f0 23 01 00 00 7b 	vfixupimmss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 55 72 7f 7b 	vfixupimmss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 55 b2 00 02 00 00 7b 	vfixupimmss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 55 72 80 7b 	vfixupimmss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 55 b2 fc fd ff ff 7b 	vfixupimmss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 03 95 47 55 f4 ab 	vfixupimmsd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 95 c7 55 f4 ab 	vfixupimmsd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 95 17 55 f4 ab 	vfixupimmsd xmm30\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 03 95 47 55 f4 7b 	vfixupimmsd xmm30\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 03 95 17 55 f4 7b 	vfixupimmsd xmm30\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 55 31 7b 	vfixupimmsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 23 95 47 55 b4 f0 23 01 00 00 7b 	vfixupimmsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 55 72 7f 7b 	vfixupimmsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 55 b2 00 04 00 00 7b 	vfixupimmsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 55 72 80 7b 	vfixupimmsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 55 b2 f8 fb ff ff 7b 	vfixupimmsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 03 95 47 0b f4 ab 	vrndscalesd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 95 c7 0b f4 ab 	vrndscalesd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 95 17 0b f4 ab 	vrndscalesd xmm30\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 03 95 47 0b f4 7b 	vrndscalesd xmm30\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 03 95 17 0b f4 7b 	vrndscalesd xmm30\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 0b 31 7b 	vrndscalesd xmm30\{k7\},xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 23 95 47 0b b4 f0 23 01 00 00 7b 	vrndscalesd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 0b 72 7f 7b 	vrndscalesd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 0b b2 00 04 00 00 7b 	vrndscalesd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 0b 72 80 7b 	vrndscalesd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 0b b2 f8 fb ff ff 7b 	vrndscalesd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 03 15 47 0a f4 ab 	vrndscaless xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 15 c7 0a f4 ab 	vrndscaless xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 15 17 0a f4 ab 	vrndscaless xmm30\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 03 15 47 0a f4 7b 	vrndscaless xmm30\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 03 15 17 0a f4 7b 	vrndscaless xmm30\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 0a 31 7b 	vrndscaless xmm30\{k7\},xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 23 15 47 0a b4 f0 23 01 00 00 7b 	vrndscaless xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 0a 72 7f 7b 	vrndscaless xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 0a b2 00 02 00 00 7b 	vrndscaless xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 0a 72 80 7b 	vrndscaless xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 0a b2 fc fd ff ff 7b 	vrndscaless xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 01 97 47 58 f4    	vaddsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 58 f4    	vaddsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 58 f4    	vaddsd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 97 57 58 f4    	vaddsd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 97 37 58 f4    	vaddsd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 97 77 58 f4    	vaddsd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 58 31    	vaddsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 58 b4 f0 34 12 00 00 	vaddsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 97 47 58 72 7f 	vaddsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 58 b2 00 04 00 00 	vaddsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 58 72 80 	vaddsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 58 b2 f8 fb ff ff 	vaddsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 58 f4    	vaddss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 58 f4    	vaddss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 58 f4    	vaddss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 16 57 58 f4    	vaddss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 16 37 58 f4    	vaddss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 16 77 58 f4    	vaddss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 58 31    	vaddss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 58 b4 f0 34 12 00 00 	vaddss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 16 47 58 72 7f 	vaddss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 58 b2 00 02 00 00 	vaddss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 58 72 80 	vaddss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 58 b2 fc fd ff ff 	vaddss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec ab 	vcmpsd k5\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec ab 	vcmpsd k5\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 7b 	vcmpsd k5\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 7b 	vcmpsd k5\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 7b 	vcmpsd k5\{k7\},xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 7b 	vcmpsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 7b 	vcmpsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 7b 	vcmpsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 7b 	vcmpsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 7b 	vcmpsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 00 	vcmpeqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 00 	vcmpeqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 00 	vcmpeqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 00 	vcmpeqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 00 	vcmpeqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 01 	vcmpltsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 01 	vcmpltsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 01 	vcmpltsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 01 	vcmpltsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 01 	vcmpltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 02 	vcmplesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 02 	vcmplesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 02 	vcmplesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 02 	vcmplesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 02 	vcmplesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 03 	vcmpunordsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 03 	vcmpunordsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 03 	vcmpunordsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 03 	vcmpunordsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 03 	vcmpunordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 04 	vcmpneqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 04 	vcmpneqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 04 	vcmpneqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 04 	vcmpneqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 04 	vcmpneqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 05 	vcmpnltsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 05 	vcmpnltsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 05 	vcmpnltsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 05 	vcmpnltsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 05 	vcmpnltsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 06 	vcmpnlesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 06 	vcmpnlesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 06 	vcmpnlesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 06 	vcmpnlesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 06 	vcmpnlesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 07 	vcmpordsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 07 	vcmpordsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 07 	vcmpordsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 07 	vcmpordsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 07 	vcmpordsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 08 	vcmpeq_uqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 08 	vcmpeq_uqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 08 	vcmpeq_uqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 08 	vcmpeq_uqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 08 	vcmpeq_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 08 	vcmpeq_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 08 	vcmpeq_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 08 	vcmpeq_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 09 	vcmpngesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 09 	vcmpngesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 09 	vcmpngesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 09 	vcmpngesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 09 	vcmpngesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0a 	vcmpngtsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0a 	vcmpngtsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0a 	vcmpngtsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0a 	vcmpngtsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0a 	vcmpngtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0b 	vcmpfalsesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0b 	vcmpfalsesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0c 	vcmpneq_oqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0c 	vcmpneq_oqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0c 	vcmpneq_oqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0c 	vcmpneq_oqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0c 	vcmpneq_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0c 	vcmpneq_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0c 	vcmpneq_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0c 	vcmpneq_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0d 	vcmpgesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0d 	vcmpgesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0d 	vcmpgesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0d 	vcmpgesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0d 	vcmpgesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0e 	vcmpgtsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0e 	vcmpgtsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0e 	vcmpgtsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0e 	vcmpgtsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0e 	vcmpgtsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0f 	vcmptruesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0f 	vcmptruesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0f 	vcmptruesd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0f 	vcmptruesd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0f 	vcmptruesd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 10 	vcmpeq_ossd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 10 	vcmpeq_ossd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 10 	vcmpeq_ossd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 10 	vcmpeq_ossd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 10 	vcmpeq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 10 	vcmpeq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 10 	vcmpeq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 10 	vcmpeq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 11 	vcmplt_oqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 11 	vcmplt_oqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 11 	vcmplt_oqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 11 	vcmplt_oqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 11 	vcmplt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 11 	vcmplt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 11 	vcmplt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 11 	vcmplt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 12 	vcmple_oqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 12 	vcmple_oqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 12 	vcmple_oqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 12 	vcmple_oqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 12 	vcmple_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 12 	vcmple_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 12 	vcmple_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 12 	vcmple_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 13 	vcmpunord_ssd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 13 	vcmpunord_ssd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 13 	vcmpunord_ssd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 13 	vcmpunord_ssd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 13 	vcmpunord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 13 	vcmpunord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 13 	vcmpunord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 13 	vcmpunord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 14 	vcmpneq_ussd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 14 	vcmpneq_ussd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 14 	vcmpneq_ussd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 14 	vcmpneq_ussd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 14 	vcmpneq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 14 	vcmpneq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 14 	vcmpneq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 14 	vcmpneq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 15 	vcmpnlt_uqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 15 	vcmpnlt_uqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 15 	vcmpnlt_uqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 15 	vcmpnlt_uqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 15 	vcmpnlt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 15 	vcmpnlt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 15 	vcmpnlt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 15 	vcmpnlt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 16 	vcmpnle_uqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 16 	vcmpnle_uqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 16 	vcmpnle_uqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 16 	vcmpnle_uqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 16 	vcmpnle_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 16 	vcmpnle_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 16 	vcmpnle_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 16 	vcmpnle_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 17 	vcmpord_ssd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 17 	vcmpord_ssd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 17 	vcmpord_ssd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 17 	vcmpord_ssd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 17 	vcmpord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 17 	vcmpord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 17 	vcmpord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 17 	vcmpord_ssd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 18 	vcmpeq_ussd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 18 	vcmpeq_ussd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 18 	vcmpeq_ussd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 18 	vcmpeq_ussd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 18 	vcmpeq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 18 	vcmpeq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 18 	vcmpeq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 18 	vcmpeq_ussd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 19 	vcmpnge_uqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 19 	vcmpnge_uqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 19 	vcmpnge_uqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 19 	vcmpnge_uqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 19 	vcmpnge_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 19 	vcmpnge_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 19 	vcmpnge_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 19 	vcmpnge_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1a 	vcmpngt_uqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1a 	vcmpngt_uqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1a 	vcmpngt_uqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 1a 	vcmpngt_uqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1a 	vcmpngt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1a 	vcmpngt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1a 	vcmpngt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1a 	vcmpngt_uqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1b 	vcmpfalse_ossd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1b 	vcmpfalse_ossd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1b 	vcmpfalse_ossd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 1b 	vcmpfalse_ossd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1b 	vcmpfalse_ossd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1b 	vcmpfalse_ossd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1b 	vcmpfalse_ossd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1b 	vcmpfalse_ossd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1c 	vcmpneq_ossd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1c 	vcmpneq_ossd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1c 	vcmpneq_ossd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 1c 	vcmpneq_ossd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1c 	vcmpneq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1c 	vcmpneq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1c 	vcmpneq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1c 	vcmpneq_ossd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1d 	vcmpge_oqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1d 	vcmpge_oqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1d 	vcmpge_oqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 1d 	vcmpge_oqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1d 	vcmpge_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1d 	vcmpge_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1d 	vcmpge_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1d 	vcmpge_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1e 	vcmpgt_oqsd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1e 	vcmpgt_oqsd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1e 	vcmpgt_oqsd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 1e 	vcmpgt_oqsd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1e 	vcmpgt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1e 	vcmpgt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1e 	vcmpgt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1e 	vcmpgt_oqsd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1f 	vcmptrue_ussd k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1f 	vcmptrue_ussd k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1f 	vcmptrue_ussd k5\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 1f 	vcmptrue_ussd k5\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1f 	vcmptrue_ussd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1f 	vcmptrue_ussd k5\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1f 	vcmptrue_ussd k5\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1f 	vcmptrue_ussd k5\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec ab 	vcmpss k5\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec ab 	vcmpss k5\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 7b 	vcmpss k5\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 7b 	vcmpss k5\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 7b 	vcmpss k5\{k7\},xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 7b 	vcmpss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 7b 	vcmpss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 7b 	vcmpss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 7b 	vcmpss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 7b 	vcmpss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 00 	vcmpeqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 00 	vcmpeqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 00 	vcmpeqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 00 	vcmpeqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 00 	vcmpeqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 01 	vcmpltss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 01 	vcmpltss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 01 	vcmpltss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 01 	vcmpltss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 01 	vcmpltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 02 	vcmpless k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 02 	vcmpless k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 02 	vcmpless k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 02 	vcmpless k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 02 	vcmpless k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 03 	vcmpunordss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 03 	vcmpunordss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 03 	vcmpunordss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 03 	vcmpunordss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 03 	vcmpunordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 04 	vcmpneqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 04 	vcmpneqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 04 	vcmpneqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 04 	vcmpneqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 04 	vcmpneqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 05 	vcmpnltss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 05 	vcmpnltss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 05 	vcmpnltss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 05 	vcmpnltss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 05 	vcmpnltss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 06 	vcmpnless k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 06 	vcmpnless k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 06 	vcmpnless k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 06 	vcmpnless k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 06 	vcmpnless k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 07 	vcmpordss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 07 	vcmpordss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 07 	vcmpordss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 07 	vcmpordss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 07 	vcmpordss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 08 	vcmpeq_uqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 08 	vcmpeq_uqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 08 	vcmpeq_uqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 08 	vcmpeq_uqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 08 	vcmpeq_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 08 	vcmpeq_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 08 	vcmpeq_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 08 	vcmpeq_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 09 	vcmpngess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 09 	vcmpngess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 09 	vcmpngess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 09 	vcmpngess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 09 	vcmpngess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0a 	vcmpngtss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0a 	vcmpngtss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0a 	vcmpngtss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0a 	vcmpngtss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0a 	vcmpngtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0b 	vcmpfalsess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0b 	vcmpfalsess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0b 	vcmpfalsess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0b 	vcmpfalsess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0b 	vcmpfalsess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0c 	vcmpneq_oqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0c 	vcmpneq_oqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0c 	vcmpneq_oqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0c 	vcmpneq_oqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0c 	vcmpneq_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0c 	vcmpneq_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0c 	vcmpneq_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0c 	vcmpneq_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0d 	vcmpgess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0d 	vcmpgess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0d 	vcmpgess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0d 	vcmpgess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0d 	vcmpgess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0e 	vcmpgtss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0e 	vcmpgtss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0e 	vcmpgtss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0e 	vcmpgtss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0e 	vcmpgtss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0f 	vcmptruess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0f 	vcmptruess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0f 	vcmptruess k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0f 	vcmptruess k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0f 	vcmptruess k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 10 	vcmpeq_osss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 10 	vcmpeq_osss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 10 	vcmpeq_osss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 10 	vcmpeq_osss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 10 	vcmpeq_osss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 10 	vcmpeq_osss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 10 	vcmpeq_osss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 10 	vcmpeq_osss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 11 	vcmplt_oqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 11 	vcmplt_oqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 11 	vcmplt_oqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 11 	vcmplt_oqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 11 	vcmplt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 11 	vcmplt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 11 	vcmplt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 11 	vcmplt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 12 	vcmple_oqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 12 	vcmple_oqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 12 	vcmple_oqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 12 	vcmple_oqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 12 	vcmple_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 12 	vcmple_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 12 	vcmple_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 12 	vcmple_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 13 	vcmpunord_sss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 13 	vcmpunord_sss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 13 	vcmpunord_sss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 13 	vcmpunord_sss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 13 	vcmpunord_sss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 13 	vcmpunord_sss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 13 	vcmpunord_sss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 13 	vcmpunord_sss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 14 	vcmpneq_usss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 14 	vcmpneq_usss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 14 	vcmpneq_usss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 14 	vcmpneq_usss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 14 	vcmpneq_usss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 14 	vcmpneq_usss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 14 	vcmpneq_usss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 14 	vcmpneq_usss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 15 	vcmpnlt_uqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 15 	vcmpnlt_uqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 15 	vcmpnlt_uqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 15 	vcmpnlt_uqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 15 	vcmpnlt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 15 	vcmpnlt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 15 	vcmpnlt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 15 	vcmpnlt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 16 	vcmpnle_uqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 16 	vcmpnle_uqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 16 	vcmpnle_uqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 16 	vcmpnle_uqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 16 	vcmpnle_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 16 	vcmpnle_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 16 	vcmpnle_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 16 	vcmpnle_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 17 	vcmpord_sss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 17 	vcmpord_sss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 17 	vcmpord_sss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 17 	vcmpord_sss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 17 	vcmpord_sss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 17 	vcmpord_sss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 17 	vcmpord_sss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 17 	vcmpord_sss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 18 	vcmpeq_usss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 18 	vcmpeq_usss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 18 	vcmpeq_usss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 18 	vcmpeq_usss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 18 	vcmpeq_usss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 18 	vcmpeq_usss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 18 	vcmpeq_usss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 18 	vcmpeq_usss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 19 	vcmpnge_uqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 19 	vcmpnge_uqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 19 	vcmpnge_uqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 19 	vcmpnge_uqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 19 	vcmpnge_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 19 	vcmpnge_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 19 	vcmpnge_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 19 	vcmpnge_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1a 	vcmpngt_uqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1a 	vcmpngt_uqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1a 	vcmpngt_uqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 1a 	vcmpngt_uqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1a 	vcmpngt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1a 	vcmpngt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1a 	vcmpngt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1a 	vcmpngt_uqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1b 	vcmpfalse_osss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1b 	vcmpfalse_osss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1b 	vcmpfalse_osss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 1b 	vcmpfalse_osss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1b 	vcmpfalse_osss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1b 	vcmpfalse_osss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1b 	vcmpfalse_osss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1b 	vcmpfalse_osss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1c 	vcmpneq_osss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1c 	vcmpneq_osss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1c 	vcmpneq_osss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 1c 	vcmpneq_osss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1c 	vcmpneq_osss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1c 	vcmpneq_osss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1c 	vcmpneq_osss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1c 	vcmpneq_osss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1d 	vcmpge_oqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1d 	vcmpge_oqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1d 	vcmpge_oqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 1d 	vcmpge_oqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1d 	vcmpge_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1d 	vcmpge_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1d 	vcmpge_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1d 	vcmpge_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1e 	vcmpgt_oqss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1e 	vcmpgt_oqss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1e 	vcmpgt_oqss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 1e 	vcmpgt_oqss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1e 	vcmpgt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1e 	vcmpgt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1e 	vcmpgt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1e 	vcmpgt_oqss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1f 	vcmptrue_usss k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1f 	vcmptrue_usss k5\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1f 	vcmptrue_usss k5\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 1f 	vcmptrue_usss k5\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1f 	vcmptrue_usss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1f 	vcmptrue_usss k5\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1f 	vcmptrue_usss k5\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1f 	vcmptrue_usss k5\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 01 fd 48 2f f5    	vcomisd xmm30,xmm29
[ 	]*[a-f0-9]+:	62 01 fd 18 2f f5    	vcomisd xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:	62 61 fd 48 2f 31    	vcomisd xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 fd 48 2f b4 f0 34 12 00 00 	vcomisd xmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2f 72 7f 	vcomisd xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2f b2 00 04 00 00 	vcomisd xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2f 72 80 	vcomisd xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2f b2 f8 fb ff ff 	vcomisd xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 7c 48 2f f5    	vcomiss xmm30,xmm29
[ 	]*[a-f0-9]+:	62 01 7c 18 2f f5    	vcomiss xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:	62 61 7c 48 2f 31    	vcomiss xmm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 7c 48 2f b4 f0 34 12 00 00 	vcomiss xmm30,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2f 72 7f 	vcomiss xmm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2f b2 00 02 00 00 	vcomiss xmm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2f 72 80 	vcomiss xmm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2f b2 fc fd ff ff 	vcomiss xmm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 7f 18 2d c6    	vcvtsd2si eax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 58 2d c6    	vcvtsd2si eax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 38 2d c6    	vcvtsd2si eax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 78 2d c6    	vcvtsd2si eax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 18 2d ee    	vcvtsd2si ebp,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 58 2d ee    	vcvtsd2si ebp,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 38 2d ee    	vcvtsd2si ebp,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 78 2d ee    	vcvtsd2si ebp,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 18 2d ee    	vcvtsd2si r13d,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 58 2d ee    	vcvtsd2si r13d,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 38 2d ee    	vcvtsd2si r13d,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 78 2d ee    	vcvtsd2si r13d,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 18 2d c6    	vcvtsd2si rax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 58 2d c6    	vcvtsd2si rax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 38 2d c6    	vcvtsd2si rax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 78 2d c6    	vcvtsd2si rax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 18 2d c6    	vcvtsd2si r8,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 58 2d c6    	vcvtsd2si r8,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 38 2d c6    	vcvtsd2si r8,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 78 2d c6    	vcvtsd2si r8,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 01 97 47 5a f4    	vcvtsd2ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 5a f4    	vcvtsd2ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 5a f4    	vcvtsd2ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 97 57 5a f4    	vcvtsd2ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 97 37 5a f4    	vcvtsd2ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 97 77 5a f4    	vcvtsd2ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 5a 31    	vcvtsd2ss xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 5a b4 f0 34 12 00 00 	vcvtsd2ss xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 97 47 5a 72 7f 	vcvtsd2ss xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 5a b2 00 04 00 00 	vcvtsd2ss xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5a 72 80 	vcvtsd2ss xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5a b2 f8 fb ff ff 	vcvtsd2ss xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 61 17 40 2a f0    	vcvtsi2sd xmm30,xmm29,eax
[ 	]*[a-f0-9]+:	62 61 17 40 2a f5    	vcvtsi2sd xmm30,xmm29,ebp
[ 	]*[a-f0-9]+:	62 41 17 40 2a f5    	vcvtsi2sd xmm30,xmm29,r13d
[ 	]*[a-f0-9]+:	62 61 17 40 2a 31    	vcvtsi2sd xmm30,xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 17 40 2a b4 f0 34 12 00 00 	vcvtsi2sd xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 17 40 2a 72 7f 	vcvtsi2sd xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 17 40 2a b2 00 02 00 00 	vcvtsi2sd xmm30,xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 17 40 2a 72 80 	vcvtsi2sd xmm30,xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 17 40 2a b2 fc fd ff ff 	vcvtsi2sd xmm30,xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 97 40 2a f0    	vcvtsi2sd xmm30,xmm29,rax
[ 	]*[a-f0-9]+:	62 61 97 10 2a f0    	vcvtsi2sd xmm30,xmm29,rax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 97 50 2a f0    	vcvtsi2sd xmm30,xmm29,rax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 97 30 2a f0    	vcvtsi2sd xmm30,xmm29,rax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 97 70 2a f0    	vcvtsi2sd xmm30,xmm29,rax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 41 97 40 2a f0    	vcvtsi2sd xmm30,xmm29,r8
[ 	]*[a-f0-9]+:	62 41 97 10 2a f0    	vcvtsi2sd xmm30,xmm29,r8\{rn-sae\}
[ 	]*[a-f0-9]+:	62 41 97 50 2a f0    	vcvtsi2sd xmm30,xmm29,r8\{ru-sae\}
[ 	]*[a-f0-9]+:	62 41 97 30 2a f0    	vcvtsi2sd xmm30,xmm29,r8\{rd-sae\}
[ 	]*[a-f0-9]+:	62 41 97 70 2a f0    	vcvtsi2sd xmm30,xmm29,r8\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 40 2a 31    	vcvtsi2sd xmm30,xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 40 2a b4 f0 34 12 00 00 	vcvtsi2sd xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 97 40 2a 72 7f 	vcvtsi2sd xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 40 2a b2 00 04 00 00 	vcvtsi2sd xmm30,xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 40 2a 72 80 	vcvtsi2sd xmm30,xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 40 2a b2 f8 fb ff ff 	vcvtsi2sd xmm30,xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 61 16 40 2a f0    	vcvtsi2ss xmm30,xmm29,eax
[ 	]*[a-f0-9]+:	62 61 16 10 2a f0    	vcvtsi2ss xmm30,xmm29,eax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 16 50 2a f0    	vcvtsi2ss xmm30,xmm29,eax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 16 30 2a f0    	vcvtsi2ss xmm30,xmm29,eax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 16 70 2a f0    	vcvtsi2ss xmm30,xmm29,eax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 40 2a f5    	vcvtsi2ss xmm30,xmm29,ebp
[ 	]*[a-f0-9]+:	62 61 16 10 2a f5    	vcvtsi2ss xmm30,xmm29,ebp\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 16 50 2a f5    	vcvtsi2ss xmm30,xmm29,ebp\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 16 30 2a f5    	vcvtsi2ss xmm30,xmm29,ebp\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 16 70 2a f5    	vcvtsi2ss xmm30,xmm29,ebp\{rz-sae\}
[ 	]*[a-f0-9]+:	62 41 16 40 2a f5    	vcvtsi2ss xmm30,xmm29,r13d
[ 	]*[a-f0-9]+:	62 41 16 10 2a f5    	vcvtsi2ss xmm30,xmm29,r13d\{rn-sae\}
[ 	]*[a-f0-9]+:	62 41 16 50 2a f5    	vcvtsi2ss xmm30,xmm29,r13d\{ru-sae\}
[ 	]*[a-f0-9]+:	62 41 16 30 2a f5    	vcvtsi2ss xmm30,xmm29,r13d\{rd-sae\}
[ 	]*[a-f0-9]+:	62 41 16 70 2a f5    	vcvtsi2ss xmm30,xmm29,r13d\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 40 2a 31    	vcvtsi2ss xmm30,xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 40 2a b4 f0 34 12 00 00 	vcvtsi2ss xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 16 40 2a 72 7f 	vcvtsi2ss xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 40 2a b2 00 02 00 00 	vcvtsi2ss xmm30,xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 40 2a 72 80 	vcvtsi2ss xmm30,xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 40 2a b2 fc fd ff ff 	vcvtsi2ss xmm30,xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 96 40 2a f0    	vcvtsi2ss xmm30,xmm29,rax
[ 	]*[a-f0-9]+:	62 61 96 10 2a f0    	vcvtsi2ss xmm30,xmm29,rax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 96 50 2a f0    	vcvtsi2ss xmm30,xmm29,rax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 96 30 2a f0    	vcvtsi2ss xmm30,xmm29,rax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 96 70 2a f0    	vcvtsi2ss xmm30,xmm29,rax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 41 96 40 2a f0    	vcvtsi2ss xmm30,xmm29,r8
[ 	]*[a-f0-9]+:	62 41 96 10 2a f0    	vcvtsi2ss xmm30,xmm29,r8\{rn-sae\}
[ 	]*[a-f0-9]+:	62 41 96 50 2a f0    	vcvtsi2ss xmm30,xmm29,r8\{ru-sae\}
[ 	]*[a-f0-9]+:	62 41 96 30 2a f0    	vcvtsi2ss xmm30,xmm29,r8\{rd-sae\}
[ 	]*[a-f0-9]+:	62 41 96 70 2a f0    	vcvtsi2ss xmm30,xmm29,r8\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 96 40 2a 31    	vcvtsi2ss xmm30,xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 96 40 2a b4 f0 34 12 00 00 	vcvtsi2ss xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 96 40 2a 72 7f 	vcvtsi2ss xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 96 40 2a b2 00 04 00 00 	vcvtsi2ss xmm30,xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 96 40 2a 72 80 	vcvtsi2ss xmm30,xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 96 40 2a b2 f8 fb ff ff 	vcvtsi2ss xmm30,xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 5a f4    	vcvtss2sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 5a f4    	vcvtss2sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 5a f4    	vcvtss2sd xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 5a 31    	vcvtss2sd xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 5a b4 f0 34 12 00 00 	vcvtss2sd xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 16 47 5a 72 7f 	vcvtss2sd xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 5a b2 00 02 00 00 	vcvtss2sd xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5a 72 80 	vcvtss2sd xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5a b2 fc fd ff ff 	vcvtss2sd xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 7e 18 2d c6    	vcvtss2si eax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 58 2d c6    	vcvtss2si eax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 38 2d c6    	vcvtss2si eax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 78 2d c6    	vcvtss2si eax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 18 2d ee    	vcvtss2si ebp,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 58 2d ee    	vcvtss2si ebp,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 38 2d ee    	vcvtss2si ebp,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 78 2d ee    	vcvtss2si ebp,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 18 2d ee    	vcvtss2si r13d,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 58 2d ee    	vcvtss2si r13d,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 38 2d ee    	vcvtss2si r13d,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 78 2d ee    	vcvtss2si r13d,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 18 2d c6    	vcvtss2si rax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 58 2d c6    	vcvtss2si rax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 38 2d c6    	vcvtss2si rax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 78 2d c6    	vcvtss2si rax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 18 2d c6    	vcvtss2si r8,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 58 2d c6    	vcvtss2si r8,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 38 2d c6    	vcvtss2si r8,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 78 2d c6    	vcvtss2si r8,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 18 2c c6    	vcvttsd2si eax,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 91 7f 18 2c ee    	vcvttsd2si ebp,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 11 7f 18 2c ee    	vcvttsd2si r13d,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 91 ff 18 2c c6    	vcvttsd2si rax,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 11 ff 18 2c c6    	vcvttsd2si r8,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 91 7e 18 2c c6    	vcvttss2si eax,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 91 7e 18 2c ee    	vcvttss2si ebp,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 11 7e 18 2c ee    	vcvttss2si r13d,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 91 fe 18 2c c6    	vcvttss2si rax,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 11 fe 18 2c c6    	vcvttss2si r8,xmm30\{sae\}
[ 	]*[a-f0-9]+:	62 01 97 47 5e f4    	vdivsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 5e f4    	vdivsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 5e f4    	vdivsd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 97 57 5e f4    	vdivsd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 97 37 5e f4    	vdivsd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 97 77 5e f4    	vdivsd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 5e 31    	vdivsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 5e b4 f0 34 12 00 00 	vdivsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 97 47 5e 72 7f 	vdivsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 5e b2 00 04 00 00 	vdivsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5e 72 80 	vdivsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5e b2 f8 fb ff ff 	vdivsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 5e f4    	vdivss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 5e f4    	vdivss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 5e f4    	vdivss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 16 57 5e f4    	vdivss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 16 37 5e f4    	vdivss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 16 77 5e f4    	vdivss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 5e 31    	vdivss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 5e b4 f0 34 12 00 00 	vdivss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 16 47 5e 72 7f 	vdivss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 5e b2 00 02 00 00 	vdivss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5e 72 80 	vdivss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5e b2 fc fd ff ff 	vdivss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 99 f4    	vfmadd132sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 99 f4    	vfmadd132sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 99 f4    	vfmadd132sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 99 f4    	vfmadd132sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 99 f4    	vfmadd132sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 99 f4    	vfmadd132sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 99 31    	vfmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 99 b4 f0 34 12 00 00 	vfmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 99 72 7f 	vfmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 99 b2 00 04 00 00 	vfmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 99 72 80 	vfmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 99 b2 f8 fb ff ff 	vfmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 99 f4    	vfmadd132ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 99 f4    	vfmadd132ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 99 f4    	vfmadd132ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 99 f4    	vfmadd132ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 99 f4    	vfmadd132ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 99 f4    	vfmadd132ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 99 31    	vfmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 99 b4 f0 34 12 00 00 	vfmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 99 72 7f 	vfmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 99 b2 00 02 00 00 	vfmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 99 72 80 	vfmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 99 b2 fc fd ff ff 	vfmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 a9 f4    	vfmadd213sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 a9 f4    	vfmadd213sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 a9 f4    	vfmadd213sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 a9 f4    	vfmadd213sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 a9 f4    	vfmadd213sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 a9 f4    	vfmadd213sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 a9 31    	vfmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 a9 b4 f0 34 12 00 00 	vfmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 a9 72 7f 	vfmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 a9 b2 00 04 00 00 	vfmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 a9 72 80 	vfmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 a9 b2 f8 fb ff ff 	vfmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 a9 f4    	vfmadd213ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 a9 f4    	vfmadd213ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 a9 f4    	vfmadd213ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 a9 f4    	vfmadd213ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 a9 f4    	vfmadd213ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 a9 f4    	vfmadd213ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 a9 31    	vfmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 a9 b4 f0 34 12 00 00 	vfmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 a9 72 7f 	vfmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 a9 b2 00 02 00 00 	vfmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 a9 72 80 	vfmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 a9 b2 fc fd ff ff 	vfmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 b9 f4    	vfmadd231sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 b9 f4    	vfmadd231sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 b9 f4    	vfmadd231sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 b9 f4    	vfmadd231sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 b9 f4    	vfmadd231sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 b9 f4    	vfmadd231sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 b9 31    	vfmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 b9 b4 f0 34 12 00 00 	vfmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 b9 72 7f 	vfmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 b9 b2 00 04 00 00 	vfmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 b9 72 80 	vfmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 b9 b2 f8 fb ff ff 	vfmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 b9 f4    	vfmadd231ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 b9 f4    	vfmadd231ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 b9 f4    	vfmadd231ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 b9 f4    	vfmadd231ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 b9 f4    	vfmadd231ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 b9 f4    	vfmadd231ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 b9 31    	vfmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 b9 b4 f0 34 12 00 00 	vfmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 b9 72 7f 	vfmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 b9 b2 00 02 00 00 	vfmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 b9 72 80 	vfmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 b9 b2 fc fd ff ff 	vfmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 9b f4    	vfmsub132sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 9b f4    	vfmsub132sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 9b f4    	vfmsub132sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 9b f4    	vfmsub132sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 9b f4    	vfmsub132sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 9b f4    	vfmsub132sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 9b 31    	vfmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 9b b4 f0 34 12 00 00 	vfmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 9b 72 7f 	vfmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 9b b2 00 04 00 00 	vfmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 9b 72 80 	vfmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 9b b2 f8 fb ff ff 	vfmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 9b f4    	vfmsub132ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 9b f4    	vfmsub132ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 9b f4    	vfmsub132ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 9b f4    	vfmsub132ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 9b f4    	vfmsub132ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 9b f4    	vfmsub132ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 9b 31    	vfmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 9b b4 f0 34 12 00 00 	vfmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 9b 72 7f 	vfmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 9b b2 00 02 00 00 	vfmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 9b 72 80 	vfmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 9b b2 fc fd ff ff 	vfmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 ab f4    	vfmsub213sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 ab f4    	vfmsub213sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 ab f4    	vfmsub213sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 ab f4    	vfmsub213sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 ab f4    	vfmsub213sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 ab f4    	vfmsub213sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 ab 31    	vfmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 ab b4 f0 34 12 00 00 	vfmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 ab 72 7f 	vfmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 ab b2 00 04 00 00 	vfmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 ab 72 80 	vfmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 ab b2 f8 fb ff ff 	vfmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 ab f4    	vfmsub213ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 ab f4    	vfmsub213ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 ab f4    	vfmsub213ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 ab f4    	vfmsub213ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 ab f4    	vfmsub213ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 ab f4    	vfmsub213ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 ab 31    	vfmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 ab b4 f0 34 12 00 00 	vfmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 ab 72 7f 	vfmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 ab b2 00 02 00 00 	vfmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 ab 72 80 	vfmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 ab b2 fc fd ff ff 	vfmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 bb f4    	vfmsub231sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 bb f4    	vfmsub231sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 bb f4    	vfmsub231sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 bb f4    	vfmsub231sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 bb f4    	vfmsub231sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 bb f4    	vfmsub231sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 bb 31    	vfmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 bb b4 f0 34 12 00 00 	vfmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 bb 72 7f 	vfmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 bb b2 00 04 00 00 	vfmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 bb 72 80 	vfmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 bb b2 f8 fb ff ff 	vfmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 bb f4    	vfmsub231ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 bb f4    	vfmsub231ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 bb f4    	vfmsub231ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 bb f4    	vfmsub231ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 bb f4    	vfmsub231ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 bb f4    	vfmsub231ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 bb 31    	vfmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 bb b4 f0 34 12 00 00 	vfmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 bb 72 7f 	vfmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 bb b2 00 02 00 00 	vfmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 bb 72 80 	vfmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 bb b2 fc fd ff ff 	vfmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 9d f4    	vfnmadd132sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 9d f4    	vfnmadd132sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 9d f4    	vfnmadd132sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 9d f4    	vfnmadd132sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 9d f4    	vfnmadd132sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 9d f4    	vfnmadd132sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 9d 31    	vfnmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 9d b4 f0 34 12 00 00 	vfnmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 9d 72 7f 	vfnmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 9d b2 00 04 00 00 	vfnmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 9d 72 80 	vfnmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 9d b2 f8 fb ff ff 	vfnmadd132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 9d f4    	vfnmadd132ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 9d f4    	vfnmadd132ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 9d f4    	vfnmadd132ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 9d f4    	vfnmadd132ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 9d f4    	vfnmadd132ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 9d f4    	vfnmadd132ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 9d 31    	vfnmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 9d b4 f0 34 12 00 00 	vfnmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 9d 72 7f 	vfnmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 9d b2 00 02 00 00 	vfnmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 9d 72 80 	vfnmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 9d b2 fc fd ff ff 	vfnmadd132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 ad f4    	vfnmadd213sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 ad f4    	vfnmadd213sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 ad f4    	vfnmadd213sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 ad f4    	vfnmadd213sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 ad f4    	vfnmadd213sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 ad f4    	vfnmadd213sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 ad 31    	vfnmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 ad b4 f0 34 12 00 00 	vfnmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 ad 72 7f 	vfnmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 ad b2 00 04 00 00 	vfnmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 ad 72 80 	vfnmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 ad b2 f8 fb ff ff 	vfnmadd213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 ad f4    	vfnmadd213ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 ad f4    	vfnmadd213ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 ad f4    	vfnmadd213ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 ad f4    	vfnmadd213ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 ad f4    	vfnmadd213ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 ad f4    	vfnmadd213ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 ad 31    	vfnmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 ad b4 f0 34 12 00 00 	vfnmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 ad 72 7f 	vfnmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 ad b2 00 02 00 00 	vfnmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 ad 72 80 	vfnmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 ad b2 fc fd ff ff 	vfnmadd213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 bd f4    	vfnmadd231sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 bd f4    	vfnmadd231sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 bd f4    	vfnmadd231sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 bd f4    	vfnmadd231sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 bd f4    	vfnmadd231sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 bd f4    	vfnmadd231sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 bd 31    	vfnmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 bd b4 f0 34 12 00 00 	vfnmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 bd 72 7f 	vfnmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 bd b2 00 04 00 00 	vfnmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 bd 72 80 	vfnmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 bd b2 f8 fb ff ff 	vfnmadd231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 bd f4    	vfnmadd231ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 bd f4    	vfnmadd231ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 bd f4    	vfnmadd231ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 bd f4    	vfnmadd231ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 bd f4    	vfnmadd231ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 bd f4    	vfnmadd231ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 bd 31    	vfnmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 bd b4 f0 34 12 00 00 	vfnmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 bd 72 7f 	vfnmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 bd b2 00 02 00 00 	vfnmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 bd 72 80 	vfnmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 bd b2 fc fd ff ff 	vfnmadd231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 9f f4    	vfnmsub132sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 9f f4    	vfnmsub132sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 9f f4    	vfnmsub132sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 9f f4    	vfnmsub132sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 9f f4    	vfnmsub132sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 9f f4    	vfnmsub132sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 9f 31    	vfnmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 9f b4 f0 34 12 00 00 	vfnmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 9f 72 7f 	vfnmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 9f b2 00 04 00 00 	vfnmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 9f 72 80 	vfnmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 9f b2 f8 fb ff ff 	vfnmsub132sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 9f f4    	vfnmsub132ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 9f f4    	vfnmsub132ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 9f f4    	vfnmsub132ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 9f f4    	vfnmsub132ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 9f f4    	vfnmsub132ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 9f f4    	vfnmsub132ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 9f 31    	vfnmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 9f b4 f0 34 12 00 00 	vfnmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 9f 72 7f 	vfnmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 9f b2 00 02 00 00 	vfnmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 9f 72 80 	vfnmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 9f b2 fc fd ff ff 	vfnmsub132ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 af f4    	vfnmsub213sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 af f4    	vfnmsub213sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 af f4    	vfnmsub213sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 af f4    	vfnmsub213sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 af f4    	vfnmsub213sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 af f4    	vfnmsub213sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 af 31    	vfnmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 af b4 f0 34 12 00 00 	vfnmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 af 72 7f 	vfnmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 af b2 00 04 00 00 	vfnmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 af 72 80 	vfnmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 af b2 f8 fb ff ff 	vfnmsub213sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 af f4    	vfnmsub213ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 af f4    	vfnmsub213ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 af f4    	vfnmsub213ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 af f4    	vfnmsub213ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 af f4    	vfnmsub213ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 af f4    	vfnmsub213ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 af 31    	vfnmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 af b4 f0 34 12 00 00 	vfnmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 af 72 7f 	vfnmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 af b2 00 02 00 00 	vfnmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 af 72 80 	vfnmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 af b2 fc fd ff ff 	vfnmsub213ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 bf f4    	vfnmsub231sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 bf f4    	vfnmsub231sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 bf f4    	vfnmsub231sd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 bf f4    	vfnmsub231sd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 bf f4    	vfnmsub231sd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 bf f4    	vfnmsub231sd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 bf 31    	vfnmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 bf b4 f0 34 12 00 00 	vfnmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 bf 72 7f 	vfnmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 bf b2 00 04 00 00 	vfnmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 bf 72 80 	vfnmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 bf b2 f8 fb ff ff 	vfnmsub231sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 bf f4    	vfnmsub231ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 bf f4    	vfnmsub231ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 bf f4    	vfnmsub231ss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 bf f4    	vfnmsub231ss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 bf f4    	vfnmsub231ss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 bf f4    	vfnmsub231ss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 bf 31    	vfnmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 bf b4 f0 34 12 00 00 	vfnmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 bf 72 7f 	vfnmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 bf b2 00 02 00 00 	vfnmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 bf 72 80 	vfnmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 bf b2 fc fd ff ff 	vfnmsub231ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 43 f4    	vgetexpsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 43 f4    	vgetexpsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 43 f4    	vgetexpsd xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 43 31    	vgetexpsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 43 b4 f0 34 12 00 00 	vgetexpsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 43 72 7f 	vgetexpsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 43 b2 00 04 00 00 	vgetexpsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 43 72 80 	vgetexpsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 43 b2 f8 fb ff ff 	vgetexpsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 43 f4    	vgetexpss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 43 f4    	vgetexpss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 43 f4    	vgetexpss xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 43 31    	vgetexpss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 43 b4 f0 34 12 00 00 	vgetexpss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 43 72 7f 	vgetexpss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 43 b2 00 02 00 00 	vgetexpss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 43 72 80 	vgetexpss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 43 b2 fc fd ff ff 	vgetexpss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 03 95 47 27 f4 ab 	vgetmantsd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 95 c7 27 f4 ab 	vgetmantsd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 95 17 27 f4 ab 	vgetmantsd xmm30\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 03 95 47 27 f4 7b 	vgetmantsd xmm30\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 03 95 17 27 f4 7b 	vgetmantsd xmm30\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 27 31 7b 	vgetmantsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 23 95 47 27 b4 f0 34 12 00 00 7b 	vgetmantsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 27 72 7f 7b 	vgetmantsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 27 b2 00 04 00 00 7b 	vgetmantsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 27 72 80 7b 	vgetmantsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 27 b2 f8 fb ff ff 7b 	vgetmantsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 03 15 47 27 f4 ab 	vgetmantss xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 15 c7 27 f4 ab 	vgetmantss xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 15 17 27 f4 ab 	vgetmantss xmm30\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 03 15 47 27 f4 7b 	vgetmantss xmm30\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 03 15 17 27 f4 7b 	vgetmantss xmm30\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 27 31 7b 	vgetmantss xmm30\{k7\},xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 23 15 47 27 b4 f0 34 12 00 00 7b 	vgetmantss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 27 72 7f 7b 	vgetmantss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 27 b2 00 02 00 00 7b 	vgetmantss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 27 72 80 7b 	vgetmantss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 27 b2 fc fd ff ff 7b 	vgetmantss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 01 97 47 5f f4    	vmaxsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 5f f4    	vmaxsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 5f f4    	vmaxsd xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 5f 31    	vmaxsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 5f b4 f0 34 12 00 00 	vmaxsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 97 47 5f 72 7f 	vmaxsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 5f b2 00 04 00 00 	vmaxsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5f 72 80 	vmaxsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5f b2 f8 fb ff ff 	vmaxsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 5f f4    	vmaxss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 5f f4    	vmaxss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 5f f4    	vmaxss xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 5f 31    	vmaxss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 5f b4 f0 34 12 00 00 	vmaxss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 16 47 5f 72 7f 	vmaxss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 5f b2 00 02 00 00 	vmaxss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5f 72 80 	vmaxss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5f b2 fc fd ff ff 	vmaxss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 01 97 47 5d f4    	vminsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 5d f4    	vminsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 5d f4    	vminsd xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 5d 31    	vminsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 5d b4 f0 34 12 00 00 	vminsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 97 47 5d 72 7f 	vminsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 5d b2 00 04 00 00 	vminsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5d 72 80 	vminsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5d b2 f8 fb ff ff 	vminsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 5d f4    	vminss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 5d f4    	vminss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 5d f4    	vminss xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 5d 31    	vminss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 5d b4 f0 34 12 00 00 	vminss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 16 47 5d 72 7f 	vminss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 5d b2 00 02 00 00 	vminss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5d 72 80 	vminss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5d b2 fc fd ff ff 	vminss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 31    	vmovsd xmm30\{k7\},QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 61 ff cf 10 31    	vmovsd xmm30\{k7\}\{z\},QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 ff 4f 10 b4 f0 34 12 00 00 	vmovsd xmm30\{k7\},QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 72 7f 	vmovsd xmm30\{k7\},QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 b2 00 04 00 00 	vmovsd xmm30\{k7\},QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 72 80 	vmovsd xmm30\{k7\},QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 b2 f8 fb ff ff 	vmovsd xmm30\{k7\},QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 31    	vmovsd QWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 21 ff 4f 11 b4 f0 34 12 00 00 	vmovsd QWORD PTR \[rax\+r14\*8\+0x1234\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 72 7f 	vmovsd QWORD PTR \[rdx\+0x3f8\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 b2 00 04 00 00 	vmovsd QWORD PTR \[rdx\+0x400\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 72 80 	vmovsd QWORD PTR \[rdx-0x400\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 b2 f8 fb ff ff 	vmovsd QWORD PTR \[rdx-0x408\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 01 97 47 10 f4    	vmovsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 10 f4    	vmovsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 31    	vmovss xmm30\{k7\},DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 61 7e cf 10 31    	vmovss xmm30\{k7\}\{z\},DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 7e 4f 10 b4 f0 34 12 00 00 	vmovss xmm30\{k7\},DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 72 7f 	vmovss xmm30\{k7\},DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 b2 00 02 00 00 	vmovss xmm30\{k7\},DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 72 80 	vmovss xmm30\{k7\},DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 b2 fc fd ff ff 	vmovss xmm30\{k7\},DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 31    	vmovss DWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 21 7e 4f 11 b4 f0 34 12 00 00 	vmovss DWORD PTR \[rax\+r14\*8\+0x1234\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 72 7f 	vmovss DWORD PTR \[rdx\+0x1fc\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 b2 00 02 00 00 	vmovss DWORD PTR \[rdx\+0x200\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 72 80 	vmovss DWORD PTR \[rdx-0x200\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 b2 fc fd ff ff 	vmovss DWORD PTR \[rdx-0x204\]\{k7\},xmm30
[ 	]*[a-f0-9]+:	62 01 16 47 10 f4    	vmovss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 10 f4    	vmovss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 47 59 f4    	vmulsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 59 f4    	vmulsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 59 f4    	vmulsd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 97 57 59 f4    	vmulsd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 97 37 59 f4    	vmulsd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 97 77 59 f4    	vmulsd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 59 31    	vmulsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 59 b4 f0 34 12 00 00 	vmulsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 97 47 59 72 7f 	vmulsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 59 b2 00 04 00 00 	vmulsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 59 72 80 	vmulsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 59 b2 f8 fb ff ff 	vmulsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 59 f4    	vmulss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 59 f4    	vmulss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 59 f4    	vmulss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 16 57 59 f4    	vmulss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 16 37 59 f4    	vmulss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 16 77 59 f4    	vmulss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 59 31    	vmulss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 59 b4 f0 34 12 00 00 	vmulss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 16 47 59 72 7f 	vmulss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 59 b2 00 02 00 00 	vmulss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 59 72 80 	vmulss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 59 b2 fc fd ff ff 	vmulss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 4d f4    	vrcp14sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 4d f4    	vrcp14sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 62 95 47 4d 31    	vrcp14sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 4d b4 f0 34 12 00 00 	vrcp14sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 4d 72 7f 	vrcp14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 4d b2 00 04 00 00 	vrcp14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 4d 72 80 	vrcp14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 4d b2 f8 fb ff ff 	vrcp14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 4d f4    	vrcp14ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 4d f4    	vrcp14ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 62 15 47 4d 31    	vrcp14ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 4d b4 f0 34 12 00 00 	vrcp14ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 4d 72 7f 	vrcp14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 4d b2 00 02 00 00 	vrcp14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 4d 72 80 	vrcp14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 4d b2 fc fd ff ff 	vrcp14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 15 47 cb f4    	vrcp28ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 cb f4    	vrcp28ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 cb f4    	vrcp28ss xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 cb 31    	vrcp28ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 cb b4 f0 34 12 00 00 	vrcp28ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 cb 72 7f 	vrcp28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 cb b2 00 02 00 00 	vrcp28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 cb 72 80 	vrcp28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 cb b2 fc fd ff ff 	vrcp28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 cb f4    	vrcp28sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 cb f4    	vrcp28sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 cb f4    	vrcp28sd xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 cb 31    	vrcp28sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 cb b4 f0 34 12 00 00 	vrcp28sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 cb 72 7f 	vrcp28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 cb b2 00 04 00 00 	vrcp28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 cb 72 80 	vrcp28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 cb b2 f8 fb ff ff 	vrcp28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 95 47 4f f4    	vrsqrt14sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 4f f4    	vrsqrt14sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 62 95 47 4f 31    	vrsqrt14sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 4f b4 f0 34 12 00 00 	vrsqrt14sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 4f 72 7f 	vrsqrt14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 4f b2 00 04 00 00 	vrsqrt14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 4f 72 80 	vrsqrt14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 4f b2 f8 fb ff ff 	vrsqrt14sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 4f f4    	vrsqrt14ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 4f f4    	vrsqrt14ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 62 15 47 4f 31    	vrsqrt14ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 4f b4 f0 34 12 00 00 	vrsqrt14ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 4f 72 7f 	vrsqrt14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 4f b2 00 02 00 00 	vrsqrt14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 4f 72 80 	vrsqrt14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 4f b2 fc fd ff ff 	vrsqrt14ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 15 47 cd f4    	vrsqrt28ss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 cd f4    	vrsqrt28ss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 cd f4    	vrsqrt28ss xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 cd 31    	vrsqrt28ss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 cd b4 f0 34 12 00 00 	vrsqrt28ss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 cd 72 7f 	vrsqrt28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 cd b2 00 02 00 00 	vrsqrt28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 cd 72 80 	vrsqrt28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 cd b2 fc fd ff ff 	vrsqrt28ss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 95 47 cd f4    	vrsqrt28sd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 cd f4    	vrsqrt28sd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 cd f4    	vrsqrt28sd xmm30\{k7\},xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 cd 31    	vrsqrt28sd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 cd b4 f0 34 12 00 00 	vrsqrt28sd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 cd 72 7f 	vrsqrt28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 cd b2 00 04 00 00 	vrsqrt28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 cd 72 80 	vrsqrt28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 cd b2 f8 fb ff ff 	vrsqrt28sd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 97 47 51 f4    	vsqrtsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 51 f4    	vsqrtsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 51 f4    	vsqrtsd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 97 57 51 f4    	vsqrtsd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 97 37 51 f4    	vsqrtsd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 97 77 51 f4    	vsqrtsd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 51 31    	vsqrtsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 51 b4 f0 34 12 00 00 	vsqrtsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 97 47 51 72 7f 	vsqrtsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 51 b2 00 04 00 00 	vsqrtsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 51 72 80 	vsqrtsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 51 b2 f8 fb ff ff 	vsqrtsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 51 f4    	vsqrtss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 51 f4    	vsqrtss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 51 f4    	vsqrtss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 16 57 51 f4    	vsqrtss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 16 37 51 f4    	vsqrtss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 16 77 51 f4    	vsqrtss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 51 31    	vsqrtss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 51 b4 f0 34 12 00 00 	vsqrtss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 16 47 51 72 7f 	vsqrtss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 51 b2 00 02 00 00 	vsqrtss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 51 72 80 	vsqrtss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 51 b2 fc fd ff ff 	vsqrtss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 01 97 47 5c f4    	vsubsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 c7 5c f4    	vsubsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 97 17 5c f4    	vsubsd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 97 57 5c f4    	vsubsd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 97 37 5c f4    	vsubsd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 97 77 5c f4    	vsubsd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 47 5c 31    	vsubsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 47 5c b4 f0 34 12 00 00 	vsubsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 97 47 5c 72 7f 	vsubsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 47 5c b2 00 04 00 00 	vsubsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5c 72 80 	vsubsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 47 5c b2 f8 fb ff ff 	vsubsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 16 47 5c f4    	vsubss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 c7 5c f4    	vsubss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 01 16 17 5c f4    	vsubss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 01 16 57 5c f4    	vsubss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 01 16 37 5c f4    	vsubss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 01 16 77 5c f4    	vsubss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 47 5c 31    	vsubss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 47 5c b4 f0 34 12 00 00 	vsubss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 16 47 5c 72 7f 	vsubss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 47 5c b2 00 02 00 00 	vsubss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5c 72 80 	vsubss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 47 5c b2 fc fd ff ff 	vsubss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 01 fd 48 2e f5    	vucomisd xmm30,xmm29
[ 	]*[a-f0-9]+:	62 01 fd 18 2e f5    	vucomisd xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:	62 61 fd 48 2e 31    	vucomisd xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 fd 48 2e b4 f0 34 12 00 00 	vucomisd xmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2e 72 7f 	vucomisd xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2e b2 00 04 00 00 	vucomisd xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2e 72 80 	vucomisd xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 fd 48 2e b2 f8 fb ff ff 	vucomisd xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 01 7c 48 2e f5    	vucomiss xmm30,xmm29
[ 	]*[a-f0-9]+:	62 01 7c 18 2e f5    	vucomiss xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:	62 61 7c 48 2e 31    	vucomiss xmm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 7c 48 2e b4 f0 34 12 00 00 	vucomiss xmm30,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2e 72 7f 	vucomiss xmm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2e b2 00 02 00 00 	vucomiss xmm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2e 72 80 	vucomiss xmm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 7c 48 2e b2 fc fd ff ff 	vucomiss xmm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 7f 48 79 c6    	vcvtsd2usi eax,xmm30
[ 	]*[a-f0-9]+:	62 91 7f 18 79 c6    	vcvtsd2usi eax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 58 79 c6    	vcvtsd2usi eax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 38 79 c6    	vcvtsd2usi eax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 78 79 c6    	vcvtsd2usi eax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 01    	vcvtsd2usi eax,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 7f 48 79 84 f0 34 12 00 00 	vcvtsd2usi eax,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 42 7f 	vcvtsd2usi eax,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 82 00 04 00 00 	vcvtsd2usi eax,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 42 80 	vcvtsd2usi eax,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 82 f8 fb ff ff 	vcvtsd2usi eax,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 7f 48 79 ee    	vcvtsd2usi ebp,xmm30
[ 	]*[a-f0-9]+:	62 91 7f 18 79 ee    	vcvtsd2usi ebp,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 58 79 ee    	vcvtsd2usi ebp,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 38 79 ee    	vcvtsd2usi ebp,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7f 78 79 ee    	vcvtsd2usi ebp,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 29    	vcvtsd2usi ebp,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 7f 48 79 ac f0 34 12 00 00 	vcvtsd2usi ebp,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 6a 7f 	vcvtsd2usi ebp,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 aa 00 04 00 00 	vcvtsd2usi ebp,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 6a 80 	vcvtsd2usi ebp,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 aa f8 fb ff ff 	vcvtsd2usi ebp,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 11 7f 48 79 ee    	vcvtsd2usi r13d,xmm30
[ 	]*[a-f0-9]+:	62 11 7f 18 79 ee    	vcvtsd2usi r13d,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 58 79 ee    	vcvtsd2usi r13d,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 38 79 ee    	vcvtsd2usi r13d,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 7f 78 79 ee    	vcvtsd2usi r13d,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 71 7f 48 79 29    	vcvtsd2usi r13d,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 31 7f 48 79 ac f0 34 12 00 00 	vcvtsd2usi r13d,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 71 7f 48 79 6a 7f 	vcvtsd2usi r13d,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 71 7f 48 79 aa 00 04 00 00 	vcvtsd2usi r13d,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 71 7f 48 79 6a 80 	vcvtsd2usi r13d,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 71 7f 48 79 aa f8 fb ff ff 	vcvtsd2usi r13d,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 ff 48 79 c6    	vcvtsd2usi rax,xmm30
[ 	]*[a-f0-9]+:	62 91 ff 18 79 c6    	vcvtsd2usi rax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 58 79 c6    	vcvtsd2usi rax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 38 79 c6    	vcvtsd2usi rax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 ff 78 79 c6    	vcvtsd2usi rax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 01    	vcvtsd2usi rax,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 ff 48 79 84 f0 34 12 00 00 	vcvtsd2usi rax,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 42 7f 	vcvtsd2usi rax,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 82 00 04 00 00 	vcvtsd2usi rax,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 42 80 	vcvtsd2usi rax,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 82 f8 fb ff ff 	vcvtsd2usi rax,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 11 ff 48 79 c6    	vcvtsd2usi r8,xmm30
[ 	]*[a-f0-9]+:	62 11 ff 18 79 c6    	vcvtsd2usi r8,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 58 79 c6    	vcvtsd2usi r8,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 38 79 c6    	vcvtsd2usi r8,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 ff 78 79 c6    	vcvtsd2usi r8,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 71 ff 48 79 01    	vcvtsd2usi r8,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 31 ff 48 79 84 f0 34 12 00 00 	vcvtsd2usi r8,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 71 ff 48 79 42 7f 	vcvtsd2usi r8,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 71 ff 48 79 82 00 04 00 00 	vcvtsd2usi r8,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 71 ff 48 79 42 80 	vcvtsd2usi r8,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 71 ff 48 79 82 f8 fb ff ff 	vcvtsd2usi r8,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 91 7e 48 79 c6    	vcvtss2usi eax,xmm30
[ 	]*[a-f0-9]+:	62 91 7e 18 79 c6    	vcvtss2usi eax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 58 79 c6    	vcvtss2usi eax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 38 79 c6    	vcvtss2usi eax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 78 79 c6    	vcvtss2usi eax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 01    	vcvtss2usi eax,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 7e 48 79 84 f0 34 12 00 00 	vcvtss2usi eax,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 42 7f 	vcvtss2usi eax,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 82 00 02 00 00 	vcvtss2usi eax,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 42 80 	vcvtss2usi eax,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 82 fc fd ff ff 	vcvtss2usi eax,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 7e 48 79 ee    	vcvtss2usi ebp,xmm30
[ 	]*[a-f0-9]+:	62 91 7e 18 79 ee    	vcvtss2usi ebp,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 58 79 ee    	vcvtss2usi ebp,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 38 79 ee    	vcvtss2usi ebp,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 7e 78 79 ee    	vcvtss2usi ebp,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 29    	vcvtss2usi ebp,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 7e 48 79 ac f0 34 12 00 00 	vcvtss2usi ebp,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 6a 7f 	vcvtss2usi ebp,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 aa 00 02 00 00 	vcvtss2usi ebp,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 6a 80 	vcvtss2usi ebp,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 aa fc fd ff ff 	vcvtss2usi ebp,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 11 7e 48 79 ee    	vcvtss2usi r13d,xmm30
[ 	]*[a-f0-9]+:	62 11 7e 18 79 ee    	vcvtss2usi r13d,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 58 79 ee    	vcvtss2usi r13d,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 38 79 ee    	vcvtss2usi r13d,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 7e 78 79 ee    	vcvtss2usi r13d,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 71 7e 48 79 29    	vcvtss2usi r13d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 31 7e 48 79 ac f0 34 12 00 00 	vcvtss2usi r13d,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 71 7e 48 79 6a 7f 	vcvtss2usi r13d,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 71 7e 48 79 aa 00 02 00 00 	vcvtss2usi r13d,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 71 7e 48 79 6a 80 	vcvtss2usi r13d,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 71 7e 48 79 aa fc fd ff ff 	vcvtss2usi r13d,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 91 fe 48 79 c6    	vcvtss2usi rax,xmm30
[ 	]*[a-f0-9]+:	62 91 fe 18 79 c6    	vcvtss2usi rax,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 58 79 c6    	vcvtss2usi rax,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 38 79 c6    	vcvtss2usi rax,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 91 fe 78 79 c6    	vcvtss2usi rax,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 01    	vcvtss2usi rax,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 b1 fe 48 79 84 f0 34 12 00 00 	vcvtss2usi rax,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 42 7f 	vcvtss2usi rax,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 82 00 02 00 00 	vcvtss2usi rax,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 42 80 	vcvtss2usi rax,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 82 fc fd ff ff 	vcvtss2usi rax,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 11 fe 48 79 c6    	vcvtss2usi r8,xmm30
[ 	]*[a-f0-9]+:	62 11 fe 18 79 c6    	vcvtss2usi r8,xmm30\{rn-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 58 79 c6    	vcvtss2usi r8,xmm30\{ru-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 38 79 c6    	vcvtss2usi r8,xmm30\{rd-sae\}
[ 	]*[a-f0-9]+:	62 11 fe 78 79 c6    	vcvtss2usi r8,xmm30\{rz-sae\}
[ 	]*[a-f0-9]+:	62 71 fe 48 79 01    	vcvtss2usi r8,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 31 fe 48 79 84 f0 34 12 00 00 	vcvtss2usi r8,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 71 fe 48 79 42 7f 	vcvtss2usi r8,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 71 fe 48 79 82 00 02 00 00 	vcvtss2usi r8,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 71 fe 48 79 42 80 	vcvtss2usi r8,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 71 fe 48 79 82 fc fd ff ff 	vcvtss2usi r8,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 17 40 7b f0    	vcvtusi2sd xmm30,xmm29,eax
[ 	]*[a-f0-9]+:	62 61 17 40 7b f5    	vcvtusi2sd xmm30,xmm29,ebp
[ 	]*[a-f0-9]+:	62 41 17 40 7b f5    	vcvtusi2sd xmm30,xmm29,r13d
[ 	]*[a-f0-9]+:	62 61 17 40 7b 31    	vcvtusi2sd xmm30,xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 17 40 7b b4 f0 34 12 00 00 	vcvtusi2sd xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 17 40 7b 72 7f 	vcvtusi2sd xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 17 40 7b b2 00 02 00 00 	vcvtusi2sd xmm30,xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 17 40 7b 72 80 	vcvtusi2sd xmm30,xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 17 40 7b b2 fc fd ff ff 	vcvtusi2sd xmm30,xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 97 40 7b f0    	vcvtusi2sd xmm30,xmm29,rax
[ 	]*[a-f0-9]+:	62 61 97 10 7b f0    	vcvtusi2sd xmm30,xmm29,rax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 97 50 7b f0    	vcvtusi2sd xmm30,xmm29,rax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 97 30 7b f0    	vcvtusi2sd xmm30,xmm29,rax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 97 70 7b f0    	vcvtusi2sd xmm30,xmm29,rax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 41 97 40 7b f0    	vcvtusi2sd xmm30,xmm29,r8
[ 	]*[a-f0-9]+:	62 41 97 10 7b f0    	vcvtusi2sd xmm30,xmm29,r8\{rn-sae\}
[ 	]*[a-f0-9]+:	62 41 97 50 7b f0    	vcvtusi2sd xmm30,xmm29,r8\{ru-sae\}
[ 	]*[a-f0-9]+:	62 41 97 30 7b f0    	vcvtusi2sd xmm30,xmm29,r8\{rd-sae\}
[ 	]*[a-f0-9]+:	62 41 97 70 7b f0    	vcvtusi2sd xmm30,xmm29,r8\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 97 40 7b 31    	vcvtusi2sd xmm30,xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 97 40 7b b4 f0 34 12 00 00 	vcvtusi2sd xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 97 40 7b 72 7f 	vcvtusi2sd xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 97 40 7b b2 00 04 00 00 	vcvtusi2sd xmm30,xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 97 40 7b 72 80 	vcvtusi2sd xmm30,xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 97 40 7b b2 f8 fb ff ff 	vcvtusi2sd xmm30,xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 61 16 40 7b f0    	vcvtusi2ss xmm30,xmm29,eax
[ 	]*[a-f0-9]+:	62 61 16 10 7b f0    	vcvtusi2ss xmm30,xmm29,eax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 16 50 7b f0    	vcvtusi2ss xmm30,xmm29,eax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 16 30 7b f0    	vcvtusi2ss xmm30,xmm29,eax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 16 70 7b f0    	vcvtusi2ss xmm30,xmm29,eax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 40 7b f5    	vcvtusi2ss xmm30,xmm29,ebp
[ 	]*[a-f0-9]+:	62 61 16 10 7b f5    	vcvtusi2ss xmm30,xmm29,ebp\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 16 50 7b f5    	vcvtusi2ss xmm30,xmm29,ebp\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 16 30 7b f5    	vcvtusi2ss xmm30,xmm29,ebp\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 16 70 7b f5    	vcvtusi2ss xmm30,xmm29,ebp\{rz-sae\}
[ 	]*[a-f0-9]+:	62 41 16 40 7b f5    	vcvtusi2ss xmm30,xmm29,r13d
[ 	]*[a-f0-9]+:	62 41 16 10 7b f5    	vcvtusi2ss xmm30,xmm29,r13d\{rn-sae\}
[ 	]*[a-f0-9]+:	62 41 16 50 7b f5    	vcvtusi2ss xmm30,xmm29,r13d\{ru-sae\}
[ 	]*[a-f0-9]+:	62 41 16 30 7b f5    	vcvtusi2ss xmm30,xmm29,r13d\{rd-sae\}
[ 	]*[a-f0-9]+:	62 41 16 70 7b f5    	vcvtusi2ss xmm30,xmm29,r13d\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 16 40 7b 31    	vcvtusi2ss xmm30,xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 16 40 7b b4 f0 34 12 00 00 	vcvtusi2ss xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 16 40 7b 72 7f 	vcvtusi2ss xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 61 16 40 7b b2 00 02 00 00 	vcvtusi2ss xmm30,xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 61 16 40 7b 72 80 	vcvtusi2ss xmm30,xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 61 16 40 7b b2 fc fd ff ff 	vcvtusi2ss xmm30,xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 61 96 40 7b f0    	vcvtusi2ss xmm30,xmm29,rax
[ 	]*[a-f0-9]+:	62 61 96 10 7b f0    	vcvtusi2ss xmm30,xmm29,rax\{rn-sae\}
[ 	]*[a-f0-9]+:	62 61 96 50 7b f0    	vcvtusi2ss xmm30,xmm29,rax\{ru-sae\}
[ 	]*[a-f0-9]+:	62 61 96 30 7b f0    	vcvtusi2ss xmm30,xmm29,rax\{rd-sae\}
[ 	]*[a-f0-9]+:	62 61 96 70 7b f0    	vcvtusi2ss xmm30,xmm29,rax\{rz-sae\}
[ 	]*[a-f0-9]+:	62 41 96 40 7b f0    	vcvtusi2ss xmm30,xmm29,r8
[ 	]*[a-f0-9]+:	62 41 96 10 7b f0    	vcvtusi2ss xmm30,xmm29,r8\{rn-sae\}
[ 	]*[a-f0-9]+:	62 41 96 50 7b f0    	vcvtusi2ss xmm30,xmm29,r8\{ru-sae\}
[ 	]*[a-f0-9]+:	62 41 96 30 7b f0    	vcvtusi2ss xmm30,xmm29,r8\{rd-sae\}
[ 	]*[a-f0-9]+:	62 41 96 70 7b f0    	vcvtusi2ss xmm30,xmm29,r8\{rz-sae\}
[ 	]*[a-f0-9]+:	62 61 96 40 7b 31    	vcvtusi2ss xmm30,xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 21 96 40 7b b4 f0 34 12 00 00 	vcvtusi2ss xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 61 96 40 7b 72 7f 	vcvtusi2ss xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 61 96 40 7b b2 00 04 00 00 	vcvtusi2ss xmm30,xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 61 96 40 7b 72 80 	vcvtusi2ss xmm30,xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 61 96 40 7b b2 f8 fb ff ff 	vcvtusi2ss xmm30,xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 95 47 2d f4    	vscalefsd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 c7 2d f4    	vscalefsd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 95 17 2d f4    	vscalefsd xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 95 57 2d f4    	vscalefsd xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 95 37 2d f4    	vscalefsd xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 95 77 2d f4    	vscalefsd xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 95 47 2d 31    	vscalefsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 95 47 2d b4 f0 34 12 00 00 	vscalefsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 95 47 2d 72 7f 	vscalefsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 95 47 2d b2 00 04 00 00 	vscalefsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 2d 72 80 	vscalefsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 95 47 2d b2 f8 fb ff ff 	vscalefsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 15 47 2d f4    	vscalefss xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 c7 2d f4    	vscalefss xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:	62 02 15 17 2d f4    	vscalefss xmm30\{k7\},xmm29,xmm28\{rn-sae\}
[ 	]*[a-f0-9]+:	62 02 15 57 2d f4    	vscalefss xmm30\{k7\},xmm29,xmm28\{ru-sae\}
[ 	]*[a-f0-9]+:	62 02 15 37 2d f4    	vscalefss xmm30\{k7\},xmm29,xmm28\{rd-sae\}
[ 	]*[a-f0-9]+:	62 02 15 77 2d f4    	vscalefss xmm30\{k7\},xmm29,xmm28\{rz-sae\}
[ 	]*[a-f0-9]+:	62 62 15 47 2d 31    	vscalefss xmm30\{k7\},xmm29,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 15 47 2d b4 f0 34 12 00 00 	vscalefss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 15 47 2d 72 7f 	vscalefss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 15 47 2d b2 00 02 00 00 	vscalefss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 2d 72 80 	vscalefss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 15 47 2d b2 fc fd ff ff 	vscalefss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 03 15 47 55 f4 ab 	vfixupimmss xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 15 c7 55 f4 ab 	vfixupimmss xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 15 17 55 f4 ab 	vfixupimmss xmm30\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 03 15 47 55 f4 7b 	vfixupimmss xmm30\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 03 15 17 55 f4 7b 	vfixupimmss xmm30\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 55 31 7b 	vfixupimmss xmm30\{k7\},xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 23 15 47 55 b4 f0 34 12 00 00 7b 	vfixupimmss xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 55 72 7f 7b 	vfixupimmss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 55 b2 00 02 00 00 7b 	vfixupimmss xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 55 72 80 7b 	vfixupimmss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 55 b2 fc fd ff ff 7b 	vfixupimmss xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:	62 03 95 47 55 f4 ab 	vfixupimmsd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 95 c7 55 f4 ab 	vfixupimmsd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 95 17 55 f4 ab 	vfixupimmsd xmm30\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 03 95 47 55 f4 7b 	vfixupimmsd xmm30\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 03 95 17 55 f4 7b 	vfixupimmsd xmm30\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 55 31 7b 	vfixupimmsd xmm30\{k7\},xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 23 95 47 55 b4 f0 34 12 00 00 7b 	vfixupimmsd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 55 72 7f 7b 	vfixupimmsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 55 b2 00 04 00 00 7b 	vfixupimmsd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 55 72 80 7b 	vfixupimmsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 55 b2 f8 fb ff ff 7b 	vfixupimmsd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 03 95 47 0b f4 ab 	vrndscalesd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 95 c7 0b f4 ab 	vrndscalesd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 95 17 0b f4 ab 	vrndscalesd xmm30\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 03 95 47 0b f4 7b 	vrndscalesd xmm30\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 03 95 17 0b f4 7b 	vrndscalesd xmm30\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 0b 31 7b 	vrndscalesd xmm30\{k7\},xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 23 95 47 0b b4 f0 34 12 00 00 7b 	vrndscalesd xmm30\{k7\},xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 0b 72 7f 7b 	vrndscalesd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 0b b2 00 04 00 00 7b 	vrndscalesd xmm30\{k7\},xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 0b 72 80 7b 	vrndscalesd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:	62 63 95 47 0b b2 f8 fb ff ff 7b 	vrndscalesd xmm30\{k7\},xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:	62 03 15 47 0a f4 ab 	vrndscaless xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 15 c7 0a f4 ab 	vrndscaless xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:	62 03 15 17 0a f4 ab 	vrndscaless xmm30\{k7\},xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:	62 03 15 47 0a f4 7b 	vrndscaless xmm30\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:	62 03 15 17 0a f4 7b 	vrndscaless xmm30\{k7\},xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 0a 31 7b 	vrndscaless xmm30\{k7\},xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	62 23 15 47 0a b4 f0 34 12 00 00 7b 	vrndscaless xmm30\{k7\},xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 0a 72 7f 7b 	vrndscaless xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 0a b2 00 02 00 00 7b 	vrndscaless xmm30\{k7\},xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 0a 72 80 7b 	vrndscaless xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:	62 63 15 47 0a b2 fc fd ff ff 7b 	vrndscaless xmm30\{k7\},xmm29,DWORD PTR \[rdx-0x204\],0x7b
#pass
