#name: X32 GDesc -> IE 2
#as: --x32
#ld: -melf32_x86_64 -shared
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
 +[a-f0-9]+:	48 8b 05 ([0-9a-f]{2} ){4}[ \t]+mov    0x[a-f0-9]+\(%rip\),%rax[ \t]+# [a-f0-9]+ <_DYNAMIC\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 90                	xchg   %ax,%ax
 +[a-f0-9]+:	64 8b 0c 25 00 00 00 00 	mov    %fs:0x0,%ecx
 +[a-f0-9]+:	40 03 0d ([0-9a-f]{2} ){4}[ \t]+rex add 0x[a-f0-9]+\(%rip\),%ecx[ \t]+# [a-f0-9]+ <_DYNAMIC\+0x[a-f0-9]+>
#pass
