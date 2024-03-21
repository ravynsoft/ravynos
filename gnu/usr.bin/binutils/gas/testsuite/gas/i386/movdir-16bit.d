#as: -I${srcdir}/$subdir
#objdump: -dw -Mi8086
#name: i386 16-bit MOVDIR[I,64B] insns

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	67 0f 38 f9 01       	movdiri %eax,\(%ecx\)
 +[a-f0-9]+:	67 66 0f 38 f8 01    	movdir64b \(%ecx\),%eax
 +[a-f0-9]+:	66 0f 38 f8 04       	movdir64b \(%si\),%ax
 +[a-f0-9]+:	66 0f 38 f8 0e 00 00 	movdir64b 0x0,%cx
 +[a-f0-9]+:	66 0f 38 f8 0e 34 12 	movdir64b 0x1234,%cx
 +[a-f0-9]+:	67 0f 38 f9 01       	movdiri %eax,\(%ecx\)
 +[a-f0-9]+:	67 0f 38 f9 01       	movdiri %eax,\(%ecx\)
 +[a-f0-9]+:	67 66 0f 38 f8 01    	movdir64b \(%ecx\),%eax
 +[a-f0-9]+:	66 0f 38 f8 04       	movdir64b \(%si\),%ax
 +[a-f0-9]+:	66 0f 38 f8 0e 00 00 	movdir64b 0x0,%cx
 +[a-f0-9]+:	66 0f 38 f8 0e 34 12 	movdir64b 0x1234,%cx
#pass
