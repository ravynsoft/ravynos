#as: -mevexwig=1
#objdump: -dw
#name: x86_64 AVX512BW wig insns
#source: x86-64-avx512bw-wig.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 1c f5[ 	]*vpabsb %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 1c f5[ 	]*vpabsb %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 1c f5[ 	]*vpabsb %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c 31[ 	]*vpabsb \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 1c b4 f0 23 01 00 00[ 	]*vpabsb 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c 72 7f[ 	]*vpabsb 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c b2 00 20 00 00[ 	]*vpabsb 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c 72 80[ 	]*vpabsb -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c b2 c0 df ff ff[ 	]*vpabsb -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 1d f5[ 	]*vpabsw %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 1d f5[ 	]*vpabsw %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 1d f5[ 	]*vpabsw %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d 31[ 	]*vpabsw \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 1d b4 f0 23 01 00 00[ 	]*vpabsw 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d 72 7f[ 	]*vpabsw 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d b2 00 20 00 00[ 	]*vpabsw 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d 72 80[ 	]*vpabsw -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d b2 c0 df ff ff[ 	]*vpabsw -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 63 f4[ 	]*vpacksswb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 63 f4[ 	]*vpacksswb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 63 f4[ 	]*vpacksswb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 31[ 	]*vpacksswb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 63 b4 f0 23 01 00 00[ 	]*vpacksswb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 72 7f[ 	]*vpacksswb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 b2 00 20 00 00[ 	]*vpacksswb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 72 80[ 	]*vpacksswb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 b2 c0 df ff ff[ 	]*vpacksswb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 67 f4[ 	]*vpackuswb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 67 f4[ 	]*vpackuswb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 67 f4[ 	]*vpackuswb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 31[ 	]*vpackuswb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 67 b4 f0 23 01 00 00[ 	]*vpackuswb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 72 7f[ 	]*vpackuswb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 b2 00 20 00 00[ 	]*vpackuswb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 72 80[ 	]*vpackuswb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 b2 c0 df ff ff[ 	]*vpackuswb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 fc f4[ 	]*vpaddb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 fc f4[ 	]*vpaddb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 fc f4[ 	]*vpaddb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc 31[ 	]*vpaddb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 fc b4 f0 23 01 00 00[ 	]*vpaddb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc 72 7f[ 	]*vpaddb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc b2 00 20 00 00[ 	]*vpaddb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc 72 80[ 	]*vpaddb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc b2 c0 df ff ff[ 	]*vpaddb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ec f4[ 	]*vpaddsb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ec f4[ 	]*vpaddsb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ec f4[ 	]*vpaddsb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec 31[ 	]*vpaddsb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ec b4 f0 23 01 00 00[ 	]*vpaddsb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec 72 7f[ 	]*vpaddsb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec b2 00 20 00 00[ 	]*vpaddsb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec 72 80[ 	]*vpaddsb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec b2 c0 df ff ff[ 	]*vpaddsb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ed f4[ 	]*vpaddsw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ed f4[ 	]*vpaddsw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ed f4[ 	]*vpaddsw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed 31[ 	]*vpaddsw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ed b4 f0 23 01 00 00[ 	]*vpaddsw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed 72 7f[ 	]*vpaddsw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed b2 00 20 00 00[ 	]*vpaddsw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed 72 80[ 	]*vpaddsw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed b2 c0 df ff ff[ 	]*vpaddsw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 dc f4[ 	]*vpaddusb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 dc f4[ 	]*vpaddusb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 dc f4[ 	]*vpaddusb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc 31[ 	]*vpaddusb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 dc b4 f0 23 01 00 00[ 	]*vpaddusb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc 72 7f[ 	]*vpaddusb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc b2 00 20 00 00[ 	]*vpaddusb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc 72 80[ 	]*vpaddusb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc b2 c0 df ff ff[ 	]*vpaddusb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 dd f4[ 	]*vpaddusw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 dd f4[ 	]*vpaddusw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 dd f4[ 	]*vpaddusw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd 31[ 	]*vpaddusw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 dd b4 f0 23 01 00 00[ 	]*vpaddusw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd 72 7f[ 	]*vpaddusw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd b2 00 20 00 00[ 	]*vpaddusw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd 72 80[ 	]*vpaddusw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd b2 c0 df ff ff[ 	]*vpaddusw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 fd f4[ 	]*vpaddw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 fd f4[ 	]*vpaddw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 fd f4[ 	]*vpaddw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd 31[ 	]*vpaddw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 fd b4 f0 23 01 00 00[ 	]*vpaddw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd 72 7f[ 	]*vpaddw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd b2 00 20 00 00[ 	]*vpaddw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd 72 80[ 	]*vpaddw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd b2 c0 df ff ff[ 	]*vpaddw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 0f f4 ab[ 	]*vpalignr \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 0f f4 ab[ 	]*vpalignr \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 0f f4 ab[ 	]*vpalignr \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 0f f4 7b[ 	]*vpalignr \$0x7b,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f 31 7b[ 	]*vpalignr \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 0f b4 f0 23 01 00 00 7b[ 	]*vpalignr \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f 72 7f 7b[ 	]*vpalignr \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f b2 00 20 00 00 7b[ 	]*vpalignr \$0x7b,0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f 72 80 7b[ 	]*vpalignr \$0x7b,-0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f b2 c0 df ff ff 7b[ 	]*vpalignr \$0x7b,-0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e0 f4[ 	]*vpavgb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e0 f4[ 	]*vpavgb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e0 f4[ 	]*vpavgb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 31[ 	]*vpavgb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e0 b4 f0 23 01 00 00[ 	]*vpavgb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 72 7f[ 	]*vpavgb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 b2 00 20 00 00[ 	]*vpavgb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 72 80[ 	]*vpavgb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 b2 c0 df ff ff[ 	]*vpavgb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e3 f4[ 	]*vpavgw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e3 f4[ 	]*vpavgw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e3 f4[ 	]*vpavgw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 31[ 	]*vpavgw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e3 b4 f0 23 01 00 00[ 	]*vpavgw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 72 7f[ 	]*vpavgw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 b2 00 20 00 00[ 	]*vpavgw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 72 80[ 	]*vpavgw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 b2 c0 df ff ff[ 	]*vpavgw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 74 ed[ 	]*vpcmpeqb %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 74 ed[ 	]*vpcmpeqb %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 29[ 	]*vpcmpeqb \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 74 ac f0 23 01 00 00[ 	]*vpcmpeqb 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 6a 7f[ 	]*vpcmpeqb 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 aa 00 20 00 00[ 	]*vpcmpeqb 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 6a 80[ 	]*vpcmpeqb -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 aa c0 df ff ff[ 	]*vpcmpeqb -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 75 ed[ 	]*vpcmpeqw %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 75 ed[ 	]*vpcmpeqw %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 29[ 	]*vpcmpeqw \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 75 ac f0 23 01 00 00[ 	]*vpcmpeqw 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 6a 7f[ 	]*vpcmpeqw 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 aa 00 20 00 00[ 	]*vpcmpeqw 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 6a 80[ 	]*vpcmpeqw -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 aa c0 df ff ff[ 	]*vpcmpeqw -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 64 ed[ 	]*vpcmpgtb %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 64 ed[ 	]*vpcmpgtb %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 29[ 	]*vpcmpgtb \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 64 ac f0 23 01 00 00[ 	]*vpcmpgtb 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 6a 7f[ 	]*vpcmpgtb 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 aa 00 20 00 00[ 	]*vpcmpgtb 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 6a 80[ 	]*vpcmpgtb -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 aa c0 df ff ff[ 	]*vpcmpgtb -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 65 ed[ 	]*vpcmpgtw %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 65 ed[ 	]*vpcmpgtw %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 29[ 	]*vpcmpgtw \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 65 ac f0 23 01 00 00[ 	]*vpcmpgtw 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 6a 7f[ 	]*vpcmpgtw 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 aa 00 20 00 00[ 	]*vpcmpgtw 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 6a 80[ 	]*vpcmpgtw -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 aa c0 df ff ff[ 	]*vpcmpgtw -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 e8 ab[ 	]*vpextrb \$0xab,%xmm29,%eax
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 e8 7b[ 	]*vpextrb \$0x7b,%xmm29,%eax
[ 	]*[a-f0-9]+:[ 	]*62 43 fd 08 14 e8 7b[ 	]*vpextrb \$0x7b,%xmm29,%r8d
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 29 7b[ 	]*vpextrb \$0x7b,%xmm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 14 ac f0 23 01 00 00 7b[ 	]*vpextrb \$0x7b,%xmm29,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 6a 7f 7b[ 	]*vpextrb \$0x7b,%xmm29,0x7f\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 aa 80 00 00 00 7b[ 	]*vpextrb \$0x7b,%xmm29,0x80\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 6a 80 7b[ 	]*vpextrb \$0x7b,%xmm29,-0x80\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 aa 7f ff ff ff 7b[ 	]*vpextrb \$0x7b,%xmm29,-0x81\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 29 7b[ 	]*vpextrw \$0x7b,%xmm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 15 ac f0 23 01 00 00 7b[ 	]*vpextrw \$0x7b,%xmm29,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 6a 7f 7b[ 	]*vpextrw \$0x7b,%xmm29,0xfe\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 aa 00 01 00 00 7b[ 	]*vpextrw \$0x7b,%xmm29,0x100\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 6a 80 7b[ 	]*vpextrw \$0x7b,%xmm29,-0x100\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 aa fe fe ff ff 7b[ 	]*vpextrw \$0x7b,%xmm29,-0x102\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 91 fd 08 c5 c6 ab[ 	]*vpextrw \$0xab,%xmm30,%eax
[ 	]*[a-f0-9]+:[ 	]*62 91 fd 08 c5 c6 7b[ 	]*vpextrw \$0x7b,%xmm30,%eax
[ 	]*[a-f0-9]+:[ 	]*62 11 fd 08 c5 c6 7b[ 	]*vpextrw \$0x7b,%xmm30,%r8d
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 f0 ab[ 	]*vpinsrb \$0xab,%eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 f0 7b[ 	]*vpinsrb \$0x7b,%eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 f5 7b[ 	]*vpinsrb \$0x7b,%ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 43 95 00 20 f5 7b[ 	]*vpinsrb \$0x7b,%r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 31 7b[ 	]*vpinsrb \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 20 b4 f0 23 01 00 00 7b[ 	]*vpinsrb \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 72 7f 7b[ 	]*vpinsrb \$0x7b,0x7f\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 b2 80 00 00 00 7b[ 	]*vpinsrb \$0x7b,0x80\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 72 80 7b[ 	]*vpinsrb \$0x7b,-0x80\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 b2 7f ff ff ff 7b[ 	]*vpinsrb \$0x7b,-0x81\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 f0 ab[ 	]*vpinsrw \$0xab,%eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 f0 7b[ 	]*vpinsrw \$0x7b,%eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 f5 7b[ 	]*vpinsrw \$0x7b,%ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 41 95 00 c4 f5 7b[ 	]*vpinsrw \$0x7b,%r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 31 7b[ 	]*vpinsrw \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 c4 b4 f0 23 01 00 00 7b[ 	]*vpinsrw \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 72 7f 7b[ 	]*vpinsrw \$0x7b,0xfe\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 b2 00 01 00 00 7b[ 	]*vpinsrw \$0x7b,0x100\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 72 80 7b[ 	]*vpinsrw \$0x7b,-0x100\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 b2 fe fe ff ff 7b[ 	]*vpinsrw \$0x7b,-0x102\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 04 f4[ 	]*vpmaddubsw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 04 f4[ 	]*vpmaddubsw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 04 f4[ 	]*vpmaddubsw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 31[ 	]*vpmaddubsw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 04 b4 f0 23 01 00 00[ 	]*vpmaddubsw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 72 7f[ 	]*vpmaddubsw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 b2 00 20 00 00[ 	]*vpmaddubsw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 72 80[ 	]*vpmaddubsw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 b2 c0 df ff ff[ 	]*vpmaddubsw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f5 f4[ 	]*vpmaddwd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f5 f4[ 	]*vpmaddwd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f5 f4[ 	]*vpmaddwd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 31[ 	]*vpmaddwd \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f5 b4 f0 23 01 00 00[ 	]*vpmaddwd 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 72 7f[ 	]*vpmaddwd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 b2 00 20 00 00[ 	]*vpmaddwd 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 72 80[ 	]*vpmaddwd -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 b2 c0 df ff ff[ 	]*vpmaddwd -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 3c f4[ 	]*vpmaxsb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 3c f4[ 	]*vpmaxsb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 3c f4[ 	]*vpmaxsb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c 31[ 	]*vpmaxsb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 3c b4 f0 23 01 00 00[ 	]*vpmaxsb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c 72 7f[ 	]*vpmaxsb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c b2 00 20 00 00[ 	]*vpmaxsb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c 72 80[ 	]*vpmaxsb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c b2 c0 df ff ff[ 	]*vpmaxsb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ee f4[ 	]*vpmaxsw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ee f4[ 	]*vpmaxsw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ee f4[ 	]*vpmaxsw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee 31[ 	]*vpmaxsw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ee b4 f0 23 01 00 00[ 	]*vpmaxsw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee 72 7f[ 	]*vpmaxsw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee b2 00 20 00 00[ 	]*vpmaxsw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee 72 80[ 	]*vpmaxsw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee b2 c0 df ff ff[ 	]*vpmaxsw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 de f4[ 	]*vpmaxub %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 de f4[ 	]*vpmaxub %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 de f4[ 	]*vpmaxub %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de 31[ 	]*vpmaxub \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 de b4 f0 23 01 00 00[ 	]*vpmaxub 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de 72 7f[ 	]*vpmaxub 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de b2 00 20 00 00[ 	]*vpmaxub 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de 72 80[ 	]*vpmaxub -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de b2 c0 df ff ff[ 	]*vpmaxub -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 3e f4[ 	]*vpmaxuw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 3e f4[ 	]*vpmaxuw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 3e f4[ 	]*vpmaxuw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e 31[ 	]*vpmaxuw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 3e b4 f0 23 01 00 00[ 	]*vpmaxuw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e 72 7f[ 	]*vpmaxuw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e b2 00 20 00 00[ 	]*vpmaxuw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e 72 80[ 	]*vpmaxuw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e b2 c0 df ff ff[ 	]*vpmaxuw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 38 f4[ 	]*vpminsb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 38 f4[ 	]*vpminsb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 38 f4[ 	]*vpminsb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 31[ 	]*vpminsb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 38 b4 f0 23 01 00 00[ 	]*vpminsb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 72 7f[ 	]*vpminsb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 b2 00 20 00 00[ 	]*vpminsb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 72 80[ 	]*vpminsb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 b2 c0 df ff ff[ 	]*vpminsb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ea f4[ 	]*vpminsw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ea f4[ 	]*vpminsw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ea f4[ 	]*vpminsw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea 31[ 	]*vpminsw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ea b4 f0 23 01 00 00[ 	]*vpminsw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea 72 7f[ 	]*vpminsw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea b2 00 20 00 00[ 	]*vpminsw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea 72 80[ 	]*vpminsw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea b2 c0 df ff ff[ 	]*vpminsw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 da f4[ 	]*vpminub %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 da f4[ 	]*vpminub %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 da f4[ 	]*vpminub %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da 31[ 	]*vpminub \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 da b4 f0 23 01 00 00[ 	]*vpminub 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da 72 7f[ 	]*vpminub 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da b2 00 20 00 00[ 	]*vpminub 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da 72 80[ 	]*vpminub -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da b2 c0 df ff ff[ 	]*vpminub -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 3a f4[ 	]*vpminuw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 3a f4[ 	]*vpminuw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 3a f4[ 	]*vpminuw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a 31[ 	]*vpminuw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 3a b4 f0 23 01 00 00[ 	]*vpminuw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a 72 7f[ 	]*vpminuw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a b2 00 20 00 00[ 	]*vpminuw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a 72 80[ 	]*vpminuw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a b2 c0 df ff ff[ 	]*vpminuw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 20 f5[ 	]*vpmovsxbw %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 20 f5[ 	]*vpmovsxbw %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 20 f5[ 	]*vpmovsxbw %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 31[ 	]*vpmovsxbw \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 20 b4 f0 23 01 00 00[ 	]*vpmovsxbw 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 72 7f[ 	]*vpmovsxbw 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 b2 00 10 00 00[ 	]*vpmovsxbw 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 72 80[ 	]*vpmovsxbw -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 b2 e0 ef ff ff[ 	]*vpmovsxbw -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 30 f5[ 	]*vpmovzxbw %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 30 f5[ 	]*vpmovzxbw %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 30 f5[ 	]*vpmovzxbw %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 31[ 	]*vpmovzxbw \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 30 b4 f0 23 01 00 00[ 	]*vpmovzxbw 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 72 7f[ 	]*vpmovzxbw 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 b2 00 10 00 00[ 	]*vpmovzxbw 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 72 80[ 	]*vpmovzxbw -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 b2 e0 ef ff ff[ 	]*vpmovzxbw -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 0b f4[ 	]*vpmulhrsw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 0b f4[ 	]*vpmulhrsw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 0b f4[ 	]*vpmulhrsw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b 31[ 	]*vpmulhrsw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 0b b4 f0 23 01 00 00[ 	]*vpmulhrsw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b 72 7f[ 	]*vpmulhrsw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b b2 00 20 00 00[ 	]*vpmulhrsw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b 72 80[ 	]*vpmulhrsw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b b2 c0 df ff ff[ 	]*vpmulhrsw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e4 f4[ 	]*vpmulhuw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e4 f4[ 	]*vpmulhuw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e4 f4[ 	]*vpmulhuw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 31[ 	]*vpmulhuw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e4 b4 f0 23 01 00 00[ 	]*vpmulhuw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 72 7f[ 	]*vpmulhuw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 b2 00 20 00 00[ 	]*vpmulhuw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 72 80[ 	]*vpmulhuw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 b2 c0 df ff ff[ 	]*vpmulhuw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e5 f4[ 	]*vpmulhw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e5 f4[ 	]*vpmulhw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e5 f4[ 	]*vpmulhw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 31[ 	]*vpmulhw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e5 b4 f0 23 01 00 00[ 	]*vpmulhw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 72 7f[ 	]*vpmulhw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 b2 00 20 00 00[ 	]*vpmulhw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 72 80[ 	]*vpmulhw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 b2 c0 df ff ff[ 	]*vpmulhw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d5 f4[ 	]*vpmullw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d5 f4[ 	]*vpmullw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d5 f4[ 	]*vpmullw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 31[ 	]*vpmullw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d5 b4 f0 23 01 00 00[ 	]*vpmullw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 72 7f[ 	]*vpmullw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 b2 00 20 00 00[ 	]*vpmullw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 72 80[ 	]*vpmullw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 b2 c0 df ff ff[ 	]*vpmullw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f6 f4[ 	]*vpsadbw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 31[ 	]*vpsadbw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f6 b4 f0 23 01 00 00[ 	]*vpsadbw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 72 7f[ 	]*vpsadbw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 b2 00 20 00 00[ 	]*vpsadbw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 72 80[ 	]*vpsadbw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 b2 c0 df ff ff[ 	]*vpsadbw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 00 f4[ 	]*vpshufb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 00 f4[ 	]*vpshufb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 00 f4[ 	]*vpshufb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 31[ 	]*vpshufb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 00 b4 f0 23 01 00 00[ 	]*vpshufb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 72 7f[ 	]*vpshufb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 b2 00 20 00 00[ 	]*vpshufb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 72 80[ 	]*vpshufb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 b2 c0 df ff ff[ 	]*vpshufb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 70 f5 ab[ 	]*vpshufhw \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 4f 70 f5 ab[ 	]*vpshufhw \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe cf 70 f5 ab[ 	]*vpshufhw \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 70 f5 7b[ 	]*vpshufhw \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 31 7b[ 	]*vpshufhw \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 48 70 b4 f0 23 01 00 00 7b[ 	]*vpshufhw \$0x7b,0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 72 7f 7b[ 	]*vpshufhw \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 b2 00 20 00 00 7b[ 	]*vpshufhw \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 72 80 7b[ 	]*vpshufhw \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 b2 c0 df ff ff 7b[ 	]*vpshufhw \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 70 f5 ab[ 	]*vpshuflw \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 4f 70 f5 ab[ 	]*vpshuflw \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff cf 70 f5 ab[ 	]*vpshuflw \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 70 f5 7b[ 	]*vpshuflw \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 31 7b[ 	]*vpshuflw \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 48 70 b4 f0 23 01 00 00 7b[ 	]*vpshuflw \$0x7b,0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 72 7f 7b[ 	]*vpshuflw \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 b2 00 20 00 00 7b[ 	]*vpshuflw \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 72 80 7b[ 	]*vpshuflw \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 b2 c0 df ff ff 7b[ 	]*vpshuflw \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f1 f4[ 	]*vpsllw %xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f1 f4[ 	]*vpsllw %xmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f1 f4[ 	]*vpsllw %xmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 31[ 	]*vpsllw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f1 b4 f0 23 01 00 00[ 	]*vpsllw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 72 7f[ 	]*vpsllw 0x7f0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 b2 00 08 00 00[ 	]*vpsllw 0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 72 80[ 	]*vpsllw -0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 b2 f0 f7 ff ff[ 	]*vpsllw -0x810\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e1 f4[ 	]*vpsraw %xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e1 f4[ 	]*vpsraw %xmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e1 f4[ 	]*vpsraw %xmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 31[ 	]*vpsraw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e1 b4 f0 23 01 00 00[ 	]*vpsraw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 72 7f[ 	]*vpsraw 0x7f0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 b2 00 08 00 00[ 	]*vpsraw 0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 72 80[ 	]*vpsraw -0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 b2 f0 f7 ff ff[ 	]*vpsraw -0x810\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d1 f4[ 	]*vpsrlw %xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d1 f4[ 	]*vpsrlw %xmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d1 f4[ 	]*vpsrlw %xmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 31[ 	]*vpsrlw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d1 b4 f0 23 01 00 00[ 	]*vpsrlw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 72 7f[ 	]*vpsrlw 0x7f0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 b2 00 08 00 00[ 	]*vpsrlw 0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 72 80[ 	]*vpsrlw -0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 b2 f0 f7 ff ff[ 	]*vpsrlw -0x810\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 dd ab[ 	]*vpsrldq \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 dd 7b[ 	]*vpsrldq \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 19 7b[ 	]*vpsrldq \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 73 9c f0 23 01 00 00 7b[ 	]*vpsrldq \$0x7b,0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 5a 7f 7b[ 	]*vpsrldq \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 9a 00 20 00 00 7b[ 	]*vpsrldq \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 5a 80 7b[ 	]*vpsrldq \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 9a c0 df ff ff 7b[ 	]*vpsrldq \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 d5 ab[ 	]*vpsrlw \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 71 d5 ab[ 	]*vpsrlw \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d c7 71 d5 ab[ 	]*vpsrlw \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 d5 7b[ 	]*vpsrlw \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 11 7b[ 	]*vpsrlw \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 71 94 f0 23 01 00 00 7b[ 	]*vpsrlw \$0x7b,0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 52 7f 7b[ 	]*vpsrlw \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 92 00 20 00 00 7b[ 	]*vpsrlw \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 52 80 7b[ 	]*vpsrlw \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 92 c0 df ff ff 7b[ 	]*vpsrlw \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 e5 ab[ 	]*vpsraw \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 71 e5 ab[ 	]*vpsraw \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d c7 71 e5 ab[ 	]*vpsraw \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 e5 7b[ 	]*vpsraw \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 21 7b[ 	]*vpsraw \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 71 a4 f0 23 01 00 00 7b[ 	]*vpsraw \$0x7b,0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 62 7f 7b[ 	]*vpsraw \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 a2 00 20 00 00 7b[ 	]*vpsraw \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 62 80 7b[ 	]*vpsraw \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 a2 c0 df ff ff 7b[ 	]*vpsraw \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f8 f4[ 	]*vpsubb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f8 f4[ 	]*vpsubb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f8 f4[ 	]*vpsubb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 31[ 	]*vpsubb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f8 b4 f0 23 01 00 00[ 	]*vpsubb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 72 7f[ 	]*vpsubb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 b2 00 20 00 00[ 	]*vpsubb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 72 80[ 	]*vpsubb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 b2 c0 df ff ff[ 	]*vpsubb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e8 f4[ 	]*vpsubsb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e8 f4[ 	]*vpsubsb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e8 f4[ 	]*vpsubsb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 31[ 	]*vpsubsb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e8 b4 f0 23 01 00 00[ 	]*vpsubsb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 72 7f[ 	]*vpsubsb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 b2 00 20 00 00[ 	]*vpsubsb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 72 80[ 	]*vpsubsb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 b2 c0 df ff ff[ 	]*vpsubsb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e9 f4[ 	]*vpsubsw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e9 f4[ 	]*vpsubsw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e9 f4[ 	]*vpsubsw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 31[ 	]*vpsubsw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e9 b4 f0 23 01 00 00[ 	]*vpsubsw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 72 7f[ 	]*vpsubsw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 b2 00 20 00 00[ 	]*vpsubsw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 72 80[ 	]*vpsubsw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 b2 c0 df ff ff[ 	]*vpsubsw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d8 f4[ 	]*vpsubusb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d8 f4[ 	]*vpsubusb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d8 f4[ 	]*vpsubusb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 31[ 	]*vpsubusb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d8 b4 f0 23 01 00 00[ 	]*vpsubusb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 72 7f[ 	]*vpsubusb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 b2 00 20 00 00[ 	]*vpsubusb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 72 80[ 	]*vpsubusb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 b2 c0 df ff ff[ 	]*vpsubusb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d9 f4[ 	]*vpsubusw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d9 f4[ 	]*vpsubusw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d9 f4[ 	]*vpsubusw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 31[ 	]*vpsubusw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d9 b4 f0 23 01 00 00[ 	]*vpsubusw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 72 7f[ 	]*vpsubusw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 b2 00 20 00 00[ 	]*vpsubusw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 72 80[ 	]*vpsubusw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 b2 c0 df ff ff[ 	]*vpsubusw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f9 f4[ 	]*vpsubw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f9 f4[ 	]*vpsubw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f9 f4[ 	]*vpsubw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 31[ 	]*vpsubw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f9 b4 f0 23 01 00 00[ 	]*vpsubw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 72 7f[ 	]*vpsubw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 b2 00 20 00 00[ 	]*vpsubw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 72 80[ 	]*vpsubw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 b2 c0 df ff ff[ 	]*vpsubw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 68 f4[ 	]*vpunpckhbw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 68 f4[ 	]*vpunpckhbw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 68 f4[ 	]*vpunpckhbw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 31[ 	]*vpunpckhbw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 68 b4 f0 23 01 00 00[ 	]*vpunpckhbw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 72 7f[ 	]*vpunpckhbw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 b2 00 20 00 00[ 	]*vpunpckhbw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 72 80[ 	]*vpunpckhbw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 b2 c0 df ff ff[ 	]*vpunpckhbw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 69 f4[ 	]*vpunpckhwd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 69 f4[ 	]*vpunpckhwd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 69 f4[ 	]*vpunpckhwd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 31[ 	]*vpunpckhwd \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 69 b4 f0 23 01 00 00[ 	]*vpunpckhwd 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 72 7f[ 	]*vpunpckhwd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 b2 00 20 00 00[ 	]*vpunpckhwd 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 72 80[ 	]*vpunpckhwd -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 b2 c0 df ff ff[ 	]*vpunpckhwd -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 60 f4[ 	]*vpunpcklbw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 60 f4[ 	]*vpunpcklbw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 60 f4[ 	]*vpunpcklbw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 31[ 	]*vpunpcklbw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 60 b4 f0 23 01 00 00[ 	]*vpunpcklbw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 72 7f[ 	]*vpunpcklbw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 b2 00 20 00 00[ 	]*vpunpcklbw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 72 80[ 	]*vpunpcklbw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 b2 c0 df ff ff[ 	]*vpunpcklbw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 61 f4[ 	]*vpunpcklwd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 61 f4[ 	]*vpunpcklwd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 61 f4[ 	]*vpunpcklwd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 31[ 	]*vpunpcklwd \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 61 b4 f0 23 01 00 00[ 	]*vpunpcklwd 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 72 7f[ 	]*vpunpcklwd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 b2 00 20 00 00[ 	]*vpunpcklwd 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 72 80[ 	]*vpunpcklwd -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 b2 c0 df ff ff[ 	]*vpunpcklwd -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 fd ab[ 	]*vpslldq \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 fd 7b[ 	]*vpslldq \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 39 7b[ 	]*vpslldq \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 73 bc f0 23 01 00 00 7b[ 	]*vpslldq \$0x7b,0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 7a 7f 7b[ 	]*vpslldq \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 ba 00 20 00 00 7b[ 	]*vpslldq \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 7a 80 7b[ 	]*vpslldq \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 ba c0 df ff ff 7b[ 	]*vpslldq \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 f5 ab[ 	]*vpsllw \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 71 f5 ab[ 	]*vpsllw \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d c7 71 f5 ab[ 	]*vpsllw \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 f5 7b[ 	]*vpsllw \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 31 7b[ 	]*vpsllw \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 71 b4 f0 23 01 00 00 7b[ 	]*vpsllw \$0x7b,0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 72 7f 7b[ 	]*vpsllw \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 b2 00 20 00 00 7b[ 	]*vpsllw \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 72 80 7b[ 	]*vpsllw \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 b2 c0 df ff ff 7b[ 	]*vpsllw \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 1c f5[ 	]*vpabsb %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 1c f5[ 	]*vpabsb %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 1c f5[ 	]*vpabsb %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c 31[ 	]*vpabsb \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 1c b4 f0 34 12 00 00[ 	]*vpabsb 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c 72 7f[ 	]*vpabsb 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c b2 00 20 00 00[ 	]*vpabsb 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c 72 80[ 	]*vpabsb -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c b2 c0 df ff ff[ 	]*vpabsb -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 1d f5[ 	]*vpabsw %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 1d f5[ 	]*vpabsw %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 1d f5[ 	]*vpabsw %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d 31[ 	]*vpabsw \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 1d b4 f0 34 12 00 00[ 	]*vpabsw 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d 72 7f[ 	]*vpabsw 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d b2 00 20 00 00[ 	]*vpabsw 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d 72 80[ 	]*vpabsw -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d b2 c0 df ff ff[ 	]*vpabsw -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 63 f4[ 	]*vpacksswb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 63 f4[ 	]*vpacksswb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 63 f4[ 	]*vpacksswb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 31[ 	]*vpacksswb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 63 b4 f0 34 12 00 00[ 	]*vpacksswb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 72 7f[ 	]*vpacksswb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 b2 00 20 00 00[ 	]*vpacksswb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 72 80[ 	]*vpacksswb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 b2 c0 df ff ff[ 	]*vpacksswb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 67 f4[ 	]*vpackuswb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 67 f4[ 	]*vpackuswb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 67 f4[ 	]*vpackuswb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 31[ 	]*vpackuswb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 67 b4 f0 34 12 00 00[ 	]*vpackuswb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 72 7f[ 	]*vpackuswb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 b2 00 20 00 00[ 	]*vpackuswb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 72 80[ 	]*vpackuswb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 b2 c0 df ff ff[ 	]*vpackuswb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 fc f4[ 	]*vpaddb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 fc f4[ 	]*vpaddb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 fc f4[ 	]*vpaddb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc 31[ 	]*vpaddb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 fc b4 f0 34 12 00 00[ 	]*vpaddb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc 72 7f[ 	]*vpaddb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc b2 00 20 00 00[ 	]*vpaddb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc 72 80[ 	]*vpaddb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc b2 c0 df ff ff[ 	]*vpaddb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ec f4[ 	]*vpaddsb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ec f4[ 	]*vpaddsb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ec f4[ 	]*vpaddsb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec 31[ 	]*vpaddsb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ec b4 f0 34 12 00 00[ 	]*vpaddsb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec 72 7f[ 	]*vpaddsb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec b2 00 20 00 00[ 	]*vpaddsb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec 72 80[ 	]*vpaddsb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec b2 c0 df ff ff[ 	]*vpaddsb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ed f4[ 	]*vpaddsw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ed f4[ 	]*vpaddsw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ed f4[ 	]*vpaddsw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed 31[ 	]*vpaddsw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ed b4 f0 34 12 00 00[ 	]*vpaddsw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed 72 7f[ 	]*vpaddsw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed b2 00 20 00 00[ 	]*vpaddsw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed 72 80[ 	]*vpaddsw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed b2 c0 df ff ff[ 	]*vpaddsw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 dc f4[ 	]*vpaddusb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 dc f4[ 	]*vpaddusb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 dc f4[ 	]*vpaddusb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc 31[ 	]*vpaddusb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 dc b4 f0 34 12 00 00[ 	]*vpaddusb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc 72 7f[ 	]*vpaddusb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc b2 00 20 00 00[ 	]*vpaddusb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc 72 80[ 	]*vpaddusb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc b2 c0 df ff ff[ 	]*vpaddusb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 dd f4[ 	]*vpaddusw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 dd f4[ 	]*vpaddusw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 dd f4[ 	]*vpaddusw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd 31[ 	]*vpaddusw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 dd b4 f0 34 12 00 00[ 	]*vpaddusw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd 72 7f[ 	]*vpaddusw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd b2 00 20 00 00[ 	]*vpaddusw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd 72 80[ 	]*vpaddusw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd b2 c0 df ff ff[ 	]*vpaddusw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 fd f4[ 	]*vpaddw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 fd f4[ 	]*vpaddw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 fd f4[ 	]*vpaddw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd 31[ 	]*vpaddw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 fd b4 f0 34 12 00 00[ 	]*vpaddw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd 72 7f[ 	]*vpaddw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd b2 00 20 00 00[ 	]*vpaddw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd 72 80[ 	]*vpaddw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd b2 c0 df ff ff[ 	]*vpaddw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 0f f4 ab[ 	]*vpalignr \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 0f f4 ab[ 	]*vpalignr \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 0f f4 ab[ 	]*vpalignr \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 0f f4 7b[ 	]*vpalignr \$0x7b,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f 31 7b[ 	]*vpalignr \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 0f b4 f0 34 12 00 00 7b[ 	]*vpalignr \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f 72 7f 7b[ 	]*vpalignr \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f b2 00 20 00 00 7b[ 	]*vpalignr \$0x7b,0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f 72 80 7b[ 	]*vpalignr \$0x7b,-0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f b2 c0 df ff ff 7b[ 	]*vpalignr \$0x7b,-0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e0 f4[ 	]*vpavgb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e0 f4[ 	]*vpavgb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e0 f4[ 	]*vpavgb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 31[ 	]*vpavgb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e0 b4 f0 34 12 00 00[ 	]*vpavgb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 72 7f[ 	]*vpavgb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 b2 00 20 00 00[ 	]*vpavgb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 72 80[ 	]*vpavgb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 b2 c0 df ff ff[ 	]*vpavgb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e3 f4[ 	]*vpavgw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e3 f4[ 	]*vpavgw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e3 f4[ 	]*vpavgw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 31[ 	]*vpavgw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e3 b4 f0 34 12 00 00[ 	]*vpavgw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 72 7f[ 	]*vpavgw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 b2 00 20 00 00[ 	]*vpavgw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 72 80[ 	]*vpavgw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 b2 c0 df ff ff[ 	]*vpavgw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 74 ed[ 	]*vpcmpeqb %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 74 ed[ 	]*vpcmpeqb %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 29[ 	]*vpcmpeqb \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 74 ac f0 34 12 00 00[ 	]*vpcmpeqb 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 6a 7f[ 	]*vpcmpeqb 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 aa 00 20 00 00[ 	]*vpcmpeqb 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 6a 80[ 	]*vpcmpeqb -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 aa c0 df ff ff[ 	]*vpcmpeqb -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 75 ed[ 	]*vpcmpeqw %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 75 ed[ 	]*vpcmpeqw %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 29[ 	]*vpcmpeqw \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 75 ac f0 34 12 00 00[ 	]*vpcmpeqw 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 6a 7f[ 	]*vpcmpeqw 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 aa 00 20 00 00[ 	]*vpcmpeqw 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 6a 80[ 	]*vpcmpeqw -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 aa c0 df ff ff[ 	]*vpcmpeqw -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 64 ed[ 	]*vpcmpgtb %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 64 ed[ 	]*vpcmpgtb %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 29[ 	]*vpcmpgtb \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 64 ac f0 34 12 00 00[ 	]*vpcmpgtb 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 6a 7f[ 	]*vpcmpgtb 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 aa 00 20 00 00[ 	]*vpcmpgtb 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 6a 80[ 	]*vpcmpgtb -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 aa c0 df ff ff[ 	]*vpcmpgtb -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 65 ed[ 	]*vpcmpgtw %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 65 ed[ 	]*vpcmpgtw %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 29[ 	]*vpcmpgtw \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 65 ac f0 34 12 00 00[ 	]*vpcmpgtw 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 6a 7f[ 	]*vpcmpgtw 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 aa 00 20 00 00[ 	]*vpcmpgtw 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 6a 80[ 	]*vpcmpgtw -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 aa c0 df ff ff[ 	]*vpcmpgtw -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 e8 ab[ 	]*vpextrb \$0xab,%xmm29,%eax
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 e8 7b[ 	]*vpextrb \$0x7b,%xmm29,%eax
[ 	]*[a-f0-9]+:[ 	]*62 43 fd 08 14 e8 7b[ 	]*vpextrb \$0x7b,%xmm29,%r8d
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 29 7b[ 	]*vpextrb \$0x7b,%xmm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 14 ac f0 34 12 00 00 7b[ 	]*vpextrb \$0x7b,%xmm29,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 6a 7f 7b[ 	]*vpextrb \$0x7b,%xmm29,0x7f\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 aa 80 00 00 00 7b[ 	]*vpextrb \$0x7b,%xmm29,0x80\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 6a 80 7b[ 	]*vpextrb \$0x7b,%xmm29,-0x80\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 aa 7f ff ff ff 7b[ 	]*vpextrb \$0x7b,%xmm29,-0x81\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 29 7b[ 	]*vpextrw \$0x7b,%xmm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 15 ac f0 34 12 00 00 7b[ 	]*vpextrw \$0x7b,%xmm29,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 6a 7f 7b[ 	]*vpextrw \$0x7b,%xmm29,0xfe\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 aa 00 01 00 00 7b[ 	]*vpextrw \$0x7b,%xmm29,0x100\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 6a 80 7b[ 	]*vpextrw \$0x7b,%xmm29,-0x100\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 aa fe fe ff ff 7b[ 	]*vpextrw \$0x7b,%xmm29,-0x102\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 91 fd 08 c5 c6 ab[ 	]*vpextrw \$0xab,%xmm30,%eax
[ 	]*[a-f0-9]+:[ 	]*62 91 fd 08 c5 c6 7b[ 	]*vpextrw \$0x7b,%xmm30,%eax
[ 	]*[a-f0-9]+:[ 	]*62 11 fd 08 c5 c6 7b[ 	]*vpextrw \$0x7b,%xmm30,%r8d
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 f0 ab[ 	]*vpinsrb \$0xab,%eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 f0 7b[ 	]*vpinsrb \$0x7b,%eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 f5 7b[ 	]*vpinsrb \$0x7b,%ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 43 95 00 20 f5 7b[ 	]*vpinsrb \$0x7b,%r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 31 7b[ 	]*vpinsrb \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 20 b4 f0 34 12 00 00 7b[ 	]*vpinsrb \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 72 7f 7b[ 	]*vpinsrb \$0x7b,0x7f\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 b2 80 00 00 00 7b[ 	]*vpinsrb \$0x7b,0x80\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 72 80 7b[ 	]*vpinsrb \$0x7b,-0x80\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 b2 7f ff ff ff 7b[ 	]*vpinsrb \$0x7b,-0x81\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 f0 ab[ 	]*vpinsrw \$0xab,%eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 f0 7b[ 	]*vpinsrw \$0x7b,%eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 f5 7b[ 	]*vpinsrw \$0x7b,%ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 41 95 00 c4 f5 7b[ 	]*vpinsrw \$0x7b,%r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 31 7b[ 	]*vpinsrw \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 c4 b4 f0 34 12 00 00 7b[ 	]*vpinsrw \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 72 7f 7b[ 	]*vpinsrw \$0x7b,0xfe\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 b2 00 01 00 00 7b[ 	]*vpinsrw \$0x7b,0x100\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 72 80 7b[ 	]*vpinsrw \$0x7b,-0x100\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 b2 fe fe ff ff 7b[ 	]*vpinsrw \$0x7b,-0x102\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 04 f4[ 	]*vpmaddubsw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 04 f4[ 	]*vpmaddubsw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 04 f4[ 	]*vpmaddubsw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 31[ 	]*vpmaddubsw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 04 b4 f0 34 12 00 00[ 	]*vpmaddubsw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 72 7f[ 	]*vpmaddubsw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 b2 00 20 00 00[ 	]*vpmaddubsw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 72 80[ 	]*vpmaddubsw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 b2 c0 df ff ff[ 	]*vpmaddubsw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f5 f4[ 	]*vpmaddwd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f5 f4[ 	]*vpmaddwd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f5 f4[ 	]*vpmaddwd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 31[ 	]*vpmaddwd \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f5 b4 f0 34 12 00 00[ 	]*vpmaddwd 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 72 7f[ 	]*vpmaddwd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 b2 00 20 00 00[ 	]*vpmaddwd 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 72 80[ 	]*vpmaddwd -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 b2 c0 df ff ff[ 	]*vpmaddwd -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 3c f4[ 	]*vpmaxsb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 3c f4[ 	]*vpmaxsb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 3c f4[ 	]*vpmaxsb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c 31[ 	]*vpmaxsb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 3c b4 f0 34 12 00 00[ 	]*vpmaxsb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c 72 7f[ 	]*vpmaxsb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c b2 00 20 00 00[ 	]*vpmaxsb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c 72 80[ 	]*vpmaxsb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c b2 c0 df ff ff[ 	]*vpmaxsb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ee f4[ 	]*vpmaxsw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ee f4[ 	]*vpmaxsw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ee f4[ 	]*vpmaxsw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee 31[ 	]*vpmaxsw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ee b4 f0 34 12 00 00[ 	]*vpmaxsw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee 72 7f[ 	]*vpmaxsw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee b2 00 20 00 00[ 	]*vpmaxsw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee 72 80[ 	]*vpmaxsw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee b2 c0 df ff ff[ 	]*vpmaxsw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 de f4[ 	]*vpmaxub %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 de f4[ 	]*vpmaxub %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 de f4[ 	]*vpmaxub %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de 31[ 	]*vpmaxub \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 de b4 f0 34 12 00 00[ 	]*vpmaxub 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de 72 7f[ 	]*vpmaxub 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de b2 00 20 00 00[ 	]*vpmaxub 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de 72 80[ 	]*vpmaxub -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de b2 c0 df ff ff[ 	]*vpmaxub -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 3e f4[ 	]*vpmaxuw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 3e f4[ 	]*vpmaxuw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 3e f4[ 	]*vpmaxuw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e 31[ 	]*vpmaxuw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 3e b4 f0 34 12 00 00[ 	]*vpmaxuw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e 72 7f[ 	]*vpmaxuw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e b2 00 20 00 00[ 	]*vpmaxuw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e 72 80[ 	]*vpmaxuw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e b2 c0 df ff ff[ 	]*vpmaxuw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 38 f4[ 	]*vpminsb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 38 f4[ 	]*vpminsb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 38 f4[ 	]*vpminsb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 31[ 	]*vpminsb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 38 b4 f0 34 12 00 00[ 	]*vpminsb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 72 7f[ 	]*vpminsb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 b2 00 20 00 00[ 	]*vpminsb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 72 80[ 	]*vpminsb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 b2 c0 df ff ff[ 	]*vpminsb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ea f4[ 	]*vpminsw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ea f4[ 	]*vpminsw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ea f4[ 	]*vpminsw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea 31[ 	]*vpminsw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ea b4 f0 34 12 00 00[ 	]*vpminsw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea 72 7f[ 	]*vpminsw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea b2 00 20 00 00[ 	]*vpminsw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea 72 80[ 	]*vpminsw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea b2 c0 df ff ff[ 	]*vpminsw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 da f4[ 	]*vpminub %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 da f4[ 	]*vpminub %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 da f4[ 	]*vpminub %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da 31[ 	]*vpminub \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 da b4 f0 34 12 00 00[ 	]*vpminub 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da 72 7f[ 	]*vpminub 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da b2 00 20 00 00[ 	]*vpminub 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da 72 80[ 	]*vpminub -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da b2 c0 df ff ff[ 	]*vpminub -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 3a f4[ 	]*vpminuw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 3a f4[ 	]*vpminuw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 3a f4[ 	]*vpminuw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a 31[ 	]*vpminuw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 3a b4 f0 34 12 00 00[ 	]*vpminuw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a 72 7f[ 	]*vpminuw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a b2 00 20 00 00[ 	]*vpminuw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a 72 80[ 	]*vpminuw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a b2 c0 df ff ff[ 	]*vpminuw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 20 f5[ 	]*vpmovsxbw %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 20 f5[ 	]*vpmovsxbw %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 20 f5[ 	]*vpmovsxbw %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 31[ 	]*vpmovsxbw \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 20 b4 f0 34 12 00 00[ 	]*vpmovsxbw 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 72 7f[ 	]*vpmovsxbw 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 b2 00 10 00 00[ 	]*vpmovsxbw 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 72 80[ 	]*vpmovsxbw -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 b2 e0 ef ff ff[ 	]*vpmovsxbw -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 30 f5[ 	]*vpmovzxbw %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 30 f5[ 	]*vpmovzxbw %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 30 f5[ 	]*vpmovzxbw %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 31[ 	]*vpmovzxbw \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 30 b4 f0 34 12 00 00[ 	]*vpmovzxbw 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 72 7f[ 	]*vpmovzxbw 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 b2 00 10 00 00[ 	]*vpmovzxbw 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 72 80[ 	]*vpmovzxbw -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 b2 e0 ef ff ff[ 	]*vpmovzxbw -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 0b f4[ 	]*vpmulhrsw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 0b f4[ 	]*vpmulhrsw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 0b f4[ 	]*vpmulhrsw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b 31[ 	]*vpmulhrsw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 0b b4 f0 34 12 00 00[ 	]*vpmulhrsw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b 72 7f[ 	]*vpmulhrsw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b b2 00 20 00 00[ 	]*vpmulhrsw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b 72 80[ 	]*vpmulhrsw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b b2 c0 df ff ff[ 	]*vpmulhrsw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e4 f4[ 	]*vpmulhuw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e4 f4[ 	]*vpmulhuw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e4 f4[ 	]*vpmulhuw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 31[ 	]*vpmulhuw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e4 b4 f0 34 12 00 00[ 	]*vpmulhuw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 72 7f[ 	]*vpmulhuw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 b2 00 20 00 00[ 	]*vpmulhuw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 72 80[ 	]*vpmulhuw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 b2 c0 df ff ff[ 	]*vpmulhuw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e5 f4[ 	]*vpmulhw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e5 f4[ 	]*vpmulhw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e5 f4[ 	]*vpmulhw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 31[ 	]*vpmulhw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e5 b4 f0 34 12 00 00[ 	]*vpmulhw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 72 7f[ 	]*vpmulhw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 b2 00 20 00 00[ 	]*vpmulhw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 72 80[ 	]*vpmulhw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 b2 c0 df ff ff[ 	]*vpmulhw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d5 f4[ 	]*vpmullw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d5 f4[ 	]*vpmullw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d5 f4[ 	]*vpmullw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 31[ 	]*vpmullw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d5 b4 f0 34 12 00 00[ 	]*vpmullw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 72 7f[ 	]*vpmullw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 b2 00 20 00 00[ 	]*vpmullw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 72 80[ 	]*vpmullw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 b2 c0 df ff ff[ 	]*vpmullw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f6 f4[ 	]*vpsadbw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 31[ 	]*vpsadbw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f6 b4 f0 34 12 00 00[ 	]*vpsadbw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 72 7f[ 	]*vpsadbw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 b2 00 20 00 00[ 	]*vpsadbw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 72 80[ 	]*vpsadbw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 b2 c0 df ff ff[ 	]*vpsadbw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 00 f4[ 	]*vpshufb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 00 f4[ 	]*vpshufb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 00 f4[ 	]*vpshufb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 31[ 	]*vpshufb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 00 b4 f0 34 12 00 00[ 	]*vpshufb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 72 7f[ 	]*vpshufb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 b2 00 20 00 00[ 	]*vpshufb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 72 80[ 	]*vpshufb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 b2 c0 df ff ff[ 	]*vpshufb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 70 f5 ab[ 	]*vpshufhw \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 4f 70 f5 ab[ 	]*vpshufhw \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe cf 70 f5 ab[ 	]*vpshufhw \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 70 f5 7b[ 	]*vpshufhw \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 31 7b[ 	]*vpshufhw \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 48 70 b4 f0 34 12 00 00 7b[ 	]*vpshufhw \$0x7b,0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 72 7f 7b[ 	]*vpshufhw \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 b2 00 20 00 00 7b[ 	]*vpshufhw \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 72 80 7b[ 	]*vpshufhw \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 b2 c0 df ff ff 7b[ 	]*vpshufhw \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 70 f5 ab[ 	]*vpshuflw \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 4f 70 f5 ab[ 	]*vpshuflw \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff cf 70 f5 ab[ 	]*vpshuflw \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 70 f5 7b[ 	]*vpshuflw \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 31 7b[ 	]*vpshuflw \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 48 70 b4 f0 34 12 00 00 7b[ 	]*vpshuflw \$0x7b,0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 72 7f 7b[ 	]*vpshuflw \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 b2 00 20 00 00 7b[ 	]*vpshuflw \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 72 80 7b[ 	]*vpshuflw \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 b2 c0 df ff ff 7b[ 	]*vpshuflw \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f1 f4[ 	]*vpsllw %xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f1 f4[ 	]*vpsllw %xmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f1 f4[ 	]*vpsllw %xmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 31[ 	]*vpsllw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f1 b4 f0 34 12 00 00[ 	]*vpsllw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 72 7f[ 	]*vpsllw 0x7f0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 b2 00 08 00 00[ 	]*vpsllw 0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 72 80[ 	]*vpsllw -0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 b2 f0 f7 ff ff[ 	]*vpsllw -0x810\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e1 f4[ 	]*vpsraw %xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e1 f4[ 	]*vpsraw %xmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e1 f4[ 	]*vpsraw %xmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 31[ 	]*vpsraw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e1 b4 f0 34 12 00 00[ 	]*vpsraw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 72 7f[ 	]*vpsraw 0x7f0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 b2 00 08 00 00[ 	]*vpsraw 0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 72 80[ 	]*vpsraw -0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 b2 f0 f7 ff ff[ 	]*vpsraw -0x810\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d1 f4[ 	]*vpsrlw %xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d1 f4[ 	]*vpsrlw %xmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d1 f4[ 	]*vpsrlw %xmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 31[ 	]*vpsrlw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d1 b4 f0 34 12 00 00[ 	]*vpsrlw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 72 7f[ 	]*vpsrlw 0x7f0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 b2 00 08 00 00[ 	]*vpsrlw 0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 72 80[ 	]*vpsrlw -0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 b2 f0 f7 ff ff[ 	]*vpsrlw -0x810\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 dd ab[ 	]*vpsrldq \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 dd 7b[ 	]*vpsrldq \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 19 7b[ 	]*vpsrldq \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 73 9c f0 34 12 00 00 7b[ 	]*vpsrldq \$0x7b,0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 5a 7f 7b[ 	]*vpsrldq \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 9a 00 20 00 00 7b[ 	]*vpsrldq \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 5a 80 7b[ 	]*vpsrldq \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 9a c0 df ff ff 7b[ 	]*vpsrldq \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 d5 ab[ 	]*vpsrlw \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 71 d5 ab[ 	]*vpsrlw \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d c7 71 d5 ab[ 	]*vpsrlw \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 d5 7b[ 	]*vpsrlw \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 11 7b[ 	]*vpsrlw \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 71 94 f0 34 12 00 00 7b[ 	]*vpsrlw \$0x7b,0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 52 7f 7b[ 	]*vpsrlw \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 92 00 20 00 00 7b[ 	]*vpsrlw \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 52 80 7b[ 	]*vpsrlw \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 92 c0 df ff ff 7b[ 	]*vpsrlw \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 e5 ab[ 	]*vpsraw \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 71 e5 ab[ 	]*vpsraw \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d c7 71 e5 ab[ 	]*vpsraw \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 e5 7b[ 	]*vpsraw \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 21 7b[ 	]*vpsraw \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 71 a4 f0 34 12 00 00 7b[ 	]*vpsraw \$0x7b,0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 62 7f 7b[ 	]*vpsraw \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 a2 00 20 00 00 7b[ 	]*vpsraw \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 62 80 7b[ 	]*vpsraw \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 a2 c0 df ff ff 7b[ 	]*vpsraw \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f8 f4[ 	]*vpsubb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f8 f4[ 	]*vpsubb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f8 f4[ 	]*vpsubb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 31[ 	]*vpsubb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f8 b4 f0 34 12 00 00[ 	]*vpsubb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 72 7f[ 	]*vpsubb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 b2 00 20 00 00[ 	]*vpsubb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 72 80[ 	]*vpsubb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 b2 c0 df ff ff[ 	]*vpsubb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e8 f4[ 	]*vpsubsb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e8 f4[ 	]*vpsubsb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e8 f4[ 	]*vpsubsb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 31[ 	]*vpsubsb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e8 b4 f0 34 12 00 00[ 	]*vpsubsb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 72 7f[ 	]*vpsubsb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 b2 00 20 00 00[ 	]*vpsubsb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 72 80[ 	]*vpsubsb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 b2 c0 df ff ff[ 	]*vpsubsb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e9 f4[ 	]*vpsubsw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e9 f4[ 	]*vpsubsw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e9 f4[ 	]*vpsubsw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 31[ 	]*vpsubsw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e9 b4 f0 34 12 00 00[ 	]*vpsubsw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 72 7f[ 	]*vpsubsw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 b2 00 20 00 00[ 	]*vpsubsw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 72 80[ 	]*vpsubsw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 b2 c0 df ff ff[ 	]*vpsubsw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d8 f4[ 	]*vpsubusb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d8 f4[ 	]*vpsubusb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d8 f4[ 	]*vpsubusb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 31[ 	]*vpsubusb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d8 b4 f0 34 12 00 00[ 	]*vpsubusb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 72 7f[ 	]*vpsubusb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 b2 00 20 00 00[ 	]*vpsubusb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 72 80[ 	]*vpsubusb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 b2 c0 df ff ff[ 	]*vpsubusb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d9 f4[ 	]*vpsubusw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d9 f4[ 	]*vpsubusw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d9 f4[ 	]*vpsubusw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 31[ 	]*vpsubusw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d9 b4 f0 34 12 00 00[ 	]*vpsubusw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 72 7f[ 	]*vpsubusw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 b2 00 20 00 00[ 	]*vpsubusw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 72 80[ 	]*vpsubusw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 b2 c0 df ff ff[ 	]*vpsubusw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f9 f4[ 	]*vpsubw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f9 f4[ 	]*vpsubw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f9 f4[ 	]*vpsubw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 31[ 	]*vpsubw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f9 b4 f0 34 12 00 00[ 	]*vpsubw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 72 7f[ 	]*vpsubw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 b2 00 20 00 00[ 	]*vpsubw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 72 80[ 	]*vpsubw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 b2 c0 df ff ff[ 	]*vpsubw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 68 f4[ 	]*vpunpckhbw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 68 f4[ 	]*vpunpckhbw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 68 f4[ 	]*vpunpckhbw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 31[ 	]*vpunpckhbw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 68 b4 f0 34 12 00 00[ 	]*vpunpckhbw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 72 7f[ 	]*vpunpckhbw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 b2 00 20 00 00[ 	]*vpunpckhbw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 72 80[ 	]*vpunpckhbw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 b2 c0 df ff ff[ 	]*vpunpckhbw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 69 f4[ 	]*vpunpckhwd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 69 f4[ 	]*vpunpckhwd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 69 f4[ 	]*vpunpckhwd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 31[ 	]*vpunpckhwd \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 69 b4 f0 34 12 00 00[ 	]*vpunpckhwd 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 72 7f[ 	]*vpunpckhwd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 b2 00 20 00 00[ 	]*vpunpckhwd 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 72 80[ 	]*vpunpckhwd -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 b2 c0 df ff ff[ 	]*vpunpckhwd -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 60 f4[ 	]*vpunpcklbw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 60 f4[ 	]*vpunpcklbw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 60 f4[ 	]*vpunpcklbw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 31[ 	]*vpunpcklbw \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 60 b4 f0 34 12 00 00[ 	]*vpunpcklbw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 72 7f[ 	]*vpunpcklbw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 b2 00 20 00 00[ 	]*vpunpcklbw 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 72 80[ 	]*vpunpcklbw -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 b2 c0 df ff ff[ 	]*vpunpcklbw -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 61 f4[ 	]*vpunpcklwd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 61 f4[ 	]*vpunpcklwd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 61 f4[ 	]*vpunpcklwd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 31[ 	]*vpunpcklwd \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 61 b4 f0 34 12 00 00[ 	]*vpunpcklwd 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 72 7f[ 	]*vpunpcklwd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 b2 00 20 00 00[ 	]*vpunpcklwd 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 72 80[ 	]*vpunpcklwd -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 b2 c0 df ff ff[ 	]*vpunpcklwd -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 fd ab[ 	]*vpslldq \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 fd 7b[ 	]*vpslldq \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 39 7b[ 	]*vpslldq \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 73 bc f0 34 12 00 00 7b[ 	]*vpslldq \$0x7b,0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 7a 7f 7b[ 	]*vpslldq \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 ba 00 20 00 00 7b[ 	]*vpslldq \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 7a 80 7b[ 	]*vpslldq \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 ba c0 df ff ff 7b[ 	]*vpslldq \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 f5 ab[ 	]*vpsllw \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 71 f5 ab[ 	]*vpsllw \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d c7 71 f5 ab[ 	]*vpsllw \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 f5 7b[ 	]*vpsllw \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 31 7b[ 	]*vpsllw \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 71 b4 f0 34 12 00 00 7b[ 	]*vpsllw \$0x7b,0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 72 7f 7b[ 	]*vpsllw \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 b2 00 20 00 00 7b[ 	]*vpsllw \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 72 80 7b[ 	]*vpsllw \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 b2 c0 df ff ff 7b[ 	]*vpsllw \$0x7b,-0x2040\(%rdx\),%zmm30
#pass
