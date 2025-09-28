#as:
#objdump: -dw
#name: i386 BF16 insns
#source: avx512_bf16.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 f2 57 48 72 f4    	vcvtne2ps2bf16 %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 57 4f 72 b4 f4 00 00 00 10 	vcvtne2ps2bf16 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 57 58 72 31    	vcvtne2ps2bf16 \(%ecx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 57 48 72 71 7f 	vcvtne2ps2bf16 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 57 df 72 b2 00 e0 ff ff 	vcvtne2ps2bf16 -0x2000\(%edx\)\{1to16\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7e 48 72 f5    	vcvtneps2bf16 %zmm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 7e 4f 72 b4 f4 00 00 00 10 	vcvtneps2bf16 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7e 58 72 31    	vcvtneps2bf16 \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:	62 f2 7e 48 72 71 7f 	vcvtneps2bf16 0x1fc0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	62 f2 7e df 72 b2 00 e0 ff ff 	vcvtneps2bf16 -0x2000\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 56 48 52 f4    	vdpbf16ps %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 56 4f 52 b4 f4 00 00 00 10 	vdpbf16ps 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 56 58 52 31    	vdpbf16ps \(%ecx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 56 48 52 71 7f 	vdpbf16ps 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 56 df 52 b2 00 e0 ff ff 	vdpbf16ps -0x2000\(%edx\)\{1to16\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 57 48 72 f4    	vcvtne2ps2bf16 %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 57 4f 72 b4 f4 00 00 00 10 	vcvtne2ps2bf16 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 57 58 72 31    	vcvtne2ps2bf16 \(%ecx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 57 48 72 71 7f 	vcvtne2ps2bf16 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 57 df 72 b2 00 e0 ff ff 	vcvtne2ps2bf16 -0x2000\(%edx\)\{1to16\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7e 48 72 f5    	vcvtneps2bf16 %zmm5,%ymm6
[ 	]*[a-f0-9]+:	62 f2 7e 4f 72 b4 f4 00 00 00 10 	vcvtneps2bf16 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7e 58 72 31    	vcvtneps2bf16 \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:	62 f2 7e 48 72 71 7f 	vcvtneps2bf16 0x1fc0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	62 f2 7e df 72 b2 00 e0 ff ff 	vcvtneps2bf16 -0x2000\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 56 48 52 f4    	vdpbf16ps %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 56 4f 52 b4 f4 00 00 00 10 	vdpbf16ps 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 56 58 52 31    	vdpbf16ps \(%ecx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 56 48 52 71 7f 	vdpbf16ps 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 56 df 52 b2 00 e0 ff ff 	vdpbf16ps -0x2000\(%edx\)\{1to16\},%zmm5,%zmm6\{%k7\}\{z\}
#pass
