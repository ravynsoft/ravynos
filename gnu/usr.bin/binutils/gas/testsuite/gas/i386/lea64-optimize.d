#as: -O -q
#objdump: -drw
#name: x86-64 LEA optimizations
#source: lea64.s

.*: +file format .*

Disassembly of section .text:

0+ <start>:
[ 	]*[0-9a-f]+:[ 	]+8d 04 08[ 	]+lea[ 	]+\(%rax,%rcx(,1)?\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 04 08[ 	]+lea[ 	]+\(%rax,%rcx(,1)?\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 48 01[ 	]+lea[ 	]+0x1\(%rax\),%ecx
[ 	]*[0-9a-f]+:[ 	]+8d 88 00 00 00 00[ 	]+lea[ 	]+0x0\(%rax\),%ecx[ 	]+[0-9a-f]+: (R_X86_64_32|IMAGE_REL_AMD64_ADDR32)[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+8d 0c 25 00 00 00 00[ 	]+lea[ 	]+0x0,%ecx[ 	]+[0-9a-f]+: (R_X86_64_32|IMAGE_REL_AMD64_ADDR32)[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+8d 04 00[ 	]+lea[ 	]+\(%rax,%rax(,1)?\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 04 45 00 00 00 00[ 	]+lea[ 	]+0x0\(,%rax,2\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 05 00 00 00 00[ 	]+lea[ 	]+0x0\(%rip\),%eax($| *#.*)
[ 	]*[0-9a-f]+:[ 	]+8d 04 25 00 00 00 00[ 	]+lea[ 	]+0x0,%eax
[ 	]*[0-9a-f]+:[ 	]+48 8b c0[ 	]+mov[ 	]+%rax,%rax
[ 	]*[0-9a-f]+:[ 	]+48 8b c8[ 	]+mov[ 	]+%rax,%rcx
[ 	]*[0-9a-f]+:[ 	]+48 8b c8[ 	]+mov[ 	]+%rax,%rcx
[ 	]*[0-9a-f]+:[ 	]+48 8b c8[ 	]+mov[ 	]+%rax,%rcx
[ 	]*[0-9a-f]+:[ 	]+8b c6[ 	]+mov[ 	]+%esi,%eax
[ 	]*[0-9a-f]+:[ 	]+8b f6[ 	]+mov[ 	]+%esi,%esi
[ 	]*[0-9a-f]+:[ 	]+8b c6[ 	]+mov[ 	]+%esi,%eax
[ 	]*[0-9a-f]+:[ 	]+66 8b c6[ 	]+mov[ 	]+%si,%ax
[ 	]*[0-9a-f]+:[ 	]+66 8b f6[ 	]+mov[ 	]+%si,%si
[ 	]*[0-9a-f]+:[ 	]+66 8b c6[ 	]+mov[ 	]+%si,%ax
[ 	]*[0-9a-f]+:[ 	]+8b c0[ 	]+mov[ 	]+%eax,%eax
[ 	]*[0-9a-f]+:[ 	]+8b c8[ 	]+mov[ 	]+%eax,%ecx
[ 	]*[0-9a-f]+:[ 	]+8b c8[ 	]+mov[ 	]+%eax,%ecx
[ 	]*[0-9a-f]+:[ 	]+8b c0[ 	]+mov[ 	]+%eax,%eax
[ 	]*[0-9a-f]+:[ 	]+8b c8[ 	]+mov[ 	]+%eax,%ecx
[ 	]*[0-9a-f]+:[ 	]+66 8b c6[ 	]+mov[ 	]+%si,%ax
[ 	]*[0-9a-f]+:[ 	]+66 8b f6[ 	]+mov[ 	]+%si,%si
[ 	]*[0-9a-f]+:[ 	]+66 8b c6[ 	]+mov[ 	]+%si,%ax
[ 	]*[0-9a-f]+:[ 	]+48 8b c9[ 	]+mov[ 	]+%rcx,%rcx
[ 	]*[0-9a-f]+:[ 	]+48 8b c1[ 	]+mov[ 	]+%rcx,%rax
[ 	]*[0-9a-f]+:[ 	]+8b c9[ 	]+mov[ 	]+%ecx,%ecx
[ 	]*[0-9a-f]+:[ 	]+8b c1[ 	]+mov[ 	]+%ecx,%eax
[ 	]*[0-9a-f]+:[ 	]+66 8b c9[ 	]+mov[ 	]+%cx,%cx
[ 	]*[0-9a-f]+:[ 	]+66 8b c1[ 	]+mov[ 	]+%cx,%ax
[ 	]*[0-9a-f]+:[ 	]+8b c9[ 	]+mov[ 	]+%ecx,%ecx
[ 	]*[0-9a-f]+:[ 	]+8b c1[ 	]+mov[ 	]+%ecx,%eax
[ 	]*[0-9a-f]+:[ 	]+8b c9[ 	]+mov[ 	]+%ecx,%ecx
[ 	]*[0-9a-f]+:[ 	]+8b c1[ 	]+mov[ 	]+%ecx,%eax
[ 	]*[0-9a-f]+:[ 	]+66 8b c9[ 	]+mov[ 	]+%cx,%cx
[ 	]*[0-9a-f]+:[ 	]+66 8b c1[ 	]+mov[ 	]+%cx,%ax
[ 	]*[0-9a-f]+:[ 	]+b8 01 00 00 00[ 	]+mov[ 	]+\$0x1,%eax
[ 	]*[0-9a-f]+:[ 	]+b8 02 00 00 00[ 	]+mov[ 	]+\$0x2,%eax
[ 	]*[0-9a-f]+:[ 	]+66 b8 03 00[ 	]+mov[ 	]+\$0x3,%ax
[ 	]*[0-9a-f]+:[ 	]+48 c7 c0 ff ff ff ff[ 	]+mov[ 	]+\$0xffffffffffffffff,%rax
[ 	]*[0-9a-f]+:[ 	]+b8 fe ff ff ff[ 	]+mov[ 	]+\$0xfffffffe,%eax
[ 	]*[0-9a-f]+:[ 	]+66 b8 fd ff[ 	]+mov[ 	]+\$0xfffd,%ax
[ 	]*[0-9a-f]+:[ 	]+b8 01 00 00 00[ 	]+mov[ 	]+\$0x1,%eax
[ 	]*[0-9a-f]+:[ 	]+b8 02 00 00 00[ 	]+mov[ 	]+\$0x2,%eax
[ 	]*[0-9a-f]+:[ 	]+66 b8 03 00[ 	]+mov[ 	]+\$0x3,%ax
[ 	]*[0-9a-f]+:[ 	]+b8 ff ff ff ff[ 	]+mov[ 	]+\$0xffffffff,%eax
[ 	]*[0-9a-f]+:[ 	]+b8 fe ff ff ff[ 	]+mov[ 	]+\$0xfffffffe,%eax
[ 	]*[0-9a-f]+:[ 	]+66 b8 fd ff[ 	]+mov[ 	]+\$0xfffd,%ax
[ 	]*[0-9a-f]+:[ 	]+48 c7 c0 00 00 00 00[ 	]+mov[ 	]+\$0x0,%rax[ 	]+[0-9a-f]+: R_X86_64_32S[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+b8 00 00 00 00[ 	]+mov[ 	]+\$0x0,%eax[ 	]+[0-9a-f]+: (R_X86_64_32|IMAGE_REL_AMD64_ADDR32)[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+66 8d 04 25 00 00 00 00[ 	]+lea[ 	]+0x0,%ax[ 	]+[0-9a-f]+: (R_X86_64_32|IMAGE_REL_AMD64_ADDR32)[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+b8 00 00 00 00[ 	]+mov[ 	]+\$0x0,%eax[ 	]+[0-9a-f]+: (R_X86_64_32|IMAGE_REL_AMD64_ADDR32)[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+b8 00 00 00 00[ 	]+mov[ 	]+\$0x0,%eax[ 	]+[0-9a-f]+: (R_X86_64_32|IMAGE_REL_AMD64_ADDR32)[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+66 8d 04 25 00 00 00 00[ 	]+lea[ 	]+0x0,%ax[ 	]+[0-9a-f]+: (R_X86_64_32|IMAGE_REL_AMD64_ADDR32)[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+48 c7 c0 00 00 00 00[ 	]+mov[ 	]+\$0x0,%rax
[ 	]*[0-9a-f]+:[ 	]+b8 00 00 00 00[ 	]+mov[ 	]+\$0x0,%eax
[ 	]*[0-9a-f]+:[ 	]+66 b8 00 00[ 	]+mov[ 	]+\$0x0,%ax
[ 	]*[0-9a-f]+:[ 	]+8d 41 ff[ 	]+lea[ 	]+-0x1\(%rcx\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 41 ff[ 	]+lea[ 	]+-0x1\(%rcx\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 41 ff[ 	]+lea[ 	]+-0x1\(%rcx\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 81 01 00 00 00[ 	]+lea[ 	]+0x1\(%rcx\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 81 01 00 00 00[ 	]+lea[ 	]+0x1\(%rcx\),%eax
[ 	]*[0-9a-f]+:[ 	]+8d 81 01 00 00 00[ 	]+lea[ 	]+0x1\(%rcx\),%eax
#pass
