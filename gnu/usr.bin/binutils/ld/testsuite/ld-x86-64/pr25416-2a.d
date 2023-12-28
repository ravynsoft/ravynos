#name: X32 GDesc 2
#source: pr25416-2.s
#as: --x32
#ld: -melf32_x86_64 -shared
#objdump: -dw

.*: +file format .*


#...
Disassembly of section .text:

[a-f0-9]+ <_start>:
 +[a-f0-9]+:	48 8d 05 ([0-9a-f]{2} ){4}[ \t]+lea    0x[a-f0-9]+\(%rip\),%rax[ \t]+# [a-f0-9]+ <_GLOBAL_OFFSET_TABLE_\+0x[a-f0-9]+>
 +[a-f0-9]+:	ff 10                	call   \*\(%rax\)
#pass
