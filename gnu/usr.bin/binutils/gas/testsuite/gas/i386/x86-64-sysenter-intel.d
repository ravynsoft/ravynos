#as: -mintel64
#objdump: -dw -Mintel64
#name: x86-64 sysenter (Intel64/Intel64)
#source: x86-64-sysenter-amd.s

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	0f 34                	sysenter
[ 	]*[a-f0-9]+:	0f 35                	sysexitl
[ 	]*[a-f0-9]+:	48 0f 35             	sysexitq
[ 	]*[a-f0-9]+:	0f 34                	sysenter
[ 	]*[a-f0-9]+:	0f 35                	sysexitl
[ 	]*[a-f0-9]+:	48 0f 35             	sysexitq
#pass
