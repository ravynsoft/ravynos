#as:
#objdump: -dw -Mintel
#name: i386 ENQCMD[S] insns (Intel disassembly)
#source: enqcmd.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
 +[a-f0-9]+:	f2 0f 38 f8 01       	enqcmd eax,\[ecx\]
 +[a-f0-9]+:	67 f2 0f 38 f8 04    	enqcmd ax,\[si\]
 +[a-f0-9]+:	f3 0f 38 f8 01       	enqcmds eax,\[ecx\]
 +[a-f0-9]+:	67 f3 0f 38 f8 04    	enqcmds ax,\[si\]
 +[a-f0-9]+:	67 f2 0f 38 f8 0e 00 00 	enqcmd cx,ds:0x0
 +[a-f0-9]+:	67 f2 0f 38 f8 0e 34 12 	enqcmd cx,ds:0x1234
 +[a-f0-9]+:	67 f3 0f 38 f8 0e 00 00 	enqcmds cx,ds:0x0
 +[a-f0-9]+:	67 f3 0f 38 f8 0e 34 12 	enqcmds cx,ds:0x1234
 +[a-f0-9]+:	f2 0f 38 f8 01       	enqcmd eax,\[ecx\]
 +[a-f0-9]+:	67 f2 0f 38 f8 04    	enqcmd ax,\[si\]
 +[a-f0-9]+:	f3 0f 38 f8 01       	enqcmds eax,\[ecx\]
 +[a-f0-9]+:	67 f3 0f 38 f8 04    	enqcmds ax,\[si\]
 +[a-f0-9]+:	67 f2 0f 38 f8 0e 00 00 	enqcmd cx,ds:0x0
 +[a-f0-9]+:	67 f2 0f 38 f8 0e 34 12 	enqcmd cx,ds:0x1234
 +[a-f0-9]+:	67 f3 0f 38 f8 0e 00 00 	enqcmds cx,ds:0x0
 +[a-f0-9]+:	67 f3 0f 38 f8 0e 34 12 	enqcmds cx,ds:0x1234
#pass
