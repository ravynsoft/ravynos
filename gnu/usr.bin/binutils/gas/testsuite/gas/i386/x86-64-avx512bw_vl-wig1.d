#as: -mevexwig=1
#objdump: -dw
#name: x86_64 AVX512BW/VL wig insns
#source: x86-64-avx512bw_vl-wig.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 1c f5[ 	]*vpabsb %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 1c f5[ 	]*vpabsb %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 1c f5[ 	]*vpabsb %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1c 31[ 	]*vpabsb \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 1c b4 f0 23 01 00 00[ 	]*vpabsb 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1c 72 7f[ 	]*vpabsb 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1c b2 00 08 00 00[ 	]*vpabsb 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1c 72 80[ 	]*vpabsb -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1c b2 f0 f7 ff ff[ 	]*vpabsb -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 1c f5[ 	]*vpabsb %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 1c f5[ 	]*vpabsb %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 1c f5[ 	]*vpabsb %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1c 31[ 	]*vpabsb \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 1c b4 f0 23 01 00 00[ 	]*vpabsb 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1c 72 7f[ 	]*vpabsb 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1c b2 00 10 00 00[ 	]*vpabsb 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1c 72 80[ 	]*vpabsb -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1c b2 e0 ef ff ff[ 	]*vpabsb -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 1d f5[ 	]*vpabsw %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 1d f5[ 	]*vpabsw %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 1d f5[ 	]*vpabsw %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1d 31[ 	]*vpabsw \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 1d b4 f0 23 01 00 00[ 	]*vpabsw 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1d 72 7f[ 	]*vpabsw 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1d b2 00 08 00 00[ 	]*vpabsw 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1d 72 80[ 	]*vpabsw -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1d b2 f0 f7 ff ff[ 	]*vpabsw -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 1d f5[ 	]*vpabsw %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 1d f5[ 	]*vpabsw %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 1d f5[ 	]*vpabsw %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1d 31[ 	]*vpabsw \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 1d b4 f0 23 01 00 00[ 	]*vpabsw 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1d 72 7f[ 	]*vpabsw 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1d b2 00 10 00 00[ 	]*vpabsw 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1d 72 80[ 	]*vpabsw -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1d b2 e0 ef ff ff[ 	]*vpabsw -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 63 f4[ 	]*vpacksswb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 63 f4[ 	]*vpacksswb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 63 f4[ 	]*vpacksswb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 63 31[ 	]*vpacksswb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 63 b4 f0 23 01 00 00[ 	]*vpacksswb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 63 72 7f[ 	]*vpacksswb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 63 b2 00 08 00 00[ 	]*vpacksswb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 63 72 80[ 	]*vpacksswb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 63 b2 f0 f7 ff ff[ 	]*vpacksswb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 63 f4[ 	]*vpacksswb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 63 f4[ 	]*vpacksswb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 63 f4[ 	]*vpacksswb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 63 31[ 	]*vpacksswb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 63 b4 f0 23 01 00 00[ 	]*vpacksswb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 63 72 7f[ 	]*vpacksswb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 63 b2 00 10 00 00[ 	]*vpacksswb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 63 72 80[ 	]*vpacksswb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 63 b2 e0 ef ff ff[ 	]*vpacksswb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 67 f4[ 	]*vpackuswb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 67 f4[ 	]*vpackuswb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 67 f4[ 	]*vpackuswb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 67 31[ 	]*vpackuswb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 67 b4 f0 23 01 00 00[ 	]*vpackuswb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 67 72 7f[ 	]*vpackuswb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 67 b2 00 08 00 00[ 	]*vpackuswb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 67 72 80[ 	]*vpackuswb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 67 b2 f0 f7 ff ff[ 	]*vpackuswb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 67 f4[ 	]*vpackuswb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 67 f4[ 	]*vpackuswb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 67 f4[ 	]*vpackuswb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 67 31[ 	]*vpackuswb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 67 b4 f0 23 01 00 00[ 	]*vpackuswb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 67 72 7f[ 	]*vpackuswb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 67 b2 00 10 00 00[ 	]*vpackuswb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 67 72 80[ 	]*vpackuswb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 67 b2 e0 ef ff ff[ 	]*vpackuswb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 fc f4[ 	]*vpaddb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 fc f4[ 	]*vpaddb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 fc f4[ 	]*vpaddb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fc 31[ 	]*vpaddb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 fc b4 f0 23 01 00 00[ 	]*vpaddb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fc 72 7f[ 	]*vpaddb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fc b2 00 08 00 00[ 	]*vpaddb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fc 72 80[ 	]*vpaddb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fc b2 f0 f7 ff ff[ 	]*vpaddb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 fc f4[ 	]*vpaddb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 fc f4[ 	]*vpaddb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 fc f4[ 	]*vpaddb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fc 31[ 	]*vpaddb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 fc b4 f0 23 01 00 00[ 	]*vpaddb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fc 72 7f[ 	]*vpaddb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fc b2 00 10 00 00[ 	]*vpaddb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fc 72 80[ 	]*vpaddb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fc b2 e0 ef ff ff[ 	]*vpaddb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 ec f4[ 	]*vpaddsb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 ec f4[ 	]*vpaddsb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 ec f4[ 	]*vpaddsb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ec 31[ 	]*vpaddsb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 ec b4 f0 23 01 00 00[ 	]*vpaddsb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ec 72 7f[ 	]*vpaddsb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ec b2 00 08 00 00[ 	]*vpaddsb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ec 72 80[ 	]*vpaddsb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ec b2 f0 f7 ff ff[ 	]*vpaddsb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 ec f4[ 	]*vpaddsb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 ec f4[ 	]*vpaddsb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 ec f4[ 	]*vpaddsb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ec 31[ 	]*vpaddsb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 ec b4 f0 23 01 00 00[ 	]*vpaddsb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ec 72 7f[ 	]*vpaddsb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ec b2 00 10 00 00[ 	]*vpaddsb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ec 72 80[ 	]*vpaddsb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ec b2 e0 ef ff ff[ 	]*vpaddsb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 ed f4[ 	]*vpaddsw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 ed f4[ 	]*vpaddsw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 ed f4[ 	]*vpaddsw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ed 31[ 	]*vpaddsw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 ed b4 f0 23 01 00 00[ 	]*vpaddsw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ed 72 7f[ 	]*vpaddsw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ed b2 00 08 00 00[ 	]*vpaddsw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ed 72 80[ 	]*vpaddsw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ed b2 f0 f7 ff ff[ 	]*vpaddsw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 ed f4[ 	]*vpaddsw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 ed f4[ 	]*vpaddsw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 ed f4[ 	]*vpaddsw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ed 31[ 	]*vpaddsw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 ed b4 f0 23 01 00 00[ 	]*vpaddsw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ed 72 7f[ 	]*vpaddsw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ed b2 00 10 00 00[ 	]*vpaddsw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ed 72 80[ 	]*vpaddsw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ed b2 e0 ef ff ff[ 	]*vpaddsw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 dc f4[ 	]*vpaddusb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 dc f4[ 	]*vpaddusb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 dc f4[ 	]*vpaddusb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dc 31[ 	]*vpaddusb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 dc b4 f0 23 01 00 00[ 	]*vpaddusb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dc 72 7f[ 	]*vpaddusb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dc b2 00 08 00 00[ 	]*vpaddusb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dc 72 80[ 	]*vpaddusb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dc b2 f0 f7 ff ff[ 	]*vpaddusb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 dc f4[ 	]*vpaddusb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 dc f4[ 	]*vpaddusb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 dc f4[ 	]*vpaddusb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dc 31[ 	]*vpaddusb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 dc b4 f0 23 01 00 00[ 	]*vpaddusb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dc 72 7f[ 	]*vpaddusb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dc b2 00 10 00 00[ 	]*vpaddusb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dc 72 80[ 	]*vpaddusb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dc b2 e0 ef ff ff[ 	]*vpaddusb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 dd f4[ 	]*vpaddusw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 dd f4[ 	]*vpaddusw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 dd f4[ 	]*vpaddusw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dd 31[ 	]*vpaddusw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 dd b4 f0 23 01 00 00[ 	]*vpaddusw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dd 72 7f[ 	]*vpaddusw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dd b2 00 08 00 00[ 	]*vpaddusw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dd 72 80[ 	]*vpaddusw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dd b2 f0 f7 ff ff[ 	]*vpaddusw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 dd f4[ 	]*vpaddusw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 dd f4[ 	]*vpaddusw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 dd f4[ 	]*vpaddusw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dd 31[ 	]*vpaddusw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 dd b4 f0 23 01 00 00[ 	]*vpaddusw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dd 72 7f[ 	]*vpaddusw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dd b2 00 10 00 00[ 	]*vpaddusw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dd 72 80[ 	]*vpaddusw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dd b2 e0 ef ff ff[ 	]*vpaddusw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 fd f4[ 	]*vpaddw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 fd f4[ 	]*vpaddw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 fd f4[ 	]*vpaddw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fd 31[ 	]*vpaddw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 fd b4 f0 23 01 00 00[ 	]*vpaddw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fd 72 7f[ 	]*vpaddw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fd b2 00 08 00 00[ 	]*vpaddw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fd 72 80[ 	]*vpaddw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fd b2 f0 f7 ff ff[ 	]*vpaddw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 fd f4[ 	]*vpaddw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 fd f4[ 	]*vpaddw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 fd f4[ 	]*vpaddw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fd 31[ 	]*vpaddw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 fd b4 f0 23 01 00 00[ 	]*vpaddw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fd 72 7f[ 	]*vpaddw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fd b2 00 10 00 00[ 	]*vpaddw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fd 72 80[ 	]*vpaddw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fd b2 e0 ef ff ff[ 	]*vpaddw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 0f f4 ab[ 	]*vpalignr \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 0f f4 ab[ 	]*vpalignr \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 0f f4 ab[ 	]*vpalignr \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 0f f4 7b[ 	]*vpalignr \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 0f 31 7b[ 	]*vpalignr \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 0f b4 f0 23 01 00 00 7b[ 	]*vpalignr \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 0f 72 7f 7b[ 	]*vpalignr \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 0f b2 00 08 00 00 7b[ 	]*vpalignr \$0x7b,0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 0f 72 80 7b[ 	]*vpalignr \$0x7b,-0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 0f b2 f0 f7 ff ff 7b[ 	]*vpalignr \$0x7b,-0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 0f f4 ab[ 	]*vpalignr \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 0f f4 ab[ 	]*vpalignr \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 0f f4 ab[ 	]*vpalignr \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 0f f4 7b[ 	]*vpalignr \$0x7b,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 0f 31 7b[ 	]*vpalignr \$0x7b,\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 0f b4 f0 23 01 00 00 7b[ 	]*vpalignr \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 0f 72 7f 7b[ 	]*vpalignr \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 0f b2 00 10 00 00 7b[ 	]*vpalignr \$0x7b,0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 0f 72 80 7b[ 	]*vpalignr \$0x7b,-0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 0f b2 e0 ef ff ff 7b[ 	]*vpalignr \$0x7b,-0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e0 f4[ 	]*vpavgb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e0 f4[ 	]*vpavgb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e0 f4[ 	]*vpavgb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e0 31[ 	]*vpavgb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e0 b4 f0 23 01 00 00[ 	]*vpavgb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e0 72 7f[ 	]*vpavgb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e0 b2 00 08 00 00[ 	]*vpavgb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e0 72 80[ 	]*vpavgb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e0 b2 f0 f7 ff ff[ 	]*vpavgb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e0 f4[ 	]*vpavgb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e0 f4[ 	]*vpavgb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e0 f4[ 	]*vpavgb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e0 31[ 	]*vpavgb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e0 b4 f0 23 01 00 00[ 	]*vpavgb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e0 72 7f[ 	]*vpavgb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e0 b2 00 10 00 00[ 	]*vpavgb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e0 72 80[ 	]*vpavgb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e0 b2 e0 ef ff ff[ 	]*vpavgb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e3 f4[ 	]*vpavgw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e3 f4[ 	]*vpavgw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e3 f4[ 	]*vpavgw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e3 31[ 	]*vpavgw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e3 b4 f0 23 01 00 00[ 	]*vpavgw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e3 72 7f[ 	]*vpavgw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e3 b2 00 08 00 00[ 	]*vpavgw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e3 72 80[ 	]*vpavgw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e3 b2 f0 f7 ff ff[ 	]*vpavgw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e3 f4[ 	]*vpavgw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e3 f4[ 	]*vpavgw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e3 f4[ 	]*vpavgw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e3 31[ 	]*vpavgw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e3 b4 f0 23 01 00 00[ 	]*vpavgw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e3 72 7f[ 	]*vpavgw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e3 b2 00 10 00 00[ 	]*vpavgw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e3 72 80[ 	]*vpavgw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e3 b2 e0 ef ff ff[ 	]*vpavgw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 74 ed[ 	]*vpcmpeqb %xmm29,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 74 ed[ 	]*vpcmpeqb %xmm29,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 74 29[ 	]*vpcmpeqb \(%rcx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 74 ac f0 23 01 00 00[ 	]*vpcmpeqb 0x123\(%rax,%r14,8\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 74 6a 7f[ 	]*vpcmpeqb 0x7f0\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 74 aa 00 08 00 00[ 	]*vpcmpeqb 0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 74 6a 80[ 	]*vpcmpeqb -0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 74 aa f0 f7 ff ff[ 	]*vpcmpeqb -0x810\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 74 ed[ 	]*vpcmpeqb %ymm29,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 74 ed[ 	]*vpcmpeqb %ymm29,%ymm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 74 29[ 	]*vpcmpeqb \(%rcx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 74 ac f0 23 01 00 00[ 	]*vpcmpeqb 0x123\(%rax,%r14,8\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 74 6a 7f[ 	]*vpcmpeqb 0xfe0\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 74 aa 00 10 00 00[ 	]*vpcmpeqb 0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 74 6a 80[ 	]*vpcmpeqb -0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 74 aa e0 ef ff ff[ 	]*vpcmpeqb -0x1020\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 75 ed[ 	]*vpcmpeqw %xmm29,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 75 ed[ 	]*vpcmpeqw %xmm29,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 75 29[ 	]*vpcmpeqw \(%rcx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 75 ac f0 23 01 00 00[ 	]*vpcmpeqw 0x123\(%rax,%r14,8\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 75 6a 7f[ 	]*vpcmpeqw 0x7f0\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 75 aa 00 08 00 00[ 	]*vpcmpeqw 0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 75 6a 80[ 	]*vpcmpeqw -0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 75 aa f0 f7 ff ff[ 	]*vpcmpeqw -0x810\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 75 ed[ 	]*vpcmpeqw %ymm29,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 75 ed[ 	]*vpcmpeqw %ymm29,%ymm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 75 29[ 	]*vpcmpeqw \(%rcx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 75 ac f0 23 01 00 00[ 	]*vpcmpeqw 0x123\(%rax,%r14,8\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 75 6a 7f[ 	]*vpcmpeqw 0xfe0\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 75 aa 00 10 00 00[ 	]*vpcmpeqw 0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 75 6a 80[ 	]*vpcmpeqw -0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 75 aa e0 ef ff ff[ 	]*vpcmpeqw -0x1020\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 64 ed[ 	]*vpcmpgtb %xmm29,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 64 ed[ 	]*vpcmpgtb %xmm29,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 64 29[ 	]*vpcmpgtb \(%rcx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 64 ac f0 23 01 00 00[ 	]*vpcmpgtb 0x123\(%rax,%r14,8\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 64 6a 7f[ 	]*vpcmpgtb 0x7f0\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 64 aa 00 08 00 00[ 	]*vpcmpgtb 0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 64 6a 80[ 	]*vpcmpgtb -0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 64 aa f0 f7 ff ff[ 	]*vpcmpgtb -0x810\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 64 ed[ 	]*vpcmpgtb %ymm29,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 64 ed[ 	]*vpcmpgtb %ymm29,%ymm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 64 29[ 	]*vpcmpgtb \(%rcx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 64 ac f0 23 01 00 00[ 	]*vpcmpgtb 0x123\(%rax,%r14,8\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 64 6a 7f[ 	]*vpcmpgtb 0xfe0\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 64 aa 00 10 00 00[ 	]*vpcmpgtb 0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 64 6a 80[ 	]*vpcmpgtb -0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 64 aa e0 ef ff ff[ 	]*vpcmpgtb -0x1020\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 65 ed[ 	]*vpcmpgtw %xmm29,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 65 ed[ 	]*vpcmpgtw %xmm29,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 65 29[ 	]*vpcmpgtw \(%rcx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 65 ac f0 23 01 00 00[ 	]*vpcmpgtw 0x123\(%rax,%r14,8\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 65 6a 7f[ 	]*vpcmpgtw 0x7f0\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 65 aa 00 08 00 00[ 	]*vpcmpgtw 0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 65 6a 80[ 	]*vpcmpgtw -0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 65 aa f0 f7 ff ff[ 	]*vpcmpgtw -0x810\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 65 ed[ 	]*vpcmpgtw %ymm29,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 65 ed[ 	]*vpcmpgtw %ymm29,%ymm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 65 29[ 	]*vpcmpgtw \(%rcx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 65 ac f0 23 01 00 00[ 	]*vpcmpgtw 0x123\(%rax,%r14,8\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 65 6a 7f[ 	]*vpcmpgtw 0xfe0\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 65 aa 00 10 00 00[ 	]*vpcmpgtw 0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 65 6a 80[ 	]*vpcmpgtw -0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 65 aa e0 ef ff ff[ 	]*vpcmpgtw -0x1020\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 04 f4[ 	]*vpmaddubsw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 04 f4[ 	]*vpmaddubsw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 04 f4[ 	]*vpmaddubsw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 04 31[ 	]*vpmaddubsw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 04 b4 f0 23 01 00 00[ 	]*vpmaddubsw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 04 72 7f[ 	]*vpmaddubsw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 04 b2 00 08 00 00[ 	]*vpmaddubsw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 04 72 80[ 	]*vpmaddubsw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 04 b2 f0 f7 ff ff[ 	]*vpmaddubsw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 04 f4[ 	]*vpmaddubsw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 04 f4[ 	]*vpmaddubsw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 04 f4[ 	]*vpmaddubsw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 04 31[ 	]*vpmaddubsw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 04 b4 f0 23 01 00 00[ 	]*vpmaddubsw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 04 72 7f[ 	]*vpmaddubsw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 04 b2 00 10 00 00[ 	]*vpmaddubsw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 04 72 80[ 	]*vpmaddubsw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 04 b2 e0 ef ff ff[ 	]*vpmaddubsw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 f5 f4[ 	]*vpmaddwd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 f5 f4[ 	]*vpmaddwd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 f5 f4[ 	]*vpmaddwd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f5 31[ 	]*vpmaddwd \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 f5 b4 f0 23 01 00 00[ 	]*vpmaddwd 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f5 72 7f[ 	]*vpmaddwd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f5 b2 00 08 00 00[ 	]*vpmaddwd 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f5 72 80[ 	]*vpmaddwd -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f5 b2 f0 f7 ff ff[ 	]*vpmaddwd -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 f5 f4[ 	]*vpmaddwd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 f5 f4[ 	]*vpmaddwd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 f5 f4[ 	]*vpmaddwd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f5 31[ 	]*vpmaddwd \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 f5 b4 f0 23 01 00 00[ 	]*vpmaddwd 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f5 72 7f[ 	]*vpmaddwd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f5 b2 00 10 00 00[ 	]*vpmaddwd 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f5 72 80[ 	]*vpmaddwd -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f5 b2 e0 ef ff ff[ 	]*vpmaddwd -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 3c f4[ 	]*vpmaxsb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 3c f4[ 	]*vpmaxsb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 3c f4[ 	]*vpmaxsb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3c 31[ 	]*vpmaxsb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 3c b4 f0 23 01 00 00[ 	]*vpmaxsb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3c 72 7f[ 	]*vpmaxsb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3c b2 00 08 00 00[ 	]*vpmaxsb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3c 72 80[ 	]*vpmaxsb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3c b2 f0 f7 ff ff[ 	]*vpmaxsb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 3c f4[ 	]*vpmaxsb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 3c f4[ 	]*vpmaxsb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 3c f4[ 	]*vpmaxsb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3c 31[ 	]*vpmaxsb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 3c b4 f0 23 01 00 00[ 	]*vpmaxsb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3c 72 7f[ 	]*vpmaxsb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3c b2 00 10 00 00[ 	]*vpmaxsb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3c 72 80[ 	]*vpmaxsb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3c b2 e0 ef ff ff[ 	]*vpmaxsb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 ee f4[ 	]*vpmaxsw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 ee f4[ 	]*vpmaxsw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 ee f4[ 	]*vpmaxsw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ee 31[ 	]*vpmaxsw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 ee b4 f0 23 01 00 00[ 	]*vpmaxsw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ee 72 7f[ 	]*vpmaxsw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ee b2 00 08 00 00[ 	]*vpmaxsw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ee 72 80[ 	]*vpmaxsw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ee b2 f0 f7 ff ff[ 	]*vpmaxsw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 ee f4[ 	]*vpmaxsw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 ee f4[ 	]*vpmaxsw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 ee f4[ 	]*vpmaxsw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ee 31[ 	]*vpmaxsw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 ee b4 f0 23 01 00 00[ 	]*vpmaxsw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ee 72 7f[ 	]*vpmaxsw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ee b2 00 10 00 00[ 	]*vpmaxsw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ee 72 80[ 	]*vpmaxsw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ee b2 e0 ef ff ff[ 	]*vpmaxsw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 de f4[ 	]*vpmaxub %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 de f4[ 	]*vpmaxub %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 de f4[ 	]*vpmaxub %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 de 31[ 	]*vpmaxub \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 de b4 f0 23 01 00 00[ 	]*vpmaxub 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 de 72 7f[ 	]*vpmaxub 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 de b2 00 08 00 00[ 	]*vpmaxub 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 de 72 80[ 	]*vpmaxub -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 de b2 f0 f7 ff ff[ 	]*vpmaxub -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 de f4[ 	]*vpmaxub %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 de f4[ 	]*vpmaxub %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 de f4[ 	]*vpmaxub %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 de 31[ 	]*vpmaxub \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 de b4 f0 23 01 00 00[ 	]*vpmaxub 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 de 72 7f[ 	]*vpmaxub 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 de b2 00 10 00 00[ 	]*vpmaxub 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 de 72 80[ 	]*vpmaxub -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 de b2 e0 ef ff ff[ 	]*vpmaxub -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 3e f4[ 	]*vpmaxuw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 3e f4[ 	]*vpmaxuw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 3e f4[ 	]*vpmaxuw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3e 31[ 	]*vpmaxuw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 3e b4 f0 23 01 00 00[ 	]*vpmaxuw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3e 72 7f[ 	]*vpmaxuw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3e b2 00 08 00 00[ 	]*vpmaxuw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3e 72 80[ 	]*vpmaxuw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3e b2 f0 f7 ff ff[ 	]*vpmaxuw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 3e f4[ 	]*vpmaxuw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 3e f4[ 	]*vpmaxuw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 3e f4[ 	]*vpmaxuw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3e 31[ 	]*vpmaxuw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 3e b4 f0 23 01 00 00[ 	]*vpmaxuw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3e 72 7f[ 	]*vpmaxuw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3e b2 00 10 00 00[ 	]*vpmaxuw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3e 72 80[ 	]*vpmaxuw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3e b2 e0 ef ff ff[ 	]*vpmaxuw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 38 f4[ 	]*vpminsb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 38 f4[ 	]*vpminsb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 38 f4[ 	]*vpminsb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 38 31[ 	]*vpminsb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 38 b4 f0 23 01 00 00[ 	]*vpminsb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 38 72 7f[ 	]*vpminsb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 38 b2 00 08 00 00[ 	]*vpminsb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 38 72 80[ 	]*vpminsb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 38 b2 f0 f7 ff ff[ 	]*vpminsb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 38 f4[ 	]*vpminsb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 38 f4[ 	]*vpminsb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 38 f4[ 	]*vpminsb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 38 31[ 	]*vpminsb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 38 b4 f0 23 01 00 00[ 	]*vpminsb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 38 72 7f[ 	]*vpminsb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 38 b2 00 10 00 00[ 	]*vpminsb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 38 72 80[ 	]*vpminsb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 38 b2 e0 ef ff ff[ 	]*vpminsb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 ea f4[ 	]*vpminsw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 ea f4[ 	]*vpminsw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 ea f4[ 	]*vpminsw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ea 31[ 	]*vpminsw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 ea b4 f0 23 01 00 00[ 	]*vpminsw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ea 72 7f[ 	]*vpminsw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ea b2 00 08 00 00[ 	]*vpminsw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ea 72 80[ 	]*vpminsw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ea b2 f0 f7 ff ff[ 	]*vpminsw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 ea f4[ 	]*vpminsw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 ea f4[ 	]*vpminsw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 ea f4[ 	]*vpminsw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ea 31[ 	]*vpminsw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 ea b4 f0 23 01 00 00[ 	]*vpminsw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ea 72 7f[ 	]*vpminsw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ea b2 00 10 00 00[ 	]*vpminsw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ea 72 80[ 	]*vpminsw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ea b2 e0 ef ff ff[ 	]*vpminsw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 da f4[ 	]*vpminub %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 da f4[ 	]*vpminub %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 da f4[ 	]*vpminub %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 da 31[ 	]*vpminub \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 da b4 f0 23 01 00 00[ 	]*vpminub 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 da 72 7f[ 	]*vpminub 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 da b2 00 08 00 00[ 	]*vpminub 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 da 72 80[ 	]*vpminub -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 da b2 f0 f7 ff ff[ 	]*vpminub -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 da f4[ 	]*vpminub %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 da f4[ 	]*vpminub %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 da f4[ 	]*vpminub %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 da 31[ 	]*vpminub \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 da b4 f0 23 01 00 00[ 	]*vpminub 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 da 72 7f[ 	]*vpminub 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 da b2 00 10 00 00[ 	]*vpminub 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 da 72 80[ 	]*vpminub -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 da b2 e0 ef ff ff[ 	]*vpminub -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 3a f4[ 	]*vpminuw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 3a f4[ 	]*vpminuw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 3a f4[ 	]*vpminuw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3a 31[ 	]*vpminuw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 3a b4 f0 23 01 00 00[ 	]*vpminuw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3a 72 7f[ 	]*vpminuw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3a b2 00 08 00 00[ 	]*vpminuw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3a 72 80[ 	]*vpminuw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3a b2 f0 f7 ff ff[ 	]*vpminuw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 3a f4[ 	]*vpminuw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 3a f4[ 	]*vpminuw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 3a f4[ 	]*vpminuw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3a 31[ 	]*vpminuw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 3a b4 f0 23 01 00 00[ 	]*vpminuw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3a 72 7f[ 	]*vpminuw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3a b2 00 10 00 00[ 	]*vpminuw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3a 72 80[ 	]*vpminuw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3a b2 e0 ef ff ff[ 	]*vpminuw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 20 f5[ 	]*vpmovsxbw %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 20 f5[ 	]*vpmovsxbw %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 20 f5[ 	]*vpmovsxbw %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 20 31[ 	]*vpmovsxbw \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 20 b4 f0 23 01 00 00[ 	]*vpmovsxbw 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 20 72 7f[ 	]*vpmovsxbw 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 20 b2 00 04 00 00[ 	]*vpmovsxbw 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 20 72 80[ 	]*vpmovsxbw -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 20 b2 f8 fb ff ff[ 	]*vpmovsxbw -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 20 f5[ 	]*vpmovsxbw %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 20 f5[ 	]*vpmovsxbw %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 20 f5[ 	]*vpmovsxbw %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 20 31[ 	]*vpmovsxbw \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 20 b4 f0 23 01 00 00[ 	]*vpmovsxbw 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 20 72 7f[ 	]*vpmovsxbw 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 20 b2 00 08 00 00[ 	]*vpmovsxbw 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 20 72 80[ 	]*vpmovsxbw -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 20 b2 f0 f7 ff ff[ 	]*vpmovsxbw -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 30 f5[ 	]*vpmovzxbw %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 30 f5[ 	]*vpmovzxbw %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 30 f5[ 	]*vpmovzxbw %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 30 31[ 	]*vpmovzxbw \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 30 b4 f0 23 01 00 00[ 	]*vpmovzxbw 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 30 72 7f[ 	]*vpmovzxbw 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 30 b2 00 04 00 00[ 	]*vpmovzxbw 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 30 72 80[ 	]*vpmovzxbw -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 30 b2 f8 fb ff ff[ 	]*vpmovzxbw -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 30 f5[ 	]*vpmovzxbw %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 30 f5[ 	]*vpmovzxbw %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 30 f5[ 	]*vpmovzxbw %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 30 31[ 	]*vpmovzxbw \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 30 b4 f0 23 01 00 00[ 	]*vpmovzxbw 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 30 72 7f[ 	]*vpmovzxbw 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 30 b2 00 08 00 00[ 	]*vpmovzxbw 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 30 72 80[ 	]*vpmovzxbw -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 30 b2 f0 f7 ff ff[ 	]*vpmovzxbw -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 0b f4[ 	]*vpmulhrsw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 0b f4[ 	]*vpmulhrsw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 0b f4[ 	]*vpmulhrsw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 0b 31[ 	]*vpmulhrsw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 0b b4 f0 23 01 00 00[ 	]*vpmulhrsw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 0b 72 7f[ 	]*vpmulhrsw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 0b b2 00 08 00 00[ 	]*vpmulhrsw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 0b 72 80[ 	]*vpmulhrsw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 0b b2 f0 f7 ff ff[ 	]*vpmulhrsw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 0b f4[ 	]*vpmulhrsw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 0b f4[ 	]*vpmulhrsw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 0b f4[ 	]*vpmulhrsw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 0b 31[ 	]*vpmulhrsw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 0b b4 f0 23 01 00 00[ 	]*vpmulhrsw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 0b 72 7f[ 	]*vpmulhrsw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 0b b2 00 10 00 00[ 	]*vpmulhrsw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 0b 72 80[ 	]*vpmulhrsw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 0b b2 e0 ef ff ff[ 	]*vpmulhrsw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e4 f4[ 	]*vpmulhuw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e4 f4[ 	]*vpmulhuw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e4 f4[ 	]*vpmulhuw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e4 31[ 	]*vpmulhuw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e4 b4 f0 23 01 00 00[ 	]*vpmulhuw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e4 72 7f[ 	]*vpmulhuw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e4 b2 00 08 00 00[ 	]*vpmulhuw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e4 72 80[ 	]*vpmulhuw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e4 b2 f0 f7 ff ff[ 	]*vpmulhuw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e4 f4[ 	]*vpmulhuw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e4 f4[ 	]*vpmulhuw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e4 f4[ 	]*vpmulhuw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e4 31[ 	]*vpmulhuw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e4 b4 f0 23 01 00 00[ 	]*vpmulhuw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e4 72 7f[ 	]*vpmulhuw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e4 b2 00 10 00 00[ 	]*vpmulhuw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e4 72 80[ 	]*vpmulhuw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e4 b2 e0 ef ff ff[ 	]*vpmulhuw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e5 f4[ 	]*vpmulhw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e5 f4[ 	]*vpmulhw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e5 f4[ 	]*vpmulhw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e5 31[ 	]*vpmulhw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e5 b4 f0 23 01 00 00[ 	]*vpmulhw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e5 72 7f[ 	]*vpmulhw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e5 b2 00 08 00 00[ 	]*vpmulhw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e5 72 80[ 	]*vpmulhw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e5 b2 f0 f7 ff ff[ 	]*vpmulhw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e5 f4[ 	]*vpmulhw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e5 f4[ 	]*vpmulhw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e5 f4[ 	]*vpmulhw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e5 31[ 	]*vpmulhw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e5 b4 f0 23 01 00 00[ 	]*vpmulhw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e5 72 7f[ 	]*vpmulhw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e5 b2 00 10 00 00[ 	]*vpmulhw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e5 72 80[ 	]*vpmulhw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e5 b2 e0 ef ff ff[ 	]*vpmulhw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 d5 f4[ 	]*vpmullw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 d5 f4[ 	]*vpmullw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 d5 f4[ 	]*vpmullw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d5 31[ 	]*vpmullw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 d5 b4 f0 23 01 00 00[ 	]*vpmullw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d5 72 7f[ 	]*vpmullw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d5 b2 00 08 00 00[ 	]*vpmullw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d5 72 80[ 	]*vpmullw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d5 b2 f0 f7 ff ff[ 	]*vpmullw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 d5 f4[ 	]*vpmullw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 d5 f4[ 	]*vpmullw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 d5 f4[ 	]*vpmullw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d5 31[ 	]*vpmullw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 d5 b4 f0 23 01 00 00[ 	]*vpmullw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d5 72 7f[ 	]*vpmullw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d5 b2 00 10 00 00[ 	]*vpmullw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d5 72 80[ 	]*vpmullw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d5 b2 e0 ef ff ff[ 	]*vpmullw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 f6 f4[ 	]*vpsadbw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f6 31[ 	]*vpsadbw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 f6 b4 f0 23 01 00 00[ 	]*vpsadbw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f6 72 7f[ 	]*vpsadbw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f6 b2 00 08 00 00[ 	]*vpsadbw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f6 72 80[ 	]*vpsadbw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f6 b2 f0 f7 ff ff[ 	]*vpsadbw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 f6 f4[ 	]*vpsadbw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f6 31[ 	]*vpsadbw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 f6 b4 f0 23 01 00 00[ 	]*vpsadbw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f6 72 7f[ 	]*vpsadbw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f6 b2 00 10 00 00[ 	]*vpsadbw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f6 72 80[ 	]*vpsadbw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f6 b2 e0 ef ff ff[ 	]*vpsadbw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 00 f4[ 	]*vpshufb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 00 f4[ 	]*vpshufb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 00 f4[ 	]*vpshufb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 00 31[ 	]*vpshufb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 00 b4 f0 23 01 00 00[ 	]*vpshufb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 00 72 7f[ 	]*vpshufb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 00 b2 00 08 00 00[ 	]*vpshufb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 00 72 80[ 	]*vpshufb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 00 b2 f0 f7 ff ff[ 	]*vpshufb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 00 f4[ 	]*vpshufb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 00 f4[ 	]*vpshufb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 00 f4[ 	]*vpshufb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 00 31[ 	]*vpshufb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 00 b4 f0 23 01 00 00[ 	]*vpshufb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 00 72 7f[ 	]*vpshufb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 00 b2 00 10 00 00[ 	]*vpshufb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 00 72 80[ 	]*vpshufb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 00 b2 e0 ef ff ff[ 	]*vpshufb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 08 70 f5 ab[ 	]*vpshufhw \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 0f 70 f5 ab[ 	]*vpshufhw \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 8f 70 f5 ab[ 	]*vpshufhw \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 08 70 f5 7b[ 	]*vpshufhw \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 70 31 7b[ 	]*vpshufhw \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 08 70 b4 f0 23 01 00 00 7b[ 	]*vpshufhw \$0x7b,0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 70 72 7f 7b[ 	]*vpshufhw \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 70 b2 00 08 00 00 7b[ 	]*vpshufhw \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 70 72 80 7b[ 	]*vpshufhw \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 70 b2 f0 f7 ff ff 7b[ 	]*vpshufhw \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 28 70 f5 ab[ 	]*vpshufhw \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 2f 70 f5 ab[ 	]*vpshufhw \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe af 70 f5 ab[ 	]*vpshufhw \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 28 70 f5 7b[ 	]*vpshufhw \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 70 31 7b[ 	]*vpshufhw \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 28 70 b4 f0 23 01 00 00 7b[ 	]*vpshufhw \$0x7b,0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 70 72 7f 7b[ 	]*vpshufhw \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 70 b2 00 10 00 00 7b[ 	]*vpshufhw \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 70 72 80 7b[ 	]*vpshufhw \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 70 b2 e0 ef ff ff 7b[ 	]*vpshufhw \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 70 f5 ab[ 	]*vpshuflw \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 70 f5 ab[ 	]*vpshuflw \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 70 f5 ab[ 	]*vpshuflw \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 70 f5 7b[ 	]*vpshuflw \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 70 31 7b[ 	]*vpshuflw \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 08 70 b4 f0 23 01 00 00 7b[ 	]*vpshuflw \$0x7b,0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 70 72 7f 7b[ 	]*vpshuflw \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 70 b2 00 08 00 00 7b[ 	]*vpshuflw \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 70 72 80 7b[ 	]*vpshuflw \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 70 b2 f0 f7 ff ff 7b[ 	]*vpshuflw \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 70 f5 ab[ 	]*vpshuflw \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 70 f5 ab[ 	]*vpshuflw \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 70 f5 ab[ 	]*vpshuflw \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 70 f5 7b[ 	]*vpshuflw \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 70 31 7b[ 	]*vpshuflw \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 28 70 b4 f0 23 01 00 00 7b[ 	]*vpshuflw \$0x7b,0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 70 72 7f 7b[ 	]*vpshuflw \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 70 b2 00 10 00 00 7b[ 	]*vpshuflw \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 70 72 80 7b[ 	]*vpshuflw \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 70 b2 e0 ef ff ff 7b[ 	]*vpshuflw \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 f1 f4[ 	]*vpsllw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 f1 f4[ 	]*vpsllw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 f1 f4[ 	]*vpsllw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f1 31[ 	]*vpsllw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 f1 b4 f0 23 01 00 00[ 	]*vpsllw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f1 72 7f[ 	]*vpsllw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f1 b2 00 08 00 00[ 	]*vpsllw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f1 72 80[ 	]*vpsllw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f1 b2 f0 f7 ff ff[ 	]*vpsllw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 f1 f4[ 	]*vpsllw %xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 f1 f4[ 	]*vpsllw %xmm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 f1 f4[ 	]*vpsllw %xmm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f1 31[ 	]*vpsllw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 f1 b4 f0 23 01 00 00[ 	]*vpsllw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f1 72 7f[ 	]*vpsllw 0x7f0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f1 b2 00 08 00 00[ 	]*vpsllw 0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f1 72 80[ 	]*vpsllw -0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f1 b2 f0 f7 ff ff[ 	]*vpsllw -0x810\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e1 f4[ 	]*vpsraw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e1 f4[ 	]*vpsraw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e1 f4[ 	]*vpsraw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e1 31[ 	]*vpsraw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e1 b4 f0 23 01 00 00[ 	]*vpsraw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e1 72 7f[ 	]*vpsraw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e1 b2 00 08 00 00[ 	]*vpsraw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e1 72 80[ 	]*vpsraw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e1 b2 f0 f7 ff ff[ 	]*vpsraw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e1 f4[ 	]*vpsraw %xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e1 f4[ 	]*vpsraw %xmm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e1 f4[ 	]*vpsraw %xmm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e1 31[ 	]*vpsraw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e1 b4 f0 23 01 00 00[ 	]*vpsraw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e1 72 7f[ 	]*vpsraw 0x7f0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e1 b2 00 08 00 00[ 	]*vpsraw 0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e1 72 80[ 	]*vpsraw -0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e1 b2 f0 f7 ff ff[ 	]*vpsraw -0x810\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 d1 f4[ 	]*vpsrlw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 d1 f4[ 	]*vpsrlw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 d1 f4[ 	]*vpsrlw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d1 31[ 	]*vpsrlw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 d1 b4 f0 23 01 00 00[ 	]*vpsrlw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d1 72 7f[ 	]*vpsrlw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d1 b2 00 08 00 00[ 	]*vpsrlw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d1 72 80[ 	]*vpsrlw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d1 b2 f0 f7 ff ff[ 	]*vpsrlw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 d1 f4[ 	]*vpsrlw %xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 d1 f4[ 	]*vpsrlw %xmm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 d1 f4[ 	]*vpsrlw %xmm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d1 31[ 	]*vpsrlw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 d1 b4 f0 23 01 00 00[ 	]*vpsrlw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d1 72 7f[ 	]*vpsrlw 0x7f0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d1 b2 00 08 00 00[ 	]*vpsrlw 0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d1 72 80[ 	]*vpsrlw -0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d1 b2 f0 f7 ff ff[ 	]*vpsrlw -0x810\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 73 dd ab[ 	]*vpsrldq \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 73 dd 7b[ 	]*vpsrldq \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 19 7b[ 	]*vpsrldq \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 73 9c f0 23 01 00 00 7b[ 	]*vpsrldq \$0x7b,0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 5a 7f 7b[ 	]*vpsrldq \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 9a 00 08 00 00 7b[ 	]*vpsrldq \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 5a 80 7b[ 	]*vpsrldq \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 9a f0 f7 ff ff 7b[ 	]*vpsrldq \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 73 dd ab[ 	]*vpsrldq \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 73 dd 7b[ 	]*vpsrldq \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 19 7b[ 	]*vpsrldq \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 73 9c f0 23 01 00 00 7b[ 	]*vpsrldq \$0x7b,0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 5a 7f 7b[ 	]*vpsrldq \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 9a 00 10 00 00 7b[ 	]*vpsrldq \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 5a 80 7b[ 	]*vpsrldq \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 9a e0 ef ff ff 7b[ 	]*vpsrldq \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 71 d5 ab[ 	]*vpsrlw \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 71 d5 ab[ 	]*vpsrlw \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 87 71 d5 ab[ 	]*vpsrlw \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 71 d5 7b[ 	]*vpsrlw \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 11 7b[ 	]*vpsrlw \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 71 94 f0 23 01 00 00 7b[ 	]*vpsrlw \$0x7b,0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 52 7f 7b[ 	]*vpsrlw \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 92 00 08 00 00 7b[ 	]*vpsrlw \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 52 80 7b[ 	]*vpsrlw \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 92 f0 f7 ff ff 7b[ 	]*vpsrlw \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 71 d5 ab[ 	]*vpsrlw \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 71 d5 ab[ 	]*vpsrlw \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d a7 71 d5 ab[ 	]*vpsrlw \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 71 d5 7b[ 	]*vpsrlw \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 11 7b[ 	]*vpsrlw \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 71 94 f0 23 01 00 00 7b[ 	]*vpsrlw \$0x7b,0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 52 7f 7b[ 	]*vpsrlw \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 92 00 10 00 00 7b[ 	]*vpsrlw \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 52 80 7b[ 	]*vpsrlw \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 92 e0 ef ff ff 7b[ 	]*vpsrlw \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 71 e5 ab[ 	]*vpsraw \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 71 e5 ab[ 	]*vpsraw \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 87 71 e5 ab[ 	]*vpsraw \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 71 e5 7b[ 	]*vpsraw \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 21 7b[ 	]*vpsraw \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 71 a4 f0 23 01 00 00 7b[ 	]*vpsraw \$0x7b,0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 62 7f 7b[ 	]*vpsraw \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 a2 00 08 00 00 7b[ 	]*vpsraw \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 62 80 7b[ 	]*vpsraw \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 a2 f0 f7 ff ff 7b[ 	]*vpsraw \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 71 e5 ab[ 	]*vpsraw \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 71 e5 ab[ 	]*vpsraw \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d a7 71 e5 ab[ 	]*vpsraw \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 71 e5 7b[ 	]*vpsraw \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 21 7b[ 	]*vpsraw \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 71 a4 f0 23 01 00 00 7b[ 	]*vpsraw \$0x7b,0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 62 7f 7b[ 	]*vpsraw \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 a2 00 10 00 00 7b[ 	]*vpsraw \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 62 80 7b[ 	]*vpsraw \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 a2 e0 ef ff ff 7b[ 	]*vpsraw \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 f8 f4[ 	]*vpsubb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 f8 f4[ 	]*vpsubb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 f8 f4[ 	]*vpsubb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f8 31[ 	]*vpsubb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 f8 b4 f0 23 01 00 00[ 	]*vpsubb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f8 72 7f[ 	]*vpsubb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f8 b2 00 08 00 00[ 	]*vpsubb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f8 72 80[ 	]*vpsubb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f8 b2 f0 f7 ff ff[ 	]*vpsubb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 f8 f4[ 	]*vpsubb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 f8 f4[ 	]*vpsubb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 f8 f4[ 	]*vpsubb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f8 31[ 	]*vpsubb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 f8 b4 f0 23 01 00 00[ 	]*vpsubb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f8 72 7f[ 	]*vpsubb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f8 b2 00 10 00 00[ 	]*vpsubb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f8 72 80[ 	]*vpsubb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f8 b2 e0 ef ff ff[ 	]*vpsubb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e8 f4[ 	]*vpsubsb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e8 f4[ 	]*vpsubsb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e8 f4[ 	]*vpsubsb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e8 31[ 	]*vpsubsb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e8 b4 f0 23 01 00 00[ 	]*vpsubsb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e8 72 7f[ 	]*vpsubsb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e8 b2 00 08 00 00[ 	]*vpsubsb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e8 72 80[ 	]*vpsubsb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e8 b2 f0 f7 ff ff[ 	]*vpsubsb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e8 f4[ 	]*vpsubsb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e8 f4[ 	]*vpsubsb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e8 f4[ 	]*vpsubsb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e8 31[ 	]*vpsubsb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e8 b4 f0 23 01 00 00[ 	]*vpsubsb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e8 72 7f[ 	]*vpsubsb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e8 b2 00 10 00 00[ 	]*vpsubsb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e8 72 80[ 	]*vpsubsb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e8 b2 e0 ef ff ff[ 	]*vpsubsb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e9 f4[ 	]*vpsubsw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e9 f4[ 	]*vpsubsw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e9 f4[ 	]*vpsubsw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e9 31[ 	]*vpsubsw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e9 b4 f0 23 01 00 00[ 	]*vpsubsw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e9 72 7f[ 	]*vpsubsw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e9 b2 00 08 00 00[ 	]*vpsubsw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e9 72 80[ 	]*vpsubsw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e9 b2 f0 f7 ff ff[ 	]*vpsubsw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e9 f4[ 	]*vpsubsw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e9 f4[ 	]*vpsubsw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e9 f4[ 	]*vpsubsw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e9 31[ 	]*vpsubsw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e9 b4 f0 23 01 00 00[ 	]*vpsubsw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e9 72 7f[ 	]*vpsubsw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e9 b2 00 10 00 00[ 	]*vpsubsw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e9 72 80[ 	]*vpsubsw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e9 b2 e0 ef ff ff[ 	]*vpsubsw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 d8 f4[ 	]*vpsubusb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 d8 f4[ 	]*vpsubusb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 d8 f4[ 	]*vpsubusb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d8 31[ 	]*vpsubusb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 d8 b4 f0 23 01 00 00[ 	]*vpsubusb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d8 72 7f[ 	]*vpsubusb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d8 b2 00 08 00 00[ 	]*vpsubusb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d8 72 80[ 	]*vpsubusb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d8 b2 f0 f7 ff ff[ 	]*vpsubusb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 d8 f4[ 	]*vpsubusb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 d8 f4[ 	]*vpsubusb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 d8 f4[ 	]*vpsubusb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d8 31[ 	]*vpsubusb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 d8 b4 f0 23 01 00 00[ 	]*vpsubusb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d8 72 7f[ 	]*vpsubusb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d8 b2 00 10 00 00[ 	]*vpsubusb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d8 72 80[ 	]*vpsubusb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d8 b2 e0 ef ff ff[ 	]*vpsubusb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 d9 f4[ 	]*vpsubusw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 d9 f4[ 	]*vpsubusw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 d9 f4[ 	]*vpsubusw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d9 31[ 	]*vpsubusw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 d9 b4 f0 23 01 00 00[ 	]*vpsubusw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d9 72 7f[ 	]*vpsubusw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d9 b2 00 08 00 00[ 	]*vpsubusw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d9 72 80[ 	]*vpsubusw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d9 b2 f0 f7 ff ff[ 	]*vpsubusw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 d9 f4[ 	]*vpsubusw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 d9 f4[ 	]*vpsubusw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 d9 f4[ 	]*vpsubusw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d9 31[ 	]*vpsubusw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 d9 b4 f0 23 01 00 00[ 	]*vpsubusw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d9 72 7f[ 	]*vpsubusw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d9 b2 00 10 00 00[ 	]*vpsubusw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d9 72 80[ 	]*vpsubusw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d9 b2 e0 ef ff ff[ 	]*vpsubusw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 f9 f4[ 	]*vpsubw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 f9 f4[ 	]*vpsubw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 f9 f4[ 	]*vpsubw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f9 31[ 	]*vpsubw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 f9 b4 f0 23 01 00 00[ 	]*vpsubw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f9 72 7f[ 	]*vpsubw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f9 b2 00 08 00 00[ 	]*vpsubw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f9 72 80[ 	]*vpsubw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f9 b2 f0 f7 ff ff[ 	]*vpsubw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 f9 f4[ 	]*vpsubw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 f9 f4[ 	]*vpsubw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 f9 f4[ 	]*vpsubw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f9 31[ 	]*vpsubw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 f9 b4 f0 23 01 00 00[ 	]*vpsubw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f9 72 7f[ 	]*vpsubw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f9 b2 00 10 00 00[ 	]*vpsubw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f9 72 80[ 	]*vpsubw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f9 b2 e0 ef ff ff[ 	]*vpsubw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 68 f4[ 	]*vpunpckhbw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 68 f4[ 	]*vpunpckhbw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 68 f4[ 	]*vpunpckhbw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 68 31[ 	]*vpunpckhbw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 68 b4 f0 23 01 00 00[ 	]*vpunpckhbw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 68 72 7f[ 	]*vpunpckhbw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 68 b2 00 08 00 00[ 	]*vpunpckhbw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 68 72 80[ 	]*vpunpckhbw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 68 b2 f0 f7 ff ff[ 	]*vpunpckhbw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 68 f4[ 	]*vpunpckhbw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 68 f4[ 	]*vpunpckhbw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 68 f4[ 	]*vpunpckhbw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 68 31[ 	]*vpunpckhbw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 68 b4 f0 23 01 00 00[ 	]*vpunpckhbw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 68 72 7f[ 	]*vpunpckhbw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 68 b2 00 10 00 00[ 	]*vpunpckhbw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 68 72 80[ 	]*vpunpckhbw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 68 b2 e0 ef ff ff[ 	]*vpunpckhbw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 69 f4[ 	]*vpunpckhwd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 69 f4[ 	]*vpunpckhwd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 69 f4[ 	]*vpunpckhwd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 69 31[ 	]*vpunpckhwd \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 69 b4 f0 23 01 00 00[ 	]*vpunpckhwd 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 69 72 7f[ 	]*vpunpckhwd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 69 b2 00 08 00 00[ 	]*vpunpckhwd 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 69 72 80[ 	]*vpunpckhwd -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 69 b2 f0 f7 ff ff[ 	]*vpunpckhwd -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 69 f4[ 	]*vpunpckhwd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 69 f4[ 	]*vpunpckhwd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 69 f4[ 	]*vpunpckhwd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 69 31[ 	]*vpunpckhwd \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 69 b4 f0 23 01 00 00[ 	]*vpunpckhwd 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 69 72 7f[ 	]*vpunpckhwd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 69 b2 00 10 00 00[ 	]*vpunpckhwd 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 69 72 80[ 	]*vpunpckhwd -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 69 b2 e0 ef ff ff[ 	]*vpunpckhwd -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 60 f4[ 	]*vpunpcklbw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 60 f4[ 	]*vpunpcklbw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 60 f4[ 	]*vpunpcklbw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 60 31[ 	]*vpunpcklbw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 60 b4 f0 23 01 00 00[ 	]*vpunpcklbw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 60 72 7f[ 	]*vpunpcklbw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 60 b2 00 08 00 00[ 	]*vpunpcklbw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 60 72 80[ 	]*vpunpcklbw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 60 b2 f0 f7 ff ff[ 	]*vpunpcklbw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 60 f4[ 	]*vpunpcklbw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 60 f4[ 	]*vpunpcklbw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 60 f4[ 	]*vpunpcklbw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 60 31[ 	]*vpunpcklbw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 60 b4 f0 23 01 00 00[ 	]*vpunpcklbw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 60 72 7f[ 	]*vpunpcklbw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 60 b2 00 10 00 00[ 	]*vpunpcklbw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 60 72 80[ 	]*vpunpcklbw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 60 b2 e0 ef ff ff[ 	]*vpunpcklbw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 61 f4[ 	]*vpunpcklwd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 61 f4[ 	]*vpunpcklwd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 61 f4[ 	]*vpunpcklwd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 61 31[ 	]*vpunpcklwd \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 61 b4 f0 23 01 00 00[ 	]*vpunpcklwd 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 61 72 7f[ 	]*vpunpcklwd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 61 b2 00 08 00 00[ 	]*vpunpcklwd 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 61 72 80[ 	]*vpunpcklwd -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 61 b2 f0 f7 ff ff[ 	]*vpunpcklwd -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 61 f4[ 	]*vpunpcklwd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 61 f4[ 	]*vpunpcklwd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 61 f4[ 	]*vpunpcklwd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 61 31[ 	]*vpunpcklwd \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 61 b4 f0 23 01 00 00[ 	]*vpunpcklwd 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 61 72 7f[ 	]*vpunpcklwd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 61 b2 00 10 00 00[ 	]*vpunpcklwd 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 61 72 80[ 	]*vpunpcklwd -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 61 b2 e0 ef ff ff[ 	]*vpunpcklwd -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 73 fd ab[ 	]*vpslldq \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 73 fd 7b[ 	]*vpslldq \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 39 7b[ 	]*vpslldq \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 73 bc f0 23 01 00 00 7b[ 	]*vpslldq \$0x7b,0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 7a 7f 7b[ 	]*vpslldq \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 ba 00 08 00 00 7b[ 	]*vpslldq \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 7a 80 7b[ 	]*vpslldq \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 ba f0 f7 ff ff 7b[ 	]*vpslldq \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 73 fd ab[ 	]*vpslldq \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 73 fd 7b[ 	]*vpslldq \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 39 7b[ 	]*vpslldq \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 73 bc f0 23 01 00 00 7b[ 	]*vpslldq \$0x7b,0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 7a 7f 7b[ 	]*vpslldq \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 ba 00 10 00 00 7b[ 	]*vpslldq \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 7a 80 7b[ 	]*vpslldq \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 ba e0 ef ff ff 7b[ 	]*vpslldq \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 71 f5 ab[ 	]*vpsllw \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 71 f5 ab[ 	]*vpsllw \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 87 71 f5 ab[ 	]*vpsllw \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 71 f5 7b[ 	]*vpsllw \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 31 7b[ 	]*vpsllw \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 71 b4 f0 23 01 00 00 7b[ 	]*vpsllw \$0x7b,0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 72 7f 7b[ 	]*vpsllw \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 b2 00 08 00 00 7b[ 	]*vpsllw \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 72 80 7b[ 	]*vpsllw \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 b2 f0 f7 ff ff 7b[ 	]*vpsllw \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 71 f5 ab[ 	]*vpsllw \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 71 f5 ab[ 	]*vpsllw \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d a7 71 f5 ab[ 	]*vpsllw \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 71 f5 7b[ 	]*vpsllw \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 31 7b[ 	]*vpsllw \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 71 b4 f0 23 01 00 00 7b[ 	]*vpsllw \$0x7b,0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 72 7f 7b[ 	]*vpsllw \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 b2 00 10 00 00 7b[ 	]*vpsllw \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 72 80 7b[ 	]*vpsllw \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 b2 e0 ef ff ff 7b[ 	]*vpsllw \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 1c f5[ 	]*vpabsb %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 1c f5[ 	]*vpabsb %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 1c f5[ 	]*vpabsb %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1c 31[ 	]*vpabsb \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 1c b4 f0 34 12 00 00[ 	]*vpabsb 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1c 72 7f[ 	]*vpabsb 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1c b2 00 08 00 00[ 	]*vpabsb 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1c 72 80[ 	]*vpabsb -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1c b2 f0 f7 ff ff[ 	]*vpabsb -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 1c f5[ 	]*vpabsb %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 1c f5[ 	]*vpabsb %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 1c f5[ 	]*vpabsb %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1c 31[ 	]*vpabsb \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 1c b4 f0 34 12 00 00[ 	]*vpabsb 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1c 72 7f[ 	]*vpabsb 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1c b2 00 10 00 00[ 	]*vpabsb 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1c 72 80[ 	]*vpabsb -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1c b2 e0 ef ff ff[ 	]*vpabsb -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 1d f5[ 	]*vpabsw %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 1d f5[ 	]*vpabsw %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 1d f5[ 	]*vpabsw %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1d 31[ 	]*vpabsw \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 1d b4 f0 34 12 00 00[ 	]*vpabsw 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1d 72 7f[ 	]*vpabsw 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1d b2 00 08 00 00[ 	]*vpabsw 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1d 72 80[ 	]*vpabsw -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 1d b2 f0 f7 ff ff[ 	]*vpabsw -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 1d f5[ 	]*vpabsw %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 1d f5[ 	]*vpabsw %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 1d f5[ 	]*vpabsw %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1d 31[ 	]*vpabsw \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 1d b4 f0 34 12 00 00[ 	]*vpabsw 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1d 72 7f[ 	]*vpabsw 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1d b2 00 10 00 00[ 	]*vpabsw 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1d 72 80[ 	]*vpabsw -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1d b2 e0 ef ff ff[ 	]*vpabsw -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 63 f4[ 	]*vpacksswb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 63 f4[ 	]*vpacksswb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 63 f4[ 	]*vpacksswb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 63 31[ 	]*vpacksswb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 63 b4 f0 34 12 00 00[ 	]*vpacksswb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 63 72 7f[ 	]*vpacksswb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 63 b2 00 08 00 00[ 	]*vpacksswb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 63 72 80[ 	]*vpacksswb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 63 b2 f0 f7 ff ff[ 	]*vpacksswb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 63 f4[ 	]*vpacksswb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 63 f4[ 	]*vpacksswb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 63 f4[ 	]*vpacksswb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 63 31[ 	]*vpacksswb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 63 b4 f0 34 12 00 00[ 	]*vpacksswb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 63 72 7f[ 	]*vpacksswb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 63 b2 00 10 00 00[ 	]*vpacksswb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 63 72 80[ 	]*vpacksswb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 63 b2 e0 ef ff ff[ 	]*vpacksswb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 67 f4[ 	]*vpackuswb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 67 f4[ 	]*vpackuswb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 67 f4[ 	]*vpackuswb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 67 31[ 	]*vpackuswb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 67 b4 f0 34 12 00 00[ 	]*vpackuswb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 67 72 7f[ 	]*vpackuswb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 67 b2 00 08 00 00[ 	]*vpackuswb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 67 72 80[ 	]*vpackuswb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 67 b2 f0 f7 ff ff[ 	]*vpackuswb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 67 f4[ 	]*vpackuswb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 67 f4[ 	]*vpackuswb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 67 f4[ 	]*vpackuswb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 67 31[ 	]*vpackuswb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 67 b4 f0 34 12 00 00[ 	]*vpackuswb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 67 72 7f[ 	]*vpackuswb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 67 b2 00 10 00 00[ 	]*vpackuswb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 67 72 80[ 	]*vpackuswb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 67 b2 e0 ef ff ff[ 	]*vpackuswb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 fc f4[ 	]*vpaddb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 fc f4[ 	]*vpaddb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 fc f4[ 	]*vpaddb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fc 31[ 	]*vpaddb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 fc b4 f0 34 12 00 00[ 	]*vpaddb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fc 72 7f[ 	]*vpaddb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fc b2 00 08 00 00[ 	]*vpaddb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fc 72 80[ 	]*vpaddb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fc b2 f0 f7 ff ff[ 	]*vpaddb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 fc f4[ 	]*vpaddb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 fc f4[ 	]*vpaddb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 fc f4[ 	]*vpaddb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fc 31[ 	]*vpaddb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 fc b4 f0 34 12 00 00[ 	]*vpaddb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fc 72 7f[ 	]*vpaddb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fc b2 00 10 00 00[ 	]*vpaddb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fc 72 80[ 	]*vpaddb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fc b2 e0 ef ff ff[ 	]*vpaddb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 ec f4[ 	]*vpaddsb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 ec f4[ 	]*vpaddsb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 ec f4[ 	]*vpaddsb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ec 31[ 	]*vpaddsb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 ec b4 f0 34 12 00 00[ 	]*vpaddsb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ec 72 7f[ 	]*vpaddsb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ec b2 00 08 00 00[ 	]*vpaddsb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ec 72 80[ 	]*vpaddsb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ec b2 f0 f7 ff ff[ 	]*vpaddsb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 ec f4[ 	]*vpaddsb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 ec f4[ 	]*vpaddsb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 ec f4[ 	]*vpaddsb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ec 31[ 	]*vpaddsb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 ec b4 f0 34 12 00 00[ 	]*vpaddsb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ec 72 7f[ 	]*vpaddsb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ec b2 00 10 00 00[ 	]*vpaddsb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ec 72 80[ 	]*vpaddsb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ec b2 e0 ef ff ff[ 	]*vpaddsb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 ed f4[ 	]*vpaddsw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 ed f4[ 	]*vpaddsw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 ed f4[ 	]*vpaddsw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ed 31[ 	]*vpaddsw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 ed b4 f0 34 12 00 00[ 	]*vpaddsw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ed 72 7f[ 	]*vpaddsw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ed b2 00 08 00 00[ 	]*vpaddsw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ed 72 80[ 	]*vpaddsw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ed b2 f0 f7 ff ff[ 	]*vpaddsw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 ed f4[ 	]*vpaddsw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 ed f4[ 	]*vpaddsw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 ed f4[ 	]*vpaddsw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ed 31[ 	]*vpaddsw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 ed b4 f0 34 12 00 00[ 	]*vpaddsw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ed 72 7f[ 	]*vpaddsw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ed b2 00 10 00 00[ 	]*vpaddsw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ed 72 80[ 	]*vpaddsw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ed b2 e0 ef ff ff[ 	]*vpaddsw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 dc f4[ 	]*vpaddusb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 dc f4[ 	]*vpaddusb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 dc f4[ 	]*vpaddusb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dc 31[ 	]*vpaddusb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 dc b4 f0 34 12 00 00[ 	]*vpaddusb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dc 72 7f[ 	]*vpaddusb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dc b2 00 08 00 00[ 	]*vpaddusb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dc 72 80[ 	]*vpaddusb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dc b2 f0 f7 ff ff[ 	]*vpaddusb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 dc f4[ 	]*vpaddusb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 dc f4[ 	]*vpaddusb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 dc f4[ 	]*vpaddusb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dc 31[ 	]*vpaddusb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 dc b4 f0 34 12 00 00[ 	]*vpaddusb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dc 72 7f[ 	]*vpaddusb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dc b2 00 10 00 00[ 	]*vpaddusb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dc 72 80[ 	]*vpaddusb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dc b2 e0 ef ff ff[ 	]*vpaddusb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 dd f4[ 	]*vpaddusw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 dd f4[ 	]*vpaddusw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 dd f4[ 	]*vpaddusw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dd 31[ 	]*vpaddusw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 dd b4 f0 34 12 00 00[ 	]*vpaddusw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dd 72 7f[ 	]*vpaddusw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dd b2 00 08 00 00[ 	]*vpaddusw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dd 72 80[ 	]*vpaddusw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 dd b2 f0 f7 ff ff[ 	]*vpaddusw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 dd f4[ 	]*vpaddusw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 dd f4[ 	]*vpaddusw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 dd f4[ 	]*vpaddusw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dd 31[ 	]*vpaddusw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 dd b4 f0 34 12 00 00[ 	]*vpaddusw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dd 72 7f[ 	]*vpaddusw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dd b2 00 10 00 00[ 	]*vpaddusw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dd 72 80[ 	]*vpaddusw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 dd b2 e0 ef ff ff[ 	]*vpaddusw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 fd f4[ 	]*vpaddw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 fd f4[ 	]*vpaddw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 fd f4[ 	]*vpaddw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fd 31[ 	]*vpaddw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 fd b4 f0 34 12 00 00[ 	]*vpaddw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fd 72 7f[ 	]*vpaddw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fd b2 00 08 00 00[ 	]*vpaddw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fd 72 80[ 	]*vpaddw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 fd b2 f0 f7 ff ff[ 	]*vpaddw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 fd f4[ 	]*vpaddw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 fd f4[ 	]*vpaddw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 fd f4[ 	]*vpaddw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fd 31[ 	]*vpaddw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 fd b4 f0 34 12 00 00[ 	]*vpaddw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fd 72 7f[ 	]*vpaddw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fd b2 00 10 00 00[ 	]*vpaddw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fd 72 80[ 	]*vpaddw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 fd b2 e0 ef ff ff[ 	]*vpaddw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 0f f4 ab[ 	]*vpalignr \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 0f f4 ab[ 	]*vpalignr \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 0f f4 ab[ 	]*vpalignr \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 0f f4 7b[ 	]*vpalignr \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 0f 31 7b[ 	]*vpalignr \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 0f b4 f0 34 12 00 00 7b[ 	]*vpalignr \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 0f 72 7f 7b[ 	]*vpalignr \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 0f b2 00 08 00 00 7b[ 	]*vpalignr \$0x7b,0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 0f 72 80 7b[ 	]*vpalignr \$0x7b,-0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 0f b2 f0 f7 ff ff 7b[ 	]*vpalignr \$0x7b,-0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 0f f4 ab[ 	]*vpalignr \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 0f f4 ab[ 	]*vpalignr \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 0f f4 ab[ 	]*vpalignr \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 0f f4 7b[ 	]*vpalignr \$0x7b,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 0f 31 7b[ 	]*vpalignr \$0x7b,\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 0f b4 f0 34 12 00 00 7b[ 	]*vpalignr \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 0f 72 7f 7b[ 	]*vpalignr \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 0f b2 00 10 00 00 7b[ 	]*vpalignr \$0x7b,0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 0f 72 80 7b[ 	]*vpalignr \$0x7b,-0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 0f b2 e0 ef ff ff 7b[ 	]*vpalignr \$0x7b,-0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e0 f4[ 	]*vpavgb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e0 f4[ 	]*vpavgb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e0 f4[ 	]*vpavgb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e0 31[ 	]*vpavgb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e0 b4 f0 34 12 00 00[ 	]*vpavgb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e0 72 7f[ 	]*vpavgb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e0 b2 00 08 00 00[ 	]*vpavgb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e0 72 80[ 	]*vpavgb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e0 b2 f0 f7 ff ff[ 	]*vpavgb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e0 f4[ 	]*vpavgb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e0 f4[ 	]*vpavgb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e0 f4[ 	]*vpavgb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e0 31[ 	]*vpavgb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e0 b4 f0 34 12 00 00[ 	]*vpavgb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e0 72 7f[ 	]*vpavgb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e0 b2 00 10 00 00[ 	]*vpavgb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e0 72 80[ 	]*vpavgb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e0 b2 e0 ef ff ff[ 	]*vpavgb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e3 f4[ 	]*vpavgw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e3 f4[ 	]*vpavgw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e3 f4[ 	]*vpavgw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e3 31[ 	]*vpavgw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e3 b4 f0 34 12 00 00[ 	]*vpavgw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e3 72 7f[ 	]*vpavgw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e3 b2 00 08 00 00[ 	]*vpavgw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e3 72 80[ 	]*vpavgw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e3 b2 f0 f7 ff ff[ 	]*vpavgw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e3 f4[ 	]*vpavgw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e3 f4[ 	]*vpavgw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e3 f4[ 	]*vpavgw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e3 31[ 	]*vpavgw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e3 b4 f0 34 12 00 00[ 	]*vpavgw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e3 72 7f[ 	]*vpavgw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e3 b2 00 10 00 00[ 	]*vpavgw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e3 72 80[ 	]*vpavgw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e3 b2 e0 ef ff ff[ 	]*vpavgw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 74 ed[ 	]*vpcmpeqb %xmm29,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 74 ed[ 	]*vpcmpeqb %xmm29,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 74 29[ 	]*vpcmpeqb \(%rcx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 74 ac f0 34 12 00 00[ 	]*vpcmpeqb 0x1234\(%rax,%r14,8\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 74 6a 7f[ 	]*vpcmpeqb 0x7f0\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 74 aa 00 08 00 00[ 	]*vpcmpeqb 0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 74 6a 80[ 	]*vpcmpeqb -0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 74 aa f0 f7 ff ff[ 	]*vpcmpeqb -0x810\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 74 ed[ 	]*vpcmpeqb %ymm29,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 74 ed[ 	]*vpcmpeqb %ymm29,%ymm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 74 29[ 	]*vpcmpeqb \(%rcx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 74 ac f0 34 12 00 00[ 	]*vpcmpeqb 0x1234\(%rax,%r14,8\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 74 6a 7f[ 	]*vpcmpeqb 0xfe0\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 74 aa 00 10 00 00[ 	]*vpcmpeqb 0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 74 6a 80[ 	]*vpcmpeqb -0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 74 aa e0 ef ff ff[ 	]*vpcmpeqb -0x1020\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 75 ed[ 	]*vpcmpeqw %xmm29,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 75 ed[ 	]*vpcmpeqw %xmm29,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 75 29[ 	]*vpcmpeqw \(%rcx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 75 ac f0 34 12 00 00[ 	]*vpcmpeqw 0x1234\(%rax,%r14,8\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 75 6a 7f[ 	]*vpcmpeqw 0x7f0\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 75 aa 00 08 00 00[ 	]*vpcmpeqw 0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 75 6a 80[ 	]*vpcmpeqw -0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 75 aa f0 f7 ff ff[ 	]*vpcmpeqw -0x810\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 75 ed[ 	]*vpcmpeqw %ymm29,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 75 ed[ 	]*vpcmpeqw %ymm29,%ymm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 75 29[ 	]*vpcmpeqw \(%rcx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 75 ac f0 34 12 00 00[ 	]*vpcmpeqw 0x1234\(%rax,%r14,8\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 75 6a 7f[ 	]*vpcmpeqw 0xfe0\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 75 aa 00 10 00 00[ 	]*vpcmpeqw 0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 75 6a 80[ 	]*vpcmpeqw -0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 75 aa e0 ef ff ff[ 	]*vpcmpeqw -0x1020\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 64 ed[ 	]*vpcmpgtb %xmm29,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 64 ed[ 	]*vpcmpgtb %xmm29,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 64 29[ 	]*vpcmpgtb \(%rcx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 64 ac f0 34 12 00 00[ 	]*vpcmpgtb 0x1234\(%rax,%r14,8\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 64 6a 7f[ 	]*vpcmpgtb 0x7f0\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 64 aa 00 08 00 00[ 	]*vpcmpgtb 0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 64 6a 80[ 	]*vpcmpgtb -0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 64 aa f0 f7 ff ff[ 	]*vpcmpgtb -0x810\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 64 ed[ 	]*vpcmpgtb %ymm29,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 64 ed[ 	]*vpcmpgtb %ymm29,%ymm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 64 29[ 	]*vpcmpgtb \(%rcx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 64 ac f0 34 12 00 00[ 	]*vpcmpgtb 0x1234\(%rax,%r14,8\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 64 6a 7f[ 	]*vpcmpgtb 0xfe0\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 64 aa 00 10 00 00[ 	]*vpcmpgtb 0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 64 6a 80[ 	]*vpcmpgtb -0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 64 aa e0 ef ff ff[ 	]*vpcmpgtb -0x1020\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 65 ed[ 	]*vpcmpgtw %xmm29,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 65 ed[ 	]*vpcmpgtw %xmm29,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 65 29[ 	]*vpcmpgtw \(%rcx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 65 ac f0 34 12 00 00[ 	]*vpcmpgtw 0x1234\(%rax,%r14,8\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 65 6a 7f[ 	]*vpcmpgtw 0x7f0\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 65 aa 00 08 00 00[ 	]*vpcmpgtw 0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 65 6a 80[ 	]*vpcmpgtw -0x800\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 65 aa f0 f7 ff ff[ 	]*vpcmpgtw -0x810\(%rdx\),%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 65 ed[ 	]*vpcmpgtw %ymm29,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 65 ed[ 	]*vpcmpgtw %ymm29,%ymm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 65 29[ 	]*vpcmpgtw \(%rcx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 65 ac f0 34 12 00 00[ 	]*vpcmpgtw 0x1234\(%rax,%r14,8\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 65 6a 7f[ 	]*vpcmpgtw 0xfe0\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 65 aa 00 10 00 00[ 	]*vpcmpgtw 0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 65 6a 80[ 	]*vpcmpgtw -0x1000\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 65 aa e0 ef ff ff[ 	]*vpcmpgtw -0x1020\(%rdx\),%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 04 f4[ 	]*vpmaddubsw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 04 f4[ 	]*vpmaddubsw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 04 f4[ 	]*vpmaddubsw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 04 31[ 	]*vpmaddubsw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 04 b4 f0 34 12 00 00[ 	]*vpmaddubsw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 04 72 7f[ 	]*vpmaddubsw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 04 b2 00 08 00 00[ 	]*vpmaddubsw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 04 72 80[ 	]*vpmaddubsw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 04 b2 f0 f7 ff ff[ 	]*vpmaddubsw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 04 f4[ 	]*vpmaddubsw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 04 f4[ 	]*vpmaddubsw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 04 f4[ 	]*vpmaddubsw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 04 31[ 	]*vpmaddubsw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 04 b4 f0 34 12 00 00[ 	]*vpmaddubsw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 04 72 7f[ 	]*vpmaddubsw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 04 b2 00 10 00 00[ 	]*vpmaddubsw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 04 72 80[ 	]*vpmaddubsw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 04 b2 e0 ef ff ff[ 	]*vpmaddubsw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 f5 f4[ 	]*vpmaddwd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 f5 f4[ 	]*vpmaddwd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 f5 f4[ 	]*vpmaddwd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f5 31[ 	]*vpmaddwd \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 f5 b4 f0 34 12 00 00[ 	]*vpmaddwd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f5 72 7f[ 	]*vpmaddwd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f5 b2 00 08 00 00[ 	]*vpmaddwd 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f5 72 80[ 	]*vpmaddwd -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f5 b2 f0 f7 ff ff[ 	]*vpmaddwd -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 f5 f4[ 	]*vpmaddwd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 f5 f4[ 	]*vpmaddwd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 f5 f4[ 	]*vpmaddwd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f5 31[ 	]*vpmaddwd \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 f5 b4 f0 34 12 00 00[ 	]*vpmaddwd 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f5 72 7f[ 	]*vpmaddwd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f5 b2 00 10 00 00[ 	]*vpmaddwd 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f5 72 80[ 	]*vpmaddwd -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f5 b2 e0 ef ff ff[ 	]*vpmaddwd -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 3c f4[ 	]*vpmaxsb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 3c f4[ 	]*vpmaxsb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 3c f4[ 	]*vpmaxsb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3c 31[ 	]*vpmaxsb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 3c b4 f0 34 12 00 00[ 	]*vpmaxsb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3c 72 7f[ 	]*vpmaxsb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3c b2 00 08 00 00[ 	]*vpmaxsb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3c 72 80[ 	]*vpmaxsb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3c b2 f0 f7 ff ff[ 	]*vpmaxsb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 3c f4[ 	]*vpmaxsb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 3c f4[ 	]*vpmaxsb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 3c f4[ 	]*vpmaxsb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3c 31[ 	]*vpmaxsb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 3c b4 f0 34 12 00 00[ 	]*vpmaxsb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3c 72 7f[ 	]*vpmaxsb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3c b2 00 10 00 00[ 	]*vpmaxsb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3c 72 80[ 	]*vpmaxsb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3c b2 e0 ef ff ff[ 	]*vpmaxsb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 ee f4[ 	]*vpmaxsw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 ee f4[ 	]*vpmaxsw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 ee f4[ 	]*vpmaxsw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ee 31[ 	]*vpmaxsw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 ee b4 f0 34 12 00 00[ 	]*vpmaxsw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ee 72 7f[ 	]*vpmaxsw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ee b2 00 08 00 00[ 	]*vpmaxsw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ee 72 80[ 	]*vpmaxsw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ee b2 f0 f7 ff ff[ 	]*vpmaxsw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 ee f4[ 	]*vpmaxsw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 ee f4[ 	]*vpmaxsw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 ee f4[ 	]*vpmaxsw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ee 31[ 	]*vpmaxsw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 ee b4 f0 34 12 00 00[ 	]*vpmaxsw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ee 72 7f[ 	]*vpmaxsw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ee b2 00 10 00 00[ 	]*vpmaxsw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ee 72 80[ 	]*vpmaxsw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ee b2 e0 ef ff ff[ 	]*vpmaxsw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 de f4[ 	]*vpmaxub %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 de f4[ 	]*vpmaxub %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 de f4[ 	]*vpmaxub %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 de 31[ 	]*vpmaxub \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 de b4 f0 34 12 00 00[ 	]*vpmaxub 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 de 72 7f[ 	]*vpmaxub 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 de b2 00 08 00 00[ 	]*vpmaxub 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 de 72 80[ 	]*vpmaxub -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 de b2 f0 f7 ff ff[ 	]*vpmaxub -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 de f4[ 	]*vpmaxub %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 de f4[ 	]*vpmaxub %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 de f4[ 	]*vpmaxub %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 de 31[ 	]*vpmaxub \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 de b4 f0 34 12 00 00[ 	]*vpmaxub 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 de 72 7f[ 	]*vpmaxub 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 de b2 00 10 00 00[ 	]*vpmaxub 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 de 72 80[ 	]*vpmaxub -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 de b2 e0 ef ff ff[ 	]*vpmaxub -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 3e f4[ 	]*vpmaxuw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 3e f4[ 	]*vpmaxuw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 3e f4[ 	]*vpmaxuw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3e 31[ 	]*vpmaxuw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 3e b4 f0 34 12 00 00[ 	]*vpmaxuw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3e 72 7f[ 	]*vpmaxuw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3e b2 00 08 00 00[ 	]*vpmaxuw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3e 72 80[ 	]*vpmaxuw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3e b2 f0 f7 ff ff[ 	]*vpmaxuw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 3e f4[ 	]*vpmaxuw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 3e f4[ 	]*vpmaxuw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 3e f4[ 	]*vpmaxuw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3e 31[ 	]*vpmaxuw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 3e b4 f0 34 12 00 00[ 	]*vpmaxuw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3e 72 7f[ 	]*vpmaxuw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3e b2 00 10 00 00[ 	]*vpmaxuw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3e 72 80[ 	]*vpmaxuw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3e b2 e0 ef ff ff[ 	]*vpmaxuw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 38 f4[ 	]*vpminsb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 38 f4[ 	]*vpminsb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 38 f4[ 	]*vpminsb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 38 31[ 	]*vpminsb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 38 b4 f0 34 12 00 00[ 	]*vpminsb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 38 72 7f[ 	]*vpminsb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 38 b2 00 08 00 00[ 	]*vpminsb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 38 72 80[ 	]*vpminsb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 38 b2 f0 f7 ff ff[ 	]*vpminsb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 38 f4[ 	]*vpminsb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 38 f4[ 	]*vpminsb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 38 f4[ 	]*vpminsb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 38 31[ 	]*vpminsb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 38 b4 f0 34 12 00 00[ 	]*vpminsb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 38 72 7f[ 	]*vpminsb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 38 b2 00 10 00 00[ 	]*vpminsb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 38 72 80[ 	]*vpminsb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 38 b2 e0 ef ff ff[ 	]*vpminsb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 ea f4[ 	]*vpminsw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 ea f4[ 	]*vpminsw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 ea f4[ 	]*vpminsw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ea 31[ 	]*vpminsw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 ea b4 f0 34 12 00 00[ 	]*vpminsw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ea 72 7f[ 	]*vpminsw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ea b2 00 08 00 00[ 	]*vpminsw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ea 72 80[ 	]*vpminsw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 ea b2 f0 f7 ff ff[ 	]*vpminsw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 ea f4[ 	]*vpminsw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 ea f4[ 	]*vpminsw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 ea f4[ 	]*vpminsw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ea 31[ 	]*vpminsw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 ea b4 f0 34 12 00 00[ 	]*vpminsw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ea 72 7f[ 	]*vpminsw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ea b2 00 10 00 00[ 	]*vpminsw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ea 72 80[ 	]*vpminsw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 ea b2 e0 ef ff ff[ 	]*vpminsw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 da f4[ 	]*vpminub %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 da f4[ 	]*vpminub %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 da f4[ 	]*vpminub %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 da 31[ 	]*vpminub \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 da b4 f0 34 12 00 00[ 	]*vpminub 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 da 72 7f[ 	]*vpminub 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 da b2 00 08 00 00[ 	]*vpminub 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 da 72 80[ 	]*vpminub -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 da b2 f0 f7 ff ff[ 	]*vpminub -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 da f4[ 	]*vpminub %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 da f4[ 	]*vpminub %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 da f4[ 	]*vpminub %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 da 31[ 	]*vpminub \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 da b4 f0 34 12 00 00[ 	]*vpminub 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 da 72 7f[ 	]*vpminub 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 da b2 00 10 00 00[ 	]*vpminub 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 da 72 80[ 	]*vpminub -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 da b2 e0 ef ff ff[ 	]*vpminub -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 3a f4[ 	]*vpminuw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 3a f4[ 	]*vpminuw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 3a f4[ 	]*vpminuw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3a 31[ 	]*vpminuw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 3a b4 f0 34 12 00 00[ 	]*vpminuw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3a 72 7f[ 	]*vpminuw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3a b2 00 08 00 00[ 	]*vpminuw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3a 72 80[ 	]*vpminuw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 3a b2 f0 f7 ff ff[ 	]*vpminuw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 3a f4[ 	]*vpminuw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 3a f4[ 	]*vpminuw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 3a f4[ 	]*vpminuw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3a 31[ 	]*vpminuw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 3a b4 f0 34 12 00 00[ 	]*vpminuw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3a 72 7f[ 	]*vpminuw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3a b2 00 10 00 00[ 	]*vpminuw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3a 72 80[ 	]*vpminuw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 3a b2 e0 ef ff ff[ 	]*vpminuw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 20 f5[ 	]*vpmovsxbw %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 20 f5[ 	]*vpmovsxbw %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 20 f5[ 	]*vpmovsxbw %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 20 31[ 	]*vpmovsxbw \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 20 b4 f0 34 12 00 00[ 	]*vpmovsxbw 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 20 72 7f[ 	]*vpmovsxbw 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 20 b2 00 04 00 00[ 	]*vpmovsxbw 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 20 72 80[ 	]*vpmovsxbw -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 20 b2 f8 fb ff ff[ 	]*vpmovsxbw -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 20 f5[ 	]*vpmovsxbw %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 20 f5[ 	]*vpmovsxbw %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 20 f5[ 	]*vpmovsxbw %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 20 31[ 	]*vpmovsxbw \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 20 b4 f0 34 12 00 00[ 	]*vpmovsxbw 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 20 72 7f[ 	]*vpmovsxbw 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 20 b2 00 08 00 00[ 	]*vpmovsxbw 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 20 72 80[ 	]*vpmovsxbw -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 20 b2 f0 f7 ff ff[ 	]*vpmovsxbw -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 30 f5[ 	]*vpmovzxbw %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 30 f5[ 	]*vpmovzxbw %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 30 f5[ 	]*vpmovzxbw %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 30 31[ 	]*vpmovzxbw \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 30 b4 f0 34 12 00 00[ 	]*vpmovzxbw 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 30 72 7f[ 	]*vpmovzxbw 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 30 b2 00 04 00 00[ 	]*vpmovzxbw 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 30 72 80[ 	]*vpmovzxbw -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 30 b2 f8 fb ff ff[ 	]*vpmovzxbw -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 30 f5[ 	]*vpmovzxbw %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 30 f5[ 	]*vpmovzxbw %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 30 f5[ 	]*vpmovzxbw %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 30 31[ 	]*vpmovzxbw \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 30 b4 f0 34 12 00 00[ 	]*vpmovzxbw 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 30 72 7f[ 	]*vpmovzxbw 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 30 b2 00 08 00 00[ 	]*vpmovzxbw 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 30 72 80[ 	]*vpmovzxbw -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 30 b2 f0 f7 ff ff[ 	]*vpmovzxbw -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 0b f4[ 	]*vpmulhrsw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 0b f4[ 	]*vpmulhrsw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 0b f4[ 	]*vpmulhrsw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 0b 31[ 	]*vpmulhrsw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 0b b4 f0 34 12 00 00[ 	]*vpmulhrsw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 0b 72 7f[ 	]*vpmulhrsw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 0b b2 00 08 00 00[ 	]*vpmulhrsw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 0b 72 80[ 	]*vpmulhrsw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 0b b2 f0 f7 ff ff[ 	]*vpmulhrsw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 0b f4[ 	]*vpmulhrsw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 0b f4[ 	]*vpmulhrsw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 0b f4[ 	]*vpmulhrsw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 0b 31[ 	]*vpmulhrsw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 0b b4 f0 34 12 00 00[ 	]*vpmulhrsw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 0b 72 7f[ 	]*vpmulhrsw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 0b b2 00 10 00 00[ 	]*vpmulhrsw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 0b 72 80[ 	]*vpmulhrsw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 0b b2 e0 ef ff ff[ 	]*vpmulhrsw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e4 f4[ 	]*vpmulhuw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e4 f4[ 	]*vpmulhuw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e4 f4[ 	]*vpmulhuw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e4 31[ 	]*vpmulhuw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e4 b4 f0 34 12 00 00[ 	]*vpmulhuw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e4 72 7f[ 	]*vpmulhuw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e4 b2 00 08 00 00[ 	]*vpmulhuw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e4 72 80[ 	]*vpmulhuw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e4 b2 f0 f7 ff ff[ 	]*vpmulhuw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e4 f4[ 	]*vpmulhuw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e4 f4[ 	]*vpmulhuw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e4 f4[ 	]*vpmulhuw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e4 31[ 	]*vpmulhuw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e4 b4 f0 34 12 00 00[ 	]*vpmulhuw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e4 72 7f[ 	]*vpmulhuw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e4 b2 00 10 00 00[ 	]*vpmulhuw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e4 72 80[ 	]*vpmulhuw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e4 b2 e0 ef ff ff[ 	]*vpmulhuw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e5 f4[ 	]*vpmulhw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e5 f4[ 	]*vpmulhw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e5 f4[ 	]*vpmulhw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e5 31[ 	]*vpmulhw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e5 b4 f0 34 12 00 00[ 	]*vpmulhw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e5 72 7f[ 	]*vpmulhw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e5 b2 00 08 00 00[ 	]*vpmulhw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e5 72 80[ 	]*vpmulhw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e5 b2 f0 f7 ff ff[ 	]*vpmulhw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e5 f4[ 	]*vpmulhw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e5 f4[ 	]*vpmulhw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e5 f4[ 	]*vpmulhw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e5 31[ 	]*vpmulhw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e5 b4 f0 34 12 00 00[ 	]*vpmulhw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e5 72 7f[ 	]*vpmulhw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e5 b2 00 10 00 00[ 	]*vpmulhw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e5 72 80[ 	]*vpmulhw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e5 b2 e0 ef ff ff[ 	]*vpmulhw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 d5 f4[ 	]*vpmullw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 d5 f4[ 	]*vpmullw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 d5 f4[ 	]*vpmullw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d5 31[ 	]*vpmullw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 d5 b4 f0 34 12 00 00[ 	]*vpmullw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d5 72 7f[ 	]*vpmullw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d5 b2 00 08 00 00[ 	]*vpmullw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d5 72 80[ 	]*vpmullw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d5 b2 f0 f7 ff ff[ 	]*vpmullw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 d5 f4[ 	]*vpmullw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 d5 f4[ 	]*vpmullw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 d5 f4[ 	]*vpmullw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d5 31[ 	]*vpmullw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 d5 b4 f0 34 12 00 00[ 	]*vpmullw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d5 72 7f[ 	]*vpmullw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d5 b2 00 10 00 00[ 	]*vpmullw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d5 72 80[ 	]*vpmullw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d5 b2 e0 ef ff ff[ 	]*vpmullw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 f6 f4[ 	]*vpsadbw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f6 31[ 	]*vpsadbw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 f6 b4 f0 34 12 00 00[ 	]*vpsadbw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f6 72 7f[ 	]*vpsadbw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f6 b2 00 08 00 00[ 	]*vpsadbw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f6 72 80[ 	]*vpsadbw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f6 b2 f0 f7 ff ff[ 	]*vpsadbw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 f6 f4[ 	]*vpsadbw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f6 31[ 	]*vpsadbw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 f6 b4 f0 34 12 00 00[ 	]*vpsadbw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f6 72 7f[ 	]*vpsadbw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f6 b2 00 10 00 00[ 	]*vpsadbw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f6 72 80[ 	]*vpsadbw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f6 b2 e0 ef ff ff[ 	]*vpsadbw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 00 f4[ 	]*vpshufb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 00 f4[ 	]*vpshufb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 00 f4[ 	]*vpshufb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 00 31[ 	]*vpshufb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 00 b4 f0 34 12 00 00[ 	]*vpshufb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 00 72 7f[ 	]*vpshufb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 00 b2 00 08 00 00[ 	]*vpshufb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 00 72 80[ 	]*vpshufb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 00 b2 f0 f7 ff ff[ 	]*vpshufb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 00 f4[ 	]*vpshufb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 00 f4[ 	]*vpshufb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 00 f4[ 	]*vpshufb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 00 31[ 	]*vpshufb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 00 b4 f0 34 12 00 00[ 	]*vpshufb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 00 72 7f[ 	]*vpshufb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 00 b2 00 10 00 00[ 	]*vpshufb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 00 72 80[ 	]*vpshufb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 00 b2 e0 ef ff ff[ 	]*vpshufb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 08 70 f5 ab[ 	]*vpshufhw \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 0f 70 f5 ab[ 	]*vpshufhw \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 8f 70 f5 ab[ 	]*vpshufhw \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 08 70 f5 7b[ 	]*vpshufhw \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 70 31 7b[ 	]*vpshufhw \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 08 70 b4 f0 34 12 00 00 7b[ 	]*vpshufhw \$0x7b,0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 70 72 7f 7b[ 	]*vpshufhw \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 70 b2 00 08 00 00 7b[ 	]*vpshufhw \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 70 72 80 7b[ 	]*vpshufhw \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 70 b2 f0 f7 ff ff 7b[ 	]*vpshufhw \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 28 70 f5 ab[ 	]*vpshufhw \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 2f 70 f5 ab[ 	]*vpshufhw \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe af 70 f5 ab[ 	]*vpshufhw \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 28 70 f5 7b[ 	]*vpshufhw \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 70 31 7b[ 	]*vpshufhw \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 28 70 b4 f0 34 12 00 00 7b[ 	]*vpshufhw \$0x7b,0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 70 72 7f 7b[ 	]*vpshufhw \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 70 b2 00 10 00 00 7b[ 	]*vpshufhw \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 70 72 80 7b[ 	]*vpshufhw \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 70 b2 e0 ef ff ff 7b[ 	]*vpshufhw \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 70 f5 ab[ 	]*vpshuflw \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 70 f5 ab[ 	]*vpshuflw \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 70 f5 ab[ 	]*vpshuflw \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 70 f5 7b[ 	]*vpshuflw \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 70 31 7b[ 	]*vpshuflw \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 08 70 b4 f0 34 12 00 00 7b[ 	]*vpshuflw \$0x7b,0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 70 72 7f 7b[ 	]*vpshuflw \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 70 b2 00 08 00 00 7b[ 	]*vpshuflw \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 70 72 80 7b[ 	]*vpshuflw \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 70 b2 f0 f7 ff ff 7b[ 	]*vpshuflw \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 70 f5 ab[ 	]*vpshuflw \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 70 f5 ab[ 	]*vpshuflw \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 70 f5 ab[ 	]*vpshuflw \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 70 f5 7b[ 	]*vpshuflw \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 70 31 7b[ 	]*vpshuflw \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 28 70 b4 f0 34 12 00 00 7b[ 	]*vpshuflw \$0x7b,0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 70 72 7f 7b[ 	]*vpshuflw \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 70 b2 00 10 00 00 7b[ 	]*vpshuflw \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 70 72 80 7b[ 	]*vpshuflw \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 70 b2 e0 ef ff ff 7b[ 	]*vpshuflw \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 f1 f4[ 	]*vpsllw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 f1 f4[ 	]*vpsllw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 f1 f4[ 	]*vpsllw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f1 31[ 	]*vpsllw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 f1 b4 f0 34 12 00 00[ 	]*vpsllw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f1 72 7f[ 	]*vpsllw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f1 b2 00 08 00 00[ 	]*vpsllw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f1 72 80[ 	]*vpsllw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f1 b2 f0 f7 ff ff[ 	]*vpsllw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 f1 f4[ 	]*vpsllw %xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 f1 f4[ 	]*vpsllw %xmm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 f1 f4[ 	]*vpsllw %xmm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f1 31[ 	]*vpsllw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 f1 b4 f0 34 12 00 00[ 	]*vpsllw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f1 72 7f[ 	]*vpsllw 0x7f0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f1 b2 00 08 00 00[ 	]*vpsllw 0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f1 72 80[ 	]*vpsllw -0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f1 b2 f0 f7 ff ff[ 	]*vpsllw -0x810\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e1 f4[ 	]*vpsraw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e1 f4[ 	]*vpsraw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e1 f4[ 	]*vpsraw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e1 31[ 	]*vpsraw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e1 b4 f0 34 12 00 00[ 	]*vpsraw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e1 72 7f[ 	]*vpsraw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e1 b2 00 08 00 00[ 	]*vpsraw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e1 72 80[ 	]*vpsraw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e1 b2 f0 f7 ff ff[ 	]*vpsraw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e1 f4[ 	]*vpsraw %xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e1 f4[ 	]*vpsraw %xmm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e1 f4[ 	]*vpsraw %xmm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e1 31[ 	]*vpsraw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e1 b4 f0 34 12 00 00[ 	]*vpsraw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e1 72 7f[ 	]*vpsraw 0x7f0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e1 b2 00 08 00 00[ 	]*vpsraw 0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e1 72 80[ 	]*vpsraw -0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e1 b2 f0 f7 ff ff[ 	]*vpsraw -0x810\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 d1 f4[ 	]*vpsrlw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 d1 f4[ 	]*vpsrlw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 d1 f4[ 	]*vpsrlw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d1 31[ 	]*vpsrlw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 d1 b4 f0 34 12 00 00[ 	]*vpsrlw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d1 72 7f[ 	]*vpsrlw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d1 b2 00 08 00 00[ 	]*vpsrlw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d1 72 80[ 	]*vpsrlw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d1 b2 f0 f7 ff ff[ 	]*vpsrlw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 d1 f4[ 	]*vpsrlw %xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 d1 f4[ 	]*vpsrlw %xmm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 d1 f4[ 	]*vpsrlw %xmm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d1 31[ 	]*vpsrlw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 d1 b4 f0 34 12 00 00[ 	]*vpsrlw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d1 72 7f[ 	]*vpsrlw 0x7f0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d1 b2 00 08 00 00[ 	]*vpsrlw 0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d1 72 80[ 	]*vpsrlw -0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d1 b2 f0 f7 ff ff[ 	]*vpsrlw -0x810\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 73 dd ab[ 	]*vpsrldq \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 73 dd 7b[ 	]*vpsrldq \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 19 7b[ 	]*vpsrldq \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 73 9c f0 34 12 00 00 7b[ 	]*vpsrldq \$0x7b,0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 5a 7f 7b[ 	]*vpsrldq \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 9a 00 08 00 00 7b[ 	]*vpsrldq \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 5a 80 7b[ 	]*vpsrldq \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 9a f0 f7 ff ff 7b[ 	]*vpsrldq \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 73 dd ab[ 	]*vpsrldq \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 73 dd 7b[ 	]*vpsrldq \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 19 7b[ 	]*vpsrldq \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 73 9c f0 34 12 00 00 7b[ 	]*vpsrldq \$0x7b,0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 5a 7f 7b[ 	]*vpsrldq \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 9a 00 10 00 00 7b[ 	]*vpsrldq \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 5a 80 7b[ 	]*vpsrldq \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 9a e0 ef ff ff 7b[ 	]*vpsrldq \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 71 d5 ab[ 	]*vpsrlw \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 71 d5 ab[ 	]*vpsrlw \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 87 71 d5 ab[ 	]*vpsrlw \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 71 d5 7b[ 	]*vpsrlw \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 11 7b[ 	]*vpsrlw \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 71 94 f0 34 12 00 00 7b[ 	]*vpsrlw \$0x7b,0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 52 7f 7b[ 	]*vpsrlw \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 92 00 08 00 00 7b[ 	]*vpsrlw \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 52 80 7b[ 	]*vpsrlw \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 92 f0 f7 ff ff 7b[ 	]*vpsrlw \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 71 d5 ab[ 	]*vpsrlw \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 71 d5 ab[ 	]*vpsrlw \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d a7 71 d5 ab[ 	]*vpsrlw \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 71 d5 7b[ 	]*vpsrlw \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 11 7b[ 	]*vpsrlw \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 71 94 f0 34 12 00 00 7b[ 	]*vpsrlw \$0x7b,0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 52 7f 7b[ 	]*vpsrlw \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 92 00 10 00 00 7b[ 	]*vpsrlw \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 52 80 7b[ 	]*vpsrlw \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 92 e0 ef ff ff 7b[ 	]*vpsrlw \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 71 e5 ab[ 	]*vpsraw \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 71 e5 ab[ 	]*vpsraw \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 87 71 e5 ab[ 	]*vpsraw \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 71 e5 7b[ 	]*vpsraw \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 21 7b[ 	]*vpsraw \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 71 a4 f0 34 12 00 00 7b[ 	]*vpsraw \$0x7b,0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 62 7f 7b[ 	]*vpsraw \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 a2 00 08 00 00 7b[ 	]*vpsraw \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 62 80 7b[ 	]*vpsraw \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 a2 f0 f7 ff ff 7b[ 	]*vpsraw \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 71 e5 ab[ 	]*vpsraw \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 71 e5 ab[ 	]*vpsraw \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d a7 71 e5 ab[ 	]*vpsraw \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 71 e5 7b[ 	]*vpsraw \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 21 7b[ 	]*vpsraw \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 71 a4 f0 34 12 00 00 7b[ 	]*vpsraw \$0x7b,0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 62 7f 7b[ 	]*vpsraw \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 a2 00 10 00 00 7b[ 	]*vpsraw \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 62 80 7b[ 	]*vpsraw \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 a2 e0 ef ff ff 7b[ 	]*vpsraw \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 f8 f4[ 	]*vpsubb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 f8 f4[ 	]*vpsubb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 f8 f4[ 	]*vpsubb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f8 31[ 	]*vpsubb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 f8 b4 f0 34 12 00 00[ 	]*vpsubb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f8 72 7f[ 	]*vpsubb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f8 b2 00 08 00 00[ 	]*vpsubb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f8 72 80[ 	]*vpsubb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f8 b2 f0 f7 ff ff[ 	]*vpsubb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 f8 f4[ 	]*vpsubb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 f8 f4[ 	]*vpsubb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 f8 f4[ 	]*vpsubb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f8 31[ 	]*vpsubb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 f8 b4 f0 34 12 00 00[ 	]*vpsubb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f8 72 7f[ 	]*vpsubb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f8 b2 00 10 00 00[ 	]*vpsubb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f8 72 80[ 	]*vpsubb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f8 b2 e0 ef ff ff[ 	]*vpsubb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e8 f4[ 	]*vpsubsb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e8 f4[ 	]*vpsubsb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e8 f4[ 	]*vpsubsb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e8 31[ 	]*vpsubsb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e8 b4 f0 34 12 00 00[ 	]*vpsubsb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e8 72 7f[ 	]*vpsubsb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e8 b2 00 08 00 00[ 	]*vpsubsb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e8 72 80[ 	]*vpsubsb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e8 b2 f0 f7 ff ff[ 	]*vpsubsb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e8 f4[ 	]*vpsubsb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e8 f4[ 	]*vpsubsb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e8 f4[ 	]*vpsubsb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e8 31[ 	]*vpsubsb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e8 b4 f0 34 12 00 00[ 	]*vpsubsb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e8 72 7f[ 	]*vpsubsb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e8 b2 00 10 00 00[ 	]*vpsubsb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e8 72 80[ 	]*vpsubsb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e8 b2 e0 ef ff ff[ 	]*vpsubsb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 e9 f4[ 	]*vpsubsw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 e9 f4[ 	]*vpsubsw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 e9 f4[ 	]*vpsubsw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e9 31[ 	]*vpsubsw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 e9 b4 f0 34 12 00 00[ 	]*vpsubsw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e9 72 7f[ 	]*vpsubsw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e9 b2 00 08 00 00[ 	]*vpsubsw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e9 72 80[ 	]*vpsubsw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 e9 b2 f0 f7 ff ff[ 	]*vpsubsw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 e9 f4[ 	]*vpsubsw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 e9 f4[ 	]*vpsubsw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 e9 f4[ 	]*vpsubsw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e9 31[ 	]*vpsubsw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 e9 b4 f0 34 12 00 00[ 	]*vpsubsw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e9 72 7f[ 	]*vpsubsw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e9 b2 00 10 00 00[ 	]*vpsubsw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e9 72 80[ 	]*vpsubsw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 e9 b2 e0 ef ff ff[ 	]*vpsubsw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 d8 f4[ 	]*vpsubusb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 d8 f4[ 	]*vpsubusb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 d8 f4[ 	]*vpsubusb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d8 31[ 	]*vpsubusb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 d8 b4 f0 34 12 00 00[ 	]*vpsubusb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d8 72 7f[ 	]*vpsubusb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d8 b2 00 08 00 00[ 	]*vpsubusb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d8 72 80[ 	]*vpsubusb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d8 b2 f0 f7 ff ff[ 	]*vpsubusb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 d8 f4[ 	]*vpsubusb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 d8 f4[ 	]*vpsubusb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 d8 f4[ 	]*vpsubusb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d8 31[ 	]*vpsubusb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 d8 b4 f0 34 12 00 00[ 	]*vpsubusb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d8 72 7f[ 	]*vpsubusb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d8 b2 00 10 00 00[ 	]*vpsubusb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d8 72 80[ 	]*vpsubusb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d8 b2 e0 ef ff ff[ 	]*vpsubusb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 d9 f4[ 	]*vpsubusw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 d9 f4[ 	]*vpsubusw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 d9 f4[ 	]*vpsubusw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d9 31[ 	]*vpsubusw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 d9 b4 f0 34 12 00 00[ 	]*vpsubusw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d9 72 7f[ 	]*vpsubusw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d9 b2 00 08 00 00[ 	]*vpsubusw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d9 72 80[ 	]*vpsubusw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 d9 b2 f0 f7 ff ff[ 	]*vpsubusw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 d9 f4[ 	]*vpsubusw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 d9 f4[ 	]*vpsubusw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 d9 f4[ 	]*vpsubusw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d9 31[ 	]*vpsubusw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 d9 b4 f0 34 12 00 00[ 	]*vpsubusw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d9 72 7f[ 	]*vpsubusw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d9 b2 00 10 00 00[ 	]*vpsubusw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d9 72 80[ 	]*vpsubusw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 d9 b2 e0 ef ff ff[ 	]*vpsubusw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 f9 f4[ 	]*vpsubw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 f9 f4[ 	]*vpsubw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 f9 f4[ 	]*vpsubw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f9 31[ 	]*vpsubw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 f9 b4 f0 34 12 00 00[ 	]*vpsubw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f9 72 7f[ 	]*vpsubw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f9 b2 00 08 00 00[ 	]*vpsubw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f9 72 80[ 	]*vpsubw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 f9 b2 f0 f7 ff ff[ 	]*vpsubw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 f9 f4[ 	]*vpsubw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 f9 f4[ 	]*vpsubw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 f9 f4[ 	]*vpsubw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f9 31[ 	]*vpsubw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 f9 b4 f0 34 12 00 00[ 	]*vpsubw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f9 72 7f[ 	]*vpsubw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f9 b2 00 10 00 00[ 	]*vpsubw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f9 72 80[ 	]*vpsubw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 f9 b2 e0 ef ff ff[ 	]*vpsubw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 68 f4[ 	]*vpunpckhbw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 68 f4[ 	]*vpunpckhbw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 68 f4[ 	]*vpunpckhbw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 68 31[ 	]*vpunpckhbw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 68 b4 f0 34 12 00 00[ 	]*vpunpckhbw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 68 72 7f[ 	]*vpunpckhbw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 68 b2 00 08 00 00[ 	]*vpunpckhbw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 68 72 80[ 	]*vpunpckhbw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 68 b2 f0 f7 ff ff[ 	]*vpunpckhbw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 68 f4[ 	]*vpunpckhbw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 68 f4[ 	]*vpunpckhbw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 68 f4[ 	]*vpunpckhbw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 68 31[ 	]*vpunpckhbw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 68 b4 f0 34 12 00 00[ 	]*vpunpckhbw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 68 72 7f[ 	]*vpunpckhbw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 68 b2 00 10 00 00[ 	]*vpunpckhbw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 68 72 80[ 	]*vpunpckhbw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 68 b2 e0 ef ff ff[ 	]*vpunpckhbw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 69 f4[ 	]*vpunpckhwd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 69 f4[ 	]*vpunpckhwd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 69 f4[ 	]*vpunpckhwd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 69 31[ 	]*vpunpckhwd \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 69 b4 f0 34 12 00 00[ 	]*vpunpckhwd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 69 72 7f[ 	]*vpunpckhwd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 69 b2 00 08 00 00[ 	]*vpunpckhwd 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 69 72 80[ 	]*vpunpckhwd -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 69 b2 f0 f7 ff ff[ 	]*vpunpckhwd -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 69 f4[ 	]*vpunpckhwd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 69 f4[ 	]*vpunpckhwd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 69 f4[ 	]*vpunpckhwd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 69 31[ 	]*vpunpckhwd \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 69 b4 f0 34 12 00 00[ 	]*vpunpckhwd 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 69 72 7f[ 	]*vpunpckhwd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 69 b2 00 10 00 00[ 	]*vpunpckhwd 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 69 72 80[ 	]*vpunpckhwd -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 69 b2 e0 ef ff ff[ 	]*vpunpckhwd -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 60 f4[ 	]*vpunpcklbw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 60 f4[ 	]*vpunpcklbw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 60 f4[ 	]*vpunpcklbw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 60 31[ 	]*vpunpcklbw \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 60 b4 f0 34 12 00 00[ 	]*vpunpcklbw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 60 72 7f[ 	]*vpunpcklbw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 60 b2 00 08 00 00[ 	]*vpunpcklbw 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 60 72 80[ 	]*vpunpcklbw -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 60 b2 f0 f7 ff ff[ 	]*vpunpcklbw -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 60 f4[ 	]*vpunpcklbw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 60 f4[ 	]*vpunpcklbw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 60 f4[ 	]*vpunpcklbw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 60 31[ 	]*vpunpcklbw \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 60 b4 f0 34 12 00 00[ 	]*vpunpcklbw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 60 72 7f[ 	]*vpunpcklbw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 60 b2 00 10 00 00[ 	]*vpunpcklbw 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 60 72 80[ 	]*vpunpcklbw -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 60 b2 e0 ef ff ff[ 	]*vpunpcklbw -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 61 f4[ 	]*vpunpcklwd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 61 f4[ 	]*vpunpcklwd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 61 f4[ 	]*vpunpcklwd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 61 31[ 	]*vpunpcklwd \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 61 b4 f0 34 12 00 00[ 	]*vpunpcklwd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 61 72 7f[ 	]*vpunpcklwd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 61 b2 00 08 00 00[ 	]*vpunpcklwd 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 61 72 80[ 	]*vpunpcklwd -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 61 b2 f0 f7 ff ff[ 	]*vpunpcklwd -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 61 f4[ 	]*vpunpcklwd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 61 f4[ 	]*vpunpcklwd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 61 f4[ 	]*vpunpcklwd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 61 31[ 	]*vpunpcklwd \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 61 b4 f0 34 12 00 00[ 	]*vpunpcklwd 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 61 72 7f[ 	]*vpunpcklwd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 61 b2 00 10 00 00[ 	]*vpunpcklwd 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 61 72 80[ 	]*vpunpcklwd -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 61 b2 e0 ef ff ff[ 	]*vpunpcklwd -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 73 fd ab[ 	]*vpslldq \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 73 fd 7b[ 	]*vpslldq \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 39 7b[ 	]*vpslldq \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 73 bc f0 34 12 00 00 7b[ 	]*vpslldq \$0x7b,0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 7a 7f 7b[ 	]*vpslldq \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 ba 00 08 00 00 7b[ 	]*vpslldq \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 7a 80 7b[ 	]*vpslldq \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 73 ba f0 f7 ff ff 7b[ 	]*vpslldq \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 73 fd ab[ 	]*vpslldq \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 73 fd 7b[ 	]*vpslldq \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 39 7b[ 	]*vpslldq \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 73 bc f0 34 12 00 00 7b[ 	]*vpslldq \$0x7b,0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 7a 7f 7b[ 	]*vpslldq \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 ba 00 10 00 00 7b[ 	]*vpslldq \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 7a 80 7b[ 	]*vpslldq \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 73 ba e0 ef ff ff 7b[ 	]*vpslldq \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 71 f5 ab[ 	]*vpsllw \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 07 71 f5 ab[ 	]*vpsllw \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 87 71 f5 ab[ 	]*vpsllw \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 00 71 f5 7b[ 	]*vpsllw \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 31 7b[ 	]*vpsllw \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 00 71 b4 f0 34 12 00 00 7b[ 	]*vpsllw \$0x7b,0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 72 7f 7b[ 	]*vpsllw \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 b2 00 08 00 00 7b[ 	]*vpsllw \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 72 80 7b[ 	]*vpsllw \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 00 71 b2 f0 f7 ff ff 7b[ 	]*vpsllw \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 71 f5 ab[ 	]*vpsllw \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 27 71 f5 ab[ 	]*vpsllw \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d a7 71 f5 ab[ 	]*vpsllw \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 20 71 f5 7b[ 	]*vpsllw \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 31 7b[ 	]*vpsllw \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 20 71 b4 f0 34 12 00 00 7b[ 	]*vpsllw \$0x7b,0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 72 7f 7b[ 	]*vpsllw \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 b2 00 10 00 00 7b[ 	]*vpsllw \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 72 80 7b[ 	]*vpsllw \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 20 71 b2 e0 ef ff ff 7b[ 	]*vpsllw \$0x7b,-0x1020\(%rdx\),%ymm30
#pass
