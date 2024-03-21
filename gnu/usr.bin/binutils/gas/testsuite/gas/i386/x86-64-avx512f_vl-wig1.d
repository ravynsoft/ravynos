#as: -mevexwig=1
#objdump: -dw
#name: x86_64 AVX512F/VL wig insns
#source: x86-64-avx512f_vl-wig.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 21 f5[ 	]*vpmovsxbd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 21 f5[ 	]*vpmovsxbd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 21 f5[ 	]*vpmovsxbd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 31[ 	]*vpmovsxbd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 21 b4 f0 23 01 00 00[ 	]*vpmovsxbd 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 72 7f[ 	]*vpmovsxbd 0x1fc\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 b2 00 02 00 00[ 	]*vpmovsxbd 0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 72 80[ 	]*vpmovsxbd -0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 b2 fc fd ff ff[ 	]*vpmovsxbd -0x204\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 21 f5[ 	]*vpmovsxbd %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 21 f5[ 	]*vpmovsxbd %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 21 f5[ 	]*vpmovsxbd %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 31[ 	]*vpmovsxbd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 21 b4 f0 23 01 00 00[ 	]*vpmovsxbd 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 72 7f[ 	]*vpmovsxbd 0x3f8\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 b2 00 04 00 00[ 	]*vpmovsxbd 0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 72 80[ 	]*vpmovsxbd -0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 b2 f8 fb ff ff[ 	]*vpmovsxbd -0x408\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 22 f5[ 	]*vpmovsxbq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 22 f5[ 	]*vpmovsxbq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 22 f5[ 	]*vpmovsxbq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 31[ 	]*vpmovsxbq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 22 b4 f0 23 01 00 00[ 	]*vpmovsxbq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 72 7f[ 	]*vpmovsxbq 0xfe\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 b2 00 01 00 00[ 	]*vpmovsxbq 0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 72 80[ 	]*vpmovsxbq -0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 b2 fe fe ff ff[ 	]*vpmovsxbq -0x102\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 22 f5[ 	]*vpmovsxbq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 22 f5[ 	]*vpmovsxbq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 22 f5[ 	]*vpmovsxbq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 31[ 	]*vpmovsxbq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 22 b4 f0 23 01 00 00[ 	]*vpmovsxbq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 72 7f[ 	]*vpmovsxbq 0x1fc\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 b2 00 02 00 00[ 	]*vpmovsxbq 0x200\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 72 80[ 	]*vpmovsxbq -0x200\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 b2 fc fd ff ff[ 	]*vpmovsxbq -0x204\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 23 f5[ 	]*vpmovsxwd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 23 f5[ 	]*vpmovsxwd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 23 f5[ 	]*vpmovsxwd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 31[ 	]*vpmovsxwd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 23 b4 f0 23 01 00 00[ 	]*vpmovsxwd 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 72 7f[ 	]*vpmovsxwd 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 b2 00 04 00 00[ 	]*vpmovsxwd 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 72 80[ 	]*vpmovsxwd -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 b2 f8 fb ff ff[ 	]*vpmovsxwd -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 23 f5[ 	]*vpmovsxwd %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 23 f5[ 	]*vpmovsxwd %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 23 f5[ 	]*vpmovsxwd %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 31[ 	]*vpmovsxwd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 23 b4 f0 23 01 00 00[ 	]*vpmovsxwd 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 72 7f[ 	]*vpmovsxwd 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 b2 00 08 00 00[ 	]*vpmovsxwd 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 72 80[ 	]*vpmovsxwd -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 b2 f0 f7 ff ff[ 	]*vpmovsxwd -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 24 f5[ 	]*vpmovsxwq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 24 f5[ 	]*vpmovsxwq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 24 f5[ 	]*vpmovsxwq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 31[ 	]*vpmovsxwq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 24 b4 f0 23 01 00 00[ 	]*vpmovsxwq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 72 7f[ 	]*vpmovsxwq 0x1fc\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 b2 00 02 00 00[ 	]*vpmovsxwq 0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 72 80[ 	]*vpmovsxwq -0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 b2 fc fd ff ff[ 	]*vpmovsxwq -0x204\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 24 f5[ 	]*vpmovsxwq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 24 f5[ 	]*vpmovsxwq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 24 f5[ 	]*vpmovsxwq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 31[ 	]*vpmovsxwq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 24 b4 f0 23 01 00 00[ 	]*vpmovsxwq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 72 7f[ 	]*vpmovsxwq 0x3f8\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 b2 00 04 00 00[ 	]*vpmovsxwq 0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 72 80[ 	]*vpmovsxwq -0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 b2 f8 fb ff ff[ 	]*vpmovsxwq -0x408\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 31 f5[ 	]*vpmovzxbd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 31 f5[ 	]*vpmovzxbd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 31 f5[ 	]*vpmovzxbd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 31[ 	]*vpmovzxbd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 31 b4 f0 23 01 00 00[ 	]*vpmovzxbd 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 72 7f[ 	]*vpmovzxbd 0x1fc\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 b2 00 02 00 00[ 	]*vpmovzxbd 0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 72 80[ 	]*vpmovzxbd -0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 b2 fc fd ff ff[ 	]*vpmovzxbd -0x204\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 31 f5[ 	]*vpmovzxbd %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 31 f5[ 	]*vpmovzxbd %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 31 f5[ 	]*vpmovzxbd %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 31[ 	]*vpmovzxbd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 31 b4 f0 23 01 00 00[ 	]*vpmovzxbd 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 72 7f[ 	]*vpmovzxbd 0x3f8\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 b2 00 04 00 00[ 	]*vpmovzxbd 0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 72 80[ 	]*vpmovzxbd -0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 b2 f8 fb ff ff[ 	]*vpmovzxbd -0x408\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 32 f5[ 	]*vpmovzxbq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 32 f5[ 	]*vpmovzxbq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 32 f5[ 	]*vpmovzxbq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 31[ 	]*vpmovzxbq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 32 b4 f0 23 01 00 00[ 	]*vpmovzxbq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 72 7f[ 	]*vpmovzxbq 0xfe\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 b2 00 01 00 00[ 	]*vpmovzxbq 0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 72 80[ 	]*vpmovzxbq -0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 b2 fe fe ff ff[ 	]*vpmovzxbq -0x102\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 32 f5[ 	]*vpmovzxbq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 32 f5[ 	]*vpmovzxbq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 32 f5[ 	]*vpmovzxbq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 31[ 	]*vpmovzxbq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 32 b4 f0 23 01 00 00[ 	]*vpmovzxbq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 72 7f[ 	]*vpmovzxbq 0x1fc\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 b2 00 02 00 00[ 	]*vpmovzxbq 0x200\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 72 80[ 	]*vpmovzxbq -0x200\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 b2 fc fd ff ff[ 	]*vpmovzxbq -0x204\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 33 f5[ 	]*vpmovzxwd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 33 f5[ 	]*vpmovzxwd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 33 f5[ 	]*vpmovzxwd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 31[ 	]*vpmovzxwd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 33 b4 f0 23 01 00 00[ 	]*vpmovzxwd 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 72 7f[ 	]*vpmovzxwd 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 b2 00 04 00 00[ 	]*vpmovzxwd 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 72 80[ 	]*vpmovzxwd -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 b2 f8 fb ff ff[ 	]*vpmovzxwd -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 33 f5[ 	]*vpmovzxwd %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 33 f5[ 	]*vpmovzxwd %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 33 f5[ 	]*vpmovzxwd %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 31[ 	]*vpmovzxwd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 33 b4 f0 23 01 00 00[ 	]*vpmovzxwd 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 72 7f[ 	]*vpmovzxwd 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 b2 00 08 00 00[ 	]*vpmovzxwd 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 72 80[ 	]*vpmovzxwd -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 b2 f0 f7 ff ff[ 	]*vpmovzxwd -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 34 f5[ 	]*vpmovzxwq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 34 f5[ 	]*vpmovzxwq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 34 f5[ 	]*vpmovzxwq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 31[ 	]*vpmovzxwq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 34 b4 f0 23 01 00 00[ 	]*vpmovzxwq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 72 7f[ 	]*vpmovzxwq 0x1fc\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 b2 00 02 00 00[ 	]*vpmovzxwq 0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 72 80[ 	]*vpmovzxwq -0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 b2 fc fd ff ff[ 	]*vpmovzxwq -0x204\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 34 f5[ 	]*vpmovzxwq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 34 f5[ 	]*vpmovzxwq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 34 f5[ 	]*vpmovzxwq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 31[ 	]*vpmovzxwq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 34 b4 f0 23 01 00 00[ 	]*vpmovzxwq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 72 7f[ 	]*vpmovzxwq 0x3f8\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 b2 00 04 00 00[ 	]*vpmovzxwq 0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 72 80[ 	]*vpmovzxwq -0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 b2 f8 fb ff ff[ 	]*vpmovzxwq -0x408\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 21 f5[ 	]*vpmovsxbd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 21 f5[ 	]*vpmovsxbd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 21 f5[ 	]*vpmovsxbd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 31[ 	]*vpmovsxbd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 21 b4 f0 34 12 00 00[ 	]*vpmovsxbd 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 72 7f[ 	]*vpmovsxbd 0x1fc\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 b2 00 02 00 00[ 	]*vpmovsxbd 0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 72 80[ 	]*vpmovsxbd -0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 b2 fc fd ff ff[ 	]*vpmovsxbd -0x204\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 21 f5[ 	]*vpmovsxbd %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 21 f5[ 	]*vpmovsxbd %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 21 f5[ 	]*vpmovsxbd %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 31[ 	]*vpmovsxbd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 21 b4 f0 34 12 00 00[ 	]*vpmovsxbd 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 72 7f[ 	]*vpmovsxbd 0x3f8\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 b2 00 04 00 00[ 	]*vpmovsxbd 0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 72 80[ 	]*vpmovsxbd -0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 b2 f8 fb ff ff[ 	]*vpmovsxbd -0x408\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 22 f5[ 	]*vpmovsxbq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 22 f5[ 	]*vpmovsxbq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 22 f5[ 	]*vpmovsxbq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 31[ 	]*vpmovsxbq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 22 b4 f0 34 12 00 00[ 	]*vpmovsxbq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 72 7f[ 	]*vpmovsxbq 0xfe\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 b2 00 01 00 00[ 	]*vpmovsxbq 0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 72 80[ 	]*vpmovsxbq -0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 b2 fe fe ff ff[ 	]*vpmovsxbq -0x102\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 22 f5[ 	]*vpmovsxbq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 22 f5[ 	]*vpmovsxbq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 22 f5[ 	]*vpmovsxbq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 31[ 	]*vpmovsxbq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 22 b4 f0 34 12 00 00[ 	]*vpmovsxbq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 72 7f[ 	]*vpmovsxbq 0x1fc\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 b2 00 02 00 00[ 	]*vpmovsxbq 0x200\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 72 80[ 	]*vpmovsxbq -0x200\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 b2 fc fd ff ff[ 	]*vpmovsxbq -0x204\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 23 f5[ 	]*vpmovsxwd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 23 f5[ 	]*vpmovsxwd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 23 f5[ 	]*vpmovsxwd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 31[ 	]*vpmovsxwd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 23 b4 f0 34 12 00 00[ 	]*vpmovsxwd 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 72 7f[ 	]*vpmovsxwd 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 b2 00 04 00 00[ 	]*vpmovsxwd 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 72 80[ 	]*vpmovsxwd -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 b2 f8 fb ff ff[ 	]*vpmovsxwd -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 23 f5[ 	]*vpmovsxwd %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 23 f5[ 	]*vpmovsxwd %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 23 f5[ 	]*vpmovsxwd %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 31[ 	]*vpmovsxwd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 23 b4 f0 34 12 00 00[ 	]*vpmovsxwd 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 72 7f[ 	]*vpmovsxwd 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 b2 00 08 00 00[ 	]*vpmovsxwd 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 72 80[ 	]*vpmovsxwd -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 b2 f0 f7 ff ff[ 	]*vpmovsxwd -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 24 f5[ 	]*vpmovsxwq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 24 f5[ 	]*vpmovsxwq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 24 f5[ 	]*vpmovsxwq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 31[ 	]*vpmovsxwq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 24 b4 f0 34 12 00 00[ 	]*vpmovsxwq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 72 7f[ 	]*vpmovsxwq 0x1fc\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 b2 00 02 00 00[ 	]*vpmovsxwq 0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 72 80[ 	]*vpmovsxwq -0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 b2 fc fd ff ff[ 	]*vpmovsxwq -0x204\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 24 f5[ 	]*vpmovsxwq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 24 f5[ 	]*vpmovsxwq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 24 f5[ 	]*vpmovsxwq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 31[ 	]*vpmovsxwq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 24 b4 f0 34 12 00 00[ 	]*vpmovsxwq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 72 7f[ 	]*vpmovsxwq 0x3f8\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 b2 00 04 00 00[ 	]*vpmovsxwq 0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 72 80[ 	]*vpmovsxwq -0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 b2 f8 fb ff ff[ 	]*vpmovsxwq -0x408\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 31 f5[ 	]*vpmovzxbd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 31 f5[ 	]*vpmovzxbd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 31 f5[ 	]*vpmovzxbd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 31[ 	]*vpmovzxbd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 31 b4 f0 34 12 00 00[ 	]*vpmovzxbd 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 72 7f[ 	]*vpmovzxbd 0x1fc\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 b2 00 02 00 00[ 	]*vpmovzxbd 0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 72 80[ 	]*vpmovzxbd -0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 b2 fc fd ff ff[ 	]*vpmovzxbd -0x204\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 31 f5[ 	]*vpmovzxbd %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 31 f5[ 	]*vpmovzxbd %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 31 f5[ 	]*vpmovzxbd %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 31[ 	]*vpmovzxbd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 31 b4 f0 34 12 00 00[ 	]*vpmovzxbd 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 72 7f[ 	]*vpmovzxbd 0x3f8\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 b2 00 04 00 00[ 	]*vpmovzxbd 0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 72 80[ 	]*vpmovzxbd -0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 b2 f8 fb ff ff[ 	]*vpmovzxbd -0x408\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 32 f5[ 	]*vpmovzxbq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 32 f5[ 	]*vpmovzxbq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 32 f5[ 	]*vpmovzxbq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 31[ 	]*vpmovzxbq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 32 b4 f0 34 12 00 00[ 	]*vpmovzxbq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 72 7f[ 	]*vpmovzxbq 0xfe\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 b2 00 01 00 00[ 	]*vpmovzxbq 0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 72 80[ 	]*vpmovzxbq -0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 b2 fe fe ff ff[ 	]*vpmovzxbq -0x102\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 32 f5[ 	]*vpmovzxbq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 32 f5[ 	]*vpmovzxbq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 32 f5[ 	]*vpmovzxbq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 31[ 	]*vpmovzxbq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 32 b4 f0 34 12 00 00[ 	]*vpmovzxbq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 72 7f[ 	]*vpmovzxbq 0x1fc\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 b2 00 02 00 00[ 	]*vpmovzxbq 0x200\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 72 80[ 	]*vpmovzxbq -0x200\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 b2 fc fd ff ff[ 	]*vpmovzxbq -0x204\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 33 f5[ 	]*vpmovzxwd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 33 f5[ 	]*vpmovzxwd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 33 f5[ 	]*vpmovzxwd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 31[ 	]*vpmovzxwd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 33 b4 f0 34 12 00 00[ 	]*vpmovzxwd 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 72 7f[ 	]*vpmovzxwd 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 b2 00 04 00 00[ 	]*vpmovzxwd 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 72 80[ 	]*vpmovzxwd -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 b2 f8 fb ff ff[ 	]*vpmovzxwd -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 33 f5[ 	]*vpmovzxwd %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 33 f5[ 	]*vpmovzxwd %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 33 f5[ 	]*vpmovzxwd %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 31[ 	]*vpmovzxwd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 33 b4 f0 34 12 00 00[ 	]*vpmovzxwd 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 72 7f[ 	]*vpmovzxwd 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 b2 00 08 00 00[ 	]*vpmovzxwd 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 72 80[ 	]*vpmovzxwd -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 b2 f0 f7 ff ff[ 	]*vpmovzxwd -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 34 f5[ 	]*vpmovzxwq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 34 f5[ 	]*vpmovzxwq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 34 f5[ 	]*vpmovzxwq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 31[ 	]*vpmovzxwq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 34 b4 f0 34 12 00 00[ 	]*vpmovzxwq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 72 7f[ 	]*vpmovzxwq 0x1fc\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 b2 00 02 00 00[ 	]*vpmovzxwq 0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 72 80[ 	]*vpmovzxwq -0x200\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 b2 fc fd ff ff[ 	]*vpmovzxwq -0x204\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 34 f5[ 	]*vpmovzxwq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 34 f5[ 	]*vpmovzxwq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 34 f5[ 	]*vpmovzxwq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 31[ 	]*vpmovzxwq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 34 b4 f0 34 12 00 00[ 	]*vpmovzxwq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 72 7f[ 	]*vpmovzxwq 0x3f8\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 b2 00 04 00 00[ 	]*vpmovzxwq 0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 72 80[ 	]*vpmovzxwq -0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 b2 f8 fb ff ff[ 	]*vpmovzxwq -0x408\(%rdx\),%ymm30
#pass
