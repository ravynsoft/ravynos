#as:
#objdump: -dw
#name: x86_64 PTWRITE insns
#source: x86-64-ptwrite.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite %ecx
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite %ecx
 +[a-f0-9]+:	f3 48 0f ae e1       	ptwrite %rcx
 +[a-f0-9]+:	f3 48 0f ae e1       	ptwrite %rcx
 +[a-f0-9]+:	f3 0f ae 21          	ptwritel \(%rcx\)
 +[a-f0-9]+:	f3 48 0f ae 21       	ptwriteq \(%rcx\)
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite %ecx
 +[a-f0-9]+:	f3 48 0f ae e1       	ptwrite %rcx
 +[a-f0-9]+:	f3 0f ae 21          	ptwritel \(%rcx\)
 +[a-f0-9]+:	f3 48 0f ae 21       	ptwriteq \(%rcx\)
#pass
