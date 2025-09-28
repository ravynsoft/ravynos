#objdump: -drw
#name: x86-64 MPX addr32 tests

.*: +file format .*

Disassembly of section .text:

0000000000000000 <.text>:
[ 	]*[a-f0-9]+:	67 f3 0f 1b 08       	addr32 bndmk \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1b 4c 19 03 	addr32 bndmk 0x3\(%rcx,%rbx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 66 41 0f 1a 08    	addr32 bndmov \(%r8\),%bnd1
[ 	]*[a-f0-9]+:	67 66 41 0f 1a 4c 11 03 	addr32 bndmov 0x3\(%r9,%rdx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1b 08       	addr32 bndmov %bnd1,\(%rax\)
[ 	]*[a-f0-9]+:	67 66 0f 1b 4c 01 03 	addr32 bndmov %bnd1,0x3\(%rcx,%rax,1\)
[ 	]*[a-f0-9]+:	67 f3 0f 1a 09       	addr32 bndcl \(%rcx\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 4c 01 03 	addr32 bndcl 0x3\(%rcx,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 09       	addr32 bndcu \(%rcx\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 4c 01 03 	addr32 bndcu 0x3\(%rcx,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 09       	addr32 bndcn \(%rcx\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 4c 01 03 	addr32 bndcn 0x3\(%rcx,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 0f 1b 44 18 03    	addr32 bndstx %bnd0,0x3\(%rax,%rbx,1\)
[ 	]*[a-f0-9]+:	67 0f 1b 53 03       	addr32 bndstx %bnd2,0x3\(%rbx\)
[ 	]*[a-f0-9]+:	67 0f 1a 44 18 03    	addr32 bndldx 0x3\(%rax,%rbx,1\),%bnd0
[ 	]*[a-f0-9]+:	67 0f 1a 53 03       	addr32 bndldx 0x3\(%rbx\),%bnd2
[ 	]*[a-f0-9]+:	67 f3 0f 1b 08       	addr32 bndmk \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1b 4c 02 03 	addr32 bndmk 0x3\(%rdx,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1a 08       	addr32 bndmov \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1a 4c 02 03 	addr32 bndmov 0x3\(%rdx,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1b 08       	addr32 bndmov %bnd1,\(%rax\)
[ 	]*[a-f0-9]+:	67 66 0f 1b 4c 02 03 	addr32 bndmov %bnd1,0x3\(%rdx,%rax,1\)
[ 	]*[a-f0-9]+:	67 f3 0f 1a 08       	addr32 bndcl \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 4c 02 03 	addr32 bndcl 0x3\(%rdx,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 08       	addr32 bndcu \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 4c 02 03 	addr32 bndcu 0x3\(%rdx,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 08       	addr32 bndcn \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 4c 02 03 	addr32 bndcn 0x3\(%rdx,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 0f 1b 44 18 03    	addr32 bndstx %bnd0,0x3\(%rax,%rbx,1\)
[ 	]*[a-f0-9]+:	67 0f 1b 14 1d 03 00 00 00 	addr32 bndstx %bnd2,0x3\(,%rbx,1\)
[ 	]*[a-f0-9]+:	67 0f 1a 44 18 03    	addr32 bndldx 0x3\(%rax,%rbx,1\),%bnd0
[ 	]*[a-f0-9]+:	67 0f 1a 14 1d 03 00 00 00 	addr32 bndldx 0x3\(,%rbx,1\),%bnd2
#pass
