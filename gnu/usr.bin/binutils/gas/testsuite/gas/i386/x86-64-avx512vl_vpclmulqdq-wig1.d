#as: -mevexwig=1
#objdump: -dw
#name: x86_64 AVX512VL/VPCLMULQDQ wig insns
#source: x86-64-avx512vl_vpclmulqdq.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 44 ca ab[ 	]*vpclmulqdq \$0xab,%xmm18,%xmm29,%xmm25
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 44 8c f0 23 01 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm25
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 44 4a 7f 7b[ 	]*vpclmulqdq \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm25
[ 	]*[a-f0-9]+:[ 	]*62 23 ed 20 44 ea ab[ 	]*vpclmulqdq \$0xab,%ymm18,%ymm18,%ymm29
[ 	]*[a-f0-9]+:[ 	]*62 23 ed 20 44 ac f0 23 01 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x123\(%rax,%r14,8\),%ymm18,%ymm29
[ 	]*[a-f0-9]+:[ 	]*62 63 ed 20 44 6a 7f 7b[ 	]*vpclmulqdq \$0x7b,0xfe0\(%rdx\),%ymm18,%ymm29
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 44 ca ab[ 	]*vpclmulqdq \$0xab,%xmm18,%xmm29,%xmm25
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 44 8c f0 23 01 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm25
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 44 4a 7f 7b[ 	]*vpclmulqdq \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm25
[ 	]*[a-f0-9]+:[ 	]*62 23 ed 20 44 ea ab[ 	]*vpclmulqdq \$0xab,%ymm18,%ymm18,%ymm29
[ 	]*[a-f0-9]+:[ 	]*62 23 ed 20 44 ac f0 23 01 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x123\(%rax,%r14,8\),%ymm18,%ymm29
[ 	]*[a-f0-9]+:[ 	]*62 63 ed 20 44 6a 7f 7b[ 	]*vpclmulqdq \$0x7b,0xfe0\(%rdx\),%ymm18,%ymm29
[ 	]*[a-f0-9]+:[ 	]*62 a3 d5 00 44 f4 11[ 	]*vpclmulhqhqdq %xmm20,%xmm21,%xmm22
[ 	]*[a-f0-9]+:[ 	]*62 a3 cd 00 44 fd 01[ 	]*vpclmulhqlqdq %xmm21,%xmm22,%xmm23
[ 	]*[a-f0-9]+:[ 	]*62 23 c5 00 44 c6 10[ 	]*vpclmullqhqdq %xmm22,%xmm23,%xmm24
[ 	]*[a-f0-9]+:[ 	]*62 23 bd 00 44 cf 00[ 	]*vpclmullqlqdq %xmm23,%xmm24,%xmm25
[ 	]*[a-f0-9]+:[ 	]*62 a3 d5 20 44 f4 11[ 	]*vpclmulhqhqdq %ymm20,%ymm21,%ymm22
[ 	]*[a-f0-9]+:[ 	]*62 a3 cd 20 44 fd 01[ 	]*vpclmulhqlqdq %ymm21,%ymm22,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 23 c5 20 44 c6 10[ 	]*vpclmullqhqdq %ymm22,%ymm23,%ymm24
[ 	]*[a-f0-9]+:[ 	]*62 23 bd 20 44 cf 00[ 	]*vpclmullqlqdq %ymm23,%ymm24,%ymm25
[ 	]*[a-f0-9]+:[ 	]*62 a3 ad 00 44 dc ab[ 	]*vpclmulqdq \$0xab,%xmm20,%xmm26,%xmm19
[ 	]*[a-f0-9]+:[ 	]*62 a3 ad 00 44 9c f0 34 12 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x1234\(%rax,%r14,8\),%xmm26,%xmm19
[ 	]*[a-f0-9]+:[ 	]*62 e3 ad 00 44 5a 7f 7b[ 	]*vpclmulqdq \$0x7b,0x7f0\(%rdx\),%xmm26,%xmm19
[ 	]*[a-f0-9]+:[ 	]*62 83 95 20 44 fb ab[ 	]*vpclmulqdq \$0xab,%ymm27,%ymm29,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 a3 95 20 44 bc f0 34 12 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 e3 95 20 44 7a 7f 7b[ 	]*vpclmulqdq \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 a3 ad 00 44 dc ab[ 	]*vpclmulqdq \$0xab,%xmm20,%xmm26,%xmm19
[ 	]*[a-f0-9]+:[ 	]*62 a3 ad 00 44 9c f0 34 12 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x1234\(%rax,%r14,8\),%xmm26,%xmm19
[ 	]*[a-f0-9]+:[ 	]*62 e3 ad 00 44 5a 7f 7b[ 	]*vpclmulqdq \$0x7b,0x7f0\(%rdx\),%xmm26,%xmm19
[ 	]*[a-f0-9]+:[ 	]*62 83 95 20 44 fb ab[ 	]*vpclmulqdq \$0xab,%ymm27,%ymm29,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 a3 95 20 44 bc f0 34 12 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 e3 95 20 44 7a 7f 7b[ 	]*vpclmulqdq \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm23
#pass
