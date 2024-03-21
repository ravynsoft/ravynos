#name: X32 GDesc -> IE 1
#as: --x32
#ld: -melf32_x86_64 -shared
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
 +[a-f0-9]+:	40 8b 05 ([0-9a-f]{2} ){4}[ \t]+rex mov 0x[a-f0-9]+\(%rip\),%eax[ \t]+# [a-f0-9]+ <_DYNAMIC\+0x[a-f0-9]+>
 +[a-f0-9]+:	0f 1f 00             	nopl   \(%rax\)
 +[a-f0-9]+:	64 8b 0c 25 00 00 00 00 	mov    %fs:0x0,%ecx
 +[a-f0-9]+:	40 03 0d ([0-9a-f]{2} ){4}[ \t]+rex add 0x[a-f0-9]+\(%rip\),%ecx[ \t]+# [a-f0-9]+ <_DYNAMIC\+0x[a-f0-9]+>
#pass
