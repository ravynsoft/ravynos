#as:
#objdump: -dw
#name: i386 AVX512BW/VL insns
#source: avx512bw_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c f5[ 	]*vpabsb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 1c f5[ 	]*vpabsb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c 31[ 	]*vpabsb \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c b4 f4 c0 1d fe ff[ 	]*vpabsb -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c 72 7f[ 	]*vpabsb 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c b2 00 08 00 00[ 	]*vpabsb 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c 72 80[ 	]*vpabsb -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c b2 f0 f7 ff ff[ 	]*vpabsb -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c f5[ 	]*vpabsb %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 1c f5[ 	]*vpabsb %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c 31[ 	]*vpabsb \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c b4 f4 c0 1d fe ff[ 	]*vpabsb -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c 72 7f[ 	]*vpabsb 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c b2 00 10 00 00[ 	]*vpabsb 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c 72 80[ 	]*vpabsb -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c b2 e0 ef ff ff[ 	]*vpabsb -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d f5[ 	]*vpabsw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 1d f5[ 	]*vpabsw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d 31[ 	]*vpabsw \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d b4 f4 c0 1d fe ff[ 	]*vpabsw -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d 72 7f[ 	]*vpabsw 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d b2 00 08 00 00[ 	]*vpabsw 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d 72 80[ 	]*vpabsw -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d b2 f0 f7 ff ff[ 	]*vpabsw -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d f5[ 	]*vpabsw %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 1d f5[ 	]*vpabsw %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d 31[ 	]*vpabsw \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d b4 f4 c0 1d fe ff[ 	]*vpabsw -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d 72 7f[ 	]*vpabsw 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d b2 00 10 00 00[ 	]*vpabsw 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d 72 80[ 	]*vpabsw -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d b2 e0 ef ff ff[ 	]*vpabsw -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b f4[ 	]*vpackssdw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 6b f4[ 	]*vpackssdw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b 31[ 	]*vpackssdw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b b4 f4 c0 1d fe ff[ 	]*vpackssdw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b 30[ 	]*vpackssdw \(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b 72 7f[ 	]*vpackssdw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b b2 00 08 00 00[ 	]*vpackssdw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b 72 80[ 	]*vpackssdw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b b2 f0 f7 ff ff[ 	]*vpackssdw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b 72 7f[ 	]*vpackssdw 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b b2 00 02 00 00[ 	]*vpackssdw 0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b 72 80[ 	]*vpackssdw -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b b2 fc fd ff ff[ 	]*vpackssdw -0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b f4[ 	]*vpackssdw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 6b f4[ 	]*vpackssdw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b 31[ 	]*vpackssdw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b b4 f4 c0 1d fe ff[ 	]*vpackssdw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b 30[ 	]*vpackssdw \(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b 72 7f[ 	]*vpackssdw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b b2 00 10 00 00[ 	]*vpackssdw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b 72 80[ 	]*vpackssdw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b b2 e0 ef ff ff[ 	]*vpackssdw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b 72 7f[ 	]*vpackssdw 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b b2 00 02 00 00[ 	]*vpackssdw 0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b 72 80[ 	]*vpackssdw -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b b2 fc fd ff ff[ 	]*vpackssdw -0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 f4[ 	]*vpacksswb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 63 f4[ 	]*vpacksswb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 31[ 	]*vpacksswb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 72 7f[ 	]*vpacksswb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 b2 00 08 00 00[ 	]*vpacksswb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 72 80[ 	]*vpacksswb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 b2 f0 f7 ff ff[ 	]*vpacksswb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 f4[ 	]*vpacksswb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 63 f4[ 	]*vpacksswb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 31[ 	]*vpacksswb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 72 7f[ 	]*vpacksswb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 b2 00 10 00 00[ 	]*vpacksswb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 72 80[ 	]*vpacksswb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 b2 e0 ef ff ff[ 	]*vpacksswb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b f4[ 	]*vpackusdw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 2b f4[ 	]*vpackusdw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b 31[ 	]*vpackusdw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b b4 f4 c0 1d fe ff[ 	]*vpackusdw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b 30[ 	]*vpackusdw \(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b 72 7f[ 	]*vpackusdw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b b2 00 08 00 00[ 	]*vpackusdw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b 72 80[ 	]*vpackusdw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b b2 f0 f7 ff ff[ 	]*vpackusdw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b 72 7f[ 	]*vpackusdw 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b b2 00 02 00 00[ 	]*vpackusdw 0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b 72 80[ 	]*vpackusdw -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b b2 fc fd ff ff[ 	]*vpackusdw -0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b f4[ 	]*vpackusdw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 2b f4[ 	]*vpackusdw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b 31[ 	]*vpackusdw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b b4 f4 c0 1d fe ff[ 	]*vpackusdw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b 30[ 	]*vpackusdw \(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b 72 7f[ 	]*vpackusdw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b b2 00 10 00 00[ 	]*vpackusdw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b 72 80[ 	]*vpackusdw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b b2 e0 ef ff ff[ 	]*vpackusdw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b 72 7f[ 	]*vpackusdw 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b b2 00 02 00 00[ 	]*vpackusdw 0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b 72 80[ 	]*vpackusdw -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b b2 fc fd ff ff[ 	]*vpackusdw -0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 f4[ 	]*vpackuswb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 67 f4[ 	]*vpackuswb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 31[ 	]*vpackuswb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 72 7f[ 	]*vpackuswb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 b2 00 08 00 00[ 	]*vpackuswb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 72 80[ 	]*vpackuswb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 b2 f0 f7 ff ff[ 	]*vpackuswb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 f4[ 	]*vpackuswb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 67 f4[ 	]*vpackuswb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 31[ 	]*vpackuswb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 72 7f[ 	]*vpackuswb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 b2 00 10 00 00[ 	]*vpackuswb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 72 80[ 	]*vpackuswb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 b2 e0 ef ff ff[ 	]*vpackuswb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc f4[ 	]*vpaddb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f fc f4[ 	]*vpaddb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc 31[ 	]*vpaddb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc b4 f4 c0 1d fe ff[ 	]*vpaddb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc 72 7f[ 	]*vpaddb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc b2 00 08 00 00[ 	]*vpaddb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc 72 80[ 	]*vpaddb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc b2 f0 f7 ff ff[ 	]*vpaddb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc f4[ 	]*vpaddb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af fc f4[ 	]*vpaddb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc 31[ 	]*vpaddb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc b4 f4 c0 1d fe ff[ 	]*vpaddb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc 72 7f[ 	]*vpaddb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc b2 00 10 00 00[ 	]*vpaddb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc 72 80[ 	]*vpaddb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc b2 e0 ef ff ff[ 	]*vpaddb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec f4[ 	]*vpaddsb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ec f4[ 	]*vpaddsb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec 31[ 	]*vpaddsb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec b4 f4 c0 1d fe ff[ 	]*vpaddsb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec 72 7f[ 	]*vpaddsb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec b2 00 08 00 00[ 	]*vpaddsb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec 72 80[ 	]*vpaddsb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec b2 f0 f7 ff ff[ 	]*vpaddsb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec f4[ 	]*vpaddsb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ec f4[ 	]*vpaddsb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec 31[ 	]*vpaddsb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec b4 f4 c0 1d fe ff[ 	]*vpaddsb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec 72 7f[ 	]*vpaddsb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec b2 00 10 00 00[ 	]*vpaddsb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec 72 80[ 	]*vpaddsb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec b2 e0 ef ff ff[ 	]*vpaddsb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed f4[ 	]*vpaddsw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ed f4[ 	]*vpaddsw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed 31[ 	]*vpaddsw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed b4 f4 c0 1d fe ff[ 	]*vpaddsw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed 72 7f[ 	]*vpaddsw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed b2 00 08 00 00[ 	]*vpaddsw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed 72 80[ 	]*vpaddsw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed b2 f0 f7 ff ff[ 	]*vpaddsw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed f4[ 	]*vpaddsw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ed f4[ 	]*vpaddsw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed 31[ 	]*vpaddsw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed b4 f4 c0 1d fe ff[ 	]*vpaddsw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed 72 7f[ 	]*vpaddsw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed b2 00 10 00 00[ 	]*vpaddsw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed 72 80[ 	]*vpaddsw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed b2 e0 ef ff ff[ 	]*vpaddsw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc f4[ 	]*vpaddusb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f dc f4[ 	]*vpaddusb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc 31[ 	]*vpaddusb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc b4 f4 c0 1d fe ff[ 	]*vpaddusb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc 72 7f[ 	]*vpaddusb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc b2 00 08 00 00[ 	]*vpaddusb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc 72 80[ 	]*vpaddusb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc b2 f0 f7 ff ff[ 	]*vpaddusb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc f4[ 	]*vpaddusb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af dc f4[ 	]*vpaddusb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc 31[ 	]*vpaddusb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc b4 f4 c0 1d fe ff[ 	]*vpaddusb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc 72 7f[ 	]*vpaddusb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc b2 00 10 00 00[ 	]*vpaddusb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc 72 80[ 	]*vpaddusb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc b2 e0 ef ff ff[ 	]*vpaddusb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd f4[ 	]*vpaddusw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f dd f4[ 	]*vpaddusw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd 31[ 	]*vpaddusw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd b4 f4 c0 1d fe ff[ 	]*vpaddusw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd 72 7f[ 	]*vpaddusw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd b2 00 08 00 00[ 	]*vpaddusw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd 72 80[ 	]*vpaddusw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd b2 f0 f7 ff ff[ 	]*vpaddusw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd f4[ 	]*vpaddusw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af dd f4[ 	]*vpaddusw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd 31[ 	]*vpaddusw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd b4 f4 c0 1d fe ff[ 	]*vpaddusw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd 72 7f[ 	]*vpaddusw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd b2 00 10 00 00[ 	]*vpaddusw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd 72 80[ 	]*vpaddusw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd b2 e0 ef ff ff[ 	]*vpaddusw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd f4[ 	]*vpaddw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f fd f4[ 	]*vpaddw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd 31[ 	]*vpaddw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd b4 f4 c0 1d fe ff[ 	]*vpaddw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd 72 7f[ 	]*vpaddw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd b2 00 08 00 00[ 	]*vpaddw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd 72 80[ 	]*vpaddw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd b2 f0 f7 ff ff[ 	]*vpaddw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd f4[ 	]*vpaddw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af fd f4[ 	]*vpaddw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd 31[ 	]*vpaddw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd b4 f4 c0 1d fe ff[ 	]*vpaddw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd 72 7f[ 	]*vpaddw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd b2 00 10 00 00[ 	]*vpaddw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd 72 80[ 	]*vpaddw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd b2 e0 ef ff ff[ 	]*vpaddw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f f4 ab[ 	]*vpalignr \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 0f f4 ab[ 	]*vpalignr \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f f4 7b[ 	]*vpalignr \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f 31 7b[ 	]*vpalignr \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f 72 7f 7b[ 	]*vpalignr \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f b2 00 08 00 00 7b[ 	]*vpalignr \$0x7b,0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f 72 80 7b[ 	]*vpalignr \$0x7b,-0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f b2 f0 f7 ff ff 7b[ 	]*vpalignr \$0x7b,-0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f f4 ab[ 	]*vpalignr \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 0f f4 ab[ 	]*vpalignr \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f f4 7b[ 	]*vpalignr \$0x7b,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f 31 7b[ 	]*vpalignr \$0x7b,\(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f 72 7f 7b[ 	]*vpalignr \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f b2 00 10 00 00 7b[ 	]*vpalignr \$0x7b,0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f 72 80 7b[ 	]*vpalignr \$0x7b,-0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f b2 e0 ef ff ff 7b[ 	]*vpalignr \$0x7b,-0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 f4[ 	]*vpavgb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e0 f4[ 	]*vpavgb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 31[ 	]*vpavgb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 b4 f4 c0 1d fe ff[ 	]*vpavgb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 72 7f[ 	]*vpavgb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 b2 00 08 00 00[ 	]*vpavgb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 72 80[ 	]*vpavgb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 b2 f0 f7 ff ff[ 	]*vpavgb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 f4[ 	]*vpavgb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e0 f4[ 	]*vpavgb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 31[ 	]*vpavgb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 b4 f4 c0 1d fe ff[ 	]*vpavgb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 72 7f[ 	]*vpavgb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 b2 00 10 00 00[ 	]*vpavgb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 72 80[ 	]*vpavgb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 b2 e0 ef ff ff[ 	]*vpavgb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 f4[ 	]*vpavgw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e3 f4[ 	]*vpavgw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 31[ 	]*vpavgw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 b4 f4 c0 1d fe ff[ 	]*vpavgw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 72 7f[ 	]*vpavgw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 b2 00 08 00 00[ 	]*vpavgw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 72 80[ 	]*vpavgw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 b2 f0 f7 ff ff[ 	]*vpavgw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 f4[ 	]*vpavgw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e3 f4[ 	]*vpavgw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 31[ 	]*vpavgw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 b4 f4 c0 1d fe ff[ 	]*vpavgw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 72 7f[ 	]*vpavgw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 b2 00 10 00 00[ 	]*vpavgw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 72 80[ 	]*vpavgw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 b2 e0 ef ff ff[ 	]*vpavgw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 f4[ 	]*vpblendmb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 66 f4[ 	]*vpblendmb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 31[ 	]*vpblendmb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 72 7f[ 	]*vpblendmb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 b2 00 08 00 00[ 	]*vpblendmb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 72 80[ 	]*vpblendmb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 b2 f0 f7 ff ff[ 	]*vpblendmb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 f4[ 	]*vpblendmb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 66 f4[ 	]*vpblendmb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 31[ 	]*vpblendmb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 72 7f[ 	]*vpblendmb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 b2 00 10 00 00[ 	]*vpblendmb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 72 80[ 	]*vpblendmb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 b2 e0 ef ff ff[ 	]*vpblendmb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 f5[ 	]*vpbroadcastb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 78 f5[ 	]*vpbroadcastb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 31[ 	]*vpbroadcastb \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 b4 f4 c0 1d fe ff[ 	]*vpbroadcastb -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 72 7f[ 	]*vpbroadcastb 0x7f\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 b2 80 00 00 00[ 	]*vpbroadcastb 0x80\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 72 80[ 	]*vpbroadcastb -0x80\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 b2 7f ff ff ff[ 	]*vpbroadcastb -0x81\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 f5[ 	]*vpbroadcastb %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 78 f5[ 	]*vpbroadcastb %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 31[ 	]*vpbroadcastb \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 b4 f4 c0 1d fe ff[ 	]*vpbroadcastb -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 72 7f[ 	]*vpbroadcastb 0x7f\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 b2 80 00 00 00[ 	]*vpbroadcastb 0x80\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 72 80[ 	]*vpbroadcastb -0x80\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 b2 7f ff ff ff[ 	]*vpbroadcastb -0x81\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7a f0[ 	]*vpbroadcastb %eax,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 7a f0[ 	]*vpbroadcastb %eax,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7a f5[ 	]*vpbroadcastb %ebp,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7a f0[ 	]*vpbroadcastb %eax,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 7a f0[ 	]*vpbroadcastb %eax,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7a f5[ 	]*vpbroadcastb %ebp,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 f5[ 	]*vpbroadcastw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 79 f5[ 	]*vpbroadcastw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 31[ 	]*vpbroadcastw \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 b4 f4 c0 1d fe ff[ 	]*vpbroadcastw -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 72 7f[ 	]*vpbroadcastw 0xfe\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 b2 00 01 00 00[ 	]*vpbroadcastw 0x100\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 72 80[ 	]*vpbroadcastw -0x100\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 b2 fe fe ff ff[ 	]*vpbroadcastw -0x102\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 f5[ 	]*vpbroadcastw %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 79 f5[ 	]*vpbroadcastw %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 31[ 	]*vpbroadcastw \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 b4 f4 c0 1d fe ff[ 	]*vpbroadcastw -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 72 7f[ 	]*vpbroadcastw 0xfe\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 b2 00 01 00 00[ 	]*vpbroadcastw 0x100\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 72 80[ 	]*vpbroadcastw -0x100\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 b2 fe fe ff ff[ 	]*vpbroadcastw -0x102\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7b f0[ 	]*vpbroadcastw %eax,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 7b f0[ 	]*vpbroadcastw %eax,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7b f5[ 	]*vpbroadcastw %ebp,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7b f0[ 	]*vpbroadcastw %eax,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 7b f0[ 	]*vpbroadcastw %eax,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7b f5[ 	]*vpbroadcastw %ebp,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 ed[ 	]*vpcmpeqb %xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 29[ 	]*vpcmpeqb \(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb -0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 6a 7f[ 	]*vpcmpeqb 0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 aa 00 08 00 00[ 	]*vpcmpeqb 0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 6a 80[ 	]*vpcmpeqb -0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 aa f0 f7 ff ff[ 	]*vpcmpeqb -0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 ed[ 	]*vpcmpeqb %ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 29[ 	]*vpcmpeqb \(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb -0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 6a 7f[ 	]*vpcmpeqb 0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 aa 00 10 00 00[ 	]*vpcmpeqb 0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 6a 80[ 	]*vpcmpeqb -0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 aa e0 ef ff ff[ 	]*vpcmpeqb -0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 ed[ 	]*vpcmpeqw %xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 29[ 	]*vpcmpeqw \(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw -0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 6a 7f[ 	]*vpcmpeqw 0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 aa 00 08 00 00[ 	]*vpcmpeqw 0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 6a 80[ 	]*vpcmpeqw -0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 aa f0 f7 ff ff[ 	]*vpcmpeqw -0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 ed[ 	]*vpcmpeqw %ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 29[ 	]*vpcmpeqw \(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw -0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 6a 7f[ 	]*vpcmpeqw 0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 aa 00 10 00 00[ 	]*vpcmpeqw 0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 6a 80[ 	]*vpcmpeqw -0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 aa e0 ef ff ff[ 	]*vpcmpeqw -0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 ed[ 	]*vpcmpgtb %xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 29[ 	]*vpcmpgtb \(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb -0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 6a 7f[ 	]*vpcmpgtb 0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 aa 00 08 00 00[ 	]*vpcmpgtb 0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 6a 80[ 	]*vpcmpgtb -0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 aa f0 f7 ff ff[ 	]*vpcmpgtb -0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 ed[ 	]*vpcmpgtb %ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 29[ 	]*vpcmpgtb \(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb -0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 6a 7f[ 	]*vpcmpgtb 0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 aa 00 10 00 00[ 	]*vpcmpgtb 0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 6a 80[ 	]*vpcmpgtb -0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 aa e0 ef ff ff[ 	]*vpcmpgtb -0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 ed[ 	]*vpcmpgtw %xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 29[ 	]*vpcmpgtw \(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw -0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 6a 7f[ 	]*vpcmpgtw 0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 aa 00 08 00 00[ 	]*vpcmpgtw 0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 6a 80[ 	]*vpcmpgtw -0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 aa f0 f7 ff ff[ 	]*vpcmpgtw -0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 ed[ 	]*vpcmpgtw %ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 29[ 	]*vpcmpgtw \(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw -0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 6a 7f[ 	]*vpcmpgtw 0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 aa 00 10 00 00[ 	]*vpcmpgtw 0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 6a 80[ 	]*vpcmpgtw -0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 aa e0 ef ff ff[ 	]*vpcmpgtw -0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 f4[ 	]*vpblendmw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 66 f4[ 	]*vpblendmw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 31[ 	]*vpblendmw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 72 7f[ 	]*vpblendmw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 b2 00 08 00 00[ 	]*vpblendmw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 72 80[ 	]*vpblendmw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 b2 f0 f7 ff ff[ 	]*vpblendmw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 f4[ 	]*vpblendmw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 66 f4[ 	]*vpblendmw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 31[ 	]*vpblendmw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 72 7f[ 	]*vpblendmw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 b2 00 10 00 00[ 	]*vpblendmw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 72 80[ 	]*vpblendmw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 b2 e0 ef ff ff[ 	]*vpblendmw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 f4[ 	]*vpmaddubsw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 04 f4[ 	]*vpmaddubsw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 31[ 	]*vpmaddubsw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 72 7f[ 	]*vpmaddubsw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 b2 00 08 00 00[ 	]*vpmaddubsw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 72 80[ 	]*vpmaddubsw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 b2 f0 f7 ff ff[ 	]*vpmaddubsw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 f4[ 	]*vpmaddubsw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 04 f4[ 	]*vpmaddubsw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 31[ 	]*vpmaddubsw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 72 7f[ 	]*vpmaddubsw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 b2 00 10 00 00[ 	]*vpmaddubsw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 72 80[ 	]*vpmaddubsw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 b2 e0 ef ff ff[ 	]*vpmaddubsw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 f4[ 	]*vpmaddwd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f5 f4[ 	]*vpmaddwd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 31[ 	]*vpmaddwd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 72 7f[ 	]*vpmaddwd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 b2 00 08 00 00[ 	]*vpmaddwd 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 72 80[ 	]*vpmaddwd -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 b2 f0 f7 ff ff[ 	]*vpmaddwd -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 f4[ 	]*vpmaddwd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f5 f4[ 	]*vpmaddwd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 31[ 	]*vpmaddwd \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 72 7f[ 	]*vpmaddwd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 b2 00 10 00 00[ 	]*vpmaddwd 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 72 80[ 	]*vpmaddwd -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 b2 e0 ef ff ff[ 	]*vpmaddwd -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c f4[ 	]*vpmaxsb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 3c f4[ 	]*vpmaxsb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c 31[ 	]*vpmaxsb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c 72 7f[ 	]*vpmaxsb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c b2 00 08 00 00[ 	]*vpmaxsb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c 72 80[ 	]*vpmaxsb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c b2 f0 f7 ff ff[ 	]*vpmaxsb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c f4[ 	]*vpmaxsb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 3c f4[ 	]*vpmaxsb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c 31[ 	]*vpmaxsb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c 72 7f[ 	]*vpmaxsb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c b2 00 10 00 00[ 	]*vpmaxsb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c 72 80[ 	]*vpmaxsb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c b2 e0 ef ff ff[ 	]*vpmaxsb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee f4[ 	]*vpmaxsw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ee f4[ 	]*vpmaxsw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee 31[ 	]*vpmaxsw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee 72 7f[ 	]*vpmaxsw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee b2 00 08 00 00[ 	]*vpmaxsw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee 72 80[ 	]*vpmaxsw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee b2 f0 f7 ff ff[ 	]*vpmaxsw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee f4[ 	]*vpmaxsw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ee f4[ 	]*vpmaxsw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee 31[ 	]*vpmaxsw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee 72 7f[ 	]*vpmaxsw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee b2 00 10 00 00[ 	]*vpmaxsw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee 72 80[ 	]*vpmaxsw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee b2 e0 ef ff ff[ 	]*vpmaxsw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de f4[ 	]*vpmaxub %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f de f4[ 	]*vpmaxub %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de 31[ 	]*vpmaxub \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de b4 f4 c0 1d fe ff[ 	]*vpmaxub -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de 72 7f[ 	]*vpmaxub 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de b2 00 08 00 00[ 	]*vpmaxub 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de 72 80[ 	]*vpmaxub -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de b2 f0 f7 ff ff[ 	]*vpmaxub -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de f4[ 	]*vpmaxub %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af de f4[ 	]*vpmaxub %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de 31[ 	]*vpmaxub \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de b4 f4 c0 1d fe ff[ 	]*vpmaxub -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de 72 7f[ 	]*vpmaxub 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de b2 00 10 00 00[ 	]*vpmaxub 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de 72 80[ 	]*vpmaxub -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de b2 e0 ef ff ff[ 	]*vpmaxub -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e f4[ 	]*vpmaxuw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 3e f4[ 	]*vpmaxuw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e 31[ 	]*vpmaxuw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e 72 7f[ 	]*vpmaxuw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e b2 00 08 00 00[ 	]*vpmaxuw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e 72 80[ 	]*vpmaxuw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e b2 f0 f7 ff ff[ 	]*vpmaxuw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e f4[ 	]*vpmaxuw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 3e f4[ 	]*vpmaxuw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e 31[ 	]*vpmaxuw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e 72 7f[ 	]*vpmaxuw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e b2 00 10 00 00[ 	]*vpmaxuw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e 72 80[ 	]*vpmaxuw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e b2 e0 ef ff ff[ 	]*vpmaxuw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 f4[ 	]*vpminsb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 38 f4[ 	]*vpminsb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 31[ 	]*vpminsb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 b4 f4 c0 1d fe ff[ 	]*vpminsb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 72 7f[ 	]*vpminsb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 b2 00 08 00 00[ 	]*vpminsb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 72 80[ 	]*vpminsb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 b2 f0 f7 ff ff[ 	]*vpminsb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 f4[ 	]*vpminsb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 38 f4[ 	]*vpminsb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 31[ 	]*vpminsb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 b4 f4 c0 1d fe ff[ 	]*vpminsb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 72 7f[ 	]*vpminsb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 b2 00 10 00 00[ 	]*vpminsb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 72 80[ 	]*vpminsb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 b2 e0 ef ff ff[ 	]*vpminsb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea f4[ 	]*vpminsw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ea f4[ 	]*vpminsw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea 31[ 	]*vpminsw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea b4 f4 c0 1d fe ff[ 	]*vpminsw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea 72 7f[ 	]*vpminsw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea b2 00 08 00 00[ 	]*vpminsw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea 72 80[ 	]*vpminsw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea b2 f0 f7 ff ff[ 	]*vpminsw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea f4[ 	]*vpminsw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ea f4[ 	]*vpminsw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea 31[ 	]*vpminsw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea b4 f4 c0 1d fe ff[ 	]*vpminsw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea 72 7f[ 	]*vpminsw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea b2 00 10 00 00[ 	]*vpminsw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea 72 80[ 	]*vpminsw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea b2 e0 ef ff ff[ 	]*vpminsw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da f4[ 	]*vpminub %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f da f4[ 	]*vpminub %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da 31[ 	]*vpminub \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da b4 f4 c0 1d fe ff[ 	]*vpminub -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da 72 7f[ 	]*vpminub 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da b2 00 08 00 00[ 	]*vpminub 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da 72 80[ 	]*vpminub -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da b2 f0 f7 ff ff[ 	]*vpminub -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da f4[ 	]*vpminub %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af da f4[ 	]*vpminub %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da 31[ 	]*vpminub \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da b4 f4 c0 1d fe ff[ 	]*vpminub -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da 72 7f[ 	]*vpminub 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da b2 00 10 00 00[ 	]*vpminub 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da 72 80[ 	]*vpminub -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da b2 e0 ef ff ff[ 	]*vpminub -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a f4[ 	]*vpminuw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 3a f4[ 	]*vpminuw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a 31[ 	]*vpminuw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a b4 f4 c0 1d fe ff[ 	]*vpminuw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a 72 7f[ 	]*vpminuw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a b2 00 08 00 00[ 	]*vpminuw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a 72 80[ 	]*vpminuw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a b2 f0 f7 ff ff[ 	]*vpminuw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a f4[ 	]*vpminuw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 3a f4[ 	]*vpminuw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a 31[ 	]*vpminuw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a b4 f4 c0 1d fe ff[ 	]*vpminuw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a 72 7f[ 	]*vpminuw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a b2 00 10 00 00[ 	]*vpminuw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a 72 80[ 	]*vpminuw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a b2 e0 ef ff ff[ 	]*vpminuw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 f5[ 	]*vpmovsxbw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 20 f5[ 	]*vpmovsxbw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 31[ 	]*vpmovsxbw \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 72 7f[ 	]*vpmovsxbw 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 b2 00 04 00 00[ 	]*vpmovsxbw 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 72 80[ 	]*vpmovsxbw -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 b2 f8 fb ff ff[ 	]*vpmovsxbw -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 f5[ 	]*vpmovsxbw %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 20 f5[ 	]*vpmovsxbw %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 31[ 	]*vpmovsxbw \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 72 7f[ 	]*vpmovsxbw 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 b2 00 08 00 00[ 	]*vpmovsxbw 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 72 80[ 	]*vpmovsxbw -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 b2 f0 f7 ff ff[ 	]*vpmovsxbw -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 f5[ 	]*vpmovzxbw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 30 f5[ 	]*vpmovzxbw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 31[ 	]*vpmovzxbw \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 72 7f[ 	]*vpmovzxbw 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 b2 00 04 00 00[ 	]*vpmovzxbw 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 72 80[ 	]*vpmovzxbw -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 b2 f8 fb ff ff[ 	]*vpmovzxbw -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 f5[ 	]*vpmovzxbw %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 30 f5[ 	]*vpmovzxbw %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 31[ 	]*vpmovzxbw \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 72 7f[ 	]*vpmovzxbw 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 b2 00 08 00 00[ 	]*vpmovzxbw 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 72 80[ 	]*vpmovzxbw -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 b2 f0 f7 ff ff[ 	]*vpmovzxbw -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b f4[ 	]*vpmulhrsw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 0b f4[ 	]*vpmulhrsw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b 31[ 	]*vpmulhrsw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b 72 7f[ 	]*vpmulhrsw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b b2 00 08 00 00[ 	]*vpmulhrsw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b 72 80[ 	]*vpmulhrsw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b b2 f0 f7 ff ff[ 	]*vpmulhrsw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b f4[ 	]*vpmulhrsw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 0b f4[ 	]*vpmulhrsw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b 31[ 	]*vpmulhrsw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b 72 7f[ 	]*vpmulhrsw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b b2 00 10 00 00[ 	]*vpmulhrsw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b 72 80[ 	]*vpmulhrsw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b b2 e0 ef ff ff[ 	]*vpmulhrsw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 f4[ 	]*vpmulhuw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e4 f4[ 	]*vpmulhuw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 31[ 	]*vpmulhuw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 72 7f[ 	]*vpmulhuw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 b2 00 08 00 00[ 	]*vpmulhuw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 72 80[ 	]*vpmulhuw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 b2 f0 f7 ff ff[ 	]*vpmulhuw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 f4[ 	]*vpmulhuw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e4 f4[ 	]*vpmulhuw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 31[ 	]*vpmulhuw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 72 7f[ 	]*vpmulhuw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 b2 00 10 00 00[ 	]*vpmulhuw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 72 80[ 	]*vpmulhuw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 b2 e0 ef ff ff[ 	]*vpmulhuw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 f4[ 	]*vpmulhw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e5 f4[ 	]*vpmulhw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 31[ 	]*vpmulhw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 72 7f[ 	]*vpmulhw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 b2 00 08 00 00[ 	]*vpmulhw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 72 80[ 	]*vpmulhw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 b2 f0 f7 ff ff[ 	]*vpmulhw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 f4[ 	]*vpmulhw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e5 f4[ 	]*vpmulhw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 31[ 	]*vpmulhw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 72 7f[ 	]*vpmulhw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 b2 00 10 00 00[ 	]*vpmulhw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 72 80[ 	]*vpmulhw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 b2 e0 ef ff ff[ 	]*vpmulhw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 f4[ 	]*vpmullw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d5 f4[ 	]*vpmullw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 31[ 	]*vpmullw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 b4 f4 c0 1d fe ff[ 	]*vpmullw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 72 7f[ 	]*vpmullw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 b2 00 08 00 00[ 	]*vpmullw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 72 80[ 	]*vpmullw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 b2 f0 f7 ff ff[ 	]*vpmullw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 f4[ 	]*vpmullw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d5 f4[ 	]*vpmullw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 31[ 	]*vpmullw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 b4 f4 c0 1d fe ff[ 	]*vpmullw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 72 7f[ 	]*vpmullw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 b2 00 10 00 00[ 	]*vpmullw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 72 80[ 	]*vpmullw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 b2 e0 ef ff ff[ 	]*vpmullw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 f4[ 	]*vpshufb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 00 f4[ 	]*vpshufb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 31[ 	]*vpshufb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 b4 f4 c0 1d fe ff[ 	]*vpshufb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 72 7f[ 	]*vpshufb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 b2 00 08 00 00[ 	]*vpshufb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 72 80[ 	]*vpshufb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 b2 f0 f7 ff ff[ 	]*vpshufb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 f4[ 	]*vpshufb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 00 f4[ 	]*vpshufb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 31[ 	]*vpshufb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 b4 f4 c0 1d fe ff[ 	]*vpshufb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 72 7f[ 	]*vpshufb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 b2 00 10 00 00[ 	]*vpshufb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 72 80[ 	]*vpshufb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 b2 e0 ef ff ff[ 	]*vpshufb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 f5 ab[ 	]*vpshufhw \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 8f 70 f5 ab[ 	]*vpshufhw \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 f5 7b[ 	]*vpshufhw \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 31 7b[ 	]*vpshufhw \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 72 7f 7b[ 	]*vpshufhw \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 b2 00 08 00 00 7b[ 	]*vpshufhw \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 72 80 7b[ 	]*vpshufhw \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 b2 f0 f7 ff ff 7b[ 	]*vpshufhw \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 f5 ab[ 	]*vpshufhw \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e af 70 f5 ab[ 	]*vpshufhw \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 f5 7b[ 	]*vpshufhw \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 31 7b[ 	]*vpshufhw \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 72 7f 7b[ 	]*vpshufhw \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 b2 00 10 00 00 7b[ 	]*vpshufhw \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 72 80 7b[ 	]*vpshufhw \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 b2 e0 ef ff ff 7b[ 	]*vpshufhw \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 f5 ab[ 	]*vpshuflw \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 70 f5 ab[ 	]*vpshuflw \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 f5 7b[ 	]*vpshuflw \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 31 7b[ 	]*vpshuflw \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 72 7f 7b[ 	]*vpshuflw \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 b2 00 08 00 00 7b[ 	]*vpshuflw \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 72 80 7b[ 	]*vpshuflw \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 b2 f0 f7 ff ff 7b[ 	]*vpshuflw \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 f5 ab[ 	]*vpshuflw \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 70 f5 ab[ 	]*vpshuflw \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 f5 7b[ 	]*vpshuflw \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 31 7b[ 	]*vpshuflw \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 72 7f 7b[ 	]*vpshuflw \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 b2 00 10 00 00 7b[ 	]*vpshuflw \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 72 80 7b[ 	]*vpshuflw \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 b2 e0 ef ff ff 7b[ 	]*vpshuflw \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 f4[ 	]*vpsllw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f1 f4[ 	]*vpsllw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 31[ 	]*vpsllw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 72 7f[ 	]*vpsllw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 b2 00 08 00 00[ 	]*vpsllw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 72 80[ 	]*vpsllw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 b2 f0 f7 ff ff[ 	]*vpsllw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 f4[ 	]*vpsllw %xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f1 f4[ 	]*vpsllw %xmm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 31[ 	]*vpsllw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 72 7f[ 	]*vpsllw 0x7f0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 b2 00 08 00 00[ 	]*vpsllw 0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 72 80[ 	]*vpsllw -0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 b2 f0 f7 ff ff[ 	]*vpsllw -0x810\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 f4[ 	]*vpsraw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e1 f4[ 	]*vpsraw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 31[ 	]*vpsraw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 72 7f[ 	]*vpsraw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 b2 00 08 00 00[ 	]*vpsraw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 72 80[ 	]*vpsraw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 b2 f0 f7 ff ff[ 	]*vpsraw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 f4[ 	]*vpsraw %xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e1 f4[ 	]*vpsraw %xmm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 31[ 	]*vpsraw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 72 7f[ 	]*vpsraw 0x7f0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 b2 00 08 00 00[ 	]*vpsraw 0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 72 80[ 	]*vpsraw -0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 b2 f0 f7 ff ff[ 	]*vpsraw -0x810\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 f4[ 	]*vpsrlw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d1 f4[ 	]*vpsrlw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 31[ 	]*vpsrlw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 72 7f[ 	]*vpsrlw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 b2 00 08 00 00[ 	]*vpsrlw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 72 80[ 	]*vpsrlw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 b2 f0 f7 ff ff[ 	]*vpsrlw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 f4[ 	]*vpsrlw %xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d1 f4[ 	]*vpsrlw %xmm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 31[ 	]*vpsrlw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 72 7f[ 	]*vpsrlw 0x7f0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 b2 00 08 00 00[ 	]*vpsrlw 0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 72 80[ 	]*vpsrlw -0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 b2 f0 f7 ff ff[ 	]*vpsrlw -0x810\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 d5 ab[ 	]*vpsrlw \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 8f 71 d5 ab[ 	]*vpsrlw \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 d5 7b[ 	]*vpsrlw \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 11 7b[ 	]*vpsrlw \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 52 7f 7b[ 	]*vpsrlw \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 92 00 08 00 00 7b[ 	]*vpsrlw \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 52 80 7b[ 	]*vpsrlw \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 92 f0 f7 ff ff 7b[ 	]*vpsrlw \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 d5 ab[ 	]*vpsrlw \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d af 71 d5 ab[ 	]*vpsrlw \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 d5 7b[ 	]*vpsrlw \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 11 7b[ 	]*vpsrlw \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 52 7f 7b[ 	]*vpsrlw \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 92 00 10 00 00 7b[ 	]*vpsrlw \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 52 80 7b[ 	]*vpsrlw \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 92 e0 ef ff ff 7b[ 	]*vpsrlw \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 e5 ab[ 	]*vpsraw \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 8f 71 e5 ab[ 	]*vpsraw \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 e5 7b[ 	]*vpsraw \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 21 7b[ 	]*vpsraw \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 62 7f 7b[ 	]*vpsraw \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 a2 00 08 00 00 7b[ 	]*vpsraw \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 62 80 7b[ 	]*vpsraw \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 a2 f0 f7 ff ff 7b[ 	]*vpsraw \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 e5 ab[ 	]*vpsraw \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d af 71 e5 ab[ 	]*vpsraw \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 e5 7b[ 	]*vpsraw \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 21 7b[ 	]*vpsraw \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 62 7f 7b[ 	]*vpsraw \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 a2 00 10 00 00 7b[ 	]*vpsraw \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 62 80 7b[ 	]*vpsraw \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 a2 e0 ef ff ff 7b[ 	]*vpsraw \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 f4[ 	]*vpsrlvw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 10 f4[ 	]*vpsrlvw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 31[ 	]*vpsrlvw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 b4 f4 c0 1d fe ff[ 	]*vpsrlvw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 72 7f[ 	]*vpsrlvw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 b2 00 08 00 00[ 	]*vpsrlvw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 72 80[ 	]*vpsrlvw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 b2 f0 f7 ff ff[ 	]*vpsrlvw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 f4[ 	]*vpsrlvw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 10 f4[ 	]*vpsrlvw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 31[ 	]*vpsrlvw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 b4 f4 c0 1d fe ff[ 	]*vpsrlvw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 72 7f[ 	]*vpsrlvw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 b2 00 10 00 00[ 	]*vpsrlvw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 72 80[ 	]*vpsrlvw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 b2 e0 ef ff ff[ 	]*vpsrlvw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 f4[ 	]*vpsravw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 11 f4[ 	]*vpsravw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 31[ 	]*vpsravw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 b4 f4 c0 1d fe ff[ 	]*vpsravw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 72 7f[ 	]*vpsravw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 b2 00 08 00 00[ 	]*vpsravw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 72 80[ 	]*vpsravw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 b2 f0 f7 ff ff[ 	]*vpsravw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 f4[ 	]*vpsravw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 11 f4[ 	]*vpsravw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 31[ 	]*vpsravw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 b4 f4 c0 1d fe ff[ 	]*vpsravw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 72 7f[ 	]*vpsravw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 b2 00 10 00 00[ 	]*vpsravw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 72 80[ 	]*vpsravw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 b2 e0 ef ff ff[ 	]*vpsravw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 f4[ 	]*vpsubb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f8 f4[ 	]*vpsubb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 31[ 	]*vpsubb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 b4 f4 c0 1d fe ff[ 	]*vpsubb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 72 7f[ 	]*vpsubb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 b2 00 08 00 00[ 	]*vpsubb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 72 80[ 	]*vpsubb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 b2 f0 f7 ff ff[ 	]*vpsubb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 f4[ 	]*vpsubb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f8 f4[ 	]*vpsubb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 31[ 	]*vpsubb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 b4 f4 c0 1d fe ff[ 	]*vpsubb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 72 7f[ 	]*vpsubb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 b2 00 10 00 00[ 	]*vpsubb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 72 80[ 	]*vpsubb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 b2 e0 ef ff ff[ 	]*vpsubb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 f4[ 	]*vpsubsb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e8 f4[ 	]*vpsubsb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 31[ 	]*vpsubsb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 72 7f[ 	]*vpsubsb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 b2 00 08 00 00[ 	]*vpsubsb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 72 80[ 	]*vpsubsb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 b2 f0 f7 ff ff[ 	]*vpsubsb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 f4[ 	]*vpsubsb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e8 f4[ 	]*vpsubsb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 31[ 	]*vpsubsb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 72 7f[ 	]*vpsubsb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 b2 00 10 00 00[ 	]*vpsubsb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 72 80[ 	]*vpsubsb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 b2 e0 ef ff ff[ 	]*vpsubsb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 f4[ 	]*vpsubsw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e9 f4[ 	]*vpsubsw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 31[ 	]*vpsubsw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 72 7f[ 	]*vpsubsw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 b2 00 08 00 00[ 	]*vpsubsw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 72 80[ 	]*vpsubsw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 b2 f0 f7 ff ff[ 	]*vpsubsw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 f4[ 	]*vpsubsw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e9 f4[ 	]*vpsubsw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 31[ 	]*vpsubsw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 72 7f[ 	]*vpsubsw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 b2 00 10 00 00[ 	]*vpsubsw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 72 80[ 	]*vpsubsw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 b2 e0 ef ff ff[ 	]*vpsubsw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 f4[ 	]*vpsubusb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d8 f4[ 	]*vpsubusb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 31[ 	]*vpsubusb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 72 7f[ 	]*vpsubusb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 b2 00 08 00 00[ 	]*vpsubusb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 72 80[ 	]*vpsubusb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 b2 f0 f7 ff ff[ 	]*vpsubusb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 f4[ 	]*vpsubusb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d8 f4[ 	]*vpsubusb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 31[ 	]*vpsubusb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 72 7f[ 	]*vpsubusb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 b2 00 10 00 00[ 	]*vpsubusb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 72 80[ 	]*vpsubusb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 b2 e0 ef ff ff[ 	]*vpsubusb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 f4[ 	]*vpsubusw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d9 f4[ 	]*vpsubusw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 31[ 	]*vpsubusw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 72 7f[ 	]*vpsubusw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 b2 00 08 00 00[ 	]*vpsubusw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 72 80[ 	]*vpsubusw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 b2 f0 f7 ff ff[ 	]*vpsubusw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 f4[ 	]*vpsubusw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d9 f4[ 	]*vpsubusw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 31[ 	]*vpsubusw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 72 7f[ 	]*vpsubusw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 b2 00 10 00 00[ 	]*vpsubusw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 72 80[ 	]*vpsubusw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 b2 e0 ef ff ff[ 	]*vpsubusw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 f4[ 	]*vpsubw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f9 f4[ 	]*vpsubw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 31[ 	]*vpsubw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 b4 f4 c0 1d fe ff[ 	]*vpsubw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 72 7f[ 	]*vpsubw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 b2 00 08 00 00[ 	]*vpsubw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 72 80[ 	]*vpsubw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 b2 f0 f7 ff ff[ 	]*vpsubw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 f4[ 	]*vpsubw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f9 f4[ 	]*vpsubw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 31[ 	]*vpsubw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 b4 f4 c0 1d fe ff[ 	]*vpsubw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 72 7f[ 	]*vpsubw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 b2 00 10 00 00[ 	]*vpsubw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 72 80[ 	]*vpsubw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 b2 e0 ef ff ff[ 	]*vpsubw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 f4[ 	]*vpunpckhbw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 68 f4[ 	]*vpunpckhbw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 31[ 	]*vpunpckhbw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 72 7f[ 	]*vpunpckhbw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 b2 00 08 00 00[ 	]*vpunpckhbw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 72 80[ 	]*vpunpckhbw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 b2 f0 f7 ff ff[ 	]*vpunpckhbw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 f4[ 	]*vpunpckhbw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 68 f4[ 	]*vpunpckhbw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 31[ 	]*vpunpckhbw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 72 7f[ 	]*vpunpckhbw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 b2 00 10 00 00[ 	]*vpunpckhbw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 72 80[ 	]*vpunpckhbw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 b2 e0 ef ff ff[ 	]*vpunpckhbw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 f4[ 	]*vpunpckhwd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 69 f4[ 	]*vpunpckhwd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 31[ 	]*vpunpckhwd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 72 7f[ 	]*vpunpckhwd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 b2 00 08 00 00[ 	]*vpunpckhwd 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 72 80[ 	]*vpunpckhwd -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 b2 f0 f7 ff ff[ 	]*vpunpckhwd -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 f4[ 	]*vpunpckhwd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 69 f4[ 	]*vpunpckhwd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 31[ 	]*vpunpckhwd \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 72 7f[ 	]*vpunpckhwd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 b2 00 10 00 00[ 	]*vpunpckhwd 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 72 80[ 	]*vpunpckhwd -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 b2 e0 ef ff ff[ 	]*vpunpckhwd -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 f4[ 	]*vpunpcklbw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 60 f4[ 	]*vpunpcklbw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 31[ 	]*vpunpcklbw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 72 7f[ 	]*vpunpcklbw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 b2 00 08 00 00[ 	]*vpunpcklbw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 72 80[ 	]*vpunpcklbw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 b2 f0 f7 ff ff[ 	]*vpunpcklbw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 f4[ 	]*vpunpcklbw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 60 f4[ 	]*vpunpcklbw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 31[ 	]*vpunpcklbw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 72 7f[ 	]*vpunpcklbw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 b2 00 10 00 00[ 	]*vpunpcklbw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 72 80[ 	]*vpunpcklbw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 b2 e0 ef ff ff[ 	]*vpunpcklbw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 f4[ 	]*vpunpcklwd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 61 f4[ 	]*vpunpcklwd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 31[ 	]*vpunpcklwd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 72 7f[ 	]*vpunpcklwd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 b2 00 08 00 00[ 	]*vpunpcklwd 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 72 80[ 	]*vpunpcklwd -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 b2 f0 f7 ff ff[ 	]*vpunpcklwd -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 f4[ 	]*vpunpcklwd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 61 f4[ 	]*vpunpcklwd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 31[ 	]*vpunpcklwd \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 72 7f[ 	]*vpunpcklwd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 b2 00 10 00 00[ 	]*vpunpcklwd 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 72 80[ 	]*vpunpcklwd -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 b2 e0 ef ff ff[ 	]*vpunpcklwd -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 ee[ 	]*vpmovwb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 8f 30 ee[ 	]*vpmovwb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 ee[ 	]*vpmovwb %ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e af 30 ee[ 	]*vpmovwb %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 ee[ 	]*vpmovswb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 8f 20 ee[ 	]*vpmovswb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 ee[ 	]*vpmovswb %ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e af 20 ee[ 	]*vpmovswb %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 ee[ 	]*vpmovuswb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 8f 10 ee[ 	]*vpmovuswb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 ee[ 	]*vpmovuswb %ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e af 10 ee[ 	]*vpmovuswb %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 f4 ab[ 	]*vdbpsadbw \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 42 f4 ab[ 	]*vdbpsadbw \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 f4 7b[ 	]*vdbpsadbw \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 31 7b[ 	]*vdbpsadbw \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 b4 f4 c0 1d fe ff 7b[ 	]*vdbpsadbw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 72 7f 7b[ 	]*vdbpsadbw \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 b2 00 08 00 00 7b[ 	]*vdbpsadbw \$0x7b,0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 72 80 7b[ 	]*vdbpsadbw \$0x7b,-0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 b2 f0 f7 ff ff 7b[ 	]*vdbpsadbw \$0x7b,-0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 f4 ab[ 	]*vdbpsadbw \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 42 f4 ab[ 	]*vdbpsadbw \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 f4 7b[ 	]*vdbpsadbw \$0x7b,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 31 7b[ 	]*vdbpsadbw \$0x7b,\(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 b4 f4 c0 1d fe ff 7b[ 	]*vdbpsadbw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 72 7f 7b[ 	]*vdbpsadbw \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 b2 00 10 00 00 7b[ 	]*vdbpsadbw \$0x7b,0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 72 80 7b[ 	]*vdbpsadbw \$0x7b,-0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 b2 e0 ef ff ff 7b[ 	]*vdbpsadbw \$0x7b,-0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d f4[ 	]*vpermw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 8d f4[ 	]*vpermw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d 31[ 	]*vpermw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d b4 f4 c0 1d fe ff[ 	]*vpermw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d 72 7f[ 	]*vpermw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d b2 00 08 00 00[ 	]*vpermw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d 72 80[ 	]*vpermw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d b2 f0 f7 ff ff[ 	]*vpermw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d f4[ 	]*vpermw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 8d f4[ 	]*vpermw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d 31[ 	]*vpermw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d b4 f4 c0 1d fe ff[ 	]*vpermw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d 72 7f[ 	]*vpermw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d b2 00 10 00 00[ 	]*vpermw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d 72 80[ 	]*vpermw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d b2 e0 ef ff ff[ 	]*vpermw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d f4[ 	]*vpermt2w %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 7d f4[ 	]*vpermt2w %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d 31[ 	]*vpermt2w \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2w -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d 72 7f[ 	]*vpermt2w 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d b2 00 08 00 00[ 	]*vpermt2w 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d 72 80[ 	]*vpermt2w -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d b2 f0 f7 ff ff[ 	]*vpermt2w -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d f4[ 	]*vpermt2w %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 7d f4[ 	]*vpermt2w %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d 31[ 	]*vpermt2w \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2w -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d 72 7f[ 	]*vpermt2w 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d b2 00 10 00 00[ 	]*vpermt2w 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d 72 80[ 	]*vpermt2w -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d b2 e0 ef ff ff[ 	]*vpermt2w -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 f5 ab[ 	]*vpsllw \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 8f 71 f5 ab[ 	]*vpsllw \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 f5 7b[ 	]*vpsllw \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 31 7b[ 	]*vpsllw \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 72 7f 7b[ 	]*vpsllw \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 b2 00 08 00 00 7b[ 	]*vpsllw \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 72 80 7b[ 	]*vpsllw \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 b2 f0 f7 ff ff 7b[ 	]*vpsllw \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 f5 ab[ 	]*vpsllw \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d af 71 f5 ab[ 	]*vpsllw \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 f5 7b[ 	]*vpsllw \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 31 7b[ 	]*vpsllw \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 72 7f 7b[ 	]*vpsllw \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 b2 00 10 00 00 7b[ 	]*vpsllw \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 72 80 7b[ 	]*vpsllw \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 b2 e0 ef ff ff 7b[ 	]*vpsllw \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 f4[ 	]*vpsllvw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 12 f4[ 	]*vpsllvw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 31[ 	]*vpsllvw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 b4 f4 c0 1d fe ff[ 	]*vpsllvw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 72 7f[ 	]*vpsllvw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 b2 00 08 00 00[ 	]*vpsllvw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 72 80[ 	]*vpsllvw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 b2 f0 f7 ff ff[ 	]*vpsllvw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 f4[ 	]*vpsllvw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 12 f4[ 	]*vpsllvw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 31[ 	]*vpsllvw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 b4 f4 c0 1d fe ff[ 	]*vpsllvw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 72 7f[ 	]*vpsllvw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 b2 00 10 00 00[ 	]*vpsllvw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 72 80[ 	]*vpsllvw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 b2 e0 ef ff ff[ 	]*vpsllvw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f f5[ 	]*vmovdqu8 %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 6f f5[ 	]*vmovdqu8 %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f 31[ 	]*vmovdqu8 \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f 72 7f[ 	]*vmovdqu8 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f b2 00 08 00 00[ 	]*vmovdqu8 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f 72 80[ 	]*vmovdqu8 -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f b2 f0 f7 ff ff[ 	]*vmovdqu8 -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f f5[ 	]*vmovdqu8 %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 6f f5[ 	]*vmovdqu8 %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f 31[ 	]*vmovdqu8 \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f 72 7f[ 	]*vmovdqu8 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f b2 00 10 00 00[ 	]*vmovdqu8 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f 72 80[ 	]*vmovdqu8 -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f b2 e0 ef ff ff[ 	]*vmovdqu8 -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f f5[ 	]*vmovdqu16 %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 6f f5[ 	]*vmovdqu16 %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f 31[ 	]*vmovdqu16 \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f 72 7f[ 	]*vmovdqu16 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f b2 00 08 00 00[ 	]*vmovdqu16 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f 72 80[ 	]*vmovdqu16 -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f b2 f0 f7 ff ff[ 	]*vmovdqu16 -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f f5[ 	]*vmovdqu16 %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 6f f5[ 	]*vmovdqu16 %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f 31[ 	]*vmovdqu16 \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f 72 7f[ 	]*vmovdqu16 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f b2 00 10 00 00[ 	]*vmovdqu16 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f 72 80[ 	]*vmovdqu16 -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f b2 e0 ef ff ff[ 	]*vmovdqu16 -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 31[ 	]*vpmovwb %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 b4 f4 c0 1d fe ff[ 	]*vpmovwb %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 72 7f[ 	]*vpmovwb %xmm6,0x3f8\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 b2 00 04 00 00[ 	]*vpmovwb %xmm6,0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 72 80[ 	]*vpmovwb %xmm6,-0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 b2 f8 fb ff ff[ 	]*vpmovwb %xmm6,-0x408\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 31[ 	]*vpmovwb %ymm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 b4 f4 c0 1d fe ff[ 	]*vpmovwb %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 72 7f[ 	]*vpmovwb %ymm6,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 b2 00 08 00 00[ 	]*vpmovwb %ymm6,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 72 80[ 	]*vpmovwb %ymm6,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 b2 f0 f7 ff ff[ 	]*vpmovwb %ymm6,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 31[ 	]*vpmovswb %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 b4 f4 c0 1d fe ff[ 	]*vpmovswb %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 72 7f[ 	]*vpmovswb %xmm6,0x3f8\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 b2 00 04 00 00[ 	]*vpmovswb %xmm6,0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 72 80[ 	]*vpmovswb %xmm6,-0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 b2 f8 fb ff ff[ 	]*vpmovswb %xmm6,-0x408\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 31[ 	]*vpmovswb %ymm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 b4 f4 c0 1d fe ff[ 	]*vpmovswb %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 72 7f[ 	]*vpmovswb %ymm6,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 b2 00 08 00 00[ 	]*vpmovswb %ymm6,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 72 80[ 	]*vpmovswb %ymm6,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 b2 f0 f7 ff ff[ 	]*vpmovswb %ymm6,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 31[ 	]*vpmovuswb %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 b4 f4 c0 1d fe ff[ 	]*vpmovuswb %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 72 7f[ 	]*vpmovuswb %xmm6,0x3f8\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 b2 00 04 00 00[ 	]*vpmovuswb %xmm6,0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 72 80[ 	]*vpmovuswb %xmm6,-0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 b2 f8 fb ff ff[ 	]*vpmovuswb %xmm6,-0x408\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 31[ 	]*vpmovuswb %ymm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 b4 f4 c0 1d fe ff[ 	]*vpmovuswb %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 72 7f[ 	]*vpmovuswb %ymm6,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 b2 00 08 00 00[ 	]*vpmovuswb %ymm6,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 72 80[ 	]*vpmovuswb %ymm6,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 b2 f0 f7 ff ff[ 	]*vpmovuswb %ymm6,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f 31[ 	]*vmovdqu8 %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f 72 7f[ 	]*vmovdqu8 %xmm6,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f b2 00 08 00 00[ 	]*vmovdqu8 %xmm6,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f 72 80[ 	]*vmovdqu8 %xmm6,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f b2 f0 f7 ff ff[ 	]*vmovdqu8 %xmm6,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f 31[ 	]*vmovdqu8 %ymm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f 72 7f[ 	]*vmovdqu8 %ymm6,0xfe0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f b2 00 10 00 00[ 	]*vmovdqu8 %ymm6,0x1000\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f 72 80[ 	]*vmovdqu8 %ymm6,-0x1000\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f b2 e0 ef ff ff[ 	]*vmovdqu8 %ymm6,-0x1020\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f 31[ 	]*vmovdqu16 %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f 72 7f[ 	]*vmovdqu16 %xmm6,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f b2 00 08 00 00[ 	]*vmovdqu16 %xmm6,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f 72 80[ 	]*vmovdqu16 %xmm6,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f b2 f0 f7 ff ff[ 	]*vmovdqu16 %xmm6,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f 31[ 	]*vmovdqu16 %ymm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f 72 7f[ 	]*vmovdqu16 %ymm6,0xfe0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f b2 00 10 00 00[ 	]*vmovdqu16 %ymm6,0x1000\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f 72 80[ 	]*vmovdqu16 %ymm6,-0x1000\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f b2 e0 ef ff ff[ 	]*vmovdqu16 %ymm6,-0x1020\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 f4[ 	]*vpermi2w %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 75 f4[ 	]*vpermi2w %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 31[ 	]*vpermi2w \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2w -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 72 7f[ 	]*vpermi2w 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 b2 00 08 00 00[ 	]*vpermi2w 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 72 80[ 	]*vpermi2w -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 b2 f0 f7 ff ff[ 	]*vpermi2w -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 f4[ 	]*vpermi2w %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 75 f4[ 	]*vpermi2w %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 31[ 	]*vpermi2w \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2w -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 72 7f[ 	]*vpermi2w 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 b2 00 10 00 00[ 	]*vpermi2w 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 72 80[ 	]*vpermi2w -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 b2 e0 ef ff ff[ 	]*vpermi2w -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 ed[ 	]*vptestmb %xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 29[ 	]*vptestmb \(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 ac f4 c0 1d fe ff[ 	]*vptestmb -0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 6a 7f[ 	]*vptestmb 0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 aa 00 08 00 00[ 	]*vptestmb 0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 6a 80[ 	]*vptestmb -0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 aa f0 f7 ff ff[ 	]*vptestmb -0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 ed[ 	]*vptestmb %ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 29[ 	]*vptestmb \(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 ac f4 c0 1d fe ff[ 	]*vptestmb -0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 6a 7f[ 	]*vptestmb 0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 aa 00 10 00 00[ 	]*vptestmb 0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 6a 80[ 	]*vptestmb -0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 aa e0 ef ff ff[ 	]*vptestmb -0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 ed[ 	]*vptestmw %xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 29[ 	]*vptestmw \(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 ac f4 c0 1d fe ff[ 	]*vptestmw -0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 6a 7f[ 	]*vptestmw 0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 aa 00 08 00 00[ 	]*vptestmw 0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 6a 80[ 	]*vptestmw -0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 aa f0 f7 ff ff[ 	]*vptestmw -0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 ed[ 	]*vptestmw %ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 29[ 	]*vptestmw \(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 ac f4 c0 1d fe ff[ 	]*vptestmw -0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 6a 7f[ 	]*vptestmw 0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 aa 00 10 00 00[ 	]*vptestmw 0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 6a 80[ 	]*vptestmw -0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 aa e0 ef ff ff[ 	]*vptestmw -0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 29 ee[ 	]*vpmovb2m %xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 29 ee[ 	]*vpmovb2m %ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 29 ee[ 	]*vpmovw2m %xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 29 ee[ 	]*vpmovw2m %ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 28 f5[ 	]*vpmovm2b %k5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 28 f5[ 	]*vpmovm2b %k5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 28 f5[ 	]*vpmovm2w %k5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 28 f5[ 	]*vpmovm2w %k5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 ec[ 	]*vptestnmb %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 29[ 	]*vptestnmb \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 ac f4 c0 1d fe ff[ 	]*vptestnmb -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 6a 7f[ 	]*vptestnmb 0x7f0\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 aa 00 08 00 00[ 	]*vptestnmb 0x800\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 6a 80[ 	]*vptestnmb -0x800\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 aa f0 f7 ff ff[ 	]*vptestnmb -0x810\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 ec[ 	]*vptestnmb %ymm4,%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 29[ 	]*vptestnmb \(%ecx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 ac f4 c0 1d fe ff[ 	]*vptestnmb -0x1e240\(%esp,%esi,8\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 6a 7f[ 	]*vptestnmb 0xfe0\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 aa 00 10 00 00[ 	]*vptestnmb 0x1000\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 6a 80[ 	]*vptestnmb -0x1000\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 aa e0 ef ff ff[ 	]*vptestnmb -0x1020\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 ec[ 	]*vptestnmw %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 29[ 	]*vptestnmw \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 ac f4 c0 1d fe ff[ 	]*vptestnmw -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 6a 7f[ 	]*vptestnmw 0x7f0\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 aa 00 08 00 00[ 	]*vptestnmw 0x800\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 6a 80[ 	]*vptestnmw -0x800\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 aa f0 f7 ff ff[ 	]*vptestnmw -0x810\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 ec[ 	]*vptestnmw %ymm4,%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 29[ 	]*vptestnmw \(%ecx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 ac f4 c0 1d fe ff[ 	]*vptestnmw -0x1e240\(%esp,%esi,8\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 6a 7f[ 	]*vptestnmw 0xfe0\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 aa 00 10 00 00[ 	]*vptestnmw 0x1000\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 6a 80[ 	]*vptestnmw -0x1000\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 aa e0 ef ff ff[ 	]*vptestnmw -0x1020\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f ed ab[ 	]*vpcmpb \$0xab,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f ed 7b[ 	]*vpcmpb \$0x7b,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f 29 7b[ 	]*vpcmpb \$0x7b,\(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpb \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f 6a 7f 7b[ 	]*vpcmpb \$0x7b,0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f aa 00 08 00 00 7b[ 	]*vpcmpb \$0x7b,0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f 6a 80 7b[ 	]*vpcmpb \$0x7b,-0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f aa f0 f7 ff ff 7b[ 	]*vpcmpb \$0x7b,-0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f ed ab[ 	]*vpcmpb \$0xab,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f ed 7b[ 	]*vpcmpb \$0x7b,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f 29 7b[ 	]*vpcmpb \$0x7b,\(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpb \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f 6a 7f 7b[ 	]*vpcmpb \$0x7b,0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f aa 00 10 00 00 7b[ 	]*vpcmpb \$0x7b,0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f 6a 80 7b[ 	]*vpcmpb \$0x7b,-0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f aa e0 ef ff ff 7b[ 	]*vpcmpb \$0x7b,-0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f ed 00[ 	]*vpcmpeqb %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f ed 00[ 	]*vpcmpeqb %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f ed 02[ 	]*vpcmpleb %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f 68 7f 02[ 	]*vpcmpleb 0x7f0\(%eax\),%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f a8 00 08 00 00 02[ 	]*vpcmpleb 0x800\(%eax\),%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f ed 02[ 	]*vpcmpleb %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f 68 7f 02[ 	]*vpcmpleb 0xfe0\(%eax\),%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f a8 00 10 00 00 02[ 	]*vpcmpleb 0x1000\(%eax\),%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f ed 01[ 	]*vpcmpltb %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f ed 01[ 	]*vpcmpltb %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f ed 04[ 	]*vpcmpneqb %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f ed 04[ 	]*vpcmpneqb %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f ed 06[ 	]*vpcmpnleb %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f ed 06[ 	]*vpcmpnleb %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f ed 05[ 	]*vpcmpnltb %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f ed 05[ 	]*vpcmpnltb %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f ed ab[ 	]*vpcmpw \$0xab,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f ed 7b[ 	]*vpcmpw \$0x7b,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f 29 7b[ 	]*vpcmpw \$0x7b,\(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f 6a 7f 7b[ 	]*vpcmpw \$0x7b,0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f aa 00 08 00 00 7b[ 	]*vpcmpw \$0x7b,0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f 6a 80 7b[ 	]*vpcmpw \$0x7b,-0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f aa f0 f7 ff ff 7b[ 	]*vpcmpw \$0x7b,-0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f ed ab[ 	]*vpcmpw \$0xab,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f ed 7b[ 	]*vpcmpw \$0x7b,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f 29 7b[ 	]*vpcmpw \$0x7b,\(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f 6a 7f 7b[ 	]*vpcmpw \$0x7b,0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f aa 00 10 00 00 7b[ 	]*vpcmpw \$0x7b,0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f 6a 80 7b[ 	]*vpcmpw \$0x7b,-0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f aa e0 ef ff ff 7b[ 	]*vpcmpw \$0x7b,-0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f ed 00[ 	]*vpcmpeqw %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f ed 00[ 	]*vpcmpeqw %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f ed 02[ 	]*vpcmplew %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f 68 7f 02[ 	]*vpcmplew 0x7f0\(%eax\),%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f a8 00 08 00 00 02[ 	]*vpcmplew 0x800\(%eax\),%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f ed 02[ 	]*vpcmplew %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f 68 7f 02[ 	]*vpcmplew 0xfe0\(%eax\),%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f a8 00 10 00 00 02[ 	]*vpcmplew 0x1000\(%eax\),%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f ed 01[ 	]*vpcmpltw %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f ed 01[ 	]*vpcmpltw %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f ed 04[ 	]*vpcmpneqw %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f ed 04[ 	]*vpcmpneqw %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f ed 06[ 	]*vpcmpnlew %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f ed 06[ 	]*vpcmpnlew %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f ed 05[ 	]*vpcmpnltw %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f ed 05[ 	]*vpcmpnltw %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e ed ab[ 	]*vpcmpub \$0xab,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e ed 7b[ 	]*vpcmpub \$0x7b,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e 29 7b[ 	]*vpcmpub \$0x7b,\(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpub \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e 6a 7f 7b[ 	]*vpcmpub \$0x7b,0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e aa 00 08 00 00 7b[ 	]*vpcmpub \$0x7b,0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e 6a 80 7b[ 	]*vpcmpub \$0x7b,-0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e aa f0 f7 ff ff 7b[ 	]*vpcmpub \$0x7b,-0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e ed ab[ 	]*vpcmpub \$0xab,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e ed 7b[ 	]*vpcmpub \$0x7b,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e 29 7b[ 	]*vpcmpub \$0x7b,\(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpub \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e 6a 7f 7b[ 	]*vpcmpub \$0x7b,0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e aa 00 10 00 00 7b[ 	]*vpcmpub \$0x7b,0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e 6a 80 7b[ 	]*vpcmpub \$0x7b,-0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e aa e0 ef ff ff 7b[ 	]*vpcmpub \$0x7b,-0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3e ed 00[ 	]*vpcmpequb %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3e ed 00[ 	]*vpcmpequb %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3e ed 02[ 	]*vpcmpleub %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3e ed 02[ 	]*vpcmpleub %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3e ed 01[ 	]*vpcmpltub %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3e ed 01[ 	]*vpcmpltub %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3e ed 04[ 	]*vpcmpnequb %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3e ed 04[ 	]*vpcmpnequb %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3e ed 06[ 	]*vpcmpnleub %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3e ed 06[ 	]*vpcmpnleub %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3e ed 05[ 	]*vpcmpnltub %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3e ed 05[ 	]*vpcmpnltub %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e ed ab[ 	]*vpcmpuw \$0xab,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e ed 7b[ 	]*vpcmpuw \$0x7b,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e 29 7b[ 	]*vpcmpuw \$0x7b,\(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpuw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e 6a 7f 7b[ 	]*vpcmpuw \$0x7b,0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e aa 00 08 00 00 7b[ 	]*vpcmpuw \$0x7b,0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e 6a 80 7b[ 	]*vpcmpuw \$0x7b,-0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e aa f0 f7 ff ff 7b[ 	]*vpcmpuw \$0x7b,-0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e ed ab[ 	]*vpcmpuw \$0xab,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e ed 7b[ 	]*vpcmpuw \$0x7b,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e 29 7b[ 	]*vpcmpuw \$0x7b,\(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpuw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e 6a 7f 7b[ 	]*vpcmpuw \$0x7b,0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e aa 00 10 00 00 7b[ 	]*vpcmpuw \$0x7b,0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e 6a 80 7b[ 	]*vpcmpuw \$0x7b,-0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e aa e0 ef ff ff 7b[ 	]*vpcmpuw \$0x7b,-0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3e ed 00[ 	]*vpcmpequw %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3e ed 00[ 	]*vpcmpequw %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3e ed 02[ 	]*vpcmpleuw %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3e ed 02[ 	]*vpcmpleuw %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3e ed 01[ 	]*vpcmpltuw %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3e ed 01[ 	]*vpcmpltuw %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3e ed 04[ 	]*vpcmpnequw %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3e ed 04[ 	]*vpcmpnequw %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3e ed 06[ 	]*vpcmpnleuw %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3e ed 06[ 	]*vpcmpnleuw %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3e ed 05[ 	]*vpcmpnltuw %xmm5,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3e ed 05[ 	]*vpcmpnltuw %ymm5,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c f5[ 	]*vpabsb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 1c f5[ 	]*vpabsb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c 31[ 	]*vpabsb \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c b4 f4 c0 1d fe ff[ 	]*vpabsb -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c 72 7f[ 	]*vpabsb 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c b2 00 08 00 00[ 	]*vpabsb 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c 72 80[ 	]*vpabsb -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c b2 f0 f7 ff ff[ 	]*vpabsb -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c f5[ 	]*vpabsb %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 1c f5[ 	]*vpabsb %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c 31[ 	]*vpabsb \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c b4 f4 c0 1d fe ff[ 	]*vpabsb -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c 72 7f[ 	]*vpabsb 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c b2 00 10 00 00[ 	]*vpabsb 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c 72 80[ 	]*vpabsb -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c b2 e0 ef ff ff[ 	]*vpabsb -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d f5[ 	]*vpabsw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 1d f5[ 	]*vpabsw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d 31[ 	]*vpabsw \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d b4 f4 c0 1d fe ff[ 	]*vpabsw -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d 72 7f[ 	]*vpabsw 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d b2 00 08 00 00[ 	]*vpabsw 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d 72 80[ 	]*vpabsw -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d b2 f0 f7 ff ff[ 	]*vpabsw -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d f5[ 	]*vpabsw %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 1d f5[ 	]*vpabsw %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d 31[ 	]*vpabsw \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d b4 f4 c0 1d fe ff[ 	]*vpabsw -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d 72 7f[ 	]*vpabsw 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d b2 00 10 00 00[ 	]*vpabsw 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d 72 80[ 	]*vpabsw -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d b2 e0 ef ff ff[ 	]*vpabsw -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b f4[ 	]*vpackssdw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 6b f4[ 	]*vpackssdw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b 31[ 	]*vpackssdw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b b4 f4 c0 1d fe ff[ 	]*vpackssdw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b 30[ 	]*vpackssdw \(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b 72 7f[ 	]*vpackssdw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b b2 00 08 00 00[ 	]*vpackssdw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b 72 80[ 	]*vpackssdw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b b2 f0 f7 ff ff[ 	]*vpackssdw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b 72 7f[ 	]*vpackssdw 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b b2 00 02 00 00[ 	]*vpackssdw 0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b 72 80[ 	]*vpackssdw -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b b2 fc fd ff ff[ 	]*vpackssdw -0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b f4[ 	]*vpackssdw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 6b f4[ 	]*vpackssdw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b 31[ 	]*vpackssdw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b b4 f4 c0 1d fe ff[ 	]*vpackssdw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b 30[ 	]*vpackssdw \(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b 72 7f[ 	]*vpackssdw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b b2 00 10 00 00[ 	]*vpackssdw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b 72 80[ 	]*vpackssdw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b b2 e0 ef ff ff[ 	]*vpackssdw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b 72 7f[ 	]*vpackssdw 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b b2 00 02 00 00[ 	]*vpackssdw 0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b 72 80[ 	]*vpackssdw -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b b2 fc fd ff ff[ 	]*vpackssdw -0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 f4[ 	]*vpacksswb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 63 f4[ 	]*vpacksswb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 31[ 	]*vpacksswb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 72 7f[ 	]*vpacksswb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 b2 00 08 00 00[ 	]*vpacksswb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 72 80[ 	]*vpacksswb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 b2 f0 f7 ff ff[ 	]*vpacksswb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 f4[ 	]*vpacksswb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 63 f4[ 	]*vpacksswb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 31[ 	]*vpacksswb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 72 7f[ 	]*vpacksswb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 b2 00 10 00 00[ 	]*vpacksswb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 72 80[ 	]*vpacksswb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 b2 e0 ef ff ff[ 	]*vpacksswb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b f4[ 	]*vpackusdw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 2b f4[ 	]*vpackusdw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b 31[ 	]*vpackusdw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b b4 f4 c0 1d fe ff[ 	]*vpackusdw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b 30[ 	]*vpackusdw \(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b 72 7f[ 	]*vpackusdw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b b2 00 08 00 00[ 	]*vpackusdw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b 72 80[ 	]*vpackusdw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b b2 f0 f7 ff ff[ 	]*vpackusdw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b 72 7f[ 	]*vpackusdw 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b b2 00 02 00 00[ 	]*vpackusdw 0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b 72 80[ 	]*vpackusdw -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b b2 fc fd ff ff[ 	]*vpackusdw -0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b f4[ 	]*vpackusdw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 2b f4[ 	]*vpackusdw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b 31[ 	]*vpackusdw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b b4 f4 c0 1d fe ff[ 	]*vpackusdw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b 30[ 	]*vpackusdw \(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b 72 7f[ 	]*vpackusdw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b b2 00 10 00 00[ 	]*vpackusdw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b 72 80[ 	]*vpackusdw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b b2 e0 ef ff ff[ 	]*vpackusdw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b 72 7f[ 	]*vpackusdw 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b b2 00 02 00 00[ 	]*vpackusdw 0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b 72 80[ 	]*vpackusdw -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b b2 fc fd ff ff[ 	]*vpackusdw -0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 f4[ 	]*vpackuswb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 67 f4[ 	]*vpackuswb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 31[ 	]*vpackuswb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 72 7f[ 	]*vpackuswb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 b2 00 08 00 00[ 	]*vpackuswb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 72 80[ 	]*vpackuswb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 b2 f0 f7 ff ff[ 	]*vpackuswb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 f4[ 	]*vpackuswb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 67 f4[ 	]*vpackuswb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 31[ 	]*vpackuswb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 72 7f[ 	]*vpackuswb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 b2 00 10 00 00[ 	]*vpackuswb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 72 80[ 	]*vpackuswb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 b2 e0 ef ff ff[ 	]*vpackuswb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc f4[ 	]*vpaddb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f fc f4[ 	]*vpaddb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc 31[ 	]*vpaddb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc b4 f4 c0 1d fe ff[ 	]*vpaddb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc 72 7f[ 	]*vpaddb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc b2 00 08 00 00[ 	]*vpaddb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc 72 80[ 	]*vpaddb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc b2 f0 f7 ff ff[ 	]*vpaddb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc f4[ 	]*vpaddb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af fc f4[ 	]*vpaddb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc 31[ 	]*vpaddb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc b4 f4 c0 1d fe ff[ 	]*vpaddb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc 72 7f[ 	]*vpaddb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc b2 00 10 00 00[ 	]*vpaddb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc 72 80[ 	]*vpaddb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc b2 e0 ef ff ff[ 	]*vpaddb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec f4[ 	]*vpaddsb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ec f4[ 	]*vpaddsb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec 31[ 	]*vpaddsb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec b4 f4 c0 1d fe ff[ 	]*vpaddsb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec 72 7f[ 	]*vpaddsb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec b2 00 08 00 00[ 	]*vpaddsb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec 72 80[ 	]*vpaddsb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec b2 f0 f7 ff ff[ 	]*vpaddsb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec f4[ 	]*vpaddsb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ec f4[ 	]*vpaddsb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec 31[ 	]*vpaddsb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec b4 f4 c0 1d fe ff[ 	]*vpaddsb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec 72 7f[ 	]*vpaddsb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec b2 00 10 00 00[ 	]*vpaddsb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec 72 80[ 	]*vpaddsb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec b2 e0 ef ff ff[ 	]*vpaddsb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed f4[ 	]*vpaddsw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ed f4[ 	]*vpaddsw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed 31[ 	]*vpaddsw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed b4 f4 c0 1d fe ff[ 	]*vpaddsw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed 72 7f[ 	]*vpaddsw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed b2 00 08 00 00[ 	]*vpaddsw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed 72 80[ 	]*vpaddsw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed b2 f0 f7 ff ff[ 	]*vpaddsw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed f4[ 	]*vpaddsw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ed f4[ 	]*vpaddsw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed 31[ 	]*vpaddsw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed b4 f4 c0 1d fe ff[ 	]*vpaddsw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed 72 7f[ 	]*vpaddsw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed b2 00 10 00 00[ 	]*vpaddsw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed 72 80[ 	]*vpaddsw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed b2 e0 ef ff ff[ 	]*vpaddsw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc f4[ 	]*vpaddusb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f dc f4[ 	]*vpaddusb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc 31[ 	]*vpaddusb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc b4 f4 c0 1d fe ff[ 	]*vpaddusb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc 72 7f[ 	]*vpaddusb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc b2 00 08 00 00[ 	]*vpaddusb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc 72 80[ 	]*vpaddusb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc b2 f0 f7 ff ff[ 	]*vpaddusb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc f4[ 	]*vpaddusb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af dc f4[ 	]*vpaddusb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc 31[ 	]*vpaddusb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc b4 f4 c0 1d fe ff[ 	]*vpaddusb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc 72 7f[ 	]*vpaddusb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc b2 00 10 00 00[ 	]*vpaddusb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc 72 80[ 	]*vpaddusb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc b2 e0 ef ff ff[ 	]*vpaddusb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd f4[ 	]*vpaddusw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f dd f4[ 	]*vpaddusw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd 31[ 	]*vpaddusw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd b4 f4 c0 1d fe ff[ 	]*vpaddusw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd 72 7f[ 	]*vpaddusw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd b2 00 08 00 00[ 	]*vpaddusw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd 72 80[ 	]*vpaddusw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd b2 f0 f7 ff ff[ 	]*vpaddusw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd f4[ 	]*vpaddusw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af dd f4[ 	]*vpaddusw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd 31[ 	]*vpaddusw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd b4 f4 c0 1d fe ff[ 	]*vpaddusw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd 72 7f[ 	]*vpaddusw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd b2 00 10 00 00[ 	]*vpaddusw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd 72 80[ 	]*vpaddusw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd b2 e0 ef ff ff[ 	]*vpaddusw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd f4[ 	]*vpaddw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f fd f4[ 	]*vpaddw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd 31[ 	]*vpaddw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd b4 f4 c0 1d fe ff[ 	]*vpaddw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd 72 7f[ 	]*vpaddw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd b2 00 08 00 00[ 	]*vpaddw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd 72 80[ 	]*vpaddw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd b2 f0 f7 ff ff[ 	]*vpaddw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd f4[ 	]*vpaddw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af fd f4[ 	]*vpaddw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd 31[ 	]*vpaddw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd b4 f4 c0 1d fe ff[ 	]*vpaddw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd 72 7f[ 	]*vpaddw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd b2 00 10 00 00[ 	]*vpaddw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd 72 80[ 	]*vpaddw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd b2 e0 ef ff ff[ 	]*vpaddw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f f4 ab[ 	]*vpalignr \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 0f f4 ab[ 	]*vpalignr \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f f4 7b[ 	]*vpalignr \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f 31 7b[ 	]*vpalignr \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f 72 7f 7b[ 	]*vpalignr \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f b2 00 08 00 00 7b[ 	]*vpalignr \$0x7b,0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f 72 80 7b[ 	]*vpalignr \$0x7b,-0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f b2 f0 f7 ff ff 7b[ 	]*vpalignr \$0x7b,-0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f f4 ab[ 	]*vpalignr \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 0f f4 ab[ 	]*vpalignr \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f f4 7b[ 	]*vpalignr \$0x7b,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f 31 7b[ 	]*vpalignr \$0x7b,\(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f 72 7f 7b[ 	]*vpalignr \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f b2 00 10 00 00 7b[ 	]*vpalignr \$0x7b,0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f 72 80 7b[ 	]*vpalignr \$0x7b,-0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f b2 e0 ef ff ff 7b[ 	]*vpalignr \$0x7b,-0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 f4[ 	]*vpavgb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e0 f4[ 	]*vpavgb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 31[ 	]*vpavgb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 b4 f4 c0 1d fe ff[ 	]*vpavgb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 72 7f[ 	]*vpavgb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 b2 00 08 00 00[ 	]*vpavgb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 72 80[ 	]*vpavgb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 b2 f0 f7 ff ff[ 	]*vpavgb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 f4[ 	]*vpavgb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e0 f4[ 	]*vpavgb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 31[ 	]*vpavgb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 b4 f4 c0 1d fe ff[ 	]*vpavgb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 72 7f[ 	]*vpavgb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 b2 00 10 00 00[ 	]*vpavgb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 72 80[ 	]*vpavgb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 b2 e0 ef ff ff[ 	]*vpavgb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 f4[ 	]*vpavgw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e3 f4[ 	]*vpavgw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 31[ 	]*vpavgw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 b4 f4 c0 1d fe ff[ 	]*vpavgw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 72 7f[ 	]*vpavgw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 b2 00 08 00 00[ 	]*vpavgw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 72 80[ 	]*vpavgw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 b2 f0 f7 ff ff[ 	]*vpavgw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 f4[ 	]*vpavgw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e3 f4[ 	]*vpavgw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 31[ 	]*vpavgw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 b4 f4 c0 1d fe ff[ 	]*vpavgw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 72 7f[ 	]*vpavgw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 b2 00 10 00 00[ 	]*vpavgw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 72 80[ 	]*vpavgw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 b2 e0 ef ff ff[ 	]*vpavgw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 f4[ 	]*vpblendmb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 66 f4[ 	]*vpblendmb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 31[ 	]*vpblendmb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 72 7f[ 	]*vpblendmb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 b2 00 08 00 00[ 	]*vpblendmb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 72 80[ 	]*vpblendmb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 b2 f0 f7 ff ff[ 	]*vpblendmb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 f4[ 	]*vpblendmb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 66 f4[ 	]*vpblendmb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 31[ 	]*vpblendmb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 72 7f[ 	]*vpblendmb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 b2 00 10 00 00[ 	]*vpblendmb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 72 80[ 	]*vpblendmb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 b2 e0 ef ff ff[ 	]*vpblendmb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 f5[ 	]*vpbroadcastb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 78 f5[ 	]*vpbroadcastb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 31[ 	]*vpbroadcastb \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 b4 f4 c0 1d fe ff[ 	]*vpbroadcastb -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 72 7f[ 	]*vpbroadcastb 0x7f\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 b2 80 00 00 00[ 	]*vpbroadcastb 0x80\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 72 80[ 	]*vpbroadcastb -0x80\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 b2 7f ff ff ff[ 	]*vpbroadcastb -0x81\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 f5[ 	]*vpbroadcastb %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 78 f5[ 	]*vpbroadcastb %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 31[ 	]*vpbroadcastb \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 b4 f4 c0 1d fe ff[ 	]*vpbroadcastb -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 72 7f[ 	]*vpbroadcastb 0x7f\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 b2 80 00 00 00[ 	]*vpbroadcastb 0x80\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 72 80[ 	]*vpbroadcastb -0x80\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 b2 7f ff ff ff[ 	]*vpbroadcastb -0x81\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7a f0[ 	]*vpbroadcastb %eax,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 7a f0[ 	]*vpbroadcastb %eax,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7a f5[ 	]*vpbroadcastb %ebp,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7a f0[ 	]*vpbroadcastb %eax,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 7a f0[ 	]*vpbroadcastb %eax,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7a f5[ 	]*vpbroadcastb %ebp,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 f5[ 	]*vpbroadcastw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 79 f5[ 	]*vpbroadcastw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 31[ 	]*vpbroadcastw \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 b4 f4 c0 1d fe ff[ 	]*vpbroadcastw -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 72 7f[ 	]*vpbroadcastw 0xfe\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 b2 00 01 00 00[ 	]*vpbroadcastw 0x100\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 72 80[ 	]*vpbroadcastw -0x100\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 b2 fe fe ff ff[ 	]*vpbroadcastw -0x102\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 f5[ 	]*vpbroadcastw %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 79 f5[ 	]*vpbroadcastw %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 31[ 	]*vpbroadcastw \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 b4 f4 c0 1d fe ff[ 	]*vpbroadcastw -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 72 7f[ 	]*vpbroadcastw 0xfe\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 b2 00 01 00 00[ 	]*vpbroadcastw 0x100\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 72 80[ 	]*vpbroadcastw -0x100\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 b2 fe fe ff ff[ 	]*vpbroadcastw -0x102\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7b f0[ 	]*vpbroadcastw %eax,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 7b f0[ 	]*vpbroadcastw %eax,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7b f5[ 	]*vpbroadcastw %ebp,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7b f0[ 	]*vpbroadcastw %eax,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 7b f0[ 	]*vpbroadcastw %eax,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7b f5[ 	]*vpbroadcastw %ebp,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 ed[ 	]*vpcmpeqb %xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 29[ 	]*vpcmpeqb \(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb -0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 6a 7f[ 	]*vpcmpeqb 0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 aa 00 08 00 00[ 	]*vpcmpeqb 0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 6a 80[ 	]*vpcmpeqb -0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 aa f0 f7 ff ff[ 	]*vpcmpeqb -0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 ed[ 	]*vpcmpeqb %ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 29[ 	]*vpcmpeqb \(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb -0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 6a 7f[ 	]*vpcmpeqb 0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 aa 00 10 00 00[ 	]*vpcmpeqb 0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 6a 80[ 	]*vpcmpeqb -0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 aa e0 ef ff ff[ 	]*vpcmpeqb -0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 ed[ 	]*vpcmpeqw %xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 29[ 	]*vpcmpeqw \(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw -0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 6a 7f[ 	]*vpcmpeqw 0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 aa 00 08 00 00[ 	]*vpcmpeqw 0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 6a 80[ 	]*vpcmpeqw -0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 aa f0 f7 ff ff[ 	]*vpcmpeqw -0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 ed[ 	]*vpcmpeqw %ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 29[ 	]*vpcmpeqw \(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw -0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 6a 7f[ 	]*vpcmpeqw 0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 aa 00 10 00 00[ 	]*vpcmpeqw 0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 6a 80[ 	]*vpcmpeqw -0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 aa e0 ef ff ff[ 	]*vpcmpeqw -0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 ed[ 	]*vpcmpgtb %xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 29[ 	]*vpcmpgtb \(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb -0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 6a 7f[ 	]*vpcmpgtb 0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 aa 00 08 00 00[ 	]*vpcmpgtb 0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 6a 80[ 	]*vpcmpgtb -0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 aa f0 f7 ff ff[ 	]*vpcmpgtb -0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 ed[ 	]*vpcmpgtb %ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 29[ 	]*vpcmpgtb \(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb -0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 6a 7f[ 	]*vpcmpgtb 0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 aa 00 10 00 00[ 	]*vpcmpgtb 0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 6a 80[ 	]*vpcmpgtb -0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 aa e0 ef ff ff[ 	]*vpcmpgtb -0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 ed[ 	]*vpcmpgtw %xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 29[ 	]*vpcmpgtw \(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw -0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 6a 7f[ 	]*vpcmpgtw 0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 aa 00 08 00 00[ 	]*vpcmpgtw 0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 6a 80[ 	]*vpcmpgtw -0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 aa f0 f7 ff ff[ 	]*vpcmpgtw -0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 ed[ 	]*vpcmpgtw %ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 29[ 	]*vpcmpgtw \(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw -0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 6a 7f[ 	]*vpcmpgtw 0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 aa 00 10 00 00[ 	]*vpcmpgtw 0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 6a 80[ 	]*vpcmpgtw -0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 aa e0 ef ff ff[ 	]*vpcmpgtw -0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 f4[ 	]*vpblendmw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 66 f4[ 	]*vpblendmw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 31[ 	]*vpblendmw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 72 7f[ 	]*vpblendmw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 b2 00 08 00 00[ 	]*vpblendmw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 72 80[ 	]*vpblendmw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 b2 f0 f7 ff ff[ 	]*vpblendmw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 f4[ 	]*vpblendmw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 66 f4[ 	]*vpblendmw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 31[ 	]*vpblendmw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 72 7f[ 	]*vpblendmw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 b2 00 10 00 00[ 	]*vpblendmw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 72 80[ 	]*vpblendmw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 b2 e0 ef ff ff[ 	]*vpblendmw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 f4[ 	]*vpmaddubsw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 04 f4[ 	]*vpmaddubsw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 31[ 	]*vpmaddubsw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 72 7f[ 	]*vpmaddubsw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 b2 00 08 00 00[ 	]*vpmaddubsw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 72 80[ 	]*vpmaddubsw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 b2 f0 f7 ff ff[ 	]*vpmaddubsw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 f4[ 	]*vpmaddubsw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 04 f4[ 	]*vpmaddubsw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 31[ 	]*vpmaddubsw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 72 7f[ 	]*vpmaddubsw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 b2 00 10 00 00[ 	]*vpmaddubsw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 72 80[ 	]*vpmaddubsw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 b2 e0 ef ff ff[ 	]*vpmaddubsw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 f4[ 	]*vpmaddwd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f5 f4[ 	]*vpmaddwd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 31[ 	]*vpmaddwd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 72 7f[ 	]*vpmaddwd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 b2 00 08 00 00[ 	]*vpmaddwd 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 72 80[ 	]*vpmaddwd -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 b2 f0 f7 ff ff[ 	]*vpmaddwd -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 f4[ 	]*vpmaddwd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f5 f4[ 	]*vpmaddwd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 31[ 	]*vpmaddwd \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 72 7f[ 	]*vpmaddwd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 b2 00 10 00 00[ 	]*vpmaddwd 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 72 80[ 	]*vpmaddwd -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 b2 e0 ef ff ff[ 	]*vpmaddwd -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c f4[ 	]*vpmaxsb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 3c f4[ 	]*vpmaxsb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c 31[ 	]*vpmaxsb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c 72 7f[ 	]*vpmaxsb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c b2 00 08 00 00[ 	]*vpmaxsb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c 72 80[ 	]*vpmaxsb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c b2 f0 f7 ff ff[ 	]*vpmaxsb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c f4[ 	]*vpmaxsb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 3c f4[ 	]*vpmaxsb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c 31[ 	]*vpmaxsb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c 72 7f[ 	]*vpmaxsb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c b2 00 10 00 00[ 	]*vpmaxsb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c 72 80[ 	]*vpmaxsb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c b2 e0 ef ff ff[ 	]*vpmaxsb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee f4[ 	]*vpmaxsw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ee f4[ 	]*vpmaxsw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee 31[ 	]*vpmaxsw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee 72 7f[ 	]*vpmaxsw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee b2 00 08 00 00[ 	]*vpmaxsw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee 72 80[ 	]*vpmaxsw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee b2 f0 f7 ff ff[ 	]*vpmaxsw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee f4[ 	]*vpmaxsw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ee f4[ 	]*vpmaxsw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee 31[ 	]*vpmaxsw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee 72 7f[ 	]*vpmaxsw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee b2 00 10 00 00[ 	]*vpmaxsw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee 72 80[ 	]*vpmaxsw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee b2 e0 ef ff ff[ 	]*vpmaxsw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de f4[ 	]*vpmaxub %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f de f4[ 	]*vpmaxub %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de 31[ 	]*vpmaxub \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de b4 f4 c0 1d fe ff[ 	]*vpmaxub -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de 72 7f[ 	]*vpmaxub 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de b2 00 08 00 00[ 	]*vpmaxub 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de 72 80[ 	]*vpmaxub -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de b2 f0 f7 ff ff[ 	]*vpmaxub -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de f4[ 	]*vpmaxub %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af de f4[ 	]*vpmaxub %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de 31[ 	]*vpmaxub \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de b4 f4 c0 1d fe ff[ 	]*vpmaxub -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de 72 7f[ 	]*vpmaxub 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de b2 00 10 00 00[ 	]*vpmaxub 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de 72 80[ 	]*vpmaxub -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de b2 e0 ef ff ff[ 	]*vpmaxub -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e f4[ 	]*vpmaxuw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 3e f4[ 	]*vpmaxuw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e 31[ 	]*vpmaxuw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e 72 7f[ 	]*vpmaxuw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e b2 00 08 00 00[ 	]*vpmaxuw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e 72 80[ 	]*vpmaxuw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e b2 f0 f7 ff ff[ 	]*vpmaxuw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e f4[ 	]*vpmaxuw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 3e f4[ 	]*vpmaxuw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e 31[ 	]*vpmaxuw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e 72 7f[ 	]*vpmaxuw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e b2 00 10 00 00[ 	]*vpmaxuw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e 72 80[ 	]*vpmaxuw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e b2 e0 ef ff ff[ 	]*vpmaxuw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 f4[ 	]*vpminsb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 38 f4[ 	]*vpminsb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 31[ 	]*vpminsb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 b4 f4 c0 1d fe ff[ 	]*vpminsb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 72 7f[ 	]*vpminsb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 b2 00 08 00 00[ 	]*vpminsb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 72 80[ 	]*vpminsb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 b2 f0 f7 ff ff[ 	]*vpminsb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 f4[ 	]*vpminsb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 38 f4[ 	]*vpminsb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 31[ 	]*vpminsb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 b4 f4 c0 1d fe ff[ 	]*vpminsb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 72 7f[ 	]*vpminsb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 b2 00 10 00 00[ 	]*vpminsb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 72 80[ 	]*vpminsb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 b2 e0 ef ff ff[ 	]*vpminsb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea f4[ 	]*vpminsw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ea f4[ 	]*vpminsw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea 31[ 	]*vpminsw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea b4 f4 c0 1d fe ff[ 	]*vpminsw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea 72 7f[ 	]*vpminsw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea b2 00 08 00 00[ 	]*vpminsw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea 72 80[ 	]*vpminsw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea b2 f0 f7 ff ff[ 	]*vpminsw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea f4[ 	]*vpminsw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ea f4[ 	]*vpminsw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea 31[ 	]*vpminsw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea b4 f4 c0 1d fe ff[ 	]*vpminsw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea 72 7f[ 	]*vpminsw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea b2 00 10 00 00[ 	]*vpminsw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea 72 80[ 	]*vpminsw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea b2 e0 ef ff ff[ 	]*vpminsw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da f4[ 	]*vpminub %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f da f4[ 	]*vpminub %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da 31[ 	]*vpminub \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da b4 f4 c0 1d fe ff[ 	]*vpminub -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da 72 7f[ 	]*vpminub 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da b2 00 08 00 00[ 	]*vpminub 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da 72 80[ 	]*vpminub -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da b2 f0 f7 ff ff[ 	]*vpminub -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da f4[ 	]*vpminub %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af da f4[ 	]*vpminub %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da 31[ 	]*vpminub \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da b4 f4 c0 1d fe ff[ 	]*vpminub -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da 72 7f[ 	]*vpminub 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da b2 00 10 00 00[ 	]*vpminub 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da 72 80[ 	]*vpminub -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da b2 e0 ef ff ff[ 	]*vpminub -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a f4[ 	]*vpminuw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 3a f4[ 	]*vpminuw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a 31[ 	]*vpminuw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a b4 f4 c0 1d fe ff[ 	]*vpminuw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a 72 7f[ 	]*vpminuw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a b2 00 08 00 00[ 	]*vpminuw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a 72 80[ 	]*vpminuw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a b2 f0 f7 ff ff[ 	]*vpminuw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a f4[ 	]*vpminuw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 3a f4[ 	]*vpminuw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a 31[ 	]*vpminuw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a b4 f4 c0 1d fe ff[ 	]*vpminuw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a 72 7f[ 	]*vpminuw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a b2 00 10 00 00[ 	]*vpminuw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a 72 80[ 	]*vpminuw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a b2 e0 ef ff ff[ 	]*vpminuw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 f5[ 	]*vpmovsxbw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 20 f5[ 	]*vpmovsxbw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 31[ 	]*vpmovsxbw \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 72 7f[ 	]*vpmovsxbw 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 b2 00 04 00 00[ 	]*vpmovsxbw 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 72 80[ 	]*vpmovsxbw -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 b2 f8 fb ff ff[ 	]*vpmovsxbw -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 f5[ 	]*vpmovsxbw %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 20 f5[ 	]*vpmovsxbw %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 31[ 	]*vpmovsxbw \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 72 7f[ 	]*vpmovsxbw 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 b2 00 08 00 00[ 	]*vpmovsxbw 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 72 80[ 	]*vpmovsxbw -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 b2 f0 f7 ff ff[ 	]*vpmovsxbw -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 f5[ 	]*vpmovzxbw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 30 f5[ 	]*vpmovzxbw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 31[ 	]*vpmovzxbw \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 72 7f[ 	]*vpmovzxbw 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 b2 00 04 00 00[ 	]*vpmovzxbw 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 72 80[ 	]*vpmovzxbw -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 b2 f8 fb ff ff[ 	]*vpmovzxbw -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 f5[ 	]*vpmovzxbw %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 30 f5[ 	]*vpmovzxbw %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 31[ 	]*vpmovzxbw \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 72 7f[ 	]*vpmovzxbw 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 b2 00 08 00 00[ 	]*vpmovzxbw 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 72 80[ 	]*vpmovzxbw -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 b2 f0 f7 ff ff[ 	]*vpmovzxbw -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b f4[ 	]*vpmulhrsw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 0b f4[ 	]*vpmulhrsw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b 31[ 	]*vpmulhrsw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b 72 7f[ 	]*vpmulhrsw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b b2 00 08 00 00[ 	]*vpmulhrsw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b 72 80[ 	]*vpmulhrsw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b b2 f0 f7 ff ff[ 	]*vpmulhrsw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b f4[ 	]*vpmulhrsw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 0b f4[ 	]*vpmulhrsw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b 31[ 	]*vpmulhrsw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b 72 7f[ 	]*vpmulhrsw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b b2 00 10 00 00[ 	]*vpmulhrsw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b 72 80[ 	]*vpmulhrsw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b b2 e0 ef ff ff[ 	]*vpmulhrsw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 f4[ 	]*vpmulhuw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e4 f4[ 	]*vpmulhuw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 31[ 	]*vpmulhuw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 72 7f[ 	]*vpmulhuw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 b2 00 08 00 00[ 	]*vpmulhuw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 72 80[ 	]*vpmulhuw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 b2 f0 f7 ff ff[ 	]*vpmulhuw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 f4[ 	]*vpmulhuw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e4 f4[ 	]*vpmulhuw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 31[ 	]*vpmulhuw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 72 7f[ 	]*vpmulhuw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 b2 00 10 00 00[ 	]*vpmulhuw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 72 80[ 	]*vpmulhuw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 b2 e0 ef ff ff[ 	]*vpmulhuw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 f4[ 	]*vpmulhw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e5 f4[ 	]*vpmulhw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 31[ 	]*vpmulhw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 72 7f[ 	]*vpmulhw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 b2 00 08 00 00[ 	]*vpmulhw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 72 80[ 	]*vpmulhw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 b2 f0 f7 ff ff[ 	]*vpmulhw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 f4[ 	]*vpmulhw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e5 f4[ 	]*vpmulhw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 31[ 	]*vpmulhw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 72 7f[ 	]*vpmulhw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 b2 00 10 00 00[ 	]*vpmulhw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 72 80[ 	]*vpmulhw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 b2 e0 ef ff ff[ 	]*vpmulhw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 f4[ 	]*vpmullw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d5 f4[ 	]*vpmullw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 31[ 	]*vpmullw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 b4 f4 c0 1d fe ff[ 	]*vpmullw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 72 7f[ 	]*vpmullw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 b2 00 08 00 00[ 	]*vpmullw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 72 80[ 	]*vpmullw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 b2 f0 f7 ff ff[ 	]*vpmullw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 f4[ 	]*vpmullw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d5 f4[ 	]*vpmullw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 31[ 	]*vpmullw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 b4 f4 c0 1d fe ff[ 	]*vpmullw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 72 7f[ 	]*vpmullw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 b2 00 10 00 00[ 	]*vpmullw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 72 80[ 	]*vpmullw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 b2 e0 ef ff ff[ 	]*vpmullw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 f4[ 	]*vpshufb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 00 f4[ 	]*vpshufb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 31[ 	]*vpshufb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 b4 f4 c0 1d fe ff[ 	]*vpshufb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 72 7f[ 	]*vpshufb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 b2 00 08 00 00[ 	]*vpshufb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 72 80[ 	]*vpshufb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 b2 f0 f7 ff ff[ 	]*vpshufb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 f4[ 	]*vpshufb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 00 f4[ 	]*vpshufb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 31[ 	]*vpshufb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 b4 f4 c0 1d fe ff[ 	]*vpshufb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 72 7f[ 	]*vpshufb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 b2 00 10 00 00[ 	]*vpshufb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 72 80[ 	]*vpshufb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 b2 e0 ef ff ff[ 	]*vpshufb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 f5 ab[ 	]*vpshufhw \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 8f 70 f5 ab[ 	]*vpshufhw \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 f5 7b[ 	]*vpshufhw \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 31 7b[ 	]*vpshufhw \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 72 7f 7b[ 	]*vpshufhw \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 b2 00 08 00 00 7b[ 	]*vpshufhw \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 72 80 7b[ 	]*vpshufhw \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 b2 f0 f7 ff ff 7b[ 	]*vpshufhw \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 f5 ab[ 	]*vpshufhw \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e af 70 f5 ab[ 	]*vpshufhw \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 f5 7b[ 	]*vpshufhw \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 31 7b[ 	]*vpshufhw \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 72 7f 7b[ 	]*vpshufhw \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 b2 00 10 00 00 7b[ 	]*vpshufhw \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 72 80 7b[ 	]*vpshufhw \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 b2 e0 ef ff ff 7b[ 	]*vpshufhw \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 f5 ab[ 	]*vpshuflw \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 70 f5 ab[ 	]*vpshuflw \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 f5 7b[ 	]*vpshuflw \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 31 7b[ 	]*vpshuflw \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 72 7f 7b[ 	]*vpshuflw \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 b2 00 08 00 00 7b[ 	]*vpshuflw \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 72 80 7b[ 	]*vpshuflw \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 b2 f0 f7 ff ff 7b[ 	]*vpshuflw \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 f5 ab[ 	]*vpshuflw \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 70 f5 ab[ 	]*vpshuflw \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 f5 7b[ 	]*vpshuflw \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 31 7b[ 	]*vpshuflw \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 72 7f 7b[ 	]*vpshuflw \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 b2 00 10 00 00 7b[ 	]*vpshuflw \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 72 80 7b[ 	]*vpshuflw \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 b2 e0 ef ff ff 7b[ 	]*vpshuflw \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 f4[ 	]*vpsllw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f1 f4[ 	]*vpsllw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 31[ 	]*vpsllw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 72 7f[ 	]*vpsllw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 b2 00 08 00 00[ 	]*vpsllw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 72 80[ 	]*vpsllw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 b2 f0 f7 ff ff[ 	]*vpsllw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 f4[ 	]*vpsllw %xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f1 f4[ 	]*vpsllw %xmm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 31[ 	]*vpsllw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 72 7f[ 	]*vpsllw 0x7f0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 b2 00 08 00 00[ 	]*vpsllw 0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 72 80[ 	]*vpsllw -0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 b2 f0 f7 ff ff[ 	]*vpsllw -0x810\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 f4[ 	]*vpsraw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e1 f4[ 	]*vpsraw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 31[ 	]*vpsraw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 72 7f[ 	]*vpsraw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 b2 00 08 00 00[ 	]*vpsraw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 72 80[ 	]*vpsraw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 b2 f0 f7 ff ff[ 	]*vpsraw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 f4[ 	]*vpsraw %xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e1 f4[ 	]*vpsraw %xmm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 31[ 	]*vpsraw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 72 7f[ 	]*vpsraw 0x7f0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 b2 00 08 00 00[ 	]*vpsraw 0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 72 80[ 	]*vpsraw -0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 b2 f0 f7 ff ff[ 	]*vpsraw -0x810\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 f4[ 	]*vpsrlw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d1 f4[ 	]*vpsrlw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 31[ 	]*vpsrlw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 72 7f[ 	]*vpsrlw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 b2 00 08 00 00[ 	]*vpsrlw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 72 80[ 	]*vpsrlw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 b2 f0 f7 ff ff[ 	]*vpsrlw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 f4[ 	]*vpsrlw %xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d1 f4[ 	]*vpsrlw %xmm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 31[ 	]*vpsrlw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 72 7f[ 	]*vpsrlw 0x7f0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 b2 00 08 00 00[ 	]*vpsrlw 0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 72 80[ 	]*vpsrlw -0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 b2 f0 f7 ff ff[ 	]*vpsrlw -0x810\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 d5 ab[ 	]*vpsrlw \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 8f 71 d5 ab[ 	]*vpsrlw \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 d5 7b[ 	]*vpsrlw \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 11 7b[ 	]*vpsrlw \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 52 7f 7b[ 	]*vpsrlw \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 92 00 08 00 00 7b[ 	]*vpsrlw \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 52 80 7b[ 	]*vpsrlw \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 92 f0 f7 ff ff 7b[ 	]*vpsrlw \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 d5 ab[ 	]*vpsrlw \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d af 71 d5 ab[ 	]*vpsrlw \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 d5 7b[ 	]*vpsrlw \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 11 7b[ 	]*vpsrlw \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 52 7f 7b[ 	]*vpsrlw \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 92 00 10 00 00 7b[ 	]*vpsrlw \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 52 80 7b[ 	]*vpsrlw \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 92 e0 ef ff ff 7b[ 	]*vpsrlw \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 e5 ab[ 	]*vpsraw \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 8f 71 e5 ab[ 	]*vpsraw \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 e5 7b[ 	]*vpsraw \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 21 7b[ 	]*vpsraw \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 62 7f 7b[ 	]*vpsraw \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 a2 00 08 00 00 7b[ 	]*vpsraw \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 62 80 7b[ 	]*vpsraw \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 a2 f0 f7 ff ff 7b[ 	]*vpsraw \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 e5 ab[ 	]*vpsraw \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d af 71 e5 ab[ 	]*vpsraw \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 e5 7b[ 	]*vpsraw \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 21 7b[ 	]*vpsraw \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 62 7f 7b[ 	]*vpsraw \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 a2 00 10 00 00 7b[ 	]*vpsraw \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 62 80 7b[ 	]*vpsraw \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 a2 e0 ef ff ff 7b[ 	]*vpsraw \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 f4[ 	]*vpsrlvw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 10 f4[ 	]*vpsrlvw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 31[ 	]*vpsrlvw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 b4 f4 c0 1d fe ff[ 	]*vpsrlvw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 72 7f[ 	]*vpsrlvw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 b2 00 08 00 00[ 	]*vpsrlvw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 72 80[ 	]*vpsrlvw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 b2 f0 f7 ff ff[ 	]*vpsrlvw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 f4[ 	]*vpsrlvw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 10 f4[ 	]*vpsrlvw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 31[ 	]*vpsrlvw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 b4 f4 c0 1d fe ff[ 	]*vpsrlvw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 72 7f[ 	]*vpsrlvw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 b2 00 10 00 00[ 	]*vpsrlvw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 72 80[ 	]*vpsrlvw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 b2 e0 ef ff ff[ 	]*vpsrlvw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 f4[ 	]*vpsravw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 11 f4[ 	]*vpsravw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 31[ 	]*vpsravw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 b4 f4 c0 1d fe ff[ 	]*vpsravw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 72 7f[ 	]*vpsravw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 b2 00 08 00 00[ 	]*vpsravw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 72 80[ 	]*vpsravw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 b2 f0 f7 ff ff[ 	]*vpsravw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 f4[ 	]*vpsravw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 11 f4[ 	]*vpsravw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 31[ 	]*vpsravw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 b4 f4 c0 1d fe ff[ 	]*vpsravw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 72 7f[ 	]*vpsravw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 b2 00 10 00 00[ 	]*vpsravw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 72 80[ 	]*vpsravw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 b2 e0 ef ff ff[ 	]*vpsravw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 f4[ 	]*vpsubb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f8 f4[ 	]*vpsubb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 31[ 	]*vpsubb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 b4 f4 c0 1d fe ff[ 	]*vpsubb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 72 7f[ 	]*vpsubb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 b2 00 08 00 00[ 	]*vpsubb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 72 80[ 	]*vpsubb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 b2 f0 f7 ff ff[ 	]*vpsubb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 f4[ 	]*vpsubb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f8 f4[ 	]*vpsubb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 31[ 	]*vpsubb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 b4 f4 c0 1d fe ff[ 	]*vpsubb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 72 7f[ 	]*vpsubb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 b2 00 10 00 00[ 	]*vpsubb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 72 80[ 	]*vpsubb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 b2 e0 ef ff ff[ 	]*vpsubb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 f4[ 	]*vpsubsb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e8 f4[ 	]*vpsubsb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 31[ 	]*vpsubsb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 72 7f[ 	]*vpsubsb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 b2 00 08 00 00[ 	]*vpsubsb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 72 80[ 	]*vpsubsb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 b2 f0 f7 ff ff[ 	]*vpsubsb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 f4[ 	]*vpsubsb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e8 f4[ 	]*vpsubsb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 31[ 	]*vpsubsb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 72 7f[ 	]*vpsubsb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 b2 00 10 00 00[ 	]*vpsubsb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 72 80[ 	]*vpsubsb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 b2 e0 ef ff ff[ 	]*vpsubsb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 f4[ 	]*vpsubsw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e9 f4[ 	]*vpsubsw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 31[ 	]*vpsubsw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 72 7f[ 	]*vpsubsw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 b2 00 08 00 00[ 	]*vpsubsw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 72 80[ 	]*vpsubsw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 b2 f0 f7 ff ff[ 	]*vpsubsw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 f4[ 	]*vpsubsw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e9 f4[ 	]*vpsubsw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 31[ 	]*vpsubsw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 72 7f[ 	]*vpsubsw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 b2 00 10 00 00[ 	]*vpsubsw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 72 80[ 	]*vpsubsw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 b2 e0 ef ff ff[ 	]*vpsubsw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 f4[ 	]*vpsubusb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d8 f4[ 	]*vpsubusb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 31[ 	]*vpsubusb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 72 7f[ 	]*vpsubusb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 b2 00 08 00 00[ 	]*vpsubusb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 72 80[ 	]*vpsubusb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 b2 f0 f7 ff ff[ 	]*vpsubusb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 f4[ 	]*vpsubusb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d8 f4[ 	]*vpsubusb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 31[ 	]*vpsubusb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 72 7f[ 	]*vpsubusb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 b2 00 10 00 00[ 	]*vpsubusb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 72 80[ 	]*vpsubusb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 b2 e0 ef ff ff[ 	]*vpsubusb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 f4[ 	]*vpsubusw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d9 f4[ 	]*vpsubusw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 31[ 	]*vpsubusw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 72 7f[ 	]*vpsubusw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 b2 00 08 00 00[ 	]*vpsubusw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 72 80[ 	]*vpsubusw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 b2 f0 f7 ff ff[ 	]*vpsubusw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 f4[ 	]*vpsubusw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d9 f4[ 	]*vpsubusw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 31[ 	]*vpsubusw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 72 7f[ 	]*vpsubusw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 b2 00 10 00 00[ 	]*vpsubusw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 72 80[ 	]*vpsubusw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 b2 e0 ef ff ff[ 	]*vpsubusw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 f4[ 	]*vpsubw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f9 f4[ 	]*vpsubw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 31[ 	]*vpsubw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 b4 f4 c0 1d fe ff[ 	]*vpsubw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 72 7f[ 	]*vpsubw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 b2 00 08 00 00[ 	]*vpsubw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 72 80[ 	]*vpsubw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 b2 f0 f7 ff ff[ 	]*vpsubw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 f4[ 	]*vpsubw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f9 f4[ 	]*vpsubw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 31[ 	]*vpsubw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 b4 f4 c0 1d fe ff[ 	]*vpsubw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 72 7f[ 	]*vpsubw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 b2 00 10 00 00[ 	]*vpsubw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 72 80[ 	]*vpsubw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 b2 e0 ef ff ff[ 	]*vpsubw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 f4[ 	]*vpunpckhbw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 68 f4[ 	]*vpunpckhbw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 31[ 	]*vpunpckhbw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 72 7f[ 	]*vpunpckhbw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 b2 00 08 00 00[ 	]*vpunpckhbw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 72 80[ 	]*vpunpckhbw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 b2 f0 f7 ff ff[ 	]*vpunpckhbw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 f4[ 	]*vpunpckhbw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 68 f4[ 	]*vpunpckhbw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 31[ 	]*vpunpckhbw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 72 7f[ 	]*vpunpckhbw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 b2 00 10 00 00[ 	]*vpunpckhbw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 72 80[ 	]*vpunpckhbw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 b2 e0 ef ff ff[ 	]*vpunpckhbw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 f4[ 	]*vpunpckhwd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 69 f4[ 	]*vpunpckhwd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 31[ 	]*vpunpckhwd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 72 7f[ 	]*vpunpckhwd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 b2 00 08 00 00[ 	]*vpunpckhwd 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 72 80[ 	]*vpunpckhwd -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 b2 f0 f7 ff ff[ 	]*vpunpckhwd -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 f4[ 	]*vpunpckhwd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 69 f4[ 	]*vpunpckhwd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 31[ 	]*vpunpckhwd \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 72 7f[ 	]*vpunpckhwd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 b2 00 10 00 00[ 	]*vpunpckhwd 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 72 80[ 	]*vpunpckhwd -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 b2 e0 ef ff ff[ 	]*vpunpckhwd -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 f4[ 	]*vpunpcklbw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 60 f4[ 	]*vpunpcklbw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 31[ 	]*vpunpcklbw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 72 7f[ 	]*vpunpcklbw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 b2 00 08 00 00[ 	]*vpunpcklbw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 72 80[ 	]*vpunpcklbw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 b2 f0 f7 ff ff[ 	]*vpunpcklbw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 f4[ 	]*vpunpcklbw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 60 f4[ 	]*vpunpcklbw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 31[ 	]*vpunpcklbw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 72 7f[ 	]*vpunpcklbw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 b2 00 10 00 00[ 	]*vpunpcklbw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 72 80[ 	]*vpunpcklbw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 b2 e0 ef ff ff[ 	]*vpunpcklbw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 f4[ 	]*vpunpcklwd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 61 f4[ 	]*vpunpcklwd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 31[ 	]*vpunpcklwd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 72 7f[ 	]*vpunpcklwd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 b2 00 08 00 00[ 	]*vpunpcklwd 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 72 80[ 	]*vpunpcklwd -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 b2 f0 f7 ff ff[ 	]*vpunpcklwd -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 f4[ 	]*vpunpcklwd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 61 f4[ 	]*vpunpcklwd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 31[ 	]*vpunpcklwd \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 72 7f[ 	]*vpunpcklwd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 b2 00 10 00 00[ 	]*vpunpcklwd 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 72 80[ 	]*vpunpcklwd -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 b2 e0 ef ff ff[ 	]*vpunpcklwd -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 ee[ 	]*vpmovwb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 8f 30 ee[ 	]*vpmovwb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 ee[ 	]*vpmovwb %ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e af 30 ee[ 	]*vpmovwb %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 ee[ 	]*vpmovswb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 8f 20 ee[ 	]*vpmovswb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 ee[ 	]*vpmovswb %ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e af 20 ee[ 	]*vpmovswb %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 ee[ 	]*vpmovuswb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 8f 10 ee[ 	]*vpmovuswb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 ee[ 	]*vpmovuswb %ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e af 10 ee[ 	]*vpmovuswb %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 f4 ab[ 	]*vdbpsadbw \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 42 f4 ab[ 	]*vdbpsadbw \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 f4 7b[ 	]*vdbpsadbw \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 31 7b[ 	]*vdbpsadbw \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 b4 f4 c0 1d fe ff 7b[ 	]*vdbpsadbw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 72 7f 7b[ 	]*vdbpsadbw \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 b2 00 08 00 00 7b[ 	]*vdbpsadbw \$0x7b,0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 72 80 7b[ 	]*vdbpsadbw \$0x7b,-0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 b2 f0 f7 ff ff 7b[ 	]*vdbpsadbw \$0x7b,-0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 f4 ab[ 	]*vdbpsadbw \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 42 f4 ab[ 	]*vdbpsadbw \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 f4 7b[ 	]*vdbpsadbw \$0x7b,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 31 7b[ 	]*vdbpsadbw \$0x7b,\(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 b4 f4 c0 1d fe ff 7b[ 	]*vdbpsadbw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 72 7f 7b[ 	]*vdbpsadbw \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 b2 00 10 00 00 7b[ 	]*vdbpsadbw \$0x7b,0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 72 80 7b[ 	]*vdbpsadbw \$0x7b,-0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 b2 e0 ef ff ff 7b[ 	]*vdbpsadbw \$0x7b,-0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d f4[ 	]*vpermw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 8d f4[ 	]*vpermw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d 31[ 	]*vpermw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d b4 f4 c0 1d fe ff[ 	]*vpermw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d 72 7f[ 	]*vpermw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d b2 00 08 00 00[ 	]*vpermw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d 72 80[ 	]*vpermw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d b2 f0 f7 ff ff[ 	]*vpermw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d f4[ 	]*vpermw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 8d f4[ 	]*vpermw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d 31[ 	]*vpermw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d b4 f4 c0 1d fe ff[ 	]*vpermw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d 72 7f[ 	]*vpermw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d b2 00 10 00 00[ 	]*vpermw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d 72 80[ 	]*vpermw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d b2 e0 ef ff ff[ 	]*vpermw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d f4[ 	]*vpermt2w %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 7d f4[ 	]*vpermt2w %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d 31[ 	]*vpermt2w \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2w -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d 72 7f[ 	]*vpermt2w 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d b2 00 08 00 00[ 	]*vpermt2w 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d 72 80[ 	]*vpermt2w -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d b2 f0 f7 ff ff[ 	]*vpermt2w -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d f4[ 	]*vpermt2w %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 7d f4[ 	]*vpermt2w %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d 31[ 	]*vpermt2w \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2w -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d 72 7f[ 	]*vpermt2w 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d b2 00 10 00 00[ 	]*vpermt2w 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d 72 80[ 	]*vpermt2w -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d b2 e0 ef ff ff[ 	]*vpermt2w -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 f5 ab[ 	]*vpsllw \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 8f 71 f5 ab[ 	]*vpsllw \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 f5 7b[ 	]*vpsllw \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 31 7b[ 	]*vpsllw \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 72 7f 7b[ 	]*vpsllw \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 b2 00 08 00 00 7b[ 	]*vpsllw \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 72 80 7b[ 	]*vpsllw \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 b2 f0 f7 ff ff 7b[ 	]*vpsllw \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 f5 ab[ 	]*vpsllw \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d af 71 f5 ab[ 	]*vpsllw \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 f5 7b[ 	]*vpsllw \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 31 7b[ 	]*vpsllw \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 72 7f 7b[ 	]*vpsllw \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 b2 00 10 00 00 7b[ 	]*vpsllw \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 72 80 7b[ 	]*vpsllw \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 b2 e0 ef ff ff 7b[ 	]*vpsllw \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 f4[ 	]*vpsllvw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 12 f4[ 	]*vpsllvw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 31[ 	]*vpsllvw \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 b4 f4 c0 1d fe ff[ 	]*vpsllvw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 72 7f[ 	]*vpsllvw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 b2 00 08 00 00[ 	]*vpsllvw 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 72 80[ 	]*vpsllvw -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 b2 f0 f7 ff ff[ 	]*vpsllvw -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 f4[ 	]*vpsllvw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 12 f4[ 	]*vpsllvw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 31[ 	]*vpsllvw \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 b4 f4 c0 1d fe ff[ 	]*vpsllvw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 72 7f[ 	]*vpsllvw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 b2 00 10 00 00[ 	]*vpsllvw 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 72 80[ 	]*vpsllvw -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 b2 e0 ef ff ff[ 	]*vpsllvw -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f f5[ 	]*vmovdqu8 %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 6f f5[ 	]*vmovdqu8 %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f 31[ 	]*vmovdqu8 \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f 72 7f[ 	]*vmovdqu8 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f b2 00 08 00 00[ 	]*vmovdqu8 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f 72 80[ 	]*vmovdqu8 -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f b2 f0 f7 ff ff[ 	]*vmovdqu8 -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f f5[ 	]*vmovdqu8 %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 6f f5[ 	]*vmovdqu8 %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f 31[ 	]*vmovdqu8 \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f 72 7f[ 	]*vmovdqu8 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f b2 00 10 00 00[ 	]*vmovdqu8 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f 72 80[ 	]*vmovdqu8 -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f b2 e0 ef ff ff[ 	]*vmovdqu8 -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f f5[ 	]*vmovdqu16 %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 6f f5[ 	]*vmovdqu16 %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f 31[ 	]*vmovdqu16 \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f 72 7f[ 	]*vmovdqu16 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f b2 00 08 00 00[ 	]*vmovdqu16 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f 72 80[ 	]*vmovdqu16 -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f b2 f0 f7 ff ff[ 	]*vmovdqu16 -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f f5[ 	]*vmovdqu16 %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 6f f5[ 	]*vmovdqu16 %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f 31[ 	]*vmovdqu16 \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f 72 7f[ 	]*vmovdqu16 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f b2 00 10 00 00[ 	]*vmovdqu16 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f 72 80[ 	]*vmovdqu16 -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f b2 e0 ef ff ff[ 	]*vmovdqu16 -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 31[ 	]*vpmovwb %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 b4 f4 c0 1d fe ff[ 	]*vpmovwb %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 72 7f[ 	]*vpmovwb %xmm6,0x3f8\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 b2 00 04 00 00[ 	]*vpmovwb %xmm6,0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 72 80[ 	]*vpmovwb %xmm6,-0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 b2 f8 fb ff ff[ 	]*vpmovwb %xmm6,-0x408\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 31[ 	]*vpmovwb %ymm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 b4 f4 c0 1d fe ff[ 	]*vpmovwb %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 72 7f[ 	]*vpmovwb %ymm6,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 b2 00 08 00 00[ 	]*vpmovwb %ymm6,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 72 80[ 	]*vpmovwb %ymm6,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 b2 f0 f7 ff ff[ 	]*vpmovwb %ymm6,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 31[ 	]*vpmovswb %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 b4 f4 c0 1d fe ff[ 	]*vpmovswb %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 72 7f[ 	]*vpmovswb %xmm6,0x3f8\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 b2 00 04 00 00[ 	]*vpmovswb %xmm6,0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 72 80[ 	]*vpmovswb %xmm6,-0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 b2 f8 fb ff ff[ 	]*vpmovswb %xmm6,-0x408\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 31[ 	]*vpmovswb %ymm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 b4 f4 c0 1d fe ff[ 	]*vpmovswb %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 72 7f[ 	]*vpmovswb %ymm6,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 b2 00 08 00 00[ 	]*vpmovswb %ymm6,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 72 80[ 	]*vpmovswb %ymm6,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 b2 f0 f7 ff ff[ 	]*vpmovswb %ymm6,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 31[ 	]*vpmovuswb %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 b4 f4 c0 1d fe ff[ 	]*vpmovuswb %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 72 7f[ 	]*vpmovuswb %xmm6,0x3f8\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 b2 00 04 00 00[ 	]*vpmovuswb %xmm6,0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 72 80[ 	]*vpmovuswb %xmm6,-0x400\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 b2 f8 fb ff ff[ 	]*vpmovuswb %xmm6,-0x408\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 31[ 	]*vpmovuswb %ymm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 b4 f4 c0 1d fe ff[ 	]*vpmovuswb %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 72 7f[ 	]*vpmovuswb %ymm6,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 b2 00 08 00 00[ 	]*vpmovuswb %ymm6,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 72 80[ 	]*vpmovuswb %ymm6,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 b2 f0 f7 ff ff[ 	]*vpmovuswb %ymm6,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f 31[ 	]*vmovdqu8 %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f 72 7f[ 	]*vmovdqu8 %xmm6,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f b2 00 08 00 00[ 	]*vmovdqu8 %xmm6,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f 72 80[ 	]*vmovdqu8 %xmm6,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f b2 f0 f7 ff ff[ 	]*vmovdqu8 %xmm6,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f 31[ 	]*vmovdqu8 %ymm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f 72 7f[ 	]*vmovdqu8 %ymm6,0xfe0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f b2 00 10 00 00[ 	]*vmovdqu8 %ymm6,0x1000\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f 72 80[ 	]*vmovdqu8 %ymm6,-0x1000\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f b2 e0 ef ff ff[ 	]*vmovdqu8 %ymm6,-0x1020\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f 31[ 	]*vmovdqu16 %xmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f 72 7f[ 	]*vmovdqu16 %xmm6,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f b2 00 08 00 00[ 	]*vmovdqu16 %xmm6,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f 72 80[ 	]*vmovdqu16 %xmm6,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f b2 f0 f7 ff ff[ 	]*vmovdqu16 %xmm6,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f 31[ 	]*vmovdqu16 %ymm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f 72 7f[ 	]*vmovdqu16 %ymm6,0xfe0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f b2 00 10 00 00[ 	]*vmovdqu16 %ymm6,0x1000\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f 72 80[ 	]*vmovdqu16 %ymm6,-0x1000\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f b2 e0 ef ff ff[ 	]*vmovdqu16 %ymm6,-0x1020\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 f4[ 	]*vpermi2w %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 75 f4[ 	]*vpermi2w %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 31[ 	]*vpermi2w \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2w -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 72 7f[ 	]*vpermi2w 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 b2 00 08 00 00[ 	]*vpermi2w 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 72 80[ 	]*vpermi2w -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 b2 f0 f7 ff ff[ 	]*vpermi2w -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 f4[ 	]*vpermi2w %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 75 f4[ 	]*vpermi2w %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 31[ 	]*vpermi2w \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2w -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 72 7f[ 	]*vpermi2w 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 b2 00 10 00 00[ 	]*vpermi2w 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 72 80[ 	]*vpermi2w -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 b2 e0 ef ff ff[ 	]*vpermi2w -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 ed[ 	]*vptestmb %xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 29[ 	]*vptestmb \(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 ac f4 c0 1d fe ff[ 	]*vptestmb -0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 6a 7f[ 	]*vptestmb 0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 aa 00 08 00 00[ 	]*vptestmb 0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 6a 80[ 	]*vptestmb -0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 aa f0 f7 ff ff[ 	]*vptestmb -0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 ed[ 	]*vptestmb %ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 29[ 	]*vptestmb \(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 ac f4 c0 1d fe ff[ 	]*vptestmb -0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 6a 7f[ 	]*vptestmb 0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 aa 00 10 00 00[ 	]*vptestmb 0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 6a 80[ 	]*vptestmb -0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 aa e0 ef ff ff[ 	]*vptestmb -0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 ed[ 	]*vptestmw %xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 29[ 	]*vptestmw \(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 ac f4 c0 1d fe ff[ 	]*vptestmw -0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 6a 7f[ 	]*vptestmw 0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 aa 00 08 00 00[ 	]*vptestmw 0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 6a 80[ 	]*vptestmw -0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 aa f0 f7 ff ff[ 	]*vptestmw -0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 ed[ 	]*vptestmw %ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 29[ 	]*vptestmw \(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 ac f4 c0 1d fe ff[ 	]*vptestmw -0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 6a 7f[ 	]*vptestmw 0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 aa 00 10 00 00[ 	]*vptestmw 0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 6a 80[ 	]*vptestmw -0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 aa e0 ef ff ff[ 	]*vptestmw -0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 29 ee[ 	]*vpmovb2m %xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 29 ee[ 	]*vpmovb2m %ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 29 ee[ 	]*vpmovw2m %xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 29 ee[ 	]*vpmovw2m %ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 28 f5[ 	]*vpmovm2b %k5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 28 f5[ 	]*vpmovm2b %k5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 28 f5[ 	]*vpmovm2w %k5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 28 f5[ 	]*vpmovm2w %k5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 ec[ 	]*vptestnmb %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 29[ 	]*vptestnmb \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 ac f4 c0 1d fe ff[ 	]*vptestnmb -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 6a 7f[ 	]*vptestnmb 0x7f0\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 aa 00 08 00 00[ 	]*vptestnmb 0x800\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 6a 80[ 	]*vptestnmb -0x800\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 aa f0 f7 ff ff[ 	]*vptestnmb -0x810\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 ec[ 	]*vptestnmb %ymm4,%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 29[ 	]*vptestnmb \(%ecx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 ac f4 c0 1d fe ff[ 	]*vptestnmb -0x1e240\(%esp,%esi,8\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 6a 7f[ 	]*vptestnmb 0xfe0\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 aa 00 10 00 00[ 	]*vptestnmb 0x1000\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 6a 80[ 	]*vptestnmb -0x1000\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 aa e0 ef ff ff[ 	]*vptestnmb -0x1020\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 ec[ 	]*vptestnmw %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 29[ 	]*vptestnmw \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 ac f4 c0 1d fe ff[ 	]*vptestnmw -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 6a 7f[ 	]*vptestnmw 0x7f0\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 aa 00 08 00 00[ 	]*vptestnmw 0x800\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 6a 80[ 	]*vptestnmw -0x800\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 aa f0 f7 ff ff[ 	]*vptestnmw -0x810\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 ec[ 	]*vptestnmw %ymm4,%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 29[ 	]*vptestnmw \(%ecx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 ac f4 c0 1d fe ff[ 	]*vptestnmw -0x1e240\(%esp,%esi,8\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 6a 7f[ 	]*vptestnmw 0xfe0\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 aa 00 10 00 00[ 	]*vptestnmw 0x1000\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 6a 80[ 	]*vptestnmw -0x1000\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 aa e0 ef ff ff[ 	]*vptestnmw -0x1020\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f ed ab[ 	]*vpcmpb \$0xab,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f ed 7b[ 	]*vpcmpb \$0x7b,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f 29 7b[ 	]*vpcmpb \$0x7b,\(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpb \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f 6a 7f 7b[ 	]*vpcmpb \$0x7b,0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f aa 00 08 00 00 7b[ 	]*vpcmpb \$0x7b,0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f 6a 80 7b[ 	]*vpcmpb \$0x7b,-0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f aa f0 f7 ff ff 7b[ 	]*vpcmpb \$0x7b,-0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f ed ab[ 	]*vpcmpb \$0xab,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f ed 7b[ 	]*vpcmpb \$0x7b,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f 29 7b[ 	]*vpcmpb \$0x7b,\(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpb \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f 6a 7f 7b[ 	]*vpcmpb \$0x7b,0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f aa 00 10 00 00 7b[ 	]*vpcmpb \$0x7b,0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f 6a 80 7b[ 	]*vpcmpb \$0x7b,-0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f aa e0 ef ff ff 7b[ 	]*vpcmpb \$0x7b,-0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f ed ab[ 	]*vpcmpw \$0xab,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f ed 7b[ 	]*vpcmpw \$0x7b,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f 29 7b[ 	]*vpcmpw \$0x7b,\(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f 6a 7f 7b[ 	]*vpcmpw \$0x7b,0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f aa 00 08 00 00 7b[ 	]*vpcmpw \$0x7b,0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f 6a 80 7b[ 	]*vpcmpw \$0x7b,-0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f aa f0 f7 ff ff 7b[ 	]*vpcmpw \$0x7b,-0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f ed ab[ 	]*vpcmpw \$0xab,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f ed 7b[ 	]*vpcmpw \$0x7b,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f 29 7b[ 	]*vpcmpw \$0x7b,\(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f 6a 7f 7b[ 	]*vpcmpw \$0x7b,0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f aa 00 10 00 00 7b[ 	]*vpcmpw \$0x7b,0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f 6a 80 7b[ 	]*vpcmpw \$0x7b,-0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f aa e0 ef ff ff 7b[ 	]*vpcmpw \$0x7b,-0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e ed ab[ 	]*vpcmpub \$0xab,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e ed 7b[ 	]*vpcmpub \$0x7b,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e 29 7b[ 	]*vpcmpub \$0x7b,\(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpub \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e 6a 7f 7b[ 	]*vpcmpub \$0x7b,0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e aa 00 08 00 00 7b[ 	]*vpcmpub \$0x7b,0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e 6a 80 7b[ 	]*vpcmpub \$0x7b,-0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e aa f0 f7 ff ff 7b[ 	]*vpcmpub \$0x7b,-0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e ed ab[ 	]*vpcmpub \$0xab,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e ed 7b[ 	]*vpcmpub \$0x7b,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e 29 7b[ 	]*vpcmpub \$0x7b,\(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpub \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e 6a 7f 7b[ 	]*vpcmpub \$0x7b,0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e aa 00 10 00 00 7b[ 	]*vpcmpub \$0x7b,0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e 6a 80 7b[ 	]*vpcmpub \$0x7b,-0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e aa e0 ef ff ff 7b[ 	]*vpcmpub \$0x7b,-0x1020\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e ed ab[ 	]*vpcmpuw \$0xab,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e ed 7b[ 	]*vpcmpuw \$0x7b,%xmm5,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e 29 7b[ 	]*vpcmpuw \$0x7b,\(%ecx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpuw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e 6a 7f 7b[ 	]*vpcmpuw \$0x7b,0x7f0\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e aa 00 08 00 00 7b[ 	]*vpcmpuw \$0x7b,0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e 6a 80 7b[ 	]*vpcmpuw \$0x7b,-0x800\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e aa f0 f7 ff ff 7b[ 	]*vpcmpuw \$0x7b,-0x810\(%edx\),%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e ed ab[ 	]*vpcmpuw \$0xab,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e ed 7b[ 	]*vpcmpuw \$0x7b,%ymm5,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e 29 7b[ 	]*vpcmpuw \$0x7b,\(%ecx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpuw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e 6a 7f 7b[ 	]*vpcmpuw \$0x7b,0xfe0\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e aa 00 10 00 00 7b[ 	]*vpcmpuw \$0x7b,0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e 6a 80 7b[ 	]*vpcmpuw \$0x7b,-0x1000\(%edx\),%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e aa e0 ef ff ff 7b[ 	]*vpcmpuw \$0x7b,-0x1020\(%edx\),%ymm6,%k5\{%k7\}
#pass
