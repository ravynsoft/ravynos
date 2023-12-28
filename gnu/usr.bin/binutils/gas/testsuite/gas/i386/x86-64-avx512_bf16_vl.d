#as:
#objdump: -dw
#name: x86-64 BF16 VL insns
#source: x86-64-avx512_bf16_vl.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 02 17 20 72 f4    	vcvtne2ps2bf16 %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:	62 02 17 00 72 f4    	vcvtne2ps2bf16 %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 22 17 27 72 b4 f5 00 00 00 10 	vcvtne2ps2bf16 0x10000000\(%rbp,%r14,8\),%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 17 30 72 31    	vcvtne2ps2bf16 \(%r9\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:	62 62 17 20 72 71 7f 	vcvtne2ps2bf16 0xfe0\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:	62 62 17 b7 72 b2 00 f0 ff ff 	vcvtne2ps2bf16 -0x1000\(%rdx\)\{1to8\},%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 22 17 07 72 b4 f5 00 00 00 10 	vcvtne2ps2bf16 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 17 10 72 31    	vcvtne2ps2bf16 \(%r9\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 62 17 00 72 71 7f 	vcvtne2ps2bf16 0x7f0\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 62 17 97 72 a2 00 f8 ff ff 	vcvtne2ps2bf16 -0x800\(%rdx\)\{1to4\},%xmm29,%xmm28\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 7e 08 72 f5    	vcvtneps2bf16 %xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 02 7e 28 72 f5    	vcvtneps2bf16 %ymm29,%xmm30
[ 	]*[a-f0-9]+:	62 22 7e 0f 72 b4 f5 00 00 00 10 	vcvtneps2bf16x 0x10000000\(%rbp,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 c2 7e 18 72 29    	vcvtneps2bf16 \(%r9\)\{1to4\},%xmm21
[ 	]*[a-f0-9]+:	62 f2 7e 18 72 09    	vcvtneps2bf16 \(%rcx\)\{1to4\},%xmm1
[ 	]*[a-f0-9]+:	62 62 7e 08 72 71 7f 	vcvtneps2bf16x 0x7f0\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:	62 62 7e 9f 72 aa 00 f8 ff ff 	vcvtneps2bf16 -0x800\(%rdx\)\{1to4\},%xmm29\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 c2 7e 38 72 31    	vcvtneps2bf16 \(%r9\)\{1to8\},%xmm22
[ 	]*[a-f0-9]+:	62 f2 7e 38 72 11    	vcvtneps2bf16 \(%rcx\)\{1to8\},%xmm2
[ 	]*[a-f0-9]+:	62 e2 7e 28 72 79 7f 	vcvtneps2bf16y 0xfe0\(%rcx\),%xmm23
[ 	]*[a-f0-9]+:	62 62 7e bf 72 9a 00 f0 ff ff 	vcvtneps2bf16 -0x1000\(%rdx\)\{1to8\},%xmm27\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 16 20 52 f4    	vdpbf16ps %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:	62 02 16 00 52 f4    	vdpbf16ps %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 22 16 27 52 b4 f5 00 00 00 10 	vdpbf16ps 0x10000000\(%rbp,%r14,8\),%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 16 30 52 31    	vdpbf16ps \(%r9\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:	62 62 16 20 52 71 7f 	vdpbf16ps 0xfe0\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:	62 62 16 b7 52 b2 00 f0 ff ff 	vdpbf16ps -0x1000\(%rdx\)\{1to8\},%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 22 16 07 52 b4 f5 00 00 00 10 	vdpbf16ps 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 16 10 52 31    	vdpbf16ps \(%r9\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 62 16 00 52 71 7f 	vdpbf16ps 0x7f0\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 62 16 97 52 b2 00 f8 ff ff 	vdpbf16ps -0x800\(%rdx\)\{1to4\},%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 17 20 72 f4    	vcvtne2ps2bf16 %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:	62 02 17 00 72 f4    	vcvtne2ps2bf16 %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 22 17 27 72 b4 f5 00 00 00 10 	vcvtne2ps2bf16 0x10000000\(%rbp,%r14,8\),%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 17 30 72 31    	vcvtne2ps2bf16 \(%r9\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:	62 62 17 20 72 71 7f 	vcvtne2ps2bf16 0xfe0\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:	62 62 17 b7 72 b2 00 f0 ff ff 	vcvtne2ps2bf16 -0x1000\(%rdx\)\{1to8\},%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 22 17 07 72 b4 f5 00 00 00 10 	vcvtne2ps2bf16 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 17 10 72 31    	vcvtne2ps2bf16 \(%r9\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 62 17 00 72 71 7f 	vcvtne2ps2bf16 0x7f0\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 62 17 97 72 b2 00 f8 ff ff 	vcvtne2ps2bf16 -0x800\(%rdx\)\{1to4\},%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 7e 08 72 f5    	vcvtneps2bf16 %xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 02 7e 28 72 f5    	vcvtneps2bf16 %ymm29,%xmm30
[ 	]*[a-f0-9]+:	62 22 7e 0f 72 b4 f5 00 00 00 10 	vcvtneps2bf16x 0x10000000\(%rbp,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7e 18 72 29    	vcvtneps2bf16 \(%rcx\)\{1to4\},%xmm5
[ 	]*[a-f0-9]+:	62 42 7e 18 72 09    	vcvtneps2bf16 \(%r9\)\{1to4\},%xmm25
[ 	]*[a-f0-9]+:	62 62 7e 08 72 71 7f 	vcvtneps2bf16x 0x7f0\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:	62 62 7e 9f 72 b2 00 f8 ff ff 	vcvtneps2bf16 -0x800\(%rdx\)\{1to4\},%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7e 38 72 21    	vcvtneps2bf16 \(%rcx\)\{1to8\},%xmm4
[ 	]*[a-f0-9]+:	62 42 7e 38 72 01    	vcvtneps2bf16 \(%r9\)\{1to8\},%xmm24
[ 	]*[a-f0-9]+:	62 62 7e 28 72 71 7f 	vcvtneps2bf16y 0xfe0\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:	62 62 7e bf 72 b2 00 f0 ff ff 	vcvtneps2bf16 -0x1000\(%rdx\)\{1to8\},%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 16 20 52 f4    	vdpbf16ps %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:	62 02 16 00 52 f4    	vdpbf16ps %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 22 16 27 52 b4 f5 00 00 00 10 	vdpbf16ps 0x10000000\(%rbp,%r14,8\),%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 16 30 52 31    	vdpbf16ps \(%r9\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:	62 62 16 20 52 71 7f 	vdpbf16ps 0xfe0\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:	62 62 16 b7 52 b2 00 f0 ff ff 	vdpbf16ps -0x1000\(%rdx\)\{1to8\},%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 22 16 07 52 b4 f5 00 00 00 10 	vdpbf16ps 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 16 10 52 31    	vdpbf16ps \(%r9\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 62 16 00 52 71 7f 	vdpbf16ps 0x7f0\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:	62 62 16 97 52 b2 00 f8 ff ff 	vdpbf16ps -0x800\(%rdx\)\{1to4\},%xmm29,%xmm30\{%k7\}\{z\}
#pass
