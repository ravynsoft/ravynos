#objdump: -dw
#name: ilp32 ENQCMD[S] insns

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
 +[a-f0-9]+:	f2 0f 38 f8 01       	enqcmd \(%rcx\),%rax
 +[a-f0-9]+:	67 f2 0f 38 f8 01    	enqcmd \(%ecx\),%eax
 +[a-f0-9]+:	f3 0f 38 f8 01       	enqcmds \(%rcx\),%rax
 +[a-f0-9]+:	67 f3 0f 38 f8 01    	enqcmds \(%ecx\),%eax
 +[a-f0-9]+:	f2 0f 38 f8 0d 00 00 00 00 	enqcmd 0x0\(%rip\),%rcx        #.*
 +[a-f0-9]+:	67 f2 0f 38 f8 0d 00 00 00 00 	enqcmd 0x0\(%eip\),%ecx        #.*
 +[a-f0-9]+:	67 f2 0f 38 f8 0d 00 00 00 00 	enqcmd 0x0\(%eip\),%ecx        #.*
 +[a-f0-9]+:	f3 0f 38 f8 0d 00 00 00 00 	enqcmds 0x0\(%rip\),%rcx        #.*
 +[a-f0-9]+:	67 f3 0f 38 f8 0d 00 00 00 00 	enqcmds 0x0\(%eip\),%ecx        #.*
 +[a-f0-9]+:	67 f3 0f 38 f8 0d 00 00 00 00 	enqcmds 0x0\(%eip\),%ecx        #.*
 +[a-f0-9]+:	67 f2 0f 38 f8 0c 25 00 00 00 00 	enqcmd 0x0\(,%eiz,1\),%ecx
 +[a-f0-9]+:	67 f2 0f 38 f8 0c 25 78 56 34 12 	enqcmd 0x12345678\(,%eiz,1\),%ecx
 +[a-f0-9]+:	67 f3 0f 38 f8 0c 25 00 00 00 00 	enqcmds 0x0\(,%eiz,1\),%ecx
 +[a-f0-9]+:	67 f3 0f 38 f8 0c 25 78 56 34 12 	enqcmds 0x12345678\(,%eiz,1\),%ecx
 +[a-f0-9]+:	f2 0f 38 f8 01       	enqcmd \(%rcx\),%rax
 +[a-f0-9]+:	67 f2 0f 38 f8 01    	enqcmd \(%ecx\),%eax
 +[a-f0-9]+:	f3 0f 38 f8 01       	enqcmds \(%rcx\),%rax
 +[a-f0-9]+:	67 f3 0f 38 f8 01    	enqcmds \(%ecx\),%eax
 +[a-f0-9]+:	f2 0f 38 f8 0d 00 00 00 00 	enqcmd 0x0\(%rip\),%rcx        #.*
 +[a-f0-9]+:	67 f2 0f 38 f8 0d 00 00 00 00 	enqcmd 0x0\(%eip\),%ecx        #.*
 +[a-f0-9]+:	67 f2 0f 38 f8 0d 00 00 00 00 	enqcmd 0x0\(%eip\),%ecx        #.*
 +[a-f0-9]+:	f3 0f 38 f8 0d 00 00 00 00 	enqcmds 0x0\(%rip\),%rcx        #.*
 +[a-f0-9]+:	67 f3 0f 38 f8 0d 00 00 00 00 	enqcmds 0x0\(%eip\),%ecx        #.*
 +[a-f0-9]+:	67 f3 0f 38 f8 0d 00 00 00 00 	enqcmds 0x0\(%eip\),%ecx        #.*
 +[a-f0-9]+:	67 f2 0f 38 f8 0c 25 00 00 00 00 	enqcmd 0x0\(,%eiz,1\),%ecx
 +[a-f0-9]+:	67 f2 0f 38 f8 0c 25 78 56 34 12 	enqcmd 0x12345678\(,%eiz,1\),%ecx
 +[a-f0-9]+:	67 f3 0f 38 f8 0c 25 00 00 00 00 	enqcmds 0x0\(,%eiz,1\),%ecx
 +[a-f0-9]+:	67 f3 0f 38 f8 0c 25 78 56 34 12 	enqcmds 0x12345678\(,%eiz,1\),%ecx
#pass
