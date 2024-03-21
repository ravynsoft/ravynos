#as:
#objdump: -dw -Mintel
#name: i386 PTWRITE insns (Intel disassembly)
#source: ptwrite.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite ecx
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite ecx
 +[a-f0-9]+:	f3 0f ae 21          	ptwrite DWORD PTR \[ecx\]
 +[a-f0-9]+:	f3 0f ae 21          	ptwrite DWORD PTR \[ecx\]
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite ecx
 +[a-f0-9]+:	f3 0f ae 21          	ptwrite DWORD PTR \[ecx\]
 +[a-f0-9]+:	f3 0f ae 21          	ptwrite DWORD PTR \[ecx\]
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite ecx
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite ecx
 +[a-f0-9]+:	67 f3 0f ae 21       	ptwrite DWORD PTR \[bx\+di\]
 +[a-f0-9]+:	67 f3 0f ae 21       	ptwrite DWORD PTR \[bx\+di\]
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite ecx
 +[a-f0-9]+:	67 f3 0f ae 21       	ptwrite DWORD PTR \[bx\+di\]
 +[a-f0-9]+:	67 f3 0f ae 21       	ptwrite DWORD PTR \[bx\+di\]
#pass
