#as:
#objdump: -dw -Mintel
#name: x86_64 CLFLUSHOPT insns (Intel disassembly)
#source: x86-64-clflushopt.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 39[ 	]*clflushopt BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*66 42 0f ae bc f0 23 01 00 00[ 	]*clflushopt BYTE PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 39[ 	]*clflushopt BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*66 42 0f ae bc f0 34 12 00 00[ 	]*clflushopt BYTE PTR \[rax\+r14\*8\+0x1234\]
#pass
