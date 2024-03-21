#as: -mevexlig=512
#objdump: -dw
#name: x86_64 AVX512 lig512 insns
#source: x86-64-evex-lig.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 01 97 47 58 f4    	vaddsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 58 f4    	vaddsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 58 f4    	vaddsd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 57 58 f4    	vaddsd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 37 58 f4    	vaddsd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 77 58 f4    	vaddsd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 58 31    	vaddsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 58 b4 f0 23 01 00 00 	vaddsd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 58 72 7f 	vaddsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 58 b2 00 04 00 00 	vaddsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 58 72 80 	vaddsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 58 b2 f8 fb ff ff 	vaddsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 58 f4    	vaddss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 58 f4    	vaddss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 58 f4    	vaddss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 57 58 f4    	vaddss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 37 58 f4    	vaddss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 77 58 f4    	vaddss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 58 31    	vaddss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 58 b4 f0 23 01 00 00 	vaddss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 58 72 7f 	vaddss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 58 b2 00 02 00 00 	vaddss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 58 72 80 	vaddss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 58 b2 fc fd ff ff 	vaddss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec ab 	vcmpsd \$0xab,%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec ab 	vcmpsd \$0xab,\{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 7b 	vcmpsd \$0x7b,%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 7b 	vcmpsd \$0x7b,\{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 7b 	vcmpsd \$0x7b,\(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 7b 	vcmpsd \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 7b 	vcmpsd \$0x7b,0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 7b 	vcmpsd \$0x7b,0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 7b 	vcmpsd \$0x7b,-0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 7b 	vcmpsd \$0x7b,-0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 00 	vcmpeqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 00 	vcmpeqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 00 	vcmpeqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 00 	vcmpeqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 00 	vcmpeqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 00 	vcmpeqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 00 	vcmpeqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 00 	vcmpeqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 00 	vcmpeqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 00 	vcmpeqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 00 	vcmpeqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 00 	vcmpeqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 00 	vcmpeqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 00 	vcmpeqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 00 	vcmpeqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 00 	vcmpeqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 01 	vcmpltsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 01 	vcmpltsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 01 	vcmpltsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 01 	vcmpltsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 01 	vcmpltsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 01 	vcmpltsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 01 	vcmpltsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 01 	vcmpltsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 01 	vcmpltsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 01 	vcmpltsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 01 	vcmpltsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 01 	vcmpltsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 01 	vcmpltsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 01 	vcmpltsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 01 	vcmpltsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 01 	vcmpltsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 02 	vcmplesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 02 	vcmplesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 02 	vcmplesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 02 	vcmplesd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 02 	vcmplesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 02 	vcmplesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 02 	vcmplesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 02 	vcmplesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 02 	vcmplesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 02 	vcmplesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 02 	vcmplesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 02 	vcmplesd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 02 	vcmplesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 02 	vcmplesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 02 	vcmplesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 02 	vcmplesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 03 	vcmpunordsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 03 	vcmpunordsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 03 	vcmpunordsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 03 	vcmpunordsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 03 	vcmpunordsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 03 	vcmpunordsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 03 	vcmpunordsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 03 	vcmpunordsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 03 	vcmpunordsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 03 	vcmpunordsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 03 	vcmpunordsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 03 	vcmpunordsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 03 	vcmpunordsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 03 	vcmpunordsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 03 	vcmpunordsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 03 	vcmpunordsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 04 	vcmpneqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 04 	vcmpneqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 04 	vcmpneqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 04 	vcmpneqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 04 	vcmpneqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 04 	vcmpneqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 04 	vcmpneqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 04 	vcmpneqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 04 	vcmpneqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 04 	vcmpneqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 04 	vcmpneqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 04 	vcmpneqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 04 	vcmpneqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 04 	vcmpneqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 04 	vcmpneqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 04 	vcmpneqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 05 	vcmpnltsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 05 	vcmpnltsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 05 	vcmpnltsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 05 	vcmpnltsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 05 	vcmpnltsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 05 	vcmpnltsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 05 	vcmpnltsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 05 	vcmpnltsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 05 	vcmpnltsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 05 	vcmpnltsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 05 	vcmpnltsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 05 	vcmpnltsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 05 	vcmpnltsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 05 	vcmpnltsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 05 	vcmpnltsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 05 	vcmpnltsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 06 	vcmpnlesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 06 	vcmpnlesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 06 	vcmpnlesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 06 	vcmpnlesd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 06 	vcmpnlesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 06 	vcmpnlesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 06 	vcmpnlesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 06 	vcmpnlesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 06 	vcmpnlesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 06 	vcmpnlesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 06 	vcmpnlesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 06 	vcmpnlesd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 06 	vcmpnlesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 06 	vcmpnlesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 06 	vcmpnlesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 06 	vcmpnlesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 07 	vcmpordsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 07 	vcmpordsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 07 	vcmpordsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 07 	vcmpordsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 07 	vcmpordsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 07 	vcmpordsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 07 	vcmpordsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 07 	vcmpordsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 07 	vcmpordsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 07 	vcmpordsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 07 	vcmpordsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 07 	vcmpordsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 07 	vcmpordsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 07 	vcmpordsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 07 	vcmpordsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 07 	vcmpordsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 08 	vcmpeq_uqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 08 	vcmpeq_uqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 08 	vcmpeq_uqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 08 	vcmpeq_uqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 08 	vcmpeq_uqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 08 	vcmpeq_uqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 08 	vcmpeq_uqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 08 	vcmpeq_uqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 09 	vcmpngesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 09 	vcmpngesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 09 	vcmpngesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 09 	vcmpngesd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 09 	vcmpngesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 09 	vcmpngesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 09 	vcmpngesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 09 	vcmpngesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 09 	vcmpngesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 09 	vcmpngesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 09 	vcmpngesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 09 	vcmpngesd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 09 	vcmpngesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 09 	vcmpngesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 09 	vcmpngesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 09 	vcmpngesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0a 	vcmpngtsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0a 	vcmpngtsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0a 	vcmpngtsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0a 	vcmpngtsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0a 	vcmpngtsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0a 	vcmpngtsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0a 	vcmpngtsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0a 	vcmpngtsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0a 	vcmpngtsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0a 	vcmpngtsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0a 	vcmpngtsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0a 	vcmpngtsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0a 	vcmpngtsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0a 	vcmpngtsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0a 	vcmpngtsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0a 	vcmpngtsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0b 	vcmpfalsesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0b 	vcmpfalsesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0b 	vcmpfalsesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0b 	vcmpfalsesd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0b 	vcmpfalsesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0b 	vcmpfalsesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0b 	vcmpfalsesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0b 	vcmpfalsesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0b 	vcmpfalsesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0b 	vcmpfalsesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0b 	vcmpfalsesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0b 	vcmpfalsesd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0b 	vcmpfalsesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0b 	vcmpfalsesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0b 	vcmpfalsesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0b 	vcmpfalsesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0c 	vcmpneq_oqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0c 	vcmpneq_oqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0c 	vcmpneq_oqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0c 	vcmpneq_oqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0c 	vcmpneq_oqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0c 	vcmpneq_oqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0c 	vcmpneq_oqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0c 	vcmpneq_oqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0d 	vcmpgesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0d 	vcmpgesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0d 	vcmpgesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0d 	vcmpgesd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0d 	vcmpgesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0d 	vcmpgesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0d 	vcmpgesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0d 	vcmpgesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0d 	vcmpgesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0d 	vcmpgesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0d 	vcmpgesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0d 	vcmpgesd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0d 	vcmpgesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0d 	vcmpgesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0d 	vcmpgesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0d 	vcmpgesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0e 	vcmpgtsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0e 	vcmpgtsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0e 	vcmpgtsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0e 	vcmpgtsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0e 	vcmpgtsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0e 	vcmpgtsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0e 	vcmpgtsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0e 	vcmpgtsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0e 	vcmpgtsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0e 	vcmpgtsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0e 	vcmpgtsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0e 	vcmpgtsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0e 	vcmpgtsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0e 	vcmpgtsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0e 	vcmpgtsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0e 	vcmpgtsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0f 	vcmptruesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0f 	vcmptruesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0f 	vcmptruesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0f 	vcmptruesd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0f 	vcmptruesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0f 	vcmptruesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0f 	vcmptruesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0f 	vcmptruesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0f 	vcmptruesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0f 	vcmptruesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0f 	vcmptruesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 0f 	vcmptruesd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0f 	vcmptruesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0f 	vcmptruesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0f 	vcmptruesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0f 	vcmptruesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 10 	vcmpeq_ossd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 10 	vcmpeq_ossd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 10 	vcmpeq_ossd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 10 	vcmpeq_ossd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 10 	vcmpeq_ossd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 10 	vcmpeq_ossd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 10 	vcmpeq_ossd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 10 	vcmpeq_ossd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 11 	vcmplt_oqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 11 	vcmplt_oqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 11 	vcmplt_oqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 11 	vcmplt_oqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 11 	vcmplt_oqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 11 	vcmplt_oqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 11 	vcmplt_oqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 11 	vcmplt_oqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 12 	vcmple_oqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 12 	vcmple_oqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 12 	vcmple_oqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 12 	vcmple_oqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 12 	vcmple_oqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 12 	vcmple_oqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 12 	vcmple_oqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 12 	vcmple_oqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 13 	vcmpunord_ssd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 13 	vcmpunord_ssd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 13 	vcmpunord_ssd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 13 	vcmpunord_ssd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 13 	vcmpunord_ssd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 13 	vcmpunord_ssd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 13 	vcmpunord_ssd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 13 	vcmpunord_ssd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 14 	vcmpneq_ussd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 14 	vcmpneq_ussd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 14 	vcmpneq_ussd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 14 	vcmpneq_ussd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 14 	vcmpneq_ussd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 14 	vcmpneq_ussd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 14 	vcmpneq_ussd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 14 	vcmpneq_ussd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 15 	vcmpnlt_uqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 15 	vcmpnlt_uqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 15 	vcmpnlt_uqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 15 	vcmpnlt_uqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 15 	vcmpnlt_uqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 15 	vcmpnlt_uqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 15 	vcmpnlt_uqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 15 	vcmpnlt_uqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 16 	vcmpnle_uqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 16 	vcmpnle_uqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 16 	vcmpnle_uqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 16 	vcmpnle_uqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 16 	vcmpnle_uqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 16 	vcmpnle_uqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 16 	vcmpnle_uqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 16 	vcmpnle_uqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 17 	vcmpord_ssd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 17 	vcmpord_ssd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 17 	vcmpord_ssd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 17 	vcmpord_ssd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 17 	vcmpord_ssd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 17 	vcmpord_ssd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 17 	vcmpord_ssd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 17 	vcmpord_ssd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 18 	vcmpeq_ussd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 18 	vcmpeq_ussd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 18 	vcmpeq_ussd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 18 	vcmpeq_ussd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 18 	vcmpeq_ussd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 18 	vcmpeq_ussd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 18 	vcmpeq_ussd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 18 	vcmpeq_ussd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 19 	vcmpnge_uqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 19 	vcmpnge_uqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 19 	vcmpnge_uqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 19 	vcmpnge_uqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 19 	vcmpnge_uqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 19 	vcmpnge_uqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 19 	vcmpnge_uqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 19 	vcmpnge_uqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1a 	vcmpngt_uqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1a 	vcmpngt_uqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1a 	vcmpngt_uqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 1a 	vcmpngt_uqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1a 	vcmpngt_uqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1a 	vcmpngt_uqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1a 	vcmpngt_uqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1a 	vcmpngt_uqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1b 	vcmpfalse_ossd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1b 	vcmpfalse_ossd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1b 	vcmpfalse_ossd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 1b 	vcmpfalse_ossd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1b 	vcmpfalse_ossd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1b 	vcmpfalse_ossd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1b 	vcmpfalse_ossd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1b 	vcmpfalse_ossd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1c 	vcmpneq_ossd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1c 	vcmpneq_ossd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1c 	vcmpneq_ossd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 1c 	vcmpneq_ossd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1c 	vcmpneq_ossd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1c 	vcmpneq_ossd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1c 	vcmpneq_ossd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1c 	vcmpneq_ossd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1d 	vcmpge_oqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1d 	vcmpge_oqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1d 	vcmpge_oqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 1d 	vcmpge_oqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1d 	vcmpge_oqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1d 	vcmpge_oqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1d 	vcmpge_oqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1d 	vcmpge_oqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1e 	vcmpgt_oqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1e 	vcmpgt_oqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1e 	vcmpgt_oqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 1e 	vcmpgt_oqsd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1e 	vcmpgt_oqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1e 	vcmpgt_oqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1e 	vcmpgt_oqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1e 	vcmpgt_oqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1f 	vcmptrue_ussd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1f 	vcmptrue_ussd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1f 	vcmptrue_ussd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 23 01 00 00 1f 	vcmptrue_ussd 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1f 	vcmptrue_ussd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1f 	vcmptrue_ussd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1f 	vcmptrue_ussd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1f 	vcmptrue_ussd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec ab 	vcmpss \$0xab,%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec ab 	vcmpss \$0xab,\{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 7b 	vcmpss \$0x7b,%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 7b 	vcmpss \$0x7b,\{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 7b 	vcmpss \$0x7b,\(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 7b 	vcmpss \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 7b 	vcmpss \$0x7b,0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 7b 	vcmpss \$0x7b,0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 7b 	vcmpss \$0x7b,-0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 7b 	vcmpss \$0x7b,-0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 00 	vcmpeqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 00 	vcmpeqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 00 	vcmpeqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 00 	vcmpeqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 00 	vcmpeqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 00 	vcmpeqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 00 	vcmpeqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 00 	vcmpeqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 00 	vcmpeqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 00 	vcmpeqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 00 	vcmpeqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 00 	vcmpeqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 00 	vcmpeqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 00 	vcmpeqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 00 	vcmpeqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 00 	vcmpeqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 01 	vcmpltss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 01 	vcmpltss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 01 	vcmpltss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 01 	vcmpltss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 01 	vcmpltss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 01 	vcmpltss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 01 	vcmpltss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 01 	vcmpltss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 01 	vcmpltss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 01 	vcmpltss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 01 	vcmpltss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 01 	vcmpltss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 01 	vcmpltss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 01 	vcmpltss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 01 	vcmpltss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 01 	vcmpltss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 02 	vcmpless %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 02 	vcmpless \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 02 	vcmpless \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 02 	vcmpless 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 02 	vcmpless 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 02 	vcmpless 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 02 	vcmpless -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 02 	vcmpless -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 02 	vcmpless %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 02 	vcmpless \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 02 	vcmpless \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 02 	vcmpless 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 02 	vcmpless 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 02 	vcmpless 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 02 	vcmpless -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 02 	vcmpless -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 03 	vcmpunordss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 03 	vcmpunordss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 03 	vcmpunordss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 03 	vcmpunordss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 03 	vcmpunordss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 03 	vcmpunordss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 03 	vcmpunordss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 03 	vcmpunordss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 03 	vcmpunordss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 03 	vcmpunordss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 03 	vcmpunordss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 03 	vcmpunordss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 03 	vcmpunordss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 03 	vcmpunordss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 03 	vcmpunordss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 03 	vcmpunordss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 04 	vcmpneqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 04 	vcmpneqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 04 	vcmpneqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 04 	vcmpneqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 04 	vcmpneqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 04 	vcmpneqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 04 	vcmpneqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 04 	vcmpneqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 04 	vcmpneqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 04 	vcmpneqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 04 	vcmpneqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 04 	vcmpneqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 04 	vcmpneqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 04 	vcmpneqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 04 	vcmpneqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 04 	vcmpneqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 05 	vcmpnltss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 05 	vcmpnltss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 05 	vcmpnltss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 05 	vcmpnltss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 05 	vcmpnltss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 05 	vcmpnltss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 05 	vcmpnltss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 05 	vcmpnltss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 05 	vcmpnltss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 05 	vcmpnltss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 05 	vcmpnltss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 05 	vcmpnltss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 05 	vcmpnltss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 05 	vcmpnltss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 05 	vcmpnltss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 05 	vcmpnltss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 06 	vcmpnless %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 06 	vcmpnless \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 06 	vcmpnless \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 06 	vcmpnless 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 06 	vcmpnless 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 06 	vcmpnless 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 06 	vcmpnless -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 06 	vcmpnless -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 06 	vcmpnless %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 06 	vcmpnless \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 06 	vcmpnless \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 06 	vcmpnless 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 06 	vcmpnless 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 06 	vcmpnless 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 06 	vcmpnless -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 06 	vcmpnless -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 07 	vcmpordss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 07 	vcmpordss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 07 	vcmpordss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 07 	vcmpordss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 07 	vcmpordss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 07 	vcmpordss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 07 	vcmpordss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 07 	vcmpordss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 07 	vcmpordss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 07 	vcmpordss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 07 	vcmpordss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 07 	vcmpordss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 07 	vcmpordss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 07 	vcmpordss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 07 	vcmpordss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 07 	vcmpordss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 08 	vcmpeq_uqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 08 	vcmpeq_uqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 08 	vcmpeq_uqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 08 	vcmpeq_uqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 08 	vcmpeq_uqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 08 	vcmpeq_uqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 08 	vcmpeq_uqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 08 	vcmpeq_uqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 09 	vcmpngess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 09 	vcmpngess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 09 	vcmpngess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 09 	vcmpngess 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 09 	vcmpngess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 09 	vcmpngess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 09 	vcmpngess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 09 	vcmpngess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 09 	vcmpngess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 09 	vcmpngess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 09 	vcmpngess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 09 	vcmpngess 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 09 	vcmpngess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 09 	vcmpngess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 09 	vcmpngess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 09 	vcmpngess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0a 	vcmpngtss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0a 	vcmpngtss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0a 	vcmpngtss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0a 	vcmpngtss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0a 	vcmpngtss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0a 	vcmpngtss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0a 	vcmpngtss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0a 	vcmpngtss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0a 	vcmpngtss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0a 	vcmpngtss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0a 	vcmpngtss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0a 	vcmpngtss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0a 	vcmpngtss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0a 	vcmpngtss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0a 	vcmpngtss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0a 	vcmpngtss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0b 	vcmpfalsess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0b 	vcmpfalsess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0b 	vcmpfalsess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0b 	vcmpfalsess 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0b 	vcmpfalsess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0b 	vcmpfalsess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0b 	vcmpfalsess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0b 	vcmpfalsess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0b 	vcmpfalsess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0b 	vcmpfalsess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0b 	vcmpfalsess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0b 	vcmpfalsess 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0b 	vcmpfalsess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0b 	vcmpfalsess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0b 	vcmpfalsess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0b 	vcmpfalsess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0c 	vcmpneq_oqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0c 	vcmpneq_oqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0c 	vcmpneq_oqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0c 	vcmpneq_oqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0c 	vcmpneq_oqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0c 	vcmpneq_oqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0c 	vcmpneq_oqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0c 	vcmpneq_oqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0d 	vcmpgess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0d 	vcmpgess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0d 	vcmpgess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0d 	vcmpgess 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0d 	vcmpgess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0d 	vcmpgess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0d 	vcmpgess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0d 	vcmpgess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0d 	vcmpgess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0d 	vcmpgess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0d 	vcmpgess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0d 	vcmpgess 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0d 	vcmpgess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0d 	vcmpgess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0d 	vcmpgess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0d 	vcmpgess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0e 	vcmpgtss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0e 	vcmpgtss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0e 	vcmpgtss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0e 	vcmpgtss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0e 	vcmpgtss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0e 	vcmpgtss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0e 	vcmpgtss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0e 	vcmpgtss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0e 	vcmpgtss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0e 	vcmpgtss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0e 	vcmpgtss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0e 	vcmpgtss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0e 	vcmpgtss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0e 	vcmpgtss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0e 	vcmpgtss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0e 	vcmpgtss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0f 	vcmptruess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0f 	vcmptruess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0f 	vcmptruess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0f 	vcmptruess 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0f 	vcmptruess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0f 	vcmptruess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0f 	vcmptruess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0f 	vcmptruess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0f 	vcmptruess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0f 	vcmptruess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0f 	vcmptruess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 0f 	vcmptruess 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0f 	vcmptruess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0f 	vcmptruess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0f 	vcmptruess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0f 	vcmptruess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 10 	vcmpeq_osss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 10 	vcmpeq_osss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 10 	vcmpeq_osss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 10 	vcmpeq_osss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 10 	vcmpeq_osss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 10 	vcmpeq_osss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 10 	vcmpeq_osss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 10 	vcmpeq_osss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 11 	vcmplt_oqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 11 	vcmplt_oqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 11 	vcmplt_oqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 11 	vcmplt_oqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 11 	vcmplt_oqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 11 	vcmplt_oqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 11 	vcmplt_oqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 11 	vcmplt_oqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 12 	vcmple_oqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 12 	vcmple_oqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 12 	vcmple_oqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 12 	vcmple_oqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 12 	vcmple_oqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 12 	vcmple_oqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 12 	vcmple_oqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 12 	vcmple_oqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 13 	vcmpunord_sss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 13 	vcmpunord_sss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 13 	vcmpunord_sss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 13 	vcmpunord_sss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 13 	vcmpunord_sss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 13 	vcmpunord_sss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 13 	vcmpunord_sss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 13 	vcmpunord_sss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 14 	vcmpneq_usss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 14 	vcmpneq_usss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 14 	vcmpneq_usss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 14 	vcmpneq_usss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 14 	vcmpneq_usss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 14 	vcmpneq_usss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 14 	vcmpneq_usss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 14 	vcmpneq_usss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 15 	vcmpnlt_uqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 15 	vcmpnlt_uqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 15 	vcmpnlt_uqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 15 	vcmpnlt_uqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 15 	vcmpnlt_uqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 15 	vcmpnlt_uqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 15 	vcmpnlt_uqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 15 	vcmpnlt_uqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 16 	vcmpnle_uqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 16 	vcmpnle_uqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 16 	vcmpnle_uqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 16 	vcmpnle_uqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 16 	vcmpnle_uqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 16 	vcmpnle_uqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 16 	vcmpnle_uqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 16 	vcmpnle_uqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 17 	vcmpord_sss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 17 	vcmpord_sss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 17 	vcmpord_sss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 17 	vcmpord_sss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 17 	vcmpord_sss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 17 	vcmpord_sss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 17 	vcmpord_sss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 17 	vcmpord_sss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 18 	vcmpeq_usss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 18 	vcmpeq_usss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 18 	vcmpeq_usss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 18 	vcmpeq_usss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 18 	vcmpeq_usss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 18 	vcmpeq_usss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 18 	vcmpeq_usss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 18 	vcmpeq_usss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 19 	vcmpnge_uqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 19 	vcmpnge_uqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 19 	vcmpnge_uqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 19 	vcmpnge_uqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 19 	vcmpnge_uqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 19 	vcmpnge_uqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 19 	vcmpnge_uqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 19 	vcmpnge_uqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1a 	vcmpngt_uqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1a 	vcmpngt_uqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1a 	vcmpngt_uqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 1a 	vcmpngt_uqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1a 	vcmpngt_uqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1a 	vcmpngt_uqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1a 	vcmpngt_uqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1a 	vcmpngt_uqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1b 	vcmpfalse_osss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1b 	vcmpfalse_osss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1b 	vcmpfalse_osss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 1b 	vcmpfalse_osss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1b 	vcmpfalse_osss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1b 	vcmpfalse_osss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1b 	vcmpfalse_osss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1b 	vcmpfalse_osss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1c 	vcmpneq_osss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1c 	vcmpneq_osss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1c 	vcmpneq_osss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 1c 	vcmpneq_osss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1c 	vcmpneq_osss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1c 	vcmpneq_osss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1c 	vcmpneq_osss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1c 	vcmpneq_osss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1d 	vcmpge_oqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1d 	vcmpge_oqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1d 	vcmpge_oqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 1d 	vcmpge_oqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1d 	vcmpge_oqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1d 	vcmpge_oqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1d 	vcmpge_oqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1d 	vcmpge_oqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1e 	vcmpgt_oqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1e 	vcmpgt_oqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1e 	vcmpgt_oqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 1e 	vcmpgt_oqss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1e 	vcmpgt_oqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1e 	vcmpgt_oqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1e 	vcmpgt_oqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1e 	vcmpgt_oqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1f 	vcmptrue_usss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1f 	vcmptrue_usss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1f 	vcmptrue_usss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 23 01 00 00 1f 	vcmptrue_usss 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1f 	vcmptrue_usss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1f 	vcmptrue_usss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1f 	vcmptrue_usss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1f 	vcmptrue_usss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 01 fd 48 2f f5    	vcomisd %xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 01 fd 18 2f f5    	vcomisd \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2f 31    	vcomisd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:	62 21 fd 48 2f b4 f0 23 01 00 00 	vcomisd 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2f 72 7f 	vcomisd 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2f b2 00 04 00 00 	vcomisd 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2f 72 80 	vcomisd -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2f b2 f8 fb ff ff 	vcomisd -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 01 7c 48 2f f5    	vcomiss %xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 01 7c 18 2f f5    	vcomiss \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2f 31    	vcomiss \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:	62 21 7c 48 2f b4 f0 23 01 00 00 	vcomiss 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2f 72 7f 	vcomiss 0x1fc\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2f b2 00 02 00 00 	vcomiss 0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2f 72 80 	vcomiss -0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2f b2 fc fd ff ff 	vcomiss -0x204\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 91 7f 18 2d c6    	vcvtsd2si \{rn-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 58 2d c6    	vcvtsd2si \{ru-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 38 2d c6    	vcvtsd2si \{rd-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 78 2d c6    	vcvtsd2si \{rz-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 18 2d ee    	vcvtsd2si \{rn-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 58 2d ee    	vcvtsd2si \{ru-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 38 2d ee    	vcvtsd2si \{rd-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 78 2d ee    	vcvtsd2si \{rz-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 11 7f 18 2d ee    	vcvtsd2si \{rn-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 58 2d ee    	vcvtsd2si \{ru-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 38 2d ee    	vcvtsd2si \{rd-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 78 2d ee    	vcvtsd2si \{rz-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 91 ff 18 2d c6    	vcvtsd2si \{rn-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 58 2d c6    	vcvtsd2si \{ru-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 38 2d c6    	vcvtsd2si \{rd-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 78 2d c6    	vcvtsd2si \{rz-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 11 ff 18 2d c6    	vcvtsd2si \{rn-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 58 2d c6    	vcvtsd2si \{ru-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 38 2d c6    	vcvtsd2si \{rd-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 78 2d c6    	vcvtsd2si \{rz-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 01 97 47 5a f4    	vcvtsd2ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 5a f4    	vcvtsd2ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 5a f4    	vcvtsd2ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 57 5a f4    	vcvtsd2ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 37 5a f4    	vcvtsd2ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 77 5a f4    	vcvtsd2ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5a 31    	vcvtsd2ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 5a b4 f0 23 01 00 00 	vcvtsd2ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5a 72 7f 	vcvtsd2ss 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5a b2 00 04 00 00 	vcvtsd2ss 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5a 72 80 	vcvtsd2ss -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5a b2 f8 fb ff ff 	vcvtsd2ss -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 17 40 2a f0    	vcvtsi2sd %eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 2a f5    	vcvtsi2sd %ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 17 40 2a f5    	vcvtsi2sd %r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 2a 31    	vcvtsi2sdl \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 17 40 2a b4 f0 23 01 00 00 	vcvtsi2sdl 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 2a 72 7f 	vcvtsi2sdl 0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 2a b2 00 02 00 00 	vcvtsi2sdl 0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 2a 72 80 	vcvtsi2sdl -0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 2a b2 fc fd ff ff 	vcvtsi2sdl -0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 2a f0    	vcvtsi2sd %rax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 10 2a f0    	vcvtsi2sd %rax,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 50 2a f0    	vcvtsi2sd %rax,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 30 2a f0    	vcvtsi2sd %rax,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 70 2a f0    	vcvtsi2sd %rax,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 40 2a f0    	vcvtsi2sd %r8,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 10 2a f0    	vcvtsi2sd %r8,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 50 2a f0    	vcvtsi2sd %r8,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 30 2a f0    	vcvtsi2sd %r8,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 70 2a f0    	vcvtsi2sd %r8,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 2a 31    	vcvtsi2sdq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 97 40 2a b4 f0 23 01 00 00 	vcvtsi2sdq 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 2a 72 7f 	vcvtsi2sdq 0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 2a b2 00 04 00 00 	vcvtsi2sdq 0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 2a 72 80 	vcvtsi2sdq -0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 2a b2 f8 fb ff ff 	vcvtsi2sdq -0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a f0    	vcvtsi2ss %eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 10 2a f0    	vcvtsi2ss %eax,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 50 2a f0    	vcvtsi2ss %eax,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 30 2a f0    	vcvtsi2ss %eax,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 70 2a f0    	vcvtsi2ss %eax,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a f5    	vcvtsi2ss %ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 10 2a f5    	vcvtsi2ss %ebp,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 50 2a f5    	vcvtsi2ss %ebp,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 30 2a f5    	vcvtsi2ss %ebp,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 70 2a f5    	vcvtsi2ss %ebp,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 40 2a f5    	vcvtsi2ss %r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 10 2a f5    	vcvtsi2ss %r13d,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 50 2a f5    	vcvtsi2ss %r13d,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 30 2a f5    	vcvtsi2ss %r13d,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 70 2a f5    	vcvtsi2ss %r13d,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a 31    	vcvtsi2ssl \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 16 40 2a b4 f0 23 01 00 00 	vcvtsi2ssl 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a 72 7f 	vcvtsi2ssl 0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a b2 00 02 00 00 	vcvtsi2ssl 0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a 72 80 	vcvtsi2ssl -0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a b2 fc fd ff ff 	vcvtsi2ssl -0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 2a f0    	vcvtsi2ss %rax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 10 2a f0    	vcvtsi2ss %rax,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 50 2a f0    	vcvtsi2ss %rax,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 30 2a f0    	vcvtsi2ss %rax,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 70 2a f0    	vcvtsi2ss %rax,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 40 2a f0    	vcvtsi2ss %r8,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 10 2a f0    	vcvtsi2ss %r8,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 50 2a f0    	vcvtsi2ss %r8,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 30 2a f0    	vcvtsi2ss %r8,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 70 2a f0    	vcvtsi2ss %r8,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 2a 31    	vcvtsi2ssq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 96 40 2a b4 f0 23 01 00 00 	vcvtsi2ssq 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 2a 72 7f 	vcvtsi2ssq 0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 2a b2 00 04 00 00 	vcvtsi2ssq 0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 2a 72 80 	vcvtsi2ssq -0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 2a b2 f8 fb ff ff 	vcvtsi2ssq -0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 01 16 47 5a f4    	vcvtss2sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 5a f4    	vcvtss2sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 5a f4    	vcvtss2sd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5a 31    	vcvtss2sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 5a b4 f0 23 01 00 00 	vcvtss2sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5a 72 7f 	vcvtss2sd 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5a b2 00 02 00 00 	vcvtss2sd 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5a 72 80 	vcvtss2sd -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5a b2 fc fd ff ff 	vcvtss2sd -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 91 7e 18 2d c6    	vcvtss2si \{rn-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 58 2d c6    	vcvtss2si \{ru-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 38 2d c6    	vcvtss2si \{rd-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 78 2d c6    	vcvtss2si \{rz-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 18 2d ee    	vcvtss2si \{rn-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 58 2d ee    	vcvtss2si \{ru-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 38 2d ee    	vcvtss2si \{rd-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 78 2d ee    	vcvtss2si \{rz-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 11 7e 18 2d ee    	vcvtss2si \{rn-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 58 2d ee    	vcvtss2si \{ru-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 38 2d ee    	vcvtss2si \{rd-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 78 2d ee    	vcvtss2si \{rz-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 91 fe 18 2d c6    	vcvtss2si \{rn-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 58 2d c6    	vcvtss2si \{ru-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 38 2d c6    	vcvtss2si \{rd-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 78 2d c6    	vcvtss2si \{rz-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 11 fe 18 2d c6    	vcvtss2si \{rn-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 58 2d c6    	vcvtss2si \{ru-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 38 2d c6    	vcvtss2si \{rd-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 78 2d c6    	vcvtss2si \{rz-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 91 7f 18 2c c6    	vcvttsd2si \{sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 18 2c ee    	vcvttsd2si \{sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 11 7f 18 2c ee    	vcvttsd2si \{sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 91 ff 18 2c c6    	vcvttsd2si \{sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 11 ff 18 2c c6    	vcvttsd2si \{sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 91 7e 18 2c c6    	vcvttss2si \{sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 18 2c ee    	vcvttss2si \{sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 11 7e 18 2c ee    	vcvttss2si \{sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 91 fe 18 2c c6    	vcvttss2si \{sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 11 fe 18 2c c6    	vcvttss2si \{sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 01 97 47 5e f4    	vdivsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 5e f4    	vdivsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 5e f4    	vdivsd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 57 5e f4    	vdivsd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 37 5e f4    	vdivsd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 77 5e f4    	vdivsd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5e 31    	vdivsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 5e b4 f0 23 01 00 00 	vdivsd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5e 72 7f 	vdivsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5e b2 00 04 00 00 	vdivsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5e 72 80 	vdivsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5e b2 f8 fb ff ff 	vdivsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 5e f4    	vdivss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 5e f4    	vdivss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 5e f4    	vdivss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 57 5e f4    	vdivss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 37 5e f4    	vdivss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 77 5e f4    	vdivss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5e 31    	vdivss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 5e b4 f0 23 01 00 00 	vdivss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5e 72 7f 	vdivss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5e b2 00 02 00 00 	vdivss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5e 72 80 	vdivss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5e b2 fc fd ff ff 	vdivss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 99 f4    	vfmadd132sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 99 f4    	vfmadd132sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 99 f4    	vfmadd132sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 99 f4    	vfmadd132sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 99 f4    	vfmadd132sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 99 f4    	vfmadd132sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 99 31    	vfmadd132sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 99 b4 f0 23 01 00 00 	vfmadd132sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 99 72 7f 	vfmadd132sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 99 b2 00 04 00 00 	vfmadd132sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 99 72 80 	vfmadd132sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 99 b2 f8 fb ff ff 	vfmadd132sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 99 f4    	vfmadd132ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 99 f4    	vfmadd132ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 99 f4    	vfmadd132ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 99 f4    	vfmadd132ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 99 f4    	vfmadd132ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 99 f4    	vfmadd132ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 99 31    	vfmadd132ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 99 b4 f0 23 01 00 00 	vfmadd132ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 99 72 7f 	vfmadd132ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 99 b2 00 02 00 00 	vfmadd132ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 99 72 80 	vfmadd132ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 99 b2 fc fd ff ff 	vfmadd132ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 a9 f4    	vfmadd213sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 a9 f4    	vfmadd213sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 a9 f4    	vfmadd213sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 a9 f4    	vfmadd213sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 a9 f4    	vfmadd213sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 a9 f4    	vfmadd213sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 a9 31    	vfmadd213sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 a9 b4 f0 23 01 00 00 	vfmadd213sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 a9 72 7f 	vfmadd213sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 a9 b2 00 04 00 00 	vfmadd213sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 a9 72 80 	vfmadd213sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 a9 b2 f8 fb ff ff 	vfmadd213sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 a9 f4    	vfmadd213ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 a9 f4    	vfmadd213ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 a9 f4    	vfmadd213ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 a9 f4    	vfmadd213ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 a9 f4    	vfmadd213ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 a9 f4    	vfmadd213ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 a9 31    	vfmadd213ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 a9 b4 f0 23 01 00 00 	vfmadd213ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 a9 72 7f 	vfmadd213ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 a9 b2 00 02 00 00 	vfmadd213ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 a9 72 80 	vfmadd213ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 a9 b2 fc fd ff ff 	vfmadd213ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 b9 f4    	vfmadd231sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 b9 f4    	vfmadd231sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 b9 f4    	vfmadd231sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 b9 f4    	vfmadd231sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 b9 f4    	vfmadd231sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 b9 f4    	vfmadd231sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 b9 31    	vfmadd231sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 b9 b4 f0 23 01 00 00 	vfmadd231sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 b9 72 7f 	vfmadd231sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 b9 b2 00 04 00 00 	vfmadd231sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 b9 72 80 	vfmadd231sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 b9 b2 f8 fb ff ff 	vfmadd231sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 b9 f4    	vfmadd231ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 b9 f4    	vfmadd231ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 b9 f4    	vfmadd231ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 b9 f4    	vfmadd231ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 b9 f4    	vfmadd231ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 b9 f4    	vfmadd231ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 b9 31    	vfmadd231ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 b9 b4 f0 23 01 00 00 	vfmadd231ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 b9 72 7f 	vfmadd231ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 b9 b2 00 02 00 00 	vfmadd231ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 b9 72 80 	vfmadd231ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 b9 b2 fc fd ff ff 	vfmadd231ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 9b f4    	vfmsub132sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 9b f4    	vfmsub132sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 9b f4    	vfmsub132sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 9b f4    	vfmsub132sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 9b f4    	vfmsub132sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 9b f4    	vfmsub132sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9b 31    	vfmsub132sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 9b b4 f0 23 01 00 00 	vfmsub132sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9b 72 7f 	vfmsub132sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9b b2 00 04 00 00 	vfmsub132sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9b 72 80 	vfmsub132sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9b b2 f8 fb ff ff 	vfmsub132sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 9b f4    	vfmsub132ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 9b f4    	vfmsub132ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 9b f4    	vfmsub132ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 9b f4    	vfmsub132ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 9b f4    	vfmsub132ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 9b f4    	vfmsub132ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9b 31    	vfmsub132ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 9b b4 f0 23 01 00 00 	vfmsub132ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9b 72 7f 	vfmsub132ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9b b2 00 02 00 00 	vfmsub132ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9b 72 80 	vfmsub132ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9b b2 fc fd ff ff 	vfmsub132ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 ab f4    	vfmsub213sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 ab f4    	vfmsub213sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 ab f4    	vfmsub213sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 ab f4    	vfmsub213sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 ab f4    	vfmsub213sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 ab f4    	vfmsub213sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ab 31    	vfmsub213sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 ab b4 f0 23 01 00 00 	vfmsub213sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ab 72 7f 	vfmsub213sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ab b2 00 04 00 00 	vfmsub213sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ab 72 80 	vfmsub213sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ab b2 f8 fb ff ff 	vfmsub213sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 ab f4    	vfmsub213ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 ab f4    	vfmsub213ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 ab f4    	vfmsub213ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 ab f4    	vfmsub213ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 ab f4    	vfmsub213ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 ab f4    	vfmsub213ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ab 31    	vfmsub213ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 ab b4 f0 23 01 00 00 	vfmsub213ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ab 72 7f 	vfmsub213ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ab b2 00 02 00 00 	vfmsub213ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ab 72 80 	vfmsub213ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ab b2 fc fd ff ff 	vfmsub213ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 bb f4    	vfmsub231sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 bb f4    	vfmsub231sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 bb f4    	vfmsub231sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 bb f4    	vfmsub231sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 bb f4    	vfmsub231sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 bb f4    	vfmsub231sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bb 31    	vfmsub231sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 bb b4 f0 23 01 00 00 	vfmsub231sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bb 72 7f 	vfmsub231sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bb b2 00 04 00 00 	vfmsub231sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bb 72 80 	vfmsub231sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bb b2 f8 fb ff ff 	vfmsub231sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 bb f4    	vfmsub231ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 bb f4    	vfmsub231ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 bb f4    	vfmsub231ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 bb f4    	vfmsub231ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 bb f4    	vfmsub231ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 bb f4    	vfmsub231ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bb 31    	vfmsub231ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 bb b4 f0 23 01 00 00 	vfmsub231ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bb 72 7f 	vfmsub231ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bb b2 00 02 00 00 	vfmsub231ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bb 72 80 	vfmsub231ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bb b2 fc fd ff ff 	vfmsub231ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 9d f4    	vfnmadd132sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 9d f4    	vfnmadd132sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 9d f4    	vfnmadd132sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 9d f4    	vfnmadd132sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 9d f4    	vfnmadd132sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 9d f4    	vfnmadd132sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9d 31    	vfnmadd132sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 9d b4 f0 23 01 00 00 	vfnmadd132sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9d 72 7f 	vfnmadd132sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9d b2 00 04 00 00 	vfnmadd132sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9d 72 80 	vfnmadd132sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9d b2 f8 fb ff ff 	vfnmadd132sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 9d f4    	vfnmadd132ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 9d f4    	vfnmadd132ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 9d f4    	vfnmadd132ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 9d f4    	vfnmadd132ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 9d f4    	vfnmadd132ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 9d f4    	vfnmadd132ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9d 31    	vfnmadd132ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 9d b4 f0 23 01 00 00 	vfnmadd132ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9d 72 7f 	vfnmadd132ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9d b2 00 02 00 00 	vfnmadd132ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9d 72 80 	vfnmadd132ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9d b2 fc fd ff ff 	vfnmadd132ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 ad f4    	vfnmadd213sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 ad f4    	vfnmadd213sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 ad f4    	vfnmadd213sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 ad f4    	vfnmadd213sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 ad f4    	vfnmadd213sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 ad f4    	vfnmadd213sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ad 31    	vfnmadd213sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 ad b4 f0 23 01 00 00 	vfnmadd213sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ad 72 7f 	vfnmadd213sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ad b2 00 04 00 00 	vfnmadd213sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ad 72 80 	vfnmadd213sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ad b2 f8 fb ff ff 	vfnmadd213sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 ad f4    	vfnmadd213ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 ad f4    	vfnmadd213ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 ad f4    	vfnmadd213ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 ad f4    	vfnmadd213ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 ad f4    	vfnmadd213ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 ad f4    	vfnmadd213ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ad 31    	vfnmadd213ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 ad b4 f0 23 01 00 00 	vfnmadd213ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ad 72 7f 	vfnmadd213ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ad b2 00 02 00 00 	vfnmadd213ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ad 72 80 	vfnmadd213ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ad b2 fc fd ff ff 	vfnmadd213ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 bd f4    	vfnmadd231sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 bd f4    	vfnmadd231sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 bd f4    	vfnmadd231sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 bd f4    	vfnmadd231sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 bd f4    	vfnmadd231sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 bd f4    	vfnmadd231sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bd 31    	vfnmadd231sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 bd b4 f0 23 01 00 00 	vfnmadd231sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bd 72 7f 	vfnmadd231sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bd b2 00 04 00 00 	vfnmadd231sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bd 72 80 	vfnmadd231sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bd b2 f8 fb ff ff 	vfnmadd231sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 bd f4    	vfnmadd231ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 bd f4    	vfnmadd231ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 bd f4    	vfnmadd231ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 bd f4    	vfnmadd231ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 bd f4    	vfnmadd231ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 bd f4    	vfnmadd231ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bd 31    	vfnmadd231ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 bd b4 f0 23 01 00 00 	vfnmadd231ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bd 72 7f 	vfnmadd231ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bd b2 00 02 00 00 	vfnmadd231ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bd 72 80 	vfnmadd231ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bd b2 fc fd ff ff 	vfnmadd231ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 9f f4    	vfnmsub132sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 9f f4    	vfnmsub132sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 9f f4    	vfnmsub132sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 9f f4    	vfnmsub132sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 9f f4    	vfnmsub132sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 9f f4    	vfnmsub132sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9f 31    	vfnmsub132sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 9f b4 f0 23 01 00 00 	vfnmsub132sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9f 72 7f 	vfnmsub132sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9f b2 00 04 00 00 	vfnmsub132sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9f 72 80 	vfnmsub132sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9f b2 f8 fb ff ff 	vfnmsub132sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 9f f4    	vfnmsub132ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 9f f4    	vfnmsub132ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 9f f4    	vfnmsub132ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 9f f4    	vfnmsub132ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 9f f4    	vfnmsub132ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 9f f4    	vfnmsub132ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9f 31    	vfnmsub132ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 9f b4 f0 23 01 00 00 	vfnmsub132ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9f 72 7f 	vfnmsub132ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9f b2 00 02 00 00 	vfnmsub132ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9f 72 80 	vfnmsub132ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9f b2 fc fd ff ff 	vfnmsub132ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 af f4    	vfnmsub213sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 af f4    	vfnmsub213sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 af f4    	vfnmsub213sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 af f4    	vfnmsub213sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 af f4    	vfnmsub213sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 af f4    	vfnmsub213sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 af 31    	vfnmsub213sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 af b4 f0 23 01 00 00 	vfnmsub213sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 af 72 7f 	vfnmsub213sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 af b2 00 04 00 00 	vfnmsub213sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 af 72 80 	vfnmsub213sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 af b2 f8 fb ff ff 	vfnmsub213sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 af f4    	vfnmsub213ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 af f4    	vfnmsub213ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 af f4    	vfnmsub213ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 af f4    	vfnmsub213ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 af f4    	vfnmsub213ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 af f4    	vfnmsub213ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 af 31    	vfnmsub213ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 af b4 f0 23 01 00 00 	vfnmsub213ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 af 72 7f 	vfnmsub213ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 af b2 00 02 00 00 	vfnmsub213ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 af 72 80 	vfnmsub213ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 af b2 fc fd ff ff 	vfnmsub213ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 bf f4    	vfnmsub231sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 bf f4    	vfnmsub231sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 bf f4    	vfnmsub231sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 bf f4    	vfnmsub231sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 bf f4    	vfnmsub231sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 bf f4    	vfnmsub231sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bf 31    	vfnmsub231sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 bf b4 f0 23 01 00 00 	vfnmsub231sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bf 72 7f 	vfnmsub231sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bf b2 00 04 00 00 	vfnmsub231sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bf 72 80 	vfnmsub231sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bf b2 f8 fb ff ff 	vfnmsub231sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 bf f4    	vfnmsub231ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 bf f4    	vfnmsub231ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 bf f4    	vfnmsub231ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 bf f4    	vfnmsub231ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 bf f4    	vfnmsub231ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 bf f4    	vfnmsub231ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bf 31    	vfnmsub231ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 bf b4 f0 23 01 00 00 	vfnmsub231ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bf 72 7f 	vfnmsub231ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bf b2 00 02 00 00 	vfnmsub231ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bf 72 80 	vfnmsub231ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bf b2 fc fd ff ff 	vfnmsub231ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 43 f4    	vgetexpsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 43 f4    	vgetexpsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 43 f4    	vgetexpsd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 43 31    	vgetexpsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 43 b4 f0 23 01 00 00 	vgetexpsd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 43 72 7f 	vgetexpsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 43 b2 00 04 00 00 	vgetexpsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 43 72 80 	vgetexpsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 43 b2 f8 fb ff ff 	vgetexpsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 43 f4    	vgetexpss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 43 f4    	vgetexpss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 43 f4    	vgetexpss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 43 31    	vgetexpss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 43 b4 f0 23 01 00 00 	vgetexpss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 43 72 7f 	vgetexpss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 43 b2 00 02 00 00 	vgetexpss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 43 72 80 	vgetexpss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 43 b2 fc fd ff ff 	vgetexpss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 47 27 f4 ab 	vgetmantsd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 c7 27 f4 ab 	vgetmantsd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 03 95 17 27 f4 ab 	vgetmantsd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 47 27 f4 7b 	vgetmantsd \$0x7b,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 17 27 f4 7b 	vgetmantsd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 27 31 7b 	vgetmantsd \$0x7b,\(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 23 95 47 27 b4 f0 23 01 00 00 7b 	vgetmantsd \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 27 72 7f 7b 	vgetmantsd \$0x7b,0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 27 b2 00 04 00 00 7b 	vgetmantsd \$0x7b,0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 27 72 80 7b 	vgetmantsd \$0x7b,-0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 27 b2 f8 fb ff ff 7b 	vgetmantsd \$0x7b,-0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 47 27 f4 ab 	vgetmantss \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 c7 27 f4 ab 	vgetmantss \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 03 15 17 27 f4 ab 	vgetmantss \$0xab,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 47 27 f4 7b 	vgetmantss \$0x7b,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 17 27 f4 7b 	vgetmantss \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 27 31 7b 	vgetmantss \$0x7b,\(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 23 15 47 27 b4 f0 23 01 00 00 7b 	vgetmantss \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 27 72 7f 7b 	vgetmantss \$0x7b,0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 27 b2 00 02 00 00 7b 	vgetmantss \$0x7b,0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 27 72 80 7b 	vgetmantss \$0x7b,-0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 27 b2 fc fd ff ff 7b 	vgetmantss \$0x7b,-0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 47 5f f4    	vmaxsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 5f f4    	vmaxsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 5f f4    	vmaxsd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5f 31    	vmaxsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 5f b4 f0 23 01 00 00 	vmaxsd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5f 72 7f 	vmaxsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5f b2 00 04 00 00 	vmaxsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5f 72 80 	vmaxsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5f b2 f8 fb ff ff 	vmaxsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 5f f4    	vmaxss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 5f f4    	vmaxss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 5f f4    	vmaxss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5f 31    	vmaxss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 5f b4 f0 23 01 00 00 	vmaxss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5f 72 7f 	vmaxss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5f b2 00 02 00 00 	vmaxss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5f 72 80 	vmaxss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5f b2 fc fd ff ff 	vmaxss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 47 5d f4    	vminsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 5d f4    	vminsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 5d f4    	vminsd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5d 31    	vminsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 5d b4 f0 23 01 00 00 	vminsd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5d 72 7f 	vminsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5d b2 00 04 00 00 	vminsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5d 72 80 	vminsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5d b2 f8 fb ff ff 	vminsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 5d f4    	vminss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 5d f4    	vminss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 5d f4    	vminss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5d 31    	vminss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 5d b4 f0 23 01 00 00 	vminss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5d 72 7f 	vminss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5d b2 00 02 00 00 	vminss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5d 72 80 	vminss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5d b2 fc fd ff ff 	vminss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 31    	vmovsd \(%rcx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff cf 10 31    	vmovsd \(%rcx\),%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 21 ff 4f 10 b4 f0 23 01 00 00 	vmovsd 0x123\(%rax,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 72 7f 	vmovsd 0x3f8\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 b2 00 04 00 00 	vmovsd 0x400\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 72 80 	vmovsd -0x400\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 b2 f8 fb ff ff 	vmovsd -0x408\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 31    	vmovsd %xmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 21 ff 4f 11 b4 f0 23 01 00 00 	vmovsd %xmm30,0x123\(%rax,%r14,8\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 72 7f 	vmovsd %xmm30,0x3f8\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 b2 00 04 00 00 	vmovsd %xmm30,0x400\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 72 80 	vmovsd %xmm30,-0x400\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 b2 f8 fb ff ff 	vmovsd %xmm30,-0x408\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 47 10 f4    	vmovsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 10 f4    	vmovsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 31    	vmovss \(%rcx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e cf 10 31    	vmovss \(%rcx\),%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 21 7e 4f 10 b4 f0 23 01 00 00 	vmovss 0x123\(%rax,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 72 7f 	vmovss 0x1fc\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 b2 00 02 00 00 	vmovss 0x200\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 72 80 	vmovss -0x200\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 b2 fc fd ff ff 	vmovss -0x204\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 31    	vmovss %xmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 21 7e 4f 11 b4 f0 23 01 00 00 	vmovss %xmm30,0x123\(%rax,%r14,8\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 72 7f 	vmovss %xmm30,0x1fc\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 b2 00 02 00 00 	vmovss %xmm30,0x200\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 72 80 	vmovss %xmm30,-0x200\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 b2 fc fd ff ff 	vmovss %xmm30,-0x204\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 10 f4    	vmovss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 10 f4    	vmovss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 47 59 f4    	vmulsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 59 f4    	vmulsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 59 f4    	vmulsd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 57 59 f4    	vmulsd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 37 59 f4    	vmulsd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 77 59 f4    	vmulsd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 59 31    	vmulsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 59 b4 f0 23 01 00 00 	vmulsd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 59 72 7f 	vmulsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 59 b2 00 04 00 00 	vmulsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 59 72 80 	vmulsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 59 b2 f8 fb ff ff 	vmulsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 59 f4    	vmulss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 59 f4    	vmulss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 59 f4    	vmulss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 57 59 f4    	vmulss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 37 59 f4    	vmulss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 77 59 f4    	vmulss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 59 31    	vmulss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 59 b4 f0 23 01 00 00 	vmulss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 59 72 7f 	vmulss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 59 b2 00 02 00 00 	vmulss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 59 72 80 	vmulss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 59 b2 fc fd ff ff 	vmulss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 4d f4    	vrcp14sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 4d f4    	vrcp14sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 95 47 4d 31    	vrcp14sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 4d b4 f0 23 01 00 00 	vrcp14sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4d 72 7f 	vrcp14sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4d b2 00 04 00 00 	vrcp14sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4d 72 80 	vrcp14sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4d b2 f8 fb ff ff 	vrcp14sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 4d f4    	vrcp14ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 4d f4    	vrcp14ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 15 47 4d 31    	vrcp14ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 4d b4 f0 23 01 00 00 	vrcp14ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4d 72 7f 	vrcp14ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4d b2 00 02 00 00 	vrcp14ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4d 72 80 	vrcp14ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4d b2 fc fd ff ff 	vrcp14ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 cb f4    	vrcp28ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 cb f4    	vrcp28ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 cb f4    	vrcp28ss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cb 31    	vrcp28ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 cb b4 f0 23 01 00 00 	vrcp28ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cb 72 7f 	vrcp28ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cb b2 00 02 00 00 	vrcp28ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cb 72 80 	vrcp28ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cb b2 fc fd ff ff 	vrcp28ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 cb f4    	vrcp28sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 cb f4    	vrcp28sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 cb f4    	vrcp28sd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cb 31    	vrcp28sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 cb b4 f0 23 01 00 00 	vrcp28sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cb 72 7f 	vrcp28sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cb b2 00 04 00 00 	vrcp28sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cb 72 80 	vrcp28sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cb b2 f8 fb ff ff 	vrcp28sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 4f f4    	vrsqrt14sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 4f f4    	vrsqrt14sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 95 47 4f 31    	vrsqrt14sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 4f b4 f0 23 01 00 00 	vrsqrt14sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4f 72 7f 	vrsqrt14sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4f b2 00 04 00 00 	vrsqrt14sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4f 72 80 	vrsqrt14sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4f b2 f8 fb ff ff 	vrsqrt14sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 4f f4    	vrsqrt14ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 4f f4    	vrsqrt14ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 15 47 4f 31    	vrsqrt14ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 4f b4 f0 23 01 00 00 	vrsqrt14ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4f 72 7f 	vrsqrt14ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4f b2 00 02 00 00 	vrsqrt14ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4f 72 80 	vrsqrt14ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4f b2 fc fd ff ff 	vrsqrt14ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 cd f4    	vrsqrt28ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 cd f4    	vrsqrt28ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 cd f4    	vrsqrt28ss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cd 31    	vrsqrt28ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 cd b4 f0 23 01 00 00 	vrsqrt28ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cd 72 7f 	vrsqrt28ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cd b2 00 02 00 00 	vrsqrt28ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cd 72 80 	vrsqrt28ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cd b2 fc fd ff ff 	vrsqrt28ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 cd f4    	vrsqrt28sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 cd f4    	vrsqrt28sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 cd f4    	vrsqrt28sd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cd 31    	vrsqrt28sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 cd b4 f0 23 01 00 00 	vrsqrt28sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cd 72 7f 	vrsqrt28sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cd b2 00 04 00 00 	vrsqrt28sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cd 72 80 	vrsqrt28sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cd b2 f8 fb ff ff 	vrsqrt28sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 47 51 f4    	vsqrtsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 51 f4    	vsqrtsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 51 f4    	vsqrtsd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 57 51 f4    	vsqrtsd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 37 51 f4    	vsqrtsd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 77 51 f4    	vsqrtsd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 51 31    	vsqrtsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 51 b4 f0 23 01 00 00 	vsqrtsd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 51 72 7f 	vsqrtsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 51 b2 00 04 00 00 	vsqrtsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 51 72 80 	vsqrtsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 51 b2 f8 fb ff ff 	vsqrtsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 51 f4    	vsqrtss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 51 f4    	vsqrtss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 51 f4    	vsqrtss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 57 51 f4    	vsqrtss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 37 51 f4    	vsqrtss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 77 51 f4    	vsqrtss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 51 31    	vsqrtss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 51 b4 f0 23 01 00 00 	vsqrtss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 51 72 7f 	vsqrtss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 51 b2 00 02 00 00 	vsqrtss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 51 72 80 	vsqrtss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 51 b2 fc fd ff ff 	vsqrtss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 47 5c f4    	vsubsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 5c f4    	vsubsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 5c f4    	vsubsd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 57 5c f4    	vsubsd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 37 5c f4    	vsubsd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 77 5c f4    	vsubsd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5c 31    	vsubsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 5c b4 f0 23 01 00 00 	vsubsd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5c 72 7f 	vsubsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5c b2 00 04 00 00 	vsubsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5c 72 80 	vsubsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5c b2 f8 fb ff ff 	vsubsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 5c f4    	vsubss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 5c f4    	vsubss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 5c f4    	vsubss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 57 5c f4    	vsubss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 37 5c f4    	vsubss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 77 5c f4    	vsubss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5c 31    	vsubss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 5c b4 f0 23 01 00 00 	vsubss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5c 72 7f 	vsubss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5c b2 00 02 00 00 	vsubss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5c 72 80 	vsubss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5c b2 fc fd ff ff 	vsubss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 fd 48 2e f5    	vucomisd %xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 01 fd 18 2e f5    	vucomisd \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2e 31    	vucomisd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:	62 21 fd 48 2e b4 f0 23 01 00 00 	vucomisd 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2e 72 7f 	vucomisd 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2e b2 00 04 00 00 	vucomisd 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2e 72 80 	vucomisd -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2e b2 f8 fb ff ff 	vucomisd -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 01 7c 48 2e f5    	vucomiss %xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 01 7c 18 2e f5    	vucomiss \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2e 31    	vucomiss \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:	62 21 7c 48 2e b4 f0 23 01 00 00 	vucomiss 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2e 72 7f 	vucomiss 0x1fc\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2e b2 00 02 00 00 	vucomiss 0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2e 72 80 	vucomiss -0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2e b2 fc fd ff ff 	vucomiss -0x204\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 91 7f 48 79 c6    	vcvtsd2usi %xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 18 79 c6    	vcvtsd2usi \{rn-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 58 79 c6    	vcvtsd2usi \{ru-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 38 79 c6    	vcvtsd2usi \{rd-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 78 79 c6    	vcvtsd2usi \{rz-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 01    	vcvtsd2usi \(%rcx\),%eax
[ 	]*[a-f0-9]+:	62 b1 7f 48 79 84 f0 23 01 00 00 	vcvtsd2usi 0x123\(%rax,%r14,8\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 42 7f 	vcvtsd2usi 0x3f8\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 82 00 04 00 00 	vcvtsd2usi 0x400\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 42 80 	vcvtsd2usi -0x400\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 82 f8 fb ff ff 	vcvtsd2usi -0x408\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 91 7f 48 79 ee    	vcvtsd2usi %xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 18 79 ee    	vcvtsd2usi \{rn-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 58 79 ee    	vcvtsd2usi \{ru-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 38 79 ee    	vcvtsd2usi \{rd-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 78 79 ee    	vcvtsd2usi \{rz-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 29    	vcvtsd2usi \(%rcx\),%ebp
[ 	]*[a-f0-9]+:	62 b1 7f 48 79 ac f0 23 01 00 00 	vcvtsd2usi 0x123\(%rax,%r14,8\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 6a 7f 	vcvtsd2usi 0x3f8\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 aa 00 04 00 00 	vcvtsd2usi 0x400\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 6a 80 	vcvtsd2usi -0x400\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 aa f8 fb ff ff 	vcvtsd2usi -0x408\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 11 7f 48 79 ee    	vcvtsd2usi %xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 18 79 ee    	vcvtsd2usi \{rn-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 58 79 ee    	vcvtsd2usi \{ru-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 38 79 ee    	vcvtsd2usi \{rd-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 78 79 ee    	vcvtsd2usi \{rz-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 71 7f 48 79 29    	vcvtsd2usi \(%rcx\),%r13d
[ 	]*[a-f0-9]+:	62 31 7f 48 79 ac f0 23 01 00 00 	vcvtsd2usi 0x123\(%rax,%r14,8\),%r13d
[ 	]*[a-f0-9]+:	62 71 7f 48 79 6a 7f 	vcvtsd2usi 0x3f8\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 71 7f 48 79 aa 00 04 00 00 	vcvtsd2usi 0x400\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 71 7f 48 79 6a 80 	vcvtsd2usi -0x400\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 71 7f 48 79 aa f8 fb ff ff 	vcvtsd2usi -0x408\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 91 ff 48 79 c6    	vcvtsd2usi %xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 18 79 c6    	vcvtsd2usi \{rn-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 58 79 c6    	vcvtsd2usi \{ru-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 38 79 c6    	vcvtsd2usi \{rd-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 78 79 c6    	vcvtsd2usi \{rz-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 01    	vcvtsd2usi \(%rcx\),%rax
[ 	]*[a-f0-9]+:	62 b1 ff 48 79 84 f0 23 01 00 00 	vcvtsd2usi 0x123\(%rax,%r14,8\),%rax
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 42 7f 	vcvtsd2usi 0x3f8\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 82 00 04 00 00 	vcvtsd2usi 0x400\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 42 80 	vcvtsd2usi -0x400\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 82 f8 fb ff ff 	vcvtsd2usi -0x408\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 11 ff 48 79 c6    	vcvtsd2usi %xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 18 79 c6    	vcvtsd2usi \{rn-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 58 79 c6    	vcvtsd2usi \{ru-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 38 79 c6    	vcvtsd2usi \{rd-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 78 79 c6    	vcvtsd2usi \{rz-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 71 ff 48 79 01    	vcvtsd2usi \(%rcx\),%r8
[ 	]*[a-f0-9]+:	62 31 ff 48 79 84 f0 23 01 00 00 	vcvtsd2usi 0x123\(%rax,%r14,8\),%r8
[ 	]*[a-f0-9]+:	62 71 ff 48 79 42 7f 	vcvtsd2usi 0x3f8\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 71 ff 48 79 82 00 04 00 00 	vcvtsd2usi 0x400\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 71 ff 48 79 42 80 	vcvtsd2usi -0x400\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 71 ff 48 79 82 f8 fb ff ff 	vcvtsd2usi -0x408\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 91 7e 48 79 c6    	vcvtss2usi %xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 18 79 c6    	vcvtss2usi \{rn-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 58 79 c6    	vcvtss2usi \{ru-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 38 79 c6    	vcvtss2usi \{rd-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 78 79 c6    	vcvtss2usi \{rz-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 01    	vcvtss2usi \(%rcx\),%eax
[ 	]*[a-f0-9]+:	62 b1 7e 48 79 84 f0 23 01 00 00 	vcvtss2usi 0x123\(%rax,%r14,8\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 42 7f 	vcvtss2usi 0x1fc\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 82 00 02 00 00 	vcvtss2usi 0x200\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 42 80 	vcvtss2usi -0x200\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 82 fc fd ff ff 	vcvtss2usi -0x204\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 91 7e 48 79 ee    	vcvtss2usi %xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 18 79 ee    	vcvtss2usi \{rn-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 58 79 ee    	vcvtss2usi \{ru-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 38 79 ee    	vcvtss2usi \{rd-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 78 79 ee    	vcvtss2usi \{rz-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 29    	vcvtss2usi \(%rcx\),%ebp
[ 	]*[a-f0-9]+:	62 b1 7e 48 79 ac f0 23 01 00 00 	vcvtss2usi 0x123\(%rax,%r14,8\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 6a 7f 	vcvtss2usi 0x1fc\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 aa 00 02 00 00 	vcvtss2usi 0x200\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 6a 80 	vcvtss2usi -0x200\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 aa fc fd ff ff 	vcvtss2usi -0x204\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 11 7e 48 79 ee    	vcvtss2usi %xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 18 79 ee    	vcvtss2usi \{rn-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 58 79 ee    	vcvtss2usi \{ru-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 38 79 ee    	vcvtss2usi \{rd-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 78 79 ee    	vcvtss2usi \{rz-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 71 7e 48 79 29    	vcvtss2usi \(%rcx\),%r13d
[ 	]*[a-f0-9]+:	62 31 7e 48 79 ac f0 23 01 00 00 	vcvtss2usi 0x123\(%rax,%r14,8\),%r13d
[ 	]*[a-f0-9]+:	62 71 7e 48 79 6a 7f 	vcvtss2usi 0x1fc\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 71 7e 48 79 aa 00 02 00 00 	vcvtss2usi 0x200\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 71 7e 48 79 6a 80 	vcvtss2usi -0x200\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 71 7e 48 79 aa fc fd ff ff 	vcvtss2usi -0x204\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 91 fe 48 79 c6    	vcvtss2usi %xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 18 79 c6    	vcvtss2usi \{rn-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 58 79 c6    	vcvtss2usi \{ru-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 38 79 c6    	vcvtss2usi \{rd-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 78 79 c6    	vcvtss2usi \{rz-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 01    	vcvtss2usi \(%rcx\),%rax
[ 	]*[a-f0-9]+:	62 b1 fe 48 79 84 f0 23 01 00 00 	vcvtss2usi 0x123\(%rax,%r14,8\),%rax
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 42 7f 	vcvtss2usi 0x1fc\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 82 00 02 00 00 	vcvtss2usi 0x200\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 42 80 	vcvtss2usi -0x200\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 82 fc fd ff ff 	vcvtss2usi -0x204\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 11 fe 48 79 c6    	vcvtss2usi %xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 18 79 c6    	vcvtss2usi \{rn-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 58 79 c6    	vcvtss2usi \{ru-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 38 79 c6    	vcvtss2usi \{rd-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 78 79 c6    	vcvtss2usi \{rz-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 71 fe 48 79 01    	vcvtss2usi \(%rcx\),%r8
[ 	]*[a-f0-9]+:	62 31 fe 48 79 84 f0 23 01 00 00 	vcvtss2usi 0x123\(%rax,%r14,8\),%r8
[ 	]*[a-f0-9]+:	62 71 fe 48 79 42 7f 	vcvtss2usi 0x1fc\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 71 fe 48 79 82 00 02 00 00 	vcvtss2usi 0x200\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 71 fe 48 79 42 80 	vcvtss2usi -0x200\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 71 fe 48 79 82 fc fd ff ff 	vcvtss2usi -0x204\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 61 17 40 7b f0    	vcvtusi2sd %eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 7b f5    	vcvtusi2sd %ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 17 40 7b f5    	vcvtusi2sd %r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 7b 31    	vcvtusi2sdl \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 17 40 7b b4 f0 23 01 00 00 	vcvtusi2sdl 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 7b 72 7f 	vcvtusi2sdl 0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 7b b2 00 02 00 00 	vcvtusi2sdl 0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 7b 72 80 	vcvtusi2sdl -0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 7b b2 fc fd ff ff 	vcvtusi2sdl -0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 7b f0    	vcvtusi2sd %rax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 10 7b f0    	vcvtusi2sd %rax,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 50 7b f0    	vcvtusi2sd %rax,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 30 7b f0    	vcvtusi2sd %rax,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 70 7b f0    	vcvtusi2sd %rax,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 40 7b f0    	vcvtusi2sd %r8,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 10 7b f0    	vcvtusi2sd %r8,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 50 7b f0    	vcvtusi2sd %r8,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 30 7b f0    	vcvtusi2sd %r8,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 70 7b f0    	vcvtusi2sd %r8,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 7b 31    	vcvtusi2sdq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 97 40 7b b4 f0 23 01 00 00 	vcvtusi2sdq 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 7b 72 7f 	vcvtusi2sdq 0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 7b b2 00 04 00 00 	vcvtusi2sdq 0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 7b 72 80 	vcvtusi2sdq -0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 7b b2 f8 fb ff ff 	vcvtusi2sdq -0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b f0    	vcvtusi2ss %eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 10 7b f0    	vcvtusi2ss %eax,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 50 7b f0    	vcvtusi2ss %eax,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 30 7b f0    	vcvtusi2ss %eax,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 70 7b f0    	vcvtusi2ss %eax,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b f5    	vcvtusi2ss %ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 10 7b f5    	vcvtusi2ss %ebp,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 50 7b f5    	vcvtusi2ss %ebp,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 30 7b f5    	vcvtusi2ss %ebp,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 70 7b f5    	vcvtusi2ss %ebp,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 40 7b f5    	vcvtusi2ss %r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 10 7b f5    	vcvtusi2ss %r13d,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 50 7b f5    	vcvtusi2ss %r13d,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 30 7b f5    	vcvtusi2ss %r13d,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 70 7b f5    	vcvtusi2ss %r13d,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b 31    	vcvtusi2ssl \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 16 40 7b b4 f0 23 01 00 00 	vcvtusi2ssl 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b 72 7f 	vcvtusi2ssl 0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b b2 00 02 00 00 	vcvtusi2ssl 0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b 72 80 	vcvtusi2ssl -0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b b2 fc fd ff ff 	vcvtusi2ssl -0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 7b f0    	vcvtusi2ss %rax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 10 7b f0    	vcvtusi2ss %rax,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 50 7b f0    	vcvtusi2ss %rax,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 30 7b f0    	vcvtusi2ss %rax,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 70 7b f0    	vcvtusi2ss %rax,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 40 7b f0    	vcvtusi2ss %r8,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 10 7b f0    	vcvtusi2ss %r8,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 50 7b f0    	vcvtusi2ss %r8,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 30 7b f0    	vcvtusi2ss %r8,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 70 7b f0    	vcvtusi2ss %r8,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 7b 31    	vcvtusi2ssq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 96 40 7b b4 f0 23 01 00 00 	vcvtusi2ssq 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 7b 72 7f 	vcvtusi2ssq 0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 7b b2 00 04 00 00 	vcvtusi2ssq 0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 7b 72 80 	vcvtusi2ssq -0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 7b b2 f8 fb ff ff 	vcvtusi2ssq -0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 02 95 47 2d f4    	vscalefsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 2d f4    	vscalefsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 2d f4    	vscalefsd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 2d f4    	vscalefsd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 2d f4    	vscalefsd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 2d f4    	vscalefsd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 2d 31    	vscalefsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 2d b4 f0 23 01 00 00 	vscalefsd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 2d 72 7f 	vscalefsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 2d b2 00 04 00 00 	vscalefsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 2d 72 80 	vscalefsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 2d b2 f8 fb ff ff 	vscalefsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 2d f4    	vscalefss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 2d f4    	vscalefss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 2d f4    	vscalefss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 2d f4    	vscalefss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 2d f4    	vscalefss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 2d f4    	vscalefss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 2d 31    	vscalefss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 2d b4 f0 23 01 00 00 	vscalefss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 2d 72 7f 	vscalefss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 2d b2 00 02 00 00 	vscalefss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 2d 72 80 	vscalefss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 2d b2 fc fd ff ff 	vscalefss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 47 55 f4 ab 	vfixupimmss \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 c7 55 f4 ab 	vfixupimmss \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 03 15 17 55 f4 ab 	vfixupimmss \$0xab,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 47 55 f4 7b 	vfixupimmss \$0x7b,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 17 55 f4 7b 	vfixupimmss \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 55 31 7b 	vfixupimmss \$0x7b,\(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 23 15 47 55 b4 f0 23 01 00 00 7b 	vfixupimmss \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 55 72 7f 7b 	vfixupimmss \$0x7b,0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 55 b2 00 02 00 00 7b 	vfixupimmss \$0x7b,0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 55 72 80 7b 	vfixupimmss \$0x7b,-0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 55 b2 fc fd ff ff 7b 	vfixupimmss \$0x7b,-0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 47 55 f4 ab 	vfixupimmsd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 c7 55 f4 ab 	vfixupimmsd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 03 95 17 55 f4 ab 	vfixupimmsd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 47 55 f4 7b 	vfixupimmsd \$0x7b,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 17 55 f4 7b 	vfixupimmsd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 55 31 7b 	vfixupimmsd \$0x7b,\(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 23 95 47 55 b4 f0 23 01 00 00 7b 	vfixupimmsd \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 55 72 7f 7b 	vfixupimmsd \$0x7b,0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 55 b2 00 04 00 00 7b 	vfixupimmsd \$0x7b,0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 55 72 80 7b 	vfixupimmsd \$0x7b,-0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 55 b2 f8 fb ff ff 7b 	vfixupimmsd \$0x7b,-0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 47 0b f4 ab 	vrndscalesd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 c7 0b f4 ab 	vrndscalesd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 03 95 17 0b f4 ab 	vrndscalesd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 47 0b f4 7b 	vrndscalesd \$0x7b,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 17 0b f4 7b 	vrndscalesd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 0b 31 7b 	vrndscalesd \$0x7b,\(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 23 95 47 0b b4 f0 23 01 00 00 7b 	vrndscalesd \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 0b 72 7f 7b 	vrndscalesd \$0x7b,0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 0b b2 00 04 00 00 7b 	vrndscalesd \$0x7b,0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 0b 72 80 7b 	vrndscalesd \$0x7b,-0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 0b b2 f8 fb ff ff 7b 	vrndscalesd \$0x7b,-0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 47 0a f4 ab 	vrndscaless \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 c7 0a f4 ab 	vrndscaless \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 03 15 17 0a f4 ab 	vrndscaless \$0xab,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 47 0a f4 7b 	vrndscaless \$0x7b,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 17 0a f4 7b 	vrndscaless \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 0a 31 7b 	vrndscaless \$0x7b,\(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 23 15 47 0a b4 f0 23 01 00 00 7b 	vrndscaless \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 0a 72 7f 7b 	vrndscaless \$0x7b,0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 0a b2 00 02 00 00 7b 	vrndscaless \$0x7b,0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 0a 72 80 7b 	vrndscaless \$0x7b,-0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 0a b2 fc fd ff ff 7b 	vrndscaless \$0x7b,-0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 47 58 f4    	vaddsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 58 f4    	vaddsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 58 f4    	vaddsd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 57 58 f4    	vaddsd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 37 58 f4    	vaddsd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 77 58 f4    	vaddsd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 58 31    	vaddsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 58 b4 f0 34 12 00 00 	vaddsd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 58 72 7f 	vaddsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 58 b2 00 04 00 00 	vaddsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 58 72 80 	vaddsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 58 b2 f8 fb ff ff 	vaddsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 58 f4    	vaddss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 58 f4    	vaddss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 58 f4    	vaddss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 57 58 f4    	vaddss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 37 58 f4    	vaddss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 77 58 f4    	vaddss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 58 31    	vaddss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 58 b4 f0 34 12 00 00 	vaddss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 58 72 7f 	vaddss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 58 b2 00 02 00 00 	vaddss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 58 72 80 	vaddss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 58 b2 fc fd ff ff 	vaddss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec ab 	vcmpsd \$0xab,%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec ab 	vcmpsd \$0xab,\{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 7b 	vcmpsd \$0x7b,%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 7b 	vcmpsd \$0x7b,\{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 7b 	vcmpsd \$0x7b,\(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 7b 	vcmpsd \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 7b 	vcmpsd \$0x7b,0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 7b 	vcmpsd \$0x7b,0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 7b 	vcmpsd \$0x7b,-0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 7b 	vcmpsd \$0x7b,-0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 00 	vcmpeqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 00 	vcmpeqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 00 	vcmpeqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 00 	vcmpeqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 00 	vcmpeqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 00 	vcmpeqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 00 	vcmpeqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 00 	vcmpeqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 00 	vcmpeqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 00 	vcmpeqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 00 	vcmpeqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 00 	vcmpeqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 00 	vcmpeqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 00 	vcmpeqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 00 	vcmpeqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 00 	vcmpeqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 01 	vcmpltsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 01 	vcmpltsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 01 	vcmpltsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 01 	vcmpltsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 01 	vcmpltsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 01 	vcmpltsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 01 	vcmpltsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 01 	vcmpltsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 01 	vcmpltsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 01 	vcmpltsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 01 	vcmpltsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 01 	vcmpltsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 01 	vcmpltsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 01 	vcmpltsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 01 	vcmpltsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 01 	vcmpltsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 02 	vcmplesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 02 	vcmplesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 02 	vcmplesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 02 	vcmplesd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 02 	vcmplesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 02 	vcmplesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 02 	vcmplesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 02 	vcmplesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 02 	vcmplesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 02 	vcmplesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 02 	vcmplesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 02 	vcmplesd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 02 	vcmplesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 02 	vcmplesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 02 	vcmplesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 02 	vcmplesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 03 	vcmpunordsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 03 	vcmpunordsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 03 	vcmpunordsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 03 	vcmpunordsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 03 	vcmpunordsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 03 	vcmpunordsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 03 	vcmpunordsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 03 	vcmpunordsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 03 	vcmpunordsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 03 	vcmpunordsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 03 	vcmpunordsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 03 	vcmpunordsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 03 	vcmpunordsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 03 	vcmpunordsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 03 	vcmpunordsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 03 	vcmpunordsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 04 	vcmpneqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 04 	vcmpneqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 04 	vcmpneqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 04 	vcmpneqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 04 	vcmpneqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 04 	vcmpneqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 04 	vcmpneqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 04 	vcmpneqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 04 	vcmpneqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 04 	vcmpneqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 04 	vcmpneqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 04 	vcmpneqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 04 	vcmpneqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 04 	vcmpneqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 04 	vcmpneqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 04 	vcmpneqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 05 	vcmpnltsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 05 	vcmpnltsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 05 	vcmpnltsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 05 	vcmpnltsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 05 	vcmpnltsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 05 	vcmpnltsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 05 	vcmpnltsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 05 	vcmpnltsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 05 	vcmpnltsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 05 	vcmpnltsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 05 	vcmpnltsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 05 	vcmpnltsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 05 	vcmpnltsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 05 	vcmpnltsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 05 	vcmpnltsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 05 	vcmpnltsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 06 	vcmpnlesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 06 	vcmpnlesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 06 	vcmpnlesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 06 	vcmpnlesd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 06 	vcmpnlesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 06 	vcmpnlesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 06 	vcmpnlesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 06 	vcmpnlesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 06 	vcmpnlesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 06 	vcmpnlesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 06 	vcmpnlesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 06 	vcmpnlesd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 06 	vcmpnlesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 06 	vcmpnlesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 06 	vcmpnlesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 06 	vcmpnlesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 07 	vcmpordsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 07 	vcmpordsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 07 	vcmpordsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 07 	vcmpordsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 07 	vcmpordsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 07 	vcmpordsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 07 	vcmpordsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 07 	vcmpordsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 07 	vcmpordsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 07 	vcmpordsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 07 	vcmpordsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 07 	vcmpordsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 07 	vcmpordsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 07 	vcmpordsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 07 	vcmpordsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 07 	vcmpordsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 08 	vcmpeq_uqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 08 	vcmpeq_uqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 08 	vcmpeq_uqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 08 	vcmpeq_uqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 08 	vcmpeq_uqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 08 	vcmpeq_uqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 08 	vcmpeq_uqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 08 	vcmpeq_uqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 09 	vcmpngesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 09 	vcmpngesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 09 	vcmpngesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 09 	vcmpngesd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 09 	vcmpngesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 09 	vcmpngesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 09 	vcmpngesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 09 	vcmpngesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 09 	vcmpngesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 09 	vcmpngesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 09 	vcmpngesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 09 	vcmpngesd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 09 	vcmpngesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 09 	vcmpngesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 09 	vcmpngesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 09 	vcmpngesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0a 	vcmpngtsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0a 	vcmpngtsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0a 	vcmpngtsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0a 	vcmpngtsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0a 	vcmpngtsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0a 	vcmpngtsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0a 	vcmpngtsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0a 	vcmpngtsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0a 	vcmpngtsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0a 	vcmpngtsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0a 	vcmpngtsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0a 	vcmpngtsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0a 	vcmpngtsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0a 	vcmpngtsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0a 	vcmpngtsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0a 	vcmpngtsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0b 	vcmpfalsesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0b 	vcmpfalsesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0b 	vcmpfalsesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0b 	vcmpfalsesd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0b 	vcmpfalsesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0b 	vcmpfalsesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0b 	vcmpfalsesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0b 	vcmpfalsesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0b 	vcmpfalsesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0b 	vcmpfalsesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0b 	vcmpfalsesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0b 	vcmpfalsesd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0b 	vcmpfalsesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0b 	vcmpfalsesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0b 	vcmpfalsesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0b 	vcmpfalsesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0c 	vcmpneq_oqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0c 	vcmpneq_oqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0c 	vcmpneq_oqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0c 	vcmpneq_oqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0c 	vcmpneq_oqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0c 	vcmpneq_oqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0c 	vcmpneq_oqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0c 	vcmpneq_oqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0d 	vcmpgesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0d 	vcmpgesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0d 	vcmpgesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0d 	vcmpgesd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0d 	vcmpgesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0d 	vcmpgesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0d 	vcmpgesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0d 	vcmpgesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0d 	vcmpgesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0d 	vcmpgesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0d 	vcmpgesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0d 	vcmpgesd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0d 	vcmpgesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0d 	vcmpgesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0d 	vcmpgesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0d 	vcmpgesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0e 	vcmpgtsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0e 	vcmpgtsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0e 	vcmpgtsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0e 	vcmpgtsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0e 	vcmpgtsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0e 	vcmpgtsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0e 	vcmpgtsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0e 	vcmpgtsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0e 	vcmpgtsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0e 	vcmpgtsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0e 	vcmpgtsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0e 	vcmpgtsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0e 	vcmpgtsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0e 	vcmpgtsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0e 	vcmpgtsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0e 	vcmpgtsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0f 	vcmptruesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0f 	vcmptruesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0f 	vcmptruesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0f 	vcmptruesd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0f 	vcmptruesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0f 	vcmptruesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0f 	vcmptruesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0f 	vcmptruesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 0f 	vcmptruesd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 0f 	vcmptruesd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 0f 	vcmptruesd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 0f 	vcmptruesd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 0f 	vcmptruesd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 0f 	vcmptruesd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 0f 	vcmptruesd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 0f 	vcmptruesd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 10 	vcmpeq_ossd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 10 	vcmpeq_ossd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 10 	vcmpeq_ossd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 10 	vcmpeq_ossd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 10 	vcmpeq_ossd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 10 	vcmpeq_ossd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 10 	vcmpeq_ossd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 10 	vcmpeq_ossd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 11 	vcmplt_oqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 11 	vcmplt_oqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 11 	vcmplt_oqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 11 	vcmplt_oqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 11 	vcmplt_oqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 11 	vcmplt_oqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 11 	vcmplt_oqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 11 	vcmplt_oqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 12 	vcmple_oqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 12 	vcmple_oqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 12 	vcmple_oqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 12 	vcmple_oqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 12 	vcmple_oqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 12 	vcmple_oqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 12 	vcmple_oqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 12 	vcmple_oqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 13 	vcmpunord_ssd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 13 	vcmpunord_ssd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 13 	vcmpunord_ssd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 13 	vcmpunord_ssd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 13 	vcmpunord_ssd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 13 	vcmpunord_ssd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 13 	vcmpunord_ssd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 13 	vcmpunord_ssd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 14 	vcmpneq_ussd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 14 	vcmpneq_ussd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 14 	vcmpneq_ussd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 14 	vcmpneq_ussd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 14 	vcmpneq_ussd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 14 	vcmpneq_ussd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 14 	vcmpneq_ussd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 14 	vcmpneq_ussd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 15 	vcmpnlt_uqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 15 	vcmpnlt_uqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 15 	vcmpnlt_uqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 15 	vcmpnlt_uqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 15 	vcmpnlt_uqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 15 	vcmpnlt_uqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 15 	vcmpnlt_uqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 15 	vcmpnlt_uqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 16 	vcmpnle_uqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 16 	vcmpnle_uqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 16 	vcmpnle_uqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 16 	vcmpnle_uqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 16 	vcmpnle_uqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 16 	vcmpnle_uqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 16 	vcmpnle_uqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 16 	vcmpnle_uqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 17 	vcmpord_ssd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 17 	vcmpord_ssd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 17 	vcmpord_ssd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 17 	vcmpord_ssd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 17 	vcmpord_ssd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 17 	vcmpord_ssd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 17 	vcmpord_ssd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 17 	vcmpord_ssd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 18 	vcmpeq_ussd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 18 	vcmpeq_ussd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 18 	vcmpeq_ussd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 18 	vcmpeq_ussd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 18 	vcmpeq_ussd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 18 	vcmpeq_ussd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 18 	vcmpeq_ussd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 18 	vcmpeq_ussd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 19 	vcmpnge_uqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 19 	vcmpnge_uqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 19 	vcmpnge_uqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 19 	vcmpnge_uqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 19 	vcmpnge_uqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 19 	vcmpnge_uqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 19 	vcmpnge_uqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 19 	vcmpnge_uqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1a 	vcmpngt_uqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1a 	vcmpngt_uqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1a 	vcmpngt_uqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 1a 	vcmpngt_uqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1a 	vcmpngt_uqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1a 	vcmpngt_uqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1a 	vcmpngt_uqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1a 	vcmpngt_uqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1b 	vcmpfalse_ossd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1b 	vcmpfalse_ossd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1b 	vcmpfalse_ossd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 1b 	vcmpfalse_ossd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1b 	vcmpfalse_ossd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1b 	vcmpfalse_ossd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1b 	vcmpfalse_ossd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1b 	vcmpfalse_ossd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1c 	vcmpneq_ossd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1c 	vcmpneq_ossd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1c 	vcmpneq_ossd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 1c 	vcmpneq_ossd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1c 	vcmpneq_ossd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1c 	vcmpneq_ossd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1c 	vcmpneq_ossd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1c 	vcmpneq_ossd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1d 	vcmpge_oqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1d 	vcmpge_oqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1d 	vcmpge_oqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 1d 	vcmpge_oqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1d 	vcmpge_oqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1d 	vcmpge_oqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1d 	vcmpge_oqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1d 	vcmpge_oqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1e 	vcmpgt_oqsd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1e 	vcmpgt_oqsd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1e 	vcmpgt_oqsd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 1e 	vcmpgt_oqsd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1e 	vcmpgt_oqsd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1e 	vcmpgt_oqsd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1e 	vcmpgt_oqsd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1e 	vcmpgt_oqsd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 47 c2 ec 1f 	vcmptrue_ussd %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 97 17 c2 ec 1f 	vcmptrue_ussd \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 29 1f 	vcmptrue_ussd \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 97 47 c2 ac f0 34 12 00 00 1f 	vcmptrue_ussd 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 7f 1f 	vcmptrue_ussd 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa 00 04 00 00 1f 	vcmptrue_ussd 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 6a 80 1f 	vcmptrue_ussd -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 97 47 c2 aa f8 fb ff ff 1f 	vcmptrue_ussd -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec ab 	vcmpss \$0xab,%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec ab 	vcmpss \$0xab,\{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 7b 	vcmpss \$0x7b,%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 7b 	vcmpss \$0x7b,\{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 7b 	vcmpss \$0x7b,\(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 7b 	vcmpss \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 7b 	vcmpss \$0x7b,0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 7b 	vcmpss \$0x7b,0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 7b 	vcmpss \$0x7b,-0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 7b 	vcmpss \$0x7b,-0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 00 	vcmpeqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 00 	vcmpeqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 00 	vcmpeqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 00 	vcmpeqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 00 	vcmpeqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 00 	vcmpeqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 00 	vcmpeqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 00 	vcmpeqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 00 	vcmpeqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 00 	vcmpeqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 00 	vcmpeqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 00 	vcmpeqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 00 	vcmpeqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 00 	vcmpeqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 00 	vcmpeqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 00 	vcmpeqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 01 	vcmpltss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 01 	vcmpltss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 01 	vcmpltss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 01 	vcmpltss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 01 	vcmpltss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 01 	vcmpltss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 01 	vcmpltss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 01 	vcmpltss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 01 	vcmpltss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 01 	vcmpltss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 01 	vcmpltss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 01 	vcmpltss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 01 	vcmpltss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 01 	vcmpltss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 01 	vcmpltss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 01 	vcmpltss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 02 	vcmpless %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 02 	vcmpless \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 02 	vcmpless \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 02 	vcmpless 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 02 	vcmpless 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 02 	vcmpless 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 02 	vcmpless -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 02 	vcmpless -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 02 	vcmpless %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 02 	vcmpless \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 02 	vcmpless \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 02 	vcmpless 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 02 	vcmpless 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 02 	vcmpless 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 02 	vcmpless -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 02 	vcmpless -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 03 	vcmpunordss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 03 	vcmpunordss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 03 	vcmpunordss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 03 	vcmpunordss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 03 	vcmpunordss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 03 	vcmpunordss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 03 	vcmpunordss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 03 	vcmpunordss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 03 	vcmpunordss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 03 	vcmpunordss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 03 	vcmpunordss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 03 	vcmpunordss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 03 	vcmpunordss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 03 	vcmpunordss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 03 	vcmpunordss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 03 	vcmpunordss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 04 	vcmpneqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 04 	vcmpneqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 04 	vcmpneqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 04 	vcmpneqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 04 	vcmpneqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 04 	vcmpneqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 04 	vcmpneqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 04 	vcmpneqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 04 	vcmpneqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 04 	vcmpneqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 04 	vcmpneqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 04 	vcmpneqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 04 	vcmpneqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 04 	vcmpneqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 04 	vcmpneqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 04 	vcmpneqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 05 	vcmpnltss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 05 	vcmpnltss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 05 	vcmpnltss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 05 	vcmpnltss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 05 	vcmpnltss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 05 	vcmpnltss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 05 	vcmpnltss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 05 	vcmpnltss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 05 	vcmpnltss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 05 	vcmpnltss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 05 	vcmpnltss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 05 	vcmpnltss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 05 	vcmpnltss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 05 	vcmpnltss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 05 	vcmpnltss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 05 	vcmpnltss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 06 	vcmpnless %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 06 	vcmpnless \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 06 	vcmpnless \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 06 	vcmpnless 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 06 	vcmpnless 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 06 	vcmpnless 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 06 	vcmpnless -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 06 	vcmpnless -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 06 	vcmpnless %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 06 	vcmpnless \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 06 	vcmpnless \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 06 	vcmpnless 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 06 	vcmpnless 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 06 	vcmpnless 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 06 	vcmpnless -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 06 	vcmpnless -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 07 	vcmpordss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 07 	vcmpordss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 07 	vcmpordss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 07 	vcmpordss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 07 	vcmpordss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 07 	vcmpordss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 07 	vcmpordss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 07 	vcmpordss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 07 	vcmpordss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 07 	vcmpordss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 07 	vcmpordss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 07 	vcmpordss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 07 	vcmpordss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 07 	vcmpordss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 07 	vcmpordss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 07 	vcmpordss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 08 	vcmpeq_uqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 08 	vcmpeq_uqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 08 	vcmpeq_uqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 08 	vcmpeq_uqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 08 	vcmpeq_uqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 08 	vcmpeq_uqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 08 	vcmpeq_uqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 08 	vcmpeq_uqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 09 	vcmpngess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 09 	vcmpngess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 09 	vcmpngess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 09 	vcmpngess 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 09 	vcmpngess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 09 	vcmpngess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 09 	vcmpngess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 09 	vcmpngess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 09 	vcmpngess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 09 	vcmpngess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 09 	vcmpngess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 09 	vcmpngess 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 09 	vcmpngess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 09 	vcmpngess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 09 	vcmpngess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 09 	vcmpngess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0a 	vcmpngtss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0a 	vcmpngtss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0a 	vcmpngtss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0a 	vcmpngtss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0a 	vcmpngtss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0a 	vcmpngtss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0a 	vcmpngtss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0a 	vcmpngtss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0a 	vcmpngtss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0a 	vcmpngtss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0a 	vcmpngtss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0a 	vcmpngtss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0a 	vcmpngtss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0a 	vcmpngtss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0a 	vcmpngtss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0a 	vcmpngtss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0b 	vcmpfalsess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0b 	vcmpfalsess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0b 	vcmpfalsess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0b 	vcmpfalsess 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0b 	vcmpfalsess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0b 	vcmpfalsess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0b 	vcmpfalsess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0b 	vcmpfalsess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0b 	vcmpfalsess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0b 	vcmpfalsess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0b 	vcmpfalsess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0b 	vcmpfalsess 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0b 	vcmpfalsess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0b 	vcmpfalsess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0b 	vcmpfalsess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0b 	vcmpfalsess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0c 	vcmpneq_oqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0c 	vcmpneq_oqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0c 	vcmpneq_oqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0c 	vcmpneq_oqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0c 	vcmpneq_oqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0c 	vcmpneq_oqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0c 	vcmpneq_oqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0c 	vcmpneq_oqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0d 	vcmpgess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0d 	vcmpgess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0d 	vcmpgess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0d 	vcmpgess 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0d 	vcmpgess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0d 	vcmpgess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0d 	vcmpgess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0d 	vcmpgess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0d 	vcmpgess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0d 	vcmpgess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0d 	vcmpgess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0d 	vcmpgess 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0d 	vcmpgess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0d 	vcmpgess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0d 	vcmpgess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0d 	vcmpgess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0e 	vcmpgtss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0e 	vcmpgtss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0e 	vcmpgtss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0e 	vcmpgtss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0e 	vcmpgtss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0e 	vcmpgtss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0e 	vcmpgtss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0e 	vcmpgtss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0e 	vcmpgtss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0e 	vcmpgtss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0e 	vcmpgtss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0e 	vcmpgtss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0e 	vcmpgtss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0e 	vcmpgtss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0e 	vcmpgtss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0e 	vcmpgtss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0f 	vcmptruess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0f 	vcmptruess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0f 	vcmptruess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0f 	vcmptruess 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0f 	vcmptruess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0f 	vcmptruess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0f 	vcmptruess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0f 	vcmptruess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 0f 	vcmptruess %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 0f 	vcmptruess \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 0f 	vcmptruess \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 0f 	vcmptruess 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 0f 	vcmptruess 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 0f 	vcmptruess 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 0f 	vcmptruess -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 0f 	vcmptruess -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 10 	vcmpeq_osss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 10 	vcmpeq_osss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 10 	vcmpeq_osss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 10 	vcmpeq_osss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 10 	vcmpeq_osss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 10 	vcmpeq_osss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 10 	vcmpeq_osss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 10 	vcmpeq_osss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 11 	vcmplt_oqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 11 	vcmplt_oqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 11 	vcmplt_oqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 11 	vcmplt_oqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 11 	vcmplt_oqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 11 	vcmplt_oqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 11 	vcmplt_oqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 11 	vcmplt_oqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 12 	vcmple_oqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 12 	vcmple_oqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 12 	vcmple_oqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 12 	vcmple_oqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 12 	vcmple_oqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 12 	vcmple_oqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 12 	vcmple_oqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 12 	vcmple_oqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 13 	vcmpunord_sss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 13 	vcmpunord_sss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 13 	vcmpunord_sss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 13 	vcmpunord_sss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 13 	vcmpunord_sss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 13 	vcmpunord_sss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 13 	vcmpunord_sss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 13 	vcmpunord_sss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 14 	vcmpneq_usss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 14 	vcmpneq_usss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 14 	vcmpneq_usss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 14 	vcmpneq_usss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 14 	vcmpneq_usss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 14 	vcmpneq_usss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 14 	vcmpneq_usss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 14 	vcmpneq_usss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 15 	vcmpnlt_uqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 15 	vcmpnlt_uqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 15 	vcmpnlt_uqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 15 	vcmpnlt_uqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 15 	vcmpnlt_uqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 15 	vcmpnlt_uqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 15 	vcmpnlt_uqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 15 	vcmpnlt_uqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 16 	vcmpnle_uqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 16 	vcmpnle_uqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 16 	vcmpnle_uqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 16 	vcmpnle_uqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 16 	vcmpnle_uqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 16 	vcmpnle_uqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 16 	vcmpnle_uqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 16 	vcmpnle_uqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 17 	vcmpord_sss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 17 	vcmpord_sss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 17 	vcmpord_sss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 17 	vcmpord_sss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 17 	vcmpord_sss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 17 	vcmpord_sss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 17 	vcmpord_sss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 17 	vcmpord_sss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 18 	vcmpeq_usss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 18 	vcmpeq_usss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 18 	vcmpeq_usss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 18 	vcmpeq_usss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 18 	vcmpeq_usss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 18 	vcmpeq_usss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 18 	vcmpeq_usss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 18 	vcmpeq_usss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 19 	vcmpnge_uqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 19 	vcmpnge_uqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 19 	vcmpnge_uqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 19 	vcmpnge_uqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 19 	vcmpnge_uqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 19 	vcmpnge_uqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 19 	vcmpnge_uqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 19 	vcmpnge_uqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1a 	vcmpngt_uqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1a 	vcmpngt_uqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1a 	vcmpngt_uqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 1a 	vcmpngt_uqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1a 	vcmpngt_uqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1a 	vcmpngt_uqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1a 	vcmpngt_uqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1a 	vcmpngt_uqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1b 	vcmpfalse_osss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1b 	vcmpfalse_osss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1b 	vcmpfalse_osss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 1b 	vcmpfalse_osss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1b 	vcmpfalse_osss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1b 	vcmpfalse_osss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1b 	vcmpfalse_osss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1b 	vcmpfalse_osss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1c 	vcmpneq_osss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1c 	vcmpneq_osss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1c 	vcmpneq_osss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 1c 	vcmpneq_osss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1c 	vcmpneq_osss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1c 	vcmpneq_osss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1c 	vcmpneq_osss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1c 	vcmpneq_osss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1d 	vcmpge_oqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1d 	vcmpge_oqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1d 	vcmpge_oqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 1d 	vcmpge_oqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1d 	vcmpge_oqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1d 	vcmpge_oqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1d 	vcmpge_oqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1d 	vcmpge_oqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1e 	vcmpgt_oqss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1e 	vcmpgt_oqss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1e 	vcmpgt_oqss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 1e 	vcmpgt_oqss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1e 	vcmpgt_oqss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1e 	vcmpgt_oqss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1e 	vcmpgt_oqss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1e 	vcmpgt_oqss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 47 c2 ec 1f 	vcmptrue_usss %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 91 16 17 c2 ec 1f 	vcmptrue_usss \{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 29 1f 	vcmptrue_usss \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 b1 16 47 c2 ac f0 34 12 00 00 1f 	vcmptrue_usss 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 7f 1f 	vcmptrue_usss 0x1fc\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa 00 02 00 00 1f 	vcmptrue_usss 0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 6a 80 1f 	vcmptrue_usss -0x200\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 16 47 c2 aa fc fd ff ff 1f 	vcmptrue_usss -0x204\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 01 fd 48 2f f5    	vcomisd %xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 01 fd 18 2f f5    	vcomisd \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2f 31    	vcomisd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:	62 21 fd 48 2f b4 f0 34 12 00 00 	vcomisd 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2f 72 7f 	vcomisd 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2f b2 00 04 00 00 	vcomisd 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2f 72 80 	vcomisd -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2f b2 f8 fb ff ff 	vcomisd -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 01 7c 48 2f f5    	vcomiss %xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 01 7c 18 2f f5    	vcomiss \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2f 31    	vcomiss \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:	62 21 7c 48 2f b4 f0 34 12 00 00 	vcomiss 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2f 72 7f 	vcomiss 0x1fc\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2f b2 00 02 00 00 	vcomiss 0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2f 72 80 	vcomiss -0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2f b2 fc fd ff ff 	vcomiss -0x204\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 91 7f 18 2d c6    	vcvtsd2si \{rn-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 58 2d c6    	vcvtsd2si \{ru-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 38 2d c6    	vcvtsd2si \{rd-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 78 2d c6    	vcvtsd2si \{rz-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 18 2d ee    	vcvtsd2si \{rn-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 58 2d ee    	vcvtsd2si \{ru-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 38 2d ee    	vcvtsd2si \{rd-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 78 2d ee    	vcvtsd2si \{rz-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 11 7f 18 2d ee    	vcvtsd2si \{rn-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 58 2d ee    	vcvtsd2si \{ru-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 38 2d ee    	vcvtsd2si \{rd-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 78 2d ee    	vcvtsd2si \{rz-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 91 ff 18 2d c6    	vcvtsd2si \{rn-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 58 2d c6    	vcvtsd2si \{ru-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 38 2d c6    	vcvtsd2si \{rd-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 78 2d c6    	vcvtsd2si \{rz-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 11 ff 18 2d c6    	vcvtsd2si \{rn-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 58 2d c6    	vcvtsd2si \{ru-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 38 2d c6    	vcvtsd2si \{rd-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 78 2d c6    	vcvtsd2si \{rz-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 01 97 47 5a f4    	vcvtsd2ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 5a f4    	vcvtsd2ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 5a f4    	vcvtsd2ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 57 5a f4    	vcvtsd2ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 37 5a f4    	vcvtsd2ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 77 5a f4    	vcvtsd2ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5a 31    	vcvtsd2ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 5a b4 f0 34 12 00 00 	vcvtsd2ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5a 72 7f 	vcvtsd2ss 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5a b2 00 04 00 00 	vcvtsd2ss 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5a 72 80 	vcvtsd2ss -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5a b2 f8 fb ff ff 	vcvtsd2ss -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 17 40 2a f0    	vcvtsi2sd %eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 2a f5    	vcvtsi2sd %ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 17 40 2a f5    	vcvtsi2sd %r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 2a 31    	vcvtsi2sdl \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 17 40 2a b4 f0 34 12 00 00 	vcvtsi2sdl 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 2a 72 7f 	vcvtsi2sdl 0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 2a b2 00 02 00 00 	vcvtsi2sdl 0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 2a 72 80 	vcvtsi2sdl -0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 2a b2 fc fd ff ff 	vcvtsi2sdl -0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 2a f0    	vcvtsi2sd %rax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 10 2a f0    	vcvtsi2sd %rax,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 50 2a f0    	vcvtsi2sd %rax,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 30 2a f0    	vcvtsi2sd %rax,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 70 2a f0    	vcvtsi2sd %rax,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 40 2a f0    	vcvtsi2sd %r8,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 10 2a f0    	vcvtsi2sd %r8,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 50 2a f0    	vcvtsi2sd %r8,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 30 2a f0    	vcvtsi2sd %r8,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 70 2a f0    	vcvtsi2sd %r8,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 2a 31    	vcvtsi2sdq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 97 40 2a b4 f0 34 12 00 00 	vcvtsi2sdq 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 2a 72 7f 	vcvtsi2sdq 0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 2a b2 00 04 00 00 	vcvtsi2sdq 0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 2a 72 80 	vcvtsi2sdq -0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 2a b2 f8 fb ff ff 	vcvtsi2sdq -0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a f0    	vcvtsi2ss %eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 10 2a f0    	vcvtsi2ss %eax,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 50 2a f0    	vcvtsi2ss %eax,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 30 2a f0    	vcvtsi2ss %eax,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 70 2a f0    	vcvtsi2ss %eax,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a f5    	vcvtsi2ss %ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 10 2a f5    	vcvtsi2ss %ebp,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 50 2a f5    	vcvtsi2ss %ebp,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 30 2a f5    	vcvtsi2ss %ebp,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 70 2a f5    	vcvtsi2ss %ebp,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 40 2a f5    	vcvtsi2ss %r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 10 2a f5    	vcvtsi2ss %r13d,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 50 2a f5    	vcvtsi2ss %r13d,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 30 2a f5    	vcvtsi2ss %r13d,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 70 2a f5    	vcvtsi2ss %r13d,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a 31    	vcvtsi2ssl \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 16 40 2a b4 f0 34 12 00 00 	vcvtsi2ssl 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a 72 7f 	vcvtsi2ssl 0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a b2 00 02 00 00 	vcvtsi2ssl 0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a 72 80 	vcvtsi2ssl -0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 2a b2 fc fd ff ff 	vcvtsi2ssl -0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 2a f0    	vcvtsi2ss %rax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 10 2a f0    	vcvtsi2ss %rax,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 50 2a f0    	vcvtsi2ss %rax,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 30 2a f0    	vcvtsi2ss %rax,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 70 2a f0    	vcvtsi2ss %rax,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 40 2a f0    	vcvtsi2ss %r8,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 10 2a f0    	vcvtsi2ss %r8,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 50 2a f0    	vcvtsi2ss %r8,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 30 2a f0    	vcvtsi2ss %r8,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 70 2a f0    	vcvtsi2ss %r8,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 2a 31    	vcvtsi2ssq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 96 40 2a b4 f0 34 12 00 00 	vcvtsi2ssq 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 2a 72 7f 	vcvtsi2ssq 0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 2a b2 00 04 00 00 	vcvtsi2ssq 0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 2a 72 80 	vcvtsi2ssq -0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 2a b2 f8 fb ff ff 	vcvtsi2ssq -0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 01 16 47 5a f4    	vcvtss2sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 5a f4    	vcvtss2sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 5a f4    	vcvtss2sd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5a 31    	vcvtss2sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 5a b4 f0 34 12 00 00 	vcvtss2sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5a 72 7f 	vcvtss2sd 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5a b2 00 02 00 00 	vcvtss2sd 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5a 72 80 	vcvtss2sd -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5a b2 fc fd ff ff 	vcvtss2sd -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 91 7e 18 2d c6    	vcvtss2si \{rn-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 58 2d c6    	vcvtss2si \{ru-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 38 2d c6    	vcvtss2si \{rd-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 78 2d c6    	vcvtss2si \{rz-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 18 2d ee    	vcvtss2si \{rn-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 58 2d ee    	vcvtss2si \{ru-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 38 2d ee    	vcvtss2si \{rd-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 78 2d ee    	vcvtss2si \{rz-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 11 7e 18 2d ee    	vcvtss2si \{rn-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 58 2d ee    	vcvtss2si \{ru-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 38 2d ee    	vcvtss2si \{rd-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 78 2d ee    	vcvtss2si \{rz-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 91 fe 18 2d c6    	vcvtss2si \{rn-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 58 2d c6    	vcvtss2si \{ru-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 38 2d c6    	vcvtss2si \{rd-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 78 2d c6    	vcvtss2si \{rz-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 11 fe 18 2d c6    	vcvtss2si \{rn-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 58 2d c6    	vcvtss2si \{ru-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 38 2d c6    	vcvtss2si \{rd-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 78 2d c6    	vcvtss2si \{rz-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 91 7f 18 2c c6    	vcvttsd2si \{sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 18 2c ee    	vcvttsd2si \{sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 11 7f 18 2c ee    	vcvttsd2si \{sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 91 ff 18 2c c6    	vcvttsd2si \{sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 11 ff 18 2c c6    	vcvttsd2si \{sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 91 7e 18 2c c6    	vcvttss2si \{sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 18 2c ee    	vcvttss2si \{sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 11 7e 18 2c ee    	vcvttss2si \{sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 91 fe 18 2c c6    	vcvttss2si \{sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 11 fe 18 2c c6    	vcvttss2si \{sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 01 97 47 5e f4    	vdivsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 5e f4    	vdivsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 5e f4    	vdivsd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 57 5e f4    	vdivsd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 37 5e f4    	vdivsd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 77 5e f4    	vdivsd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5e 31    	vdivsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 5e b4 f0 34 12 00 00 	vdivsd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5e 72 7f 	vdivsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5e b2 00 04 00 00 	vdivsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5e 72 80 	vdivsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5e b2 f8 fb ff ff 	vdivsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 5e f4    	vdivss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 5e f4    	vdivss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 5e f4    	vdivss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 57 5e f4    	vdivss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 37 5e f4    	vdivss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 77 5e f4    	vdivss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5e 31    	vdivss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 5e b4 f0 34 12 00 00 	vdivss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5e 72 7f 	vdivss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5e b2 00 02 00 00 	vdivss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5e 72 80 	vdivss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5e b2 fc fd ff ff 	vdivss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 99 f4    	vfmadd132sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 99 f4    	vfmadd132sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 99 f4    	vfmadd132sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 99 f4    	vfmadd132sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 99 f4    	vfmadd132sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 99 f4    	vfmadd132sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 99 31    	vfmadd132sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 99 b4 f0 34 12 00 00 	vfmadd132sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 99 72 7f 	vfmadd132sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 99 b2 00 04 00 00 	vfmadd132sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 99 72 80 	vfmadd132sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 99 b2 f8 fb ff ff 	vfmadd132sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 99 f4    	vfmadd132ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 99 f4    	vfmadd132ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 99 f4    	vfmadd132ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 99 f4    	vfmadd132ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 99 f4    	vfmadd132ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 99 f4    	vfmadd132ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 99 31    	vfmadd132ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 99 b4 f0 34 12 00 00 	vfmadd132ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 99 72 7f 	vfmadd132ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 99 b2 00 02 00 00 	vfmadd132ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 99 72 80 	vfmadd132ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 99 b2 fc fd ff ff 	vfmadd132ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 a9 f4    	vfmadd213sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 a9 f4    	vfmadd213sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 a9 f4    	vfmadd213sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 a9 f4    	vfmadd213sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 a9 f4    	vfmadd213sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 a9 f4    	vfmadd213sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 a9 31    	vfmadd213sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 a9 b4 f0 34 12 00 00 	vfmadd213sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 a9 72 7f 	vfmadd213sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 a9 b2 00 04 00 00 	vfmadd213sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 a9 72 80 	vfmadd213sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 a9 b2 f8 fb ff ff 	vfmadd213sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 a9 f4    	vfmadd213ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 a9 f4    	vfmadd213ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 a9 f4    	vfmadd213ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 a9 f4    	vfmadd213ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 a9 f4    	vfmadd213ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 a9 f4    	vfmadd213ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 a9 31    	vfmadd213ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 a9 b4 f0 34 12 00 00 	vfmadd213ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 a9 72 7f 	vfmadd213ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 a9 b2 00 02 00 00 	vfmadd213ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 a9 72 80 	vfmadd213ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 a9 b2 fc fd ff ff 	vfmadd213ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 b9 f4    	vfmadd231sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 b9 f4    	vfmadd231sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 b9 f4    	vfmadd231sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 b9 f4    	vfmadd231sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 b9 f4    	vfmadd231sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 b9 f4    	vfmadd231sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 b9 31    	vfmadd231sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 b9 b4 f0 34 12 00 00 	vfmadd231sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 b9 72 7f 	vfmadd231sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 b9 b2 00 04 00 00 	vfmadd231sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 b9 72 80 	vfmadd231sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 b9 b2 f8 fb ff ff 	vfmadd231sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 b9 f4    	vfmadd231ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 b9 f4    	vfmadd231ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 b9 f4    	vfmadd231ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 b9 f4    	vfmadd231ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 b9 f4    	vfmadd231ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 b9 f4    	vfmadd231ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 b9 31    	vfmadd231ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 b9 b4 f0 34 12 00 00 	vfmadd231ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 b9 72 7f 	vfmadd231ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 b9 b2 00 02 00 00 	vfmadd231ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 b9 72 80 	vfmadd231ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 b9 b2 fc fd ff ff 	vfmadd231ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 9b f4    	vfmsub132sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 9b f4    	vfmsub132sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 9b f4    	vfmsub132sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 9b f4    	vfmsub132sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 9b f4    	vfmsub132sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 9b f4    	vfmsub132sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9b 31    	vfmsub132sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 9b b4 f0 34 12 00 00 	vfmsub132sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9b 72 7f 	vfmsub132sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9b b2 00 04 00 00 	vfmsub132sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9b 72 80 	vfmsub132sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9b b2 f8 fb ff ff 	vfmsub132sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 9b f4    	vfmsub132ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 9b f4    	vfmsub132ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 9b f4    	vfmsub132ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 9b f4    	vfmsub132ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 9b f4    	vfmsub132ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 9b f4    	vfmsub132ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9b 31    	vfmsub132ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 9b b4 f0 34 12 00 00 	vfmsub132ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9b 72 7f 	vfmsub132ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9b b2 00 02 00 00 	vfmsub132ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9b 72 80 	vfmsub132ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9b b2 fc fd ff ff 	vfmsub132ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 ab f4    	vfmsub213sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 ab f4    	vfmsub213sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 ab f4    	vfmsub213sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 ab f4    	vfmsub213sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 ab f4    	vfmsub213sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 ab f4    	vfmsub213sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ab 31    	vfmsub213sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 ab b4 f0 34 12 00 00 	vfmsub213sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ab 72 7f 	vfmsub213sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ab b2 00 04 00 00 	vfmsub213sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ab 72 80 	vfmsub213sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ab b2 f8 fb ff ff 	vfmsub213sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 ab f4    	vfmsub213ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 ab f4    	vfmsub213ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 ab f4    	vfmsub213ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 ab f4    	vfmsub213ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 ab f4    	vfmsub213ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 ab f4    	vfmsub213ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ab 31    	vfmsub213ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 ab b4 f0 34 12 00 00 	vfmsub213ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ab 72 7f 	vfmsub213ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ab b2 00 02 00 00 	vfmsub213ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ab 72 80 	vfmsub213ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ab b2 fc fd ff ff 	vfmsub213ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 bb f4    	vfmsub231sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 bb f4    	vfmsub231sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 bb f4    	vfmsub231sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 bb f4    	vfmsub231sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 bb f4    	vfmsub231sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 bb f4    	vfmsub231sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bb 31    	vfmsub231sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 bb b4 f0 34 12 00 00 	vfmsub231sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bb 72 7f 	vfmsub231sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bb b2 00 04 00 00 	vfmsub231sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bb 72 80 	vfmsub231sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bb b2 f8 fb ff ff 	vfmsub231sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 bb f4    	vfmsub231ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 bb f4    	vfmsub231ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 bb f4    	vfmsub231ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 bb f4    	vfmsub231ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 bb f4    	vfmsub231ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 bb f4    	vfmsub231ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bb 31    	vfmsub231ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 bb b4 f0 34 12 00 00 	vfmsub231ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bb 72 7f 	vfmsub231ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bb b2 00 02 00 00 	vfmsub231ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bb 72 80 	vfmsub231ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bb b2 fc fd ff ff 	vfmsub231ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 9d f4    	vfnmadd132sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 9d f4    	vfnmadd132sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 9d f4    	vfnmadd132sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 9d f4    	vfnmadd132sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 9d f4    	vfnmadd132sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 9d f4    	vfnmadd132sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9d 31    	vfnmadd132sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 9d b4 f0 34 12 00 00 	vfnmadd132sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9d 72 7f 	vfnmadd132sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9d b2 00 04 00 00 	vfnmadd132sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9d 72 80 	vfnmadd132sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9d b2 f8 fb ff ff 	vfnmadd132sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 9d f4    	vfnmadd132ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 9d f4    	vfnmadd132ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 9d f4    	vfnmadd132ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 9d f4    	vfnmadd132ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 9d f4    	vfnmadd132ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 9d f4    	vfnmadd132ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9d 31    	vfnmadd132ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 9d b4 f0 34 12 00 00 	vfnmadd132ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9d 72 7f 	vfnmadd132ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9d b2 00 02 00 00 	vfnmadd132ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9d 72 80 	vfnmadd132ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9d b2 fc fd ff ff 	vfnmadd132ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 ad f4    	vfnmadd213sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 ad f4    	vfnmadd213sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 ad f4    	vfnmadd213sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 ad f4    	vfnmadd213sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 ad f4    	vfnmadd213sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 ad f4    	vfnmadd213sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ad 31    	vfnmadd213sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 ad b4 f0 34 12 00 00 	vfnmadd213sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ad 72 7f 	vfnmadd213sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ad b2 00 04 00 00 	vfnmadd213sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ad 72 80 	vfnmadd213sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 ad b2 f8 fb ff ff 	vfnmadd213sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 ad f4    	vfnmadd213ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 ad f4    	vfnmadd213ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 ad f4    	vfnmadd213ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 ad f4    	vfnmadd213ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 ad f4    	vfnmadd213ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 ad f4    	vfnmadd213ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ad 31    	vfnmadd213ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 ad b4 f0 34 12 00 00 	vfnmadd213ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ad 72 7f 	vfnmadd213ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ad b2 00 02 00 00 	vfnmadd213ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ad 72 80 	vfnmadd213ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 ad b2 fc fd ff ff 	vfnmadd213ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 bd f4    	vfnmadd231sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 bd f4    	vfnmadd231sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 bd f4    	vfnmadd231sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 bd f4    	vfnmadd231sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 bd f4    	vfnmadd231sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 bd f4    	vfnmadd231sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bd 31    	vfnmadd231sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 bd b4 f0 34 12 00 00 	vfnmadd231sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bd 72 7f 	vfnmadd231sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bd b2 00 04 00 00 	vfnmadd231sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bd 72 80 	vfnmadd231sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bd b2 f8 fb ff ff 	vfnmadd231sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 bd f4    	vfnmadd231ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 bd f4    	vfnmadd231ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 bd f4    	vfnmadd231ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 bd f4    	vfnmadd231ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 bd f4    	vfnmadd231ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 bd f4    	vfnmadd231ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bd 31    	vfnmadd231ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 bd b4 f0 34 12 00 00 	vfnmadd231ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bd 72 7f 	vfnmadd231ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bd b2 00 02 00 00 	vfnmadd231ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bd 72 80 	vfnmadd231ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bd b2 fc fd ff ff 	vfnmadd231ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 9f f4    	vfnmsub132sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 9f f4    	vfnmsub132sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 9f f4    	vfnmsub132sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 9f f4    	vfnmsub132sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 9f f4    	vfnmsub132sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 9f f4    	vfnmsub132sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9f 31    	vfnmsub132sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 9f b4 f0 34 12 00 00 	vfnmsub132sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9f 72 7f 	vfnmsub132sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9f b2 00 04 00 00 	vfnmsub132sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9f 72 80 	vfnmsub132sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 9f b2 f8 fb ff ff 	vfnmsub132sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 9f f4    	vfnmsub132ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 9f f4    	vfnmsub132ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 9f f4    	vfnmsub132ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 9f f4    	vfnmsub132ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 9f f4    	vfnmsub132ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 9f f4    	vfnmsub132ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9f 31    	vfnmsub132ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 9f b4 f0 34 12 00 00 	vfnmsub132ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9f 72 7f 	vfnmsub132ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9f b2 00 02 00 00 	vfnmsub132ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9f 72 80 	vfnmsub132ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 9f b2 fc fd ff ff 	vfnmsub132ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 af f4    	vfnmsub213sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 af f4    	vfnmsub213sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 af f4    	vfnmsub213sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 af f4    	vfnmsub213sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 af f4    	vfnmsub213sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 af f4    	vfnmsub213sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 af 31    	vfnmsub213sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 af b4 f0 34 12 00 00 	vfnmsub213sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 af 72 7f 	vfnmsub213sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 af b2 00 04 00 00 	vfnmsub213sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 af 72 80 	vfnmsub213sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 af b2 f8 fb ff ff 	vfnmsub213sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 af f4    	vfnmsub213ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 af f4    	vfnmsub213ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 af f4    	vfnmsub213ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 af f4    	vfnmsub213ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 af f4    	vfnmsub213ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 af f4    	vfnmsub213ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 af 31    	vfnmsub213ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 af b4 f0 34 12 00 00 	vfnmsub213ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 af 72 7f 	vfnmsub213ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 af b2 00 02 00 00 	vfnmsub213ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 af 72 80 	vfnmsub213ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 af b2 fc fd ff ff 	vfnmsub213ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 bf f4    	vfnmsub231sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 bf f4    	vfnmsub231sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 bf f4    	vfnmsub231sd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 bf f4    	vfnmsub231sd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 bf f4    	vfnmsub231sd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 bf f4    	vfnmsub231sd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bf 31    	vfnmsub231sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 bf b4 f0 34 12 00 00 	vfnmsub231sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bf 72 7f 	vfnmsub231sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bf b2 00 04 00 00 	vfnmsub231sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bf 72 80 	vfnmsub231sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 bf b2 f8 fb ff ff 	vfnmsub231sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 bf f4    	vfnmsub231ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 bf f4    	vfnmsub231ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 bf f4    	vfnmsub231ss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 bf f4    	vfnmsub231ss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 bf f4    	vfnmsub231ss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 bf f4    	vfnmsub231ss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bf 31    	vfnmsub231ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 bf b4 f0 34 12 00 00 	vfnmsub231ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bf 72 7f 	vfnmsub231ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bf b2 00 02 00 00 	vfnmsub231ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bf 72 80 	vfnmsub231ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 bf b2 fc fd ff ff 	vfnmsub231ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 43 f4    	vgetexpsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 43 f4    	vgetexpsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 43 f4    	vgetexpsd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 43 31    	vgetexpsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 43 b4 f0 34 12 00 00 	vgetexpsd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 43 72 7f 	vgetexpsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 43 b2 00 04 00 00 	vgetexpsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 43 72 80 	vgetexpsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 43 b2 f8 fb ff ff 	vgetexpsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 43 f4    	vgetexpss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 43 f4    	vgetexpss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 43 f4    	vgetexpss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 43 31    	vgetexpss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 43 b4 f0 34 12 00 00 	vgetexpss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 43 72 7f 	vgetexpss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 43 b2 00 02 00 00 	vgetexpss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 43 72 80 	vgetexpss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 43 b2 fc fd ff ff 	vgetexpss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 47 27 f4 ab 	vgetmantsd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 c7 27 f4 ab 	vgetmantsd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 03 95 17 27 f4 ab 	vgetmantsd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 47 27 f4 7b 	vgetmantsd \$0x7b,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 17 27 f4 7b 	vgetmantsd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 27 31 7b 	vgetmantsd \$0x7b,\(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 23 95 47 27 b4 f0 34 12 00 00 7b 	vgetmantsd \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 27 72 7f 7b 	vgetmantsd \$0x7b,0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 27 b2 00 04 00 00 7b 	vgetmantsd \$0x7b,0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 27 72 80 7b 	vgetmantsd \$0x7b,-0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 27 b2 f8 fb ff ff 7b 	vgetmantsd \$0x7b,-0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 47 27 f4 ab 	vgetmantss \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 c7 27 f4 ab 	vgetmantss \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 03 15 17 27 f4 ab 	vgetmantss \$0xab,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 47 27 f4 7b 	vgetmantss \$0x7b,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 17 27 f4 7b 	vgetmantss \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 27 31 7b 	vgetmantss \$0x7b,\(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 23 15 47 27 b4 f0 34 12 00 00 7b 	vgetmantss \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 27 72 7f 7b 	vgetmantss \$0x7b,0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 27 b2 00 02 00 00 7b 	vgetmantss \$0x7b,0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 27 72 80 7b 	vgetmantss \$0x7b,-0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 27 b2 fc fd ff ff 7b 	vgetmantss \$0x7b,-0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 47 5f f4    	vmaxsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 5f f4    	vmaxsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 5f f4    	vmaxsd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5f 31    	vmaxsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 5f b4 f0 34 12 00 00 	vmaxsd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5f 72 7f 	vmaxsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5f b2 00 04 00 00 	vmaxsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5f 72 80 	vmaxsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5f b2 f8 fb ff ff 	vmaxsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 5f f4    	vmaxss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 5f f4    	vmaxss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 5f f4    	vmaxss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5f 31    	vmaxss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 5f b4 f0 34 12 00 00 	vmaxss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5f 72 7f 	vmaxss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5f b2 00 02 00 00 	vmaxss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5f 72 80 	vmaxss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5f b2 fc fd ff ff 	vmaxss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 47 5d f4    	vminsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 5d f4    	vminsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 5d f4    	vminsd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5d 31    	vminsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 5d b4 f0 34 12 00 00 	vminsd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5d 72 7f 	vminsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5d b2 00 04 00 00 	vminsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5d 72 80 	vminsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5d b2 f8 fb ff ff 	vminsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 5d f4    	vminss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 5d f4    	vminss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 5d f4    	vminss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5d 31    	vminss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 5d b4 f0 34 12 00 00 	vminss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5d 72 7f 	vminss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5d b2 00 02 00 00 	vminss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5d 72 80 	vminss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5d b2 fc fd ff ff 	vminss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 31    	vmovsd \(%rcx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff cf 10 31    	vmovsd \(%rcx\),%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 21 ff 4f 10 b4 f0 34 12 00 00 	vmovsd 0x1234\(%rax,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 72 7f 	vmovsd 0x3f8\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 b2 00 04 00 00 	vmovsd 0x400\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 72 80 	vmovsd -0x400\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 10 b2 f8 fb ff ff 	vmovsd -0x408\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 31    	vmovsd %xmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 21 ff 4f 11 b4 f0 34 12 00 00 	vmovsd %xmm30,0x1234\(%rax,%r14,8\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 72 7f 	vmovsd %xmm30,0x3f8\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 b2 00 04 00 00 	vmovsd %xmm30,0x400\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 72 80 	vmovsd %xmm30,-0x400\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 ff 4f 11 b2 f8 fb ff ff 	vmovsd %xmm30,-0x408\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 47 10 f4    	vmovsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 10 f4    	vmovsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 31    	vmovss \(%rcx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e cf 10 31    	vmovss \(%rcx\),%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 21 7e 4f 10 b4 f0 34 12 00 00 	vmovss 0x1234\(%rax,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 72 7f 	vmovss 0x1fc\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 b2 00 02 00 00 	vmovss 0x200\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 72 80 	vmovss -0x200\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 10 b2 fc fd ff ff 	vmovss -0x204\(%rdx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 31    	vmovss %xmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 21 7e 4f 11 b4 f0 34 12 00 00 	vmovss %xmm30,0x1234\(%rax,%r14,8\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 72 7f 	vmovss %xmm30,0x1fc\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 b2 00 02 00 00 	vmovss %xmm30,0x200\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 72 80 	vmovss %xmm30,-0x200\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 61 7e 4f 11 b2 fc fd ff ff 	vmovss %xmm30,-0x204\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 10 f4    	vmovss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 10 f4    	vmovss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 47 59 f4    	vmulsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 59 f4    	vmulsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 59 f4    	vmulsd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 57 59 f4    	vmulsd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 37 59 f4    	vmulsd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 77 59 f4    	vmulsd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 59 31    	vmulsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 59 b4 f0 34 12 00 00 	vmulsd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 59 72 7f 	vmulsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 59 b2 00 04 00 00 	vmulsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 59 72 80 	vmulsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 59 b2 f8 fb ff ff 	vmulsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 59 f4    	vmulss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 59 f4    	vmulss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 59 f4    	vmulss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 57 59 f4    	vmulss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 37 59 f4    	vmulss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 77 59 f4    	vmulss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 59 31    	vmulss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 59 b4 f0 34 12 00 00 	vmulss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 59 72 7f 	vmulss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 59 b2 00 02 00 00 	vmulss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 59 72 80 	vmulss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 59 b2 fc fd ff ff 	vmulss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 4d f4    	vrcp14sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 4d f4    	vrcp14sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 95 47 4d 31    	vrcp14sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 4d b4 f0 34 12 00 00 	vrcp14sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4d 72 7f 	vrcp14sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4d b2 00 04 00 00 	vrcp14sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4d 72 80 	vrcp14sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4d b2 f8 fb ff ff 	vrcp14sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 4d f4    	vrcp14ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 4d f4    	vrcp14ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 15 47 4d 31    	vrcp14ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 4d b4 f0 34 12 00 00 	vrcp14ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4d 72 7f 	vrcp14ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4d b2 00 02 00 00 	vrcp14ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4d 72 80 	vrcp14ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4d b2 fc fd ff ff 	vrcp14ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 cb f4    	vrcp28ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 cb f4    	vrcp28ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 cb f4    	vrcp28ss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cb 31    	vrcp28ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 cb b4 f0 34 12 00 00 	vrcp28ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cb 72 7f 	vrcp28ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cb b2 00 02 00 00 	vrcp28ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cb 72 80 	vrcp28ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cb b2 fc fd ff ff 	vrcp28ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 cb f4    	vrcp28sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 cb f4    	vrcp28sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 cb f4    	vrcp28sd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cb 31    	vrcp28sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 cb b4 f0 34 12 00 00 	vrcp28sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cb 72 7f 	vrcp28sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cb b2 00 04 00 00 	vrcp28sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cb 72 80 	vrcp28sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cb b2 f8 fb ff ff 	vrcp28sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 4f f4    	vrsqrt14sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 4f f4    	vrsqrt14sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 95 47 4f 31    	vrsqrt14sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 4f b4 f0 34 12 00 00 	vrsqrt14sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4f 72 7f 	vrsqrt14sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4f b2 00 04 00 00 	vrsqrt14sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4f 72 80 	vrsqrt14sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 4f b2 f8 fb ff ff 	vrsqrt14sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 4f f4    	vrsqrt14ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 4f f4    	vrsqrt14ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 15 47 4f 31    	vrsqrt14ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 4f b4 f0 34 12 00 00 	vrsqrt14ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4f 72 7f 	vrsqrt14ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4f b2 00 02 00 00 	vrsqrt14ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4f 72 80 	vrsqrt14ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 4f b2 fc fd ff ff 	vrsqrt14ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 cd f4    	vrsqrt28ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 cd f4    	vrsqrt28ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 cd f4    	vrsqrt28ss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cd 31    	vrsqrt28ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 cd b4 f0 34 12 00 00 	vrsqrt28ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cd 72 7f 	vrsqrt28ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cd b2 00 02 00 00 	vrsqrt28ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cd 72 80 	vrsqrt28ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 cd b2 fc fd ff ff 	vrsqrt28ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 47 cd f4    	vrsqrt28sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 cd f4    	vrsqrt28sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 cd f4    	vrsqrt28sd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cd 31    	vrsqrt28sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 cd b4 f0 34 12 00 00 	vrsqrt28sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cd 72 7f 	vrsqrt28sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cd b2 00 04 00 00 	vrsqrt28sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cd 72 80 	vrsqrt28sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 cd b2 f8 fb ff ff 	vrsqrt28sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 47 51 f4    	vsqrtsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 51 f4    	vsqrtsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 51 f4    	vsqrtsd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 57 51 f4    	vsqrtsd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 37 51 f4    	vsqrtsd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 77 51 f4    	vsqrtsd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 51 31    	vsqrtsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 51 b4 f0 34 12 00 00 	vsqrtsd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 51 72 7f 	vsqrtsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 51 b2 00 04 00 00 	vsqrtsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 51 72 80 	vsqrtsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 51 b2 f8 fb ff ff 	vsqrtsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 51 f4    	vsqrtss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 51 f4    	vsqrtss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 51 f4    	vsqrtss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 57 51 f4    	vsqrtss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 37 51 f4    	vsqrtss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 77 51 f4    	vsqrtss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 51 31    	vsqrtss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 51 b4 f0 34 12 00 00 	vsqrtss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 51 72 7f 	vsqrtss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 51 b2 00 02 00 00 	vsqrtss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 51 72 80 	vsqrtss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 51 b2 fc fd ff ff 	vsqrtss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 47 5c f4    	vsubsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 c7 5c f4    	vsubsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 97 17 5c f4    	vsubsd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 57 5c f4    	vsubsd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 37 5c f4    	vsubsd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 97 77 5c f4    	vsubsd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5c 31    	vsubsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 97 47 5c b4 f0 34 12 00 00 	vsubsd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5c 72 7f 	vsubsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5c b2 00 04 00 00 	vsubsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5c 72 80 	vsubsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 97 47 5c b2 f8 fb ff ff 	vsubsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 47 5c f4    	vsubss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 c7 5c f4    	vsubss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 01 16 17 5c f4    	vsubss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 57 5c f4    	vsubss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 37 5c f4    	vsubss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 16 77 5c f4    	vsubss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5c 31    	vsubss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 21 16 47 5c b4 f0 34 12 00 00 	vsubss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5c 72 7f 	vsubss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5c b2 00 02 00 00 	vsubss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5c 72 80 	vsubss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 61 16 47 5c b2 fc fd ff ff 	vsubss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 01 fd 48 2e f5    	vucomisd %xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 01 fd 18 2e f5    	vucomisd \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2e 31    	vucomisd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:	62 21 fd 48 2e b4 f0 34 12 00 00 	vucomisd 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2e 72 7f 	vucomisd 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2e b2 00 04 00 00 	vucomisd 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2e 72 80 	vucomisd -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 fd 48 2e b2 f8 fb ff ff 	vucomisd -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 01 7c 48 2e f5    	vucomiss %xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 01 7c 18 2e f5    	vucomiss \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2e 31    	vucomiss \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:	62 21 7c 48 2e b4 f0 34 12 00 00 	vucomiss 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2e 72 7f 	vucomiss 0x1fc\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2e b2 00 02 00 00 	vucomiss 0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2e 72 80 	vucomiss -0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 61 7c 48 2e b2 fc fd ff ff 	vucomiss -0x204\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:	62 91 7f 48 79 c6    	vcvtsd2usi %xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 18 79 c6    	vcvtsd2usi \{rn-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 58 79 c6    	vcvtsd2usi \{ru-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 38 79 c6    	vcvtsd2usi \{rd-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7f 78 79 c6    	vcvtsd2usi \{rz-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 01    	vcvtsd2usi \(%rcx\),%eax
[ 	]*[a-f0-9]+:	62 b1 7f 48 79 84 f0 34 12 00 00 	vcvtsd2usi 0x1234\(%rax,%r14,8\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 42 7f 	vcvtsd2usi 0x3f8\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 82 00 04 00 00 	vcvtsd2usi 0x400\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 42 80 	vcvtsd2usi -0x400\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 82 f8 fb ff ff 	vcvtsd2usi -0x408\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 91 7f 48 79 ee    	vcvtsd2usi %xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 18 79 ee    	vcvtsd2usi \{rn-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 58 79 ee    	vcvtsd2usi \{ru-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 38 79 ee    	vcvtsd2usi \{rd-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7f 78 79 ee    	vcvtsd2usi \{rz-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 29    	vcvtsd2usi \(%rcx\),%ebp
[ 	]*[a-f0-9]+:	62 b1 7f 48 79 ac f0 34 12 00 00 	vcvtsd2usi 0x1234\(%rax,%r14,8\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 6a 7f 	vcvtsd2usi 0x3f8\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 aa 00 04 00 00 	vcvtsd2usi 0x400\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 6a 80 	vcvtsd2usi -0x400\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 aa f8 fb ff ff 	vcvtsd2usi -0x408\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 11 7f 48 79 ee    	vcvtsd2usi %xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 18 79 ee    	vcvtsd2usi \{rn-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 58 79 ee    	vcvtsd2usi \{ru-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 38 79 ee    	vcvtsd2usi \{rd-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7f 78 79 ee    	vcvtsd2usi \{rz-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 71 7f 48 79 29    	vcvtsd2usi \(%rcx\),%r13d
[ 	]*[a-f0-9]+:	62 31 7f 48 79 ac f0 34 12 00 00 	vcvtsd2usi 0x1234\(%rax,%r14,8\),%r13d
[ 	]*[a-f0-9]+:	62 71 7f 48 79 6a 7f 	vcvtsd2usi 0x3f8\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 71 7f 48 79 aa 00 04 00 00 	vcvtsd2usi 0x400\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 71 7f 48 79 6a 80 	vcvtsd2usi -0x400\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 71 7f 48 79 aa f8 fb ff ff 	vcvtsd2usi -0x408\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 91 ff 48 79 c6    	vcvtsd2usi %xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 18 79 c6    	vcvtsd2usi \{rn-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 58 79 c6    	vcvtsd2usi \{ru-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 38 79 c6    	vcvtsd2usi \{rd-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 ff 78 79 c6    	vcvtsd2usi \{rz-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 01    	vcvtsd2usi \(%rcx\),%rax
[ 	]*[a-f0-9]+:	62 b1 ff 48 79 84 f0 34 12 00 00 	vcvtsd2usi 0x1234\(%rax,%r14,8\),%rax
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 42 7f 	vcvtsd2usi 0x3f8\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 82 00 04 00 00 	vcvtsd2usi 0x400\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 42 80 	vcvtsd2usi -0x400\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 f1 ff 48 79 82 f8 fb ff ff 	vcvtsd2usi -0x408\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 11 ff 48 79 c6    	vcvtsd2usi %xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 18 79 c6    	vcvtsd2usi \{rn-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 58 79 c6    	vcvtsd2usi \{ru-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 38 79 c6    	vcvtsd2usi \{rd-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 ff 78 79 c6    	vcvtsd2usi \{rz-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 71 ff 48 79 01    	vcvtsd2usi \(%rcx\),%r8
[ 	]*[a-f0-9]+:	62 31 ff 48 79 84 f0 34 12 00 00 	vcvtsd2usi 0x1234\(%rax,%r14,8\),%r8
[ 	]*[a-f0-9]+:	62 71 ff 48 79 42 7f 	vcvtsd2usi 0x3f8\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 71 ff 48 79 82 00 04 00 00 	vcvtsd2usi 0x400\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 71 ff 48 79 42 80 	vcvtsd2usi -0x400\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 71 ff 48 79 82 f8 fb ff ff 	vcvtsd2usi -0x408\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 91 7e 48 79 c6    	vcvtss2usi %xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 18 79 c6    	vcvtss2usi \{rn-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 58 79 c6    	vcvtss2usi \{ru-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 38 79 c6    	vcvtss2usi \{rd-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 91 7e 78 79 c6    	vcvtss2usi \{rz-sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 01    	vcvtss2usi \(%rcx\),%eax
[ 	]*[a-f0-9]+:	62 b1 7e 48 79 84 f0 34 12 00 00 	vcvtss2usi 0x1234\(%rax,%r14,8\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 42 7f 	vcvtss2usi 0x1fc\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 82 00 02 00 00 	vcvtss2usi 0x200\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 42 80 	vcvtss2usi -0x200\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 82 fc fd ff ff 	vcvtss2usi -0x204\(%rdx\),%eax
[ 	]*[a-f0-9]+:	62 91 7e 48 79 ee    	vcvtss2usi %xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 18 79 ee    	vcvtss2usi \{rn-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 58 79 ee    	vcvtss2usi \{ru-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 38 79 ee    	vcvtss2usi \{rd-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 91 7e 78 79 ee    	vcvtss2usi \{rz-sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 29    	vcvtss2usi \(%rcx\),%ebp
[ 	]*[a-f0-9]+:	62 b1 7e 48 79 ac f0 34 12 00 00 	vcvtss2usi 0x1234\(%rax,%r14,8\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 6a 7f 	vcvtss2usi 0x1fc\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 aa 00 02 00 00 	vcvtss2usi 0x200\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 6a 80 	vcvtss2usi -0x200\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 aa fc fd ff ff 	vcvtss2usi -0x204\(%rdx\),%ebp
[ 	]*[a-f0-9]+:	62 11 7e 48 79 ee    	vcvtss2usi %xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 18 79 ee    	vcvtss2usi \{rn-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 58 79 ee    	vcvtss2usi \{ru-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 38 79 ee    	vcvtss2usi \{rd-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 11 7e 78 79 ee    	vcvtss2usi \{rz-sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:	62 71 7e 48 79 29    	vcvtss2usi \(%rcx\),%r13d
[ 	]*[a-f0-9]+:	62 31 7e 48 79 ac f0 34 12 00 00 	vcvtss2usi 0x1234\(%rax,%r14,8\),%r13d
[ 	]*[a-f0-9]+:	62 71 7e 48 79 6a 7f 	vcvtss2usi 0x1fc\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 71 7e 48 79 aa 00 02 00 00 	vcvtss2usi 0x200\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 71 7e 48 79 6a 80 	vcvtss2usi -0x200\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 71 7e 48 79 aa fc fd ff ff 	vcvtss2usi -0x204\(%rdx\),%r13d
[ 	]*[a-f0-9]+:	62 91 fe 48 79 c6    	vcvtss2usi %xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 18 79 c6    	vcvtss2usi \{rn-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 58 79 c6    	vcvtss2usi \{ru-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 38 79 c6    	vcvtss2usi \{rd-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 91 fe 78 79 c6    	vcvtss2usi \{rz-sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 01    	vcvtss2usi \(%rcx\),%rax
[ 	]*[a-f0-9]+:	62 b1 fe 48 79 84 f0 34 12 00 00 	vcvtss2usi 0x1234\(%rax,%r14,8\),%rax
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 42 7f 	vcvtss2usi 0x1fc\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 82 00 02 00 00 	vcvtss2usi 0x200\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 42 80 	vcvtss2usi -0x200\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 f1 fe 48 79 82 fc fd ff ff 	vcvtss2usi -0x204\(%rdx\),%rax
[ 	]*[a-f0-9]+:	62 11 fe 48 79 c6    	vcvtss2usi %xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 18 79 c6    	vcvtss2usi \{rn-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 58 79 c6    	vcvtss2usi \{ru-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 38 79 c6    	vcvtss2usi \{rd-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 11 fe 78 79 c6    	vcvtss2usi \{rz-sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:	62 71 fe 48 79 01    	vcvtss2usi \(%rcx\),%r8
[ 	]*[a-f0-9]+:	62 31 fe 48 79 84 f0 34 12 00 00 	vcvtss2usi 0x1234\(%rax,%r14,8\),%r8
[ 	]*[a-f0-9]+:	62 71 fe 48 79 42 7f 	vcvtss2usi 0x1fc\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 71 fe 48 79 82 00 02 00 00 	vcvtss2usi 0x200\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 71 fe 48 79 42 80 	vcvtss2usi -0x200\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 71 fe 48 79 82 fc fd ff ff 	vcvtss2usi -0x204\(%rdx\),%r8
[ 	]*[a-f0-9]+:	62 61 17 40 7b f0    	vcvtusi2sd %eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 7b f5    	vcvtusi2sd %ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 17 40 7b f5    	vcvtusi2sd %r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 7b 31    	vcvtusi2sdl \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 17 40 7b b4 f0 34 12 00 00 	vcvtusi2sdl 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 7b 72 7f 	vcvtusi2sdl 0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 7b b2 00 02 00 00 	vcvtusi2sdl 0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 7b 72 80 	vcvtusi2sdl -0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 17 40 7b b2 fc fd ff ff 	vcvtusi2sdl -0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 7b f0    	vcvtusi2sd %rax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 10 7b f0    	vcvtusi2sd %rax,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 50 7b f0    	vcvtusi2sd %rax,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 30 7b f0    	vcvtusi2sd %rax,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 70 7b f0    	vcvtusi2sd %rax,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 40 7b f0    	vcvtusi2sd %r8,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 10 7b f0    	vcvtusi2sd %r8,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 50 7b f0    	vcvtusi2sd %r8,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 30 7b f0    	vcvtusi2sd %r8,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 97 70 7b f0    	vcvtusi2sd %r8,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 7b 31    	vcvtusi2sdq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 97 40 7b b4 f0 34 12 00 00 	vcvtusi2sdq 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 7b 72 7f 	vcvtusi2sdq 0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 7b b2 00 04 00 00 	vcvtusi2sdq 0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 7b 72 80 	vcvtusi2sdq -0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 97 40 7b b2 f8 fb ff ff 	vcvtusi2sdq -0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b f0    	vcvtusi2ss %eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 10 7b f0    	vcvtusi2ss %eax,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 50 7b f0    	vcvtusi2ss %eax,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 30 7b f0    	vcvtusi2ss %eax,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 70 7b f0    	vcvtusi2ss %eax,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b f5    	vcvtusi2ss %ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 10 7b f5    	vcvtusi2ss %ebp,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 50 7b f5    	vcvtusi2ss %ebp,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 30 7b f5    	vcvtusi2ss %ebp,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 70 7b f5    	vcvtusi2ss %ebp,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 40 7b f5    	vcvtusi2ss %r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 10 7b f5    	vcvtusi2ss %r13d,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 50 7b f5    	vcvtusi2ss %r13d,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 30 7b f5    	vcvtusi2ss %r13d,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 16 70 7b f5    	vcvtusi2ss %r13d,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b 31    	vcvtusi2ssl \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 16 40 7b b4 f0 34 12 00 00 	vcvtusi2ssl 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b 72 7f 	vcvtusi2ssl 0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b b2 00 02 00 00 	vcvtusi2ssl 0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b 72 80 	vcvtusi2ssl -0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 16 40 7b b2 fc fd ff ff 	vcvtusi2ssl -0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 7b f0    	vcvtusi2ss %rax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 10 7b f0    	vcvtusi2ss %rax,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 50 7b f0    	vcvtusi2ss %rax,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 30 7b f0    	vcvtusi2ss %rax,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 70 7b f0    	vcvtusi2ss %rax,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 40 7b f0    	vcvtusi2ss %r8,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 10 7b f0    	vcvtusi2ss %r8,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 50 7b f0    	vcvtusi2ss %r8,\{ru-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 30 7b f0    	vcvtusi2ss %r8,\{rd-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 41 96 70 7b f0    	vcvtusi2ss %r8,\{rz-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 7b 31    	vcvtusi2ssq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 21 96 40 7b b4 f0 34 12 00 00 	vcvtusi2ssq 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 7b 72 7f 	vcvtusi2ssq 0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 7b b2 00 04 00 00 	vcvtusi2ssq 0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 7b 72 80 	vcvtusi2ssq -0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 61 96 40 7b b2 f8 fb ff ff 	vcvtusi2ssq -0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 02 95 47 2d f4    	vscalefsd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 c7 2d f4    	vscalefsd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 2d f4    	vscalefsd \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 57 2d f4    	vscalefsd \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 37 2d f4    	vscalefsd \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 77 2d f4    	vscalefsd \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 2d 31    	vscalefsd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 47 2d b4 f0 34 12 00 00 	vscalefsd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 2d 72 7f 	vscalefsd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 2d b2 00 04 00 00 	vscalefsd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 2d 72 80 	vscalefsd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 47 2d b2 f8 fb ff ff 	vscalefsd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 47 2d f4    	vscalefss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 c7 2d f4    	vscalefss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 2d f4    	vscalefss \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 57 2d f4    	vscalefss \{ru-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 37 2d f4    	vscalefss \{rd-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 77 2d f4    	vscalefss \{rz-sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 2d 31    	vscalefss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 47 2d b4 f0 34 12 00 00 	vscalefss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 2d 72 7f 	vscalefss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 2d b2 00 02 00 00 	vscalefss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 2d 72 80 	vscalefss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 47 2d b2 fc fd ff ff 	vscalefss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 47 55 f4 ab 	vfixupimmss \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 c7 55 f4 ab 	vfixupimmss \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 03 15 17 55 f4 ab 	vfixupimmss \$0xab,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 47 55 f4 7b 	vfixupimmss \$0x7b,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 17 55 f4 7b 	vfixupimmss \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 55 31 7b 	vfixupimmss \$0x7b,\(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 23 15 47 55 b4 f0 34 12 00 00 7b 	vfixupimmss \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 55 72 7f 7b 	vfixupimmss \$0x7b,0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 55 b2 00 02 00 00 7b 	vfixupimmss \$0x7b,0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 55 72 80 7b 	vfixupimmss \$0x7b,-0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 55 b2 fc fd ff ff 7b 	vfixupimmss \$0x7b,-0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 47 55 f4 ab 	vfixupimmsd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 c7 55 f4 ab 	vfixupimmsd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 03 95 17 55 f4 ab 	vfixupimmsd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 47 55 f4 7b 	vfixupimmsd \$0x7b,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 17 55 f4 7b 	vfixupimmsd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 55 31 7b 	vfixupimmsd \$0x7b,\(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 23 95 47 55 b4 f0 34 12 00 00 7b 	vfixupimmsd \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 55 72 7f 7b 	vfixupimmsd \$0x7b,0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 55 b2 00 04 00 00 7b 	vfixupimmsd \$0x7b,0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 55 72 80 7b 	vfixupimmsd \$0x7b,-0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 55 b2 f8 fb ff ff 7b 	vfixupimmsd \$0x7b,-0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 47 0b f4 ab 	vrndscalesd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 c7 0b f4 ab 	vrndscalesd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 03 95 17 0b f4 ab 	vrndscalesd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 47 0b f4 7b 	vrndscalesd \$0x7b,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 95 17 0b f4 7b 	vrndscalesd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 0b 31 7b 	vrndscalesd \$0x7b,\(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 23 95 47 0b b4 f0 34 12 00 00 7b 	vrndscalesd \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 0b 72 7f 7b 	vrndscalesd \$0x7b,0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 0b b2 00 04 00 00 7b 	vrndscalesd \$0x7b,0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 0b 72 80 7b 	vrndscalesd \$0x7b,-0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 95 47 0b b2 f8 fb ff ff 7b 	vrndscalesd \$0x7b,-0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 47 0a f4 ab 	vrndscaless \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 c7 0a f4 ab 	vrndscaless \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 03 15 17 0a f4 ab 	vrndscaless \$0xab,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 47 0a f4 7b 	vrndscaless \$0x7b,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 03 15 17 0a f4 7b 	vrndscaless \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 0a 31 7b 	vrndscaless \$0x7b,\(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 23 15 47 0a b4 f0 34 12 00 00 7b 	vrndscaless \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 0a 72 7f 7b 	vrndscaless \$0x7b,0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 0a b2 00 02 00 00 7b 	vrndscaless \$0x7b,0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 0a 72 80 7b 	vrndscaless \$0x7b,-0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 15 47 0a b2 fc fd ff ff 7b 	vrndscaless \$0x7b,-0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
#pass
