#objdump: -dwMintel
#name: i386 FSGSBase (Intel disassembly)
#source: fsgs.s

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	f3 0f ae c3          	rdfsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae cb          	rdgsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae d3          	wrfsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae db          	wrgsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae c3          	rdfsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae cb          	rdgsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae d3          	wrfsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae db          	wrgsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae c3          	rdfsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae cb          	rdgsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae d3          	wrfsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae db          	wrgsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae c3          	rdfsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae cb          	rdgsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae d3          	wrfsbase ebx
[ 	]*[a-f0-9]+:	f3 0f ae db          	wrgsbase ebx
#pass
