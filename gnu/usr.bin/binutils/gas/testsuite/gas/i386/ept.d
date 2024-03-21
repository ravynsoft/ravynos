#objdump: -dw
#name: i386 EPT

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	66 0f 38 80 19       	invept \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	66 0f 38 81 19       	invvpid \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	66 0f 38 80 19       	invept \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	66 0f 38 81 19       	invvpid \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	67 66 0f 38 80 19    	invept \(%bx,%di\),%ebx
[ 	]*[a-f0-9]+:	67 66 0f 38 81 19    	invvpid \(%bx,%di\),%ebx
[ 	]*[a-f0-9]+:	67 66 0f 38 80 19    	invept \(%bx,%di\),%ebx
[ 	]*[a-f0-9]+:	67 66 0f 38 81 19    	invvpid \(%bx,%di\),%ebx
#pass
