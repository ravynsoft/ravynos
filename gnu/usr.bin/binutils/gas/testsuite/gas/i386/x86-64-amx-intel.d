#as:
#objdump: -d -Mintel
#name: x86_64 AMX insns in Intel syntax
#source: x86-64-amx.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*c4 e2 78 49 04 51[ 	]*ldtilecfg \[rcx\+rdx\*2\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 79 49 04 51[ 	]*sttilecfg \[rcx\+rdx\*2\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 52 5c dc[ 	]*tdpbf16ps tmm3,tmm4,tmm5
[ 	]*[a-f0-9]+:[ 	]*c4 e2 63 5e ca[ 	]*tdpbssd tmm1,tmm2,tmm3
[ 	]*[a-f0-9]+:[ 	]*c4 e2 62 5e ca[ 	]*tdpbsud tmm1,tmm2,tmm3
[ 	]*[a-f0-9]+:[ 	]*c4 e2 61 5e ca[ 	]*tdpbusd tmm1,tmm2,tmm3
[ 	]*[a-f0-9]+:[ 	]*c4 e2 60 5e ca[ 	]*tdpbuud tmm1,tmm2,tmm3
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 4b 2c 25 00[ 	]*tileloadd tmm5,ds:0x0
[ 	]*[a-f0-9]+:[ 	]*00 00 00[ 	]*
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 4b 2c 21[ 	]*tileloadd tmm5,\[rcx\+riz\*1\]
[ 	]*[a-f0-9]+:[ 	]*67 c4 e2 7b 4b 2c 21[ 	]*tileloadd tmm5,\[ecx\+eiz\*1\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 4b 2c 11[ 	]*tileloadd tmm5,\[rcx\+rdx\*1\]
[ 	]*[a-f0-9]+:[ 	]*67 c4 e2 7b 4b 0c 51[ 	]*tileloadd tmm1,\[ecx\+edx\*2\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 79 4b 2c 25 00[ 	]*tileloaddt1 tmm5,ds:0x0
[ 	]*[a-f0-9]+:[ 	]*00 00 00[ 	]*
[ 	]*[a-f0-9]+:[ 	]*c4 e2 79 4b 2c 21[ 	]*tileloaddt1 tmm5,\[rcx\+riz\*1\]
[ 	]*[a-f0-9]+:[ 	]*67 c4 e2 79 4b 2c 21[ 	]*tileloaddt1 tmm5,\[ecx\+eiz\*1\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 79 4b 2c 11[ 	]*tileloaddt1 tmm5,\[rcx\+rdx\*1\]
[ 	]*[a-f0-9]+:[ 	]*67 c4 e2 79 4b 0c 51[ 	]*tileloaddt1 tmm1,\[ecx\+edx\*2\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 79 4b 0c 61[ 	]*tileloaddt1 tmm1,\[rcx\+riz\*2\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 78 49 c0[ 	]*tilerelease
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7a 4b 2c 21[ 	]*tilestored \[rcx\+riz\*1\],tmm5
[ 	]*[a-f0-9]+:[ 	]*67 c4 e2 7a 4b 2c 21[ 	]*tilestored \[ecx\+eiz\*1\],tmm5
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7a 4b 2c 11[ 	]*tilestored \[rcx\+rdx\*1\],tmm5
[ 	]*[a-f0-9]+:[ 	]*67 c4 e2 7a 4b 0c 51[ 	]*tilestored \[ecx\+edx\*2\],tmm1
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 49 c0[ 	]*tilezero tmm0
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 49 e8[ 	]*tilezero tmm5
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 49 f8[ 	]*tilezero tmm7
[ 	]*[a-f0-9]+:[ 	]*c4 e2 78 49 01[ 	]*ldtilecfg \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 78 49 03[ 	]*ldtilecfg \[rbx\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 79 49 01[ 	]*sttilecfg \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 79 49 03[ 	]*sttilecfg \[rbx\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 52 5c dc[ 	]*tdpbf16ps tmm3,tmm4,tmm5
[ 	]*[a-f0-9]+:[ 	]*c4 e2 63 5e ca[ 	]*tdpbssd tmm1,tmm2,tmm3
[ 	]*[a-f0-9]+:[ 	]*c4 e2 62 5e ca[ 	]*tdpbsud tmm1,tmm2,tmm3
[ 	]*[a-f0-9]+:[ 	]*c4 e2 61 5e ca[ 	]*tdpbusd tmm1,tmm2,tmm3
[ 	]*[a-f0-9]+:[ 	]*c4 e2 60 5e ca[ 	]*tdpbuud tmm1,tmm2,tmm3
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 4b 2c 25 00[ 	]*tileloadd tmm5,ds:0x0
[ 	]*[a-f0-9]+:[ 	]*00 00 00[ 	]*
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 4b 2c 21[ 	]*tileloadd tmm5,\[rcx\+riz\*1\]
[ 	]*[a-f0-9]+:[ 	]*67 c4 e2 7b 4b 2c 21[ 	]*tileloadd tmm5,\[ecx\+eiz\*1\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 4b 2c 11[ 	]*tileloadd tmm5,\[rcx\+rdx\*1\]
[ 	]*[a-f0-9]+:[ 	]*67 c4 e2 7b 4b 0c 51[ 	]*tileloadd tmm1,\[ecx\+edx\*2\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 79 4b 2c 25 00[ 	]*tileloaddt1 tmm5,ds:0x0
[ 	]*[a-f0-9]+:[ 	]*00 00 00[ 	]*
[ 	]*[a-f0-9]+:[ 	]*c4 e2 79 4b 2c 21[ 	]*tileloaddt1 tmm5,\[rcx\+riz\*1\]
[ 	]*[a-f0-9]+:[ 	]*67 c4 e2 79 4b 2c 21[ 	]*tileloaddt1 tmm5,\[ecx\+eiz\*1\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 79 4b 2c 11[ 	]*tileloaddt1 tmm5,\[rcx\+rdx\*1\]
[ 	]*[a-f0-9]+:[ 	]*67 c4 e2 79 4b 0c 51[ 	]*tileloaddt1 tmm1,\[ecx\+edx\*2\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 79 4b 0c 61[ 	]*tileloaddt1 tmm1,\[rcx\+riz\*2\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 78 49 c0[ 	]*tilerelease
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7a 4b 2c 21[ 	]*tilestored \[rcx\+riz\*1\],tmm5
[ 	]*[a-f0-9]+:[ 	]*67 c4 e2 7a 4b 2c 21[ 	]*tilestored \[ecx\+eiz\*1\],tmm5
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7a 4b 2c 11[ 	]*tilestored \[rcx\+rdx\*1\],tmm5
[ 	]*[a-f0-9]+:[ 	]*67 c4 e2 7a 4b 0c 51[ 	]*tilestored \[ecx\+edx\*2\],tmm1
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 49 c0[ 	]*tilezero tmm0
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 49 e8[ 	]*tilezero tmm5
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 49 f8[ 	]*tilezero tmm7
#pass
