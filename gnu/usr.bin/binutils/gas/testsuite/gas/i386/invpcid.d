#as:
#objdump: -dw
#name: i386 INVPCID insns

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 38 82 10       	invpcid \(%eax\),%edx
[ 	]*[a-f0-9]+:	66 0f 38 82 10       	invpcid \(%eax\),%edx
[ 	]*[a-f0-9]+:	66 0f 38 82 10       	invpcid \(%eax\),%edx
[ 	]*[a-f0-9]+:	67 66 0f 38 82 10    	invpcid \(%bx,%si\),%edx
[ 	]*[a-f0-9]+:	67 66 0f 38 82 10    	invpcid \(%bx,%si\),%edx
[ 	]*[a-f0-9]+:	67 66 0f 38 82 10    	invpcid \(%bx,%si\),%edx
#pass
