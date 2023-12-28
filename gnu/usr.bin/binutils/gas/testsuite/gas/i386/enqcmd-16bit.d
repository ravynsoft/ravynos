#as: -I${srcdir}/$subdir
#objdump: -dw -Mi8086
#name: i386 16-bit ENQCMD[S] insns

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	67 f2 0f 38 f8 01    	enqcmd \(%ecx\),%eax
 +[a-f0-9]+:	f2 0f 38 f8 04       	enqcmd \(%si\),%ax
 +[a-f0-9]+:	67 f3 0f 38 f8 01    	enqcmds \(%ecx\),%eax
 +[a-f0-9]+:	f3 0f 38 f8 04       	enqcmds \(%si\),%ax
 +[a-f0-9]+:	f2 0f 38 f8 0e 00 00 	enqcmd 0x0,%cx
 +[a-f0-9]+:	f2 0f 38 f8 0e 34 12 	enqcmd 0x1234,%cx
 +[a-f0-9]+:	f3 0f 38 f8 0e 00 00 	enqcmds 0x0,%cx
 +[a-f0-9]+:	f3 0f 38 f8 0e 34 12 	enqcmds 0x1234,%cx
 +[a-f0-9]+:	67 f2 0f 38 f8 01    	enqcmd \(%ecx\),%eax
 +[a-f0-9]+:	f2 0f 38 f8 04       	enqcmd \(%si\),%ax
 +[a-f0-9]+:	67 f3 0f 38 f8 01    	enqcmds \(%ecx\),%eax
 +[a-f0-9]+:	f3 0f 38 f8 04       	enqcmds \(%si\),%ax
 +[a-f0-9]+:	f2 0f 38 f8 0e 00 00 	enqcmd 0x0,%cx
 +[a-f0-9]+:	f2 0f 38 f8 0e 34 12 	enqcmd 0x1234,%cx
 +[a-f0-9]+:	f3 0f 38 f8 0e 00 00 	enqcmds 0x0,%cx
 +[a-f0-9]+:	f3 0f 38 f8 0e 34 12 	enqcmds 0x1234,%cx
#pass
