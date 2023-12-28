#as: -O -q
#objdump: -dw
#name: i386 LEA optimizations
#source: lea.s

.*: +file format .*

Disassembly of section .text:

0+ <start>:
[ 	]*[0-9a-f]+:[ 	]+8d 04 08[ 	]+lea[ 	]+\(%eax,%ecx(,1)?\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 04 08[ 	]+lea[ 	]+\(%eax,%ecx(,1)?\),%eax
[ 	]*[0-9a-f]+:[ 	]+26 8d 04 01[ 	]+lea[ 	]+%es:\(%ecx,%eax(,1)?\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 48 01[ 	]+lea[ 	]+0x1\(%eax\),%ecx
[ 	]*[0-9a-f]+:[ 	]+8d 88 00 00 00 00[ 	]+lea[ 	]+0x0\(%eax\),%ecx
[ 	]*[0-9a-f]+:[ 	]+8d 0c 25 00 00 00 00[ 	]+lea[ 	]+0x0\(,(%eiz)?(,1)?\),%ecx
[ 	]*[0-9a-f]+:[ 	]+8d 04 00[ 	]+lea[ 	]+\(%eax,%eax(,1)?\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 04 45 00 00 00 00[ 	]+lea[ 	]+0x0\(,%eax,2\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 04 25 00 00 00 00[ 	]+lea[ 	]+0x0\(,(%eiz)?(,1)?\),%eax
[ 	]*[0-9a-f]+:[ 	]+67 8d 00[ 	]+lea[ 	]+\(%bx,%si\),%eax
[ 	]*[0-9a-f]+:[ 	]+8b c0[ 	]+mov[ 	]+%eax,%eax
[ 	]*[0-9a-f]+:[ 	]+8b c8[ 	]+mov[ 	]+%eax,%ecx
[ 	]*[0-9a-f]+:[ 	]+8b c8[ 	]+mov[ 	]+%eax,%ecx
[ 	]*[0-9a-f]+:[ 	]+8b c8[ 	]+mov[ 	]+%eax,%ecx
[ 	]*[0-9a-f]+:[ 	]+64 8d 08[ 	]+lea[ 	]+%fs:\(%eax\),%ecx
[ 	]*[0-9a-f]+:[ 	]+0f b7 c6[ 	]+movzwl[ 	]+%si,%eax
[ 	]*[0-9a-f]+:[ 	]+0f b7 f6[ 	]+movzwl[ 	]+%si,%esi
[ 	]*[0-9a-f]+:[ 	]+0f b7 c6[ 	]+movzwl[ 	]+%si,%eax
[ 	]*[0-9a-f]+:[ 	]+66 8b c0[ 	]+mov[ 	]+%ax,%ax
[ 	]*[0-9a-f]+:[ 	]+66 8b c8[ 	]+mov[ 	]+%ax,%cx
[ 	]*[0-9a-f]+:[ 	]+66 8b c8[ 	]+mov[ 	]+%ax,%cx
[ 	]*[0-9a-f]+:[ 	]+66 8b c6[ 	]+mov[ 	]+%si,%ax
[ 	]*[0-9a-f]+:[ 	]+66 8b f6[ 	]+mov[ 	]+%si,%si
[ 	]*[0-9a-f]+:[ 	]+8b c9[ 	]+mov[ 	]+%ecx,%ecx
[ 	]*[0-9a-f]+:[ 	]+8b c1[ 	]+mov[ 	]+%ecx,%eax
[ 	]*[0-9a-f]+:[ 	]+b8 01 00 00 00[ 	]+mov[ 	]+\$0x1,%eax
[ 	]*[0-9a-f]+:[ 	]+66 b8 02 00[ 	]+mov[ 	]+\$0x2,%ax
[ 	]*[0-9a-f]+:[ 	]+b8 ff ff ff ff[ 	]+mov[ 	]+\$0xffffffff,%eax
[ 	]*[0-9a-f]+:[ 	]+66 b8 fe ff[ 	]+mov[ 	]+\$0xfffe,%ax
[ 	]*[0-9a-f]+:[ 	]+b8 01 00 00 00[ 	]+mov[ 	]+\$0x1,%eax
[ 	]*[0-9a-f]+:[ 	]+66 b8 02 00[ 	]+mov[ 	]+\$0x2,%ax
[ 	]*[0-9a-f]+:[ 	]+b8 ff ff 00 00[ 	]+mov[ 	]+\$0xffff,%eax
[ 	]*[0-9a-f]+:[ 	]+66 b8 fe ff[ 	]+mov[ 	]+\$0xfffe,%ax
[ 	]*[0-9a-f]+:[ 	]+b8 00 00 00 00[ 	]+mov[ 	]+\$0x0,%eax
[ 	]*[0-9a-f]+:[ 	]+66 8d 05 00 00 00 00[ 	]+lea[ 	]+0x0,%ax
[ 	]*[0-9a-f]+:[ 	]+67 8d 06 00 00[ 	]+lea[ 	]+0x0,%eax
[ 	]*[0-9a-f]+:[ 	]+66 b8 00 00[ 	]+mov[ 	]+\$0x0,%ax
[ 	]*[0-9a-f]+:[ 	]+b8 00 00 00 00[ 	]+mov[ 	]+\$0x0,%eax
[ 	]*[0-9a-f]+:[ 	]+66 b8 00 00[ 	]+mov[ 	]+\$0x0,%ax
#pass
