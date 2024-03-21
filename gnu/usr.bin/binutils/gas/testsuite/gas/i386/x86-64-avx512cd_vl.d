#as:
#objdump: -dw
#name: x86_64 AVX512CD/VL insns
#source: x86-64-avx512cd_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 c4 f5[ 	]*vpconflictd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f c4 f5[ 	]*vpconflictd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f c4 f5[ 	]*vpconflictd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 31[ 	]*vpconflictd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 c4 b4 f0 23 01 00 00[ 	]*vpconflictd 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 31[ 	]*vpconflictd \(%rcx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 72 7f[ 	]*vpconflictd 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 b2 00 08 00 00[ 	]*vpconflictd 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 72 80[ 	]*vpconflictd -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 b2 f0 f7 ff ff[ 	]*vpconflictd -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 72 7f[ 	]*vpconflictd 0x1fc\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 b2 00 02 00 00[ 	]*vpconflictd 0x200\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 72 80[ 	]*vpconflictd -0x200\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 b2 fc fd ff ff[ 	]*vpconflictd -0x204\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 c4 f5[ 	]*vpconflictd %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f c4 f5[ 	]*vpconflictd %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af c4 f5[ 	]*vpconflictd %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 31[ 	]*vpconflictd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 c4 b4 f0 23 01 00 00[ 	]*vpconflictd 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 31[ 	]*vpconflictd \(%rcx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 72 7f[ 	]*vpconflictd 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 b2 00 10 00 00[ 	]*vpconflictd 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 72 80[ 	]*vpconflictd -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 b2 e0 ef ff ff[ 	]*vpconflictd -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 72 7f[ 	]*vpconflictd 0x1fc\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 b2 00 02 00 00[ 	]*vpconflictd 0x200\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 72 80[ 	]*vpconflictd -0x200\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 b2 fc fd ff ff[ 	]*vpconflictd -0x204\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 c4 f5[ 	]*vpconflictq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f c4 f5[ 	]*vpconflictq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f c4 f5[ 	]*vpconflictq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 31[ 	]*vpconflictq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 c4 b4 f0 23 01 00 00[ 	]*vpconflictq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 31[ 	]*vpconflictq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 72 7f[ 	]*vpconflictq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 b2 00 08 00 00[ 	]*vpconflictq 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 72 80[ 	]*vpconflictq -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 b2 f0 f7 ff ff[ 	]*vpconflictq -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 72 7f[ 	]*vpconflictq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 b2 00 04 00 00[ 	]*vpconflictq 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 72 80[ 	]*vpconflictq -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 b2 f8 fb ff ff[ 	]*vpconflictq -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 c4 f5[ 	]*vpconflictq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f c4 f5[ 	]*vpconflictq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af c4 f5[ 	]*vpconflictq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 31[ 	]*vpconflictq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 c4 b4 f0 23 01 00 00[ 	]*vpconflictq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 31[ 	]*vpconflictq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 72 7f[ 	]*vpconflictq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 b2 00 10 00 00[ 	]*vpconflictq 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 72 80[ 	]*vpconflictq -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 b2 e0 ef ff ff[ 	]*vpconflictq -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 72 7f[ 	]*vpconflictq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 b2 00 04 00 00[ 	]*vpconflictq 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 72 80[ 	]*vpconflictq -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 b2 f8 fb ff ff[ 	]*vpconflictq -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 44 f5[ 	]*vplzcntd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 44 f5[ 	]*vplzcntd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 44 f5[ 	]*vplzcntd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 31[ 	]*vplzcntd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 44 b4 f0 23 01 00 00[ 	]*vplzcntd 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 31[ 	]*vplzcntd \(%rcx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 72 7f[ 	]*vplzcntd 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 b2 00 08 00 00[ 	]*vplzcntd 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 72 80[ 	]*vplzcntd -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 b2 f0 f7 ff ff[ 	]*vplzcntd -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 72 7f[ 	]*vplzcntd 0x1fc\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 b2 00 02 00 00[ 	]*vplzcntd 0x200\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 72 80[ 	]*vplzcntd -0x200\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 b2 fc fd ff ff[ 	]*vplzcntd -0x204\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 44 f5[ 	]*vplzcntd %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 44 f5[ 	]*vplzcntd %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 44 f5[ 	]*vplzcntd %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 31[ 	]*vplzcntd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 44 b4 f0 23 01 00 00[ 	]*vplzcntd 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 31[ 	]*vplzcntd \(%rcx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 72 7f[ 	]*vplzcntd 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 b2 00 10 00 00[ 	]*vplzcntd 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 72 80[ 	]*vplzcntd -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 b2 e0 ef ff ff[ 	]*vplzcntd -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 72 7f[ 	]*vplzcntd 0x1fc\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 b2 00 02 00 00[ 	]*vplzcntd 0x200\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 72 80[ 	]*vplzcntd -0x200\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 b2 fc fd ff ff[ 	]*vplzcntd -0x204\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 44 f5[ 	]*vplzcntq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 44 f5[ 	]*vplzcntq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 44 f5[ 	]*vplzcntq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 31[ 	]*vplzcntq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 44 b4 f0 23 01 00 00[ 	]*vplzcntq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 31[ 	]*vplzcntq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 72 7f[ 	]*vplzcntq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 b2 00 08 00 00[ 	]*vplzcntq 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 72 80[ 	]*vplzcntq -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 b2 f0 f7 ff ff[ 	]*vplzcntq -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 72 7f[ 	]*vplzcntq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 b2 00 04 00 00[ 	]*vplzcntq 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 72 80[ 	]*vplzcntq -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 b2 f8 fb ff ff[ 	]*vplzcntq -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 44 f5[ 	]*vplzcntq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 44 f5[ 	]*vplzcntq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 44 f5[ 	]*vplzcntq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 31[ 	]*vplzcntq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 44 b4 f0 23 01 00 00[ 	]*vplzcntq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 31[ 	]*vplzcntq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 72 7f[ 	]*vplzcntq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 b2 00 10 00 00[ 	]*vplzcntq 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 72 80[ 	]*vplzcntq -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 b2 e0 ef ff ff[ 	]*vplzcntq -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 72 7f[ 	]*vplzcntq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 b2 00 04 00 00[ 	]*vplzcntq 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 72 80[ 	]*vplzcntq -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 b2 f8 fb ff ff[ 	]*vplzcntq -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 3a f6[ 	]*vpbroadcastmw2d %k6,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 3a f6[ 	]*vpbroadcastmw2d %k6,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 08 2a f6[ 	]*vpbroadcastmb2q %k6,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 28 2a f6[ 	]*vpbroadcastmb2q %k6,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 c4 f5[ 	]*vpconflictd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f c4 f5[ 	]*vpconflictd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f c4 f5[ 	]*vpconflictd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 31[ 	]*vpconflictd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 c4 b4 f0 34 12 00 00[ 	]*vpconflictd 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 31[ 	]*vpconflictd \(%rcx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 72 7f[ 	]*vpconflictd 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 b2 00 08 00 00[ 	]*vpconflictd 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 72 80[ 	]*vpconflictd -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 b2 f0 f7 ff ff[ 	]*vpconflictd -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 72 7f[ 	]*vpconflictd 0x1fc\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 b2 00 02 00 00[ 	]*vpconflictd 0x200\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 72 80[ 	]*vpconflictd -0x200\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 b2 fc fd ff ff[ 	]*vpconflictd -0x204\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 c4 f5[ 	]*vpconflictd %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f c4 f5[ 	]*vpconflictd %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af c4 f5[ 	]*vpconflictd %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 31[ 	]*vpconflictd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 c4 b4 f0 34 12 00 00[ 	]*vpconflictd 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 31[ 	]*vpconflictd \(%rcx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 72 7f[ 	]*vpconflictd 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 b2 00 10 00 00[ 	]*vpconflictd 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 72 80[ 	]*vpconflictd -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 b2 e0 ef ff ff[ 	]*vpconflictd -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 72 7f[ 	]*vpconflictd 0x1fc\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 b2 00 02 00 00[ 	]*vpconflictd 0x200\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 72 80[ 	]*vpconflictd -0x200\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 b2 fc fd ff ff[ 	]*vpconflictd -0x204\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 c4 f5[ 	]*vpconflictq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f c4 f5[ 	]*vpconflictq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f c4 f5[ 	]*vpconflictq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 31[ 	]*vpconflictq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 c4 b4 f0 34 12 00 00[ 	]*vpconflictq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 31[ 	]*vpconflictq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 72 7f[ 	]*vpconflictq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 b2 00 08 00 00[ 	]*vpconflictq 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 72 80[ 	]*vpconflictq -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 b2 f0 f7 ff ff[ 	]*vpconflictq -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 72 7f[ 	]*vpconflictq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 b2 00 04 00 00[ 	]*vpconflictq 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 72 80[ 	]*vpconflictq -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 b2 f8 fb ff ff[ 	]*vpconflictq -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 c4 f5[ 	]*vpconflictq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f c4 f5[ 	]*vpconflictq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af c4 f5[ 	]*vpconflictq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 31[ 	]*vpconflictq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 c4 b4 f0 34 12 00 00[ 	]*vpconflictq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 31[ 	]*vpconflictq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 72 7f[ 	]*vpconflictq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 b2 00 10 00 00[ 	]*vpconflictq 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 72 80[ 	]*vpconflictq -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 b2 e0 ef ff ff[ 	]*vpconflictq -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 72 7f[ 	]*vpconflictq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 b2 00 04 00 00[ 	]*vpconflictq 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 72 80[ 	]*vpconflictq -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 b2 f8 fb ff ff[ 	]*vpconflictq -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 44 f5[ 	]*vplzcntd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 44 f5[ 	]*vplzcntd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 44 f5[ 	]*vplzcntd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 31[ 	]*vplzcntd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 44 b4 f0 34 12 00 00[ 	]*vplzcntd 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 31[ 	]*vplzcntd \(%rcx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 72 7f[ 	]*vplzcntd 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 b2 00 08 00 00[ 	]*vplzcntd 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 72 80[ 	]*vplzcntd -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 b2 f0 f7 ff ff[ 	]*vplzcntd -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 72 7f[ 	]*vplzcntd 0x1fc\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 b2 00 02 00 00[ 	]*vplzcntd 0x200\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 72 80[ 	]*vplzcntd -0x200\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 b2 fc fd ff ff[ 	]*vplzcntd -0x204\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 44 f5[ 	]*vplzcntd %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 44 f5[ 	]*vplzcntd %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 44 f5[ 	]*vplzcntd %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 31[ 	]*vplzcntd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 44 b4 f0 34 12 00 00[ 	]*vplzcntd 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 31[ 	]*vplzcntd \(%rcx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 72 7f[ 	]*vplzcntd 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 b2 00 10 00 00[ 	]*vplzcntd 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 72 80[ 	]*vplzcntd -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 b2 e0 ef ff ff[ 	]*vplzcntd -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 72 7f[ 	]*vplzcntd 0x1fc\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 b2 00 02 00 00[ 	]*vplzcntd 0x200\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 72 80[ 	]*vplzcntd -0x200\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 b2 fc fd ff ff[ 	]*vplzcntd -0x204\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 44 f5[ 	]*vplzcntq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 44 f5[ 	]*vplzcntq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 44 f5[ 	]*vplzcntq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 31[ 	]*vplzcntq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 44 b4 f0 34 12 00 00[ 	]*vplzcntq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 31[ 	]*vplzcntq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 72 7f[ 	]*vplzcntq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 b2 00 08 00 00[ 	]*vplzcntq 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 72 80[ 	]*vplzcntq -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 b2 f0 f7 ff ff[ 	]*vplzcntq -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 72 7f[ 	]*vplzcntq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 b2 00 04 00 00[ 	]*vplzcntq 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 72 80[ 	]*vplzcntq -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 b2 f8 fb ff ff[ 	]*vplzcntq -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 44 f5[ 	]*vplzcntq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 44 f5[ 	]*vplzcntq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 44 f5[ 	]*vplzcntq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 31[ 	]*vplzcntq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 44 b4 f0 34 12 00 00[ 	]*vplzcntq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 31[ 	]*vplzcntq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 72 7f[ 	]*vplzcntq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 b2 00 10 00 00[ 	]*vplzcntq 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 72 80[ 	]*vplzcntq -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 b2 e0 ef ff ff[ 	]*vplzcntq -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 72 7f[ 	]*vplzcntq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 b2 00 04 00 00[ 	]*vplzcntq 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 72 80[ 	]*vplzcntq -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 b2 f8 fb ff ff[ 	]*vplzcntq -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 3a f6[ 	]*vpbroadcastmw2d %k6,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 3a f6[ 	]*vpbroadcastmw2d %k6,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 08 2a f6[ 	]*vpbroadcastmb2q %k6,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 28 2a f6[ 	]*vpbroadcastmb2q %k6,%ymm30
#pass
