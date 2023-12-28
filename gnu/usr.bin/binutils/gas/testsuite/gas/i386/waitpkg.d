#as:
#objdump: -dw
#name: i386 WAITPKG insns
#source: waitpkg.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*f3 0f ae f0[ 	]*umonitor %eax
[ 	]*[a-f0-9]+:[ 	]*67 f3 0f ae f1[ 	]*umonitor %cx
[ 	]*[a-f0-9]+:[ 	]*f2 0f ae f1[ 	]*umwait %ecx
[ 	]*[a-f0-9]+:[ 	]*f2 0f ae f3[ 	]*umwait %ebx
[ 	]*[a-f0-9]+:[ 	]*66 0f ae f1[ 	]*tpause %ecx
[ 	]*[a-f0-9]+:[ 	]*66 0f ae f3[ 	]*tpause %ebx
[ 	]*[a-f0-9]+:[ 	]*f2 0f ae f7[ 	]*umwait %edi
[ 	]*[a-f0-9]+:[ 	]*66 0f ae f7[ 	]*tpause %edi
[ 	]*[a-f0-9]+:[ 	]*67 f3 0f ae f0[ 	]*umonitor %ax
[ 	]*[a-f0-9]+:[ 	]*f3 0f ae f1[ 	]*umonitor %ecx
[ 	]*[a-f0-9]+:[ 	]*f2 0f ae f1[ 	]*umwait %ecx
[ 	]*[a-f0-9]+:[ 	]*f2 0f ae f3[ 	]*umwait %ebx
[ 	]*[a-f0-9]+:[ 	]*66 0f ae f1[ 	]*tpause %ecx
[ 	]*[a-f0-9]+:[ 	]*66 0f ae f3[ 	]*tpause %ebx
[ 	]*[a-f0-9]+:[ 	]*f2 0f ae f7[ 	]*umwait %edi
[ 	]*[a-f0-9]+:[ 	]*66 0f ae f7[ 	]*tpause %edi
#pass
