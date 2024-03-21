#objdump: -dw
#name: ilp32 MOVDIR[I,64B] insns

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
 +[a-f0-9]+:	48 0f 38 f9 01       	movdiri %rax,\(%rcx\)
 +[a-f0-9]+:	66 0f 38 f8 01       	movdir64b \(%rcx\),%rax
 +[a-f0-9]+:	67 66 0f 38 f8 01    	movdir64b \(%ecx\),%eax
 +[a-f0-9]+:	66 0f 38 f8 0d 00 00 00 00 	movdir64b 0x0\(%rip\),%rcx        #.*
 +[a-f0-9]+:	67 66 0f 38 f8 0d 00 00 00 00 	movdir64b 0x0\(%eip\),%ecx        #.*
 +[a-f0-9]+:	67 66 0f 38 f8 0d 00 00 00 00 	movdir64b 0x0\(%eip\),%ecx        #.*
 +[a-f0-9]+:	67 66 0f 38 f8 0c 25 00 00 00 00 	movdir64b 0x0\(,%eiz,1\),%ecx
 +[a-f0-9]+:	67 66 0f 38 f8 0c 25 78 56 34 12 	movdir64b 0x12345678\(,%eiz,1\),%ecx
 +[a-f0-9]+:	0f 38 f9 01          	movdiri %eax,\(%rcx\)
 +[a-f0-9]+:	48 0f 38 f9 01       	movdiri %rax,\(%rcx\)
 +[a-f0-9]+:	0f 38 f9 01          	movdiri %eax,\(%rcx\)
 +[a-f0-9]+:	48 0f 38 f9 01       	movdiri %rax,\(%rcx\)
 +[a-f0-9]+:	66 0f 38 f8 01       	movdir64b \(%rcx\),%rax
 +[a-f0-9]+:	67 66 0f 38 f8 01    	movdir64b \(%ecx\),%eax
 +[a-f0-9]+:	66 0f 38 f8 0d 00 00 00 00 	movdir64b 0x0\(%rip\),%rcx        #.*
 +[a-f0-9]+:	67 66 0f 38 f8 0d 00 00 00 00 	movdir64b 0x0\(%eip\),%ecx        #.*
 +[a-f0-9]+:	67 66 0f 38 f8 0d 00 00 00 00 	movdir64b 0x0\(%eip\),%ecx        #.*
 +[a-f0-9]+:	67 66 0f 38 f8 0c 25 00 00 00 00 	movdir64b 0x0\(,%eiz,1\),%ecx
 +[a-f0-9]+:	67 66 0f 38 f8 0c 25 78 56 34 12 	movdir64b 0x12345678\(,%eiz,1\),%ecx
#pass
