#as:
#objdump: -dw
#name: x86-64 BF16 insns
#source: x86-64-avx512_bf16.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 02 17 40 72 f4    	vcvtne2ps2bf16 %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 22 17 47 72 b4 f5 00 00 00 10 	vcvtne2ps2bf16 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 17 50 72 31    	vcvtne2ps2bf16 \(%r9\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 17 40 72 71 7f 	vcvtne2ps2bf16 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 17 d7 72 b2 00 e0 ff ff 	vcvtne2ps2bf16 -0x2000\(%rdx\)\{1to16\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 7e 48 72 f5    	vcvtneps2bf16 %zmm29,%ymm30
[ 	]*[a-f0-9]+:	62 22 7e 4f 72 b4 f5 00 00 00 10 	vcvtneps2bf16 0x10000000\(%rbp,%r14,8\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 7e 58 72 31    	vcvtneps2bf16 \(%r9\)\{1to16\},%ymm30
[ 	]*[a-f0-9]+:	62 62 7e 48 72 71 7f 	vcvtneps2bf16 0x1fc0\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:	62 62 7e df 72 b2 00 e0 ff ff 	vcvtneps2bf16 -0x2000\(%rdx\)\{1to16\},%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 16 40 52 f4    	vdpbf16ps %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 22 16 47 52 b4 f5 00 00 00 10 	vdpbf16ps 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 16 50 52 31    	vdpbf16ps \(%r9\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 16 40 52 71 7f 	vdpbf16ps 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 16 d7 52 b2 00 e0 ff ff 	vdpbf16ps -0x2000\(%rdx\)\{1to16\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 17 40 72 f4    	vcvtne2ps2bf16 %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 22 17 47 72 b4 f5 00 00 00 10 	vcvtne2ps2bf16 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 17 50 72 31    	vcvtne2ps2bf16 \(%r9\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 17 40 72 71 7f 	vcvtne2ps2bf16 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 17 d7 72 b2 00 e0 ff ff 	vcvtne2ps2bf16 -0x2000\(%rdx\)\{1to16\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 7e 48 72 f5    	vcvtneps2bf16 %zmm29,%ymm30
[ 	]*[a-f0-9]+:	62 22 7e 4f 72 b4 f5 00 00 00 10 	vcvtneps2bf16 0x10000000\(%rbp,%r14,8\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 7e 58 72 31    	vcvtneps2bf16 \(%r9\)\{1to16\},%ymm30
[ 	]*[a-f0-9]+:	62 62 7e 48 72 71 7f 	vcvtneps2bf16 0x1fc0\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:	62 62 7e df 72 b2 00 e0 ff ff 	vcvtneps2bf16 -0x2000\(%rdx\)\{1to16\},%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 16 40 52 f4    	vdpbf16ps %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 22 16 47 52 b4 f5 00 00 00 10 	vdpbf16ps 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 42 16 50 52 31    	vdpbf16ps \(%r9\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 16 40 52 71 7f 	vdpbf16ps 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 16 d7 52 b2 00 e0 ff ff 	vdpbf16ps -0x2000\(%rdx\)\{1to16\},%zmm29,%zmm30\{%k7\}\{z\}
#pass
