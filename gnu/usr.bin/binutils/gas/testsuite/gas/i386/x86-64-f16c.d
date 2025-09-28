#objdump: -dw
#name: x86-64 F16C

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	c4 e2 7d 13 e4       	vcvtph2ps %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 42 7d 13 00       	vcvtph2ps \(%r8\),%ymm8
[ 	]*[a-f0-9]+:	c4 e2 79 13 f4       	vcvtph2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 13 21       	vcvtph2ps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e3 7d 1d e4 02    	vcvtps2ph \$0x2,%ymm4,%xmm4
[ 	]*[a-f0-9]+:	c4 43 7d 1d 00 02    	vcvtps2ph \$0x2,%ymm8,\(%r8\)
[ 	]*[a-f0-9]+:	c4 e3 79 1d e4 02    	vcvtps2ph \$0x2,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 79 1d 21 02    	vcvtps2ph \$0x2,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 7d 13 e4       	vcvtph2ps %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 42 7d 13 00       	vcvtph2ps \(%r8\),%ymm8
[ 	]*[a-f0-9]+:	c4 e2 7d 13 21       	vcvtph2ps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 79 13 f4       	vcvtph2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 13 21       	vcvtph2ps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 13 21       	vcvtph2ps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e3 7d 1d e4 02    	vcvtps2ph \$0x2,%ymm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 7d 1d 21 02    	vcvtps2ph \$0x2,%ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 7d 1d 21 02    	vcvtps2ph \$0x2,%ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 1d e4 02    	vcvtps2ph \$0x2,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 43 79 1d 00 02    	vcvtps2ph \$0x2,%xmm8,\(%r8\)
[ 	]*[a-f0-9]+:	c4 e3 79 1d 21 02    	vcvtps2ph \$0x2,%xmm4,\(%rcx\)
#pass
