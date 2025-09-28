#as: -mevexlig=512
#objdump: -dw
#name: i386 AVX512 lig512 insns
#source: evex-lig.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 f4    	vaddsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 58 f4    	vaddsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 58 f4    	vaddsd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 58 f4    	vaddsd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 58 f4    	vaddsd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 58 f4    	vaddsd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 31    	vaddsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 b4 f4 c0 1d fe ff 	vaddsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 72 7f 	vaddsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 b2 00 04 00 00 	vaddsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 72 80 	vaddsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 b2 f8 fb ff ff 	vaddsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 f4    	vaddss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 58 f4    	vaddss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 58 f4    	vaddss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 58 f4    	vaddss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 58 f4    	vaddss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 58 f4    	vaddss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 31    	vaddss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 b4 f4 c0 1d fe ff 	vaddss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 72 7f 	vaddss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 b2 00 02 00 00 	vaddss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 72 80 	vaddss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 b2 fc fd ff ff 	vaddss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec ab 	vcmpsd \$0xab,%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec ab 	vcmpsd \$0xab,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 7b 	vcmpsd \$0x7b,%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 7b 	vcmpsd \$0x7b,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 7b 	vcmpsd \$0x7b,\(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 7b 	vcmpsd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 7b 	vcmpsd \$0x7b,0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 7b 	vcmpsd \$0x7b,0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 7b 	vcmpsd \$0x7b,-0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 7b 	vcmpsd \$0x7b,-0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 00 	vcmpeqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 00 	vcmpeqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 00 	vcmpeqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 00 	vcmpeqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 00 	vcmpeqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 00 	vcmpeqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 00 	vcmpeqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 00 	vcmpeqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 00 	vcmpeqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 00 	vcmpeqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 00 	vcmpeqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 00 	vcmpeqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 00 	vcmpeqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 00 	vcmpeqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 00 	vcmpeqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 00 	vcmpeqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 01 	vcmpltsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 01 	vcmpltsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 01 	vcmpltsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 01 	vcmpltsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 01 	vcmpltsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 01 	vcmpltsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 01 	vcmpltsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 01 	vcmpltsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 01 	vcmpltsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 01 	vcmpltsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 01 	vcmpltsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 01 	vcmpltsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 01 	vcmpltsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 01 	vcmpltsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 01 	vcmpltsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 01 	vcmpltsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 02 	vcmplesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 02 	vcmplesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 02 	vcmplesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 02 	vcmplesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 02 	vcmplesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 02 	vcmplesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 02 	vcmplesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 02 	vcmplesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 02 	vcmplesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 02 	vcmplesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 02 	vcmplesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 02 	vcmplesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 02 	vcmplesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 02 	vcmplesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 02 	vcmplesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 02 	vcmplesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 03 	vcmpunordsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 03 	vcmpunordsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 03 	vcmpunordsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 03 	vcmpunordsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 03 	vcmpunordsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 03 	vcmpunordsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 03 	vcmpunordsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 03 	vcmpunordsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 03 	vcmpunordsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 03 	vcmpunordsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 03 	vcmpunordsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 03 	vcmpunordsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 03 	vcmpunordsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 03 	vcmpunordsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 03 	vcmpunordsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 03 	vcmpunordsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 04 	vcmpneqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 04 	vcmpneqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 04 	vcmpneqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 04 	vcmpneqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 04 	vcmpneqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 04 	vcmpneqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 04 	vcmpneqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 04 	vcmpneqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 04 	vcmpneqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 04 	vcmpneqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 04 	vcmpneqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 04 	vcmpneqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 04 	vcmpneqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 04 	vcmpneqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 04 	vcmpneqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 04 	vcmpneqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 05 	vcmpnltsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 05 	vcmpnltsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 05 	vcmpnltsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 05 	vcmpnltsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 05 	vcmpnltsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 05 	vcmpnltsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 05 	vcmpnltsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 05 	vcmpnltsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 05 	vcmpnltsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 05 	vcmpnltsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 05 	vcmpnltsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 05 	vcmpnltsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 05 	vcmpnltsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 05 	vcmpnltsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 05 	vcmpnltsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 05 	vcmpnltsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 06 	vcmpnlesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 06 	vcmpnlesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 06 	vcmpnlesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 06 	vcmpnlesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 06 	vcmpnlesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 06 	vcmpnlesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 06 	vcmpnlesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 06 	vcmpnlesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 06 	vcmpnlesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 06 	vcmpnlesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 06 	vcmpnlesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 06 	vcmpnlesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 06 	vcmpnlesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 06 	vcmpnlesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 06 	vcmpnlesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 06 	vcmpnlesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 07 	vcmpordsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 07 	vcmpordsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 07 	vcmpordsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 07 	vcmpordsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 07 	vcmpordsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 07 	vcmpordsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 07 	vcmpordsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 07 	vcmpordsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 07 	vcmpordsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 07 	vcmpordsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 07 	vcmpordsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 07 	vcmpordsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 07 	vcmpordsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 07 	vcmpordsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 07 	vcmpordsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 07 	vcmpordsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 08 	vcmpeq_uqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 08 	vcmpeq_uqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 08 	vcmpeq_uqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 08 	vcmpeq_uqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 08 	vcmpeq_uqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 08 	vcmpeq_uqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 08 	vcmpeq_uqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 08 	vcmpeq_uqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 09 	vcmpngesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 09 	vcmpngesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 09 	vcmpngesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 09 	vcmpngesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 09 	vcmpngesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 09 	vcmpngesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 09 	vcmpngesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 09 	vcmpngesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 09 	vcmpngesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 09 	vcmpngesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 09 	vcmpngesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 09 	vcmpngesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 09 	vcmpngesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 09 	vcmpngesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 09 	vcmpngesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 09 	vcmpngesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0a 	vcmpngtsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0a 	vcmpngtsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0a 	vcmpngtsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0a 	vcmpngtsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0a 	vcmpngtsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0a 	vcmpngtsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0a 	vcmpngtsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0a 	vcmpngtsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0a 	vcmpngtsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0a 	vcmpngtsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0a 	vcmpngtsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0a 	vcmpngtsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0a 	vcmpngtsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0a 	vcmpngtsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0a 	vcmpngtsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0a 	vcmpngtsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0b 	vcmpfalsesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0b 	vcmpfalsesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0b 	vcmpfalsesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0b 	vcmpfalsesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0b 	vcmpfalsesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0b 	vcmpfalsesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0b 	vcmpfalsesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0b 	vcmpfalsesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0b 	vcmpfalsesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0b 	vcmpfalsesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0b 	vcmpfalsesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0b 	vcmpfalsesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0b 	vcmpfalsesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0b 	vcmpfalsesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0c 	vcmpneq_oqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0c 	vcmpneq_oqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0c 	vcmpneq_oqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0c 	vcmpneq_oqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0c 	vcmpneq_oqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0c 	vcmpneq_oqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0c 	vcmpneq_oqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0c 	vcmpneq_oqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0d 	vcmpgesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0d 	vcmpgesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0d 	vcmpgesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0d 	vcmpgesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0d 	vcmpgesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0d 	vcmpgesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0d 	vcmpgesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0d 	vcmpgesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0d 	vcmpgesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0d 	vcmpgesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0d 	vcmpgesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0d 	vcmpgesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0d 	vcmpgesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0d 	vcmpgesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0d 	vcmpgesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0d 	vcmpgesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0e 	vcmpgtsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0e 	vcmpgtsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0e 	vcmpgtsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0e 	vcmpgtsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0e 	vcmpgtsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0e 	vcmpgtsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0e 	vcmpgtsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0e 	vcmpgtsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0e 	vcmpgtsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0e 	vcmpgtsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0e 	vcmpgtsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0e 	vcmpgtsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0e 	vcmpgtsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0e 	vcmpgtsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0e 	vcmpgtsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0e 	vcmpgtsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0f 	vcmptruesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0f 	vcmptruesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0f 	vcmptruesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0f 	vcmptruesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0f 	vcmptruesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0f 	vcmptruesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0f 	vcmptruesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0f 	vcmptruesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0f 	vcmptruesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0f 	vcmptruesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0f 	vcmptruesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0f 	vcmptruesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0f 	vcmptruesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0f 	vcmptruesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0f 	vcmptruesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0f 	vcmptruesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 10 	vcmpeq_ossd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 10 	vcmpeq_ossd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 10 	vcmpeq_ossd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 10 	vcmpeq_ossd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 10 	vcmpeq_ossd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 10 	vcmpeq_ossd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 10 	vcmpeq_ossd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 10 	vcmpeq_ossd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 11 	vcmplt_oqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 11 	vcmplt_oqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 11 	vcmplt_oqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 11 	vcmplt_oqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 11 	vcmplt_oqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 11 	vcmplt_oqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 11 	vcmplt_oqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 11 	vcmplt_oqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 12 	vcmple_oqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 12 	vcmple_oqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 12 	vcmple_oqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 12 	vcmple_oqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 12 	vcmple_oqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 12 	vcmple_oqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 12 	vcmple_oqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 12 	vcmple_oqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 13 	vcmpunord_ssd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 13 	vcmpunord_ssd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 13 	vcmpunord_ssd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 13 	vcmpunord_ssd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 13 	vcmpunord_ssd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 13 	vcmpunord_ssd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 13 	vcmpunord_ssd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 13 	vcmpunord_ssd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 14 	vcmpneq_ussd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 14 	vcmpneq_ussd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 14 	vcmpneq_ussd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 14 	vcmpneq_ussd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 14 	vcmpneq_ussd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 14 	vcmpneq_ussd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 14 	vcmpneq_ussd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 14 	vcmpneq_ussd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 15 	vcmpnlt_uqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 15 	vcmpnlt_uqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 15 	vcmpnlt_uqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 15 	vcmpnlt_uqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 15 	vcmpnlt_uqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 15 	vcmpnlt_uqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 15 	vcmpnlt_uqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 15 	vcmpnlt_uqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 16 	vcmpnle_uqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 16 	vcmpnle_uqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 16 	vcmpnle_uqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 16 	vcmpnle_uqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 16 	vcmpnle_uqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 16 	vcmpnle_uqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 16 	vcmpnle_uqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 16 	vcmpnle_uqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 17 	vcmpord_ssd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 17 	vcmpord_ssd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 17 	vcmpord_ssd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 17 	vcmpord_ssd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 17 	vcmpord_ssd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 17 	vcmpord_ssd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 17 	vcmpord_ssd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 17 	vcmpord_ssd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 18 	vcmpeq_ussd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 18 	vcmpeq_ussd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 18 	vcmpeq_ussd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 18 	vcmpeq_ussd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 18 	vcmpeq_ussd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 18 	vcmpeq_ussd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 18 	vcmpeq_ussd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 18 	vcmpeq_ussd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 19 	vcmpnge_uqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 19 	vcmpnge_uqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 19 	vcmpnge_uqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 19 	vcmpnge_uqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 19 	vcmpnge_uqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 19 	vcmpnge_uqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 19 	vcmpnge_uqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 19 	vcmpnge_uqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 1a 	vcmpngt_uqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1a 	vcmpngt_uqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 1a 	vcmpngt_uqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 1a 	vcmpngt_uqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 1a 	vcmpngt_uqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 1a 	vcmpngt_uqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 1a 	vcmpngt_uqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 1a 	vcmpngt_uqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 1b 	vcmpfalse_ossd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1b 	vcmpfalse_ossd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 1b 	vcmpfalse_ossd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 1b 	vcmpfalse_ossd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 1b 	vcmpfalse_ossd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 1b 	vcmpfalse_ossd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 1b 	vcmpfalse_ossd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 1b 	vcmpfalse_ossd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 1c 	vcmpneq_ossd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1c 	vcmpneq_ossd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 1c 	vcmpneq_ossd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 1c 	vcmpneq_ossd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 1c 	vcmpneq_ossd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 1c 	vcmpneq_ossd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 1c 	vcmpneq_ossd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 1c 	vcmpneq_ossd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 1d 	vcmpge_oqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1d 	vcmpge_oqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 1d 	vcmpge_oqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 1d 	vcmpge_oqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 1d 	vcmpge_oqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 1d 	vcmpge_oqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 1d 	vcmpge_oqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 1d 	vcmpge_oqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 1e 	vcmpgt_oqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1e 	vcmpgt_oqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 1e 	vcmpgt_oqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 1e 	vcmpgt_oqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 1e 	vcmpgt_oqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 1e 	vcmpgt_oqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 1e 	vcmpgt_oqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 1e 	vcmpgt_oqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 1f 	vcmptrue_ussd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1f 	vcmptrue_ussd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 1f 	vcmptrue_ussd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 1f 	vcmptrue_ussd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 1f 	vcmptrue_ussd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 1f 	vcmptrue_ussd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 1f 	vcmptrue_ussd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 1f 	vcmptrue_ussd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec ab 	vcmpss \$0xab,%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec ab 	vcmpss \$0xab,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 7b 	vcmpss \$0x7b,%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 7b 	vcmpss \$0x7b,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 7b 	vcmpss \$0x7b,\(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 7b 	vcmpss \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 7b 	vcmpss \$0x7b,0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 7b 	vcmpss \$0x7b,0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 7b 	vcmpss \$0x7b,-0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 7b 	vcmpss \$0x7b,-0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 00 	vcmpeqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 00 	vcmpeqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 00 	vcmpeqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 00 	vcmpeqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 00 	vcmpeqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 00 	vcmpeqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 00 	vcmpeqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 00 	vcmpeqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 00 	vcmpeqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 00 	vcmpeqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 00 	vcmpeqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 00 	vcmpeqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 00 	vcmpeqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 00 	vcmpeqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 00 	vcmpeqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 00 	vcmpeqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 01 	vcmpltss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 01 	vcmpltss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 01 	vcmpltss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 01 	vcmpltss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 01 	vcmpltss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 01 	vcmpltss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 01 	vcmpltss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 01 	vcmpltss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 01 	vcmpltss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 01 	vcmpltss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 01 	vcmpltss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 01 	vcmpltss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 01 	vcmpltss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 01 	vcmpltss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 01 	vcmpltss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 01 	vcmpltss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 02 	vcmpless %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 02 	vcmpless \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 02 	vcmpless \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 02 	vcmpless -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 02 	vcmpless 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 02 	vcmpless 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 02 	vcmpless -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 02 	vcmpless -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 02 	vcmpless %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 02 	vcmpless \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 02 	vcmpless \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 02 	vcmpless -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 02 	vcmpless 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 02 	vcmpless 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 02 	vcmpless -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 02 	vcmpless -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 03 	vcmpunordss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 03 	vcmpunordss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 03 	vcmpunordss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 03 	vcmpunordss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 03 	vcmpunordss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 03 	vcmpunordss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 03 	vcmpunordss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 03 	vcmpunordss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 03 	vcmpunordss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 03 	vcmpunordss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 03 	vcmpunordss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 03 	vcmpunordss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 03 	vcmpunordss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 03 	vcmpunordss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 03 	vcmpunordss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 03 	vcmpunordss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 04 	vcmpneqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 04 	vcmpneqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 04 	vcmpneqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 04 	vcmpneqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 04 	vcmpneqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 04 	vcmpneqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 04 	vcmpneqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 04 	vcmpneqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 04 	vcmpneqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 04 	vcmpneqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 04 	vcmpneqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 04 	vcmpneqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 04 	vcmpneqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 04 	vcmpneqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 04 	vcmpneqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 04 	vcmpneqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 05 	vcmpnltss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 05 	vcmpnltss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 05 	vcmpnltss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 05 	vcmpnltss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 05 	vcmpnltss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 05 	vcmpnltss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 05 	vcmpnltss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 05 	vcmpnltss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 05 	vcmpnltss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 05 	vcmpnltss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 05 	vcmpnltss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 05 	vcmpnltss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 05 	vcmpnltss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 05 	vcmpnltss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 05 	vcmpnltss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 05 	vcmpnltss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 06 	vcmpnless %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 06 	vcmpnless \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 06 	vcmpnless \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 06 	vcmpnless -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 06 	vcmpnless 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 06 	vcmpnless 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 06 	vcmpnless -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 06 	vcmpnless -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 06 	vcmpnless %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 06 	vcmpnless \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 06 	vcmpnless \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 06 	vcmpnless -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 06 	vcmpnless 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 06 	vcmpnless 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 06 	vcmpnless -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 06 	vcmpnless -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 07 	vcmpordss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 07 	vcmpordss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 07 	vcmpordss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 07 	vcmpordss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 07 	vcmpordss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 07 	vcmpordss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 07 	vcmpordss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 07 	vcmpordss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 07 	vcmpordss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 07 	vcmpordss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 07 	vcmpordss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 07 	vcmpordss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 07 	vcmpordss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 07 	vcmpordss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 07 	vcmpordss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 07 	vcmpordss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 08 	vcmpeq_uqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 08 	vcmpeq_uqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 08 	vcmpeq_uqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 08 	vcmpeq_uqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 08 	vcmpeq_uqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 08 	vcmpeq_uqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 08 	vcmpeq_uqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 08 	vcmpeq_uqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 09 	vcmpngess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 09 	vcmpngess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 09 	vcmpngess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 09 	vcmpngess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 09 	vcmpngess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 09 	vcmpngess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 09 	vcmpngess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 09 	vcmpngess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 09 	vcmpngess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 09 	vcmpngess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 09 	vcmpngess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 09 	vcmpngess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 09 	vcmpngess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 09 	vcmpngess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 09 	vcmpngess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 09 	vcmpngess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0a 	vcmpngtss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0a 	vcmpngtss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0a 	vcmpngtss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0a 	vcmpngtss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0a 	vcmpngtss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0a 	vcmpngtss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0a 	vcmpngtss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0a 	vcmpngtss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0a 	vcmpngtss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0a 	vcmpngtss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0a 	vcmpngtss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0a 	vcmpngtss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0a 	vcmpngtss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0a 	vcmpngtss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0a 	vcmpngtss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0a 	vcmpngtss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0b 	vcmpfalsess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0b 	vcmpfalsess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0b 	vcmpfalsess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0b 	vcmpfalsess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0b 	vcmpfalsess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0b 	vcmpfalsess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0b 	vcmpfalsess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0b 	vcmpfalsess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0b 	vcmpfalsess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0b 	vcmpfalsess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0b 	vcmpfalsess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0b 	vcmpfalsess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0b 	vcmpfalsess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0b 	vcmpfalsess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0c 	vcmpneq_oqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0c 	vcmpneq_oqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0c 	vcmpneq_oqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0c 	vcmpneq_oqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0c 	vcmpneq_oqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0c 	vcmpneq_oqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0c 	vcmpneq_oqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0c 	vcmpneq_oqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0d 	vcmpgess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0d 	vcmpgess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0d 	vcmpgess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0d 	vcmpgess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0d 	vcmpgess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0d 	vcmpgess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0d 	vcmpgess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0d 	vcmpgess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0d 	vcmpgess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0d 	vcmpgess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0d 	vcmpgess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0d 	vcmpgess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0d 	vcmpgess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0d 	vcmpgess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0d 	vcmpgess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0d 	vcmpgess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0e 	vcmpgtss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0e 	vcmpgtss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0e 	vcmpgtss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0e 	vcmpgtss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0e 	vcmpgtss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0e 	vcmpgtss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0e 	vcmpgtss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0e 	vcmpgtss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0e 	vcmpgtss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0e 	vcmpgtss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0e 	vcmpgtss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0e 	vcmpgtss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0e 	vcmpgtss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0e 	vcmpgtss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0e 	vcmpgtss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0e 	vcmpgtss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0f 	vcmptruess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0f 	vcmptruess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0f 	vcmptruess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0f 	vcmptruess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0f 	vcmptruess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0f 	vcmptruess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0f 	vcmptruess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0f 	vcmptruess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0f 	vcmptruess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0f 	vcmptruess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0f 	vcmptruess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0f 	vcmptruess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0f 	vcmptruess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0f 	vcmptruess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0f 	vcmptruess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0f 	vcmptruess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 10 	vcmpeq_osss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 10 	vcmpeq_osss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 10 	vcmpeq_osss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 10 	vcmpeq_osss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 10 	vcmpeq_osss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 10 	vcmpeq_osss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 10 	vcmpeq_osss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 10 	vcmpeq_osss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 11 	vcmplt_oqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 11 	vcmplt_oqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 11 	vcmplt_oqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 11 	vcmplt_oqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 11 	vcmplt_oqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 11 	vcmplt_oqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 11 	vcmplt_oqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 11 	vcmplt_oqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 12 	vcmple_oqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 12 	vcmple_oqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 12 	vcmple_oqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 12 	vcmple_oqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 12 	vcmple_oqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 12 	vcmple_oqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 12 	vcmple_oqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 12 	vcmple_oqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 13 	vcmpunord_sss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 13 	vcmpunord_sss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 13 	vcmpunord_sss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 13 	vcmpunord_sss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 13 	vcmpunord_sss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 13 	vcmpunord_sss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 13 	vcmpunord_sss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 13 	vcmpunord_sss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 14 	vcmpneq_usss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 14 	vcmpneq_usss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 14 	vcmpneq_usss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 14 	vcmpneq_usss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 14 	vcmpneq_usss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 14 	vcmpneq_usss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 14 	vcmpneq_usss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 14 	vcmpneq_usss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 15 	vcmpnlt_uqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 15 	vcmpnlt_uqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 15 	vcmpnlt_uqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 15 	vcmpnlt_uqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 15 	vcmpnlt_uqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 15 	vcmpnlt_uqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 15 	vcmpnlt_uqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 15 	vcmpnlt_uqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 16 	vcmpnle_uqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 16 	vcmpnle_uqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 16 	vcmpnle_uqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 16 	vcmpnle_uqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 16 	vcmpnle_uqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 16 	vcmpnle_uqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 16 	vcmpnle_uqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 16 	vcmpnle_uqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 17 	vcmpord_sss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 17 	vcmpord_sss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 17 	vcmpord_sss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 17 	vcmpord_sss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 17 	vcmpord_sss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 17 	vcmpord_sss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 17 	vcmpord_sss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 17 	vcmpord_sss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 18 	vcmpeq_usss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 18 	vcmpeq_usss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 18 	vcmpeq_usss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 18 	vcmpeq_usss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 18 	vcmpeq_usss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 18 	vcmpeq_usss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 18 	vcmpeq_usss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 18 	vcmpeq_usss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 19 	vcmpnge_uqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 19 	vcmpnge_uqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 19 	vcmpnge_uqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 19 	vcmpnge_uqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 19 	vcmpnge_uqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 19 	vcmpnge_uqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 19 	vcmpnge_uqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 19 	vcmpnge_uqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 1a 	vcmpngt_uqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1a 	vcmpngt_uqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 1a 	vcmpngt_uqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 1a 	vcmpngt_uqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 1a 	vcmpngt_uqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 1a 	vcmpngt_uqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 1a 	vcmpngt_uqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 1a 	vcmpngt_uqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 1b 	vcmpfalse_osss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1b 	vcmpfalse_osss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 1b 	vcmpfalse_osss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 1b 	vcmpfalse_osss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 1b 	vcmpfalse_osss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 1b 	vcmpfalse_osss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 1b 	vcmpfalse_osss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 1b 	vcmpfalse_osss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 1c 	vcmpneq_osss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1c 	vcmpneq_osss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 1c 	vcmpneq_osss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 1c 	vcmpneq_osss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 1c 	vcmpneq_osss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 1c 	vcmpneq_osss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 1c 	vcmpneq_osss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 1c 	vcmpneq_osss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 1d 	vcmpge_oqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1d 	vcmpge_oqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 1d 	vcmpge_oqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 1d 	vcmpge_oqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 1d 	vcmpge_oqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 1d 	vcmpge_oqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 1d 	vcmpge_oqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 1d 	vcmpge_oqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 1e 	vcmpgt_oqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1e 	vcmpgt_oqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 1e 	vcmpgt_oqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 1e 	vcmpgt_oqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 1e 	vcmpgt_oqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 1e 	vcmpgt_oqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 1e 	vcmpgt_oqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 1e 	vcmpgt_oqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 1f 	vcmptrue_usss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1f 	vcmptrue_usss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 1f 	vcmptrue_usss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 1f 	vcmptrue_usss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 1f 	vcmptrue_usss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 1f 	vcmptrue_usss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 1f 	vcmptrue_usss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 1f 	vcmptrue_usss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 fd 18 2f f5    	vcomisd \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 7c 18 2f f5    	vcomiss \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 7f 18 2d c6    	vcvtsd2si \{rn-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 58 2d c6    	vcvtsd2si \{ru-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 38 2d c6    	vcvtsd2si \{rd-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 78 2d c6    	vcvtsd2si \{rz-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 18 2d ee    	vcvtsd2si \{rn-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 58 2d ee    	vcvtsd2si \{ru-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 38 2d ee    	vcvtsd2si \{rd-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 78 2d ee    	vcvtsd2si \{rz-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a f4    	vcvtsd2ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 5a f4    	vcvtsd2ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5a f4    	vcvtsd2ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 5a f4    	vcvtsd2ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 5a f4    	vcvtsd2ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 5a f4    	vcvtsd2ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a 31    	vcvtsd2ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a b4 f4 c0 1d fe ff 	vcvtsd2ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a 72 7f 	vcvtsd2ss 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a b2 00 04 00 00 	vcvtsd2ss 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a 72 80 	vcvtsd2ss -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a b2 f8 fb ff ff 	vcvtsd2ss -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 18 2a f0    	vcvtsi2ss %eax,\{rn-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 58 2a f0    	vcvtsi2ss %eax,\{ru-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 38 2a f0    	vcvtsi2ss %eax,\{rd-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 78 2a f0    	vcvtsi2ss %eax,\{rz-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 18 2a f5    	vcvtsi2ss %ebp,\{rn-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 58 2a f5    	vcvtsi2ss %ebp,\{ru-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 38 2a f5    	vcvtsi2ss %ebp,\{rd-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 78 2a f5    	vcvtsi2ss %ebp,\{rz-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a f4    	vcvtss2sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 5a f4    	vcvtss2sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 5a f4    	vcvtss2sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a 31    	vcvtss2sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a b4 f4 c0 1d fe ff 	vcvtss2sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a 72 7f 	vcvtss2sd 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a b2 00 02 00 00 	vcvtss2sd 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a 72 80 	vcvtss2sd -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a b2 fc fd ff ff 	vcvtss2sd -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 18 2d c6    	vcvtss2si \{rn-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 58 2d c6    	vcvtss2si \{ru-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 38 2d c6    	vcvtss2si \{rd-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 78 2d c6    	vcvtss2si \{rz-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 18 2d ee    	vcvtss2si \{rn-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 58 2d ee    	vcvtss2si \{ru-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 38 2d ee    	vcvtss2si \{rd-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 78 2d ee    	vcvtss2si \{rz-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 18 2c c6    	vcvttsd2si \{sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 18 2c ee    	vcvttsd2si \{sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 18 2c c6    	vcvttss2si \{sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 18 2c ee    	vcvttss2si \{sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e f4    	vdivsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 5e f4    	vdivsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5e f4    	vdivsd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 5e f4    	vdivsd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 5e f4    	vdivsd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 5e f4    	vdivsd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e 31    	vdivsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e b4 f4 c0 1d fe ff 	vdivsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e 72 7f 	vdivsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e b2 00 04 00 00 	vdivsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e 72 80 	vdivsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e b2 f8 fb ff ff 	vdivsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e f4    	vdivss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 5e f4    	vdivss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 5e f4    	vdivss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 5e f4    	vdivss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 5e f4    	vdivss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 5e f4    	vdivss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e 31    	vdivss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e b4 f4 c0 1d fe ff 	vdivss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e 72 7f 	vdivss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e b2 00 02 00 00 	vdivss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e 72 80 	vdivss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e b2 fc fd ff ff 	vdivss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 f4    	vfmadd132sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 99 f4    	vfmadd132sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f 99 f4    	vfmadd132sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 99 f4    	vfmadd132sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 99 f4    	vfmadd132sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 99 f4    	vfmadd132sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 31    	vfmadd132sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 b4 f4 c0 1d fe ff 	vfmadd132sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 72 7f 	vfmadd132sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 b2 00 04 00 00 	vfmadd132sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 72 80 	vfmadd132sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 b2 f8 fb ff ff 	vfmadd132sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 f4    	vfmadd132ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 99 f4    	vfmadd132ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f 99 f4    	vfmadd132ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 99 f4    	vfmadd132ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 99 f4    	vfmadd132ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 99 f4    	vfmadd132ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 31    	vfmadd132ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 b4 f4 c0 1d fe ff 	vfmadd132ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 72 7f 	vfmadd132ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 b2 00 02 00 00 	vfmadd132ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 72 80 	vfmadd132ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 b2 fc fd ff ff 	vfmadd132ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 f4    	vfmadd213sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf a9 f4    	vfmadd213sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f a9 f4    	vfmadd213sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f a9 f4    	vfmadd213sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f a9 f4    	vfmadd213sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f a9 f4    	vfmadd213sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 31    	vfmadd213sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 b4 f4 c0 1d fe ff 	vfmadd213sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 72 7f 	vfmadd213sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 b2 00 04 00 00 	vfmadd213sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 72 80 	vfmadd213sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 b2 f8 fb ff ff 	vfmadd213sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 f4    	vfmadd213ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf a9 f4    	vfmadd213ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f a9 f4    	vfmadd213ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f a9 f4    	vfmadd213ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f a9 f4    	vfmadd213ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f a9 f4    	vfmadd213ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 31    	vfmadd213ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 b4 f4 c0 1d fe ff 	vfmadd213ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 72 7f 	vfmadd213ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 b2 00 02 00 00 	vfmadd213ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 72 80 	vfmadd213ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 b2 fc fd ff ff 	vfmadd213ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 f4    	vfmadd231sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf b9 f4    	vfmadd231sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f b9 f4    	vfmadd231sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f b9 f4    	vfmadd231sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f b9 f4    	vfmadd231sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f b9 f4    	vfmadd231sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 31    	vfmadd231sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 b4 f4 c0 1d fe ff 	vfmadd231sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 72 7f 	vfmadd231sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 b2 00 04 00 00 	vfmadd231sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 72 80 	vfmadd231sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 b2 f8 fb ff ff 	vfmadd231sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 f4    	vfmadd231ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf b9 f4    	vfmadd231ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f b9 f4    	vfmadd231ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f b9 f4    	vfmadd231ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f b9 f4    	vfmadd231ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f b9 f4    	vfmadd231ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 31    	vfmadd231ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 b4 f4 c0 1d fe ff 	vfmadd231ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 72 7f 	vfmadd231ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 b2 00 02 00 00 	vfmadd231ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 72 80 	vfmadd231ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 b2 fc fd ff ff 	vfmadd231ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b f4    	vfmsub132sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 9b f4    	vfmsub132sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f 9b f4    	vfmsub132sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 9b f4    	vfmsub132sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 9b f4    	vfmsub132sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 9b f4    	vfmsub132sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b 31    	vfmsub132sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b b4 f4 c0 1d fe ff 	vfmsub132sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b 72 7f 	vfmsub132sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b b2 00 04 00 00 	vfmsub132sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b 72 80 	vfmsub132sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b b2 f8 fb ff ff 	vfmsub132sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b f4    	vfmsub132ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 9b f4    	vfmsub132ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f 9b f4    	vfmsub132ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 9b f4    	vfmsub132ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 9b f4    	vfmsub132ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 9b f4    	vfmsub132ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b 31    	vfmsub132ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b b4 f4 c0 1d fe ff 	vfmsub132ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b 72 7f 	vfmsub132ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b b2 00 02 00 00 	vfmsub132ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b 72 80 	vfmsub132ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b b2 fc fd ff ff 	vfmsub132ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab f4    	vfmsub213sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf ab f4    	vfmsub213sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f ab f4    	vfmsub213sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f ab f4    	vfmsub213sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f ab f4    	vfmsub213sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f ab f4    	vfmsub213sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab 31    	vfmsub213sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab b4 f4 c0 1d fe ff 	vfmsub213sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab 72 7f 	vfmsub213sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab b2 00 04 00 00 	vfmsub213sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab 72 80 	vfmsub213sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab b2 f8 fb ff ff 	vfmsub213sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab f4    	vfmsub213ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf ab f4    	vfmsub213ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f ab f4    	vfmsub213ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f ab f4    	vfmsub213ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f ab f4    	vfmsub213ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f ab f4    	vfmsub213ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab 31    	vfmsub213ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab b4 f4 c0 1d fe ff 	vfmsub213ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab 72 7f 	vfmsub213ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab b2 00 02 00 00 	vfmsub213ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab 72 80 	vfmsub213ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab b2 fc fd ff ff 	vfmsub213ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb f4    	vfmsub231sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf bb f4    	vfmsub231sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f bb f4    	vfmsub231sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f bb f4    	vfmsub231sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f bb f4    	vfmsub231sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f bb f4    	vfmsub231sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb 31    	vfmsub231sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb b4 f4 c0 1d fe ff 	vfmsub231sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb 72 7f 	vfmsub231sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb b2 00 04 00 00 	vfmsub231sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb 72 80 	vfmsub231sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb b2 f8 fb ff ff 	vfmsub231sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb f4    	vfmsub231ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf bb f4    	vfmsub231ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f bb f4    	vfmsub231ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f bb f4    	vfmsub231ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f bb f4    	vfmsub231ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f bb f4    	vfmsub231ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb 31    	vfmsub231ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb b4 f4 c0 1d fe ff 	vfmsub231ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb 72 7f 	vfmsub231ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb b2 00 02 00 00 	vfmsub231ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb 72 80 	vfmsub231ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb b2 fc fd ff ff 	vfmsub231ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d f4    	vfnmadd132sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 9d f4    	vfnmadd132sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f 9d f4    	vfnmadd132sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 9d f4    	vfnmadd132sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 9d f4    	vfnmadd132sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 9d f4    	vfnmadd132sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d 31    	vfnmadd132sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d b4 f4 c0 1d fe ff 	vfnmadd132sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d 72 7f 	vfnmadd132sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d b2 00 04 00 00 	vfnmadd132sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d 72 80 	vfnmadd132sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d b2 f8 fb ff ff 	vfnmadd132sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d f4    	vfnmadd132ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 9d f4    	vfnmadd132ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f 9d f4    	vfnmadd132ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 9d f4    	vfnmadd132ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 9d f4    	vfnmadd132ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 9d f4    	vfnmadd132ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d 31    	vfnmadd132ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d b4 f4 c0 1d fe ff 	vfnmadd132ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d 72 7f 	vfnmadd132ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d b2 00 02 00 00 	vfnmadd132ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d 72 80 	vfnmadd132ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d b2 fc fd ff ff 	vfnmadd132ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad f4    	vfnmadd213sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf ad f4    	vfnmadd213sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f ad f4    	vfnmadd213sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f ad f4    	vfnmadd213sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f ad f4    	vfnmadd213sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f ad f4    	vfnmadd213sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad 31    	vfnmadd213sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad b4 f4 c0 1d fe ff 	vfnmadd213sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad 72 7f 	vfnmadd213sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad b2 00 04 00 00 	vfnmadd213sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad 72 80 	vfnmadd213sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad b2 f8 fb ff ff 	vfnmadd213sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad f4    	vfnmadd213ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf ad f4    	vfnmadd213ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f ad f4    	vfnmadd213ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f ad f4    	vfnmadd213ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f ad f4    	vfnmadd213ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f ad f4    	vfnmadd213ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad 31    	vfnmadd213ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad b4 f4 c0 1d fe ff 	vfnmadd213ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad 72 7f 	vfnmadd213ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad b2 00 02 00 00 	vfnmadd213ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad 72 80 	vfnmadd213ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad b2 fc fd ff ff 	vfnmadd213ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd f4    	vfnmadd231sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf bd f4    	vfnmadd231sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f bd f4    	vfnmadd231sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f bd f4    	vfnmadd231sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f bd f4    	vfnmadd231sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f bd f4    	vfnmadd231sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd 31    	vfnmadd231sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd b4 f4 c0 1d fe ff 	vfnmadd231sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd 72 7f 	vfnmadd231sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd b2 00 04 00 00 	vfnmadd231sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd 72 80 	vfnmadd231sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd b2 f8 fb ff ff 	vfnmadd231sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd f4    	vfnmadd231ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf bd f4    	vfnmadd231ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f bd f4    	vfnmadd231ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f bd f4    	vfnmadd231ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f bd f4    	vfnmadd231ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f bd f4    	vfnmadd231ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd 31    	vfnmadd231ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd b4 f4 c0 1d fe ff 	vfnmadd231ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd 72 7f 	vfnmadd231ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd b2 00 02 00 00 	vfnmadd231ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd 72 80 	vfnmadd231ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd b2 fc fd ff ff 	vfnmadd231ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f f4    	vfnmsub132sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 9f f4    	vfnmsub132sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f 9f f4    	vfnmsub132sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 9f f4    	vfnmsub132sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 9f f4    	vfnmsub132sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 9f f4    	vfnmsub132sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f 31    	vfnmsub132sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f b4 f4 c0 1d fe ff 	vfnmsub132sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f 72 7f 	vfnmsub132sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f b2 00 04 00 00 	vfnmsub132sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f 72 80 	vfnmsub132sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f b2 f8 fb ff ff 	vfnmsub132sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f f4    	vfnmsub132ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 9f f4    	vfnmsub132ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f 9f f4    	vfnmsub132ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 9f f4    	vfnmsub132ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 9f f4    	vfnmsub132ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 9f f4    	vfnmsub132ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f 31    	vfnmsub132ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f b4 f4 c0 1d fe ff 	vfnmsub132ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f 72 7f 	vfnmsub132ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f b2 00 02 00 00 	vfnmsub132ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f 72 80 	vfnmsub132ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f b2 fc fd ff ff 	vfnmsub132ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af f4    	vfnmsub213sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf af f4    	vfnmsub213sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f af f4    	vfnmsub213sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f af f4    	vfnmsub213sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f af f4    	vfnmsub213sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f af f4    	vfnmsub213sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af 31    	vfnmsub213sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af b4 f4 c0 1d fe ff 	vfnmsub213sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af 72 7f 	vfnmsub213sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af b2 00 04 00 00 	vfnmsub213sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af 72 80 	vfnmsub213sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af b2 f8 fb ff ff 	vfnmsub213sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af f4    	vfnmsub213ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf af f4    	vfnmsub213ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f af f4    	vfnmsub213ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f af f4    	vfnmsub213ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f af f4    	vfnmsub213ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f af f4    	vfnmsub213ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af 31    	vfnmsub213ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af b4 f4 c0 1d fe ff 	vfnmsub213ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af 72 7f 	vfnmsub213ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af b2 00 02 00 00 	vfnmsub213ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af 72 80 	vfnmsub213ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af b2 fc fd ff ff 	vfnmsub213ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf f4    	vfnmsub231sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf bf f4    	vfnmsub231sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f bf f4    	vfnmsub231sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f bf f4    	vfnmsub231sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f bf f4    	vfnmsub231sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f bf f4    	vfnmsub231sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf 31    	vfnmsub231sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf b4 f4 c0 1d fe ff 	vfnmsub231sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf 72 7f 	vfnmsub231sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf b2 00 04 00 00 	vfnmsub231sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf 72 80 	vfnmsub231sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf b2 f8 fb ff ff 	vfnmsub231sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf f4    	vfnmsub231ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf bf f4    	vfnmsub231ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f bf f4    	vfnmsub231ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f bf f4    	vfnmsub231ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f bf f4    	vfnmsub231ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f bf f4    	vfnmsub231ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf 31    	vfnmsub231ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf b4 f4 c0 1d fe ff 	vfnmsub231ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf 72 7f 	vfnmsub231ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf b2 00 02 00 00 	vfnmsub231ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf 72 80 	vfnmsub231ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf b2 fc fd ff ff 	vfnmsub231ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 f4    	vgetexpsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 43 f4    	vgetexpsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f 43 f4    	vgetexpsd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 31    	vgetexpsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 b4 f4 c0 1d fe ff 	vgetexpsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 72 7f 	vgetexpsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 b2 00 04 00 00 	vgetexpsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 72 80 	vgetexpsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 b2 f8 fb ff ff 	vgetexpsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 f4    	vgetexpss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 43 f4    	vgetexpss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f 43 f4    	vgetexpss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 31    	vgetexpss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 b4 f4 c0 1d fe ff 	vgetexpss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 72 7f 	vgetexpss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 b2 00 02 00 00 	vgetexpss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 72 80 	vgetexpss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 b2 fc fd ff ff 	vgetexpss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 f4 ab 	vgetmantsd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 cf 27 f4 ab 	vgetmantsd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f3 d5 1f 27 f4 ab 	vgetmantsd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 f4 7b 	vgetmantsd \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 1f 27 f4 7b 	vgetmantsd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 31 7b 	vgetmantsd \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 b4 f4 c0 1d fe ff 7b 	vgetmantsd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 72 7f 7b 	vgetmantsd \$0x7b,0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 b2 00 04 00 00 7b 	vgetmantsd \$0x7b,0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 72 80 7b 	vgetmantsd \$0x7b,-0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 b2 f8 fb ff ff 7b 	vgetmantsd \$0x7b,-0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 f4 ab 	vgetmantss \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 cf 27 f4 ab 	vgetmantss \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f3 55 1f 27 f4 ab 	vgetmantss \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 f4 7b 	vgetmantss \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 1f 27 f4 7b 	vgetmantss \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 31 7b 	vgetmantss \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 b4 f4 c0 1d fe ff 7b 	vgetmantss \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 72 7f 7b 	vgetmantss \$0x7b,0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 b2 00 02 00 00 7b 	vgetmantss \$0x7b,0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 72 80 7b 	vgetmantss \$0x7b,-0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 b2 fc fd ff ff 7b 	vgetmantss \$0x7b,-0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f f4    	vmaxsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 5f f4    	vmaxsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5f f4    	vmaxsd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f 31    	vmaxsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f b4 f4 c0 1d fe ff 	vmaxsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f 72 7f 	vmaxsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f b2 00 04 00 00 	vmaxsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f 72 80 	vmaxsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f b2 f8 fb ff ff 	vmaxsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f f4    	vmaxss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 5f f4    	vmaxss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 5f f4    	vmaxss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f 31    	vmaxss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f b4 f4 c0 1d fe ff 	vmaxss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f 72 7f 	vmaxss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f b2 00 02 00 00 	vmaxss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f 72 80 	vmaxss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f b2 fc fd ff ff 	vmaxss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d f4    	vminsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 5d f4    	vminsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5d f4    	vminsd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d 31    	vminsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d b4 f4 c0 1d fe ff 	vminsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d 72 7f 	vminsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d b2 00 04 00 00 	vminsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d 72 80 	vminsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d b2 f8 fb ff ff 	vminsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d f4    	vminss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 5d f4    	vminss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 5d f4    	vminss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d 31    	vminss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d b4 f4 c0 1d fe ff 	vminss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d 72 7f 	vminss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d b2 00 02 00 00 	vminss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d 72 80 	vminss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d b2 fc fd ff ff 	vminss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 10 31    	vmovsd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff cf 10 31    	vmovsd \(%ecx\),%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 10 b4 f4 c0 1d fe ff 	vmovsd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 10 72 7f 	vmovsd 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 10 b2 00 04 00 00 	vmovsd 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 10 72 80 	vmovsd -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 10 b2 f8 fb ff ff 	vmovsd -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 11 31    	vmovsd %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 11 b4 f4 c0 1d fe ff 	vmovsd %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 11 72 7f 	vmovsd %xmm6,0x3f8\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 11 b2 00 04 00 00 	vmovsd %xmm6,0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 11 72 80 	vmovsd %xmm6,-0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 11 b2 f8 fb ff ff 	vmovsd %xmm6,-0x408\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 10 f4    	vmovsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 10 f4    	vmovsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 10 31    	vmovss \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e cf 10 31    	vmovss \(%ecx\),%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 10 b4 f4 c0 1d fe ff 	vmovss -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 10 72 7f 	vmovss 0x1fc\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 10 b2 00 02 00 00 	vmovss 0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 10 72 80 	vmovss -0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 10 b2 fc fd ff ff 	vmovss -0x204\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 11 31    	vmovss %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 11 b4 f4 c0 1d fe ff 	vmovss %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 11 72 7f 	vmovss %xmm6,0x1fc\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 11 b2 00 02 00 00 	vmovss %xmm6,0x200\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 11 72 80 	vmovss %xmm6,-0x200\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 11 b2 fc fd ff ff 	vmovss %xmm6,-0x204\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 10 f4    	vmovss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 10 f4    	vmovss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 f4    	vmulsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 59 f4    	vmulsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 59 f4    	vmulsd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 59 f4    	vmulsd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 59 f4    	vmulsd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 59 f4    	vmulsd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 31    	vmulsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 b4 f4 c0 1d fe ff 	vmulsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 72 7f 	vmulsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 b2 00 04 00 00 	vmulsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 72 80 	vmulsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 b2 f8 fb ff ff 	vmulsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 f4    	vmulss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 59 f4    	vmulss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 59 f4    	vmulss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 59 f4    	vmulss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 59 f4    	vmulss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 59 f4    	vmulss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 31    	vmulss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 b4 f4 c0 1d fe ff 	vmulss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 72 7f 	vmulss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 b2 00 02 00 00 	vmulss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 72 80 	vmulss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 b2 fc fd ff ff 	vmulss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d f4    	vrcp14sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 4d f4    	vrcp14sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d 31    	vrcp14sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d b4 f4 c0 1d fe ff 	vrcp14sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d 72 7f 	vrcp14sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d b2 00 04 00 00 	vrcp14sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d 72 80 	vrcp14sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d b2 f8 fb ff ff 	vrcp14sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d f4    	vrcp14ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 4d f4    	vrcp14ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d 31    	vrcp14ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d b4 f4 c0 1d fe ff 	vrcp14ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d 72 7f 	vrcp14ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d b2 00 02 00 00 	vrcp14ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d 72 80 	vrcp14ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d b2 fc fd ff ff 	vrcp14ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb f4    	vrcp28ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf cb f4    	vrcp28ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f cb f4    	vrcp28ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb 31    	vrcp28ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb b4 f4 c0 1d fe ff 	vrcp28ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb 72 7f 	vrcp28ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb b2 00 02 00 00 	vrcp28ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb 72 80 	vrcp28ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb b2 fc fd ff ff 	vrcp28ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb f4    	vrcp28sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf cb f4    	vrcp28sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f cb f4    	vrcp28sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb 31    	vrcp28sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb b4 f4 c0 1d fe ff 	vrcp28sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb 72 7f 	vrcp28sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb b2 00 04 00 00 	vrcp28sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb 72 80 	vrcp28sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb b2 f8 fb ff ff 	vrcp28sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f f4    	vrsqrt14sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 4f f4    	vrsqrt14sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f 31    	vrsqrt14sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f b4 f4 c0 1d fe ff 	vrsqrt14sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f 72 7f 	vrsqrt14sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f b2 00 04 00 00 	vrsqrt14sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f 72 80 	vrsqrt14sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f b2 f8 fb ff ff 	vrsqrt14sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f f4    	vrsqrt14ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 4f f4    	vrsqrt14ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f 31    	vrsqrt14ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f b4 f4 c0 1d fe ff 	vrsqrt14ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f 72 7f 	vrsqrt14ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f b2 00 02 00 00 	vrsqrt14ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f 72 80 	vrsqrt14ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f b2 fc fd ff ff 	vrsqrt14ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd f4    	vrsqrt28ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf cd f4    	vrsqrt28ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f cd f4    	vrsqrt28ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd 31    	vrsqrt28ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd b4 f4 c0 1d fe ff 	vrsqrt28ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd 72 7f 	vrsqrt28ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd b2 00 02 00 00 	vrsqrt28ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd 72 80 	vrsqrt28ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd b2 fc fd ff ff 	vrsqrt28ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd f4    	vrsqrt28sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf cd f4    	vrsqrt28sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f cd f4    	vrsqrt28sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd 31    	vrsqrt28sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd b4 f4 c0 1d fe ff 	vrsqrt28sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd 72 7f 	vrsqrt28sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd b2 00 04 00 00 	vrsqrt28sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd 72 80 	vrsqrt28sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd b2 f8 fb ff ff 	vrsqrt28sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 f4    	vsqrtsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 51 f4    	vsqrtsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 51 f4    	vsqrtsd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 51 f4    	vsqrtsd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 51 f4    	vsqrtsd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 51 f4    	vsqrtsd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 31    	vsqrtsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 b4 f4 c0 1d fe ff 	vsqrtsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 72 7f 	vsqrtsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 b2 00 04 00 00 	vsqrtsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 72 80 	vsqrtsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 b2 f8 fb ff ff 	vsqrtsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 f4    	vsqrtss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 51 f4    	vsqrtss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 51 f4    	vsqrtss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 51 f4    	vsqrtss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 51 f4    	vsqrtss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 51 f4    	vsqrtss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 31    	vsqrtss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 b4 f4 c0 1d fe ff 	vsqrtss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 72 7f 	vsqrtss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 b2 00 02 00 00 	vsqrtss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 72 80 	vsqrtss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 b2 fc fd ff ff 	vsqrtss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c f4    	vsubsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 5c f4    	vsubsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5c f4    	vsubsd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 5c f4    	vsubsd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 5c f4    	vsubsd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 5c f4    	vsubsd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c 31    	vsubsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c b4 f4 c0 1d fe ff 	vsubsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c 72 7f 	vsubsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c b2 00 04 00 00 	vsubsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c 72 80 	vsubsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c b2 f8 fb ff ff 	vsubsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c f4    	vsubss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 5c f4    	vsubss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 5c f4    	vsubss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 5c f4    	vsubss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 5c f4    	vsubss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 5c f4    	vsubss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c 31    	vsubss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c b4 f4 c0 1d fe ff 	vsubss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c 72 7f 	vsubss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c b2 00 02 00 00 	vsubss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c 72 80 	vsubss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c b2 fc fd ff ff 	vsubss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 fd 18 2e f5    	vucomisd \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 7c 18 2e f5    	vucomiss \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 c6    	vcvtsd2usi %xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 18 79 c6    	vcvtsd2usi \{rn-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 58 79 c6    	vcvtsd2usi \{ru-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 38 79 c6    	vcvtsd2usi \{rd-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 78 79 c6    	vcvtsd2usi \{rz-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 01    	vcvtsd2usi \(%ecx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 84 f4 c0 1d fe ff 	vcvtsd2usi -0x1e240\(%esp,%esi,8\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 42 7f 	vcvtsd2usi 0x3f8\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 82 00 04 00 00 	vcvtsd2usi 0x400\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 42 80 	vcvtsd2usi -0x400\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 82 f8 fb ff ff 	vcvtsd2usi -0x408\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 ee    	vcvtsd2usi %xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 18 79 ee    	vcvtsd2usi \{rn-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 58 79 ee    	vcvtsd2usi \{ru-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 38 79 ee    	vcvtsd2usi \{rd-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 78 79 ee    	vcvtsd2usi \{rz-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 29    	vcvtsd2usi \(%ecx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 ac f4 c0 1d fe ff 	vcvtsd2usi -0x1e240\(%esp,%esi,8\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 6a 7f 	vcvtsd2usi 0x3f8\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 aa 00 04 00 00 	vcvtsd2usi 0x400\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 6a 80 	vcvtsd2usi -0x400\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 aa f8 fb ff ff 	vcvtsd2usi -0x408\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 c6    	vcvtss2usi %xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 18 79 c6    	vcvtss2usi \{rn-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 58 79 c6    	vcvtss2usi \{ru-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 38 79 c6    	vcvtss2usi \{rd-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 78 79 c6    	vcvtss2usi \{rz-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 01    	vcvtss2usi \(%ecx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 84 f4 c0 1d fe ff 	vcvtss2usi -0x1e240\(%esp,%esi,8\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 42 7f 	vcvtss2usi 0x1fc\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 82 00 02 00 00 	vcvtss2usi 0x200\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 42 80 	vcvtss2usi -0x200\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 82 fc fd ff ff 	vcvtss2usi -0x204\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 ee    	vcvtss2usi %xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 18 79 ee    	vcvtss2usi \{rn-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 58 79 ee    	vcvtss2usi \{ru-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 38 79 ee    	vcvtss2usi \{rd-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 78 79 ee    	vcvtss2usi \{rz-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 29    	vcvtss2usi \(%ecx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 ac f4 c0 1d fe ff 	vcvtss2usi -0x1e240\(%esp,%esi,8\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 6a 7f 	vcvtss2usi 0x1fc\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 aa 00 02 00 00 	vcvtss2usi 0x200\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 6a 80 	vcvtss2usi -0x200\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 aa fc fd ff ff 	vcvtss2usi -0x204\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 57 48 7b f0    	vcvtusi2sd %eax,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b f5    	vcvtusi2sd %ebp,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b 31    	vcvtusi2sd \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b b4 f4 c0 1d fe ff 	vcvtusi2sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b 72 7f 	vcvtusi2sd 0x1fc\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b b2 00 02 00 00 	vcvtusi2sd 0x200\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b 72 80 	vcvtusi2sd -0x200\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b b2 fc fd ff ff 	vcvtusi2sd -0x204\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b f0    	vcvtusi2ss %eax,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 18 7b f0    	vcvtusi2ss %eax,\{rn-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 58 7b f0    	vcvtusi2ss %eax,\{ru-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 38 7b f0    	vcvtusi2ss %eax,\{rd-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 78 7b f0    	vcvtusi2ss %eax,\{rz-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b f5    	vcvtusi2ss %ebp,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 18 7b f5    	vcvtusi2ss %ebp,\{rn-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 58 7b f5    	vcvtusi2ss %ebp,\{ru-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 38 7b f5    	vcvtusi2ss %ebp,\{rd-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 78 7b f5    	vcvtusi2ss %ebp,\{rz-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b 31    	vcvtusi2ss \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b b4 f4 c0 1d fe ff 	vcvtusi2ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b 72 7f 	vcvtusi2ss 0x1fc\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b b2 00 02 00 00 	vcvtusi2ss 0x200\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b 72 80 	vcvtusi2ss -0x200\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b b2 fc fd ff ff 	vcvtusi2ss -0x204\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d f4    	vscalefsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 2d f4    	vscalefsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f 2d f4    	vscalefsd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 2d f4    	vscalefsd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 2d f4    	vscalefsd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 2d f4    	vscalefsd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d 31    	vscalefsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d b4 f4 c0 1d fe ff 	vscalefsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d 72 7f 	vscalefsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d b2 00 04 00 00 	vscalefsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d 72 80 	vscalefsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d b2 f8 fb ff ff 	vscalefsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d f4    	vscalefss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 2d f4    	vscalefss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f 2d f4    	vscalefss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 2d f4    	vscalefss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 2d f4    	vscalefss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 2d f4    	vscalefss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d 31    	vscalefss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d b4 f4 c0 1d fe ff 	vscalefss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d 72 7f 	vscalefss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d b2 00 02 00 00 	vscalefss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d 72 80 	vscalefss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d b2 fc fd ff ff 	vscalefss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 f4 ab 	vfixupimmss \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 cf 55 f4 ab 	vfixupimmss \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f3 55 1f 55 f4 ab 	vfixupimmss \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 f4 7b 	vfixupimmss \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 1f 55 f4 7b 	vfixupimmss \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 31 7b 	vfixupimmss \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 b4 f4 c0 1d fe ff 7b 	vfixupimmss \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 72 7f 7b 	vfixupimmss \$0x7b,0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 b2 00 02 00 00 7b 	vfixupimmss \$0x7b,0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 72 80 7b 	vfixupimmss \$0x7b,-0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 b2 fc fd ff ff 7b 	vfixupimmss \$0x7b,-0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 f4 ab 	vfixupimmsd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 cf 55 f4 ab 	vfixupimmsd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f3 d5 1f 55 f4 ab 	vfixupimmsd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 f4 7b 	vfixupimmsd \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 1f 55 f4 7b 	vfixupimmsd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 31 7b 	vfixupimmsd \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 b4 f4 c0 1d fe ff 7b 	vfixupimmsd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 72 7f 7b 	vfixupimmsd \$0x7b,0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 b2 00 04 00 00 7b 	vfixupimmsd \$0x7b,0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 72 80 7b 	vfixupimmsd \$0x7b,-0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 b2 f8 fb ff ff 7b 	vfixupimmsd \$0x7b,-0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b f4 ab 	vrndscalesd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 cf 0b f4 ab 	vrndscalesd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f3 d5 1f 0b f4 ab 	vrndscalesd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b f4 7b 	vrndscalesd \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 1f 0b f4 7b 	vrndscalesd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b 31 7b 	vrndscalesd \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b b4 f4 c0 1d fe ff 7b 	vrndscalesd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b 72 7f 7b 	vrndscalesd \$0x7b,0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b b2 00 04 00 00 7b 	vrndscalesd \$0x7b,0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b 72 80 7b 	vrndscalesd \$0x7b,-0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b b2 f8 fb ff ff 7b 	vrndscalesd \$0x7b,-0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a f4 ab 	vrndscaless \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 cf 0a f4 ab 	vrndscaless \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f3 55 1f 0a f4 ab 	vrndscaless \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a f4 7b 	vrndscaless \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 1f 0a f4 7b 	vrndscaless \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a 31 7b 	vrndscaless \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a b4 f4 c0 1d fe ff 7b 	vrndscaless \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a 72 7f 7b 	vrndscaless \$0x7b,0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a b2 00 02 00 00 7b 	vrndscaless \$0x7b,0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a 72 80 7b 	vrndscaless \$0x7b,-0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a b2 fc fd ff ff 7b 	vrndscaless \$0x7b,-0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 56 48 c2 ec 7b 	vcmpsh \$0x7b,%xmm4,%xmm5,%k5
[ 	]*[a-f0-9]+:	62 f3 56 1f c2 ec 7b 	vcmpsh \$0x7b,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 56 48 c2 29 7b 	vcmpsh \$0x7b,\(%ecx\),%xmm5,%k5
[ 	]*[a-f0-9]+:	62 f3 56 4f c2 ac f4 c0 1d fe ff 7b 	vcmpsh \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 56 48 c2 69 7f 7b 	vcmpsh \$0x7b,0xfe\(%ecx\),%xmm5,%k5
[ 	]*[a-f0-9]+:	62 f3 56 4f c2 6a 80 7b 	vcmpsh \$0x7b,-0x100\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 7c 48 67 ec 7b 	vfpclasssh \$0x7b,%xmm4,%k5
[ 	]*[a-f0-9]+:	62 f3 7c 48 67 29 7b 	vfpclasssh \$0x7b,\(%ecx\),%k5
[ 	]*[a-f0-9]+:	62 f3 7c 4f 67 ac f4 c0 1d fe ff 7b 	vfpclasssh \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 7c 48 67 69 7f 7b 	vfpclasssh \$0x7b,0xfe\(%ecx\),%k5
[ 	]*[a-f0-9]+:	62 f3 7c 4f 67 6a 80 7b 	vfpclasssh \$0x7b,-0x100\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 f4    	vaddsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 58 f4    	vaddsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 58 f4    	vaddsd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 58 f4    	vaddsd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 58 f4    	vaddsd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 58 f4    	vaddsd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 31    	vaddsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 b4 f4 c0 1d fe ff 	vaddsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 72 7f 	vaddsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 b2 00 04 00 00 	vaddsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 72 80 	vaddsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 58 b2 f8 fb ff ff 	vaddsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 f4    	vaddss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 58 f4    	vaddss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 58 f4    	vaddss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 58 f4    	vaddss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 58 f4    	vaddss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 58 f4    	vaddss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 31    	vaddss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 b4 f4 c0 1d fe ff 	vaddss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 72 7f 	vaddss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 b2 00 02 00 00 	vaddss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 72 80 	vaddss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 58 b2 fc fd ff ff 	vaddss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec ab 	vcmpsd \$0xab,%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec ab 	vcmpsd \$0xab,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 7b 	vcmpsd \$0x7b,%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 7b 	vcmpsd \$0x7b,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 7b 	vcmpsd \$0x7b,\(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 7b 	vcmpsd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 7b 	vcmpsd \$0x7b,0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 7b 	vcmpsd \$0x7b,0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 7b 	vcmpsd \$0x7b,-0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 7b 	vcmpsd \$0x7b,-0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 00 	vcmpeqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 00 	vcmpeqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 00 	vcmpeqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 00 	vcmpeqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 00 	vcmpeqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 00 	vcmpeqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 00 	vcmpeqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 00 	vcmpeqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 00 	vcmpeqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 00 	vcmpeqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 00 	vcmpeqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 00 	vcmpeqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 00 	vcmpeqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 00 	vcmpeqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 00 	vcmpeqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 00 	vcmpeqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 01 	vcmpltsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 01 	vcmpltsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 01 	vcmpltsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 01 	vcmpltsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 01 	vcmpltsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 01 	vcmpltsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 01 	vcmpltsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 01 	vcmpltsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 01 	vcmpltsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 01 	vcmpltsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 01 	vcmpltsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 01 	vcmpltsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 01 	vcmpltsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 01 	vcmpltsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 01 	vcmpltsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 01 	vcmpltsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 02 	vcmplesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 02 	vcmplesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 02 	vcmplesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 02 	vcmplesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 02 	vcmplesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 02 	vcmplesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 02 	vcmplesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 02 	vcmplesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 02 	vcmplesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 02 	vcmplesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 02 	vcmplesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 02 	vcmplesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 02 	vcmplesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 02 	vcmplesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 02 	vcmplesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 02 	vcmplesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 03 	vcmpunordsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 03 	vcmpunordsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 03 	vcmpunordsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 03 	vcmpunordsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 03 	vcmpunordsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 03 	vcmpunordsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 03 	vcmpunordsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 03 	vcmpunordsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 03 	vcmpunordsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 03 	vcmpunordsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 03 	vcmpunordsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 03 	vcmpunordsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 03 	vcmpunordsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 03 	vcmpunordsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 03 	vcmpunordsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 03 	vcmpunordsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 04 	vcmpneqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 04 	vcmpneqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 04 	vcmpneqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 04 	vcmpneqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 04 	vcmpneqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 04 	vcmpneqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 04 	vcmpneqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 04 	vcmpneqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 04 	vcmpneqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 04 	vcmpneqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 04 	vcmpneqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 04 	vcmpneqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 04 	vcmpneqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 04 	vcmpneqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 04 	vcmpneqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 04 	vcmpneqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 05 	vcmpnltsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 05 	vcmpnltsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 05 	vcmpnltsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 05 	vcmpnltsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 05 	vcmpnltsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 05 	vcmpnltsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 05 	vcmpnltsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 05 	vcmpnltsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 05 	vcmpnltsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 05 	vcmpnltsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 05 	vcmpnltsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 05 	vcmpnltsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 05 	vcmpnltsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 05 	vcmpnltsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 05 	vcmpnltsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 05 	vcmpnltsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 06 	vcmpnlesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 06 	vcmpnlesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 06 	vcmpnlesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 06 	vcmpnlesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 06 	vcmpnlesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 06 	vcmpnlesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 06 	vcmpnlesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 06 	vcmpnlesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 06 	vcmpnlesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 06 	vcmpnlesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 06 	vcmpnlesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 06 	vcmpnlesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 06 	vcmpnlesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 06 	vcmpnlesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 06 	vcmpnlesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 06 	vcmpnlesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 07 	vcmpordsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 07 	vcmpordsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 07 	vcmpordsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 07 	vcmpordsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 07 	vcmpordsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 07 	vcmpordsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 07 	vcmpordsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 07 	vcmpordsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 07 	vcmpordsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 07 	vcmpordsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 07 	vcmpordsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 07 	vcmpordsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 07 	vcmpordsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 07 	vcmpordsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 07 	vcmpordsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 07 	vcmpordsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 08 	vcmpeq_uqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 08 	vcmpeq_uqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 08 	vcmpeq_uqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 08 	vcmpeq_uqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 08 	vcmpeq_uqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 08 	vcmpeq_uqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 08 	vcmpeq_uqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 08 	vcmpeq_uqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 09 	vcmpngesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 09 	vcmpngesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 09 	vcmpngesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 09 	vcmpngesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 09 	vcmpngesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 09 	vcmpngesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 09 	vcmpngesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 09 	vcmpngesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 09 	vcmpngesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 09 	vcmpngesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 09 	vcmpngesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 09 	vcmpngesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 09 	vcmpngesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 09 	vcmpngesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 09 	vcmpngesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 09 	vcmpngesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0a 	vcmpngtsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0a 	vcmpngtsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0a 	vcmpngtsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0a 	vcmpngtsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0a 	vcmpngtsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0a 	vcmpngtsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0a 	vcmpngtsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0a 	vcmpngtsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0a 	vcmpngtsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0a 	vcmpngtsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0a 	vcmpngtsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0a 	vcmpngtsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0a 	vcmpngtsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0a 	vcmpngtsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0a 	vcmpngtsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0a 	vcmpngtsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0b 	vcmpfalsesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0b 	vcmpfalsesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0b 	vcmpfalsesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0b 	vcmpfalsesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0b 	vcmpfalsesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0b 	vcmpfalsesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0b 	vcmpfalsesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0b 	vcmpfalsesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0b 	vcmpfalsesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0b 	vcmpfalsesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0b 	vcmpfalsesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0b 	vcmpfalsesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0b 	vcmpfalsesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0b 	vcmpfalsesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0c 	vcmpneq_oqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0c 	vcmpneq_oqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0c 	vcmpneq_oqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0c 	vcmpneq_oqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0c 	vcmpneq_oqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0c 	vcmpneq_oqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0c 	vcmpneq_oqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0c 	vcmpneq_oqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0d 	vcmpgesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0d 	vcmpgesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0d 	vcmpgesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0d 	vcmpgesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0d 	vcmpgesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0d 	vcmpgesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0d 	vcmpgesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0d 	vcmpgesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0d 	vcmpgesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0d 	vcmpgesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0d 	vcmpgesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0d 	vcmpgesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0d 	vcmpgesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0d 	vcmpgesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0d 	vcmpgesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0d 	vcmpgesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0e 	vcmpgtsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0e 	vcmpgtsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0e 	vcmpgtsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0e 	vcmpgtsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0e 	vcmpgtsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0e 	vcmpgtsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0e 	vcmpgtsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0e 	vcmpgtsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0e 	vcmpgtsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0e 	vcmpgtsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0e 	vcmpgtsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0e 	vcmpgtsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0e 	vcmpgtsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0e 	vcmpgtsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0e 	vcmpgtsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0e 	vcmpgtsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0f 	vcmptruesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0f 	vcmptruesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0f 	vcmptruesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0f 	vcmptruesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0f 	vcmptruesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0f 	vcmptruesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0f 	vcmptruesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0f 	vcmptruesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 0f 	vcmptruesd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 0f 	vcmptruesd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 0f 	vcmptruesd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 0f 	vcmptruesd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 0f 	vcmptruesd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 0f 	vcmptruesd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 0f 	vcmptruesd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 0f 	vcmptruesd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 10 	vcmpeq_ossd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 10 	vcmpeq_ossd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 10 	vcmpeq_ossd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 10 	vcmpeq_ossd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 10 	vcmpeq_ossd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 10 	vcmpeq_ossd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 10 	vcmpeq_ossd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 10 	vcmpeq_ossd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 11 	vcmplt_oqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 11 	vcmplt_oqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 11 	vcmplt_oqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 11 	vcmplt_oqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 11 	vcmplt_oqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 11 	vcmplt_oqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 11 	vcmplt_oqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 11 	vcmplt_oqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 12 	vcmple_oqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 12 	vcmple_oqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 12 	vcmple_oqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 12 	vcmple_oqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 12 	vcmple_oqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 12 	vcmple_oqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 12 	vcmple_oqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 12 	vcmple_oqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 13 	vcmpunord_ssd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 13 	vcmpunord_ssd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 13 	vcmpunord_ssd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 13 	vcmpunord_ssd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 13 	vcmpunord_ssd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 13 	vcmpunord_ssd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 13 	vcmpunord_ssd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 13 	vcmpunord_ssd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 14 	vcmpneq_ussd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 14 	vcmpneq_ussd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 14 	vcmpneq_ussd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 14 	vcmpneq_ussd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 14 	vcmpneq_ussd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 14 	vcmpneq_ussd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 14 	vcmpneq_ussd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 14 	vcmpneq_ussd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 15 	vcmpnlt_uqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 15 	vcmpnlt_uqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 15 	vcmpnlt_uqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 15 	vcmpnlt_uqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 15 	vcmpnlt_uqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 15 	vcmpnlt_uqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 15 	vcmpnlt_uqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 15 	vcmpnlt_uqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 16 	vcmpnle_uqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 16 	vcmpnle_uqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 16 	vcmpnle_uqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 16 	vcmpnle_uqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 16 	vcmpnle_uqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 16 	vcmpnle_uqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 16 	vcmpnle_uqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 16 	vcmpnle_uqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 17 	vcmpord_ssd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 17 	vcmpord_ssd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 17 	vcmpord_ssd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 17 	vcmpord_ssd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 17 	vcmpord_ssd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 17 	vcmpord_ssd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 17 	vcmpord_ssd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 17 	vcmpord_ssd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 18 	vcmpeq_ussd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 18 	vcmpeq_ussd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 18 	vcmpeq_ussd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 18 	vcmpeq_ussd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 18 	vcmpeq_ussd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 18 	vcmpeq_ussd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 18 	vcmpeq_ussd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 18 	vcmpeq_ussd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 19 	vcmpnge_uqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 19 	vcmpnge_uqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 19 	vcmpnge_uqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 19 	vcmpnge_uqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 19 	vcmpnge_uqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 19 	vcmpnge_uqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 19 	vcmpnge_uqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 19 	vcmpnge_uqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 1a 	vcmpngt_uqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1a 	vcmpngt_uqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 1a 	vcmpngt_uqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 1a 	vcmpngt_uqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 1a 	vcmpngt_uqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 1a 	vcmpngt_uqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 1a 	vcmpngt_uqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 1a 	vcmpngt_uqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 1b 	vcmpfalse_ossd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1b 	vcmpfalse_ossd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 1b 	vcmpfalse_ossd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 1b 	vcmpfalse_ossd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 1b 	vcmpfalse_ossd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 1b 	vcmpfalse_ossd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 1b 	vcmpfalse_ossd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 1b 	vcmpfalse_ossd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 1c 	vcmpneq_ossd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1c 	vcmpneq_ossd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 1c 	vcmpneq_ossd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 1c 	vcmpneq_ossd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 1c 	vcmpneq_ossd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 1c 	vcmpneq_ossd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 1c 	vcmpneq_ossd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 1c 	vcmpneq_ossd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 1d 	vcmpge_oqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1d 	vcmpge_oqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 1d 	vcmpge_oqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 1d 	vcmpge_oqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 1d 	vcmpge_oqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 1d 	vcmpge_oqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 1d 	vcmpge_oqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 1d 	vcmpge_oqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 1e 	vcmpgt_oqsd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1e 	vcmpgt_oqsd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 1e 	vcmpgt_oqsd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 1e 	vcmpgt_oqsd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 1e 	vcmpgt_oqsd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 1e 	vcmpgt_oqsd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 1e 	vcmpgt_oqsd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 1e 	vcmpgt_oqsd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ec 1f 	vcmptrue_ussd %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f c2 ec 1f 	vcmptrue_ussd \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 29 1f 	vcmptrue_ussd \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 ac f4 c0 1d fe ff 1f 	vcmptrue_ussd -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 7f 1f 	vcmptrue_ussd 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa 00 04 00 00 1f 	vcmptrue_ussd 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 6a 80 1f 	vcmptrue_ussd -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f c2 aa f8 fb ff ff 1f 	vcmptrue_ussd -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec ab 	vcmpss \$0xab,%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec ab 	vcmpss \$0xab,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 7b 	vcmpss \$0x7b,%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 7b 	vcmpss \$0x7b,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 7b 	vcmpss \$0x7b,\(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 7b 	vcmpss \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 7b 	vcmpss \$0x7b,0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 7b 	vcmpss \$0x7b,0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 7b 	vcmpss \$0x7b,-0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 7b 	vcmpss \$0x7b,-0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 00 	vcmpeqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 00 	vcmpeqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 00 	vcmpeqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 00 	vcmpeqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 00 	vcmpeqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 00 	vcmpeqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 00 	vcmpeqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 00 	vcmpeqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 00 	vcmpeqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 00 	vcmpeqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 00 	vcmpeqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 00 	vcmpeqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 00 	vcmpeqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 00 	vcmpeqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 00 	vcmpeqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 00 	vcmpeqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 01 	vcmpltss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 01 	vcmpltss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 01 	vcmpltss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 01 	vcmpltss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 01 	vcmpltss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 01 	vcmpltss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 01 	vcmpltss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 01 	vcmpltss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 01 	vcmpltss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 01 	vcmpltss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 01 	vcmpltss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 01 	vcmpltss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 01 	vcmpltss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 01 	vcmpltss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 01 	vcmpltss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 01 	vcmpltss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 02 	vcmpless %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 02 	vcmpless \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 02 	vcmpless \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 02 	vcmpless -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 02 	vcmpless 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 02 	vcmpless 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 02 	vcmpless -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 02 	vcmpless -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 02 	vcmpless %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 02 	vcmpless \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 02 	vcmpless \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 02 	vcmpless -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 02 	vcmpless 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 02 	vcmpless 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 02 	vcmpless -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 02 	vcmpless -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 03 	vcmpunordss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 03 	vcmpunordss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 03 	vcmpunordss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 03 	vcmpunordss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 03 	vcmpunordss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 03 	vcmpunordss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 03 	vcmpunordss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 03 	vcmpunordss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 03 	vcmpunordss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 03 	vcmpunordss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 03 	vcmpunordss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 03 	vcmpunordss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 03 	vcmpunordss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 03 	vcmpunordss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 03 	vcmpunordss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 03 	vcmpunordss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 04 	vcmpneqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 04 	vcmpneqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 04 	vcmpneqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 04 	vcmpneqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 04 	vcmpneqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 04 	vcmpneqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 04 	vcmpneqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 04 	vcmpneqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 04 	vcmpneqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 04 	vcmpneqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 04 	vcmpneqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 04 	vcmpneqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 04 	vcmpneqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 04 	vcmpneqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 04 	vcmpneqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 04 	vcmpneqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 05 	vcmpnltss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 05 	vcmpnltss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 05 	vcmpnltss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 05 	vcmpnltss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 05 	vcmpnltss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 05 	vcmpnltss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 05 	vcmpnltss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 05 	vcmpnltss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 05 	vcmpnltss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 05 	vcmpnltss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 05 	vcmpnltss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 05 	vcmpnltss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 05 	vcmpnltss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 05 	vcmpnltss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 05 	vcmpnltss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 05 	vcmpnltss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 06 	vcmpnless %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 06 	vcmpnless \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 06 	vcmpnless \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 06 	vcmpnless -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 06 	vcmpnless 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 06 	vcmpnless 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 06 	vcmpnless -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 06 	vcmpnless -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 06 	vcmpnless %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 06 	vcmpnless \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 06 	vcmpnless \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 06 	vcmpnless -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 06 	vcmpnless 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 06 	vcmpnless 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 06 	vcmpnless -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 06 	vcmpnless -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 07 	vcmpordss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 07 	vcmpordss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 07 	vcmpordss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 07 	vcmpordss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 07 	vcmpordss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 07 	vcmpordss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 07 	vcmpordss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 07 	vcmpordss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 07 	vcmpordss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 07 	vcmpordss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 07 	vcmpordss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 07 	vcmpordss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 07 	vcmpordss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 07 	vcmpordss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 07 	vcmpordss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 07 	vcmpordss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 08 	vcmpeq_uqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 08 	vcmpeq_uqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 08 	vcmpeq_uqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 08 	vcmpeq_uqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 08 	vcmpeq_uqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 08 	vcmpeq_uqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 08 	vcmpeq_uqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 08 	vcmpeq_uqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 09 	vcmpngess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 09 	vcmpngess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 09 	vcmpngess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 09 	vcmpngess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 09 	vcmpngess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 09 	vcmpngess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 09 	vcmpngess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 09 	vcmpngess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 09 	vcmpngess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 09 	vcmpngess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 09 	vcmpngess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 09 	vcmpngess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 09 	vcmpngess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 09 	vcmpngess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 09 	vcmpngess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 09 	vcmpngess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0a 	vcmpngtss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0a 	vcmpngtss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0a 	vcmpngtss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0a 	vcmpngtss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0a 	vcmpngtss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0a 	vcmpngtss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0a 	vcmpngtss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0a 	vcmpngtss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0a 	vcmpngtss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0a 	vcmpngtss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0a 	vcmpngtss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0a 	vcmpngtss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0a 	vcmpngtss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0a 	vcmpngtss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0a 	vcmpngtss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0a 	vcmpngtss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0b 	vcmpfalsess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0b 	vcmpfalsess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0b 	vcmpfalsess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0b 	vcmpfalsess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0b 	vcmpfalsess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0b 	vcmpfalsess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0b 	vcmpfalsess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0b 	vcmpfalsess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0b 	vcmpfalsess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0b 	vcmpfalsess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0b 	vcmpfalsess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0b 	vcmpfalsess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0b 	vcmpfalsess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0b 	vcmpfalsess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0b 	vcmpfalsess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0c 	vcmpneq_oqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0c 	vcmpneq_oqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0c 	vcmpneq_oqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0c 	vcmpneq_oqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0c 	vcmpneq_oqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0c 	vcmpneq_oqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0c 	vcmpneq_oqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0c 	vcmpneq_oqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0d 	vcmpgess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0d 	vcmpgess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0d 	vcmpgess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0d 	vcmpgess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0d 	vcmpgess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0d 	vcmpgess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0d 	vcmpgess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0d 	vcmpgess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0d 	vcmpgess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0d 	vcmpgess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0d 	vcmpgess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0d 	vcmpgess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0d 	vcmpgess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0d 	vcmpgess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0d 	vcmpgess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0d 	vcmpgess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0e 	vcmpgtss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0e 	vcmpgtss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0e 	vcmpgtss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0e 	vcmpgtss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0e 	vcmpgtss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0e 	vcmpgtss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0e 	vcmpgtss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0e 	vcmpgtss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0e 	vcmpgtss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0e 	vcmpgtss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0e 	vcmpgtss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0e 	vcmpgtss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0e 	vcmpgtss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0e 	vcmpgtss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0e 	vcmpgtss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0e 	vcmpgtss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0f 	vcmptruess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0f 	vcmptruess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0f 	vcmptruess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0f 	vcmptruess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0f 	vcmptruess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0f 	vcmptruess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0f 	vcmptruess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0f 	vcmptruess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 0f 	vcmptruess %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 0f 	vcmptruess \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 0f 	vcmptruess \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 0f 	vcmptruess -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 0f 	vcmptruess 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 0f 	vcmptruess 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 0f 	vcmptruess -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 0f 	vcmptruess -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 10 	vcmpeq_osss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 10 	vcmpeq_osss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 10 	vcmpeq_osss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 10 	vcmpeq_osss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 10 	vcmpeq_osss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 10 	vcmpeq_osss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 10 	vcmpeq_osss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 10 	vcmpeq_osss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 11 	vcmplt_oqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 11 	vcmplt_oqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 11 	vcmplt_oqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 11 	vcmplt_oqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 11 	vcmplt_oqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 11 	vcmplt_oqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 11 	vcmplt_oqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 11 	vcmplt_oqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 12 	vcmple_oqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 12 	vcmple_oqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 12 	vcmple_oqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 12 	vcmple_oqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 12 	vcmple_oqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 12 	vcmple_oqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 12 	vcmple_oqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 12 	vcmple_oqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 13 	vcmpunord_sss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 13 	vcmpunord_sss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 13 	vcmpunord_sss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 13 	vcmpunord_sss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 13 	vcmpunord_sss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 13 	vcmpunord_sss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 13 	vcmpunord_sss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 13 	vcmpunord_sss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 14 	vcmpneq_usss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 14 	vcmpneq_usss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 14 	vcmpneq_usss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 14 	vcmpneq_usss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 14 	vcmpneq_usss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 14 	vcmpneq_usss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 14 	vcmpneq_usss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 14 	vcmpneq_usss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 15 	vcmpnlt_uqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 15 	vcmpnlt_uqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 15 	vcmpnlt_uqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 15 	vcmpnlt_uqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 15 	vcmpnlt_uqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 15 	vcmpnlt_uqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 15 	vcmpnlt_uqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 15 	vcmpnlt_uqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 16 	vcmpnle_uqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 16 	vcmpnle_uqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 16 	vcmpnle_uqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 16 	vcmpnle_uqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 16 	vcmpnle_uqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 16 	vcmpnle_uqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 16 	vcmpnle_uqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 16 	vcmpnle_uqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 17 	vcmpord_sss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 17 	vcmpord_sss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 17 	vcmpord_sss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 17 	vcmpord_sss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 17 	vcmpord_sss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 17 	vcmpord_sss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 17 	vcmpord_sss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 17 	vcmpord_sss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 18 	vcmpeq_usss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 18 	vcmpeq_usss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 18 	vcmpeq_usss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 18 	vcmpeq_usss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 18 	vcmpeq_usss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 18 	vcmpeq_usss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 18 	vcmpeq_usss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 18 	vcmpeq_usss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 19 	vcmpnge_uqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 19 	vcmpnge_uqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 19 	vcmpnge_uqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 19 	vcmpnge_uqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 19 	vcmpnge_uqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 19 	vcmpnge_uqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 19 	vcmpnge_uqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 19 	vcmpnge_uqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 1a 	vcmpngt_uqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1a 	vcmpngt_uqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 1a 	vcmpngt_uqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 1a 	vcmpngt_uqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 1a 	vcmpngt_uqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 1a 	vcmpngt_uqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 1a 	vcmpngt_uqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 1a 	vcmpngt_uqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 1b 	vcmpfalse_osss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1b 	vcmpfalse_osss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 1b 	vcmpfalse_osss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 1b 	vcmpfalse_osss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 1b 	vcmpfalse_osss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 1b 	vcmpfalse_osss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 1b 	vcmpfalse_osss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 1b 	vcmpfalse_osss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 1c 	vcmpneq_osss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1c 	vcmpneq_osss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 1c 	vcmpneq_osss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 1c 	vcmpneq_osss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 1c 	vcmpneq_osss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 1c 	vcmpneq_osss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 1c 	vcmpneq_osss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 1c 	vcmpneq_osss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 1d 	vcmpge_oqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1d 	vcmpge_oqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 1d 	vcmpge_oqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 1d 	vcmpge_oqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 1d 	vcmpge_oqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 1d 	vcmpge_oqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 1d 	vcmpge_oqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 1d 	vcmpge_oqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 1e 	vcmpgt_oqss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1e 	vcmpgt_oqss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 1e 	vcmpgt_oqss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 1e 	vcmpgt_oqss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 1e 	vcmpgt_oqss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 1e 	vcmpgt_oqss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 1e 	vcmpgt_oqss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 1e 	vcmpgt_oqss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ec 1f 	vcmptrue_usss %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 1f c2 ec 1f 	vcmptrue_usss \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 29 1f 	vcmptrue_usss \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 ac f4 c0 1d fe ff 1f 	vcmptrue_usss -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 7f 1f 	vcmptrue_usss 0x1fc\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa 00 02 00 00 1f 	vcmptrue_usss 0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 6a 80 1f 	vcmptrue_usss -0x200\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f c2 aa fc fd ff ff 1f 	vcmptrue_usss -0x204\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 fd 18 2f f5    	vcomisd \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 7c 18 2f f5    	vcomiss \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 7f 18 2d c6    	vcvtsd2si \{rn-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 58 2d c6    	vcvtsd2si \{ru-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 38 2d c6    	vcvtsd2si \{rd-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 78 2d c6    	vcvtsd2si \{rz-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 18 2d ee    	vcvtsd2si \{rn-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 58 2d ee    	vcvtsd2si \{ru-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 38 2d ee    	vcvtsd2si \{rd-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 78 2d ee    	vcvtsd2si \{rz-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a f4    	vcvtsd2ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 5a f4    	vcvtsd2ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5a f4    	vcvtsd2ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 5a f4    	vcvtsd2ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 5a f4    	vcvtsd2ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 5a f4    	vcvtsd2ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a 31    	vcvtsd2ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a b4 f4 c0 1d fe ff 	vcvtsd2ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a 72 7f 	vcvtsd2ss 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a b2 00 04 00 00 	vcvtsd2ss 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a 72 80 	vcvtsd2ss -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5a b2 f8 fb ff ff 	vcvtsd2ss -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 18 2a f0    	vcvtsi2ss %eax,\{rn-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 58 2a f0    	vcvtsi2ss %eax,\{ru-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 38 2a f0    	vcvtsi2ss %eax,\{rd-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 78 2a f0    	vcvtsi2ss %eax,\{rz-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 18 2a f5    	vcvtsi2ss %ebp,\{rn-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 58 2a f5    	vcvtsi2ss %ebp,\{ru-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 38 2a f5    	vcvtsi2ss %ebp,\{rd-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 78 2a f5    	vcvtsi2ss %ebp,\{rz-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a f4    	vcvtss2sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 5a f4    	vcvtss2sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 5a f4    	vcvtss2sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a 31    	vcvtss2sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a b4 f4 c0 1d fe ff 	vcvtss2sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a 72 7f 	vcvtss2sd 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a b2 00 02 00 00 	vcvtss2sd 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a 72 80 	vcvtss2sd -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5a b2 fc fd ff ff 	vcvtss2sd -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 18 2d c6    	vcvtss2si \{rn-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 58 2d c6    	vcvtss2si \{ru-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 38 2d c6    	vcvtss2si \{rd-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 78 2d c6    	vcvtss2si \{rz-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 18 2d ee    	vcvtss2si \{rn-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 58 2d ee    	vcvtss2si \{ru-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 38 2d ee    	vcvtss2si \{rd-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 78 2d ee    	vcvtss2si \{rz-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 18 2c c6    	vcvttsd2si \{sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 18 2c ee    	vcvttsd2si \{sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 18 2c c6    	vcvttss2si \{sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 18 2c ee    	vcvttss2si \{sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e f4    	vdivsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 5e f4    	vdivsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5e f4    	vdivsd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 5e f4    	vdivsd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 5e f4    	vdivsd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 5e f4    	vdivsd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e 31    	vdivsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e b4 f4 c0 1d fe ff 	vdivsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e 72 7f 	vdivsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e b2 00 04 00 00 	vdivsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e 72 80 	vdivsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5e b2 f8 fb ff ff 	vdivsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e f4    	vdivss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 5e f4    	vdivss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 5e f4    	vdivss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 5e f4    	vdivss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 5e f4    	vdivss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 5e f4    	vdivss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e 31    	vdivss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e b4 f4 c0 1d fe ff 	vdivss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e 72 7f 	vdivss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e b2 00 02 00 00 	vdivss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e 72 80 	vdivss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5e b2 fc fd ff ff 	vdivss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 f4    	vfmadd132sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 99 f4    	vfmadd132sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f 99 f4    	vfmadd132sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 99 f4    	vfmadd132sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 99 f4    	vfmadd132sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 99 f4    	vfmadd132sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 31    	vfmadd132sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 b4 f4 c0 1d fe ff 	vfmadd132sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 72 7f 	vfmadd132sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 b2 00 04 00 00 	vfmadd132sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 72 80 	vfmadd132sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 99 b2 f8 fb ff ff 	vfmadd132sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 f4    	vfmadd132ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 99 f4    	vfmadd132ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f 99 f4    	vfmadd132ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 99 f4    	vfmadd132ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 99 f4    	vfmadd132ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 99 f4    	vfmadd132ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 31    	vfmadd132ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 b4 f4 c0 1d fe ff 	vfmadd132ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 72 7f 	vfmadd132ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 b2 00 02 00 00 	vfmadd132ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 72 80 	vfmadd132ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 99 b2 fc fd ff ff 	vfmadd132ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 f4    	vfmadd213sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf a9 f4    	vfmadd213sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f a9 f4    	vfmadd213sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f a9 f4    	vfmadd213sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f a9 f4    	vfmadd213sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f a9 f4    	vfmadd213sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 31    	vfmadd213sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 b4 f4 c0 1d fe ff 	vfmadd213sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 72 7f 	vfmadd213sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 b2 00 04 00 00 	vfmadd213sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 72 80 	vfmadd213sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f a9 b2 f8 fb ff ff 	vfmadd213sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 f4    	vfmadd213ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf a9 f4    	vfmadd213ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f a9 f4    	vfmadd213ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f a9 f4    	vfmadd213ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f a9 f4    	vfmadd213ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f a9 f4    	vfmadd213ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 31    	vfmadd213ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 b4 f4 c0 1d fe ff 	vfmadd213ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 72 7f 	vfmadd213ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 b2 00 02 00 00 	vfmadd213ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 72 80 	vfmadd213ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f a9 b2 fc fd ff ff 	vfmadd213ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 f4    	vfmadd231sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf b9 f4    	vfmadd231sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f b9 f4    	vfmadd231sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f b9 f4    	vfmadd231sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f b9 f4    	vfmadd231sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f b9 f4    	vfmadd231sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 31    	vfmadd231sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 b4 f4 c0 1d fe ff 	vfmadd231sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 72 7f 	vfmadd231sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 b2 00 04 00 00 	vfmadd231sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 72 80 	vfmadd231sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f b9 b2 f8 fb ff ff 	vfmadd231sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 f4    	vfmadd231ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf b9 f4    	vfmadd231ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f b9 f4    	vfmadd231ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f b9 f4    	vfmadd231ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f b9 f4    	vfmadd231ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f b9 f4    	vfmadd231ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 31    	vfmadd231ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 b4 f4 c0 1d fe ff 	vfmadd231ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 72 7f 	vfmadd231ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 b2 00 02 00 00 	vfmadd231ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 72 80 	vfmadd231ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f b9 b2 fc fd ff ff 	vfmadd231ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b f4    	vfmsub132sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 9b f4    	vfmsub132sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f 9b f4    	vfmsub132sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 9b f4    	vfmsub132sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 9b f4    	vfmsub132sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 9b f4    	vfmsub132sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b 31    	vfmsub132sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b b4 f4 c0 1d fe ff 	vfmsub132sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b 72 7f 	vfmsub132sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b b2 00 04 00 00 	vfmsub132sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b 72 80 	vfmsub132sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9b b2 f8 fb ff ff 	vfmsub132sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b f4    	vfmsub132ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 9b f4    	vfmsub132ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f 9b f4    	vfmsub132ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 9b f4    	vfmsub132ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 9b f4    	vfmsub132ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 9b f4    	vfmsub132ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b 31    	vfmsub132ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b b4 f4 c0 1d fe ff 	vfmsub132ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b 72 7f 	vfmsub132ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b b2 00 02 00 00 	vfmsub132ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b 72 80 	vfmsub132ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9b b2 fc fd ff ff 	vfmsub132ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab f4    	vfmsub213sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf ab f4    	vfmsub213sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f ab f4    	vfmsub213sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f ab f4    	vfmsub213sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f ab f4    	vfmsub213sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f ab f4    	vfmsub213sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab 31    	vfmsub213sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab b4 f4 c0 1d fe ff 	vfmsub213sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab 72 7f 	vfmsub213sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab b2 00 04 00 00 	vfmsub213sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab 72 80 	vfmsub213sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ab b2 f8 fb ff ff 	vfmsub213sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab f4    	vfmsub213ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf ab f4    	vfmsub213ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f ab f4    	vfmsub213ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f ab f4    	vfmsub213ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f ab f4    	vfmsub213ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f ab f4    	vfmsub213ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab 31    	vfmsub213ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab b4 f4 c0 1d fe ff 	vfmsub213ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab 72 7f 	vfmsub213ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab b2 00 02 00 00 	vfmsub213ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab 72 80 	vfmsub213ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ab b2 fc fd ff ff 	vfmsub213ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb f4    	vfmsub231sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf bb f4    	vfmsub231sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f bb f4    	vfmsub231sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f bb f4    	vfmsub231sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f bb f4    	vfmsub231sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f bb f4    	vfmsub231sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb 31    	vfmsub231sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb b4 f4 c0 1d fe ff 	vfmsub231sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb 72 7f 	vfmsub231sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb b2 00 04 00 00 	vfmsub231sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb 72 80 	vfmsub231sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bb b2 f8 fb ff ff 	vfmsub231sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb f4    	vfmsub231ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf bb f4    	vfmsub231ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f bb f4    	vfmsub231ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f bb f4    	vfmsub231ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f bb f4    	vfmsub231ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f bb f4    	vfmsub231ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb 31    	vfmsub231ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb b4 f4 c0 1d fe ff 	vfmsub231ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb 72 7f 	vfmsub231ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb b2 00 02 00 00 	vfmsub231ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb 72 80 	vfmsub231ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bb b2 fc fd ff ff 	vfmsub231ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d f4    	vfnmadd132sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 9d f4    	vfnmadd132sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f 9d f4    	vfnmadd132sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 9d f4    	vfnmadd132sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 9d f4    	vfnmadd132sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 9d f4    	vfnmadd132sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d 31    	vfnmadd132sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d b4 f4 c0 1d fe ff 	vfnmadd132sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d 72 7f 	vfnmadd132sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d b2 00 04 00 00 	vfnmadd132sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d 72 80 	vfnmadd132sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9d b2 f8 fb ff ff 	vfnmadd132sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d f4    	vfnmadd132ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 9d f4    	vfnmadd132ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f 9d f4    	vfnmadd132ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 9d f4    	vfnmadd132ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 9d f4    	vfnmadd132ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 9d f4    	vfnmadd132ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d 31    	vfnmadd132ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d b4 f4 c0 1d fe ff 	vfnmadd132ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d 72 7f 	vfnmadd132ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d b2 00 02 00 00 	vfnmadd132ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d 72 80 	vfnmadd132ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9d b2 fc fd ff ff 	vfnmadd132ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad f4    	vfnmadd213sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf ad f4    	vfnmadd213sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f ad f4    	vfnmadd213sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f ad f4    	vfnmadd213sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f ad f4    	vfnmadd213sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f ad f4    	vfnmadd213sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad 31    	vfnmadd213sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad b4 f4 c0 1d fe ff 	vfnmadd213sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad 72 7f 	vfnmadd213sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad b2 00 04 00 00 	vfnmadd213sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad 72 80 	vfnmadd213sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f ad b2 f8 fb ff ff 	vfnmadd213sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad f4    	vfnmadd213ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf ad f4    	vfnmadd213ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f ad f4    	vfnmadd213ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f ad f4    	vfnmadd213ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f ad f4    	vfnmadd213ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f ad f4    	vfnmadd213ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad 31    	vfnmadd213ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad b4 f4 c0 1d fe ff 	vfnmadd213ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad 72 7f 	vfnmadd213ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad b2 00 02 00 00 	vfnmadd213ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad 72 80 	vfnmadd213ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f ad b2 fc fd ff ff 	vfnmadd213ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd f4    	vfnmadd231sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf bd f4    	vfnmadd231sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f bd f4    	vfnmadd231sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f bd f4    	vfnmadd231sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f bd f4    	vfnmadd231sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f bd f4    	vfnmadd231sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd 31    	vfnmadd231sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd b4 f4 c0 1d fe ff 	vfnmadd231sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd 72 7f 	vfnmadd231sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd b2 00 04 00 00 	vfnmadd231sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd 72 80 	vfnmadd231sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bd b2 f8 fb ff ff 	vfnmadd231sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd f4    	vfnmadd231ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf bd f4    	vfnmadd231ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f bd f4    	vfnmadd231ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f bd f4    	vfnmadd231ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f bd f4    	vfnmadd231ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f bd f4    	vfnmadd231ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd 31    	vfnmadd231ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd b4 f4 c0 1d fe ff 	vfnmadd231ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd 72 7f 	vfnmadd231ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd b2 00 02 00 00 	vfnmadd231ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd 72 80 	vfnmadd231ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bd b2 fc fd ff ff 	vfnmadd231ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f f4    	vfnmsub132sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 9f f4    	vfnmsub132sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f 9f f4    	vfnmsub132sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 9f f4    	vfnmsub132sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 9f f4    	vfnmsub132sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 9f f4    	vfnmsub132sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f 31    	vfnmsub132sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f b4 f4 c0 1d fe ff 	vfnmsub132sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f 72 7f 	vfnmsub132sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f b2 00 04 00 00 	vfnmsub132sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f 72 80 	vfnmsub132sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 9f b2 f8 fb ff ff 	vfnmsub132sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f f4    	vfnmsub132ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 9f f4    	vfnmsub132ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f 9f f4    	vfnmsub132ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 9f f4    	vfnmsub132ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 9f f4    	vfnmsub132ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 9f f4    	vfnmsub132ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f 31    	vfnmsub132ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f b4 f4 c0 1d fe ff 	vfnmsub132ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f 72 7f 	vfnmsub132ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f b2 00 02 00 00 	vfnmsub132ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f 72 80 	vfnmsub132ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 9f b2 fc fd ff ff 	vfnmsub132ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af f4    	vfnmsub213sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf af f4    	vfnmsub213sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f af f4    	vfnmsub213sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f af f4    	vfnmsub213sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f af f4    	vfnmsub213sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f af f4    	vfnmsub213sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af 31    	vfnmsub213sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af b4 f4 c0 1d fe ff 	vfnmsub213sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af 72 7f 	vfnmsub213sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af b2 00 04 00 00 	vfnmsub213sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af 72 80 	vfnmsub213sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f af b2 f8 fb ff ff 	vfnmsub213sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af f4    	vfnmsub213ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf af f4    	vfnmsub213ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f af f4    	vfnmsub213ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f af f4    	vfnmsub213ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f af f4    	vfnmsub213ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f af f4    	vfnmsub213ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af 31    	vfnmsub213ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af b4 f4 c0 1d fe ff 	vfnmsub213ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af 72 7f 	vfnmsub213ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af b2 00 02 00 00 	vfnmsub213ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af 72 80 	vfnmsub213ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f af b2 fc fd ff ff 	vfnmsub213ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf f4    	vfnmsub231sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf bf f4    	vfnmsub231sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f bf f4    	vfnmsub231sd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f bf f4    	vfnmsub231sd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f bf f4    	vfnmsub231sd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f bf f4    	vfnmsub231sd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf 31    	vfnmsub231sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf b4 f4 c0 1d fe ff 	vfnmsub231sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf 72 7f 	vfnmsub231sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf b2 00 04 00 00 	vfnmsub231sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf 72 80 	vfnmsub231sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f bf b2 f8 fb ff ff 	vfnmsub231sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf f4    	vfnmsub231ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf bf f4    	vfnmsub231ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f bf f4    	vfnmsub231ss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f bf f4    	vfnmsub231ss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f bf f4    	vfnmsub231ss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f bf f4    	vfnmsub231ss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf 31    	vfnmsub231ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf b4 f4 c0 1d fe ff 	vfnmsub231ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf 72 7f 	vfnmsub231ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf b2 00 02 00 00 	vfnmsub231ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf 72 80 	vfnmsub231ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f bf b2 fc fd ff ff 	vfnmsub231ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 f4    	vgetexpsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 43 f4    	vgetexpsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f 43 f4    	vgetexpsd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 31    	vgetexpsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 b4 f4 c0 1d fe ff 	vgetexpsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 72 7f 	vgetexpsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 b2 00 04 00 00 	vgetexpsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 72 80 	vgetexpsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 43 b2 f8 fb ff ff 	vgetexpsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 f4    	vgetexpss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 43 f4    	vgetexpss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f 43 f4    	vgetexpss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 31    	vgetexpss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 b4 f4 c0 1d fe ff 	vgetexpss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 72 7f 	vgetexpss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 b2 00 02 00 00 	vgetexpss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 72 80 	vgetexpss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 43 b2 fc fd ff ff 	vgetexpss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 f4 ab 	vgetmantsd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 cf 27 f4 ab 	vgetmantsd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f3 d5 1f 27 f4 ab 	vgetmantsd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 f4 7b 	vgetmantsd \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 1f 27 f4 7b 	vgetmantsd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 31 7b 	vgetmantsd \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 b4 f4 c0 1d fe ff 7b 	vgetmantsd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 72 7f 7b 	vgetmantsd \$0x7b,0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 b2 00 04 00 00 7b 	vgetmantsd \$0x7b,0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 72 80 7b 	vgetmantsd \$0x7b,-0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 27 b2 f8 fb ff ff 7b 	vgetmantsd \$0x7b,-0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 f4 ab 	vgetmantss \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 cf 27 f4 ab 	vgetmantss \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f3 55 1f 27 f4 ab 	vgetmantss \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 f4 7b 	vgetmantss \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 1f 27 f4 7b 	vgetmantss \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 31 7b 	vgetmantss \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 b4 f4 c0 1d fe ff 7b 	vgetmantss \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 72 7f 7b 	vgetmantss \$0x7b,0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 b2 00 02 00 00 7b 	vgetmantss \$0x7b,0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 72 80 7b 	vgetmantss \$0x7b,-0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 27 b2 fc fd ff ff 7b 	vgetmantss \$0x7b,-0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f f4    	vmaxsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 5f f4    	vmaxsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5f f4    	vmaxsd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f 31    	vmaxsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f b4 f4 c0 1d fe ff 	vmaxsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f 72 7f 	vmaxsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f b2 00 04 00 00 	vmaxsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f 72 80 	vmaxsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5f b2 f8 fb ff ff 	vmaxsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f f4    	vmaxss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 5f f4    	vmaxss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 5f f4    	vmaxss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f 31    	vmaxss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f b4 f4 c0 1d fe ff 	vmaxss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f 72 7f 	vmaxss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f b2 00 02 00 00 	vmaxss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f 72 80 	vmaxss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5f b2 fc fd ff ff 	vmaxss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d f4    	vminsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 5d f4    	vminsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5d f4    	vminsd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d 31    	vminsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d b4 f4 c0 1d fe ff 	vminsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d 72 7f 	vminsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d b2 00 04 00 00 	vminsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d 72 80 	vminsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5d b2 f8 fb ff ff 	vminsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d f4    	vminss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 5d f4    	vminss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 5d f4    	vminss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d 31    	vminss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d b4 f4 c0 1d fe ff 	vminss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d 72 7f 	vminss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d b2 00 02 00 00 	vminss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d 72 80 	vminss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5d b2 fc fd ff ff 	vminss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 10 31    	vmovsd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff cf 10 31    	vmovsd \(%ecx\),%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 10 b4 f4 c0 1d fe ff 	vmovsd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 10 72 7f 	vmovsd 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 10 b2 00 04 00 00 	vmovsd 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 10 72 80 	vmovsd -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 10 b2 f8 fb ff ff 	vmovsd -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 11 31    	vmovsd %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 11 b4 f4 c0 1d fe ff 	vmovsd %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 11 72 7f 	vmovsd %xmm6,0x3f8\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 11 b2 00 04 00 00 	vmovsd %xmm6,0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 11 72 80 	vmovsd %xmm6,-0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 4f 11 b2 f8 fb ff ff 	vmovsd %xmm6,-0x408\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 10 f4    	vmovsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 10 f4    	vmovsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 10 31    	vmovss \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e cf 10 31    	vmovss \(%ecx\),%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 10 b4 f4 c0 1d fe ff 	vmovss -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 10 72 7f 	vmovss 0x1fc\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 10 b2 00 02 00 00 	vmovss 0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 10 72 80 	vmovss -0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 10 b2 fc fd ff ff 	vmovss -0x204\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 11 31    	vmovss %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 11 b4 f4 c0 1d fe ff 	vmovss %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 11 72 7f 	vmovss %xmm6,0x1fc\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 11 b2 00 02 00 00 	vmovss %xmm6,0x200\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 11 72 80 	vmovss %xmm6,-0x200\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 4f 11 b2 fc fd ff ff 	vmovss %xmm6,-0x204\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 10 f4    	vmovss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 10 f4    	vmovss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 f4    	vmulsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 59 f4    	vmulsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 59 f4    	vmulsd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 59 f4    	vmulsd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 59 f4    	vmulsd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 59 f4    	vmulsd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 31    	vmulsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 b4 f4 c0 1d fe ff 	vmulsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 72 7f 	vmulsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 b2 00 04 00 00 	vmulsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 72 80 	vmulsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 59 b2 f8 fb ff ff 	vmulsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 f4    	vmulss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 59 f4    	vmulss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 59 f4    	vmulss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 59 f4    	vmulss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 59 f4    	vmulss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 59 f4    	vmulss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 31    	vmulss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 b4 f4 c0 1d fe ff 	vmulss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 72 7f 	vmulss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 b2 00 02 00 00 	vmulss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 72 80 	vmulss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 59 b2 fc fd ff ff 	vmulss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d f4    	vrcp14sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 4d f4    	vrcp14sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d 31    	vrcp14sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d b4 f4 c0 1d fe ff 	vrcp14sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d 72 7f 	vrcp14sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d b2 00 04 00 00 	vrcp14sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d 72 80 	vrcp14sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4d b2 f8 fb ff ff 	vrcp14sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d f4    	vrcp14ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 4d f4    	vrcp14ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d 31    	vrcp14ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d b4 f4 c0 1d fe ff 	vrcp14ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d 72 7f 	vrcp14ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d b2 00 02 00 00 	vrcp14ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d 72 80 	vrcp14ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4d b2 fc fd ff ff 	vrcp14ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb f4    	vrcp28ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf cb f4    	vrcp28ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f cb f4    	vrcp28ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb 31    	vrcp28ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb b4 f4 c0 1d fe ff 	vrcp28ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb 72 7f 	vrcp28ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb b2 00 02 00 00 	vrcp28ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb 72 80 	vrcp28ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cb b2 fc fd ff ff 	vrcp28ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb f4    	vrcp28sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf cb f4    	vrcp28sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f cb f4    	vrcp28sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb 31    	vrcp28sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb b4 f4 c0 1d fe ff 	vrcp28sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb 72 7f 	vrcp28sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb b2 00 04 00 00 	vrcp28sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb 72 80 	vrcp28sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cb b2 f8 fb ff ff 	vrcp28sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f f4    	vrsqrt14sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 4f f4    	vrsqrt14sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f 31    	vrsqrt14sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f b4 f4 c0 1d fe ff 	vrsqrt14sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f 72 7f 	vrsqrt14sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f b2 00 04 00 00 	vrsqrt14sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f 72 80 	vrsqrt14sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 4f b2 f8 fb ff ff 	vrsqrt14sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f f4    	vrsqrt14ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 4f f4    	vrsqrt14ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f 31    	vrsqrt14ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f b4 f4 c0 1d fe ff 	vrsqrt14ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f 72 7f 	vrsqrt14ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f b2 00 02 00 00 	vrsqrt14ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f 72 80 	vrsqrt14ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 4f b2 fc fd ff ff 	vrsqrt14ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd f4    	vrsqrt28ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf cd f4    	vrsqrt28ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f cd f4    	vrsqrt28ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd 31    	vrsqrt28ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd b4 f4 c0 1d fe ff 	vrsqrt28ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd 72 7f 	vrsqrt28ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd b2 00 02 00 00 	vrsqrt28ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd 72 80 	vrsqrt28ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f cd b2 fc fd ff ff 	vrsqrt28ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd f4    	vrsqrt28sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf cd f4    	vrsqrt28sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f cd f4    	vrsqrt28sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd 31    	vrsqrt28sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd b4 f4 c0 1d fe ff 	vrsqrt28sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd 72 7f 	vrsqrt28sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd b2 00 04 00 00 	vrsqrt28sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd 72 80 	vrsqrt28sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f cd b2 f8 fb ff ff 	vrsqrt28sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 f4    	vsqrtsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 51 f4    	vsqrtsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 51 f4    	vsqrtsd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 51 f4    	vsqrtsd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 51 f4    	vsqrtsd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 51 f4    	vsqrtsd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 31    	vsqrtsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 b4 f4 c0 1d fe ff 	vsqrtsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 72 7f 	vsqrtsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 b2 00 04 00 00 	vsqrtsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 72 80 	vsqrtsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 51 b2 f8 fb ff ff 	vsqrtsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 f4    	vsqrtss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 51 f4    	vsqrtss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 51 f4    	vsqrtss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 51 f4    	vsqrtss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 51 f4    	vsqrtss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 51 f4    	vsqrtss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 31    	vsqrtss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 b4 f4 c0 1d fe ff 	vsqrtss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 72 7f 	vsqrtss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 b2 00 02 00 00 	vsqrtss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 72 80 	vsqrtss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 51 b2 fc fd ff ff 	vsqrtss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c f4    	vsubsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 cf 5c f4    	vsubsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 d7 1f 5c f4    	vsubsd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 5f 5c f4    	vsubsd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 3f 5c f4    	vsubsd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 7f 5c f4    	vsubsd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c 31    	vsubsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c b4 f4 c0 1d fe ff 	vsubsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c 72 7f 	vsubsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c b2 00 04 00 00 	vsubsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c 72 80 	vsubsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 d7 4f 5c b2 f8 fb ff ff 	vsubsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c f4    	vsubss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 cf 5c f4    	vsubss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f1 56 1f 5c f4    	vsubss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 5f 5c f4    	vsubss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 3f 5c f4    	vsubss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 7f 5c f4    	vsubss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c 31    	vsubss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c b4 f4 c0 1d fe ff 	vsubss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c 72 7f 	vsubss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c b2 00 02 00 00 	vsubss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c 72 80 	vsubss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 56 4f 5c b2 fc fd ff ff 	vsubss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 fd 18 2e f5    	vucomisd \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 7c 18 2e f5    	vucomiss \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 c6    	vcvtsd2usi %xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 18 79 c6    	vcvtsd2usi \{rn-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 58 79 c6    	vcvtsd2usi \{ru-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 38 79 c6    	vcvtsd2usi \{rd-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 78 79 c6    	vcvtsd2usi \{rz-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 01    	vcvtsd2usi \(%ecx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 84 f4 c0 1d fe ff 	vcvtsd2usi -0x1e240\(%esp,%esi,8\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 42 7f 	vcvtsd2usi 0x3f8\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 82 00 04 00 00 	vcvtsd2usi 0x400\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 42 80 	vcvtsd2usi -0x400\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 82 f8 fb ff ff 	vcvtsd2usi -0x408\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 ee    	vcvtsd2usi %xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 18 79 ee    	vcvtsd2usi \{rn-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 58 79 ee    	vcvtsd2usi \{ru-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 38 79 ee    	vcvtsd2usi \{rd-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 78 79 ee    	vcvtsd2usi \{rz-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 29    	vcvtsd2usi \(%ecx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 ac f4 c0 1d fe ff 	vcvtsd2usi -0x1e240\(%esp,%esi,8\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 6a 7f 	vcvtsd2usi 0x3f8\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 aa 00 04 00 00 	vcvtsd2usi 0x400\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 6a 80 	vcvtsd2usi -0x400\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7f 48 79 aa f8 fb ff ff 	vcvtsd2usi -0x408\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 c6    	vcvtss2usi %xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 18 79 c6    	vcvtss2usi \{rn-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 58 79 c6    	vcvtss2usi \{ru-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 38 79 c6    	vcvtss2usi \{rd-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 78 79 c6    	vcvtss2usi \{rz-sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 01    	vcvtss2usi \(%ecx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 84 f4 c0 1d fe ff 	vcvtss2usi -0x1e240\(%esp,%esi,8\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 42 7f 	vcvtss2usi 0x1fc\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 82 00 02 00 00 	vcvtss2usi 0x200\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 42 80 	vcvtss2usi -0x200\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 82 fc fd ff ff 	vcvtss2usi -0x204\(%edx\),%eax
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 ee    	vcvtss2usi %xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 18 79 ee    	vcvtss2usi \{rn-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 58 79 ee    	vcvtss2usi \{ru-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 38 79 ee    	vcvtss2usi \{rd-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 78 79 ee    	vcvtss2usi \{rz-sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 29    	vcvtss2usi \(%ecx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 ac f4 c0 1d fe ff 	vcvtss2usi -0x1e240\(%esp,%esi,8\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 6a 7f 	vcvtss2usi 0x1fc\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 aa 00 02 00 00 	vcvtss2usi 0x200\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 6a 80 	vcvtss2usi -0x200\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 7e 48 79 aa fc fd ff ff 	vcvtss2usi -0x204\(%edx\),%ebp
[ 	]*[a-f0-9]+:	62 f1 57 48 7b f0    	vcvtusi2sd %eax,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b f5    	vcvtusi2sd %ebp,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b 31    	vcvtusi2sd \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b b4 f4 c0 1d fe ff 	vcvtusi2sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b 72 7f 	vcvtusi2sd 0x1fc\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b b2 00 02 00 00 	vcvtusi2sd 0x200\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b 72 80 	vcvtusi2sd -0x200\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 57 48 7b b2 fc fd ff ff 	vcvtusi2sd -0x204\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b f0    	vcvtusi2ss %eax,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 18 7b f0    	vcvtusi2ss %eax,\{rn-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 58 7b f0    	vcvtusi2ss %eax,\{ru-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 38 7b f0    	vcvtusi2ss %eax,\{rd-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 78 7b f0    	vcvtusi2ss %eax,\{rz-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b f5    	vcvtusi2ss %ebp,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 18 7b f5    	vcvtusi2ss %ebp,\{rn-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 58 7b f5    	vcvtusi2ss %ebp,\{ru-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 38 7b f5    	vcvtusi2ss %ebp,\{rd-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 78 7b f5    	vcvtusi2ss %ebp,\{rz-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b 31    	vcvtusi2ss \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b b4 f4 c0 1d fe ff 	vcvtusi2ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b 72 7f 	vcvtusi2ss 0x1fc\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b b2 00 02 00 00 	vcvtusi2ss 0x200\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b 72 80 	vcvtusi2ss -0x200\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f1 56 48 7b b2 fc fd ff ff 	vcvtusi2ss -0x204\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d f4    	vscalefsd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 cf 2d f4    	vscalefsd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f 2d f4    	vscalefsd \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 5f 2d f4    	vscalefsd \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 3f 2d f4    	vscalefsd \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 7f 2d f4    	vscalefsd \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d 31    	vscalefsd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d b4 f4 c0 1d fe ff 	vscalefsd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d 72 7f 	vscalefsd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d b2 00 04 00 00 	vscalefsd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d 72 80 	vscalefsd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 4f 2d b2 f8 fb ff ff 	vscalefsd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d f4    	vscalefss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 cf 2d f4    	vscalefss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f 2d f4    	vscalefss \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 5f 2d f4    	vscalefss \{ru-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 3f 2d f4    	vscalefss \{rd-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 7f 2d f4    	vscalefss \{rz-sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d 31    	vscalefss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d b4 f4 c0 1d fe ff 	vscalefss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d 72 7f 	vscalefss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d b2 00 02 00 00 	vscalefss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d 72 80 	vscalefss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 4f 2d b2 fc fd ff ff 	vscalefss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 f4 ab 	vfixupimmss \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 cf 55 f4 ab 	vfixupimmss \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f3 55 1f 55 f4 ab 	vfixupimmss \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 f4 7b 	vfixupimmss \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 1f 55 f4 7b 	vfixupimmss \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 31 7b 	vfixupimmss \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 b4 f4 c0 1d fe ff 7b 	vfixupimmss \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 72 7f 7b 	vfixupimmss \$0x7b,0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 b2 00 02 00 00 7b 	vfixupimmss \$0x7b,0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 72 80 7b 	vfixupimmss \$0x7b,-0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 55 b2 fc fd ff ff 7b 	vfixupimmss \$0x7b,-0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 f4 ab 	vfixupimmsd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 cf 55 f4 ab 	vfixupimmsd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f3 d5 1f 55 f4 ab 	vfixupimmsd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 f4 7b 	vfixupimmsd \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 1f 55 f4 7b 	vfixupimmsd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 31 7b 	vfixupimmsd \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 b4 f4 c0 1d fe ff 7b 	vfixupimmsd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 72 7f 7b 	vfixupimmsd \$0x7b,0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 b2 00 04 00 00 7b 	vfixupimmsd \$0x7b,0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 72 80 7b 	vfixupimmsd \$0x7b,-0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 55 b2 f8 fb ff ff 7b 	vfixupimmsd \$0x7b,-0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b f4 ab 	vrndscalesd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 cf 0b f4 ab 	vrndscalesd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f3 d5 1f 0b f4 ab 	vrndscalesd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b f4 7b 	vrndscalesd \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 1f 0b f4 7b 	vrndscalesd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b 31 7b 	vrndscalesd \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b b4 f4 c0 1d fe ff 7b 	vrndscalesd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b 72 7f 7b 	vrndscalesd \$0x7b,0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b b2 00 04 00 00 7b 	vrndscalesd \$0x7b,0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b 72 80 7b 	vrndscalesd \$0x7b,-0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 d5 4f 0b b2 f8 fb ff ff 7b 	vrndscalesd \$0x7b,-0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a f4 ab 	vrndscaless \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 cf 0a f4 ab 	vrndscaless \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f3 55 1f 0a f4 ab 	vrndscaless \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a f4 7b 	vrndscaless \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 1f 0a f4 7b 	vrndscaless \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a 31 7b 	vrndscaless \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a b4 f4 c0 1d fe ff 7b 	vrndscaless \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a 72 7f 7b 	vrndscaless \$0x7b,0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a b2 00 02 00 00 7b 	vrndscaless \$0x7b,0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a 72 80 7b 	vrndscaless \$0x7b,-0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 55 4f 0a b2 fc fd ff ff 7b 	vrndscaless \$0x7b,-0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 56 48 c2 ec 7b 	vcmpsh \$0x7b,%xmm4,%xmm5,%k5
[ 	]*[a-f0-9]+:	62 f3 56 1f c2 ec 7b 	vcmpsh \$0x7b,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 56 48 c2 29 7b 	vcmpsh \$0x7b,\(%ecx\),%xmm5,%k5
[ 	]*[a-f0-9]+:	62 f3 56 4f c2 ac f4 c0 1d fe ff 7b 	vcmpsh \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 56 48 c2 69 7f 7b 	vcmpsh \$0x7b,0xfe\(%ecx\),%xmm5,%k5
[ 	]*[a-f0-9]+:	62 f3 56 4f c2 6a 80 7b 	vcmpsh \$0x7b,-0x100\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 7c 48 67 ec 7b 	vfpclasssh \$0x7b,%xmm4,%k5
[ 	]*[a-f0-9]+:	62 f3 7c 48 67 29 7b 	vfpclasssh \$0x7b,\(%ecx\),%k5
[ 	]*[a-f0-9]+:	62 f3 7c 4f 67 ac f4 c0 1d fe ff 7b 	vfpclasssh \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:	62 f3 7c 48 67 69 7f 7b 	vfpclasssh \$0x7b,0xfe\(%ecx\),%k5
[ 	]*[a-f0-9]+:	62 f3 7c 4f 67 6a 80 7b 	vfpclasssh \$0x7b,-0x100\(%edx\),%k5\{%k7\}
#pass
