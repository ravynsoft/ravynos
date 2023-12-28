#as:
#objdump: -dwMintel
#name: x86_64 AVX512PF insns (Intel disassembly)
#source: x86-64-avx512pf.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 8c fe 7b 00 00 00 	vgatherpf0dpd QWORD PTR \[r14\+ymm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 8c fe 7b 00 00 00 	vgatherpf0dpd QWORD PTR \[r14\+ymm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 4c 39 20 	vgatherpf0dpd QWORD PTR \[r9\+ymm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c6 8c b9 00 04 00 00 	vgatherpf0dpd QWORD PTR \[rcx\+ymm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 8c fe 7b 00 00 00 	vgatherpf0dps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 8c fe 7b 00 00 00 	vgatherpf0dps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 4c 39 40 	vgatherpf0dps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c6 8c b9 00 04 00 00 	vgatherpf0dps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 8c fe 7b 00 00 00 	vgatherpf0qpd QWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 8c fe 7b 00 00 00 	vgatherpf0qpd QWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 4c 39 20 	vgatherpf0qpd QWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c7 8c b9 00 04 00 00 	vgatherpf0qpd QWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 8c fe 7b 00 00 00 	vgatherpf0qps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 8c fe 7b 00 00 00 	vgatherpf0qps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 4c 39 40 	vgatherpf0qps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c7 8c b9 00 04 00 00 	vgatherpf0qps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 94 fe 7b 00 00 00 	vgatherpf1dpd QWORD PTR \[r14\+ymm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 94 fe 7b 00 00 00 	vgatherpf1dpd QWORD PTR \[r14\+ymm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 54 39 20 	vgatherpf1dpd QWORD PTR \[r9\+ymm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c6 94 b9 00 04 00 00 	vgatherpf1dpd QWORD PTR \[rcx\+ymm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 94 fe 7b 00 00 00 	vgatherpf1dps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 94 fe 7b 00 00 00 	vgatherpf1dps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 54 39 40 	vgatherpf1dps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c6 94 b9 00 04 00 00 	vgatherpf1dps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 94 fe 7b 00 00 00 	vgatherpf1qpd QWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 94 fe 7b 00 00 00 	vgatherpf1qpd QWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 54 39 20 	vgatherpf1qpd QWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c7 94 b9 00 04 00 00 	vgatherpf1qpd QWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 94 fe 7b 00 00 00 	vgatherpf1qps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 94 fe 7b 00 00 00 	vgatherpf1qps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 54 39 40 	vgatherpf1qps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c7 94 b9 00 04 00 00 	vgatherpf1qps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 ac fe 7b 00 00 00 	vscatterpf0dpd QWORD PTR \[r14\+ymm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 ac fe 7b 00 00 00 	vscatterpf0dpd QWORD PTR \[r14\+ymm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 6c 39 20 	vscatterpf0dpd QWORD PTR \[r9\+ymm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c6 ac b9 00 04 00 00 	vscatterpf0dpd QWORD PTR \[rcx\+ymm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 ac fe 7b 00 00 00 	vscatterpf0dps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 ac fe 7b 00 00 00 	vscatterpf0dps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 6c 39 40 	vscatterpf0dps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c6 ac b9 00 04 00 00 	vscatterpf0dps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 ac fe 7b 00 00 00 	vscatterpf0qpd QWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 ac fe 7b 00 00 00 	vscatterpf0qpd QWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 6c 39 20 	vscatterpf0qpd QWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c7 ac b9 00 04 00 00 	vscatterpf0qpd QWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 ac fe 7b 00 00 00 	vscatterpf0qps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 ac fe 7b 00 00 00 	vscatterpf0qps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 6c 39 40 	vscatterpf0qps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c7 ac b9 00 04 00 00 	vscatterpf0qps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 b4 fe 7b 00 00 00 	vscatterpf1dpd QWORD PTR \[r14\+ymm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 b4 fe 7b 00 00 00 	vscatterpf1dpd QWORD PTR \[r14\+ymm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 74 39 20 	vscatterpf1dpd QWORD PTR \[r9\+ymm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c6 b4 b9 00 04 00 00 	vscatterpf1dpd QWORD PTR \[rcx\+ymm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 b4 fe 7b 00 00 00 	vscatterpf1dps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 b4 fe 7b 00 00 00 	vscatterpf1dps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 74 39 40 	vscatterpf1dps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c6 b4 b9 00 04 00 00 	vscatterpf1dps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 b4 fe 7b 00 00 00 	vscatterpf1qpd QWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 b4 fe 7b 00 00 00 	vscatterpf1qpd QWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 74 39 20 	vscatterpf1qpd QWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c7 b4 b9 00 04 00 00 	vscatterpf1qpd QWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 b4 fe 7b 00 00 00 	vscatterpf1qps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 b4 fe 7b 00 00 00 	vscatterpf1qps DWORD PTR \[r14\+zmm31\*8\+0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 74 39 40 	vscatterpf1qps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c7 b4 b9 00 04 00 00 	vscatterpf1qps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 8c fe 85 ff ff ff 	vgatherpf0dpd QWORD PTR \[r14\+ymm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 8c fe 85 ff ff ff 	vgatherpf0dpd QWORD PTR \[r14\+ymm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 4c 39 20 	vgatherpf0dpd QWORD PTR \[r9\+ymm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c6 8c b9 00 04 00 00 	vgatherpf0dpd QWORD PTR \[rcx\+ymm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 8c fe 85 ff ff ff 	vgatherpf0dps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 8c fe 85 ff ff ff 	vgatherpf0dps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 4c 39 40 	vgatherpf0dps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c6 8c b9 00 04 00 00 	vgatherpf0dps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 8c fe 85 ff ff ff 	vgatherpf0qpd QWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 8c fe 85 ff ff ff 	vgatherpf0qpd QWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 4c 39 20 	vgatherpf0qpd QWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c7 8c b9 00 04 00 00 	vgatherpf0qpd QWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 8c fe 85 ff ff ff 	vgatherpf0qps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 8c fe 85 ff ff ff 	vgatherpf0qps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 4c 39 40 	vgatherpf0qps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c7 8c b9 00 04 00 00 	vgatherpf0qps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 94 fe 85 ff ff ff 	vgatherpf1dpd QWORD PTR \[r14\+ymm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 94 fe 85 ff ff ff 	vgatherpf1dpd QWORD PTR \[r14\+ymm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 54 39 20 	vgatherpf1dpd QWORD PTR \[r9\+ymm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c6 94 b9 00 04 00 00 	vgatherpf1dpd QWORD PTR \[rcx\+ymm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 94 fe 85 ff ff ff 	vgatherpf1dps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 94 fe 85 ff ff ff 	vgatherpf1dps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 54 39 40 	vgatherpf1dps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c6 94 b9 00 04 00 00 	vgatherpf1dps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 94 fe 85 ff ff ff 	vgatherpf1qpd QWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 94 fe 85 ff ff ff 	vgatherpf1qpd QWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 54 39 20 	vgatherpf1qpd QWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c7 94 b9 00 04 00 00 	vgatherpf1qpd QWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 94 fe 85 ff ff ff 	vgatherpf1qps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 94 fe 85 ff ff ff 	vgatherpf1qps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 54 39 40 	vgatherpf1qps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c7 94 b9 00 04 00 00 	vgatherpf1qps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 ac fe 85 ff ff ff 	vscatterpf0dpd QWORD PTR \[r14\+ymm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 ac fe 85 ff ff ff 	vscatterpf0dpd QWORD PTR \[r14\+ymm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 6c 39 20 	vscatterpf0dpd QWORD PTR \[r9\+ymm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c6 ac b9 00 04 00 00 	vscatterpf0dpd QWORD PTR \[rcx\+ymm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 ac fe 85 ff ff ff 	vscatterpf0dps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 ac fe 85 ff ff ff 	vscatterpf0dps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 6c 39 40 	vscatterpf0dps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c6 ac b9 00 04 00 00 	vscatterpf0dps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 ac fe 85 ff ff ff 	vscatterpf0qpd QWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 ac fe 85 ff ff ff 	vscatterpf0qpd QWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 6c 39 20 	vscatterpf0qpd QWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c7 ac b9 00 04 00 00 	vscatterpf0qpd QWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 ac fe 85 ff ff ff 	vscatterpf0qps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 ac fe 85 ff ff ff 	vscatterpf0qps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 6c 39 40 	vscatterpf0qps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c7 ac b9 00 04 00 00 	vscatterpf0qps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 b4 fe 85 ff ff ff 	vscatterpf1dpd QWORD PTR \[r14\+ymm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 b4 fe 85 ff ff ff 	vscatterpf1dpd QWORD PTR \[r14\+ymm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c6 74 39 20 	vscatterpf1dpd QWORD PTR \[r9\+ymm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c6 b4 b9 00 04 00 00 	vscatterpf1dpd QWORD PTR \[rcx\+ymm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 b4 fe 85 ff ff ff 	vscatterpf1dps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 b4 fe 85 ff ff ff 	vscatterpf1dps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c6 74 39 40 	vscatterpf1dps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c6 b4 b9 00 04 00 00 	vscatterpf1dps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 b4 fe 85 ff ff ff 	vscatterpf1qpd QWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 b4 fe 85 ff ff ff 	vscatterpf1qpd QWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 fd 41 c7 74 39 20 	vscatterpf1qpd QWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 fd 41 c7 b4 b9 00 04 00 00 	vscatterpf1qpd QWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 b4 fe 85 ff ff ff 	vscatterpf1qps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 b4 fe 85 ff ff ff 	vscatterpf1qps DWORD PTR \[r14\+zmm31\*8-0x7b\]\{k1\}
[ 	]*[a-f0-9]+:	62 92 7d 41 c7 74 39 40 	vscatterpf1qps DWORD PTR \[r9\+zmm31\*1\+0x100\]\{k1\}
[ 	]*[a-f0-9]+:	62 b2 7d 41 c7 b4 b9 00 04 00 00 	vscatterpf1qps DWORD PTR \[rcx\+zmm31\*4\+0x400\]\{k1\}
#pass
