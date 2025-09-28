#as:
#objdump: -dw -Mintel
#name: x86_64 XSAVES insns (Intel disassembly)
#source: x86-64-xsaves.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*48 0f c7 29[ 	]*xsaves64 \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*4a 0f c7 ac f0 23 01 00 00[ 	]*xsaves64 \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*48 0f c7 19[ 	]*xrstors64 \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*4a 0f c7 9c f0 23 01 00 00[ 	]*xrstors64 \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*48 0f c7 29[ 	]*xsaves64 \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*4a 0f c7 ac f0 34 12 00 00[ 	]*xsaves64 \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*48 0f c7 19[ 	]*xrstors64 \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*4a 0f c7 9c f0 34 12 00 00[ 	]*xrstors64 \[rax\+r14\*8\+0x1234\]
#pass
