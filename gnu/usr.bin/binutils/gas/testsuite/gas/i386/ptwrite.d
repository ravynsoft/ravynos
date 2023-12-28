#as:
#objdump: -dw
#name: i386 PTWRITE insns
#source: ptwrite.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite %ecx
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite %ecx
 +[a-f0-9]+:	f3 0f ae 21          	ptwrite \(%ecx\)
 +[a-f0-9]+:	f3 0f ae 21          	ptwrite \(%ecx\)
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite %ecx
 +[a-f0-9]+:	f3 0f ae 21          	ptwrite \(%ecx\)
 +[a-f0-9]+:	f3 0f ae 21          	ptwrite \(%ecx\)
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite %ecx
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite %ecx
 +[a-f0-9]+:	67 f3 0f ae 21       	ptwrite \(%bx,%di\)
 +[a-f0-9]+:	67 f3 0f ae 21       	ptwrite \(%bx,%di\)
 +[a-f0-9]+:	f3 0f ae e1          	ptwrite %ecx
 +[a-f0-9]+:	67 f3 0f ae 21       	ptwrite \(%bx,%di\)
 +[a-f0-9]+:	67 f3 0f ae 21       	ptwrite \(%bx,%di\)
#pass
