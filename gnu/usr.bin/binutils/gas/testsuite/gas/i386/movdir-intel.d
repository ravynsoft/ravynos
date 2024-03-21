#as:
#objdump: -dw -Mintel
#name: i386 MOVDIR[I,64B] insns (Intel disassembly)
#source: movdir.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
 +[a-f0-9]+:	0f 38 f9 01          	movdiri DWORD PTR \[ecx\],eax
 +[a-f0-9]+:	66 0f 38 f8 01       	movdir64b eax,\[ecx\]
 +[a-f0-9]+:	67 66 0f 38 f8 04    	movdir64b ax,\[si\]
 +[a-f0-9]+:	67 66 0f 38 f8 0e 00 00 	movdir64b cx,ds:0x0
 +[a-f0-9]+:	67 66 0f 38 f8 0e 34 12 	movdir64b cx,ds:0x1234
 +[a-f0-9]+:	0f 38 f9 01          	movdiri DWORD PTR \[ecx\],eax
 +[a-f0-9]+:	0f 38 f9 01          	movdiri DWORD PTR \[ecx\],eax
 +[a-f0-9]+:	66 0f 38 f8 01       	movdir64b eax,\[ecx\]
 +[a-f0-9]+:	67 66 0f 38 f8 04    	movdir64b ax,\[si\]
 +[a-f0-9]+:	67 66 0f 38 f8 0e 00 00 	movdir64b cx,ds:0x0
 +[a-f0-9]+:	67 66 0f 38 f8 0e 34 12 	movdir64b cx,ds:0x1234
#pass
