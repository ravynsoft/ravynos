#objdump: -dw
#name: i386 RDPID insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*f3 0f c7 f8[ 	]*rdpid  %eax
[ 	]*[a-f0-9]+:[ 	]*f3 0f c7 f9[ 	]*rdpid  %ecx
#pass
