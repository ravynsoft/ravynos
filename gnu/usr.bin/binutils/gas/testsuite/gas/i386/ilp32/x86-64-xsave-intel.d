#source: ../x86-64-xsave.s
#as: -J
#objdump: -dw -Mintel
#name: x86-64 (ILP32) xsave (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 01 d0             	xgetbv
[ 	]*[a-f0-9]+:	0f 01 d1             	xsetbv
[ 	]*[a-f0-9]+:	0f ae 20             	xsave  \[rax\]
[ 	]*[a-f0-9]+:	41 0f ae 20          	xsave  \[r8\]
[ 	]*[a-f0-9]+:	41 0f ae 24 00       	xsave  \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	42 0f ae 24 00       	xsave  \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	43 0f ae 24 38       	xsave  \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	48 0f ae 20          	xsave64 \[rax\]
[ 	]*[a-f0-9]+:	49 0f ae 20          	xsave64 \[r8\]
[ 	]*[a-f0-9]+:	49 0f ae 24 00       	xsave64 \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	4a 0f ae 24 00       	xsave64 \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	0f ae 28             	xrstor \[rax\]
[ 	]*[a-f0-9]+:	41 0f ae 28          	xrstor \[r8\]
[ 	]*[a-f0-9]+:	41 0f ae 2c 00       	xrstor \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	42 0f ae 2c 00       	xrstor \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	43 0f ae 2c 38       	xrstor \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	48 0f ae 28          	xrstor64 \[rax\]
[ 	]*[a-f0-9]+:	49 0f ae 28          	xrstor64 \[r8\]
[ 	]*[a-f0-9]+:	49 0f ae 2c 00       	xrstor64 \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	4a 0f ae 2c 00       	xrstor64 \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	4b 0f ae 2c 38       	xrstor64 \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	0f ae 30             	xsaveopt \[rax\]
[ 	]*[a-f0-9]+:	41 0f ae 30          	xsaveopt \[r8\]
[ 	]*[a-f0-9]+:	41 0f ae 34 00       	xsaveopt \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	42 0f ae 34 00       	xsaveopt \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	43 0f ae 34 38       	xsaveopt \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	48 0f ae 30          	xsaveopt64 \[rax\]
[ 	]*[a-f0-9]+:	49 0f ae 30          	xsaveopt64 \[r8\]
[ 	]*[a-f0-9]+:	49 0f ae 34 00       	xsaveopt64 \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	4a 0f ae 34 00       	xsaveopt64 \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	0f ae 20             	xsave  \[rax\]
[ 	]*[a-f0-9]+:	41 0f ae 20          	xsave  \[r8\]
[ 	]*[a-f0-9]+:	41 0f ae 24 00       	xsave  \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	42 0f ae 24 00       	xsave  \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	43 0f ae 24 38       	xsave  \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	48 0f ae 20          	xsave64 \[rax\]
[ 	]*[a-f0-9]+:	49 0f ae 20          	xsave64 \[r8\]
[ 	]*[a-f0-9]+:	49 0f ae 24 00       	xsave64 \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	4a 0f ae 24 00       	xsave64 \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	0f ae 28             	xrstor \[rax\]
[ 	]*[a-f0-9]+:	41 0f ae 28          	xrstor \[r8\]
[ 	]*[a-f0-9]+:	41 0f ae 2c 00       	xrstor \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	42 0f ae 2c 00       	xrstor \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	43 0f ae 2c 38       	xrstor \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	48 0f ae 28          	xrstor64 \[rax\]
[ 	]*[a-f0-9]+:	49 0f ae 28          	xrstor64 \[r8\]
[ 	]*[a-f0-9]+:	49 0f ae 2c 00       	xrstor64 \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	4a 0f ae 2c 00       	xrstor64 \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	4b 0f ae 2c 38       	xrstor64 \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	0f ae 30             	xsaveopt \[rax\]
[ 	]*[a-f0-9]+:	41 0f ae 30          	xsaveopt \[r8\]
[ 	]*[a-f0-9]+:	41 0f ae 34 00       	xsaveopt \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	42 0f ae 34 00       	xsaveopt \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	43 0f ae 34 38       	xsaveopt \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	48 0f ae 30          	xsaveopt64 \[rax\]
[ 	]*[a-f0-9]+:	49 0f ae 30          	xsaveopt64 \[r8\]
[ 	]*[a-f0-9]+:	49 0f ae 34 00       	xsaveopt64 \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	4a 0f ae 34 00       	xsaveopt64 \[rax\+r8\*1\]
#pass
