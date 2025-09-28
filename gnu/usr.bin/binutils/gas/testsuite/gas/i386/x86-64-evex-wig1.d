#as: -mevexwig=1
#objdump: -dw
#name: x86_64 AVX512 wig insns
#source: x86-64-evex-wig.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 63 fd 08 17 e8 ab 	vextractps \$0xab,%xmm29,%eax
[ 	]*[a-f0-9]+:	62 63 fd 08 17 e8 7b 	vextractps \$0x7b,%xmm29,%eax
[ 	]*[a-f0-9]+:	62 43 fd 08 17 e8 7b 	vextractps \$0x7b,%xmm29,%r8d
[ 	]*[a-f0-9]+:	62 63 fd 08 17 29 7b 	vextractps \$0x7b,%xmm29,\(%rcx\)
[ 	]*[a-f0-9]+:	62 23 fd 08 17 ac f0 23 01 00 00 7b 	vextractps \$0x7b,%xmm29,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:	62 63 fd 08 17 6a 7f 7b 	vextractps \$0x7b,%xmm29,0x1fc\(%rdx\)
[ 	]*[a-f0-9]+:	62 63 fd 08 17 aa 00 02 00 00 7b 	vextractps \$0x7b,%xmm29,0x200\(%rdx\)
[ 	]*[a-f0-9]+:	62 63 fd 08 17 6a 80 7b 	vextractps \$0x7b,%xmm29,-0x200\(%rdx\)
[ 	]*[a-f0-9]+:	62 63 fd 08 17 aa fc fd ff ff 7b 	vextractps \$0x7b,%xmm29,-0x204\(%rdx\)
[ 	]*[a-f0-9]+:	62 f3 fd 08 14 c0 00 	\{evex\} vpextrb \$0x0,%xmm0,%eax
[ 	]*[a-f0-9]+:	62 f3 fd 08 14 00 00 	\{evex\} vpextrb \$0x0,%xmm0,\(%rax\)
[ 	]*[a-f0-9]+:	62 f1 fd 08 c5 c0 00 	\{evex\} vpextrw \$0x0,%xmm0,%eax
[ 	]*[a-f0-9]+:	62 f3 fd 08 15 c0 00 	\{evex\} vpextrw \$0x0,%xmm0,%eax
[ 	]*[a-f0-9]+:	62 f3 fd 08 15 00 00 	\{evex\} vpextrw \$0x0,%xmm0,\(%rax\)
[ 	]*[a-f0-9]+:	62 f3 fd 08 20 c0 00 	\{evex\} vpinsrb \$0x0,%eax,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f3 fd 08 20 00 00 	\{evex\} vpinsrb \$0x0,\(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 fd 08 c4 c0 00 	\{evex\} vpinsrw \$0x0,%eax,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 fd 08 c4 00 00 	\{evex\} vpinsrw \$0x0,\(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 02 fd 4f 21 f5    	vpmovsxbd %xmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 21 f5    	vpmovsxbd %xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 31    	vpmovsxbd \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 21 b4 f0 23 01 00 00 	vpmovsxbd 0x123\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 72 7f 	vpmovsxbd 0x7f0\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 b2 00 08 00 00 	vpmovsxbd 0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 72 80 	vpmovsxbd -0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 b2 f0 f7 ff ff 	vpmovsxbd -0x810\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 22 f5    	vpmovsxbq %xmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 22 f5    	vpmovsxbq %xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 31    	vpmovsxbq \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 22 b4 f0 23 01 00 00 	vpmovsxbq 0x123\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 72 7f 	vpmovsxbq 0x3f8\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 b2 00 04 00 00 	vpmovsxbq 0x400\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 72 80 	vpmovsxbq -0x400\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 b2 f8 fb ff ff 	vpmovsxbq -0x408\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 23 f5    	vpmovsxwd %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 23 f5    	vpmovsxwd %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 31    	vpmovsxwd \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 23 b4 f0 23 01 00 00 	vpmovsxwd 0x123\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 72 7f 	vpmovsxwd 0xfe0\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 b2 00 10 00 00 	vpmovsxwd 0x1000\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 72 80 	vpmovsxwd -0x1000\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 b2 e0 ef ff ff 	vpmovsxwd -0x1020\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 24 f5    	vpmovsxwq %xmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 24 f5    	vpmovsxwq %xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 31    	vpmovsxwq \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 24 b4 f0 23 01 00 00 	vpmovsxwq 0x123\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 72 7f 	vpmovsxwq 0x7f0\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 b2 00 08 00 00 	vpmovsxwq 0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 72 80 	vpmovsxwq -0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 b2 f0 f7 ff ff 	vpmovsxwq -0x810\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 31 f5    	vpmovzxbd %xmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 31 f5    	vpmovzxbd %xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 31    	vpmovzxbd \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 31 b4 f0 23 01 00 00 	vpmovzxbd 0x123\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 72 7f 	vpmovzxbd 0x7f0\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 b2 00 08 00 00 	vpmovzxbd 0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 72 80 	vpmovzxbd -0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 b2 f0 f7 ff ff 	vpmovzxbd -0x810\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 32 f5    	vpmovzxbq %xmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 32 f5    	vpmovzxbq %xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 31    	vpmovzxbq \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 32 b4 f0 23 01 00 00 	vpmovzxbq 0x123\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 72 7f 	vpmovzxbq 0x3f8\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 b2 00 04 00 00 	vpmovzxbq 0x400\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 72 80 	vpmovzxbq -0x400\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 b2 f8 fb ff ff 	vpmovzxbq -0x408\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 33 f5    	vpmovzxwd %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 33 f5    	vpmovzxwd %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 31    	vpmovzxwd \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 33 b4 f0 23 01 00 00 	vpmovzxwd 0x123\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 72 7f 	vpmovzxwd 0xfe0\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 b2 00 10 00 00 	vpmovzxwd 0x1000\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 72 80 	vpmovzxwd -0x1000\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 b2 e0 ef ff ff 	vpmovzxwd -0x1020\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 34 f5    	vpmovzxwq %xmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 34 f5    	vpmovzxwq %xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 31    	vpmovzxwq \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 34 b4 f0 23 01 00 00 	vpmovzxwq 0x123\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 72 7f 	vpmovzxwq 0x7f0\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 b2 00 08 00 00 	vpmovzxwq 0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 72 80 	vpmovzxwq -0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 b2 f0 f7 ff ff 	vpmovzxwq -0x810\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 63 fd 08 17 e8 ab 	vextractps \$0xab,%xmm29,%eax
[ 	]*[a-f0-9]+:	62 63 fd 08 17 e8 7b 	vextractps \$0x7b,%xmm29,%eax
[ 	]*[a-f0-9]+:	62 43 fd 08 17 e8 7b 	vextractps \$0x7b,%xmm29,%r8d
[ 	]*[a-f0-9]+:	62 63 fd 08 17 29 7b 	vextractps \$0x7b,%xmm29,\(%rcx\)
[ 	]*[a-f0-9]+:	62 23 fd 08 17 ac f0 34 12 00 00 7b 	vextractps \$0x7b,%xmm29,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:	62 63 fd 08 17 6a 7f 7b 	vextractps \$0x7b,%xmm29,0x1fc\(%rdx\)
[ 	]*[a-f0-9]+:	62 63 fd 08 17 aa 00 02 00 00 7b 	vextractps \$0x7b,%xmm29,0x200\(%rdx\)
[ 	]*[a-f0-9]+:	62 63 fd 08 17 6a 80 7b 	vextractps \$0x7b,%xmm29,-0x200\(%rdx\)
[ 	]*[a-f0-9]+:	62 63 fd 08 17 aa fc fd ff ff 7b 	vextractps \$0x7b,%xmm29,-0x204\(%rdx\)
[ 	]*[a-f0-9]+:	62 02 fd 4f 21 f5    	vpmovsxbd %xmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 21 f5    	vpmovsxbd %xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 31    	vpmovsxbd \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 21 b4 f0 34 12 00 00 	vpmovsxbd 0x1234\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 72 7f 	vpmovsxbd 0x7f0\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 b2 00 08 00 00 	vpmovsxbd 0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 72 80 	vpmovsxbd -0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 b2 f0 f7 ff ff 	vpmovsxbd -0x810\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 22 f5    	vpmovsxbq %xmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 22 f5    	vpmovsxbq %xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 31    	vpmovsxbq \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 22 b4 f0 34 12 00 00 	vpmovsxbq 0x1234\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 72 7f 	vpmovsxbq 0x3f8\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 b2 00 04 00 00 	vpmovsxbq 0x400\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 72 80 	vpmovsxbq -0x400\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 b2 f8 fb ff ff 	vpmovsxbq -0x408\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 23 f5    	vpmovsxwd %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 23 f5    	vpmovsxwd %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 31    	vpmovsxwd \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 23 b4 f0 34 12 00 00 	vpmovsxwd 0x1234\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 72 7f 	vpmovsxwd 0xfe0\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 b2 00 10 00 00 	vpmovsxwd 0x1000\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 72 80 	vpmovsxwd -0x1000\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 b2 e0 ef ff ff 	vpmovsxwd -0x1020\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 24 f5    	vpmovsxwq %xmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 24 f5    	vpmovsxwq %xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 31    	vpmovsxwq \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 24 b4 f0 34 12 00 00 	vpmovsxwq 0x1234\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 72 7f 	vpmovsxwq 0x7f0\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 b2 00 08 00 00 	vpmovsxwq 0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 72 80 	vpmovsxwq -0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 b2 f0 f7 ff ff 	vpmovsxwq -0x810\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 31 f5    	vpmovzxbd %xmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 31 f5    	vpmovzxbd %xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 31    	vpmovzxbd \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 31 b4 f0 34 12 00 00 	vpmovzxbd 0x1234\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 72 7f 	vpmovzxbd 0x7f0\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 b2 00 08 00 00 	vpmovzxbd 0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 72 80 	vpmovzxbd -0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 b2 f0 f7 ff ff 	vpmovzxbd -0x810\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 32 f5    	vpmovzxbq %xmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 32 f5    	vpmovzxbq %xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 31    	vpmovzxbq \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 32 b4 f0 34 12 00 00 	vpmovzxbq 0x1234\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 72 7f 	vpmovzxbq 0x3f8\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 b2 00 04 00 00 	vpmovzxbq 0x400\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 72 80 	vpmovzxbq -0x400\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 b2 f8 fb ff ff 	vpmovzxbq -0x408\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 33 f5    	vpmovzxwd %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 33 f5    	vpmovzxwd %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 31    	vpmovzxwd \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 33 b4 f0 34 12 00 00 	vpmovzxwd 0x1234\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 72 7f 	vpmovzxwd 0xfe0\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 b2 00 10 00 00 	vpmovzxwd 0x1000\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 72 80 	vpmovzxwd -0x1000\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 b2 e0 ef ff ff 	vpmovzxwd -0x1020\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd 4f 34 f5    	vpmovzxwq %xmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 34 f5    	vpmovzxwq %xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 31    	vpmovzxwq \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 fd 4f 34 b4 f0 34 12 00 00 	vpmovzxwq 0x1234\(%rax,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 72 7f 	vpmovzxwq 0x7f0\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 b2 00 08 00 00 	vpmovzxwq 0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 72 80 	vpmovzxwq -0x800\(%rdx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 b2 f0 f7 ff ff 	vpmovzxwq -0x810\(%rdx\),%zmm30\{%k7\}
#pass
