#as:
#objdump: -dw
#name: x86_64 WAITPKG insns
#source: x86-64-waitpkg.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*f3 0f ae f0[ 	]*umonitor %rax
[ 	]*[a-f0-9]+:[ 	]*f3 41 0f ae f2[ 	]*umonitor %r10
[ 	]*[a-f0-9]+:[ 	]*67 f3 41 0f ae f2[ 	]*umonitor %r10d
[ 	]*[a-f0-9]+:[ 	]*f2 0f ae f1[ 	]*umwait %ecx
[ 	]*[a-f0-9]+:[ 	]*f2 41 0f ae f2[ 	]*umwait %r10d
[ 	]*[a-f0-9]+:[ 	]*f2 0f ae f7[ 	]*umwait %edi
[ 	]*[a-f0-9]+:[ 	]*66 0f ae f1[ 	]*tpause %ecx
[ 	]*[a-f0-9]+:[ 	]*66 41 0f ae f2[ 	]*tpause %r10d
[ 	]*[a-f0-9]+:[ 	]*66 0f ae f7[ 	]*tpause %edi
[ 	]*[a-f0-9]+:[ 	]*f2 0f ae f6[ 	]*umwait %esi
[ 	]*[a-f0-9]+:[ 	]*66 0f ae f6[ 	]*tpause %esi
#pass
