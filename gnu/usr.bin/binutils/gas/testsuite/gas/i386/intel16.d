#objdump: -dw -mi8086
#name: i386 intel16

.*: +file format .*

Disassembly of section .text:

0+000 <.text>:
   0:	66 0f bf 06 00 00 [ 	]*movswl 0x0,%eax
   6:	66 0f be 06 00 00 [ 	]*movsbl 0x0,%eax
   c:	0f be 06 00 00 [ 	]*movsbw 0x0,%ax
  11:	66 0f b7 06 00 00 [ 	]*movzwl 0x0,%eax
  17:	66 0f b6 06 00 00 [ 	]*movzbl 0x0,%eax
  1d:	0f b6 06 00 00 [ 	]*movzbw 0x0,%ax
  22:	8d 00 [ 	]*lea    \(%bx,%si\),%ax
  24:	8d 02 [ 	]*lea    \(%bp,%si\),%ax
  26:	8d 01 [ 	]*lea    \(%bx,%di\),%ax
  28:	8d 03 [ 	]*lea    \(%bp,%di\),%ax
  2a:	8d 00 [ 	]*lea    \(%bx,%si\),%ax
  2c:	8d 02 [ 	]*lea    \(%bp,%si\),%ax
  2e:	8d 01 [ 	]*lea    \(%bx,%di\),%ax
  30:	8d 03 [ 	]*lea    \(%bp,%di\),%ax
[ 	]*[0-9a-f]+:	67 f7 13[ 	]+notw[ 	]+\(%ebx\)
[ 	]*[0-9a-f]+:	66 f7 17[ 	]+notl[ 	]+\(%bx\)
[ 	]*[0-9a-f]+:	67 0f 1f 03[ 	]+nopw[ 	]+\(%ebx\)
[ 	]*[0-9a-f]+:	66 0f 1f 07[ 	]+nopl[ 	]+\(%bx\)
[ 	]*[0-9a-f]+:	67 83 03 05[ 	]+addw[ 	]+\$0x5,\(%ebx\)
[ 	]*[0-9a-f]+:	66 83 07 05[ 	]+addl[ 	]+\$0x5,\(%bx\)
[ 	]*[0-9a-f]+:	67 c7 03 05 00[ 	]+movw[ 	]+\$0x5,\(%ebx\)
[ 	]*[0-9a-f]+:	66 c7 07 05 00 00 00[ 	]+movl[ 	]+\$0x5,\(%bx\)
#pass
