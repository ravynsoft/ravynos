#as: -O -q -I${srcdir}/$subdir
#objdump: -dw -Mi8086
#name: i386 16-bit LEA optimizations
#source: lea16.s

.*: +file format .*

Disassembly of section .text:

0+ <start>:
[ 	]*[0-9a-f]+:[ 	]+67 66 8d 04 08[ 	]+lea[ 	]+\(%eax,%ecx(,1)?\),%eax
[ 	]*[0-9a-f]+:[ 	]+67 66 8d 04 08[ 	]+lea[ 	]+\(%eax,%ecx(,1)?\),%eax
[ 	]*[0-9a-f]+:[ 	]+26 67 66 8d 04 01[ 	]+lea[ 	]+%es:\(%ecx,%eax(,1)?\),%eax
[ 	]*[0-9a-f]+:[ 	]+67 66 8d 48 01[ 	]+lea[ 	]+0x1\(%eax\),%ecx
[ 	]*[0-9a-f]+:[ 	]+67 66 8d 88 00 00 00 00[ 	]+lea[ 	]+0x0\(%eax\),%ecx
[ 	]*[0-9a-f]+:[ 	]+67 66 8d 0c 25 00 00 00 00[ 	]+addr32 lea[ 	]+0x0,%ecx
[ 	]*[0-9a-f]+:[ 	]+67 66 8d 04 00[ 	]+lea[ 	]+\(%eax,%eax(,1)?\),%eax
[ 	]*[0-9a-f]+:[ 	]+67 66 8d 04 45 00 00 00 00[ 	]+lea[ 	]+0x0\(,%eax,2\),%eax
[ 	]*[0-9a-f]+:[ 	]+67 66 8d 04 25 00 00 00 00[ 	]+addr32 lea[ 	]+0x0,%eax
[ 	]*[0-9a-f]+:[ 	]+66 8d 00[ 	]+lea[ 	]+\(%bx,%si\),%eax
[ 	]*[0-9a-f]+:[ 	]+66 8b c0[ 	]+mov[ 	]+%eax,%eax
[ 	]*[0-9a-f]+:[ 	]+66 8b c8[ 	]+mov[ 	]+%eax,%ecx
[ 	]*[0-9a-f]+:[ 	]+66 8b c8[ 	]+mov[ 	]+%eax,%ecx
[ 	]*[0-9a-f]+:[ 	]+66 8b c8[ 	]+mov[ 	]+%eax,%ecx
[ 	]*[0-9a-f]+:[ 	]+64 67 66 8d 08[ 	]+lea[ 	]+%fs:\(%eax\),%ecx
[ 	]*[0-9a-f]+:[ 	]+66 8d 04[ 	]+lea[ 	]+\(%si\),%eax
[ 	]*[0-9a-f]+:[ 	]+66 8d 34[ 	]+lea[ 	]+\(%si\),%esi
[ 	]*[0-9a-f]+:[ 	]+66 8d 04[ 	]+lea[ 	]+\(%si\),%eax
[ 	]*[0-9a-f]+:[ 	]+8b c0[ 	]+mov[ 	]+%ax,%ax
[ 	]*[0-9a-f]+:[ 	]+8b c8[ 	]+mov[ 	]+%ax,%cx
[ 	]*[0-9a-f]+:[ 	]+8b c8[ 	]+mov[ 	]+%ax,%cx
[ 	]*[0-9a-f]+:[ 	]+8b c6[ 	]+mov[ 	]+%si,%ax
[ 	]*[0-9a-f]+:[ 	]+8b f6[ 	]+mov[ 	]+%si,%si
[ 	]*[0-9a-f]+:[ 	]+66 8b c9[ 	]+mov[ 	]+%ecx,%ecx
[ 	]*[0-9a-f]+:[ 	]+66 8b c1[ 	]+mov[ 	]+%ecx,%eax
[ 	]*[0-9a-f]+:[ 	]+66 8d 06 01 00[ 	]+lea[ 	]+0x1,%eax
[ 	]*[0-9a-f]+:[ 	]+b8 02 00[ 	]+mov[ 	]+\$0x2,%ax
[ 	]*[0-9a-f]+:[ 	]+66 8d 06 ff ff[ 	]+lea[ 	]+-0x1,%eax
[ 	]*[0-9a-f]+:[ 	]+b8 fe ff[ 	]+mov[ 	]+\$0xfffe,%ax
[ 	]*[0-9a-f]+:[ 	]+66 b8 01 00 00 00[ 	]+mov[ 	]+\$0x1,%eax
[ 	]*[0-9a-f]+:[ 	]+b8 02 00[ 	]+mov[ 	]+\$0x2,%ax
[ 	]*[0-9a-f]+:[ 	]+66 b8 ff ff ff ff[ 	]+mov[ 	]+\$0xffffffff,%eax
[ 	]*[0-9a-f]+:[ 	]+b8 fe ff[ 	]+mov[ 	]+\$0xfffe,%ax
[ 	]*[0-9a-f]+:[ 	]+66 8d 06 00 00[ 	]+lea[ 	]+0x0,%eax
[ 	]*[0-9a-f]+:[ 	]+b8 00 00[ 	]+mov[ 	]+\$0x0,%ax
[ 	]*[0-9a-f]+:[ 	]+66 b8 00 00 00 00[ 	]+mov[ 	]+\$0x0,%eax
[ 	]*[0-9a-f]+:[ 	]+67 8d 05 00 00 00 00[ 	]+addr32 lea[ 	]+0x0,%ax
[ 	]*[0-9a-f]+:[ 	]+66 8d 06 00 00[ 	]+lea[ 	]+0x0,%eax
[ 	]*[0-9a-f]+:[ 	]+b8 00 00[ 	]+mov[ 	]+\$0x0,%ax
[ 	]*[0-9a-f]+:[ 	]+8d 47 ff[ 	]+lea[ 	]+-0x1\(%bx\),%ax
[ 	]*[0-9a-f]+:[ 	]+8d 87 01 00[ 	]+lea[ 	]+0x1\(%bx\),%ax
#pass
