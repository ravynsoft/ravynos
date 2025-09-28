#objdump: -dw
#name: x86-64 LWP

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
[ 	]*[a-f0-9]+:	8f c9 78 12 c0[ 	]+llwpcb %r8d
[ 	]*[a-f0-9]+:	8f c9 78 12 c1[ 	]+llwpcb %r9d
[ 	]*[a-f0-9]+:	8f c9 78 12 c2[ 	]+llwpcb %r10d
[ 	]*[a-f0-9]+:	8f c9 78 12 c3[ 	]+llwpcb %r11d
[ 	]*[a-f0-9]+:	8f c9 78 12 c4[ 	]+llwpcb %r12d
[ 	]*[a-f0-9]+:	8f c9 78 12 c5[ 	]+llwpcb %r13d
[ 	]*[a-f0-9]+:	8f c9 78 12 c6[ 	]+llwpcb %r14d
[ 	]*[a-f0-9]+:	8f c9 78 12 c7[ 	]+llwpcb %r15d
[ 	]*[a-f0-9]+:	8f e9 f8 12 c0[ 	]+llwpcb %rax
[ 	]*[a-f0-9]+:	8f e9 f8 12 c1[ 	]+llwpcb %rcx
[ 	]*[a-f0-9]+:	8f e9 f8 12 c2[ 	]+llwpcb %rdx
[ 	]*[a-f0-9]+:	8f e9 f8 12 c3[ 	]+llwpcb %rbx
[ 	]*[a-f0-9]+:	8f e9 f8 12 c4[ 	]+llwpcb %rsp
[ 	]*[a-f0-9]+:	8f e9 f8 12 c5[ 	]+llwpcb %rbp
[ 	]*[a-f0-9]+:	8f e9 f8 12 c6[ 	]+llwpcb %rsi
[ 	]*[a-f0-9]+:	8f e9 f8 12 c7[ 	]+llwpcb %rdi
[ 	]*[a-f0-9]+:	8f c9 f8 12 c0[ 	]+llwpcb %r8
[ 	]*[a-f0-9]+:	8f c9 f8 12 c1[ 	]+llwpcb %r9
[ 	]*[a-f0-9]+:	8f c9 f8 12 c2[ 	]+llwpcb %r10
[ 	]*[a-f0-9]+:	8f c9 f8 12 c3[ 	]+llwpcb %r11
[ 	]*[a-f0-9]+:	8f c9 f8 12 c4[ 	]+llwpcb %r12
[ 	]*[a-f0-9]+:	8f c9 f8 12 c5[ 	]+llwpcb %r13
[ 	]*[a-f0-9]+:	8f c9 f8 12 c6[ 	]+llwpcb %r14
[ 	]*[a-f0-9]+:	8f c9 f8 12 c7[ 	]+llwpcb %r15
[ 	]*[a-f0-9]+:	8f c9 f8 12 cf[ 	]+slwpcb %r15
[ 	]*[a-f0-9]+:	8f c9 f8 12 ce[ 	]+slwpcb %r14
[ 	]*[a-f0-9]+:	8f c9 f8 12 cd[ 	]+slwpcb %r13
[ 	]*[a-f0-9]+:	8f c9 f8 12 cc[ 	]+slwpcb %r12
[ 	]*[a-f0-9]+:	8f c9 f8 12 cb[ 	]+slwpcb %r11
[ 	]*[a-f0-9]+:	8f c9 f8 12 ca[ 	]+slwpcb %r10
[ 	]*[a-f0-9]+:	8f c9 f8 12 c9[ 	]+slwpcb %r9
[ 	]*[a-f0-9]+:	8f c9 f8 12 c8[ 	]+slwpcb %r8
[ 	]*[a-f0-9]+:	8f e9 f8 12 cf[ 	]+slwpcb %rdi
[ 	]*[a-f0-9]+:	8f e9 f8 12 ce[ 	]+slwpcb %rsi
[ 	]*[a-f0-9]+:	8f e9 f8 12 cd[ 	]+slwpcb %rbp
[ 	]*[a-f0-9]+:	8f e9 f8 12 cc[ 	]+slwpcb %rsp
[ 	]*[a-f0-9]+:	8f e9 f8 12 cb[ 	]+slwpcb %rbx
[ 	]*[a-f0-9]+:	8f e9 f8 12 ca[ 	]+slwpcb %rdx
[ 	]*[a-f0-9]+:	8f e9 f8 12 c9[ 	]+slwpcb %rcx
[ 	]*[a-f0-9]+:	8f e9 f8 12 c8[ 	]+slwpcb %rax
[ 	]*[a-f0-9]+:	8f c9 78 12 cf[ 	]+slwpcb %r15d
[ 	]*[a-f0-9]+:	8f c9 78 12 ce[ 	]+slwpcb %r14d
[ 	]*[a-f0-9]+:	8f c9 78 12 cd[ 	]+slwpcb %r13d
[ 	]*[a-f0-9]+:	8f c9 78 12 cc[ 	]+slwpcb %r12d
[ 	]*[a-f0-9]+:	8f c9 78 12 cb[ 	]+slwpcb %r11d
[ 	]*[a-f0-9]+:	8f c9 78 12 ca[ 	]+slwpcb %r10d
[ 	]*[a-f0-9]+:	8f c9 78 12 c9[ 	]+slwpcb %r9d
[ 	]*[a-f0-9]+:	8f c9 78 12 c8[ 	]+slwpcb %r8d
[ 	]*[a-f0-9]+:	8f e9 78 12 cf[ 	]+slwpcb %edi
[ 	]*[a-f0-9]+:	8f e9 78 12 ce[ 	]+slwpcb %esi
[ 	]*[a-f0-9]+:	8f e9 78 12 cd[ 	]+slwpcb %ebp
[ 	]*[a-f0-9]+:	8f e9 78 12 cc[ 	]+slwpcb %esp
[ 	]*[a-f0-9]+:	8f e9 78 12 cb[ 	]+slwpcb %ebx
[ 	]*[a-f0-9]+:	8f e9 78 12 ca[ 	]+slwpcb %edx
[ 	]*[a-f0-9]+:	8f e9 78 12 c9[ 	]+slwpcb %ecx
[ 	]*[a-f0-9]+:	8f e9 78 12 c8[ 	]+slwpcb %eax
[ 	]*[a-f0-9]+:	8f ca 78 12 c7 78 56 34 12[ 	]+lwpins \$0x12345678,%r15d,%eax
[ 	]*[a-f0-9]+:	8f ca 70 12 c6 78 56 34 12[ 	]+lwpins \$0x12345678,%r14d,%ecx
[ 	]*[a-f0-9]+:	8f ca 68 12 c5 78 56 34 12[ 	]+lwpins \$0x12345678,%r13d,%edx
[ 	]*[a-f0-9]+:	8f ca 60 12 c4 78 56 34 12[ 	]+lwpins \$0x12345678,%r12d,%ebx
[ 	]*[a-f0-9]+:	8f ca 58 12 c3 78 56 34 12[ 	]+lwpins \$0x12345678,%r11d,%esp
[ 	]*[a-f0-9]+:	8f ca 50 12 c2 78 56 34 12[ 	]+lwpins \$0x12345678,%r10d,%ebp
[ 	]*[a-f0-9]+:	8f ca 48 12 c1 78 56 34 12[ 	]+lwpins \$0x12345678,%r9d,%esi
[ 	]*[a-f0-9]+:	8f ca 40 12 c0 78 56 34 12[ 	]+lwpins \$0x12345678,%r8d,%edi
[ 	]*[a-f0-9]+:	8f ea 38 12 c7 78 56 34 12[ 	]+lwpins \$0x12345678,%edi,%r8d
[ 	]*[a-f0-9]+:	8f ea 30 12 c6 78 56 34 12[ 	]+lwpins \$0x12345678,%esi,%r9d
[ 	]*[a-f0-9]+:	8f ea 28 12 c5 78 56 34 12[ 	]+lwpins \$0x12345678,%ebp,%r10d
[ 	]*[a-f0-9]+:	8f ea 20 12 c4 78 56 34 12[ 	]+lwpins \$0x12345678,%esp,%r11d
[ 	]*[a-f0-9]+:	8f ea 18 12 c3 78 56 34 12[ 	]+lwpins \$0x12345678,%ebx,%r12d
[ 	]*[a-f0-9]+:	8f ea 10 12 c2 78 56 34 12[ 	]+lwpins \$0x12345678,%edx,%r13d
[ 	]*[a-f0-9]+:	8f ea 08 12 c1 78 56 34 12[ 	]+lwpins \$0x12345678,%ecx,%r14d
[ 	]*[a-f0-9]+:	8f ea 00 12 c0 78 56 34 12[ 	]+lwpins \$0x12345678,%eax,%r15d
[ 	]*[a-f0-9]+:	8f ca f8 12 c7 78 56 34 12[ 	]+lwpins \$0x12345678,%r15d,%rax
[ 	]*[a-f0-9]+:	8f ca f0 12 c6 78 56 34 12[ 	]+lwpins \$0x12345678,%r14d,%rcx
[ 	]*[a-f0-9]+:	8f ca e8 12 c5 78 56 34 12[ 	]+lwpins \$0x12345678,%r13d,%rdx
[ 	]*[a-f0-9]+:	8f ca e0 12 c4 78 56 34 12[ 	]+lwpins \$0x12345678,%r12d,%rbx
[ 	]*[a-f0-9]+:	8f ca d8 12 c3 78 56 34 12[ 	]+lwpins \$0x12345678,%r11d,%rsp
[ 	]*[a-f0-9]+:	8f ca d0 12 c2 78 56 34 12[ 	]+lwpins \$0x12345678,%r10d,%rbp
[ 	]*[a-f0-9]+:	8f ca c8 12 c1 78 56 34 12[ 	]+lwpins \$0x12345678,%r9d,%rsi
[ 	]*[a-f0-9]+:	8f ca c0 12 c0 78 56 34 12[ 	]+lwpins \$0x12345678,%r8d,%rdi
[ 	]*[a-f0-9]+:	8f ea b8 12 c0 78 56 34 12[ 	]+lwpins \$0x12345678,%eax,%r8
[ 	]*[a-f0-9]+:	8f ea b0 12 c1 78 56 34 12[ 	]+lwpins \$0x12345678,%ecx,%r9
[ 	]*[a-f0-9]+:	8f ea a8 12 c2 78 56 34 12[ 	]+lwpins \$0x12345678,%edx,%r10
[ 	]*[a-f0-9]+:	8f ea a0 12 c3 78 56 34 12[ 	]+lwpins \$0x12345678,%ebx,%r11
[ 	]*[a-f0-9]+:	8f ea 98 12 c4 78 56 34 12[ 	]+lwpins \$0x12345678,%esp,%r12
[ 	]*[a-f0-9]+:	8f ea 90 12 c5 78 56 34 12[ 	]+lwpins \$0x12345678,%ebp,%r13
[ 	]*[a-f0-9]+:	8f ea 88 12 c6 78 56 34 12[ 	]+lwpins \$0x12345678,%esi,%r14
[ 	]*[a-f0-9]+:	8f ea 80 12 c7 78 56 34 12[ 	]+lwpins \$0x12345678,%edi,%r15
[ 	]*[a-f0-9]+:	8f ca 78 12 cf 78 56 34 12[ 	]+lwpval \$0x12345678,%r15d,%eax
[ 	]*[a-f0-9]+:	8f ca 70 12 ce 78 56 34 12[ 	]+lwpval \$0x12345678,%r14d,%ecx
[ 	]*[a-f0-9]+:	8f ca 68 12 cd 78 56 34 12[ 	]+lwpval \$0x12345678,%r13d,%edx
[ 	]*[a-f0-9]+:	8f ca 60 12 cc 78 56 34 12[ 	]+lwpval \$0x12345678,%r12d,%ebx
[ 	]*[a-f0-9]+:	8f ca 58 12 cb 78 56 34 12[ 	]+lwpval \$0x12345678,%r11d,%esp
[ 	]*[a-f0-9]+:	8f ca 50 12 ca 78 56 34 12[ 	]+lwpval \$0x12345678,%r10d,%ebp
[ 	]*[a-f0-9]+:	8f ca 48 12 c9 78 56 34 12[ 	]+lwpval \$0x12345678,%r9d,%esi
[ 	]*[a-f0-9]+:	8f ca 40 12 c8 78 56 34 12[ 	]+lwpval \$0x12345678,%r8d,%edi
[ 	]*[a-f0-9]+:	8f ea 38 12 cf 78 56 34 12[ 	]+lwpval \$0x12345678,%edi,%r8d
[ 	]*[a-f0-9]+:	8f ea 30 12 ce 78 56 34 12[ 	]+lwpval \$0x12345678,%esi,%r9d
[ 	]*[a-f0-9]+:	8f ea 28 12 cd 78 56 34 12[ 	]+lwpval \$0x12345678,%ebp,%r10d
[ 	]*[a-f0-9]+:	8f ea 20 12 cc 78 56 34 12[ 	]+lwpval \$0x12345678,%esp,%r11d
[ 	]*[a-f0-9]+:	8f ea 18 12 cb 78 56 34 12[ 	]+lwpval \$0x12345678,%ebx,%r12d
[ 	]*[a-f0-9]+:	8f ea 10 12 ca 78 56 34 12[ 	]+lwpval \$0x12345678,%edx,%r13d
[ 	]*[a-f0-9]+:	8f ea 08 12 c9 78 56 34 12[ 	]+lwpval \$0x12345678,%ecx,%r14d
[ 	]*[a-f0-9]+:	8f ea 00 12 c8 78 56 34 12[ 	]+lwpval \$0x12345678,%eax,%r15d
[ 	]*[a-f0-9]+:	8f ca f8 12 cf 78 56 34 12[ 	]+lwpval \$0x12345678,%r15d,%rax
[ 	]*[a-f0-9]+:	8f ca f0 12 ce 78 56 34 12[ 	]+lwpval \$0x12345678,%r14d,%rcx
[ 	]*[a-f0-9]+:	8f ca e8 12 cd 78 56 34 12[ 	]+lwpval \$0x12345678,%r13d,%rdx
[ 	]*[a-f0-9]+:	8f ca e0 12 cc 78 56 34 12[ 	]+lwpval \$0x12345678,%r12d,%rbx
[ 	]*[a-f0-9]+:	8f ca d8 12 cb 78 56 34 12[ 	]+lwpval \$0x12345678,%r11d,%rsp
[ 	]*[a-f0-9]+:	8f ca d0 12 ca 78 56 34 12[ 	]+lwpval \$0x12345678,%r10d,%rbp
[ 	]*[a-f0-9]+:	8f ca c8 12 c9 78 56 34 12[ 	]+lwpval \$0x12345678,%r9d,%rsi
[ 	]*[a-f0-9]+:	8f ca c0 12 c8 78 56 34 12[ 	]+lwpval \$0x12345678,%r8d,%rdi
[ 	]*[a-f0-9]+:	8f ea b8 12 c8 78 56 34 12[ 	]+lwpval \$0x12345678,%eax,%r8
[ 	]*[a-f0-9]+:	8f ea b0 12 c9 78 56 34 12[ 	]+lwpval \$0x12345678,%ecx,%r9
[ 	]*[a-f0-9]+:	8f ea a8 12 ca 78 56 34 12[ 	]+lwpval \$0x12345678,%edx,%r10
[ 	]*[a-f0-9]+:	8f ea a0 12 cb 78 56 34 12[ 	]+lwpval \$0x12345678,%ebx,%r11
[ 	]*[a-f0-9]+:	8f ea 98 12 cc 78 56 34 12[ 	]+lwpval \$0x12345678,%esp,%r12
[ 	]*[a-f0-9]+:	8f ea 90 12 cd 78 56 34 12[ 	]+lwpval \$0x12345678,%ebp,%r13
[ 	]*[a-f0-9]+:	8f ea 88 12 ce 78 56 34 12[ 	]+lwpval \$0x12345678,%esi,%r14
[ 	]*[a-f0-9]+:	8f ea 80 12 cf 78 56 34 12[ 	]+lwpval \$0x12345678,%edi,%r15
[ 	]*[a-f0-9]+:	67 8f ca 78 12 07 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r15d\),%eax
[ 	]*[a-f0-9]+:	67 8f ca 70 12 06 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r14d\),%ecx
[ 	]*[a-f0-9]+:	67 8f ca 68 12 45 00 78 56 34 12[ 	]+lwpins \$0x12345678,0x0\(%r13d\),%edx
[ 	]*[a-f0-9]+:	67 8f ca 60 12 04 24 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r12d\),%ebx
[ 	]*[a-f0-9]+:	67 8f ca 58 12 03 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r11d\),%esp
[ 	]*[a-f0-9]+:	67 8f ca 50 12 02 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r10d\),%ebp
[ 	]*[a-f0-9]+:	67 8f ca 48 12 01 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r9d\),%esi
[ 	]*[a-f0-9]+:	67 8f ca 40 12 00 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r8d\),%edi
[ 	]*[a-f0-9]+:	67 8f ea 38 12 07 78 56 34 12[ 	]+lwpins \$0x12345678,\(%edi\),%r8d
[ 	]*[a-f0-9]+:	67 8f ea 30 12 06 78 56 34 12[ 	]+lwpins \$0x12345678,\(%esi\),%r9d
[ 	]*[a-f0-9]+:	67 8f ea 28 12 45 00 78 56 34 12[ 	]+lwpins \$0x12345678,0x0\(%ebp\),%r10d
[ 	]*[a-f0-9]+:	67 8f ea 20 12 04 24 78 56 34 12[ 	]+lwpins \$0x12345678,\(%esp\),%r11d
[ 	]*[a-f0-9]+:	67 8f ea 18 12 03 78 56 34 12[ 	]+lwpins \$0x12345678,\(%ebx\),%r12d
[ 	]*[a-f0-9]+:	67 8f ea 10 12 02 78 56 34 12[ 	]+lwpins \$0x12345678,\(%edx\),%r13d
[ 	]*[a-f0-9]+:	67 8f ea 08 12 01 78 56 34 12[ 	]+lwpins \$0x12345678,\(%ecx\),%r14d
[ 	]*[a-f0-9]+:	67 8f ea 00 12 00 78 56 34 12[ 	]+lwpins \$0x12345678,\(%eax\),%r15d
[ 	]*[a-f0-9]+:	67 8f ca f8 12 07 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r15d\),%rax
[ 	]*[a-f0-9]+:	67 8f ca f0 12 06 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r14d\),%rcx
[ 	]*[a-f0-9]+:	67 8f ca e8 12 45 00 78 56 34 12[ 	]+lwpins \$0x12345678,0x0\(%r13d\),%rdx
[ 	]*[a-f0-9]+:	67 8f ca e0 12 04 24 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r12d\),%rbx
[ 	]*[a-f0-9]+:	67 8f ca d8 12 03 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r11d\),%rsp
[ 	]*[a-f0-9]+:	67 8f ca d0 12 02 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r10d\),%rbp
[ 	]*[a-f0-9]+:	67 8f ca c8 12 01 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r9d\),%rsi
[ 	]*[a-f0-9]+:	67 8f ca c0 12 00 78 56 34 12[ 	]+lwpins \$0x12345678,\(%r8d\),%rdi
[ 	]*[a-f0-9]+:	67 8f ea b8 12 00 78 56 34 12[ 	]+lwpins \$0x12345678,\(%eax\),%r8
[ 	]*[a-f0-9]+:	67 8f ea b0 12 01 78 56 34 12[ 	]+lwpins \$0x12345678,\(%ecx\),%r9
[ 	]*[a-f0-9]+:	67 8f ea a8 12 02 78 56 34 12[ 	]+lwpins \$0x12345678,\(%edx\),%r10
[ 	]*[a-f0-9]+:	67 8f ea a0 12 03 78 56 34 12[ 	]+lwpins \$0x12345678,\(%ebx\),%r11
[ 	]*[a-f0-9]+:	67 8f ea 98 12 04 24 78 56 34 12[ 	]+lwpins \$0x12345678,\(%esp\),%r12
[ 	]*[a-f0-9]+:	67 8f ea 90 12 45 00 78 56 34 12[ 	]+lwpins \$0x12345678,0x0\(%ebp\),%r13
[ 	]*[a-f0-9]+:	67 8f ea 88 12 06 78 56 34 12[ 	]+lwpins \$0x12345678,\(%esi\),%r14
[ 	]*[a-f0-9]+:	67 8f ea 80 12 07 78 56 34 12[ 	]+lwpins \$0x12345678,\(%edi\),%r15
[ 	]*[a-f0-9]+:	67 8f ca 78 12 0f 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r15d\),%eax
[ 	]*[a-f0-9]+:	67 8f ca 70 12 0e 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r14d\),%ecx
[ 	]*[a-f0-9]+:	67 8f ca 68 12 4d 00 78 56 34 12[ 	]+lwpval \$0x12345678,0x0\(%r13d\),%edx
[ 	]*[a-f0-9]+:	67 8f ca 60 12 0c 24 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r12d\),%ebx
[ 	]*[a-f0-9]+:	67 8f ca 58 12 0b 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r11d\),%esp
[ 	]*[a-f0-9]+:	67 8f ca 50 12 0a 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r10d\),%ebp
[ 	]*[a-f0-9]+:	67 8f ca 48 12 09 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r9d\),%esi
[ 	]*[a-f0-9]+:	67 8f ca 40 12 08 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r8d\),%edi
[ 	]*[a-f0-9]+:	67 8f ea 38 12 0f 78 56 34 12[ 	]+lwpval \$0x12345678,\(%edi\),%r8d
[ 	]*[a-f0-9]+:	67 8f ea 30 12 0e 78 56 34 12[ 	]+lwpval \$0x12345678,\(%esi\),%r9d
[ 	]*[a-f0-9]+:	67 8f ea 28 12 4d 00 78 56 34 12[ 	]+lwpval \$0x12345678,0x0\(%ebp\),%r10d
[ 	]*[a-f0-9]+:	67 8f ea 20 12 0c 24 78 56 34 12[ 	]+lwpval \$0x12345678,\(%esp\),%r11d
[ 	]*[a-f0-9]+:	67 8f ea 18 12 0b 78 56 34 12[ 	]+lwpval \$0x12345678,\(%ebx\),%r12d
[ 	]*[a-f0-9]+:	67 8f ea 10 12 0a 78 56 34 12[ 	]+lwpval \$0x12345678,\(%edx\),%r13d
[ 	]*[a-f0-9]+:	67 8f ea 08 12 09 78 56 34 12[ 	]+lwpval \$0x12345678,\(%ecx\),%r14d
[ 	]*[a-f0-9]+:	67 8f ea 00 12 08 78 56 34 12[ 	]+lwpval \$0x12345678,\(%eax\),%r15d
[ 	]*[a-f0-9]+:	67 8f ca f8 12 0f 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r15d\),%rax
[ 	]*[a-f0-9]+:	67 8f ca f0 12 0e 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r14d\),%rcx
[ 	]*[a-f0-9]+:	67 8f ca e8 12 4d 00 78 56 34 12[ 	]+lwpval \$0x12345678,0x0\(%r13d\),%rdx
[ 	]*[a-f0-9]+:	67 8f ca e0 12 0c 24 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r12d\),%rbx
[ 	]*[a-f0-9]+:	67 8f ca d8 12 0b 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r11d\),%rsp
[ 	]*[a-f0-9]+:	67 8f ca d0 12 0a 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r10d\),%rbp
[ 	]*[a-f0-9]+:	67 8f ca c8 12 09 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r9d\),%rsi
[ 	]*[a-f0-9]+:	67 8f ca c0 12 08 78 56 34 12[ 	]+lwpval \$0x12345678,\(%r8d\),%rdi
[ 	]*[a-f0-9]+:	67 8f ea b8 12 08 78 56 34 12[ 	]+lwpval \$0x12345678,\(%eax\),%r8
[ 	]*[a-f0-9]+:	67 8f ea b0 12 09 78 56 34 12[ 	]+lwpval \$0x12345678,\(%ecx\),%r9
[ 	]*[a-f0-9]+:	67 8f ea a8 12 0a 78 56 34 12[ 	]+lwpval \$0x12345678,\(%edx\),%r10
[ 	]*[a-f0-9]+:	67 8f ea a0 12 0b 78 56 34 12[ 	]+lwpval \$0x12345678,\(%ebx\),%r11
[ 	]*[a-f0-9]+:	67 8f ea 98 12 0c 24 78 56 34 12[ 	]+lwpval \$0x12345678,\(%esp\),%r12
[ 	]*[a-f0-9]+:	67 8f ea 90 12 4d 00 78 56 34 12[ 	]+lwpval \$0x12345678,0x0\(%ebp\),%r13
[ 	]*[a-f0-9]+:	67 8f ea 88 12 0e 78 56 34 12[ 	]+lwpval \$0x12345678,\(%esi\),%r14
[ 	]*[a-f0-9]+:	67 8f ea 80 12 0f 78 56 34 12[ 	]+lwpval \$0x12345678,\(%edi\),%r15
[ 	]*[a-f0-9]+:	67 8f ca 78 12 87 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r15d\),%eax
[ 	]*[a-f0-9]+:	67 8f ca 70 12 86 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r14d\),%ecx
[ 	]*[a-f0-9]+:	67 8f ca 68 12 85 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r13d\),%edx
[ 	]*[a-f0-9]+:	67 8f ca 60 12 84 24 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r12d\),%ebx
[ 	]*[a-f0-9]+:	67 8f ca 58 12 83 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r11d\),%esp
[ 	]*[a-f0-9]+:	67 8f ca 50 12 82 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r10d\),%ebp
[ 	]*[a-f0-9]+:	67 8f ca 48 12 81 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r9d\),%esi
[ 	]*[a-f0-9]+:	67 8f ca 40 12 80 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r8d\),%edi
[ 	]*[a-f0-9]+:	67 8f ea 38 12 87 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%edi\),%r8d
[ 	]*[a-f0-9]+:	67 8f ea 30 12 86 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%esi\),%r9d
[ 	]*[a-f0-9]+:	67 8f ea 28 12 85 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%ebp\),%r10d
[ 	]*[a-f0-9]+:	67 8f ea 20 12 84 24 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%esp\),%r11d
[ 	]*[a-f0-9]+:	67 8f ea 18 12 83 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%ebx\),%r12d
[ 	]*[a-f0-9]+:	67 8f ea 10 12 82 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%edx\),%r13d
[ 	]*[a-f0-9]+:	67 8f ea 08 12 81 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%ecx\),%r14d
[ 	]*[a-f0-9]+:	67 8f ea 00 12 80 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%eax\),%r15d
[ 	]*[a-f0-9]+:	67 8f ca f8 12 87 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r15d\),%rax
[ 	]*[a-f0-9]+:	67 8f ca f0 12 86 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r14d\),%rcx
[ 	]*[a-f0-9]+:	67 8f ca e8 12 85 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r13d\),%rdx
[ 	]*[a-f0-9]+:	67 8f ca e0 12 84 24 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r12d\),%rbx
[ 	]*[a-f0-9]+:	67 8f ca d8 12 83 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r11d\),%rsp
[ 	]*[a-f0-9]+:	67 8f ca d0 12 82 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r10d\),%rbp
[ 	]*[a-f0-9]+:	67 8f ca c8 12 81 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r9d\),%rsi
[ 	]*[a-f0-9]+:	67 8f ca c0 12 80 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%r8d\),%rdi
[ 	]*[a-f0-9]+:	67 8f ea b8 12 80 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%eax\),%r8
[ 	]*[a-f0-9]+:	67 8f ea b0 12 81 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%ecx\),%r9
[ 	]*[a-f0-9]+:	67 8f ea a8 12 82 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%edx\),%r10
[ 	]*[a-f0-9]+:	67 8f ea a0 12 83 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%ebx\),%r11
[ 	]*[a-f0-9]+:	67 8f ea 98 12 84 24 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%esp\),%r12
[ 	]*[a-f0-9]+:	67 8f ea 90 12 85 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%ebp\),%r13
[ 	]*[a-f0-9]+:	67 8f ea 88 12 86 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%esi\),%r14
[ 	]*[a-f0-9]+:	67 8f ea 80 12 87 fe ca 00 00 78 56 34 12[ 	]+lwpins \$0x12345678,0xcafe\(%edi\),%r15
[ 	]*[a-f0-9]+:	67 8f ca 78 12 8f fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r15d\),%eax
[ 	]*[a-f0-9]+:	67 8f ca 70 12 8e fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r14d\),%ecx
[ 	]*[a-f0-9]+:	67 8f ca 68 12 8d fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r13d\),%edx
[ 	]*[a-f0-9]+:	67 8f ca 60 12 8c 24 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r12d\),%ebx
[ 	]*[a-f0-9]+:	67 8f ca 58 12 8b fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r11d\),%esp
[ 	]*[a-f0-9]+:	67 8f ca 50 12 8a fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r10d\),%ebp
[ 	]*[a-f0-9]+:	67 8f ca 48 12 89 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r9d\),%esi
[ 	]*[a-f0-9]+:	67 8f ca 40 12 88 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r8d\),%edi
[ 	]*[a-f0-9]+:	67 8f ea 38 12 8f fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%edi\),%r8d
[ 	]*[a-f0-9]+:	67 8f ea 30 12 8e fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%esi\),%r9d
[ 	]*[a-f0-9]+:	67 8f ea 28 12 8d fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%ebp\),%r10d
[ 	]*[a-f0-9]+:	67 8f ea 20 12 8c 24 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%esp\),%r11d
[ 	]*[a-f0-9]+:	67 8f ea 18 12 8b fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%ebx\),%r12d
[ 	]*[a-f0-9]+:	67 8f ea 10 12 8a fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%edx\),%r13d
[ 	]*[a-f0-9]+:	67 8f ea 08 12 89 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%ecx\),%r14d
[ 	]*[a-f0-9]+:	67 8f ea 00 12 88 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%eax\),%r15d
[ 	]*[a-f0-9]+:	67 8f ca f8 12 8f fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r15d\),%rax
[ 	]*[a-f0-9]+:	67 8f ca f0 12 8e fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r14d\),%rcx
[ 	]*[a-f0-9]+:	67 8f ca e8 12 8d fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r13d\),%rdx
[ 	]*[a-f0-9]+:	67 8f ca e0 12 8c 24 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r12d\),%rbx
[ 	]*[a-f0-9]+:	67 8f ca d8 12 8b fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r11d\),%rsp
[ 	]*[a-f0-9]+:	67 8f ca d0 12 8a fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r10d\),%rbp
[ 	]*[a-f0-9]+:	67 8f ca c8 12 89 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r9d\),%rsi
[ 	]*[a-f0-9]+:	67 8f ca c0 12 88 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%r8d\),%rdi
[ 	]*[a-f0-9]+:	67 8f ea b8 12 88 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%eax\),%r8
[ 	]*[a-f0-9]+:	67 8f ea b0 12 89 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%ecx\),%r9
[ 	]*[a-f0-9]+:	67 8f ea a8 12 8a fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%edx\),%r10
[ 	]*[a-f0-9]+:	67 8f ea a0 12 8b fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%ebx\),%r11
[ 	]*[a-f0-9]+:	67 8f ea 98 12 8c 24 fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%esp\),%r12
[ 	]*[a-f0-9]+:	67 8f ea 90 12 8d fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%ebp\),%r13
[ 	]*[a-f0-9]+:	67 8f ea 88 12 8e fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%esi\),%r14
[ 	]*[a-f0-9]+:	67 8f ea 80 12 8f fe ca 00 00 78 56 34 12[ 	]+lwpval \$0x12345678,0xcafe\(%edi\),%r15
#pass
