#objdump: -dw
#name: x86_64 AVX512/4FMAPS insns
#source: x86-64-avx512_4fmaps.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 9a 09[ 	]*v4fmaddps \(%rcx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 4f 9a 09[ 	]*v4fmaddps \(%rcx\),%zmm8,%zmm1\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f cf 9a 09[ 	]*v4fmaddps \(%rcx\),%zmm8,%zmm1\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 3f 48 9a 8c f0 c0 1d fe ff[ 	]*v4fmaddps -0x1e240\(%rax,%r14,8\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 9a 4a 7f[ 	]*v4fmaddps 0x7f0\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 9a 8a 00 08 00 00[ 	]*v4fmaddps 0x800\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 9a 4a 80[ 	]*v4fmaddps -0x800\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 9a 8a f0 f7 ff ff[ 	]*v4fmaddps -0x810\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 aa 09[ 	]*v4fnmaddps \(%rcx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 4f aa 09[ 	]*v4fnmaddps \(%rcx\),%zmm8,%zmm1\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f cf aa 09[ 	]*v4fnmaddps \(%rcx\),%zmm8,%zmm1\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 3f 48 aa 8c f0 c0 1d fe ff[ 	]*v4fnmaddps -0x1e240\(%rax,%r14,8\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 aa 4a 7f[ 	]*v4fnmaddps 0x7f0\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 aa 8a 00 08 00 00[ 	]*v4fnmaddps 0x800\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 aa 4a 80[ 	]*v4fnmaddps -0x800\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 aa 8a f0 f7 ff ff[ 	]*v4fnmaddps -0x810\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 9b 09[ 	]*v4fmaddss \(%rcx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 0f 9b 09[ 	]*v4fmaddss \(%rcx\),%xmm8,%xmm1\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 8f 9b 09[ 	]*v4fmaddss \(%rcx\),%xmm8,%xmm1\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 3f 08 9b 8c f0 c0 1d fe ff[ 	]*v4fmaddss -0x1e240\(%rax,%r14,8\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 9b 4a 7f[ 	]*v4fmaddss 0x7f0\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 9b 8a 00 08 00 00[ 	]*v4fmaddss 0x800\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 9b 4a 80[ 	]*v4fmaddss -0x800\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 9b 8a f0 f7 ff ff[ 	]*v4fmaddss -0x810\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 ab 09[ 	]*v4fnmaddss \(%rcx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 0f ab 09[ 	]*v4fnmaddss \(%rcx\),%xmm8,%xmm1\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 8f ab 09[ 	]*v4fnmaddss \(%rcx\),%xmm8,%xmm1\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 3f 08 ab 8c f0 c0 1d fe ff[ 	]*v4fnmaddss -0x1e240\(%rax,%r14,8\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 ab 4a 7f[ 	]*v4fnmaddss 0x7f0\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 ab 8a 00 08 00 00[ 	]*v4fnmaddss 0x800\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 ab 4a 80[ 	]*v4fnmaddss -0x800\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 ab 8a f0 f7 ff ff[ 	]*v4fnmaddss -0x810\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 9a 09[ 	]*v4fmaddps \(%rcx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 9a 09[ 	]*v4fmaddps \(%rcx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 4f 9a 09[ 	]*v4fmaddps \(%rcx\),%zmm8,%zmm1\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f cf 9a 09[ 	]*v4fmaddps \(%rcx\),%zmm8,%zmm1\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 3f 48 9a 8c f0 c0 1d fe ff[ 	]*v4fmaddps -0x1e240\(%rax,%r14,8\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 9a 4a 7f[ 	]*v4fmaddps 0x7f0\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 9a 8a 00 08 00 00[ 	]*v4fmaddps 0x800\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 9a 4a 80[ 	]*v4fmaddps -0x800\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 9a 8a f0 f7 ff ff[ 	]*v4fmaddps -0x810\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 aa 09[ 	]*v4fnmaddps \(%rcx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 aa 09[ 	]*v4fnmaddps \(%rcx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 4f aa 09[ 	]*v4fnmaddps \(%rcx\),%zmm8,%zmm1\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f cf aa 09[ 	]*v4fnmaddps \(%rcx\),%zmm8,%zmm1\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 3f 48 aa 8c f0 c0 1d fe ff[ 	]*v4fnmaddps -0x1e240\(%rax,%r14,8\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 aa 4a 7f[ 	]*v4fnmaddps 0x7f0\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 aa 8a 00 08 00 00[ 	]*v4fnmaddps 0x800\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 aa 4a 80[ 	]*v4fnmaddps -0x800\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 aa 8a f0 f7 ff ff[ 	]*v4fnmaddps -0x810\(%rdx\),%zmm8,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 9b 09[ 	]*v4fmaddss \(%rcx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 9b 09[ 	]*v4fmaddss \(%rcx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 0f 9b 09[ 	]*v4fmaddss \(%rcx\),%xmm8,%xmm1\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 8f 9b 09[ 	]*v4fmaddss \(%rcx\),%xmm8,%xmm1\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 3f 08 9b 8c f0 c0 1d fe ff[ 	]*v4fmaddss -0x1e240\(%rax,%r14,8\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 9b 4a 7f[ 	]*v4fmaddss 0x7f0\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 9b 8a 00 08 00 00[ 	]*v4fmaddss 0x800\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 9b 4a 80[ 	]*v4fmaddss -0x800\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 9b 8a f0 f7 ff ff[ 	]*v4fmaddss -0x810\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 ab 09[ 	]*v4fnmaddss \(%rcx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 ab 09[ 	]*v4fnmaddss \(%rcx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 0f ab 09[ 	]*v4fnmaddss \(%rcx\),%xmm8,%xmm1\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 8f ab 09[ 	]*v4fnmaddss \(%rcx\),%xmm8,%xmm1\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 3f 08 ab 8c f0 c0 1d fe ff[ 	]*v4fnmaddss -0x1e240\(%rax,%r14,8\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 ab 4a 7f[ 	]*v4fnmaddss 0x7f0\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 ab 8a 00 08 00 00[ 	]*v4fnmaddss 0x800\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 ab 4a 80[ 	]*v4fnmaddss -0x800\(%rdx\),%xmm8,%xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 08 ab 8a f0 f7 ff ff[ 	]*v4fnmaddss -0x810\(%rdx\),%xmm8,%xmm1
#pass
