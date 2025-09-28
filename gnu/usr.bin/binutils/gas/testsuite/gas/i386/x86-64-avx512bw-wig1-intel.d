#as: -mevexwig=1
#objdump: -dw -Mintel
#name: x86_64 AVX512BW wig insns (Intel disassembly)
#source: x86-64-avx512bw-wig.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 1c f5[ 	]*vpabsb zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 1c f5[ 	]*vpabsb zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 1c f5[ 	]*vpabsb zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c 31[ 	]*vpabsb zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 1c b4 f0 23 01 00 00[ 	]*vpabsb zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c 72 7f[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c b2 00 20 00 00[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c 72 80[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c b2 c0 df ff ff[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 1d f5[ 	]*vpabsw zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 1d f5[ 	]*vpabsw zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 1d f5[ 	]*vpabsw zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d 31[ 	]*vpabsw zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 1d b4 f0 23 01 00 00[ 	]*vpabsw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d 72 7f[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d b2 00 20 00 00[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d 72 80[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d b2 c0 df ff ff[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 63 f4[ 	]*vpacksswb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 63 f4[ 	]*vpacksswb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 63 f4[ 	]*vpacksswb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 31[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 63 b4 f0 23 01 00 00[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 72 7f[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 b2 00 20 00 00[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 72 80[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 b2 c0 df ff ff[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 67 f4[ 	]*vpackuswb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 67 f4[ 	]*vpackuswb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 67 f4[ 	]*vpackuswb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 31[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 67 b4 f0 23 01 00 00[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 72 7f[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 b2 00 20 00 00[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 72 80[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 b2 c0 df ff ff[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 fc f4[ 	]*vpaddb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 fc f4[ 	]*vpaddb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 fc f4[ 	]*vpaddb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc 31[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 fc b4 f0 23 01 00 00[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc 72 7f[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc b2 00 20 00 00[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc 72 80[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc b2 c0 df ff ff[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ec f4[ 	]*vpaddsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ec f4[ 	]*vpaddsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ec f4[ 	]*vpaddsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec 31[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ec b4 f0 23 01 00 00[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec 72 7f[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec b2 00 20 00 00[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec 72 80[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec b2 c0 df ff ff[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ed f4[ 	]*vpaddsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ed f4[ 	]*vpaddsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ed f4[ 	]*vpaddsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed 31[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ed b4 f0 23 01 00 00[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed 72 7f[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed b2 00 20 00 00[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed 72 80[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed b2 c0 df ff ff[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 dc f4[ 	]*vpaddusb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 dc f4[ 	]*vpaddusb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 dc f4[ 	]*vpaddusb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc 31[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 dc b4 f0 23 01 00 00[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc 72 7f[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc b2 00 20 00 00[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc 72 80[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc b2 c0 df ff ff[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 dd f4[ 	]*vpaddusw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 dd f4[ 	]*vpaddusw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 dd f4[ 	]*vpaddusw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd 31[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 dd b4 f0 23 01 00 00[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd 72 7f[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd b2 00 20 00 00[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd 72 80[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd b2 c0 df ff ff[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 fd f4[ 	]*vpaddw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 fd f4[ 	]*vpaddw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 fd f4[ 	]*vpaddw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd 31[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 fd b4 f0 23 01 00 00[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd 72 7f[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd b2 00 20 00 00[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd 72 80[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd b2 c0 df ff ff[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 0f f4 ab[ 	]*vpalignr zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 0f f4 ab[ 	]*vpalignr zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 0f f4 ab[ 	]*vpalignr zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 0f f4 7b[ 	]*vpalignr zmm30,zmm29,zmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f 31 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 0f b4 f0 23 01 00 00 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f 72 7f 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f b2 00 20 00 00 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f 72 80 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f b2 c0 df ff ff 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e0 f4[ 	]*vpavgb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e0 f4[ 	]*vpavgb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e0 f4[ 	]*vpavgb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 31[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e0 b4 f0 23 01 00 00[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 72 7f[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 b2 00 20 00 00[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 72 80[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 b2 c0 df ff ff[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e3 f4[ 	]*vpavgw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e3 f4[ 	]*vpavgw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e3 f4[ 	]*vpavgw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 31[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e3 b4 f0 23 01 00 00[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 72 7f[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 b2 00 20 00 00[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 72 80[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 b2 c0 df ff ff[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 74 ed[ 	]*vpcmpeqb k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 74 ed[ 	]*vpcmpeqb k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 29[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 74 ac f0 23 01 00 00[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 6a 7f[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 aa 00 20 00 00[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 6a 80[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 aa c0 df ff ff[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 75 ed[ 	]*vpcmpeqw k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 75 ed[ 	]*vpcmpeqw k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 29[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 75 ac f0 23 01 00 00[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 6a 7f[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 aa 00 20 00 00[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 6a 80[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 aa c0 df ff ff[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 64 ed[ 	]*vpcmpgtb k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 64 ed[ 	]*vpcmpgtb k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 29[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 64 ac f0 23 01 00 00[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 6a 7f[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 aa 00 20 00 00[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 6a 80[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 aa c0 df ff ff[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 65 ed[ 	]*vpcmpgtw k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 65 ed[ 	]*vpcmpgtw k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 29[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 65 ac f0 23 01 00 00[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 6a 7f[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 aa 00 20 00 00[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 6a 80[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 aa c0 df ff ff[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 e8 ab[ 	]*vpextrb eax,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 e8 7b[ 	]*vpextrb eax,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 fd 08 14 e8 7b[ 	]*vpextrb r8d,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 29 7b[ 	]*vpextrb BYTE PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 14 ac f0 23 01 00 00 7b[ 	]*vpextrb BYTE PTR \[rax\+r14\*8\+0x123\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 6a 7f 7b[ 	]*vpextrb BYTE PTR \[rdx\+0x7f\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 aa 80 00 00 00 7b[ 	]*vpextrb BYTE PTR \[rdx\+0x80\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 6a 80 7b[ 	]*vpextrb BYTE PTR \[rdx-0x80\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 aa 7f ff ff ff 7b[ 	]*vpextrb BYTE PTR \[rdx-0x81\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 29 7b[ 	]*vpextrw WORD PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 15 ac f0 23 01 00 00 7b[ 	]*vpextrw WORD PTR \[rax\+r14\*8\+0x123\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 6a 7f 7b[ 	]*vpextrw WORD PTR \[rdx\+0xfe\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 aa 00 01 00 00 7b[ 	]*vpextrw WORD PTR \[rdx\+0x100\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 6a 80 7b[ 	]*vpextrw WORD PTR \[rdx-0x100\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 aa fe fe ff ff 7b[ 	]*vpextrw WORD PTR \[rdx-0x102\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 fd 08 c5 c6 ab[ 	]*vpextrw eax,xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 fd 08 c5 c6 7b[ 	]*vpextrw eax,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 11 fd 08 c5 c6 7b[ 	]*vpextrw r8d,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 f0 ab[ 	]*vpinsrb xmm30,xmm29,eax,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 f0 7b[ 	]*vpinsrb xmm30,xmm29,eax,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 f5 7b[ 	]*vpinsrb xmm30,xmm29,ebp,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 95 00 20 f5 7b[ 	]*vpinsrb xmm30,xmm29,r13d,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 31 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 20 b4 f0 23 01 00 00 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 72 7f 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx\+0x7f\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 b2 80 00 00 00 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 72 80 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx-0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 b2 7f ff ff ff 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx-0x81\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 f0 ab[ 	]*vpinsrw xmm30,xmm29,eax,0xab
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 f0 7b[ 	]*vpinsrw xmm30,xmm29,eax,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 f5 7b[ 	]*vpinsrw xmm30,xmm29,ebp,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 41 95 00 c4 f5 7b[ 	]*vpinsrw xmm30,xmm29,r13d,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 31 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 c4 b4 f0 23 01 00 00 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 72 7f 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 b2 00 01 00 00 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx\+0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 72 80 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 b2 fe fe ff ff 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx-0x102\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 04 f4[ 	]*vpmaddubsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 04 f4[ 	]*vpmaddubsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 04 f4[ 	]*vpmaddubsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 31[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 04 b4 f0 23 01 00 00[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 72 7f[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 b2 00 20 00 00[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 72 80[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 b2 c0 df ff ff[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f5 f4[ 	]*vpmaddwd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f5 f4[ 	]*vpmaddwd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f5 f4[ 	]*vpmaddwd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 31[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f5 b4 f0 23 01 00 00[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 72 7f[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 b2 00 20 00 00[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 72 80[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 b2 c0 df ff ff[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 3c f4[ 	]*vpmaxsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 3c f4[ 	]*vpmaxsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 3c f4[ 	]*vpmaxsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c 31[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 3c b4 f0 23 01 00 00[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c 72 7f[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c b2 00 20 00 00[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c 72 80[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c b2 c0 df ff ff[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ee f4[ 	]*vpmaxsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ee f4[ 	]*vpmaxsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ee f4[ 	]*vpmaxsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee 31[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ee b4 f0 23 01 00 00[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee 72 7f[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee b2 00 20 00 00[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee 72 80[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee b2 c0 df ff ff[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 de f4[ 	]*vpmaxub zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 de f4[ 	]*vpmaxub zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 de f4[ 	]*vpmaxub zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de 31[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 de b4 f0 23 01 00 00[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de 72 7f[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de b2 00 20 00 00[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de 72 80[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de b2 c0 df ff ff[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 3e f4[ 	]*vpmaxuw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 3e f4[ 	]*vpmaxuw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 3e f4[ 	]*vpmaxuw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e 31[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 3e b4 f0 23 01 00 00[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e 72 7f[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e b2 00 20 00 00[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e 72 80[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e b2 c0 df ff ff[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 38 f4[ 	]*vpminsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 38 f4[ 	]*vpminsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 38 f4[ 	]*vpminsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 31[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 38 b4 f0 23 01 00 00[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 72 7f[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 b2 00 20 00 00[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 72 80[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 b2 c0 df ff ff[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ea f4[ 	]*vpminsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ea f4[ 	]*vpminsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ea f4[ 	]*vpminsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea 31[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ea b4 f0 23 01 00 00[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea 72 7f[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea b2 00 20 00 00[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea 72 80[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea b2 c0 df ff ff[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 da f4[ 	]*vpminub zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 da f4[ 	]*vpminub zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 da f4[ 	]*vpminub zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da 31[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 da b4 f0 23 01 00 00[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da 72 7f[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da b2 00 20 00 00[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da 72 80[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da b2 c0 df ff ff[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 3a f4[ 	]*vpminuw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 3a f4[ 	]*vpminuw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 3a f4[ 	]*vpminuw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a 31[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 3a b4 f0 23 01 00 00[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a 72 7f[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a b2 00 20 00 00[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a 72 80[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a b2 c0 df ff ff[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 20 f5[ 	]*vpmovsxbw zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 20 f5[ 	]*vpmovsxbw zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 20 f5[ 	]*vpmovsxbw zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 31[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 20 b4 f0 23 01 00 00[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 72 7f[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 b2 00 10 00 00[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 72 80[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 b2 e0 ef ff ff[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 30 f5[ 	]*vpmovzxbw zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 30 f5[ 	]*vpmovzxbw zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 30 f5[ 	]*vpmovzxbw zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 31[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 30 b4 f0 23 01 00 00[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 72 7f[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 b2 00 10 00 00[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 72 80[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 b2 e0 ef ff ff[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 0b f4[ 	]*vpmulhrsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 0b f4[ 	]*vpmulhrsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 0b f4[ 	]*vpmulhrsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b 31[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 0b b4 f0 23 01 00 00[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b 72 7f[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b b2 00 20 00 00[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b 72 80[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b b2 c0 df ff ff[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e4 f4[ 	]*vpmulhuw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e4 f4[ 	]*vpmulhuw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e4 f4[ 	]*vpmulhuw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 31[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e4 b4 f0 23 01 00 00[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 72 7f[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 b2 00 20 00 00[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 72 80[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 b2 c0 df ff ff[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e5 f4[ 	]*vpmulhw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e5 f4[ 	]*vpmulhw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e5 f4[ 	]*vpmulhw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 31[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e5 b4 f0 23 01 00 00[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 72 7f[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 b2 00 20 00 00[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 72 80[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 b2 c0 df ff ff[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d5 f4[ 	]*vpmullw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d5 f4[ 	]*vpmullw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d5 f4[ 	]*vpmullw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 31[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d5 b4 f0 23 01 00 00[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 72 7f[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 b2 00 20 00 00[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 72 80[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 b2 c0 df ff ff[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f6 f4[ 	]*vpsadbw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 31[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f6 b4 f0 23 01 00 00[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 72 7f[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 b2 00 20 00 00[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 72 80[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 b2 c0 df ff ff[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 00 f4[ 	]*vpshufb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 00 f4[ 	]*vpshufb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 00 f4[ 	]*vpshufb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 31[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 00 b4 f0 23 01 00 00[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 72 7f[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 b2 00 20 00 00[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 72 80[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 b2 c0 df ff ff[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 70 f5 ab[ 	]*vpshufhw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 4f 70 f5 ab[ 	]*vpshufhw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 fe cf 70 f5 ab[ 	]*vpshufhw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 70 f5 7b[ 	]*vpshufhw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 31 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 48 70 b4 f0 23 01 00 00 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 72 7f 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 b2 00 20 00 00 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 72 80 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 b2 c0 df ff ff 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 70 f5 ab[ 	]*vpshuflw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 4f 70 f5 ab[ 	]*vpshuflw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 ff cf 70 f5 ab[ 	]*vpshuflw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 70 f5 7b[ 	]*vpshuflw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 31 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 48 70 b4 f0 23 01 00 00 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 72 7f 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 b2 00 20 00 00 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 72 80 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 b2 c0 df ff ff 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f1 f4[ 	]*vpsllw zmm30,zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f1 f4[ 	]*vpsllw zmm30\{k7\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f1 f4[ 	]*vpsllw zmm30\{k7\}\{z\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 31[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f1 b4 f0 23 01 00 00[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 72 7f[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 b2 00 08 00 00[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 72 80[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 b2 f0 f7 ff ff[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e1 f4[ 	]*vpsraw zmm30,zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e1 f4[ 	]*vpsraw zmm30\{k7\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e1 f4[ 	]*vpsraw zmm30\{k7\}\{z\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 31[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e1 b4 f0 23 01 00 00[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 72 7f[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 b2 00 08 00 00[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 72 80[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 b2 f0 f7 ff ff[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d1 f4[ 	]*vpsrlw zmm30,zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d1 f4[ 	]*vpsrlw zmm30\{k7\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d1 f4[ 	]*vpsrlw zmm30\{k7\}\{z\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 31[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d1 b4 f0 23 01 00 00[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 72 7f[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 b2 00 08 00 00[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 72 80[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 b2 f0 f7 ff ff[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 dd ab[ 	]*vpsrldq zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 dd 7b[ 	]*vpsrldq zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 19 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 73 9c f0 23 01 00 00 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 5a 7f 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 9a 00 20 00 00 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 5a 80 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 9a c0 df ff ff 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 d5 ab[ 	]*vpsrlw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 71 d5 ab[ 	]*vpsrlw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d c7 71 d5 ab[ 	]*vpsrlw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 d5 7b[ 	]*vpsrlw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 11 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 71 94 f0 23 01 00 00 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 52 7f 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 92 00 20 00 00 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 52 80 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 92 c0 df ff ff 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 e5 ab[ 	]*vpsraw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 71 e5 ab[ 	]*vpsraw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d c7 71 e5 ab[ 	]*vpsraw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 e5 7b[ 	]*vpsraw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 21 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 71 a4 f0 23 01 00 00 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 62 7f 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 a2 00 20 00 00 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 62 80 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 a2 c0 df ff ff 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f8 f4[ 	]*vpsubb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f8 f4[ 	]*vpsubb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f8 f4[ 	]*vpsubb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 31[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f8 b4 f0 23 01 00 00[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 72 7f[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 b2 00 20 00 00[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 72 80[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 b2 c0 df ff ff[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e8 f4[ 	]*vpsubsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e8 f4[ 	]*vpsubsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e8 f4[ 	]*vpsubsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 31[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e8 b4 f0 23 01 00 00[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 72 7f[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 b2 00 20 00 00[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 72 80[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 b2 c0 df ff ff[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e9 f4[ 	]*vpsubsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e9 f4[ 	]*vpsubsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e9 f4[ 	]*vpsubsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 31[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e9 b4 f0 23 01 00 00[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 72 7f[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 b2 00 20 00 00[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 72 80[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 b2 c0 df ff ff[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d8 f4[ 	]*vpsubusb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d8 f4[ 	]*vpsubusb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d8 f4[ 	]*vpsubusb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 31[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d8 b4 f0 23 01 00 00[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 72 7f[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 b2 00 20 00 00[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 72 80[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 b2 c0 df ff ff[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d9 f4[ 	]*vpsubusw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d9 f4[ 	]*vpsubusw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d9 f4[ 	]*vpsubusw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 31[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d9 b4 f0 23 01 00 00[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 72 7f[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 b2 00 20 00 00[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 72 80[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 b2 c0 df ff ff[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f9 f4[ 	]*vpsubw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f9 f4[ 	]*vpsubw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f9 f4[ 	]*vpsubw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 31[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f9 b4 f0 23 01 00 00[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 72 7f[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 b2 00 20 00 00[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 72 80[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 b2 c0 df ff ff[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 68 f4[ 	]*vpunpckhbw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 68 f4[ 	]*vpunpckhbw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 68 f4[ 	]*vpunpckhbw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 31[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 68 b4 f0 23 01 00 00[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 72 7f[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 b2 00 20 00 00[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 72 80[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 b2 c0 df ff ff[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 69 f4[ 	]*vpunpckhwd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 69 f4[ 	]*vpunpckhwd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 69 f4[ 	]*vpunpckhwd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 31[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 69 b4 f0 23 01 00 00[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 72 7f[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 b2 00 20 00 00[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 72 80[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 b2 c0 df ff ff[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 60 f4[ 	]*vpunpcklbw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 60 f4[ 	]*vpunpcklbw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 60 f4[ 	]*vpunpcklbw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 31[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 60 b4 f0 23 01 00 00[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 72 7f[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 b2 00 20 00 00[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 72 80[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 b2 c0 df ff ff[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 61 f4[ 	]*vpunpcklwd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 61 f4[ 	]*vpunpcklwd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 61 f4[ 	]*vpunpcklwd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 31[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 61 b4 f0 23 01 00 00[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 72 7f[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 b2 00 20 00 00[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 72 80[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 b2 c0 df ff ff[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 fd ab[ 	]*vpslldq zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 fd 7b[ 	]*vpslldq zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 39 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 73 bc f0 23 01 00 00 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 7a 7f 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 ba 00 20 00 00 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 7a 80 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 ba c0 df ff ff 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 f5 ab[ 	]*vpsllw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 71 f5 ab[ 	]*vpsllw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d c7 71 f5 ab[ 	]*vpsllw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 f5 7b[ 	]*vpsllw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 31 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 71 b4 f0 23 01 00 00 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 72 7f 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 b2 00 20 00 00 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 72 80 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 b2 c0 df ff ff 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 1c f5[ 	]*vpabsb zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 1c f5[ 	]*vpabsb zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 1c f5[ 	]*vpabsb zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c 31[ 	]*vpabsb zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 1c b4 f0 34 12 00 00[ 	]*vpabsb zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c 72 7f[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c b2 00 20 00 00[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c 72 80[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1c b2 c0 df ff ff[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 1d f5[ 	]*vpabsw zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 1d f5[ 	]*vpabsw zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 1d f5[ 	]*vpabsw zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d 31[ 	]*vpabsw zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 1d b4 f0 34 12 00 00[ 	]*vpabsw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d 72 7f[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d b2 00 20 00 00[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d 72 80[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1d b2 c0 df ff ff[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 63 f4[ 	]*vpacksswb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 63 f4[ 	]*vpacksswb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 63 f4[ 	]*vpacksswb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 31[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 63 b4 f0 34 12 00 00[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 72 7f[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 b2 00 20 00 00[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 72 80[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 63 b2 c0 df ff ff[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 67 f4[ 	]*vpackuswb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 67 f4[ 	]*vpackuswb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 67 f4[ 	]*vpackuswb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 31[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 67 b4 f0 34 12 00 00[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 72 7f[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 b2 00 20 00 00[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 72 80[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 67 b2 c0 df ff ff[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 fc f4[ 	]*vpaddb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 fc f4[ 	]*vpaddb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 fc f4[ 	]*vpaddb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc 31[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 fc b4 f0 34 12 00 00[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc 72 7f[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc b2 00 20 00 00[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc 72 80[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fc b2 c0 df ff ff[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ec f4[ 	]*vpaddsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ec f4[ 	]*vpaddsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ec f4[ 	]*vpaddsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec 31[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ec b4 f0 34 12 00 00[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec 72 7f[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec b2 00 20 00 00[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec 72 80[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ec b2 c0 df ff ff[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ed f4[ 	]*vpaddsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ed f4[ 	]*vpaddsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ed f4[ 	]*vpaddsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed 31[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ed b4 f0 34 12 00 00[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed 72 7f[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed b2 00 20 00 00[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed 72 80[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ed b2 c0 df ff ff[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 dc f4[ 	]*vpaddusb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 dc f4[ 	]*vpaddusb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 dc f4[ 	]*vpaddusb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc 31[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 dc b4 f0 34 12 00 00[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc 72 7f[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc b2 00 20 00 00[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc 72 80[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dc b2 c0 df ff ff[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 dd f4[ 	]*vpaddusw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 dd f4[ 	]*vpaddusw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 dd f4[ 	]*vpaddusw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd 31[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 dd b4 f0 34 12 00 00[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd 72 7f[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd b2 00 20 00 00[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd 72 80[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 dd b2 c0 df ff ff[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 fd f4[ 	]*vpaddw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 fd f4[ 	]*vpaddw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 fd f4[ 	]*vpaddw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd 31[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 fd b4 f0 34 12 00 00[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd 72 7f[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd b2 00 20 00 00[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd 72 80[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 fd b2 c0 df ff ff[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 0f f4 ab[ 	]*vpalignr zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 0f f4 ab[ 	]*vpalignr zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 0f f4 ab[ 	]*vpalignr zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 0f f4 7b[ 	]*vpalignr zmm30,zmm29,zmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f 31 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 0f b4 f0 34 12 00 00 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f 72 7f 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f b2 00 20 00 00 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f 72 80 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 0f b2 c0 df ff ff 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e0 f4[ 	]*vpavgb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e0 f4[ 	]*vpavgb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e0 f4[ 	]*vpavgb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 31[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e0 b4 f0 34 12 00 00[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 72 7f[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 b2 00 20 00 00[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 72 80[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e0 b2 c0 df ff ff[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e3 f4[ 	]*vpavgw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e3 f4[ 	]*vpavgw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e3 f4[ 	]*vpavgw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 31[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e3 b4 f0 34 12 00 00[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 72 7f[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 b2 00 20 00 00[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 72 80[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e3 b2 c0 df ff ff[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 74 ed[ 	]*vpcmpeqb k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 74 ed[ 	]*vpcmpeqb k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 29[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 74 ac f0 34 12 00 00[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 6a 7f[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 aa 00 20 00 00[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 6a 80[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 74 aa c0 df ff ff[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 75 ed[ 	]*vpcmpeqw k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 75 ed[ 	]*vpcmpeqw k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 29[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 75 ac f0 34 12 00 00[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 6a 7f[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 aa 00 20 00 00[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 6a 80[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 75 aa c0 df ff ff[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 64 ed[ 	]*vpcmpgtb k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 64 ed[ 	]*vpcmpgtb k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 29[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 64 ac f0 34 12 00 00[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 6a 7f[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 aa 00 20 00 00[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 6a 80[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 64 aa c0 df ff ff[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 65 ed[ 	]*vpcmpgtw k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 65 ed[ 	]*vpcmpgtw k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 29[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 65 ac f0 34 12 00 00[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 6a 7f[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 aa 00 20 00 00[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 6a 80[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 65 aa c0 df ff ff[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 e8 ab[ 	]*vpextrb eax,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 e8 7b[ 	]*vpextrb eax,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 fd 08 14 e8 7b[ 	]*vpextrb r8d,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 29 7b[ 	]*vpextrb BYTE PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 14 ac f0 34 12 00 00 7b[ 	]*vpextrb BYTE PTR \[rax\+r14\*8\+0x1234\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 6a 7f 7b[ 	]*vpextrb BYTE PTR \[rdx\+0x7f\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 aa 80 00 00 00 7b[ 	]*vpextrb BYTE PTR \[rdx\+0x80\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 6a 80 7b[ 	]*vpextrb BYTE PTR \[rdx-0x80\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 14 aa 7f ff ff ff 7b[ 	]*vpextrb BYTE PTR \[rdx-0x81\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 29 7b[ 	]*vpextrw WORD PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 15 ac f0 34 12 00 00 7b[ 	]*vpextrw WORD PTR \[rax\+r14\*8\+0x1234\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 6a 7f 7b[ 	]*vpextrw WORD PTR \[rdx\+0xfe\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 aa 00 01 00 00 7b[ 	]*vpextrw WORD PTR \[rdx\+0x100\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 6a 80 7b[ 	]*vpextrw WORD PTR \[rdx-0x100\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 15 aa fe fe ff ff 7b[ 	]*vpextrw WORD PTR \[rdx-0x102\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 fd 08 c5 c6 ab[ 	]*vpextrw eax,xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 fd 08 c5 c6 7b[ 	]*vpextrw eax,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 11 fd 08 c5 c6 7b[ 	]*vpextrw r8d,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 f0 ab[ 	]*vpinsrb xmm30,xmm29,eax,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 f0 7b[ 	]*vpinsrb xmm30,xmm29,eax,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 f5 7b[ 	]*vpinsrb xmm30,xmm29,ebp,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 95 00 20 f5 7b[ 	]*vpinsrb xmm30,xmm29,r13d,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 31 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 20 b4 f0 34 12 00 00 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 72 7f 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx\+0x7f\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 b2 80 00 00 00 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 72 80 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx-0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 20 b2 7f ff ff ff 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx-0x81\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 f0 ab[ 	]*vpinsrw xmm30,xmm29,eax,0xab
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 f0 7b[ 	]*vpinsrw xmm30,xmm29,eax,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 f5 7b[ 	]*vpinsrw xmm30,xmm29,ebp,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 41 95 00 c4 f5 7b[ 	]*vpinsrw xmm30,xmm29,r13d,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 31 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 c4 b4 f0 34 12 00 00 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 72 7f 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 b2 00 01 00 00 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx\+0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 72 80 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 c4 b2 fe fe ff ff 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx-0x102\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 04 f4[ 	]*vpmaddubsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 04 f4[ 	]*vpmaddubsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 04 f4[ 	]*vpmaddubsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 31[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 04 b4 f0 34 12 00 00[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 72 7f[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 b2 00 20 00 00[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 72 80[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 04 b2 c0 df ff ff[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f5 f4[ 	]*vpmaddwd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f5 f4[ 	]*vpmaddwd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f5 f4[ 	]*vpmaddwd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 31[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f5 b4 f0 34 12 00 00[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 72 7f[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 b2 00 20 00 00[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 72 80[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f5 b2 c0 df ff ff[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 3c f4[ 	]*vpmaxsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 3c f4[ 	]*vpmaxsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 3c f4[ 	]*vpmaxsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c 31[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 3c b4 f0 34 12 00 00[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c 72 7f[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c b2 00 20 00 00[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c 72 80[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3c b2 c0 df ff ff[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ee f4[ 	]*vpmaxsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ee f4[ 	]*vpmaxsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ee f4[ 	]*vpmaxsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee 31[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ee b4 f0 34 12 00 00[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee 72 7f[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee b2 00 20 00 00[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee 72 80[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ee b2 c0 df ff ff[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 de f4[ 	]*vpmaxub zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 de f4[ 	]*vpmaxub zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 de f4[ 	]*vpmaxub zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de 31[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 de b4 f0 34 12 00 00[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de 72 7f[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de b2 00 20 00 00[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de 72 80[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 de b2 c0 df ff ff[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 3e f4[ 	]*vpmaxuw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 3e f4[ 	]*vpmaxuw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 3e f4[ 	]*vpmaxuw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e 31[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 3e b4 f0 34 12 00 00[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e 72 7f[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e b2 00 20 00 00[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e 72 80[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3e b2 c0 df ff ff[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 38 f4[ 	]*vpminsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 38 f4[ 	]*vpminsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 38 f4[ 	]*vpminsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 31[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 38 b4 f0 34 12 00 00[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 72 7f[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 b2 00 20 00 00[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 72 80[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 38 b2 c0 df ff ff[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 ea f4[ 	]*vpminsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 ea f4[ 	]*vpminsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 ea f4[ 	]*vpminsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea 31[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 ea b4 f0 34 12 00 00[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea 72 7f[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea b2 00 20 00 00[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea 72 80[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 ea b2 c0 df ff ff[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 da f4[ 	]*vpminub zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 da f4[ 	]*vpminub zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 da f4[ 	]*vpminub zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da 31[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 da b4 f0 34 12 00 00[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da 72 7f[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da b2 00 20 00 00[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da 72 80[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 da b2 c0 df ff ff[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 3a f4[ 	]*vpminuw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 3a f4[ 	]*vpminuw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 3a f4[ 	]*vpminuw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a 31[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 3a b4 f0 34 12 00 00[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a 72 7f[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a b2 00 20 00 00[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a 72 80[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 3a b2 c0 df ff ff[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 20 f5[ 	]*vpmovsxbw zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 20 f5[ 	]*vpmovsxbw zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 20 f5[ 	]*vpmovsxbw zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 31[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 20 b4 f0 34 12 00 00[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 72 7f[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 b2 00 10 00 00[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 72 80[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 20 b2 e0 ef ff ff[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 30 f5[ 	]*vpmovzxbw zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 30 f5[ 	]*vpmovzxbw zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 30 f5[ 	]*vpmovzxbw zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 31[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 30 b4 f0 34 12 00 00[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 72 7f[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 b2 00 10 00 00[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 72 80[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 30 b2 e0 ef ff ff[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 0b f4[ 	]*vpmulhrsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 0b f4[ 	]*vpmulhrsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 0b f4[ 	]*vpmulhrsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b 31[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 0b b4 f0 34 12 00 00[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b 72 7f[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b b2 00 20 00 00[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b 72 80[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 0b b2 c0 df ff ff[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e4 f4[ 	]*vpmulhuw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e4 f4[ 	]*vpmulhuw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e4 f4[ 	]*vpmulhuw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 31[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e4 b4 f0 34 12 00 00[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 72 7f[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 b2 00 20 00 00[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 72 80[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e4 b2 c0 df ff ff[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e5 f4[ 	]*vpmulhw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e5 f4[ 	]*vpmulhw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e5 f4[ 	]*vpmulhw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 31[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e5 b4 f0 34 12 00 00[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 72 7f[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 b2 00 20 00 00[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 72 80[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e5 b2 c0 df ff ff[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d5 f4[ 	]*vpmullw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d5 f4[ 	]*vpmullw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d5 f4[ 	]*vpmullw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 31[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d5 b4 f0 34 12 00 00[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 72 7f[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 b2 00 20 00 00[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 72 80[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d5 b2 c0 df ff ff[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f6 f4[ 	]*vpsadbw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 31[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f6 b4 f0 34 12 00 00[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 72 7f[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 b2 00 20 00 00[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 72 80[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f6 b2 c0 df ff ff[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 00 f4[ 	]*vpshufb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 00 f4[ 	]*vpshufb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 00 f4[ 	]*vpshufb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 31[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 00 b4 f0 34 12 00 00[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 72 7f[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 b2 00 20 00 00[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 72 80[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 00 b2 c0 df ff ff[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 70 f5 ab[ 	]*vpshufhw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 4f 70 f5 ab[ 	]*vpshufhw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 fe cf 70 f5 ab[ 	]*vpshufhw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 70 f5 7b[ 	]*vpshufhw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 31 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 48 70 b4 f0 34 12 00 00 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 72 7f 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 b2 00 20 00 00 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 72 80 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 70 b2 c0 df ff ff 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 70 f5 ab[ 	]*vpshuflw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 4f 70 f5 ab[ 	]*vpshuflw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 ff cf 70 f5 ab[ 	]*vpshuflw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 70 f5 7b[ 	]*vpshuflw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 31 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 48 70 b4 f0 34 12 00 00 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 72 7f 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 b2 00 20 00 00 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 72 80 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 70 b2 c0 df ff ff 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f1 f4[ 	]*vpsllw zmm30,zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f1 f4[ 	]*vpsllw zmm30\{k7\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f1 f4[ 	]*vpsllw zmm30\{k7\}\{z\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 31[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f1 b4 f0 34 12 00 00[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 72 7f[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 b2 00 08 00 00[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 72 80[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f1 b2 f0 f7 ff ff[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e1 f4[ 	]*vpsraw zmm30,zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e1 f4[ 	]*vpsraw zmm30\{k7\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e1 f4[ 	]*vpsraw zmm30\{k7\}\{z\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 31[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e1 b4 f0 34 12 00 00[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 72 7f[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 b2 00 08 00 00[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 72 80[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e1 b2 f0 f7 ff ff[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d1 f4[ 	]*vpsrlw zmm30,zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d1 f4[ 	]*vpsrlw zmm30\{k7\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d1 f4[ 	]*vpsrlw zmm30\{k7\}\{z\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 31[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d1 b4 f0 34 12 00 00[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 72 7f[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 b2 00 08 00 00[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 72 80[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d1 b2 f0 f7 ff ff[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 dd ab[ 	]*vpsrldq zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 dd 7b[ 	]*vpsrldq zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 19 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 73 9c f0 34 12 00 00 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 5a 7f 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 9a 00 20 00 00 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 5a 80 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 9a c0 df ff ff 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 d5 ab[ 	]*vpsrlw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 71 d5 ab[ 	]*vpsrlw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d c7 71 d5 ab[ 	]*vpsrlw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 d5 7b[ 	]*vpsrlw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 11 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 71 94 f0 34 12 00 00 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 52 7f 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 92 00 20 00 00 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 52 80 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 92 c0 df ff ff 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 e5 ab[ 	]*vpsraw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 71 e5 ab[ 	]*vpsraw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d c7 71 e5 ab[ 	]*vpsraw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 e5 7b[ 	]*vpsraw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 21 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 71 a4 f0 34 12 00 00 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 62 7f 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 a2 00 20 00 00 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 62 80 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 a2 c0 df ff ff 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f8 f4[ 	]*vpsubb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f8 f4[ 	]*vpsubb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f8 f4[ 	]*vpsubb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 31[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f8 b4 f0 34 12 00 00[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 72 7f[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 b2 00 20 00 00[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 72 80[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f8 b2 c0 df ff ff[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e8 f4[ 	]*vpsubsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e8 f4[ 	]*vpsubsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e8 f4[ 	]*vpsubsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 31[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e8 b4 f0 34 12 00 00[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 72 7f[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 b2 00 20 00 00[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 72 80[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e8 b2 c0 df ff ff[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 e9 f4[ 	]*vpsubsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 e9 f4[ 	]*vpsubsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 e9 f4[ 	]*vpsubsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 31[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 e9 b4 f0 34 12 00 00[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 72 7f[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 b2 00 20 00 00[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 72 80[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 e9 b2 c0 df ff ff[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d8 f4[ 	]*vpsubusb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d8 f4[ 	]*vpsubusb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d8 f4[ 	]*vpsubusb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 31[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d8 b4 f0 34 12 00 00[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 72 7f[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 b2 00 20 00 00[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 72 80[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d8 b2 c0 df ff ff[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 d9 f4[ 	]*vpsubusw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 d9 f4[ 	]*vpsubusw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 d9 f4[ 	]*vpsubusw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 31[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 d9 b4 f0 34 12 00 00[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 72 7f[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 b2 00 20 00 00[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 72 80[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 d9 b2 c0 df ff ff[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 f9 f4[ 	]*vpsubw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 f9 f4[ 	]*vpsubw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 f9 f4[ 	]*vpsubw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 31[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 f9 b4 f0 34 12 00 00[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 72 7f[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 b2 00 20 00 00[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 72 80[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 f9 b2 c0 df ff ff[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 68 f4[ 	]*vpunpckhbw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 68 f4[ 	]*vpunpckhbw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 68 f4[ 	]*vpunpckhbw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 31[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 68 b4 f0 34 12 00 00[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 72 7f[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 b2 00 20 00 00[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 72 80[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 68 b2 c0 df ff ff[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 69 f4[ 	]*vpunpckhwd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 69 f4[ 	]*vpunpckhwd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 69 f4[ 	]*vpunpckhwd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 31[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 69 b4 f0 34 12 00 00[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 72 7f[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 b2 00 20 00 00[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 72 80[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 69 b2 c0 df ff ff[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 60 f4[ 	]*vpunpcklbw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 60 f4[ 	]*vpunpcklbw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 60 f4[ 	]*vpunpcklbw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 31[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 60 b4 f0 34 12 00 00[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 72 7f[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 b2 00 20 00 00[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 72 80[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 60 b2 c0 df ff ff[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 61 f4[ 	]*vpunpcklwd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 61 f4[ 	]*vpunpcklwd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 61 f4[ 	]*vpunpcklwd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 31[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 61 b4 f0 34 12 00 00[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 72 7f[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 b2 00 20 00 00[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 72 80[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 61 b2 c0 df ff ff[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 fd ab[ 	]*vpslldq zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 73 fd 7b[ 	]*vpslldq zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 39 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 73 bc f0 34 12 00 00 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 7a 7f 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 ba 00 20 00 00 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 7a 80 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 73 ba c0 df ff ff 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 f5 ab[ 	]*vpsllw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 47 71 f5 ab[ 	]*vpsllw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d c7 71 f5 ab[ 	]*vpsllw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 40 71 f5 7b[ 	]*vpsllw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 31 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 8d 40 71 b4 f0 34 12 00 00 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 72 7f 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 b2 00 20 00 00 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 72 80 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 8d 40 71 b2 c0 df ff ff 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
#pass
