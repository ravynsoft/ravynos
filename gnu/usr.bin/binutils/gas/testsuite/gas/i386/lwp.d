#objdump: -dw
#name: i386 LWP

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	8f e9 78 12 c0[ 	]+llwpcb %eax
[ 	]*[a-f0-9]+:	8f e9 78 12 c1[ 	]+llwpcb %ecx
[ 	]*[a-f0-9]+:	8f e9 78 12 c2[ 	]+llwpcb %edx
[ 	]*[a-f0-9]+:	8f e9 78 12 c3[ 	]+llwpcb %ebx
[ 	]*[a-f0-9]+:	8f e9 78 12 c4[ 	]+llwpcb %esp
[ 	]*[a-f0-9]+:	8f e9 78 12 c5[ 	]+llwpcb %ebp
[ 	]*[a-f0-9]+:	8f e9 78 12 c6[ 	]+llwpcb %esi
[ 	]*[a-f0-9]+:	8f e9 78 12 c7[ 	]+llwpcb %edi
[ 	]*[a-f0-9]+:	8f e9 78 12 cf[ 	]+slwpcb %edi
[ 	]*[a-f0-9]+:	8f e9 78 12 ce[ 	]+slwpcb %esi
[ 	]*[a-f0-9]+:	8f e9 78 12 cd[ 	]+slwpcb %ebp
[ 	]*[a-f0-9]+:	8f e9 78 12 cc[ 	]+slwpcb %esp
[ 	]*[a-f0-9]+:	8f e9 78 12 cb[ 	]+slwpcb %ebx
[ 	]*[a-f0-9]+:	8f e9 78 12 ca[ 	]+slwpcb %edx
[ 	]*[a-f0-9]+:	8f e9 78 12 c9[ 	]+slwpcb %ecx
[ 	]*[a-f0-9]+:	8f e9 78 12 c8[ 	]+slwpcb %eax
[ 	]*[a-f0-9]+:	8f ea 78 12 c7 78 56 34 12[ 	]+lwpins \$0x12345678,%edi,%eax
[ 	]*[a-f0-9]+:	8f ea 70 12 c6 78 56 34 12[ 	]+lwpins \$0x12345678,%esi,%ecx
[ 	]*[a-f0-9]+:	8f ea 68 12 c5 78 56 34 12[ 	]+lwpins \$0x12345678,%ebp,%edx
[ 	]*[a-f0-9]+:	8f ea 60 12 c4 78 56 34 12[ 	]+lwpins \$0x12345678,%esp,%ebx
[ 	]*[a-f0-9]+:	8f ea 58 12 c3 78 56 34 12[ 	]+lwpins \$0x12345678,%ebx,%esp
[ 	]*[a-f0-9]+:	8f ea 50 12 c2 78 56 34 12[ 	]+lwpins \$0x12345678,%edx,%ebp
[ 	]*[a-f0-9]+:	8f ea 48 12 c1 78 56 34 12[ 	]+lwpins \$0x12345678,%ecx,%esi
[ 	]*[a-f0-9]+:	8f ea 40 12 c0 78 56 34 12[ 	]+lwpins \$0x12345678,%eax,%edi
[ 	]*[a-f0-9]+:	8f ea 78 12 cf 78 56 34 12[ 	]+lwpval \$0x12345678,%edi,%eax
[ 	]*[a-f0-9]+:	8f ea 70 12 ce 78 56 34 12[ 	]+lwpval \$0x12345678,%esi,%ecx
[ 	]*[a-f0-9]+:	8f ea 68 12 cd 78 56 34 12[ 	]+lwpval \$0x12345678,%ebp,%edx
[ 	]*[a-f0-9]+:	8f ea 60 12 cc 78 56 34 12[ 	]+lwpval \$0x12345678,%esp,%ebx
[ 	]*[a-f0-9]+:	8f ea 58 12 cb 78 56 34 12[ 	]+lwpval \$0x12345678,%ebx,%esp
[ 	]*[a-f0-9]+:	8f ea 50 12 ca 78 56 34 12[ 	]+lwpval \$0x12345678,%edx,%ebp
[ 	]*[a-f0-9]+:	8f ea 48 12 c9 78 56 34 12[ 	]+lwpval \$0x12345678,%ecx,%esi
[ 	]*[a-f0-9]+:	8f ea 40 12 c8 78 56 34 12[ 	]+lwpval \$0x12345678,%eax,%edi
[ 	]*[a-f0-9]+:	8f ea 78 12 07 78 56 34 12[ 	]+lwpins \$0x12345678,\(%edi\),%eax
[ 	]*[a-f0-9]+:	8f ea 70 12 06 78 56 34 12[ 	]+lwpins \$0x12345678,\(%esi\),%ecx
[ 	]*[a-f0-9]+:	8f ea 68 12 45 00 78 56 34 12[ 	]+lwpins \$0x12345678,0x0\(%ebp\),%edx
[ 	]*[a-f0-9]+:	8f ea 60 12 04 24 78 56 34 12[ 	]+lwpins \$0x12345678,\(%esp\),%ebx
[ 	]*[a-f0-9]+:	8f ea 58 12 03 78 56 34 12[ 	]+lwpins \$0x12345678,\(%ebx\),%esp
[ 	]*[a-f0-9]+:	8f ea 50 12 02 78 56 34 12[ 	]+lwpins \$0x12345678,\(%edx\),%ebp
[ 	]*[a-f0-9]+:	8f ea 48 12 01 78 56 34 12[ 	]+lwpins \$0x12345678,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	8f ea 40 12 00 78 56 34 12[ 	]+lwpins \$0x12345678,\(%eax\),%edi
[ 	]*[a-f0-9]+:	8f ea 78 12 0f 78 56 34 12[ 	]+lwpval \$0x12345678,\(%edi\),%eax
[ 	]*[a-f0-9]+:	8f ea 70 12 0e 78 56 34 12[ 	]+lwpval \$0x12345678,\(%esi\),%ecx
[ 	]*[a-f0-9]+:	8f ea 68 12 4d 00 78 56 34 12[ 	]+lwpval \$0x12345678,0x0\(%ebp\),%edx
[ 	]*[a-f0-9]+:	8f ea 60 12 0c 24 78 56 34 12[ 	]+lwpval \$0x12345678,\(%esp\),%ebx
[ 	]*[a-f0-9]+:	8f ea 58 12 0b 78 56 34 12[ 	]+lwpval \$0x12345678,\(%ebx\),%esp
[ 	]*[a-f0-9]+:	8f ea 50 12 0a 78 56 34 12[ 	]+lwpval \$0x12345678,\(%edx\),%ebp
[ 	]*[a-f0-9]+:	8f ea 48 12 09 78 56 34 12[ 	]+lwpval \$0x12345678,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	8f ea 40 12 08 78 56 34 12[ 	]+lwpval \$0x12345678,\(%eax\),%edi
[ 	]*[a-f0-9]+:	8f ea 78 12 87 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%edi\),%eax
[ 	]*[a-f0-9]+:	8f ea 70 12 86 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%esi\),%ecx
[ 	]*[a-f0-9]+:	8f ea 68 12 85 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%ebp\),%edx
[ 	]*[a-f0-9]+:	8f ea 60 12 84 24 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%esp\),%ebx
[ 	]*[a-f0-9]+:	8f ea 58 12 83 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%ebx\),%esp
[ 	]*[a-f0-9]+:	8f ea 50 12 82 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%edx\),%ebp
[ 	]*[a-f0-9]+:	8f ea 48 12 81 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%ecx\),%esi
[ 	]*[a-f0-9]+:	8f ea 40 12 80 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%eax\),%edi
[ 	]*[a-f0-9]+:	8f ea 78 12 8f fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%edi\),%eax
[ 	]*[a-f0-9]+:	8f ea 70 12 8e fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%esi\),%ecx
[ 	]*[a-f0-9]+:	8f ea 68 12 8d fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%ebp\),%edx
[ 	]*[a-f0-9]+:	8f ea 60 12 8c 24 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%esp\),%ebx
[ 	]*[a-f0-9]+:	8f ea 58 12 8b fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%ebx\),%esp
[ 	]*[a-f0-9]+:	8f ea 50 12 8a fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%edx\),%ebp
[ 	]*[a-f0-9]+:	8f ea 48 12 89 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%ecx\),%esi
[ 	]*[a-f0-9]+:	8f ea 40 12 88 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%eax\),%edi
#pass
