#objdump: -dw
#name: i386 control/debug register with ignored MOD field

.*: +file format .*

Disassembly of section .text:

0+ <start>:
[ 	]*[a-f0-9]+:	0f 22 1f             	mov    %edi,%cr3
[ 	]*[a-f0-9]+:	0f 20 1f             	mov    %cr3,%edi
[ 	]*[a-f0-9]+:	0f 21 1f             	mov    %db3,%edi
[ 	]*[a-f0-9]+:	0f 23 1f             	mov    %edi,%db3
#pass
