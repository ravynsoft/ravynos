#objdump: -drwMintel
#name: x86-64 FSGSBase (Intel mode)
#source: x86-64-fsgs.s

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	f3 0f ae c3          	rdfsbase ebx
[ 	]*[a-f0-9]+:	f3 48 0f ae c3       	rdfsbase rbx
[ 	]*[a-f0-9]+:	f3 41 0f ae c0       	rdfsbase r8d
[ 	]*[a-f0-9]+:	f3 49 0f ae c0       	rdfsbase r8
[ 	]*[a-f0-9]+:	f3 0f ae cb          	rdgsbase ebx
[ 	]*[a-f0-9]+:	f3 48 0f ae cb       	rdgsbase rbx
[ 	]*[a-f0-9]+:	f3 41 0f ae c8       	rdgsbase r8d
[ 	]*[a-f0-9]+:	f3 49 0f ae c8       	rdgsbase r8
[ 	]*[a-f0-9]+:	f3 0f ae d3          	wrfsbase ebx
[ 	]*[a-f0-9]+:	f3 48 0f ae d3       	wrfsbase rbx
[ 	]*[a-f0-9]+:	f3 41 0f ae d0       	wrfsbase r8d
[ 	]*[a-f0-9]+:	f3 49 0f ae d0       	wrfsbase r8
[ 	]*[a-f0-9]+:	f3 0f ae db          	wrgsbase ebx
[ 	]*[a-f0-9]+:	f3 48 0f ae db       	wrgsbase rbx
[ 	]*[a-f0-9]+:	f3 41 0f ae d8       	wrgsbase r8d
[ 	]*[a-f0-9]+:	f3 49 0f ae d8       	wrgsbase r8
[ 	]*[a-f0-9]+:	f3 0f ae c3          	rdfsbase ebx
[ 	]*[a-f0-9]+:	f3 48 0f ae c3       	rdfsbase rbx
[ 	]*[a-f0-9]+:	f3 41 0f ae c0       	rdfsbase r8d
[ 	]*[a-f0-9]+:	f3 49 0f ae c0       	rdfsbase r8
[ 	]*[a-f0-9]+:	f3 0f ae cb          	rdgsbase ebx
[ 	]*[a-f0-9]+:	f3 48 0f ae cb       	rdgsbase rbx
[ 	]*[a-f0-9]+:	f3 41 0f ae c8       	rdgsbase r8d
[ 	]*[a-f0-9]+:	f3 49 0f ae c8       	rdgsbase r8
[ 	]*[a-f0-9]+:	f3 0f ae d3          	wrfsbase ebx
[ 	]*[a-f0-9]+:	f3 48 0f ae d3       	wrfsbase rbx
[ 	]*[a-f0-9]+:	f3 41 0f ae d0       	wrfsbase r8d
[ 	]*[a-f0-9]+:	f3 49 0f ae d0       	wrfsbase r8
[ 	]*[a-f0-9]+:	f3 0f ae db          	wrgsbase ebx
[ 	]*[a-f0-9]+:	f3 48 0f ae db       	wrgsbase rbx
[ 	]*[a-f0-9]+:	f3 41 0f ae d8       	wrgsbase r8d
[ 	]*[a-f0-9]+:	f3 49 0f ae d8       	wrgsbase r8
#pass
