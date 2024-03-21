#source: x86-64-suffix.s
#objdump: -dw -Msuffix,intel
#name: x86-64 suffix (Intel mode)
#warning_output: x86-64-suffix.e

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	0f 01 c8             	monitor
[ 	]*[a-f0-9]+:	0f 01 c9             	mwait
[ 	]*[a-f0-9]+:	0f 01 c1             	vmcall
[ 	]*[a-f0-9]+:	0f 01 c2             	vmlaunch
[ 	]*[a-f0-9]+:	0f 01 c3             	vmresume
[ 	]*[a-f0-9]+:	0f 01 c4             	vmxoff
[ 	]*[a-f0-9]+:	66 cf                	iretw
[ 	]*[a-f0-9]+:	cf                   	iretd
[ 	]*[a-f0-9]+:	48 cf                	iretq
[ 	]*[a-f0-9]+:	0f 07                	sysretd
[ 	]*[a-f0-9]+:	48 89 e5             	mov    rbp,rsp
[ 	]*[a-f0-9]+:	48 0f 07             	sysretq
[ 	]*[a-f0-9]+:	66 cf                	iretw
[ 	]*[a-f0-9]+:	cf                   	iretd
[ 	]*[a-f0-9]+:	cf                   	iretd
[ 	]*[a-f0-9]+:	48 cf                	iretq
[ 	]*[a-f0-9]+:	0f 07                	sysretd
[ 	]*[a-f0-9]+:	48 89 e5             	mov    rbp,rsp
[ 	]*[a-f0-9]+:	48 0f 07             	sysretq
#pass
