#objdump: -dw
#name: i386 arch 4

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	0f ff 07 [ 	]*ud0    \(%edi\),%eax
[ 	]*[a-f0-9]+:	0f b9 07 [ 	]*ud1    \(%edi\),%eax
[ 	]*[a-f0-9]+:	0f 0b                	ud2
[ 	]*[a-f0-9]+:	0f 0b                	ud2
[ 	]*[a-f0-9]+:	0f b9 07 [ 	]*ud1    \(%edi\),%eax
#pass
