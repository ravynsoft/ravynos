#as: -mevexwig=1
#objdump: -dw
#name: i386 AVX512BW wig insns
#source: avx512bw-wig.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c f5[ 	]*vpabsb %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 1c f5[ 	]*vpabsb %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 1c f5[ 	]*vpabsb %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c 31[ 	]*vpabsb \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c b4 f4 c0 1d fe ff[ 	]*vpabsb -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c 72 7f[ 	]*vpabsb 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c b2 00 20 00 00[ 	]*vpabsb 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c 72 80[ 	]*vpabsb -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c b2 c0 df ff ff[ 	]*vpabsb -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d f5[ 	]*vpabsw %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 1d f5[ 	]*vpabsw %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 1d f5[ 	]*vpabsw %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d 31[ 	]*vpabsw \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d b4 f4 c0 1d fe ff[ 	]*vpabsw -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d 72 7f[ 	]*vpabsw 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d b2 00 20 00 00[ 	]*vpabsw 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d 72 80[ 	]*vpabsw -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d b2 c0 df ff ff[ 	]*vpabsw -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 f4[ 	]*vpacksswb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 63 f4[ 	]*vpacksswb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 63 f4[ 	]*vpacksswb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 31[ 	]*vpacksswb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 72 7f[ 	]*vpacksswb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 b2 00 20 00 00[ 	]*vpacksswb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 72 80[ 	]*vpacksswb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 b2 c0 df ff ff[ 	]*vpacksswb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 f4[ 	]*vpackuswb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 67 f4[ 	]*vpackuswb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 67 f4[ 	]*vpackuswb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 31[ 	]*vpackuswb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 72 7f[ 	]*vpackuswb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 b2 00 20 00 00[ 	]*vpackuswb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 72 80[ 	]*vpackuswb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 b2 c0 df ff ff[ 	]*vpackuswb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc f4[ 	]*vpaddb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f fc f4[ 	]*vpaddb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf fc f4[ 	]*vpaddb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc 31[ 	]*vpaddb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc b4 f4 c0 1d fe ff[ 	]*vpaddb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc 72 7f[ 	]*vpaddb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc b2 00 20 00 00[ 	]*vpaddb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc 72 80[ 	]*vpaddb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc b2 c0 df ff ff[ 	]*vpaddb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec f4[ 	]*vpaddsb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f ec f4[ 	]*vpaddsb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf ec f4[ 	]*vpaddsb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec 31[ 	]*vpaddsb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec b4 f4 c0 1d fe ff[ 	]*vpaddsb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec 72 7f[ 	]*vpaddsb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec b2 00 20 00 00[ 	]*vpaddsb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec 72 80[ 	]*vpaddsb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec b2 c0 df ff ff[ 	]*vpaddsb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed f4[ 	]*vpaddsw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f ed f4[ 	]*vpaddsw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf ed f4[ 	]*vpaddsw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed 31[ 	]*vpaddsw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed b4 f4 c0 1d fe ff[ 	]*vpaddsw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed 72 7f[ 	]*vpaddsw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed b2 00 20 00 00[ 	]*vpaddsw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed 72 80[ 	]*vpaddsw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed b2 c0 df ff ff[ 	]*vpaddsw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc f4[ 	]*vpaddusb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f dc f4[ 	]*vpaddusb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf dc f4[ 	]*vpaddusb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc 31[ 	]*vpaddusb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc b4 f4 c0 1d fe ff[ 	]*vpaddusb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc 72 7f[ 	]*vpaddusb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc b2 00 20 00 00[ 	]*vpaddusb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc 72 80[ 	]*vpaddusb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc b2 c0 df ff ff[ 	]*vpaddusb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd f4[ 	]*vpaddusw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f dd f4[ 	]*vpaddusw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf dd f4[ 	]*vpaddusw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd 31[ 	]*vpaddusw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd b4 f4 c0 1d fe ff[ 	]*vpaddusw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd 72 7f[ 	]*vpaddusw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd b2 00 20 00 00[ 	]*vpaddusw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd 72 80[ 	]*vpaddusw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd b2 c0 df ff ff[ 	]*vpaddusw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd f4[ 	]*vpaddw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f fd f4[ 	]*vpaddw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf fd f4[ 	]*vpaddw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd 31[ 	]*vpaddw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd b4 f4 c0 1d fe ff[ 	]*vpaddw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd 72 7f[ 	]*vpaddw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd b2 00 20 00 00[ 	]*vpaddw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd 72 80[ 	]*vpaddw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd b2 c0 df ff ff[ 	]*vpaddw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f f4 ab[ 	]*vpalignr \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 0f f4 ab[ 	]*vpalignr \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 0f f4 ab[ 	]*vpalignr \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f f4 7b[ 	]*vpalignr \$0x7b,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f 31 7b[ 	]*vpalignr \$0x7b,\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f 72 7f 7b[ 	]*vpalignr \$0x7b,0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f b2 00 20 00 00 7b[ 	]*vpalignr \$0x7b,0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f 72 80 7b[ 	]*vpalignr \$0x7b,-0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f b2 c0 df ff ff 7b[ 	]*vpalignr \$0x7b,-0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 f4[ 	]*vpavgb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e0 f4[ 	]*vpavgb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e0 f4[ 	]*vpavgb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 31[ 	]*vpavgb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 b4 f4 c0 1d fe ff[ 	]*vpavgb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 72 7f[ 	]*vpavgb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 b2 00 20 00 00[ 	]*vpavgb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 72 80[ 	]*vpavgb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 b2 c0 df ff ff[ 	]*vpavgb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 f4[ 	]*vpavgw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e3 f4[ 	]*vpavgw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e3 f4[ 	]*vpavgw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 31[ 	]*vpavgw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 b4 f4 c0 1d fe ff[ 	]*vpavgw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 72 7f[ 	]*vpavgw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 b2 00 20 00 00[ 	]*vpavgw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 72 80[ 	]*vpavgw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 b2 c0 df ff ff[ 	]*vpavgw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 ed[ 	]*vpcmpeqb %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 74 ed[ 	]*vpcmpeqb %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 29[ 	]*vpcmpeqb \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 6a 7f[ 	]*vpcmpeqb 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 aa 00 20 00 00[ 	]*vpcmpeqb 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 6a 80[ 	]*vpcmpeqb -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 aa c0 df ff ff[ 	]*vpcmpeqb -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 ed[ 	]*vpcmpeqw %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 75 ed[ 	]*vpcmpeqw %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 29[ 	]*vpcmpeqw \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 6a 7f[ 	]*vpcmpeqw 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 aa 00 20 00 00[ 	]*vpcmpeqw 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 6a 80[ 	]*vpcmpeqw -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 aa c0 df ff ff[ 	]*vpcmpeqw -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 ed[ 	]*vpcmpgtb %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 64 ed[ 	]*vpcmpgtb %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 29[ 	]*vpcmpgtb \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 6a 7f[ 	]*vpcmpgtb 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 aa 00 20 00 00[ 	]*vpcmpgtb 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 6a 80[ 	]*vpcmpgtb -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 aa c0 df ff ff[ 	]*vpcmpgtb -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 ed[ 	]*vpcmpgtw %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 65 ed[ 	]*vpcmpgtw %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 29[ 	]*vpcmpgtw \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 6a 7f[ 	]*vpcmpgtw 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 aa 00 20 00 00[ 	]*vpcmpgtw 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 6a 80[ 	]*vpcmpgtw -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 aa c0 df ff ff[ 	]*vpcmpgtw -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 f4[ 	]*vpmaddubsw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 04 f4[ 	]*vpmaddubsw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 04 f4[ 	]*vpmaddubsw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 31[ 	]*vpmaddubsw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 72 7f[ 	]*vpmaddubsw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 b2 00 20 00 00[ 	]*vpmaddubsw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 72 80[ 	]*vpmaddubsw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 b2 c0 df ff ff[ 	]*vpmaddubsw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 f4[ 	]*vpmaddwd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f5 f4[ 	]*vpmaddwd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf f5 f4[ 	]*vpmaddwd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 31[ 	]*vpmaddwd \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 72 7f[ 	]*vpmaddwd 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 b2 00 20 00 00[ 	]*vpmaddwd 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 72 80[ 	]*vpmaddwd -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 b2 c0 df ff ff[ 	]*vpmaddwd -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c f4[ 	]*vpmaxsb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 3c f4[ 	]*vpmaxsb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 3c f4[ 	]*vpmaxsb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c 31[ 	]*vpmaxsb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c 72 7f[ 	]*vpmaxsb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c b2 00 20 00 00[ 	]*vpmaxsb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c 72 80[ 	]*vpmaxsb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c b2 c0 df ff ff[ 	]*vpmaxsb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee f4[ 	]*vpmaxsw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f ee f4[ 	]*vpmaxsw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf ee f4[ 	]*vpmaxsw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee 31[ 	]*vpmaxsw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee 72 7f[ 	]*vpmaxsw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee b2 00 20 00 00[ 	]*vpmaxsw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee 72 80[ 	]*vpmaxsw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee b2 c0 df ff ff[ 	]*vpmaxsw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de f4[ 	]*vpmaxub %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f de f4[ 	]*vpmaxub %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf de f4[ 	]*vpmaxub %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de 31[ 	]*vpmaxub \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de b4 f4 c0 1d fe ff[ 	]*vpmaxub -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de 72 7f[ 	]*vpmaxub 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de b2 00 20 00 00[ 	]*vpmaxub 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de 72 80[ 	]*vpmaxub -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de b2 c0 df ff ff[ 	]*vpmaxub -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e f4[ 	]*vpmaxuw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 3e f4[ 	]*vpmaxuw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 3e f4[ 	]*vpmaxuw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e 31[ 	]*vpmaxuw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e 72 7f[ 	]*vpmaxuw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e b2 00 20 00 00[ 	]*vpmaxuw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e 72 80[ 	]*vpmaxuw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e b2 c0 df ff ff[ 	]*vpmaxuw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 f4[ 	]*vpminsb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 38 f4[ 	]*vpminsb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 38 f4[ 	]*vpminsb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 31[ 	]*vpminsb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 b4 f4 c0 1d fe ff[ 	]*vpminsb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 72 7f[ 	]*vpminsb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 b2 00 20 00 00[ 	]*vpminsb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 72 80[ 	]*vpminsb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 b2 c0 df ff ff[ 	]*vpminsb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea f4[ 	]*vpminsw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f ea f4[ 	]*vpminsw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf ea f4[ 	]*vpminsw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea 31[ 	]*vpminsw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea b4 f4 c0 1d fe ff[ 	]*vpminsw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea 72 7f[ 	]*vpminsw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea b2 00 20 00 00[ 	]*vpminsw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea 72 80[ 	]*vpminsw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea b2 c0 df ff ff[ 	]*vpminsw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da f4[ 	]*vpminub %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f da f4[ 	]*vpminub %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf da f4[ 	]*vpminub %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da 31[ 	]*vpminub \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da b4 f4 c0 1d fe ff[ 	]*vpminub -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da 72 7f[ 	]*vpminub 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da b2 00 20 00 00[ 	]*vpminub 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da 72 80[ 	]*vpminub -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da b2 c0 df ff ff[ 	]*vpminub -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a f4[ 	]*vpminuw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 3a f4[ 	]*vpminuw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 3a f4[ 	]*vpminuw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a 31[ 	]*vpminuw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a b4 f4 c0 1d fe ff[ 	]*vpminuw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a 72 7f[ 	]*vpminuw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a b2 00 20 00 00[ 	]*vpminuw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a 72 80[ 	]*vpminuw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a b2 c0 df ff ff[ 	]*vpminuw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 f5[ 	]*vpmovsxbw %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 20 f5[ 	]*vpmovsxbw %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 31[ 	]*vpmovsxbw \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 72 7f[ 	]*vpmovsxbw 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 b2 00 10 00 00[ 	]*vpmovsxbw 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 72 80[ 	]*vpmovsxbw -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 b2 e0 ef ff ff[ 	]*vpmovsxbw -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 f5[ 	]*vpmovzxbw %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 30 f5[ 	]*vpmovzxbw %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 31[ 	]*vpmovzxbw \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 72 7f[ 	]*vpmovzxbw 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 b2 00 10 00 00[ 	]*vpmovzxbw 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 72 80[ 	]*vpmovzxbw -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 b2 e0 ef ff ff[ 	]*vpmovzxbw -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b f4[ 	]*vpmulhrsw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 0b f4[ 	]*vpmulhrsw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 0b f4[ 	]*vpmulhrsw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b 31[ 	]*vpmulhrsw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b 72 7f[ 	]*vpmulhrsw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b b2 00 20 00 00[ 	]*vpmulhrsw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b 72 80[ 	]*vpmulhrsw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b b2 c0 df ff ff[ 	]*vpmulhrsw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 f4[ 	]*vpmulhuw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e4 f4[ 	]*vpmulhuw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e4 f4[ 	]*vpmulhuw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 31[ 	]*vpmulhuw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 72 7f[ 	]*vpmulhuw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 b2 00 20 00 00[ 	]*vpmulhuw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 72 80[ 	]*vpmulhuw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 b2 c0 df ff ff[ 	]*vpmulhuw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 f4[ 	]*vpmulhw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e5 f4[ 	]*vpmulhw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e5 f4[ 	]*vpmulhw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 31[ 	]*vpmulhw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 72 7f[ 	]*vpmulhw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 b2 00 20 00 00[ 	]*vpmulhw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 72 80[ 	]*vpmulhw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 b2 c0 df ff ff[ 	]*vpmulhw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 f4[ 	]*vpmullw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d5 f4[ 	]*vpmullw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf d5 f4[ 	]*vpmullw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 31[ 	]*vpmullw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 b4 f4 c0 1d fe ff[ 	]*vpmullw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 72 7f[ 	]*vpmullw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 b2 00 20 00 00[ 	]*vpmullw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 72 80[ 	]*vpmullw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 b2 c0 df ff ff[ 	]*vpmullw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 f4[ 	]*vpsadbw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 31[ 	]*vpsadbw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 b4 f4 c0 1d fe ff[ 	]*vpsadbw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 72 7f[ 	]*vpsadbw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 b2 00 20 00 00[ 	]*vpsadbw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 72 80[ 	]*vpsadbw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 b2 c0 df ff ff[ 	]*vpsadbw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 f4[ 	]*vpshufb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 00 f4[ 	]*vpshufb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 00 f4[ 	]*vpshufb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 31[ 	]*vpshufb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 b4 f4 c0 1d fe ff[ 	]*vpshufb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 72 7f[ 	]*vpshufb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 b2 00 20 00 00[ 	]*vpshufb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 72 80[ 	]*vpshufb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 b2 c0 df ff ff[ 	]*vpshufb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 f5 ab[ 	]*vpshufhw \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 4f 70 f5 ab[ 	]*vpshufhw \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe cf 70 f5 ab[ 	]*vpshufhw \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 f5 7b[ 	]*vpshufhw \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 31 7b[ 	]*vpshufhw \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 72 7f 7b[ 	]*vpshufhw \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 b2 00 20 00 00 7b[ 	]*vpshufhw \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 72 80 7b[ 	]*vpshufhw \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 b2 c0 df ff ff 7b[ 	]*vpshufhw \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 f5 ab[ 	]*vpshuflw \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 70 f5 ab[ 	]*vpshuflw \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 70 f5 ab[ 	]*vpshuflw \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 f5 7b[ 	]*vpshuflw \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 31 7b[ 	]*vpshuflw \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 72 7f 7b[ 	]*vpshuflw \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 b2 00 20 00 00 7b[ 	]*vpshuflw \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 72 80 7b[ 	]*vpshuflw \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 b2 c0 df ff ff 7b[ 	]*vpshuflw \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 f4[ 	]*vpsllw %xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf f1 f4[ 	]*vpsllw %xmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 31[ 	]*vpsllw \(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 72 7f[ 	]*vpsllw 0x7f0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 b2 00 08 00 00[ 	]*vpsllw 0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 72 80[ 	]*vpsllw -0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 b2 f0 f7 ff ff[ 	]*vpsllw -0x810\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 f4[ 	]*vpsraw %xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e1 f4[ 	]*vpsraw %xmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 31[ 	]*vpsraw \(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 72 7f[ 	]*vpsraw 0x7f0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 b2 00 08 00 00[ 	]*vpsraw 0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 72 80[ 	]*vpsraw -0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 b2 f0 f7 ff ff[ 	]*vpsraw -0x810\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 f4[ 	]*vpsrlw %xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf d1 f4[ 	]*vpsrlw %xmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 31[ 	]*vpsrlw \(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 72 7f[ 	]*vpsrlw 0x7f0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 b2 00 08 00 00[ 	]*vpsrlw 0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 72 80[ 	]*vpsrlw -0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 b2 f0 f7 ff ff[ 	]*vpsrlw -0x810\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 dd ab[ 	]*vpsrldq \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 dd 7b[ 	]*vpsrldq \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 19 7b[ 	]*vpsrldq \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 9c f4 c0 1d fe ff 7b[ 	]*vpsrldq \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 5a 7f 7b[ 	]*vpsrldq \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 9a 00 20 00 00 7b[ 	]*vpsrldq \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 5a 80 7b[ 	]*vpsrldq \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 9a c0 df ff ff 7b[ 	]*vpsrldq \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 d5 ab[ 	]*vpsrlw \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 71 d5 ab[ 	]*vpsrlw \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd cf 71 d5 ab[ 	]*vpsrlw \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 d5 7b[ 	]*vpsrlw \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 11 7b[ 	]*vpsrlw \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 52 7f 7b[ 	]*vpsrlw \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 92 00 20 00 00 7b[ 	]*vpsrlw \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 52 80 7b[ 	]*vpsrlw \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 92 c0 df ff ff 7b[ 	]*vpsrlw \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 e5 ab[ 	]*vpsraw \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 71 e5 ab[ 	]*vpsraw \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd cf 71 e5 ab[ 	]*vpsraw \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 e5 7b[ 	]*vpsraw \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 21 7b[ 	]*vpsraw \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 62 7f 7b[ 	]*vpsraw \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 a2 00 20 00 00 7b[ 	]*vpsraw \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 62 80 7b[ 	]*vpsraw \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 a2 c0 df ff ff 7b[ 	]*vpsraw \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 f4[ 	]*vpsubb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f8 f4[ 	]*vpsubb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf f8 f4[ 	]*vpsubb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 31[ 	]*vpsubb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 b4 f4 c0 1d fe ff[ 	]*vpsubb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 72 7f[ 	]*vpsubb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 b2 00 20 00 00[ 	]*vpsubb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 72 80[ 	]*vpsubb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 b2 c0 df ff ff[ 	]*vpsubb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 f4[ 	]*vpsubsb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e8 f4[ 	]*vpsubsb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e8 f4[ 	]*vpsubsb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 31[ 	]*vpsubsb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 72 7f[ 	]*vpsubsb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 b2 00 20 00 00[ 	]*vpsubsb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 72 80[ 	]*vpsubsb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 b2 c0 df ff ff[ 	]*vpsubsb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 f4[ 	]*vpsubsw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e9 f4[ 	]*vpsubsw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e9 f4[ 	]*vpsubsw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 31[ 	]*vpsubsw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 72 7f[ 	]*vpsubsw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 b2 00 20 00 00[ 	]*vpsubsw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 72 80[ 	]*vpsubsw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 b2 c0 df ff ff[ 	]*vpsubsw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 f4[ 	]*vpsubusb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d8 f4[ 	]*vpsubusb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf d8 f4[ 	]*vpsubusb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 31[ 	]*vpsubusb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 72 7f[ 	]*vpsubusb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 b2 00 20 00 00[ 	]*vpsubusb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 72 80[ 	]*vpsubusb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 b2 c0 df ff ff[ 	]*vpsubusb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 f4[ 	]*vpsubusw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d9 f4[ 	]*vpsubusw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf d9 f4[ 	]*vpsubusw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 31[ 	]*vpsubusw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 72 7f[ 	]*vpsubusw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 b2 00 20 00 00[ 	]*vpsubusw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 72 80[ 	]*vpsubusw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 b2 c0 df ff ff[ 	]*vpsubusw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 f4[ 	]*vpsubw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f9 f4[ 	]*vpsubw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf f9 f4[ 	]*vpsubw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 31[ 	]*vpsubw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 b4 f4 c0 1d fe ff[ 	]*vpsubw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 72 7f[ 	]*vpsubw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 b2 00 20 00 00[ 	]*vpsubw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 72 80[ 	]*vpsubw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 b2 c0 df ff ff[ 	]*vpsubw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 f4[ 	]*vpunpckhbw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 68 f4[ 	]*vpunpckhbw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 68 f4[ 	]*vpunpckhbw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 31[ 	]*vpunpckhbw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 72 7f[ 	]*vpunpckhbw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 b2 00 20 00 00[ 	]*vpunpckhbw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 72 80[ 	]*vpunpckhbw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 b2 c0 df ff ff[ 	]*vpunpckhbw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 f4[ 	]*vpunpckhwd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 69 f4[ 	]*vpunpckhwd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 69 f4[ 	]*vpunpckhwd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 31[ 	]*vpunpckhwd \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 72 7f[ 	]*vpunpckhwd 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 b2 00 20 00 00[ 	]*vpunpckhwd 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 72 80[ 	]*vpunpckhwd -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 b2 c0 df ff ff[ 	]*vpunpckhwd -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 f4[ 	]*vpunpcklbw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 60 f4[ 	]*vpunpcklbw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 60 f4[ 	]*vpunpcklbw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 31[ 	]*vpunpcklbw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 72 7f[ 	]*vpunpcklbw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 b2 00 20 00 00[ 	]*vpunpcklbw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 72 80[ 	]*vpunpcklbw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 b2 c0 df ff ff[ 	]*vpunpcklbw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 f4[ 	]*vpunpcklwd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 61 f4[ 	]*vpunpcklwd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 61 f4[ 	]*vpunpcklwd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 31[ 	]*vpunpcklwd \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 72 7f[ 	]*vpunpcklwd 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 b2 00 20 00 00[ 	]*vpunpcklwd 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 72 80[ 	]*vpunpcklwd -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 b2 c0 df ff ff[ 	]*vpunpcklwd -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 fd ab[ 	]*vpslldq \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 fd 7b[ 	]*vpslldq \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 39 7b[ 	]*vpslldq \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 bc f4 c0 1d fe ff 7b[ 	]*vpslldq \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 7a 7f 7b[ 	]*vpslldq \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 ba 00 20 00 00 7b[ 	]*vpslldq \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 7a 80 7b[ 	]*vpslldq \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 ba c0 df ff ff 7b[ 	]*vpslldq \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 f5 ab[ 	]*vpsllw \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 71 f5 ab[ 	]*vpsllw \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd cf 71 f5 ab[ 	]*vpsllw \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 f5 7b[ 	]*vpsllw \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 31 7b[ 	]*vpsllw \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 72 7f 7b[ 	]*vpsllw \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 b2 00 20 00 00 7b[ 	]*vpsllw \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 72 80 7b[ 	]*vpsllw \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 b2 c0 df ff ff 7b[ 	]*vpsllw \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c f5[ 	]*vpabsb %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 1c f5[ 	]*vpabsb %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 1c f5[ 	]*vpabsb %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c 31[ 	]*vpabsb \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c b4 f4 c0 1d fe ff[ 	]*vpabsb -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c 72 7f[ 	]*vpabsb 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c b2 00 20 00 00[ 	]*vpabsb 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c 72 80[ 	]*vpabsb -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1c b2 c0 df ff ff[ 	]*vpabsb -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d f5[ 	]*vpabsw %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 1d f5[ 	]*vpabsw %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 1d f5[ 	]*vpabsw %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d 31[ 	]*vpabsw \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d b4 f4 c0 1d fe ff[ 	]*vpabsw -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d 72 7f[ 	]*vpabsw 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d b2 00 20 00 00[ 	]*vpabsw 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d 72 80[ 	]*vpabsw -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1d b2 c0 df ff ff[ 	]*vpabsw -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 f4[ 	]*vpacksswb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 63 f4[ 	]*vpacksswb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 63 f4[ 	]*vpacksswb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 31[ 	]*vpacksswb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 72 7f[ 	]*vpacksswb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 b2 00 20 00 00[ 	]*vpacksswb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 72 80[ 	]*vpacksswb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 63 b2 c0 df ff ff[ 	]*vpacksswb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 f4[ 	]*vpackuswb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 67 f4[ 	]*vpackuswb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 67 f4[ 	]*vpackuswb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 31[ 	]*vpackuswb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 72 7f[ 	]*vpackuswb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 b2 00 20 00 00[ 	]*vpackuswb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 72 80[ 	]*vpackuswb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 67 b2 c0 df ff ff[ 	]*vpackuswb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc f4[ 	]*vpaddb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f fc f4[ 	]*vpaddb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf fc f4[ 	]*vpaddb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc 31[ 	]*vpaddb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc b4 f4 c0 1d fe ff[ 	]*vpaddb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc 72 7f[ 	]*vpaddb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc b2 00 20 00 00[ 	]*vpaddb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc 72 80[ 	]*vpaddb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fc b2 c0 df ff ff[ 	]*vpaddb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec f4[ 	]*vpaddsb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f ec f4[ 	]*vpaddsb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf ec f4[ 	]*vpaddsb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec 31[ 	]*vpaddsb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec b4 f4 c0 1d fe ff[ 	]*vpaddsb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec 72 7f[ 	]*vpaddsb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec b2 00 20 00 00[ 	]*vpaddsb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec 72 80[ 	]*vpaddsb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ec b2 c0 df ff ff[ 	]*vpaddsb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed f4[ 	]*vpaddsw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f ed f4[ 	]*vpaddsw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf ed f4[ 	]*vpaddsw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed 31[ 	]*vpaddsw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed b4 f4 c0 1d fe ff[ 	]*vpaddsw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed 72 7f[ 	]*vpaddsw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed b2 00 20 00 00[ 	]*vpaddsw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed 72 80[ 	]*vpaddsw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ed b2 c0 df ff ff[ 	]*vpaddsw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc f4[ 	]*vpaddusb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f dc f4[ 	]*vpaddusb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf dc f4[ 	]*vpaddusb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc 31[ 	]*vpaddusb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc b4 f4 c0 1d fe ff[ 	]*vpaddusb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc 72 7f[ 	]*vpaddusb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc b2 00 20 00 00[ 	]*vpaddusb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc 72 80[ 	]*vpaddusb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dc b2 c0 df ff ff[ 	]*vpaddusb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd f4[ 	]*vpaddusw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f dd f4[ 	]*vpaddusw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf dd f4[ 	]*vpaddusw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd 31[ 	]*vpaddusw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd b4 f4 c0 1d fe ff[ 	]*vpaddusw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd 72 7f[ 	]*vpaddusw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd b2 00 20 00 00[ 	]*vpaddusw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd 72 80[ 	]*vpaddusw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 dd b2 c0 df ff ff[ 	]*vpaddusw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd f4[ 	]*vpaddw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f fd f4[ 	]*vpaddw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf fd f4[ 	]*vpaddw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd 31[ 	]*vpaddw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd b4 f4 c0 1d fe ff[ 	]*vpaddw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd 72 7f[ 	]*vpaddw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd b2 00 20 00 00[ 	]*vpaddw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd 72 80[ 	]*vpaddw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 fd b2 c0 df ff ff[ 	]*vpaddw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f f4 ab[ 	]*vpalignr \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 0f f4 ab[ 	]*vpalignr \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 0f f4 ab[ 	]*vpalignr \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f f4 7b[ 	]*vpalignr \$0x7b,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f 31 7b[ 	]*vpalignr \$0x7b,\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f 72 7f 7b[ 	]*vpalignr \$0x7b,0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f b2 00 20 00 00 7b[ 	]*vpalignr \$0x7b,0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f 72 80 7b[ 	]*vpalignr \$0x7b,-0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 0f b2 c0 df ff ff 7b[ 	]*vpalignr \$0x7b,-0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 f4[ 	]*vpavgb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e0 f4[ 	]*vpavgb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e0 f4[ 	]*vpavgb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 31[ 	]*vpavgb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 b4 f4 c0 1d fe ff[ 	]*vpavgb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 72 7f[ 	]*vpavgb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 b2 00 20 00 00[ 	]*vpavgb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 72 80[ 	]*vpavgb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e0 b2 c0 df ff ff[ 	]*vpavgb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 f4[ 	]*vpavgw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e3 f4[ 	]*vpavgw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e3 f4[ 	]*vpavgw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 31[ 	]*vpavgw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 b4 f4 c0 1d fe ff[ 	]*vpavgw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 72 7f[ 	]*vpavgw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 b2 00 20 00 00[ 	]*vpavgw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 72 80[ 	]*vpavgw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e3 b2 c0 df ff ff[ 	]*vpavgw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 ed[ 	]*vpcmpeqb %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 74 ed[ 	]*vpcmpeqb %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 29[ 	]*vpcmpeqb \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 6a 7f[ 	]*vpcmpeqb 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 aa 00 20 00 00[ 	]*vpcmpeqb 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 6a 80[ 	]*vpcmpeqb -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 74 aa c0 df ff ff[ 	]*vpcmpeqb -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 ed[ 	]*vpcmpeqw %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 75 ed[ 	]*vpcmpeqw %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 29[ 	]*vpcmpeqw \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 6a 7f[ 	]*vpcmpeqw 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 aa 00 20 00 00[ 	]*vpcmpeqw 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 6a 80[ 	]*vpcmpeqw -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 75 aa c0 df ff ff[ 	]*vpcmpeqw -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 ed[ 	]*vpcmpgtb %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 64 ed[ 	]*vpcmpgtb %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 29[ 	]*vpcmpgtb \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 6a 7f[ 	]*vpcmpgtb 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 aa 00 20 00 00[ 	]*vpcmpgtb 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 6a 80[ 	]*vpcmpgtb -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 64 aa c0 df ff ff[ 	]*vpcmpgtb -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 ed[ 	]*vpcmpgtw %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 65 ed[ 	]*vpcmpgtw %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 29[ 	]*vpcmpgtw \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 6a 7f[ 	]*vpcmpgtw 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 aa 00 20 00 00[ 	]*vpcmpgtw 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 6a 80[ 	]*vpcmpgtw -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 65 aa c0 df ff ff[ 	]*vpcmpgtw -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 f4[ 	]*vpmaddubsw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 04 f4[ 	]*vpmaddubsw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 04 f4[ 	]*vpmaddubsw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 31[ 	]*vpmaddubsw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 72 7f[ 	]*vpmaddubsw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 b2 00 20 00 00[ 	]*vpmaddubsw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 72 80[ 	]*vpmaddubsw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 04 b2 c0 df ff ff[ 	]*vpmaddubsw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 f4[ 	]*vpmaddwd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f5 f4[ 	]*vpmaddwd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf f5 f4[ 	]*vpmaddwd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 31[ 	]*vpmaddwd \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 72 7f[ 	]*vpmaddwd 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 b2 00 20 00 00[ 	]*vpmaddwd 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 72 80[ 	]*vpmaddwd -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f5 b2 c0 df ff ff[ 	]*vpmaddwd -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c f4[ 	]*vpmaxsb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 3c f4[ 	]*vpmaxsb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 3c f4[ 	]*vpmaxsb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c 31[ 	]*vpmaxsb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c 72 7f[ 	]*vpmaxsb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c b2 00 20 00 00[ 	]*vpmaxsb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c 72 80[ 	]*vpmaxsb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3c b2 c0 df ff ff[ 	]*vpmaxsb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee f4[ 	]*vpmaxsw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f ee f4[ 	]*vpmaxsw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf ee f4[ 	]*vpmaxsw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee 31[ 	]*vpmaxsw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee 72 7f[ 	]*vpmaxsw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee b2 00 20 00 00[ 	]*vpmaxsw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee 72 80[ 	]*vpmaxsw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ee b2 c0 df ff ff[ 	]*vpmaxsw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de f4[ 	]*vpmaxub %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f de f4[ 	]*vpmaxub %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf de f4[ 	]*vpmaxub %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de 31[ 	]*vpmaxub \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de b4 f4 c0 1d fe ff[ 	]*vpmaxub -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de 72 7f[ 	]*vpmaxub 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de b2 00 20 00 00[ 	]*vpmaxub 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de 72 80[ 	]*vpmaxub -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 de b2 c0 df ff ff[ 	]*vpmaxub -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e f4[ 	]*vpmaxuw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 3e f4[ 	]*vpmaxuw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 3e f4[ 	]*vpmaxuw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e 31[ 	]*vpmaxuw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e 72 7f[ 	]*vpmaxuw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e b2 00 20 00 00[ 	]*vpmaxuw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e 72 80[ 	]*vpmaxuw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3e b2 c0 df ff ff[ 	]*vpmaxuw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 f4[ 	]*vpminsb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 38 f4[ 	]*vpminsb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 38 f4[ 	]*vpminsb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 31[ 	]*vpminsb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 b4 f4 c0 1d fe ff[ 	]*vpminsb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 72 7f[ 	]*vpminsb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 b2 00 20 00 00[ 	]*vpminsb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 72 80[ 	]*vpminsb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 38 b2 c0 df ff ff[ 	]*vpminsb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea f4[ 	]*vpminsw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f ea f4[ 	]*vpminsw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf ea f4[ 	]*vpminsw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea 31[ 	]*vpminsw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea b4 f4 c0 1d fe ff[ 	]*vpminsw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea 72 7f[ 	]*vpminsw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea b2 00 20 00 00[ 	]*vpminsw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea 72 80[ 	]*vpminsw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 ea b2 c0 df ff ff[ 	]*vpminsw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da f4[ 	]*vpminub %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f da f4[ 	]*vpminub %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf da f4[ 	]*vpminub %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da 31[ 	]*vpminub \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da b4 f4 c0 1d fe ff[ 	]*vpminub -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da 72 7f[ 	]*vpminub 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da b2 00 20 00 00[ 	]*vpminub 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da 72 80[ 	]*vpminub -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 da b2 c0 df ff ff[ 	]*vpminub -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a f4[ 	]*vpminuw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 3a f4[ 	]*vpminuw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 3a f4[ 	]*vpminuw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a 31[ 	]*vpminuw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a b4 f4 c0 1d fe ff[ 	]*vpminuw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a 72 7f[ 	]*vpminuw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a b2 00 20 00 00[ 	]*vpminuw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a 72 80[ 	]*vpminuw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 3a b2 c0 df ff ff[ 	]*vpminuw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 f5[ 	]*vpmovsxbw %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 20 f5[ 	]*vpmovsxbw %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 31[ 	]*vpmovsxbw \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 72 7f[ 	]*vpmovsxbw 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 b2 00 10 00 00[ 	]*vpmovsxbw 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 72 80[ 	]*vpmovsxbw -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 20 b2 e0 ef ff ff[ 	]*vpmovsxbw -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 f5[ 	]*vpmovzxbw %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 30 f5[ 	]*vpmovzxbw %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 31[ 	]*vpmovzxbw \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 72 7f[ 	]*vpmovzxbw 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 b2 00 10 00 00[ 	]*vpmovzxbw 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 72 80[ 	]*vpmovzxbw -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 30 b2 e0 ef ff ff[ 	]*vpmovzxbw -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b f4[ 	]*vpmulhrsw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 0b f4[ 	]*vpmulhrsw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 0b f4[ 	]*vpmulhrsw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b 31[ 	]*vpmulhrsw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b 72 7f[ 	]*vpmulhrsw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b b2 00 20 00 00[ 	]*vpmulhrsw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b 72 80[ 	]*vpmulhrsw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 0b b2 c0 df ff ff[ 	]*vpmulhrsw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 f4[ 	]*vpmulhuw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e4 f4[ 	]*vpmulhuw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e4 f4[ 	]*vpmulhuw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 31[ 	]*vpmulhuw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 72 7f[ 	]*vpmulhuw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 b2 00 20 00 00[ 	]*vpmulhuw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 72 80[ 	]*vpmulhuw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e4 b2 c0 df ff ff[ 	]*vpmulhuw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 f4[ 	]*vpmulhw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e5 f4[ 	]*vpmulhw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e5 f4[ 	]*vpmulhw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 31[ 	]*vpmulhw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 72 7f[ 	]*vpmulhw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 b2 00 20 00 00[ 	]*vpmulhw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 72 80[ 	]*vpmulhw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e5 b2 c0 df ff ff[ 	]*vpmulhw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 f4[ 	]*vpmullw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d5 f4[ 	]*vpmullw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf d5 f4[ 	]*vpmullw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 31[ 	]*vpmullw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 b4 f4 c0 1d fe ff[ 	]*vpmullw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 72 7f[ 	]*vpmullw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 b2 00 20 00 00[ 	]*vpmullw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 72 80[ 	]*vpmullw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d5 b2 c0 df ff ff[ 	]*vpmullw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 f4[ 	]*vpsadbw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 31[ 	]*vpsadbw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 b4 f4 c0 1d fe ff[ 	]*vpsadbw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 72 7f[ 	]*vpsadbw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 b2 00 20 00 00[ 	]*vpsadbw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 72 80[ 	]*vpsadbw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f6 b2 c0 df ff ff[ 	]*vpsadbw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 f4[ 	]*vpshufb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 00 f4[ 	]*vpshufb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 00 f4[ 	]*vpshufb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 31[ 	]*vpshufb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 b4 f4 c0 1d fe ff[ 	]*vpshufb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 72 7f[ 	]*vpshufb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 b2 00 20 00 00[ 	]*vpshufb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 72 80[ 	]*vpshufb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 00 b2 c0 df ff ff[ 	]*vpshufb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 f5 ab[ 	]*vpshufhw \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 4f 70 f5 ab[ 	]*vpshufhw \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe cf 70 f5 ab[ 	]*vpshufhw \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 f5 7b[ 	]*vpshufhw \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 31 7b[ 	]*vpshufhw \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 72 7f 7b[ 	]*vpshufhw \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 b2 00 20 00 00 7b[ 	]*vpshufhw \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 72 80 7b[ 	]*vpshufhw \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 70 b2 c0 df ff ff 7b[ 	]*vpshufhw \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 f5 ab[ 	]*vpshuflw \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 70 f5 ab[ 	]*vpshuflw \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 70 f5 ab[ 	]*vpshuflw \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 f5 7b[ 	]*vpshuflw \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 31 7b[ 	]*vpshuflw \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 72 7f 7b[ 	]*vpshuflw \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 b2 00 20 00 00 7b[ 	]*vpshuflw \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 72 80 7b[ 	]*vpshuflw \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 70 b2 c0 df ff ff 7b[ 	]*vpshuflw \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 f4[ 	]*vpsllw %xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf f1 f4[ 	]*vpsllw %xmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 31[ 	]*vpsllw \(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 72 7f[ 	]*vpsllw 0x7f0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 b2 00 08 00 00[ 	]*vpsllw 0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 72 80[ 	]*vpsllw -0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f1 b2 f0 f7 ff ff[ 	]*vpsllw -0x810\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 f4[ 	]*vpsraw %xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e1 f4[ 	]*vpsraw %xmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 31[ 	]*vpsraw \(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 72 7f[ 	]*vpsraw 0x7f0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 b2 00 08 00 00[ 	]*vpsraw 0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 72 80[ 	]*vpsraw -0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e1 b2 f0 f7 ff ff[ 	]*vpsraw -0x810\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 f4[ 	]*vpsrlw %xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf d1 f4[ 	]*vpsrlw %xmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 31[ 	]*vpsrlw \(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 72 7f[ 	]*vpsrlw 0x7f0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 b2 00 08 00 00[ 	]*vpsrlw 0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 72 80[ 	]*vpsrlw -0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d1 b2 f0 f7 ff ff[ 	]*vpsrlw -0x810\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 dd ab[ 	]*vpsrldq \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 dd 7b[ 	]*vpsrldq \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 19 7b[ 	]*vpsrldq \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 9c f4 c0 1d fe ff 7b[ 	]*vpsrldq \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 5a 7f 7b[ 	]*vpsrldq \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 9a 00 20 00 00 7b[ 	]*vpsrldq \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 5a 80 7b[ 	]*vpsrldq \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 9a c0 df ff ff 7b[ 	]*vpsrldq \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 d5 ab[ 	]*vpsrlw \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 71 d5 ab[ 	]*vpsrlw \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd cf 71 d5 ab[ 	]*vpsrlw \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 d5 7b[ 	]*vpsrlw \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 11 7b[ 	]*vpsrlw \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 52 7f 7b[ 	]*vpsrlw \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 92 00 20 00 00 7b[ 	]*vpsrlw \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 52 80 7b[ 	]*vpsrlw \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 92 c0 df ff ff 7b[ 	]*vpsrlw \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 e5 ab[ 	]*vpsraw \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 71 e5 ab[ 	]*vpsraw \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd cf 71 e5 ab[ 	]*vpsraw \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 e5 7b[ 	]*vpsraw \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 21 7b[ 	]*vpsraw \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 62 7f 7b[ 	]*vpsraw \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 a2 00 20 00 00 7b[ 	]*vpsraw \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 62 80 7b[ 	]*vpsraw \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 a2 c0 df ff ff 7b[ 	]*vpsraw \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 f4[ 	]*vpsubb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f8 f4[ 	]*vpsubb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf f8 f4[ 	]*vpsubb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 31[ 	]*vpsubb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 b4 f4 c0 1d fe ff[ 	]*vpsubb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 72 7f[ 	]*vpsubb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 b2 00 20 00 00[ 	]*vpsubb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 72 80[ 	]*vpsubb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f8 b2 c0 df ff ff[ 	]*vpsubb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 f4[ 	]*vpsubsb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e8 f4[ 	]*vpsubsb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e8 f4[ 	]*vpsubsb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 31[ 	]*vpsubsb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 72 7f[ 	]*vpsubsb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 b2 00 20 00 00[ 	]*vpsubsb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 72 80[ 	]*vpsubsb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e8 b2 c0 df ff ff[ 	]*vpsubsb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 f4[ 	]*vpsubsw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f e9 f4[ 	]*vpsubsw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf e9 f4[ 	]*vpsubsw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 31[ 	]*vpsubsw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 72 7f[ 	]*vpsubsw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 b2 00 20 00 00[ 	]*vpsubsw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 72 80[ 	]*vpsubsw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 e9 b2 c0 df ff ff[ 	]*vpsubsw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 f4[ 	]*vpsubusb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d8 f4[ 	]*vpsubusb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf d8 f4[ 	]*vpsubusb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 31[ 	]*vpsubusb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 72 7f[ 	]*vpsubusb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 b2 00 20 00 00[ 	]*vpsubusb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 72 80[ 	]*vpsubusb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d8 b2 c0 df ff ff[ 	]*vpsubusb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 f4[ 	]*vpsubusw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f d9 f4[ 	]*vpsubusw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf d9 f4[ 	]*vpsubusw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 31[ 	]*vpsubusw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 72 7f[ 	]*vpsubusw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 b2 00 20 00 00[ 	]*vpsubusw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 72 80[ 	]*vpsubusw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 d9 b2 c0 df ff ff[ 	]*vpsubusw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 f4[ 	]*vpsubw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f f9 f4[ 	]*vpsubw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf f9 f4[ 	]*vpsubw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 31[ 	]*vpsubw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 b4 f4 c0 1d fe ff[ 	]*vpsubw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 72 7f[ 	]*vpsubw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 b2 00 20 00 00[ 	]*vpsubw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 72 80[ 	]*vpsubw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 f9 b2 c0 df ff ff[ 	]*vpsubw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 f4[ 	]*vpunpckhbw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 68 f4[ 	]*vpunpckhbw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 68 f4[ 	]*vpunpckhbw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 31[ 	]*vpunpckhbw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 72 7f[ 	]*vpunpckhbw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 b2 00 20 00 00[ 	]*vpunpckhbw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 72 80[ 	]*vpunpckhbw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 68 b2 c0 df ff ff[ 	]*vpunpckhbw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 f4[ 	]*vpunpckhwd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 69 f4[ 	]*vpunpckhwd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 69 f4[ 	]*vpunpckhwd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 31[ 	]*vpunpckhwd \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 72 7f[ 	]*vpunpckhwd 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 b2 00 20 00 00[ 	]*vpunpckhwd 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 72 80[ 	]*vpunpckhwd -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 69 b2 c0 df ff ff[ 	]*vpunpckhwd -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 f4[ 	]*vpunpcklbw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 60 f4[ 	]*vpunpcklbw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 60 f4[ 	]*vpunpcklbw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 31[ 	]*vpunpcklbw \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 72 7f[ 	]*vpunpcklbw 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 b2 00 20 00 00[ 	]*vpunpcklbw 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 72 80[ 	]*vpunpcklbw -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 60 b2 c0 df ff ff[ 	]*vpunpcklbw -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 f4[ 	]*vpunpcklwd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 61 f4[ 	]*vpunpcklwd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 61 f4[ 	]*vpunpcklwd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 31[ 	]*vpunpcklwd \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 72 7f[ 	]*vpunpcklwd 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 b2 00 20 00 00[ 	]*vpunpcklwd 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 72 80[ 	]*vpunpcklwd -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 61 b2 c0 df ff ff[ 	]*vpunpcklwd -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 fd ab[ 	]*vpslldq \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 fd 7b[ 	]*vpslldq \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 39 7b[ 	]*vpslldq \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 bc f4 c0 1d fe ff 7b[ 	]*vpslldq \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 7a 7f 7b[ 	]*vpslldq \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 ba 00 20 00 00 7b[ 	]*vpslldq \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 7a 80 7b[ 	]*vpslldq \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 73 ba c0 df ff ff 7b[ 	]*vpslldq \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 f5 ab[ 	]*vpsllw \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 4f 71 f5 ab[ 	]*vpsllw \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd cf 71 f5 ab[ 	]*vpsllw \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 f5 7b[ 	]*vpsllw \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 31 7b[ 	]*vpsllw \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 72 7f 7b[ 	]*vpsllw \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 b2 00 20 00 00 7b[ 	]*vpsllw \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 72 80 7b[ 	]*vpsllw \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 48 71 b2 c0 df ff ff 7b[ 	]*vpsllw \$0x7b,-0x2040\(%edx\),%zmm6
#pass
