#as:
#objdump: -dw
#name: i386 BF16 VL insns
#source: avx512_bf16_vl.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 f2 57 28 72 f4    	vcvtne2ps2bf16 %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 57 08 72 f4    	vcvtne2ps2bf16 %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 57 2f 72 b4 f4 00 00 00 10 	vcvtne2ps2bf16 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 57 38 72 31    	vcvtne2ps2bf16 \(%ecx\)\{1to8\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 57 28 72 71 7f 	vcvtne2ps2bf16 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 57 bf 72 b2 00 f0 ff ff 	vcvtne2ps2bf16 -0x1000\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 57 0f 72 b4 f4 00 00 00 10 	vcvtne2ps2bf16 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 57 18 72 31    	vcvtne2ps2bf16 \(%ecx\)\{1to4\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 57 08 72 71 7f 	vcvtne2ps2bf16 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 57 9f 72 b2 00 f8 ff ff 	vcvtne2ps2bf16 -0x800\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7e 08 72 f5    	vcvtneps2bf16 %xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 28 72 f5    	vcvtneps2bf16 %ymm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 0f 72 b4 f4 00 00 00 10 	vcvtneps2bf16x 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7e 18 72 31    	vcvtneps2bf16 \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 18 72 31    	vcvtneps2bf16 \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 08 72 71 7f 	vcvtneps2bf16x 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 9f 72 b2 00 f8 ff ff 	vcvtneps2bf16 -0x800\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7e 38 72 31    	vcvtneps2bf16 \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 38 72 31    	vcvtneps2bf16 \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 28 72 71 7f 	vcvtneps2bf16y 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e bf 72 b2 00 f0 ff ff 	vcvtneps2bf16 -0x1000\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 56 28 52 f4    	vdpbf16ps %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 56 08 52 f4    	vdpbf16ps %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 56 2f 52 b4 f4 00 00 00 10 	vdpbf16ps 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 56 38 52 31    	vdpbf16ps \(%ecx\)\{1to8\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 56 28 52 71 7f 	vdpbf16ps 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 56 bf 52 b2 00 f0 ff ff 	vdpbf16ps -0x1000\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 56 0f 52 b4 f4 00 00 00 10 	vdpbf16ps 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 56 18 52 31    	vdpbf16ps \(%ecx\)\{1to4\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 56 08 52 71 7f 	vdpbf16ps 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 56 9f 52 b2 00 f8 ff ff 	vdpbf16ps -0x800\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 57 28 72 f4    	vcvtne2ps2bf16 %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 57 08 72 f4    	vcvtne2ps2bf16 %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 57 2f 72 b4 f4 00 00 00 10 	vcvtne2ps2bf16 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 57 38 72 31    	vcvtne2ps2bf16 \(%ecx\)\{1to8\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 57 28 72 71 7f 	vcvtne2ps2bf16 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 57 bf 72 b2 00 f0 ff ff 	vcvtne2ps2bf16 -0x1000\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 57 0f 72 b4 f4 00 00 00 10 	vcvtne2ps2bf16 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 57 18 72 31    	vcvtne2ps2bf16 \(%ecx\)\{1to4\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 57 08 72 71 7f 	vcvtne2ps2bf16 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 57 9f 72 b2 00 f8 ff ff 	vcvtne2ps2bf16 -0x800\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7e 08 72 f5    	vcvtneps2bf16 %xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 28 72 f5    	vcvtneps2bf16 %ymm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 0f 72 b4 f4 00 00 00 10 	vcvtneps2bf16x 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7e 18 72 31    	vcvtneps2bf16 \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 18 72 31    	vcvtneps2bf16 \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 08 72 71 7f 	vcvtneps2bf16x 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 9f 72 b2 00 f8 ff ff 	vcvtneps2bf16 -0x800\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7e 38 72 31    	vcvtneps2bf16 \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 38 72 31    	vcvtneps2bf16 \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e 28 72 71 7f 	vcvtneps2bf16y 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	62 f2 7e bf 72 b2 00 f0 ff ff 	vcvtneps2bf16 -0x1000\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 56 28 52 f4    	vdpbf16ps %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 56 08 52 f4    	vdpbf16ps %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 56 2f 52 b4 f4 00 00 00 10 	vdpbf16ps 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 56 38 52 31    	vdpbf16ps \(%ecx\)\{1to8\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 56 28 52 71 7f 	vdpbf16ps 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 56 bf 52 b2 00 f0 ff ff 	vdpbf16ps -0x1000\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 56 0f 52 b4 f4 00 00 00 10 	vdpbf16ps 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 56 18 52 31    	vdpbf16ps \(%ecx\)\{1to4\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 56 08 52 71 7f 	vdpbf16ps 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	62 f2 56 9f 52 b2 00 f8 ff ff 	vdpbf16ps -0x800\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}\{z\}
#pass
