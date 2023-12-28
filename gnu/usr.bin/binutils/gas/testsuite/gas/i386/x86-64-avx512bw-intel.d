#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512BW insns (Intel disassembly)
#source: x86-64-avx512bw.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 1c f5[ 	]*vpabsb zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 1c f5[ 	]*vpabsb zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 1c f5[ 	]*vpabsb zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1c 31[ 	]*vpabsb zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 1c b4 f0 23 01 00 00[ 	]*vpabsb zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1c 72 7f[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1c b2 00 20 00 00[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1c 72 80[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1c b2 c0 df ff ff[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 1d f5[ 	]*vpabsw zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 1d f5[ 	]*vpabsw zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 1d f5[ 	]*vpabsw zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1d 31[ 	]*vpabsw zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 1d b4 f0 23 01 00 00[ 	]*vpabsw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1d 72 7f[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1d b2 00 20 00 00[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1d 72 80[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1d b2 c0 df ff ff[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 6b f4[ 	]*vpackssdw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 6b f4[ 	]*vpackssdw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 6b f4[ 	]*vpackssdw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 6b 31[ 	]*vpackssdw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 6b b4 f0 23 01 00 00[ 	]*vpackssdw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 50 6b 31[ 	]*vpackssdw zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 6b 72 7f[ 	]*vpackssdw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 6b b2 00 20 00 00[ 	]*vpackssdw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 6b 72 80[ 	]*vpackssdw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 6b b2 c0 df ff ff[ 	]*vpackssdw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 50 6b 72 7f[ 	]*vpackssdw zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 50 6b b2 00 02 00 00[ 	]*vpackssdw zmm30,zmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 50 6b 72 80[ 	]*vpackssdw zmm30,zmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 50 6b b2 fc fd ff ff[ 	]*vpackssdw zmm30,zmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 63 f4[ 	]*vpacksswb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 63 f4[ 	]*vpacksswb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 63 f4[ 	]*vpacksswb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 63 31[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 63 b4 f0 23 01 00 00[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 63 72 7f[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 63 b2 00 20 00 00[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 63 72 80[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 63 b2 c0 df ff ff[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 2b f4[ 	]*vpackusdw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 2b f4[ 	]*vpackusdw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 2b f4[ 	]*vpackusdw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 2b 31[ 	]*vpackusdw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 2b b4 f0 23 01 00 00[ 	]*vpackusdw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 2b 31[ 	]*vpackusdw zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 2b 72 7f[ 	]*vpackusdw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 2b b2 00 20 00 00[ 	]*vpackusdw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 2b 72 80[ 	]*vpackusdw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 2b b2 c0 df ff ff[ 	]*vpackusdw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 2b 72 7f[ 	]*vpackusdw zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 2b b2 00 02 00 00[ 	]*vpackusdw zmm30,zmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 2b 72 80[ 	]*vpackusdw zmm30,zmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 2b b2 fc fd ff ff[ 	]*vpackusdw zmm30,zmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 67 f4[ 	]*vpackuswb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 67 f4[ 	]*vpackuswb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 67 f4[ 	]*vpackuswb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 67 31[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 67 b4 f0 23 01 00 00[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 67 72 7f[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 67 b2 00 20 00 00[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 67 72 80[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 67 b2 c0 df ff ff[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 fc f4[ 	]*vpaddb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 fc f4[ 	]*vpaddb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 fc f4[ 	]*vpaddb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fc 31[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 fc b4 f0 23 01 00 00[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fc 72 7f[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fc b2 00 20 00 00[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fc 72 80[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fc b2 c0 df ff ff[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 ec f4[ 	]*vpaddsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 ec f4[ 	]*vpaddsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 ec f4[ 	]*vpaddsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ec 31[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 ec b4 f0 23 01 00 00[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ec 72 7f[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ec b2 00 20 00 00[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ec 72 80[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ec b2 c0 df ff ff[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 ed f4[ 	]*vpaddsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 ed f4[ 	]*vpaddsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 ed f4[ 	]*vpaddsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ed 31[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 ed b4 f0 23 01 00 00[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ed 72 7f[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ed b2 00 20 00 00[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ed 72 80[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ed b2 c0 df ff ff[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 dc f4[ 	]*vpaddusb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 dc f4[ 	]*vpaddusb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 dc f4[ 	]*vpaddusb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dc 31[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 dc b4 f0 23 01 00 00[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dc 72 7f[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dc b2 00 20 00 00[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dc 72 80[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dc b2 c0 df ff ff[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 dd f4[ 	]*vpaddusw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 dd f4[ 	]*vpaddusw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 dd f4[ 	]*vpaddusw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dd 31[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 dd b4 f0 23 01 00 00[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dd 72 7f[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dd b2 00 20 00 00[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dd 72 80[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dd b2 c0 df ff ff[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 fd f4[ 	]*vpaddw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 fd f4[ 	]*vpaddw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 fd f4[ 	]*vpaddw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fd 31[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 fd b4 f0 23 01 00 00[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fd 72 7f[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fd b2 00 20 00 00[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fd 72 80[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fd b2 c0 df ff ff[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 0f f4 ab[ 	]*vpalignr zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 0f f4 ab[ 	]*vpalignr zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 0f f4 ab[ 	]*vpalignr zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 0f f4 7b[ 	]*vpalignr zmm30,zmm29,zmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 0f 31 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 0f b4 f0 23 01 00 00 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 0f 72 7f 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 0f b2 00 20 00 00 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 0f 72 80 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 0f b2 c0 df ff ff 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e0 f4[ 	]*vpavgb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e0 f4[ 	]*vpavgb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e0 f4[ 	]*vpavgb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e0 31[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e0 b4 f0 23 01 00 00[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e0 72 7f[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e0 b2 00 20 00 00[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e0 72 80[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e0 b2 c0 df ff ff[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e3 f4[ 	]*vpavgw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e3 f4[ 	]*vpavgw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e3 f4[ 	]*vpavgw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e3 31[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e3 b4 f0 23 01 00 00[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e3 72 7f[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e3 b2 00 20 00 00[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e3 72 80[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e3 b2 c0 df ff ff[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 66 f4[ 	]*vpblendmb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 66 f4[ 	]*vpblendmb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 66 f4[ 	]*vpblendmb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 66 31[ 	]*vpblendmb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 66 b4 f0 23 01 00 00[ 	]*vpblendmb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 66 72 7f[ 	]*vpblendmb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 66 b2 00 20 00 00[ 	]*vpblendmb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 66 72 80[ 	]*vpblendmb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 66 b2 c0 df ff ff[ 	]*vpblendmb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 78 f5[ 	]*vpbroadcastb zmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 78 f5[ 	]*vpbroadcastb zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 78 f5[ 	]*vpbroadcastb zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 78 31[ 	]*vpbroadcastb zmm30,BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 78 b4 f0 23 01 00 00[ 	]*vpbroadcastb zmm30,BYTE PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 78 72 7f[ 	]*vpbroadcastb zmm30,BYTE PTR \[rdx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 78 b2 80 00 00 00[ 	]*vpbroadcastb zmm30,BYTE PTR \[rdx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 78 72 80[ 	]*vpbroadcastb zmm30,BYTE PTR \[rdx-0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 78 b2 7f ff ff ff[ 	]*vpbroadcastb zmm30,BYTE PTR \[rdx-0x81\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 7a f0[ 	]*vpbroadcastb zmm30,eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 7a f0[ 	]*vpbroadcastb zmm30\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 7a f0[ 	]*vpbroadcastb zmm30\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 79 f5[ 	]*vpbroadcastw zmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 79 f5[ 	]*vpbroadcastw zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 79 f5[ 	]*vpbroadcastw zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 79 31[ 	]*vpbroadcastw zmm30,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 79 b4 f0 23 01 00 00[ 	]*vpbroadcastw zmm30,WORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 79 72 7f[ 	]*vpbroadcastw zmm30,WORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 79 b2 00 01 00 00[ 	]*vpbroadcastw zmm30,WORD PTR \[rdx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 79 72 80[ 	]*vpbroadcastw zmm30,WORD PTR \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 79 b2 fe fe ff ff[ 	]*vpbroadcastw zmm30,WORD PTR \[rdx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 7b f0[ 	]*vpbroadcastw zmm30,eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 7b f0[ 	]*vpbroadcastw zmm30\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 7b f0[ 	]*vpbroadcastw zmm30\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 74 ed[ 	]*vpcmpeqb k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 74 ed[ 	]*vpcmpeqb k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 74 29[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 74 ac f0 23 01 00 00[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 74 6a 7f[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 74 aa 00 20 00 00[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 74 6a 80[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 74 aa c0 df ff ff[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 75 ed[ 	]*vpcmpeqw k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 75 ed[ 	]*vpcmpeqw k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 75 29[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 75 ac f0 23 01 00 00[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 75 6a 7f[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 75 aa 00 20 00 00[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 75 6a 80[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 75 aa c0 df ff ff[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 64 ed[ 	]*vpcmpgtb k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 64 ed[ 	]*vpcmpgtb k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 64 29[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 64 ac f0 23 01 00 00[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 64 6a 7f[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 64 aa 00 20 00 00[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 64 6a 80[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 64 aa c0 df ff ff[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 65 ed[ 	]*vpcmpgtw k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 65 ed[ 	]*vpcmpgtw k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 65 29[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 65 ac f0 23 01 00 00[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 65 6a 7f[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 65 aa 00 20 00 00[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 65 6a 80[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 65 aa c0 df ff ff[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 66 f4[ 	]*vpblendmw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 66 f4[ 	]*vpblendmw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 66 f4[ 	]*vpblendmw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 66 31[ 	]*vpblendmw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 66 b4 f0 23 01 00 00[ 	]*vpblendmw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 66 72 7f[ 	]*vpblendmw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 66 b2 00 20 00 00[ 	]*vpblendmw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 66 72 80[ 	]*vpblendmw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 66 b2 c0 df ff ff[ 	]*vpblendmw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 e8 ab[ 	]*vpextrb eax,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 e8 7b[ 	]*vpextrb eax,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7d 08 14 e8 7b[ 	]*vpextrb r8d,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 29 7b[ 	]*vpextrb BYTE PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 08 14 ac f0 23 01 00 00 7b[ 	]*vpextrb BYTE PTR \[rax\+r14\*8\+0x123\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 6a 7f 7b[ 	]*vpextrb BYTE PTR \[rdx\+0x7f\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 aa 80 00 00 00 7b[ 	]*vpextrb BYTE PTR \[rdx\+0x80\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 6a 80 7b[ 	]*vpextrb BYTE PTR \[rdx-0x80\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 aa 7f ff ff ff 7b[ 	]*vpextrb BYTE PTR \[rdx-0x81\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 15 29 7b[ 	]*vpextrw WORD PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 08 15 ac f0 23 01 00 00 7b[ 	]*vpextrw WORD PTR \[rax\+r14\*8\+0x123\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 15 6a 7f 7b[ 	]*vpextrw WORD PTR \[rdx\+0xfe\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 15 aa 00 01 00 00 7b[ 	]*vpextrw WORD PTR \[rdx\+0x100\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 15 6a 80 7b[ 	]*vpextrw WORD PTR \[rdx-0x100\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 15 aa fe fe ff ff 7b[ 	]*vpextrw WORD PTR \[rdx-0x102\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 7d 08 c5 c6 ab[ 	]*vpextrw eax,xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 7d 08 c5 c6 7b[ 	]*vpextrw eax,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 11 7d 08 c5 c6 7b[ 	]*vpextrw r8d,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 f0 ab[ 	]*vpinsrb xmm30,xmm29,eax,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 f0 7b[ 	]*vpinsrb xmm30,xmm29,eax,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 f5 7b[ 	]*vpinsrb xmm30,xmm29,ebp,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 15 00 20 f5 7b[ 	]*vpinsrb xmm30,xmm29,r13d,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 31 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 20 b4 f0 23 01 00 00 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 72 7f 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx\+0x7f\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 b2 80 00 00 00 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 72 80 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx-0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 b2 7f ff ff ff 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx-0x81\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 f0 ab[ 	]*vpinsrw xmm30,xmm29,eax,0xab
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 f0 7b[ 	]*vpinsrw xmm30,xmm29,eax,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 f5 7b[ 	]*vpinsrw xmm30,xmm29,ebp,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 41 15 00 c4 f5 7b[ 	]*vpinsrw xmm30,xmm29,r13d,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 31 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 c4 b4 f0 23 01 00 00 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 72 7f 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 b2 00 01 00 00 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx\+0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 72 80 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 b2 fe fe ff ff 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx-0x102\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 04 f4[ 	]*vpmaddubsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 04 f4[ 	]*vpmaddubsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 04 f4[ 	]*vpmaddubsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 04 31[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 04 b4 f0 23 01 00 00[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 04 72 7f[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 04 b2 00 20 00 00[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 04 72 80[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 04 b2 c0 df ff ff[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 f5 f4[ 	]*vpmaddwd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 f5 f4[ 	]*vpmaddwd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 f5 f4[ 	]*vpmaddwd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f5 31[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 f5 b4 f0 23 01 00 00[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f5 72 7f[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f5 b2 00 20 00 00[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f5 72 80[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f5 b2 c0 df ff ff[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 3c f4[ 	]*vpmaxsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 3c f4[ 	]*vpmaxsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 3c f4[ 	]*vpmaxsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3c 31[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 3c b4 f0 23 01 00 00[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3c 72 7f[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3c b2 00 20 00 00[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3c 72 80[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3c b2 c0 df ff ff[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 ee f4[ 	]*vpmaxsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 ee f4[ 	]*vpmaxsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 ee f4[ 	]*vpmaxsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ee 31[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 ee b4 f0 23 01 00 00[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ee 72 7f[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ee b2 00 20 00 00[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ee 72 80[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ee b2 c0 df ff ff[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 de f4[ 	]*vpmaxub zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 de f4[ 	]*vpmaxub zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 de f4[ 	]*vpmaxub zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 de 31[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 de b4 f0 23 01 00 00[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 de 72 7f[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 de b2 00 20 00 00[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 de 72 80[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 de b2 c0 df ff ff[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 3e f4[ 	]*vpmaxuw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 3e f4[ 	]*vpmaxuw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 3e f4[ 	]*vpmaxuw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3e 31[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 3e b4 f0 23 01 00 00[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3e 72 7f[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3e b2 00 20 00 00[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3e 72 80[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3e b2 c0 df ff ff[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 38 f4[ 	]*vpminsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 38 f4[ 	]*vpminsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 38 f4[ 	]*vpminsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 38 31[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 38 b4 f0 23 01 00 00[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 38 72 7f[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 38 b2 00 20 00 00[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 38 72 80[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 38 b2 c0 df ff ff[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 ea f4[ 	]*vpminsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 ea f4[ 	]*vpminsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 ea f4[ 	]*vpminsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ea 31[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 ea b4 f0 23 01 00 00[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ea 72 7f[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ea b2 00 20 00 00[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ea 72 80[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ea b2 c0 df ff ff[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 da f4[ 	]*vpminub zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 da f4[ 	]*vpminub zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 da f4[ 	]*vpminub zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 da 31[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 da b4 f0 23 01 00 00[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 da 72 7f[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 da b2 00 20 00 00[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 da 72 80[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 da b2 c0 df ff ff[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 3a f4[ 	]*vpminuw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 3a f4[ 	]*vpminuw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 3a f4[ 	]*vpminuw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3a 31[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 3a b4 f0 23 01 00 00[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3a 72 7f[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3a b2 00 20 00 00[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3a 72 80[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3a b2 c0 df ff ff[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 20 f5[ 	]*vpmovsxbw zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 20 f5[ 	]*vpmovsxbw zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 20 f5[ 	]*vpmovsxbw zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 20 31[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 20 b4 f0 23 01 00 00[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 20 72 7f[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 20 b2 00 10 00 00[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 20 72 80[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 20 b2 e0 ef ff ff[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 30 f5[ 	]*vpmovzxbw zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 30 f5[ 	]*vpmovzxbw zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 30 f5[ 	]*vpmovzxbw zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 30 31[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 30 b4 f0 23 01 00 00[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 30 72 7f[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 30 b2 00 10 00 00[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 30 72 80[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 30 b2 e0 ef ff ff[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 0b f4[ 	]*vpmulhrsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 0b f4[ 	]*vpmulhrsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 0b f4[ 	]*vpmulhrsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 0b 31[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 0b b4 f0 23 01 00 00[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 0b 72 7f[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 0b b2 00 20 00 00[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 0b 72 80[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 0b b2 c0 df ff ff[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e4 f4[ 	]*vpmulhuw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e4 f4[ 	]*vpmulhuw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e4 f4[ 	]*vpmulhuw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e4 31[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e4 b4 f0 23 01 00 00[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e4 72 7f[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e4 b2 00 20 00 00[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e4 72 80[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e4 b2 c0 df ff ff[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e5 f4[ 	]*vpmulhw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e5 f4[ 	]*vpmulhw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e5 f4[ 	]*vpmulhw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e5 31[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e5 b4 f0 23 01 00 00[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e5 72 7f[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e5 b2 00 20 00 00[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e5 72 80[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e5 b2 c0 df ff ff[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 d5 f4[ 	]*vpmullw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 d5 f4[ 	]*vpmullw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 d5 f4[ 	]*vpmullw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d5 31[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 d5 b4 f0 23 01 00 00[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d5 72 7f[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d5 b2 00 20 00 00[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d5 72 80[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d5 b2 c0 df ff ff[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 f6 f4[ 	]*vpsadbw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f6 31[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 f6 b4 f0 23 01 00 00[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f6 72 7f[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f6 b2 00 20 00 00[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f6 72 80[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f6 b2 c0 df ff ff[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 00 f4[ 	]*vpshufb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 00 f4[ 	]*vpshufb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 00 f4[ 	]*vpshufb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 00 31[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 00 b4 f0 23 01 00 00[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 00 72 7f[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 00 b2 00 20 00 00[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 00 72 80[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 00 b2 c0 df ff ff[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 48 70 f5 ab[ 	]*vpshufhw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 4f 70 f5 ab[ 	]*vpshufhw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e cf 70 f5 ab[ 	]*vpshufhw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 48 70 f5 7b[ 	]*vpshufhw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 48 70 31 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 7e 48 70 b4 f0 23 01 00 00 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 48 70 72 7f 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 48 70 b2 00 20 00 00 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 48 70 72 80 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 48 70 b2 c0 df ff ff 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 48 70 f5 ab[ 	]*vpshuflw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 4f 70 f5 ab[ 	]*vpshuflw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f cf 70 f5 ab[ 	]*vpshuflw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 48 70 f5 7b[ 	]*vpshuflw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 70 31 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 48 70 b4 f0 23 01 00 00 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 70 72 7f 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 70 b2 00 20 00 00 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 70 72 80 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 70 b2 c0 df ff ff 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 f1 f4[ 	]*vpsllw zmm30,zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 f1 f4[ 	]*vpsllw zmm30\{k7\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 f1 f4[ 	]*vpsllw zmm30\{k7\}\{z\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f1 31[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 f1 b4 f0 23 01 00 00[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f1 72 7f[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f1 b2 00 08 00 00[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f1 72 80[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f1 b2 f0 f7 ff ff[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e1 f4[ 	]*vpsraw zmm30,zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e1 f4[ 	]*vpsraw zmm30\{k7\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e1 f4[ 	]*vpsraw zmm30\{k7\}\{z\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e1 31[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e1 b4 f0 23 01 00 00[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e1 72 7f[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e1 b2 00 08 00 00[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e1 72 80[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e1 b2 f0 f7 ff ff[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 d1 f4[ 	]*vpsrlw zmm30,zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 d1 f4[ 	]*vpsrlw zmm30\{k7\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 d1 f4[ 	]*vpsrlw zmm30\{k7\}\{z\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d1 31[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 d1 b4 f0 23 01 00 00[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d1 72 7f[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d1 b2 00 08 00 00[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d1 72 80[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d1 b2 f0 f7 ff ff[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 73 dd ab[ 	]*vpsrldq zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 73 dd 7b[ 	]*vpsrldq zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 19 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 73 9c f0 23 01 00 00 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 5a 7f 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 9a 00 20 00 00 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 5a 80 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 9a c0 df ff ff 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 71 d5 ab[ 	]*vpsrlw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 71 d5 ab[ 	]*vpsrlw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d c7 71 d5 ab[ 	]*vpsrlw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 71 d5 7b[ 	]*vpsrlw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 11 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 71 94 f0 23 01 00 00 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 52 7f 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 92 00 20 00 00 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 52 80 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 92 c0 df ff ff 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 71 e5 ab[ 	]*vpsraw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 71 e5 ab[ 	]*vpsraw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d c7 71 e5 ab[ 	]*vpsraw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 71 e5 7b[ 	]*vpsraw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 21 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 71 a4 f0 23 01 00 00 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 62 7f 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 a2 00 20 00 00 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 62 80 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 a2 c0 df ff ff 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 10 f4[ 	]*vpsrlvw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 10 f4[ 	]*vpsrlvw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 10 f4[ 	]*vpsrlvw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 10 31[ 	]*vpsrlvw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 10 b4 f0 23 01 00 00[ 	]*vpsrlvw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 10 72 7f[ 	]*vpsrlvw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 10 b2 00 20 00 00[ 	]*vpsrlvw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 10 72 80[ 	]*vpsrlvw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 10 b2 c0 df ff ff[ 	]*vpsrlvw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 11 f4[ 	]*vpsravw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 11 f4[ 	]*vpsravw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 11 f4[ 	]*vpsravw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 11 31[ 	]*vpsravw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 11 b4 f0 23 01 00 00[ 	]*vpsravw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 11 72 7f[ 	]*vpsravw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 11 b2 00 20 00 00[ 	]*vpsravw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 11 72 80[ 	]*vpsravw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 11 b2 c0 df ff ff[ 	]*vpsravw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 f8 f4[ 	]*vpsubb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 f8 f4[ 	]*vpsubb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 f8 f4[ 	]*vpsubb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f8 31[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 f8 b4 f0 23 01 00 00[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f8 72 7f[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f8 b2 00 20 00 00[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f8 72 80[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f8 b2 c0 df ff ff[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e8 f4[ 	]*vpsubsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e8 f4[ 	]*vpsubsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e8 f4[ 	]*vpsubsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e8 31[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e8 b4 f0 23 01 00 00[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e8 72 7f[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e8 b2 00 20 00 00[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e8 72 80[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e8 b2 c0 df ff ff[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e9 f4[ 	]*vpsubsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e9 f4[ 	]*vpsubsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e9 f4[ 	]*vpsubsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e9 31[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e9 b4 f0 23 01 00 00[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e9 72 7f[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e9 b2 00 20 00 00[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e9 72 80[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e9 b2 c0 df ff ff[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 d8 f4[ 	]*vpsubusb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 d8 f4[ 	]*vpsubusb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 d8 f4[ 	]*vpsubusb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d8 31[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 d8 b4 f0 23 01 00 00[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d8 72 7f[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d8 b2 00 20 00 00[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d8 72 80[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d8 b2 c0 df ff ff[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 d9 f4[ 	]*vpsubusw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 d9 f4[ 	]*vpsubusw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 d9 f4[ 	]*vpsubusw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d9 31[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 d9 b4 f0 23 01 00 00[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d9 72 7f[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d9 b2 00 20 00 00[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d9 72 80[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d9 b2 c0 df ff ff[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 f9 f4[ 	]*vpsubw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 f9 f4[ 	]*vpsubw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 f9 f4[ 	]*vpsubw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f9 31[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 f9 b4 f0 23 01 00 00[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f9 72 7f[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f9 b2 00 20 00 00[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f9 72 80[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f9 b2 c0 df ff ff[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 68 f4[ 	]*vpunpckhbw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 68 f4[ 	]*vpunpckhbw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 68 f4[ 	]*vpunpckhbw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 68 31[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 68 b4 f0 23 01 00 00[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 68 72 7f[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 68 b2 00 20 00 00[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 68 72 80[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 68 b2 c0 df ff ff[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 69 f4[ 	]*vpunpckhwd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 69 f4[ 	]*vpunpckhwd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 69 f4[ 	]*vpunpckhwd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 69 31[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 69 b4 f0 23 01 00 00[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 69 72 7f[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 69 b2 00 20 00 00[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 69 72 80[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 69 b2 c0 df ff ff[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 60 f4[ 	]*vpunpcklbw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 60 f4[ 	]*vpunpcklbw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 60 f4[ 	]*vpunpcklbw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 60 31[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 60 b4 f0 23 01 00 00[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 60 72 7f[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 60 b2 00 20 00 00[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 60 72 80[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 60 b2 c0 df ff ff[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 61 f4[ 	]*vpunpcklwd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 61 f4[ 	]*vpunpcklwd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 61 f4[ 	]*vpunpcklwd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 61 31[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 61 b4 f0 23 01 00 00[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 61 72 7f[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 61 b2 00 20 00 00[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 61 72 80[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 61 b2 c0 df ff ff[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 48 30 ee[ 	]*vpmovwb ymm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 4f 30 ee[ 	]*vpmovwb ymm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e cf 30 ee[ 	]*vpmovwb ymm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 48 20 ee[ 	]*vpmovswb ymm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 4f 20 ee[ 	]*vpmovswb ymm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e cf 20 ee[ 	]*vpmovswb ymm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 48 10 ee[ 	]*vpmovuswb ymm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 4f 10 ee[ 	]*vpmovuswb ymm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e cf 10 ee[ 	]*vpmovuswb ymm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 42 f4 ab[ 	]*vdbpsadbw zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 42 f4 ab[ 	]*vdbpsadbw zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 42 f4 ab[ 	]*vdbpsadbw zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 42 f4 7b[ 	]*vdbpsadbw zmm30,zmm29,zmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 42 31 7b[ 	]*vdbpsadbw zmm30,zmm29,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 42 b4 f0 23 01 00 00 7b[ 	]*vdbpsadbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 42 72 7f 7b[ 	]*vdbpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 42 b2 00 20 00 00 7b[ 	]*vdbpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 42 72 80 7b[ 	]*vdbpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 42 b2 c0 df ff ff 7b[ 	]*vdbpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 8d f4[ 	]*vpermw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 8d f4[ 	]*vpermw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 8d f4[ 	]*vpermw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 8d 31[ 	]*vpermw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 8d b4 f0 23 01 00 00[ 	]*vpermw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 8d 72 7f[ 	]*vpermw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 8d b2 00 20 00 00[ 	]*vpermw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 8d 72 80[ 	]*vpermw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 8d b2 c0 df ff ff[ 	]*vpermw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 7d f4[ 	]*vpermt2w zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 7d f4[ 	]*vpermt2w zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 7d f4[ 	]*vpermt2w zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 7d 31[ 	]*vpermt2w zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 7d b4 f0 23 01 00 00[ 	]*vpermt2w zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 7d 72 7f[ 	]*vpermt2w zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 7d b2 00 20 00 00[ 	]*vpermt2w zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 7d 72 80[ 	]*vpermt2w zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 7d b2 c0 df ff ff[ 	]*vpermt2w zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 73 fd ab[ 	]*vpslldq zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 73 fd 7b[ 	]*vpslldq zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 39 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 73 bc f0 23 01 00 00 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 7a 7f 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 ba 00 20 00 00 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 7a 80 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 ba c0 df ff ff 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 71 f5 ab[ 	]*vpsllw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 71 f5 ab[ 	]*vpsllw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d c7 71 f5 ab[ 	]*vpsllw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 71 f5 7b[ 	]*vpsllw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 31 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 71 b4 f0 23 01 00 00 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 72 7f 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 b2 00 20 00 00 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 72 80 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 b2 c0 df ff ff 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 12 f4[ 	]*vpsllvw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 12 f4[ 	]*vpsllvw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 12 f4[ 	]*vpsllvw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 12 31[ 	]*vpsllvw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 12 b4 f0 23 01 00 00[ 	]*vpsllvw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 12 72 7f[ 	]*vpsllvw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 12 b2 00 20 00 00[ 	]*vpsllvw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 12 72 80[ 	]*vpsllvw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 12 b2 c0 df ff ff[ 	]*vpsllvw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 48 6f f5[ 	]*vmovdqu8 zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 4f 6f f5[ 	]*vmovdqu8 zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7f cf 6f f5[ 	]*vmovdqu8 zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 6f 31[ 	]*vmovdqu8 zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 48 6f b4 f0 23 01 00 00[ 	]*vmovdqu8 zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 6f 72 7f[ 	]*vmovdqu8 zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 6f b2 00 20 00 00[ 	]*vmovdqu8 zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 6f 72 80[ 	]*vmovdqu8 zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 6f b2 c0 df ff ff[ 	]*vmovdqu8 zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 6f f5[ 	]*vmovdqu16 zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 4f 6f f5[ 	]*vmovdqu16 zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff cf 6f f5[ 	]*vmovdqu16 zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 6f 31[ 	]*vmovdqu16 zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 48 6f b4 f0 23 01 00 00[ 	]*vmovdqu16 zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 6f 72 7f[ 	]*vmovdqu16 zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 6f b2 00 20 00 00[ 	]*vmovdqu16 zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 6f 72 80[ 	]*vmovdqu16 zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 6f b2 c0 df ff ff[ 	]*vmovdqu16 zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 41 ef[ 	]*kandq  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cd 41 ef[ 	]*kandd  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 42 ef[ 	]*kandnq k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cd 42 ef[ 	]*kandnd k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 45 ef[ 	]*korq   k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cd 45 ef[ 	]*kord   k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 46 ef[ 	]*kxnorq k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cd 46 ef[ 	]*kxnord k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 47 ef[ 	]*kxorq  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cd 47 ef[ 	]*kxord  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f8 44 ee[ 	]*knotq  k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f9 44 ee[ 	]*knotd  k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f8 98 ee[ 	]*kortestq k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f9 98 ee[ 	]*kortestd k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f8 99 ee[ 	]*ktestq k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f9 99 ee[ 	]*ktestd k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 f9 31 ee ab[ 	]*kshiftrq k5,k6,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 f9 31 ee 7b[ 	]*kshiftrq k5,k6,0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 31 ee ab[ 	]*kshiftrd k5,k6,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 31 ee 7b[ 	]*kshiftrd k5,k6,0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 f9 33 ee ab[ 	]*kshiftlq k5,k6,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 f9 33 ee 7b[ 	]*kshiftlq k5,k6,0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 33 ee ab[ 	]*kshiftld k5,k6,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 33 ee 7b[ 	]*kshiftld k5,k6,0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f8 90 ee[ 	]*kmovq  k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f8 90 29[ 	]*kmovq  k5,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*c4 a1 f8 90 ac f0 23 01 00 00[ 	]*kmovq  k5,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f9 90 ee[ 	]*kmovd  k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f9 90 29[ 	]*kmovd  k5,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*c4 a1 f9 90 ac f0 23 01 00 00[ 	]*kmovd  k5,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f8 91 29[ 	]*kmovq  QWORD PTR \[rcx\],k5
[ 	]*[a-f0-9]+:[ 	]*c4 a1 f8 91 ac f0 23 01 00 00[ 	]*kmovq  QWORD PTR \[rax\+r14\*8\+0x123\],k5
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f9 91 29[ 	]*kmovd  DWORD PTR \[rcx\],k5
[ 	]*[a-f0-9]+:[ 	]*c4 a1 f9 91 ac f0 23 01 00 00[ 	]*kmovd  DWORD PTR \[rax\+r14\*8\+0x123\],k5
[ 	]*[a-f0-9]+:[ 	]*c4 e1 fb 92 e8[ 	]*kmovq  k5,rax
[ 	]*[a-f0-9]+:[ 	]*c4 c1 fb 92 e8[ 	]*kmovq  k5,r8
[ 	]*[a-f0-9]+:[ 	]*c5 fb 92 e8[ 	]*kmovd  k5,eax
[ 	]*[a-f0-9]+:[ 	]*c5 fb 92 ed[ 	]*kmovd  k5,ebp
[ 	]*[a-f0-9]+:[ 	]*c4 c1 7b 92 ed[ 	]*kmovd  k5,r13d
[ 	]*[a-f0-9]+:[ 	]*c4 e1 fb 93 c5[ 	]*kmovq  rax,k5
[ 	]*[a-f0-9]+:[ 	]*c4 61 fb 93 c5[ 	]*kmovq  r8,k5
[ 	]*[a-f0-9]+:[ 	]*c5 fb 93 c5[ 	]*kmovd  eax,k5
[ 	]*[a-f0-9]+:[ 	]*c5 fb 93 ed[ 	]*kmovd  ebp,k5
[ 	]*[a-f0-9]+:[ 	]*c5 7b 93 ed[ 	]*kmovd  r13d,k5
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 4a ef[ 	]*kaddq  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cd 4a ef[ 	]*kaddd  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cc 4b ef[ 	]*kunpckwd k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 4b ef[ 	]*kunpckdq k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 30 31[ 	]*vpmovwb YMMWORD PTR \[rcx\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 4f 30 31[ 	]*vpmovwb YMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 48 30 b4 f0 23 01 00 00[ 	]*vpmovwb YMMWORD PTR \[rax\+r14\*8\+0x123\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 30 72 7f[ 	]*vpmovwb YMMWORD PTR \[rdx\+0xfe0\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 30 b2 00 10 00 00[ 	]*vpmovwb YMMWORD PTR \[rdx\+0x1000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 30 72 80[ 	]*vpmovwb YMMWORD PTR \[rdx-0x1000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 30 b2 e0 ef ff ff[ 	]*vpmovwb YMMWORD PTR \[rdx-0x1020\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 20 31[ 	]*vpmovswb YMMWORD PTR \[rcx\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 4f 20 31[ 	]*vpmovswb YMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 48 20 b4 f0 23 01 00 00[ 	]*vpmovswb YMMWORD PTR \[rax\+r14\*8\+0x123\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 20 72 7f[ 	]*vpmovswb YMMWORD PTR \[rdx\+0xfe0\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 20 b2 00 10 00 00[ 	]*vpmovswb YMMWORD PTR \[rdx\+0x1000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 20 72 80[ 	]*vpmovswb YMMWORD PTR \[rdx-0x1000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 20 b2 e0 ef ff ff[ 	]*vpmovswb YMMWORD PTR \[rdx-0x1020\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 10 31[ 	]*vpmovuswb YMMWORD PTR \[rcx\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 4f 10 31[ 	]*vpmovuswb YMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 48 10 b4 f0 23 01 00 00[ 	]*vpmovuswb YMMWORD PTR \[rax\+r14\*8\+0x123\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 10 72 7f[ 	]*vpmovuswb YMMWORD PTR \[rdx\+0xfe0\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 10 b2 00 10 00 00[ 	]*vpmovuswb YMMWORD PTR \[rdx\+0x1000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 10 72 80[ 	]*vpmovuswb YMMWORD PTR \[rdx-0x1000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 10 b2 e0 ef ff ff[ 	]*vpmovuswb YMMWORD PTR \[rdx-0x1020\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 7f 31[ 	]*vmovdqu8 ZMMWORD PTR \[rcx\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 4f 7f 31[ 	]*vmovdqu8 ZMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 48 7f b4 f0 23 01 00 00[ 	]*vmovdqu8 ZMMWORD PTR \[rax\+r14\*8\+0x123\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 7f 72 7f[ 	]*vmovdqu8 ZMMWORD PTR \[rdx\+0x1fc0\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 7f b2 00 20 00 00[ 	]*vmovdqu8 ZMMWORD PTR \[rdx\+0x2000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 7f 72 80[ 	]*vmovdqu8 ZMMWORD PTR \[rdx-0x2000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 7f b2 c0 df ff ff[ 	]*vmovdqu8 ZMMWORD PTR \[rdx-0x2040\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7f 31[ 	]*vmovdqu16 ZMMWORD PTR \[rcx\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 4f 7f 31[ 	]*vmovdqu16 ZMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 48 7f b4 f0 23 01 00 00[ 	]*vmovdqu16 ZMMWORD PTR \[rax\+r14\*8\+0x123\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7f 72 7f[ 	]*vmovdqu16 ZMMWORD PTR \[rdx\+0x1fc0\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7f b2 00 20 00 00[ 	]*vmovdqu16 ZMMWORD PTR \[rdx\+0x2000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7f 72 80[ 	]*vmovdqu16 ZMMWORD PTR \[rdx-0x2000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7f b2 c0 df ff ff[ 	]*vmovdqu16 ZMMWORD PTR \[rdx-0x2040\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 75 f4[ 	]*vpermi2w zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 75 f4[ 	]*vpermi2w zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 75 f4[ 	]*vpermi2w zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 75 31[ 	]*vpermi2w zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 75 b4 f0 23 01 00 00[ 	]*vpermi2w zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 75 72 7f[ 	]*vpermi2w zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 75 b2 00 20 00 00[ 	]*vpermi2w zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 75 72 80[ 	]*vpermi2w zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 75 b2 c0 df ff ff[ 	]*vpermi2w zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 92 0d 40 26 ed[ 	]*vptestmb k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 92 0d 47 26 ed[ 	]*vptestmb k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 40 26 29[ 	]*vptestmb k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 0d 40 26 ac f0 23 01 00 00[ 	]*vptestmb k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 40 26 6a 7f[ 	]*vptestmb k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 40 26 aa 00 20 00 00[ 	]*vptestmb k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 40 26 6a 80[ 	]*vptestmb k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 40 26 aa c0 df ff ff[ 	]*vptestmb k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 92 8d 40 26 ed[ 	]*vptestmw k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 92 8d 47 26 ed[ 	]*vptestmw k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 40 26 29[ 	]*vptestmw k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 8d 40 26 ac f0 23 01 00 00[ 	]*vptestmw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 40 26 6a 7f[ 	]*vptestmw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 40 26 aa 00 20 00 00[ 	]*vptestmw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 40 26 6a 80[ 	]*vptestmw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 40 26 aa c0 df ff ff[ 	]*vptestmw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 48 29 ee[ 	]*vpmovb2m k5,zmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 48 29 ee[ 	]*vpmovw2m k5,zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 28 f5[ 	]*vpmovm2b zmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 48 28 f5[ 	]*vpmovm2w zmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 92 16 40 26 ec[ 	]*vptestnmb k5,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 92 16 47 26 ec[ 	]*vptestnmb k5\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 40 26 29[ 	]*vptestnmb k5,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 16 40 26 ac f0 23 01 00 00[ 	]*vptestnmb k5,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 40 26 6a 7f[ 	]*vptestnmb k5,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 40 26 aa 00 20 00 00[ 	]*vptestnmb k5,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 40 26 6a 80[ 	]*vptestnmb k5,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 40 26 aa c0 df ff ff[ 	]*vptestnmb k5,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 92 96 40 26 ec[ 	]*vptestnmw k5,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 92 96 47 26 ec[ 	]*vptestnmw k5\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 40 26 29[ 	]*vptestnmw k5,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 96 40 26 ac f0 23 01 00 00[ 	]*vptestnmw k5,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 40 26 6a 7f[ 	]*vptestnmw k5,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 40 26 aa 00 20 00 00[ 	]*vptestnmw k5,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 40 26 6a 80[ 	]*vptestnmw k5,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 40 26 aa c0 df ff ff[ 	]*vptestnmw k5,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 40 3f ed ab[ 	]*vpcmpb k5,zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 47 3f ed ab[ 	]*vpcmpb k5\{k7\},zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 40 3f ed 7b[ 	]*vpcmpb k5,zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3f 29 7b[ 	]*vpcmpb k5,zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 0d 40 3f ac f0 23 01 00 00 7b[ 	]*vpcmpb k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3f 6a 7f 7b[ 	]*vpcmpb k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3f aa 00 20 00 00 7b[ 	]*vpcmpb k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3f 6a 80 7b[ 	]*vpcmpb k5,zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3f aa c0 df ff ff 7b[ 	]*vpcmpb k5,zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 40 3f ed ab[ 	]*vpcmpw k5,zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 47 3f ed ab[ 	]*vpcmpw k5\{k7\},zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 40 3f ed 7b[ 	]*vpcmpw k5,zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3f 29 7b[ 	]*vpcmpw k5,zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 8d 40 3f ac f0 23 01 00 00 7b[ 	]*vpcmpw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3f 6a 7f 7b[ 	]*vpcmpw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3f aa 00 20 00 00 7b[ 	]*vpcmpw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3f 6a 80 7b[ 	]*vpcmpw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3f aa c0 df ff ff 7b[ 	]*vpcmpw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 40 3e ed ab[ 	]*vpcmpub k5,zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 47 3e ed ab[ 	]*vpcmpub k5\{k7\},zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 40 3e ed 7b[ 	]*vpcmpub k5,zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3e 29 7b[ 	]*vpcmpub k5,zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 0d 40 3e ac f0 23 01 00 00 7b[ 	]*vpcmpub k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3e 6a 7f 7b[ 	]*vpcmpub k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3e aa 00 20 00 00 7b[ 	]*vpcmpub k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3e 6a 80 7b[ 	]*vpcmpub k5,zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3e aa c0 df ff ff 7b[ 	]*vpcmpub k5,zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 40 3e ed ab[ 	]*vpcmpuw k5,zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 47 3e ed ab[ 	]*vpcmpuw k5\{k7\},zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 40 3e ed 7b[ 	]*vpcmpuw k5,zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3e 29 7b[ 	]*vpcmpuw k5,zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 8d 40 3e ac f0 23 01 00 00 7b[ 	]*vpcmpuw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3e 6a 7f 7b[ 	]*vpcmpuw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3e aa 00 20 00 00 7b[ 	]*vpcmpuw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3e 6a 80 7b[ 	]*vpcmpuw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3e aa c0 df ff ff 7b[ 	]*vpcmpuw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 1c f5[ 	]*vpabsb zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 1c f5[ 	]*vpabsb zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 1c f5[ 	]*vpabsb zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1c 31[ 	]*vpabsb zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 1c b4 f0 34 12 00 00[ 	]*vpabsb zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1c 72 7f[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1c b2 00 20 00 00[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1c 72 80[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1c b2 c0 df ff ff[ 	]*vpabsb zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 1d f5[ 	]*vpabsw zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 1d f5[ 	]*vpabsw zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 1d f5[ 	]*vpabsw zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1d 31[ 	]*vpabsw zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 1d b4 f0 34 12 00 00[ 	]*vpabsw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1d 72 7f[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1d b2 00 20 00 00[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1d 72 80[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1d b2 c0 df ff ff[ 	]*vpabsw zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 6b f4[ 	]*vpackssdw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 6b f4[ 	]*vpackssdw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 6b f4[ 	]*vpackssdw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 6b 31[ 	]*vpackssdw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 6b b4 f0 34 12 00 00[ 	]*vpackssdw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 50 6b 31[ 	]*vpackssdw zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 6b 72 7f[ 	]*vpackssdw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 6b b2 00 20 00 00[ 	]*vpackssdw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 6b 72 80[ 	]*vpackssdw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 6b b2 c0 df ff ff[ 	]*vpackssdw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 50 6b 72 7f[ 	]*vpackssdw zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 50 6b b2 00 02 00 00[ 	]*vpackssdw zmm30,zmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 50 6b 72 80[ 	]*vpackssdw zmm30,zmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 50 6b b2 fc fd ff ff[ 	]*vpackssdw zmm30,zmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 63 f4[ 	]*vpacksswb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 63 f4[ 	]*vpacksswb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 63 f4[ 	]*vpacksswb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 63 31[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 63 b4 f0 34 12 00 00[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 63 72 7f[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 63 b2 00 20 00 00[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 63 72 80[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 63 b2 c0 df ff ff[ 	]*vpacksswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 2b f4[ 	]*vpackusdw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 2b f4[ 	]*vpackusdw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 2b f4[ 	]*vpackusdw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 2b 31[ 	]*vpackusdw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 2b b4 f0 34 12 00 00[ 	]*vpackusdw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 2b 31[ 	]*vpackusdw zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 2b 72 7f[ 	]*vpackusdw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 2b b2 00 20 00 00[ 	]*vpackusdw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 2b 72 80[ 	]*vpackusdw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 2b b2 c0 df ff ff[ 	]*vpackusdw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 2b 72 7f[ 	]*vpackusdw zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 2b b2 00 02 00 00[ 	]*vpackusdw zmm30,zmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 2b 72 80[ 	]*vpackusdw zmm30,zmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 2b b2 fc fd ff ff[ 	]*vpackusdw zmm30,zmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 67 f4[ 	]*vpackuswb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 67 f4[ 	]*vpackuswb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 67 f4[ 	]*vpackuswb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 67 31[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 67 b4 f0 34 12 00 00[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 67 72 7f[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 67 b2 00 20 00 00[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 67 72 80[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 67 b2 c0 df ff ff[ 	]*vpackuswb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 fc f4[ 	]*vpaddb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 fc f4[ 	]*vpaddb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 fc f4[ 	]*vpaddb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fc 31[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 fc b4 f0 34 12 00 00[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fc 72 7f[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fc b2 00 20 00 00[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fc 72 80[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fc b2 c0 df ff ff[ 	]*vpaddb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 ec f4[ 	]*vpaddsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 ec f4[ 	]*vpaddsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 ec f4[ 	]*vpaddsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ec 31[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 ec b4 f0 34 12 00 00[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ec 72 7f[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ec b2 00 20 00 00[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ec 72 80[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ec b2 c0 df ff ff[ 	]*vpaddsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 ed f4[ 	]*vpaddsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 ed f4[ 	]*vpaddsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 ed f4[ 	]*vpaddsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ed 31[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 ed b4 f0 34 12 00 00[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ed 72 7f[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ed b2 00 20 00 00[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ed 72 80[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ed b2 c0 df ff ff[ 	]*vpaddsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 dc f4[ 	]*vpaddusb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 dc f4[ 	]*vpaddusb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 dc f4[ 	]*vpaddusb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dc 31[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 dc b4 f0 34 12 00 00[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dc 72 7f[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dc b2 00 20 00 00[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dc 72 80[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dc b2 c0 df ff ff[ 	]*vpaddusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 dd f4[ 	]*vpaddusw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 dd f4[ 	]*vpaddusw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 dd f4[ 	]*vpaddusw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dd 31[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 dd b4 f0 34 12 00 00[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dd 72 7f[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dd b2 00 20 00 00[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dd 72 80[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 dd b2 c0 df ff ff[ 	]*vpaddusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 fd f4[ 	]*vpaddw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 fd f4[ 	]*vpaddw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 fd f4[ 	]*vpaddw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fd 31[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 fd b4 f0 34 12 00 00[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fd 72 7f[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fd b2 00 20 00 00[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fd 72 80[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 fd b2 c0 df ff ff[ 	]*vpaddw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 0f f4 ab[ 	]*vpalignr zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 0f f4 ab[ 	]*vpalignr zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 0f f4 ab[ 	]*vpalignr zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 0f f4 7b[ 	]*vpalignr zmm30,zmm29,zmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 0f 31 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 0f b4 f0 34 12 00 00 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 0f 72 7f 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 0f b2 00 20 00 00 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 0f 72 80 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 0f b2 c0 df ff ff 7b[ 	]*vpalignr zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e0 f4[ 	]*vpavgb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e0 f4[ 	]*vpavgb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e0 f4[ 	]*vpavgb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e0 31[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e0 b4 f0 34 12 00 00[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e0 72 7f[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e0 b2 00 20 00 00[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e0 72 80[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e0 b2 c0 df ff ff[ 	]*vpavgb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e3 f4[ 	]*vpavgw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e3 f4[ 	]*vpavgw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e3 f4[ 	]*vpavgw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e3 31[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e3 b4 f0 34 12 00 00[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e3 72 7f[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e3 b2 00 20 00 00[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e3 72 80[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e3 b2 c0 df ff ff[ 	]*vpavgw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 66 f4[ 	]*vpblendmb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 66 f4[ 	]*vpblendmb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 66 f4[ 	]*vpblendmb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 66 31[ 	]*vpblendmb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 66 b4 f0 34 12 00 00[ 	]*vpblendmb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 66 72 7f[ 	]*vpblendmb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 66 b2 00 20 00 00[ 	]*vpblendmb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 66 72 80[ 	]*vpblendmb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 66 b2 c0 df ff ff[ 	]*vpblendmb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 78 f5[ 	]*vpbroadcastb zmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 78 f5[ 	]*vpbroadcastb zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 78 f5[ 	]*vpbroadcastb zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 78 31[ 	]*vpbroadcastb zmm30,BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 78 b4 f0 34 12 00 00[ 	]*vpbroadcastb zmm30,BYTE PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 78 72 7f[ 	]*vpbroadcastb zmm30,BYTE PTR \[rdx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 78 b2 80 00 00 00[ 	]*vpbroadcastb zmm30,BYTE PTR \[rdx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 78 72 80[ 	]*vpbroadcastb zmm30,BYTE PTR \[rdx-0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 78 b2 7f ff ff ff[ 	]*vpbroadcastb zmm30,BYTE PTR \[rdx-0x81\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 7a f0[ 	]*vpbroadcastb zmm30,eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 7a f0[ 	]*vpbroadcastb zmm30\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 7a f0[ 	]*vpbroadcastb zmm30\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 79 f5[ 	]*vpbroadcastw zmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 79 f5[ 	]*vpbroadcastw zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 79 f5[ 	]*vpbroadcastw zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 79 31[ 	]*vpbroadcastw zmm30,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 79 b4 f0 34 12 00 00[ 	]*vpbroadcastw zmm30,WORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 79 72 7f[ 	]*vpbroadcastw zmm30,WORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 79 b2 00 01 00 00[ 	]*vpbroadcastw zmm30,WORD PTR \[rdx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 79 72 80[ 	]*vpbroadcastw zmm30,WORD PTR \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 79 b2 fe fe ff ff[ 	]*vpbroadcastw zmm30,WORD PTR \[rdx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 7b f0[ 	]*vpbroadcastw zmm30,eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 7b f0[ 	]*vpbroadcastw zmm30\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 7b f0[ 	]*vpbroadcastw zmm30\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 74 ed[ 	]*vpcmpeqb k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 74 ed[ 	]*vpcmpeqb k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 74 29[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 74 ac f0 34 12 00 00[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 74 6a 7f[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 74 aa 00 20 00 00[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 74 6a 80[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 74 aa c0 df ff ff[ 	]*vpcmpeqb k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 75 ed[ 	]*vpcmpeqw k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 75 ed[ 	]*vpcmpeqw k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 75 29[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 75 ac f0 34 12 00 00[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 75 6a 7f[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 75 aa 00 20 00 00[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 75 6a 80[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 75 aa c0 df ff ff[ 	]*vpcmpeqw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 64 ed[ 	]*vpcmpgtb k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 64 ed[ 	]*vpcmpgtb k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 64 29[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 64 ac f0 34 12 00 00[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 64 6a 7f[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 64 aa 00 20 00 00[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 64 6a 80[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 64 aa c0 df ff ff[ 	]*vpcmpgtb k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 65 ed[ 	]*vpcmpgtw k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 65 ed[ 	]*vpcmpgtw k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 65 29[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 65 ac f0 34 12 00 00[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 65 6a 7f[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 65 aa 00 20 00 00[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 65 6a 80[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 65 aa c0 df ff ff[ 	]*vpcmpgtw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 66 f4[ 	]*vpblendmw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 66 f4[ 	]*vpblendmw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 66 f4[ 	]*vpblendmw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 66 31[ 	]*vpblendmw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 66 b4 f0 34 12 00 00[ 	]*vpblendmw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 66 72 7f[ 	]*vpblendmw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 66 b2 00 20 00 00[ 	]*vpblendmw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 66 72 80[ 	]*vpblendmw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 66 b2 c0 df ff ff[ 	]*vpblendmw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 e8 ab[ 	]*vpextrb eax,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 e8 7b[ 	]*vpextrb eax,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7d 08 14 e8 7b[ 	]*vpextrb r8d,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 29 7b[ 	]*vpextrb BYTE PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 08 14 ac f0 34 12 00 00 7b[ 	]*vpextrb BYTE PTR \[rax\+r14\*8\+0x1234\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 6a 7f 7b[ 	]*vpextrb BYTE PTR \[rdx\+0x7f\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 aa 80 00 00 00 7b[ 	]*vpextrb BYTE PTR \[rdx\+0x80\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 6a 80 7b[ 	]*vpextrb BYTE PTR \[rdx-0x80\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 14 aa 7f ff ff ff 7b[ 	]*vpextrb BYTE PTR \[rdx-0x81\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 15 29 7b[ 	]*vpextrw WORD PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 08 15 ac f0 34 12 00 00 7b[ 	]*vpextrw WORD PTR \[rax\+r14\*8\+0x1234\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 15 6a 7f 7b[ 	]*vpextrw WORD PTR \[rdx\+0xfe\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 15 aa 00 01 00 00 7b[ 	]*vpextrw WORD PTR \[rdx\+0x100\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 15 6a 80 7b[ 	]*vpextrw WORD PTR \[rdx-0x100\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 15 aa fe fe ff ff 7b[ 	]*vpextrw WORD PTR \[rdx-0x102\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 7d 08 c5 c6 ab[ 	]*vpextrw eax,xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 7d 08 c5 c6 7b[ 	]*vpextrw eax,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 11 7d 08 c5 c6 7b[ 	]*vpextrw r8d,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 f0 ab[ 	]*vpinsrb xmm30,xmm29,eax,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 f0 7b[ 	]*vpinsrb xmm30,xmm29,eax,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 f5 7b[ 	]*vpinsrb xmm30,xmm29,ebp,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 15 00 20 f5 7b[ 	]*vpinsrb xmm30,xmm29,r13d,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 31 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 20 b4 f0 34 12 00 00 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 72 7f 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx\+0x7f\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 b2 80 00 00 00 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 72 80 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx-0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 20 b2 7f ff ff ff 7b[ 	]*vpinsrb xmm30,xmm29,BYTE PTR \[rdx-0x81\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 f0 ab[ 	]*vpinsrw xmm30,xmm29,eax,0xab
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 f0 7b[ 	]*vpinsrw xmm30,xmm29,eax,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 f5 7b[ 	]*vpinsrw xmm30,xmm29,ebp,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 41 15 00 c4 f5 7b[ 	]*vpinsrw xmm30,xmm29,r13d,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 31 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 c4 b4 f0 34 12 00 00 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 72 7f 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 b2 00 01 00 00 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx\+0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 72 80 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 c4 b2 fe fe ff ff 7b[ 	]*vpinsrw xmm30,xmm29,WORD PTR \[rdx-0x102\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 04 f4[ 	]*vpmaddubsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 04 f4[ 	]*vpmaddubsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 04 f4[ 	]*vpmaddubsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 04 31[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 04 b4 f0 34 12 00 00[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 04 72 7f[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 04 b2 00 20 00 00[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 04 72 80[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 04 b2 c0 df ff ff[ 	]*vpmaddubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 f5 f4[ 	]*vpmaddwd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 f5 f4[ 	]*vpmaddwd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 f5 f4[ 	]*vpmaddwd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f5 31[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 f5 b4 f0 34 12 00 00[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f5 72 7f[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f5 b2 00 20 00 00[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f5 72 80[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f5 b2 c0 df ff ff[ 	]*vpmaddwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 3c f4[ 	]*vpmaxsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 3c f4[ 	]*vpmaxsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 3c f4[ 	]*vpmaxsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3c 31[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 3c b4 f0 34 12 00 00[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3c 72 7f[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3c b2 00 20 00 00[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3c 72 80[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3c b2 c0 df ff ff[ 	]*vpmaxsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 ee f4[ 	]*vpmaxsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 ee f4[ 	]*vpmaxsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 ee f4[ 	]*vpmaxsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ee 31[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 ee b4 f0 34 12 00 00[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ee 72 7f[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ee b2 00 20 00 00[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ee 72 80[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ee b2 c0 df ff ff[ 	]*vpmaxsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 de f4[ 	]*vpmaxub zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 de f4[ 	]*vpmaxub zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 de f4[ 	]*vpmaxub zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 de 31[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 de b4 f0 34 12 00 00[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 de 72 7f[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 de b2 00 20 00 00[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 de 72 80[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 de b2 c0 df ff ff[ 	]*vpmaxub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 3e f4[ 	]*vpmaxuw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 3e f4[ 	]*vpmaxuw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 3e f4[ 	]*vpmaxuw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3e 31[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 3e b4 f0 34 12 00 00[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3e 72 7f[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3e b2 00 20 00 00[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3e 72 80[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3e b2 c0 df ff ff[ 	]*vpmaxuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 38 f4[ 	]*vpminsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 38 f4[ 	]*vpminsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 38 f4[ 	]*vpminsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 38 31[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 38 b4 f0 34 12 00 00[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 38 72 7f[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 38 b2 00 20 00 00[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 38 72 80[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 38 b2 c0 df ff ff[ 	]*vpminsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 ea f4[ 	]*vpminsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 ea f4[ 	]*vpminsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 ea f4[ 	]*vpminsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ea 31[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 ea b4 f0 34 12 00 00[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ea 72 7f[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ea b2 00 20 00 00[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ea 72 80[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 ea b2 c0 df ff ff[ 	]*vpminsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 da f4[ 	]*vpminub zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 da f4[ 	]*vpminub zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 da f4[ 	]*vpminub zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 da 31[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 da b4 f0 34 12 00 00[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 da 72 7f[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 da b2 00 20 00 00[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 da 72 80[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 da b2 c0 df ff ff[ 	]*vpminub zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 3a f4[ 	]*vpminuw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 3a f4[ 	]*vpminuw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 3a f4[ 	]*vpminuw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3a 31[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 3a b4 f0 34 12 00 00[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3a 72 7f[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3a b2 00 20 00 00[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3a 72 80[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 3a b2 c0 df ff ff[ 	]*vpminuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 20 f5[ 	]*vpmovsxbw zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 20 f5[ 	]*vpmovsxbw zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 20 f5[ 	]*vpmovsxbw zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 20 31[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 20 b4 f0 34 12 00 00[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 20 72 7f[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 20 b2 00 10 00 00[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 20 72 80[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 20 b2 e0 ef ff ff[ 	]*vpmovsxbw zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 30 f5[ 	]*vpmovzxbw zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 30 f5[ 	]*vpmovzxbw zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 30 f5[ 	]*vpmovzxbw zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 30 31[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 30 b4 f0 34 12 00 00[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 30 72 7f[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 30 b2 00 10 00 00[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 30 72 80[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 30 b2 e0 ef ff ff[ 	]*vpmovzxbw zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 0b f4[ 	]*vpmulhrsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 0b f4[ 	]*vpmulhrsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 0b f4[ 	]*vpmulhrsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 0b 31[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 0b b4 f0 34 12 00 00[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 0b 72 7f[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 0b b2 00 20 00 00[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 0b 72 80[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 0b b2 c0 df ff ff[ 	]*vpmulhrsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e4 f4[ 	]*vpmulhuw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e4 f4[ 	]*vpmulhuw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e4 f4[ 	]*vpmulhuw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e4 31[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e4 b4 f0 34 12 00 00[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e4 72 7f[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e4 b2 00 20 00 00[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e4 72 80[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e4 b2 c0 df ff ff[ 	]*vpmulhuw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e5 f4[ 	]*vpmulhw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e5 f4[ 	]*vpmulhw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e5 f4[ 	]*vpmulhw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e5 31[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e5 b4 f0 34 12 00 00[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e5 72 7f[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e5 b2 00 20 00 00[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e5 72 80[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e5 b2 c0 df ff ff[ 	]*vpmulhw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 d5 f4[ 	]*vpmullw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 d5 f4[ 	]*vpmullw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 d5 f4[ 	]*vpmullw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d5 31[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 d5 b4 f0 34 12 00 00[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d5 72 7f[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d5 b2 00 20 00 00[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d5 72 80[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d5 b2 c0 df ff ff[ 	]*vpmullw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 f6 f4[ 	]*vpsadbw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f6 31[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 f6 b4 f0 34 12 00 00[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f6 72 7f[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f6 b2 00 20 00 00[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f6 72 80[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f6 b2 c0 df ff ff[ 	]*vpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 00 f4[ 	]*vpshufb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 00 f4[ 	]*vpshufb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 00 f4[ 	]*vpshufb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 00 31[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 00 b4 f0 34 12 00 00[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 00 72 7f[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 00 b2 00 20 00 00[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 00 72 80[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 00 b2 c0 df ff ff[ 	]*vpshufb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 48 70 f5 ab[ 	]*vpshufhw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 4f 70 f5 ab[ 	]*vpshufhw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e cf 70 f5 ab[ 	]*vpshufhw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 48 70 f5 7b[ 	]*vpshufhw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 48 70 31 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 7e 48 70 b4 f0 34 12 00 00 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 48 70 72 7f 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 48 70 b2 00 20 00 00 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 48 70 72 80 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 48 70 b2 c0 df ff ff 7b[ 	]*vpshufhw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 48 70 f5 ab[ 	]*vpshuflw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 4f 70 f5 ab[ 	]*vpshuflw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f cf 70 f5 ab[ 	]*vpshuflw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 48 70 f5 7b[ 	]*vpshuflw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 70 31 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 48 70 b4 f0 34 12 00 00 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 70 72 7f 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 70 b2 00 20 00 00 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 70 72 80 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 70 b2 c0 df ff ff 7b[ 	]*vpshuflw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 f1 f4[ 	]*vpsllw zmm30,zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 f1 f4[ 	]*vpsllw zmm30\{k7\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 f1 f4[ 	]*vpsllw zmm30\{k7\}\{z\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f1 31[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 f1 b4 f0 34 12 00 00[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f1 72 7f[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f1 b2 00 08 00 00[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f1 72 80[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f1 b2 f0 f7 ff ff[ 	]*vpsllw zmm30,zmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e1 f4[ 	]*vpsraw zmm30,zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e1 f4[ 	]*vpsraw zmm30\{k7\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e1 f4[ 	]*vpsraw zmm30\{k7\}\{z\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e1 31[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e1 b4 f0 34 12 00 00[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e1 72 7f[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e1 b2 00 08 00 00[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e1 72 80[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e1 b2 f0 f7 ff ff[ 	]*vpsraw zmm30,zmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 d1 f4[ 	]*vpsrlw zmm30,zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 d1 f4[ 	]*vpsrlw zmm30\{k7\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 d1 f4[ 	]*vpsrlw zmm30\{k7\}\{z\},zmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d1 31[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 d1 b4 f0 34 12 00 00[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d1 72 7f[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d1 b2 00 08 00 00[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d1 72 80[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d1 b2 f0 f7 ff ff[ 	]*vpsrlw zmm30,zmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 73 dd ab[ 	]*vpsrldq zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 73 dd 7b[ 	]*vpsrldq zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 19 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 73 9c f0 34 12 00 00 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 5a 7f 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 9a 00 20 00 00 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 5a 80 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 9a c0 df ff ff 7b[ 	]*vpsrldq zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 71 d5 ab[ 	]*vpsrlw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 71 d5 ab[ 	]*vpsrlw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d c7 71 d5 ab[ 	]*vpsrlw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 71 d5 7b[ 	]*vpsrlw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 11 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 71 94 f0 34 12 00 00 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 52 7f 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 92 00 20 00 00 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 52 80 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 92 c0 df ff ff 7b[ 	]*vpsrlw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 71 e5 ab[ 	]*vpsraw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 71 e5 ab[ 	]*vpsraw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d c7 71 e5 ab[ 	]*vpsraw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 71 e5 7b[ 	]*vpsraw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 21 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 71 a4 f0 34 12 00 00 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 62 7f 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 a2 00 20 00 00 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 62 80 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 a2 c0 df ff ff 7b[ 	]*vpsraw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 10 f4[ 	]*vpsrlvw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 10 f4[ 	]*vpsrlvw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 10 f4[ 	]*vpsrlvw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 10 31[ 	]*vpsrlvw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 10 b4 f0 34 12 00 00[ 	]*vpsrlvw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 10 72 7f[ 	]*vpsrlvw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 10 b2 00 20 00 00[ 	]*vpsrlvw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 10 72 80[ 	]*vpsrlvw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 10 b2 c0 df ff ff[ 	]*vpsrlvw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 11 f4[ 	]*vpsravw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 11 f4[ 	]*vpsravw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 11 f4[ 	]*vpsravw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 11 31[ 	]*vpsravw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 11 b4 f0 34 12 00 00[ 	]*vpsravw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 11 72 7f[ 	]*vpsravw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 11 b2 00 20 00 00[ 	]*vpsravw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 11 72 80[ 	]*vpsravw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 11 b2 c0 df ff ff[ 	]*vpsravw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 f8 f4[ 	]*vpsubb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 f8 f4[ 	]*vpsubb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 f8 f4[ 	]*vpsubb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f8 31[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 f8 b4 f0 34 12 00 00[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f8 72 7f[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f8 b2 00 20 00 00[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f8 72 80[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f8 b2 c0 df ff ff[ 	]*vpsubb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e8 f4[ 	]*vpsubsb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e8 f4[ 	]*vpsubsb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e8 f4[ 	]*vpsubsb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e8 31[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e8 b4 f0 34 12 00 00[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e8 72 7f[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e8 b2 00 20 00 00[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e8 72 80[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e8 b2 c0 df ff ff[ 	]*vpsubsb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 e9 f4[ 	]*vpsubsw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 e9 f4[ 	]*vpsubsw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 e9 f4[ 	]*vpsubsw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e9 31[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 e9 b4 f0 34 12 00 00[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e9 72 7f[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e9 b2 00 20 00 00[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e9 72 80[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 e9 b2 c0 df ff ff[ 	]*vpsubsw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 d8 f4[ 	]*vpsubusb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 d8 f4[ 	]*vpsubusb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 d8 f4[ 	]*vpsubusb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d8 31[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 d8 b4 f0 34 12 00 00[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d8 72 7f[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d8 b2 00 20 00 00[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d8 72 80[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d8 b2 c0 df ff ff[ 	]*vpsubusb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 d9 f4[ 	]*vpsubusw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 d9 f4[ 	]*vpsubusw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 d9 f4[ 	]*vpsubusw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d9 31[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 d9 b4 f0 34 12 00 00[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d9 72 7f[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d9 b2 00 20 00 00[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d9 72 80[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 d9 b2 c0 df ff ff[ 	]*vpsubusw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 f9 f4[ 	]*vpsubw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 f9 f4[ 	]*vpsubw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 f9 f4[ 	]*vpsubw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f9 31[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 f9 b4 f0 34 12 00 00[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f9 72 7f[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f9 b2 00 20 00 00[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f9 72 80[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 f9 b2 c0 df ff ff[ 	]*vpsubw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 68 f4[ 	]*vpunpckhbw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 68 f4[ 	]*vpunpckhbw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 68 f4[ 	]*vpunpckhbw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 68 31[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 68 b4 f0 34 12 00 00[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 68 72 7f[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 68 b2 00 20 00 00[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 68 72 80[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 68 b2 c0 df ff ff[ 	]*vpunpckhbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 69 f4[ 	]*vpunpckhwd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 69 f4[ 	]*vpunpckhwd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 69 f4[ 	]*vpunpckhwd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 69 31[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 69 b4 f0 34 12 00 00[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 69 72 7f[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 69 b2 00 20 00 00[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 69 72 80[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 69 b2 c0 df ff ff[ 	]*vpunpckhwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 60 f4[ 	]*vpunpcklbw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 60 f4[ 	]*vpunpcklbw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 60 f4[ 	]*vpunpcklbw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 60 31[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 60 b4 f0 34 12 00 00[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 60 72 7f[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 60 b2 00 20 00 00[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 60 72 80[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 60 b2 c0 df ff ff[ 	]*vpunpcklbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 40 61 f4[ 	]*vpunpcklwd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 47 61 f4[ 	]*vpunpcklwd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 c7 61 f4[ 	]*vpunpcklwd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 61 31[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 40 61 b4 f0 34 12 00 00[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 61 72 7f[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 61 b2 00 20 00 00[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 61 72 80[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 40 61 b2 c0 df ff ff[ 	]*vpunpcklwd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 48 30 ee[ 	]*vpmovwb ymm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 4f 30 ee[ 	]*vpmovwb ymm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e cf 30 ee[ 	]*vpmovwb ymm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 48 20 ee[ 	]*vpmovswb ymm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 4f 20 ee[ 	]*vpmovswb ymm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e cf 20 ee[ 	]*vpmovswb ymm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 48 10 ee[ 	]*vpmovuswb ymm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 4f 10 ee[ 	]*vpmovuswb ymm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e cf 10 ee[ 	]*vpmovuswb ymm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 42 f4 ab[ 	]*vdbpsadbw zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 42 f4 ab[ 	]*vdbpsadbw zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 42 f4 ab[ 	]*vdbpsadbw zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 42 f4 7b[ 	]*vdbpsadbw zmm30,zmm29,zmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 42 31 7b[ 	]*vdbpsadbw zmm30,zmm29,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 42 b4 f0 34 12 00 00 7b[ 	]*vdbpsadbw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 42 72 7f 7b[ 	]*vdbpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 42 b2 00 20 00 00 7b[ 	]*vdbpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 42 72 80 7b[ 	]*vdbpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 42 b2 c0 df ff ff 7b[ 	]*vdbpsadbw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 8d f4[ 	]*vpermw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 8d f4[ 	]*vpermw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 8d f4[ 	]*vpermw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 8d 31[ 	]*vpermw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 8d b4 f0 34 12 00 00[ 	]*vpermw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 8d 72 7f[ 	]*vpermw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 8d b2 00 20 00 00[ 	]*vpermw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 8d 72 80[ 	]*vpermw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 8d b2 c0 df ff ff[ 	]*vpermw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 7d f4[ 	]*vpermt2w zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 7d f4[ 	]*vpermt2w zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 7d f4[ 	]*vpermt2w zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 7d 31[ 	]*vpermt2w zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 7d b4 f0 34 12 00 00[ 	]*vpermt2w zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 7d 72 7f[ 	]*vpermt2w zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 7d b2 00 20 00 00[ 	]*vpermt2w zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 7d 72 80[ 	]*vpermt2w zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 7d b2 c0 df ff ff[ 	]*vpermt2w zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 73 fd ab[ 	]*vpslldq zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 73 fd 7b[ 	]*vpslldq zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 39 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 73 bc f0 34 12 00 00 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 7a 7f 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 ba 00 20 00 00 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 7a 80 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 73 ba c0 df ff ff 7b[ 	]*vpslldq zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 71 f5 ab[ 	]*vpsllw zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 47 71 f5 ab[ 	]*vpsllw zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d c7 71 f5 ab[ 	]*vpsllw zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 40 71 f5 7b[ 	]*vpsllw zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 31 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 40 71 b4 f0 34 12 00 00 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 72 7f 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 b2 00 20 00 00 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 72 80 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 40 71 b2 c0 df ff ff 7b[ 	]*vpsllw zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 12 f4[ 	]*vpsllvw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 12 f4[ 	]*vpsllvw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 12 f4[ 	]*vpsllvw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 12 31[ 	]*vpsllvw zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 12 b4 f0 34 12 00 00[ 	]*vpsllvw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 12 72 7f[ 	]*vpsllvw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 12 b2 00 20 00 00[ 	]*vpsllvw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 12 72 80[ 	]*vpsllvw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 12 b2 c0 df ff ff[ 	]*vpsllvw zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 48 6f f5[ 	]*vmovdqu8 zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 4f 6f f5[ 	]*vmovdqu8 zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7f cf 6f f5[ 	]*vmovdqu8 zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 6f 31[ 	]*vmovdqu8 zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 48 6f b4 f0 34 12 00 00[ 	]*vmovdqu8 zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 6f 72 7f[ 	]*vmovdqu8 zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 6f b2 00 20 00 00[ 	]*vmovdqu8 zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 6f 72 80[ 	]*vmovdqu8 zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 6f b2 c0 df ff ff[ 	]*vmovdqu8 zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 6f f5[ 	]*vmovdqu16 zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 4f 6f f5[ 	]*vmovdqu16 zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff cf 6f f5[ 	]*vmovdqu16 zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 6f 31[ 	]*vmovdqu16 zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 48 6f b4 f0 34 12 00 00[ 	]*vmovdqu16 zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 6f 72 7f[ 	]*vmovdqu16 zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 6f b2 00 20 00 00[ 	]*vmovdqu16 zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 6f 72 80[ 	]*vmovdqu16 zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 6f b2 c0 df ff ff[ 	]*vmovdqu16 zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 41 ef[ 	]*kandq  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cd 41 ef[ 	]*kandd  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 42 ef[ 	]*kandnq k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cd 42 ef[ 	]*kandnd k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 45 ef[ 	]*korq   k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cd 45 ef[ 	]*kord   k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 46 ef[ 	]*kxnorq k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cd 46 ef[ 	]*kxnord k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 47 ef[ 	]*kxorq  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cd 47 ef[ 	]*kxord  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f8 44 ee[ 	]*knotq  k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f9 44 ee[ 	]*knotd  k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f8 98 ee[ 	]*kortestq k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f9 98 ee[ 	]*kortestd k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f8 99 ee[ 	]*ktestq k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f9 99 ee[ 	]*ktestd k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 f9 31 ee ab[ 	]*kshiftrq k5,k6,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 f9 31 ee 7b[ 	]*kshiftrq k5,k6,0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 31 ee ab[ 	]*kshiftrd k5,k6,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 31 ee 7b[ 	]*kshiftrd k5,k6,0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 f9 33 ee ab[ 	]*kshiftlq k5,k6,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 f9 33 ee 7b[ 	]*kshiftlq k5,k6,0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 33 ee ab[ 	]*kshiftld k5,k6,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 33 ee 7b[ 	]*kshiftld k5,k6,0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f8 90 ee[ 	]*kmovq  k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f8 90 29[ 	]*kmovq  k5,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*c4 a1 f8 90 ac f0 34 12 00 00[ 	]*kmovq  k5,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f9 90 ee[ 	]*kmovd  k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f9 90 29[ 	]*kmovd  k5,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*c4 a1 f9 90 ac f0 34 12 00 00[ 	]*kmovd  k5,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f8 91 29[ 	]*kmovq  QWORD PTR \[rcx\],k5
[ 	]*[a-f0-9]+:[ 	]*c4 a1 f8 91 ac f0 34 12 00 00[ 	]*kmovq  QWORD PTR \[rax\+r14\*8\+0x1234\],k5
[ 	]*[a-f0-9]+:[ 	]*c4 e1 f9 91 29[ 	]*kmovd  DWORD PTR \[rcx\],k5
[ 	]*[a-f0-9]+:[ 	]*c4 a1 f9 91 ac f0 34 12 00 00[ 	]*kmovd  DWORD PTR \[rax\+r14\*8\+0x1234\],k5
[ 	]*[a-f0-9]+:[ 	]*c4 e1 fb 92 e8[ 	]*kmovq  k5,rax
[ 	]*[a-f0-9]+:[ 	]*c4 c1 fb 92 e8[ 	]*kmovq  k5,r8
[ 	]*[a-f0-9]+:[ 	]*c5 fb 92 e8[ 	]*kmovd  k5,eax
[ 	]*[a-f0-9]+:[ 	]*c5 fb 92 ed[ 	]*kmovd  k5,ebp
[ 	]*[a-f0-9]+:[ 	]*c4 c1 7b 92 ed[ 	]*kmovd  k5,r13d
[ 	]*[a-f0-9]+:[ 	]*c4 e1 fb 93 c5[ 	]*kmovq  rax,k5
[ 	]*[a-f0-9]+:[ 	]*c4 61 fb 93 c5[ 	]*kmovq  r8,k5
[ 	]*[a-f0-9]+:[ 	]*c5 fb 93 c5[ 	]*kmovd  eax,k5
[ 	]*[a-f0-9]+:[ 	]*c5 fb 93 ed[ 	]*kmovd  ebp,k5
[ 	]*[a-f0-9]+:[ 	]*c5 7b 93 ed[ 	]*kmovd  r13d,k5
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 4a ef[ 	]*kaddq  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cd 4a ef[ 	]*kaddd  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cc 4b ef[ 	]*kunpckwd k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c4 e1 cc 4b ef[ 	]*kunpckdq k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 30 31[ 	]*vpmovwb YMMWORD PTR \[rcx\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 4f 30 31[ 	]*vpmovwb YMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 48 30 b4 f0 34 12 00 00[ 	]*vpmovwb YMMWORD PTR \[rax\+r14\*8\+0x1234\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 30 72 7f[ 	]*vpmovwb YMMWORD PTR \[rdx\+0xfe0\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 30 b2 00 10 00 00[ 	]*vpmovwb YMMWORD PTR \[rdx\+0x1000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 30 72 80[ 	]*vpmovwb YMMWORD PTR \[rdx-0x1000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 30 b2 e0 ef ff ff[ 	]*vpmovwb YMMWORD PTR \[rdx-0x1020\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 20 31[ 	]*vpmovswb YMMWORD PTR \[rcx\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 4f 20 31[ 	]*vpmovswb YMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 48 20 b4 f0 34 12 00 00[ 	]*vpmovswb YMMWORD PTR \[rax\+r14\*8\+0x1234\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 20 72 7f[ 	]*vpmovswb YMMWORD PTR \[rdx\+0xfe0\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 20 b2 00 10 00 00[ 	]*vpmovswb YMMWORD PTR \[rdx\+0x1000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 20 72 80[ 	]*vpmovswb YMMWORD PTR \[rdx-0x1000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 20 b2 e0 ef ff ff[ 	]*vpmovswb YMMWORD PTR \[rdx-0x1020\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 10 31[ 	]*vpmovuswb YMMWORD PTR \[rcx\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 4f 10 31[ 	]*vpmovuswb YMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 48 10 b4 f0 34 12 00 00[ 	]*vpmovuswb YMMWORD PTR \[rax\+r14\*8\+0x1234\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 10 72 7f[ 	]*vpmovuswb YMMWORD PTR \[rdx\+0xfe0\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 10 b2 00 10 00 00[ 	]*vpmovuswb YMMWORD PTR \[rdx\+0x1000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 10 72 80[ 	]*vpmovuswb YMMWORD PTR \[rdx-0x1000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 10 b2 e0 ef ff ff[ 	]*vpmovuswb YMMWORD PTR \[rdx-0x1020\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 7f 31[ 	]*vmovdqu8 ZMMWORD PTR \[rcx\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 4f 7f 31[ 	]*vmovdqu8 ZMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 48 7f b4 f0 34 12 00 00[ 	]*vmovdqu8 ZMMWORD PTR \[rax\+r14\*8\+0x1234\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 7f 72 7f[ 	]*vmovdqu8 ZMMWORD PTR \[rdx\+0x1fc0\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 7f b2 00 20 00 00[ 	]*vmovdqu8 ZMMWORD PTR \[rdx\+0x2000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 7f 72 80[ 	]*vmovdqu8 ZMMWORD PTR \[rdx-0x2000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 48 7f b2 c0 df ff ff[ 	]*vmovdqu8 ZMMWORD PTR \[rdx-0x2040\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7f 31[ 	]*vmovdqu16 ZMMWORD PTR \[rcx\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 4f 7f 31[ 	]*vmovdqu16 ZMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 48 7f b4 f0 34 12 00 00[ 	]*vmovdqu16 ZMMWORD PTR \[rax\+r14\*8\+0x1234\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7f 72 7f[ 	]*vmovdqu16 ZMMWORD PTR \[rdx\+0x1fc0\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7f b2 00 20 00 00[ 	]*vmovdqu16 ZMMWORD PTR \[rdx\+0x2000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7f 72 80[ 	]*vmovdqu16 ZMMWORD PTR \[rdx-0x2000\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7f b2 c0 df ff ff[ 	]*vmovdqu16 ZMMWORD PTR \[rdx-0x2040\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 75 f4[ 	]*vpermi2w zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 75 f4[ 	]*vpermi2w zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 75 f4[ 	]*vpermi2w zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 75 31[ 	]*vpermi2w zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 75 b4 f0 34 12 00 00[ 	]*vpermi2w zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 75 72 7f[ 	]*vpermi2w zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 75 b2 00 20 00 00[ 	]*vpermi2w zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 75 72 80[ 	]*vpermi2w zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 75 b2 c0 df ff ff[ 	]*vpermi2w zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 92 0d 40 26 ed[ 	]*vptestmb k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 92 0d 47 26 ed[ 	]*vptestmb k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 40 26 29[ 	]*vptestmb k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 0d 40 26 ac f0 34 12 00 00[ 	]*vptestmb k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 40 26 6a 7f[ 	]*vptestmb k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 40 26 aa 00 20 00 00[ 	]*vptestmb k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 40 26 6a 80[ 	]*vptestmb k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 40 26 aa c0 df ff ff[ 	]*vptestmb k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 92 8d 40 26 ed[ 	]*vptestmw k5,zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 92 8d 47 26 ed[ 	]*vptestmw k5\{k7\},zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 40 26 29[ 	]*vptestmw k5,zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 8d 40 26 ac f0 34 12 00 00[ 	]*vptestmw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 40 26 6a 7f[ 	]*vptestmw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 40 26 aa 00 20 00 00[ 	]*vptestmw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 40 26 6a 80[ 	]*vptestmw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 40 26 aa c0 df ff ff[ 	]*vptestmw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 48 29 ee[ 	]*vpmovb2m k5,zmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 48 29 ee[ 	]*vpmovw2m k5,zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 28 f5[ 	]*vpmovm2b zmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 48 28 f5[ 	]*vpmovm2w zmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 92 16 40 26 ec[ 	]*vptestnmb k5,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 92 16 47 26 ec[ 	]*vptestnmb k5\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 40 26 29[ 	]*vptestnmb k5,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 16 40 26 ac f0 34 12 00 00[ 	]*vptestnmb k5,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 40 26 6a 7f[ 	]*vptestnmb k5,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 40 26 aa 00 20 00 00[ 	]*vptestnmb k5,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 40 26 6a 80[ 	]*vptestnmb k5,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 40 26 aa c0 df ff ff[ 	]*vptestnmb k5,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 92 96 40 26 ec[ 	]*vptestnmw k5,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 92 96 47 26 ec[ 	]*vptestnmw k5\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 40 26 29[ 	]*vptestnmw k5,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 96 40 26 ac f0 34 12 00 00[ 	]*vptestnmw k5,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 40 26 6a 7f[ 	]*vptestnmw k5,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 40 26 aa 00 20 00 00[ 	]*vptestnmw k5,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 40 26 6a 80[ 	]*vptestnmw k5,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 40 26 aa c0 df ff ff[ 	]*vptestnmw k5,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 40 3f ed ab[ 	]*vpcmpb k5,zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 47 3f ed ab[ 	]*vpcmpb k5\{k7\},zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 40 3f ed 7b[ 	]*vpcmpb k5,zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3f 29 7b[ 	]*vpcmpb k5,zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 0d 40 3f ac f0 34 12 00 00 7b[ 	]*vpcmpb k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3f 6a 7f 7b[ 	]*vpcmpb k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3f aa 00 20 00 00 7b[ 	]*vpcmpb k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3f 6a 80 7b[ 	]*vpcmpb k5,zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3f aa c0 df ff ff 7b[ 	]*vpcmpb k5,zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 40 3f ed ab[ 	]*vpcmpw k5,zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 47 3f ed ab[ 	]*vpcmpw k5\{k7\},zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 40 3f ed 7b[ 	]*vpcmpw k5,zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3f 29 7b[ 	]*vpcmpw k5,zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 8d 40 3f ac f0 34 12 00 00 7b[ 	]*vpcmpw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3f 6a 7f 7b[ 	]*vpcmpw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3f aa 00 20 00 00 7b[ 	]*vpcmpw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3f 6a 80 7b[ 	]*vpcmpw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3f aa c0 df ff ff 7b[ 	]*vpcmpw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 40 3e ed ab[ 	]*vpcmpub k5,zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 47 3e ed ab[ 	]*vpcmpub k5\{k7\},zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 40 3e ed 7b[ 	]*vpcmpub k5,zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3e 29 7b[ 	]*vpcmpub k5,zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 0d 40 3e ac f0 34 12 00 00 7b[ 	]*vpcmpub k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3e 6a 7f 7b[ 	]*vpcmpub k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3e aa 00 20 00 00 7b[ 	]*vpcmpub k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3e 6a 80 7b[ 	]*vpcmpub k5,zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 40 3e aa c0 df ff ff 7b[ 	]*vpcmpub k5,zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 40 3e ed ab[ 	]*vpcmpuw k5,zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 47 3e ed ab[ 	]*vpcmpuw k5\{k7\},zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 40 3e ed 7b[ 	]*vpcmpuw k5,zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3e 29 7b[ 	]*vpcmpuw k5,zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 8d 40 3e ac f0 34 12 00 00 7b[ 	]*vpcmpuw k5,zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3e 6a 7f 7b[ 	]*vpcmpuw k5,zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3e aa 00 20 00 00 7b[ 	]*vpcmpuw k5,zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3e 6a 80 7b[ 	]*vpcmpuw k5,zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 40 3e aa c0 df ff ff 7b[ 	]*vpcmpuw k5,zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
#pass
