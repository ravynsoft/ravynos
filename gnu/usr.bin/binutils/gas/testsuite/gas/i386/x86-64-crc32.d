#objdump: -dw
#name: x86-64 crc32

.*:     file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	f2 0f 38 f0 06       	crc32b \(%rsi\),%eax
[ 	]*[a-f0-9]+:	f2 48 0f 38 f0 06    	crc32b \(%rsi\),%rax
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 06    	crc32w \(%rsi\),%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f1 06       	crc32l \(%rsi\),%eax
[ 	]*[a-f0-9]+:	f2 48 0f 38 f1 06    	crc32q \(%rsi\),%rax
[ 	]*[a-f0-9]+:	f2 0f 38 f0 c0       	crc32  %al,%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f0 c0       	crc32  %al,%eax
[ 	]*[a-f0-9]+:	f2 48 0f 38 f0 c0    	crc32  %al,%rax
[ 	]*[a-f0-9]+:	f2 48 0f 38 f0 c0    	crc32  %al,%rax
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 c0    	crc32  %ax,%eax
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 c0    	crc32  %ax,%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f1 c0       	crc32  %eax,%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f1 c0       	crc32  %eax,%eax
[ 	]*[a-f0-9]+:	f2 48 0f 38 f1 c0    	crc32  %rax,%rax
[ 	]*[a-f0-9]+:	f2 48 0f 38 f1 c0    	crc32  %rax,%rax
[ 	]*[a-f0-9]+:	f2 48 0f 38 f0 06    	crc32b \(%rsi\),%rax
[ 	]*[a-f0-9]+:	f2 0f 38 f0 06       	crc32b \(%rsi\),%eax
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 06    	crc32w \(%rsi\),%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f1 06       	crc32l \(%rsi\),%eax
[ 	]*[a-f0-9]+:	f2 48 0f 38 f1 06    	crc32q \(%rsi\),%rax
[ 	]*[a-f0-9]+:	f2 0f 38 f0 c0       	crc32  %al,%eax
[ 	]*[a-f0-9]+:	f2 48 0f 38 f0 c0    	crc32  %al,%rax
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 c0    	crc32  %ax,%eax
[ 	]*[a-f0-9]+:	f2 0f 38 f1 c0       	crc32  %eax,%eax
[ 	]*[a-f0-9]+:	f2 48 0f 38 f1 c0    	crc32  %rax,%rax
#pass
