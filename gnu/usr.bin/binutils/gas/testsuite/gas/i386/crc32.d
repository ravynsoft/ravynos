#objdump: -dw
#name: i386 crc32

.*:     file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	f2 0f 38 f0 06       	crc32b \(%esi\),%eax
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 06    	crc32w \(%esi\),%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f1 06       	crc32l \(%esi\),%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f0 c0       	crc32  %al,%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f0 c0       	crc32  %al,%eax
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 c0    	crc32  %ax,%eax
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 c0    	crc32  %ax,%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f1 c0       	crc32  %eax,%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f1 c0       	crc32  %eax,%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f0 06       	crc32b \(%esi\),%eax
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 06    	crc32w \(%esi\),%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f1 06       	crc32l \(%esi\),%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f0 c0       	crc32  %al,%eax
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 c0    	crc32  %ax,%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f1 c0       	crc32  %eax,%eax
#pass
