#objdump: -dwMintel
#name: x86-64 fxsave/fxrstor insns (Intel disassembly)
#source: x86-64-fxsave.s

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	0f ae 00             	fxsave \[rax\]
[ 	]*[a-f0-9]+:	41 0f ae 00          	fxsave \[r8\]
[ 	]*[a-f0-9]+:	41 0f ae 04 00       	fxsave \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	42 0f ae 04 00       	fxsave \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	43 0f ae 04 38       	fxsave \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	48 0f ae 00          	fxsave64 \[rax\]
[ 	]*[a-f0-9]+:	49 0f ae 00          	fxsave64 \[r8\]
[ 	]*[a-f0-9]+:	49 0f ae 04 00       	fxsave64 \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	4a 0f ae 04 00       	fxsave64 \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	0f ae 08             	fxrstor \[rax\]
[ 	]*[a-f0-9]+:	41 0f ae 08          	fxrstor \[r8\]
[ 	]*[a-f0-9]+:	41 0f ae 0c 00       	fxrstor \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	42 0f ae 0c 00       	fxrstor \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	43 0f ae 0c 38       	fxrstor \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	48 0f ae 08          	fxrstor64 \[rax\]
[ 	]*[a-f0-9]+:	49 0f ae 08          	fxrstor64 \[r8\]
[ 	]*[a-f0-9]+:	49 0f ae 0c 00       	fxrstor64 \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	4a 0f ae 0c 00       	fxrstor64 \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	4b 0f ae 0c 38       	fxrstor64 \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	0f ae 00             	fxsave \[rax\]
[ 	]*[a-f0-9]+:	41 0f ae 00          	fxsave \[r8\]
[ 	]*[a-f0-9]+:	41 0f ae 04 00       	fxsave \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	42 0f ae 04 00       	fxsave \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	43 0f ae 04 38       	fxsave \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	48 0f ae 00          	fxsave64 \[rax\]
[ 	]*[a-f0-9]+:	49 0f ae 00          	fxsave64 \[r8\]
[ 	]*[a-f0-9]+:	49 0f ae 04 00       	fxsave64 \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	4a 0f ae 04 00       	fxsave64 \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	0f ae 08             	fxrstor \[rax\]
[ 	]*[a-f0-9]+:	41 0f ae 08          	fxrstor \[r8\]
[ 	]*[a-f0-9]+:	41 0f ae 0c 00       	fxrstor \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	42 0f ae 0c 00       	fxrstor \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	43 0f ae 0c 38       	fxrstor \[r8\+r15\*1\]
[ 	]*[a-f0-9]+:	48 0f ae 08          	fxrstor64 \[rax\]
[ 	]*[a-f0-9]+:	49 0f ae 08          	fxrstor64 \[r8\]
[ 	]*[a-f0-9]+:	49 0f ae 0c 00       	fxrstor64 \[r8\+rax\*1\]
[ 	]*[a-f0-9]+:	4a 0f ae 0c 00       	fxrstor64 \[rax\+r8\*1\]
[ 	]*[a-f0-9]+:	4b 0f ae 0c 38       	fxrstor64 \[r8\+r15\*1\]
#pass
