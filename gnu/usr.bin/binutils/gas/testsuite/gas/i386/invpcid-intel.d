#as:
#objdump: -dwMintel
#name: i386 INVPCID insns (Intel disassembly)
#source: invpcid.s

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 38 82 10       	invpcid edx,\[eax\]
[ 	]*[a-f0-9]+:	66 0f 38 82 10       	invpcid edx,\[eax\]
[ 	]*[a-f0-9]+:	66 0f 38 82 10       	invpcid edx,\[eax\]
[ 	]*[a-f0-9]+:	67 66 0f 38 82 10    	invpcid edx,\[bx\+si\]
[ 	]*[a-f0-9]+:	67 66 0f 38 82 10    	invpcid edx,\[bx\+si\]
[ 	]*[a-f0-9]+:	67 66 0f 38 82 10    	invpcid edx,\[bx\+si\]
#pass
