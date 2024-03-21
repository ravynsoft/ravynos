#as:
#objdump: -dw
#name: i386 AVX512CD/VL insns
#source: avx512cd_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 f5[ 	]*vpconflictd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f c4 f5[ 	]*vpconflictd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 31[ 	]*vpconflictd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 30[ 	]*vpconflictd \(%eax\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 72 7f[ 	]*vpconflictd 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 b2 00 08 00 00[ 	]*vpconflictd 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 72 80[ 	]*vpconflictd -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 b2 f0 f7 ff ff[ 	]*vpconflictd -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 72 7f[ 	]*vpconflictd 0x1fc\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 b2 00 02 00 00[ 	]*vpconflictd 0x200\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 72 80[ 	]*vpconflictd -0x200\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 b2 fc fd ff ff[ 	]*vpconflictd -0x204\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 f5[ 	]*vpconflictd %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af c4 f5[ 	]*vpconflictd %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 31[ 	]*vpconflictd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 30[ 	]*vpconflictd \(%eax\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 72 7f[ 	]*vpconflictd 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 b2 00 10 00 00[ 	]*vpconflictd 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 72 80[ 	]*vpconflictd -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 b2 e0 ef ff ff[ 	]*vpconflictd -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 72 7f[ 	]*vpconflictd 0x1fc\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 b2 00 02 00 00[ 	]*vpconflictd 0x200\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 72 80[ 	]*vpconflictd -0x200\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 b2 fc fd ff ff[ 	]*vpconflictd -0x204\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 f5[ 	]*vpconflictq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f c4 f5[ 	]*vpconflictq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 31[ 	]*vpconflictq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 30[ 	]*vpconflictq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 72 7f[ 	]*vpconflictq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 b2 00 08 00 00[ 	]*vpconflictq 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 72 80[ 	]*vpconflictq -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 b2 f0 f7 ff ff[ 	]*vpconflictq -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 72 7f[ 	]*vpconflictq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 b2 00 04 00 00[ 	]*vpconflictq 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 72 80[ 	]*vpconflictq -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 b2 f8 fb ff ff[ 	]*vpconflictq -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 f5[ 	]*vpconflictq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af c4 f5[ 	]*vpconflictq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 31[ 	]*vpconflictq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 30[ 	]*vpconflictq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 72 7f[ 	]*vpconflictq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 b2 00 10 00 00[ 	]*vpconflictq 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 72 80[ 	]*vpconflictq -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 b2 e0 ef ff ff[ 	]*vpconflictq -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 72 7f[ 	]*vpconflictq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 b2 00 04 00 00[ 	]*vpconflictq 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 72 80[ 	]*vpconflictq -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 b2 f8 fb ff ff[ 	]*vpconflictq -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 f5[ 	]*vplzcntd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 44 f5[ 	]*vplzcntd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 31[ 	]*vplzcntd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 30[ 	]*vplzcntd \(%eax\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 72 7f[ 	]*vplzcntd 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 b2 00 08 00 00[ 	]*vplzcntd 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 72 80[ 	]*vplzcntd -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 b2 f0 f7 ff ff[ 	]*vplzcntd -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 72 7f[ 	]*vplzcntd 0x1fc\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 b2 00 02 00 00[ 	]*vplzcntd 0x200\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 72 80[ 	]*vplzcntd -0x200\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 b2 fc fd ff ff[ 	]*vplzcntd -0x204\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 f5[ 	]*vplzcntd %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 44 f5[ 	]*vplzcntd %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 31[ 	]*vplzcntd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 30[ 	]*vplzcntd \(%eax\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 72 7f[ 	]*vplzcntd 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 b2 00 10 00 00[ 	]*vplzcntd 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 72 80[ 	]*vplzcntd -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 b2 e0 ef ff ff[ 	]*vplzcntd -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 72 7f[ 	]*vplzcntd 0x1fc\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 b2 00 02 00 00[ 	]*vplzcntd 0x200\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 72 80[ 	]*vplzcntd -0x200\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 b2 fc fd ff ff[ 	]*vplzcntd -0x204\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 f5[ 	]*vplzcntq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 44 f5[ 	]*vplzcntq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 31[ 	]*vplzcntq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 30[ 	]*vplzcntq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 72 7f[ 	]*vplzcntq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 b2 00 08 00 00[ 	]*vplzcntq 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 72 80[ 	]*vplzcntq -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 b2 f0 f7 ff ff[ 	]*vplzcntq -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 72 7f[ 	]*vplzcntq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 b2 00 04 00 00[ 	]*vplzcntq 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 72 80[ 	]*vplzcntq -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 b2 f8 fb ff ff[ 	]*vplzcntq -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 f5[ 	]*vplzcntq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 44 f5[ 	]*vplzcntq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 31[ 	]*vplzcntq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 30[ 	]*vplzcntq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 72 7f[ 	]*vplzcntq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 b2 00 10 00 00[ 	]*vplzcntq 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 72 80[ 	]*vplzcntq -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 b2 e0 ef ff ff[ 	]*vplzcntq -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 72 7f[ 	]*vplzcntq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 b2 00 04 00 00[ 	]*vplzcntq 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 72 80[ 	]*vplzcntq -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 b2 f8 fb ff ff[ 	]*vplzcntq -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 3a f6[ 	]*vpbroadcastmw2d %k6,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 3a f6[ 	]*vpbroadcastmw2d %k6,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 2a f6[ 	]*vpbroadcastmb2q %k6,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 2a f6[ 	]*vpbroadcastmb2q %k6,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 f5[ 	]*vpconflictd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f c4 f5[ 	]*vpconflictd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 31[ 	]*vpconflictd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 30[ 	]*vpconflictd \(%eax\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 72 7f[ 	]*vpconflictd 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 b2 00 08 00 00[ 	]*vpconflictd 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 72 80[ 	]*vpconflictd -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 b2 f0 f7 ff ff[ 	]*vpconflictd -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 72 7f[ 	]*vpconflictd 0x1fc\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 b2 00 02 00 00[ 	]*vpconflictd 0x200\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 72 80[ 	]*vpconflictd -0x200\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 b2 fc fd ff ff[ 	]*vpconflictd -0x204\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 f5[ 	]*vpconflictd %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af c4 f5[ 	]*vpconflictd %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 31[ 	]*vpconflictd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 30[ 	]*vpconflictd \(%eax\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 72 7f[ 	]*vpconflictd 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 b2 00 10 00 00[ 	]*vpconflictd 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 72 80[ 	]*vpconflictd -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 b2 e0 ef ff ff[ 	]*vpconflictd -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 72 7f[ 	]*vpconflictd 0x1fc\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 b2 00 02 00 00[ 	]*vpconflictd 0x200\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 72 80[ 	]*vpconflictd -0x200\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 b2 fc fd ff ff[ 	]*vpconflictd -0x204\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 f5[ 	]*vpconflictq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f c4 f5[ 	]*vpconflictq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 31[ 	]*vpconflictq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 30[ 	]*vpconflictq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 72 7f[ 	]*vpconflictq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 b2 00 08 00 00[ 	]*vpconflictq 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 72 80[ 	]*vpconflictq -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 b2 f0 f7 ff ff[ 	]*vpconflictq -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 72 7f[ 	]*vpconflictq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 b2 00 04 00 00[ 	]*vpconflictq 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 72 80[ 	]*vpconflictq -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 b2 f8 fb ff ff[ 	]*vpconflictq -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 f5[ 	]*vpconflictq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af c4 f5[ 	]*vpconflictq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 31[ 	]*vpconflictq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 30[ 	]*vpconflictq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 72 7f[ 	]*vpconflictq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 b2 00 10 00 00[ 	]*vpconflictq 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 72 80[ 	]*vpconflictq -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 b2 e0 ef ff ff[ 	]*vpconflictq -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 72 7f[ 	]*vpconflictq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 b2 00 04 00 00[ 	]*vpconflictq 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 72 80[ 	]*vpconflictq -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 b2 f8 fb ff ff[ 	]*vpconflictq -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 f5[ 	]*vplzcntd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 44 f5[ 	]*vplzcntd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 31[ 	]*vplzcntd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 30[ 	]*vplzcntd \(%eax\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 72 7f[ 	]*vplzcntd 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 b2 00 08 00 00[ 	]*vplzcntd 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 72 80[ 	]*vplzcntd -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 b2 f0 f7 ff ff[ 	]*vplzcntd -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 72 7f[ 	]*vplzcntd 0x1fc\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 b2 00 02 00 00[ 	]*vplzcntd 0x200\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 72 80[ 	]*vplzcntd -0x200\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 b2 fc fd ff ff[ 	]*vplzcntd -0x204\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 f5[ 	]*vplzcntd %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 44 f5[ 	]*vplzcntd %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 31[ 	]*vplzcntd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 30[ 	]*vplzcntd \(%eax\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 72 7f[ 	]*vplzcntd 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 b2 00 10 00 00[ 	]*vplzcntd 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 72 80[ 	]*vplzcntd -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 b2 e0 ef ff ff[ 	]*vplzcntd -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 72 7f[ 	]*vplzcntd 0x1fc\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 b2 00 02 00 00[ 	]*vplzcntd 0x200\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 72 80[ 	]*vplzcntd -0x200\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 b2 fc fd ff ff[ 	]*vplzcntd -0x204\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 f5[ 	]*vplzcntq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 44 f5[ 	]*vplzcntq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 31[ 	]*vplzcntq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 30[ 	]*vplzcntq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 72 7f[ 	]*vplzcntq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 b2 00 08 00 00[ 	]*vplzcntq 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 72 80[ 	]*vplzcntq -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 b2 f0 f7 ff ff[ 	]*vplzcntq -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 72 7f[ 	]*vplzcntq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 b2 00 04 00 00[ 	]*vplzcntq 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 72 80[ 	]*vplzcntq -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 b2 f8 fb ff ff[ 	]*vplzcntq -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 f5[ 	]*vplzcntq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 44 f5[ 	]*vplzcntq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 31[ 	]*vplzcntq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 30[ 	]*vplzcntq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 72 7f[ 	]*vplzcntq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 b2 00 10 00 00[ 	]*vplzcntq 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 72 80[ 	]*vplzcntq -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 b2 e0 ef ff ff[ 	]*vplzcntq -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 72 7f[ 	]*vplzcntq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 b2 00 04 00 00[ 	]*vplzcntq 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 72 80[ 	]*vplzcntq -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 b2 f8 fb ff ff[ 	]*vplzcntq -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 3a f6[ 	]*vpbroadcastmw2d %k6,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 3a f6[ 	]*vpbroadcastmw2d %k6,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 2a f6[ 	]*vpbroadcastmb2q %k6,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 2a f6[ 	]*vpbroadcastmb2q %k6,%ymm6
#pass
