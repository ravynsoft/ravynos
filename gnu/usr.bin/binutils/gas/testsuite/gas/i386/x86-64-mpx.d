#objdump: -drw
#name: x86-64 MPX

.*: +file format .*


Disassembly of section .text:

0+ <start>:
[ 	]*[a-f0-9]+:	f3 41 0f 1b 0b       	bndmk  \(%r11\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1b 08          	bndmk  \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1b 0c 25 99 03 00 00 	bndmk  0x399,%bnd1
[ 	]*[a-f0-9]+:	f3 41 0f 1b 49 03    	bndmk  0x3\(%r9\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1b 48 03       	bndmk  0x3\(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f3 42 0f 1b 0c 25 03 00 00 00 	bndmk  0x3\(,%r12,1\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1b 0c 08       	bndmk  \(%rax,%rcx,1\),%bnd1
[ 	]*[a-f0-9]+:	f3 41 0f 1b 4c 43 03 	bndmk  0x3\(%r11,%rax,2\),%bnd1
[ 	]*[a-f0-9]+:	f3 42 0f 1b 4c 0b 03 	bndmk  0x3\(%rbx,%r9,1\),%bnd1
[ 	]*[a-f0-9]+:	66 41 0f 1a 0b       	bndmov \(%r11\),%bnd1
[ 	]*[a-f0-9]+:	66 0f 1a 08          	bndmov \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	66 0f 1a 0c 25 99 03 00 00 	bndmov 0x399,%bnd1
[ 	]*[a-f0-9]+:	66 41 0f 1a 51 03    	bndmov 0x3\(%r9\),%bnd2
[ 	]*[a-f0-9]+:	66 0f 1a 50 03       	bndmov 0x3\(%rax\),%bnd2
[ 	]*[a-f0-9]+:	66 0f 1a 15 33 33 00 00 	bndmov 0x3333\(%rip\),%bnd2 ?.*
[ 	]*[a-f0-9]+:	66 42 0f 1a 04 25 03 00 00 00 	bndmov 0x3\(,%r12,1\),%bnd0
[ 	]*[a-f0-9]+:	66 0f 1a 14 10       	bndmov \(%rax,%rdx,1\),%bnd2
[ 	]*[a-f0-9]+:	66 41 0f 1a 4c 43 03 	bndmov 0x3\(%r11,%rax,2\),%bnd1
[ 	]*[a-f0-9]+:	66 42 0f 1a 4c 0b 03 	bndmov 0x3\(%rbx,%r9,1\),%bnd1
[ 	]*[a-f0-9]+:	66 0f 1a c2          	bndmov %bnd2,%bnd0
[ 	]*[a-f0-9]+:	66 41 0f 1b 0b       	bndmov %bnd1,\(%r11\)
[ 	]*[a-f0-9]+:	66 0f 1b 08          	bndmov %bnd1,\(%rax\)
[ 	]*[a-f0-9]+:	66 0f 1b 0c 25 99 03 00 00 	bndmov %bnd1,0x399
[ 	]*[a-f0-9]+:	66 41 0f 1b 51 03    	bndmov %bnd2,0x3\(%r9\)
[ 	]*[a-f0-9]+:	66 0f 1b 50 03       	bndmov %bnd2,0x3\(%rax\)
[ 	]*[a-f0-9]+:	66 0f 1b 15 33 33 00 00 	bndmov %bnd2,0x3333\(%rip\) ?.*
[ 	]*[a-f0-9]+:	66 42 0f 1b 04 25 03 00 00 00 	bndmov %bnd0,0x3\(,%r12,1\)
[ 	]*[a-f0-9]+:	66 0f 1b 14 10       	bndmov %bnd2,\(%rax,%rdx,1\)
[ 	]*[a-f0-9]+:	66 41 0f 1b 4c 43 03 	bndmov %bnd1,0x3\(%r11,%rax,2\)
[ 	]*[a-f0-9]+:	66 42 0f 1b 4c 0b 03 	bndmov %bnd1,0x3\(%rbx,%r9,1\)
[ 	]*[a-f0-9]+:	66 0f 1a d0          	bndmov %bnd0,%bnd2
[ 	]*[a-f0-9]+:	f3 41 0f 1a 0b       	bndcl  \(%r11\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1a 08          	bndcl  \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f3 41 0f 1a cb       	bndcl  %r11,%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1a c9          	bndcl  %rcx,%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1a 0c 25 99 03 00 00 	bndcl  0x399,%bnd1
[ 	]*[a-f0-9]+:	f3 41 0f 1a 51 03    	bndcl  0x3\(%r9\),%bnd2
[ 	]*[a-f0-9]+:	f3 0f 1a 50 03       	bndcl  0x3\(%rax\),%bnd2
[ 	]*[a-f0-9]+:	f3 0f 1a 15 33 33 00 00 	bndcl  0x3333\(%rip\),%bnd2 ?.*
[ 	]*[a-f0-9]+:	f3 42 0f 1a 04 25 03 00 00 00 	bndcl  0x3\(,%r12,1\),%bnd0
[ 	]*[a-f0-9]+:	f3 0f 1a 14 10       	bndcl  \(%rax,%rdx,1\),%bnd2
[ 	]*[a-f0-9]+:	f3 41 0f 1a 4c 43 03 	bndcl  0x3\(%r11,%rax,2\),%bnd1
[ 	]*[a-f0-9]+:	f3 42 0f 1a 4c 0b 03 	bndcl  0x3\(%rbx,%r9,1\),%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1a 0b       	bndcu  \(%r11\),%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1a 08          	bndcu  \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1a cb       	bndcu  %r11,%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1a c9          	bndcu  %rcx,%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1a 0c 25 99 03 00 00 	bndcu  0x399,%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1a 51 03    	bndcu  0x3\(%r9\),%bnd2
[ 	]*[a-f0-9]+:	f2 0f 1a 50 03       	bndcu  0x3\(%rax\),%bnd2
[ 	]*[a-f0-9]+:	f2 0f 1a 15 33 33 00 00 	bndcu  0x3333\(%rip\),%bnd2 ?.*
[ 	]*[a-f0-9]+:	f2 42 0f 1a 04 25 03 00 00 00 	bndcu  0x3\(,%r12,1\),%bnd0
[ 	]*[a-f0-9]+:	f2 0f 1a 14 10       	bndcu  \(%rax,%rdx,1\),%bnd2
[ 	]*[a-f0-9]+:	f2 41 0f 1a 4c 43 03 	bndcu  0x3\(%r11,%rax,2\),%bnd1
[ 	]*[a-f0-9]+:	f2 42 0f 1a 4c 0b 03 	bndcu  0x3\(%rbx,%r9,1\),%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1b 0b       	bndcn  \(%r11\),%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1b 08          	bndcn  \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1b cb       	bndcn  %r11,%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1b c9          	bndcn  %rcx,%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1b 0c 25 99 03 00 00 	bndcn  0x399,%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1b 51 03    	bndcn  0x3\(%r9\),%bnd2
[ 	]*[a-f0-9]+:	f2 0f 1b 50 03       	bndcn  0x3\(%rax\),%bnd2
[ 	]*[a-f0-9]+:	f2 0f 1b 15 33 33 00 00 	bndcn  0x3333\(%rip\),%bnd2 ?.*
[ 	]*[a-f0-9]+:	f2 42 0f 1b 04 25 03 00 00 00 	bndcn  0x3\(,%r12,1\),%bnd0
[ 	]*[a-f0-9]+:	f2 0f 1b 14 10       	bndcn  \(%rax,%rdx,1\),%bnd2
[ 	]*[a-f0-9]+:	f2 41 0f 1b 4c 43 03 	bndcn  0x3\(%r11,%rax,2\),%bnd1
[ 	]*[a-f0-9]+:	f2 42 0f 1b 4c 0b 03 	bndcn  0x3\(%rbx,%r9,1\),%bnd1
[ 	]*[a-f0-9]+:	0f 1b 44 18 03       	bndstx %bnd0,0x3\(%rax,%rbx,1\)
[ 	]*[a-f0-9]+:	0f 1b 54 13 03       	bndstx %bnd2,0x3\(%rbx,%rdx,1\)
[ 	]*[a-f0-9]+:	41 0f 1b 9c 24 99 03 00 00 	bndstx %bnd3,0x399\(%r12\)
[ 	]*[a-f0-9]+:	41 0f 1b 8b 34 12 00 00 	bndstx %bnd1,0x1234\(%r11\)
[ 	]*[a-f0-9]+:	0f 1b 93 34 12 00 00 	bndstx %bnd2,0x1234\(%rbx\)
[ 	]*[a-f0-9]+:	0f 1b 14 1d 03 00 00 00 	bndstx %bnd2,0x3\(,%rbx,1\)
[ 	]*[a-f0-9]+:	42 0f 1b 14 25 03 00 00 00 	bndstx %bnd2,0x3\(,%r12,1\)
[ 	]*[a-f0-9]+:	0f 1b 0a             	bndstx %bnd1,\(%rdx\)
[ 	]*[a-f0-9]+:	0f 1a 44 18 03       	bndldx 0x3\(%rax,%rbx,1\),%bnd0
[ 	]*[a-f0-9]+:	0f 1a 54 13 03       	bndldx 0x3\(%rbx,%rdx,1\),%bnd2
[ 	]*[a-f0-9]+:	41 0f 1a 9c 24 99 03 00 00 	bndldx 0x399\(%r12\),%bnd3
[ 	]*[a-f0-9]+:	41 0f 1a 8b 34 12 00 00 	bndldx 0x1234\(%r11\),%bnd1
[ 	]*[a-f0-9]+:	0f 1a 93 34 12 00 00 	bndldx 0x1234\(%rbx\),%bnd2
[ 	]*[a-f0-9]+:	0f 1a 14 1d 03 00 00 00 	bndldx 0x3\(,%rbx,1\),%bnd2
[ 	]*[a-f0-9]+:	42 0f 1a 14 25 03 00 00 00 	bndldx 0x3\(,%r12,1\),%bnd2
[ 	]*[a-f0-9]+:	0f 1a 0a             	bndldx \(%rdx\),%bnd1
[ 	]*[a-f0-9]+:	f2 e8 25 02 00 00    	bnd call [0-9a-f]+ <foo>
[ 	]*[a-f0-9]+:	f2 ff 10             	bnd call \*\(%rax\)
[ 	]*[a-f0-9]+:	f2 41 ff 13          	bnd call \*\(%r11\)
[ 	]*[a-f0-9]+:	f2 0f 84 17 02 00 00 	bnd je [0-9a-f]+ <foo>
[ 	]*[a-f0-9]+:	f2 e9 11 02 00 00    	bnd jmp [0-9a-f]+ <foo>
[ 	]*[a-f0-9]+:	f2 ff 21             	bnd jmp \*\(%rcx\)
[ 	]*[a-f0-9]+:	f2 41 ff 24 24       	bnd jmp \*\(%r12\)
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret
[ 	]*[a-f0-9]+:	f3 41 0f 1b 0b       	bndmk  \(%r11\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1b 08          	bndmk  \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1b 0c 25 99 03 00 00 	bndmk  0x399,%bnd1
[ 	]*[a-f0-9]+:	f3 41 0f 1b 49 03    	bndmk  0x3\(%r9\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1b 48 03       	bndmk  0x3\(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f3 42 0f 1b 0c 25 03 00 00 00 	bndmk  0x3\(,%r12,1\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1b 0c 08       	bndmk  \(%rax,%rcx,1\),%bnd1
[ 	]*[a-f0-9]+:	f3 41 0f 1b 4c 03 03 	bndmk  0x3\(%r11,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	f3 42 0f 1b 4c 0b 03 	bndmk  0x3\(%rbx,%r9,1\),%bnd1
[ 	]*[a-f0-9]+:	66 41 0f 1a 0b       	bndmov \(%r11\),%bnd1
[ 	]*[a-f0-9]+:	66 0f 1a 08          	bndmov \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	66 0f 1a 0c 25 99 03 00 00 	bndmov 0x399,%bnd1
[ 	]*[a-f0-9]+:	66 41 0f 1a 51 03    	bndmov 0x3\(%r9\),%bnd2
[ 	]*[a-f0-9]+:	66 0f 1a 50 03       	bndmov 0x3\(%rax\),%bnd2
[ 	]*[a-f0-9]+:	66 42 0f 1a 04 25 03 00 00 00 	bndmov 0x3\(,%r12,1\),%bnd0
[ 	]*[a-f0-9]+:	66 0f 1a 14 10       	bndmov \(%rax,%rdx,1\),%bnd2
[ 	]*[a-f0-9]+:	66 41 0f 1a 4c 03 03 	bndmov 0x3\(%r11,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	66 42 0f 1a 4c 0b 03 	bndmov 0x3\(%rbx,%r9,1\),%bnd1
[ 	]*[a-f0-9]+:	66 0f 1a c2          	bndmov %bnd2,%bnd0
[ 	]*[a-f0-9]+:	66 41 0f 1b 0b       	bndmov %bnd1,\(%r11\)
[ 	]*[a-f0-9]+:	66 0f 1b 08          	bndmov %bnd1,\(%rax\)
[ 	]*[a-f0-9]+:	66 0f 1b 0c 25 99 03 00 00 	bndmov %bnd1,0x399
[ 	]*[a-f0-9]+:	66 41 0f 1b 51 03    	bndmov %bnd2,0x3\(%r9\)
[ 	]*[a-f0-9]+:	66 0f 1b 50 03       	bndmov %bnd2,0x3\(%rax\)
[ 	]*[a-f0-9]+:	66 42 0f 1b 04 25 03 00 00 00 	bndmov %bnd0,0x3\(,%r12,1\)
[ 	]*[a-f0-9]+:	66 0f 1b 14 10       	bndmov %bnd2,\(%rax,%rdx,1\)
[ 	]*[a-f0-9]+:	66 41 0f 1b 4c 03 03 	bndmov %bnd1,0x3\(%r11,%rax,1\)
[ 	]*[a-f0-9]+:	66 42 0f 1b 4c 0b 03 	bndmov %bnd1,0x3\(%rbx,%r9,1\)
[ 	]*[a-f0-9]+:	66 0f 1a d0          	bndmov %bnd0,%bnd2
[ 	]*[a-f0-9]+:	f3 41 0f 1a 0b       	bndcl  \(%r11\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1a 08          	bndcl  \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f3 41 0f 1a cb       	bndcl  %r11,%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1a c9          	bndcl  %rcx,%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1a 0c 25 99 03 00 00 	bndcl  0x399,%bnd1
[ 	]*[a-f0-9]+:	f3 41 0f 1a 49 03    	bndcl  0x3\(%r9\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1a 48 03       	bndcl  0x3\(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f3 42 0f 1a 0c 25 03 00 00 00 	bndcl  0x3\(,%r12,1\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1a 0c 08       	bndcl  \(%rax,%rcx,1\),%bnd1
[ 	]*[a-f0-9]+:	f3 41 0f 1a 4c 03 03 	bndcl  0x3\(%r11,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	f3 42 0f 1a 4c 0b 03 	bndcl  0x3\(%rbx,%r9,1\),%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1a 0b       	bndcu  \(%r11\),%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1a 08          	bndcu  \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1a cb       	bndcu  %r11,%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1a c9          	bndcu  %rcx,%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1a 0c 25 99 03 00 00 	bndcu  0x399,%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1a 49 03    	bndcu  0x3\(%r9\),%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1a 48 03       	bndcu  0x3\(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f2 42 0f 1a 0c 25 03 00 00 00 	bndcu  0x3\(,%r12,1\),%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1a 0c 08       	bndcu  \(%rax,%rcx,1\),%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1a 4c 03 03 	bndcu  0x3\(%r11,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	f2 42 0f 1a 4c 0b 03 	bndcu  0x3\(%rbx,%r9,1\),%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1b 0b       	bndcn  \(%r11\),%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1b 08          	bndcn  \(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1b cb       	bndcn  %r11,%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1b c9          	bndcn  %rcx,%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1b 0c 25 99 03 00 00 	bndcn  0x399,%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1b 49 03    	bndcn  0x3\(%r9\),%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1b 48 03       	bndcn  0x3\(%rax\),%bnd1
[ 	]*[a-f0-9]+:	f2 42 0f 1b 0c 0d 03 00 00 00 	bndcn  0x3\(,%r9,1\),%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1b 0c 08       	bndcn  \(%rax,%rcx,1\),%bnd1
[ 	]*[a-f0-9]+:	f2 41 0f 1b 4c 03 03 	bndcn  0x3\(%r11,%rax,1\),%bnd1
[ 	]*[a-f0-9]+:	f2 42 0f 1b 4c 0b 03 	bndcn  0x3\(%rbx,%r9,1\),%bnd1
[ 	]*[a-f0-9]+:	0f 1b 44 18 03       	bndstx %bnd0,0x3\(%rax,%rbx,1\)
[ 	]*[a-f0-9]+:	0f 1b 54 13 03       	bndstx %bnd2,0x3\(%rbx,%rdx,1\)
[ 	]*[a-f0-9]+:	41 0f 1b 9c 24 99 03 00 00 	bndstx %bnd3,0x399\(%r12\)
[ 	]*[a-f0-9]+:	41 0f 1b 8b 34 12 00 00 	bndstx %bnd1,0x1234\(%r11\)
[ 	]*[a-f0-9]+:	0f 1b 93 34 12 00 00 	bndstx %bnd2,0x1234\(%rbx\)
[ 	]*[a-f0-9]+:	0f 1b 14 1d 03 00 00 00 	bndstx %bnd2,0x3\(,%rbx,1\)
[ 	]*[a-f0-9]+:	42 0f 1b 14 25 03 00 00 00 	bndstx %bnd2,0x3\(,%r12,1\)
[ 	]*[a-f0-9]+:	0f 1b 0a             	bndstx %bnd1,\(%rdx\)
[ 	]*[a-f0-9]+:	0f 1a 44 18 03       	bndldx 0x3\(%rax,%rbx,1\),%bnd0
[ 	]*[a-f0-9]+:	0f 1a 54 13 03       	bndldx 0x3\(%rbx,%rdx,1\),%bnd2
[ 	]*[a-f0-9]+:	41 0f 1a 9c 24 99 03 00 00 	bndldx 0x399\(%r12\),%bnd3
[ 	]*[a-f0-9]+:	41 0f 1a 8b 34 12 00 00 	bndldx 0x1234\(%r11\),%bnd1
[ 	]*[a-f0-9]+:	0f 1a 93 34 12 00 00 	bndldx 0x1234\(%rbx\),%bnd2
[ 	]*[a-f0-9]+:	0f 1a 14 1d 03 00 00 00 	bndldx 0x3\(,%rbx,1\),%bnd2
[ 	]*[a-f0-9]+:	42 0f 1a 14 25 03 00 00 00 	bndldx 0x3\(,%r12,1\),%bnd2
[ 	]*[a-f0-9]+:	0f 1a 0a             	bndldx \(%rdx\),%bnd1
[ 	]*[a-f0-9]+:	f2 e8 16 00 00 00    	bnd call [0-9a-f]+ <foo>
[ 	]*[a-f0-9]+:	f2 ff d0             	bnd call \*%rax
[ 	]*[a-f0-9]+:	f2 41 ff d3          	bnd call \*%r11
[ 	]*[a-f0-9]+:	f2 74 0c             	bnd je [0-9a-f]+ <foo>
[ 	]*[a-f0-9]+:	f2 eb 09             	bnd jmp [0-9a-f]+ <foo>
[ 	]*[a-f0-9]+:	f2 ff e1             	bnd jmp \*%rcx
[ 	]*[a-f0-9]+:	f2 41 ff e4          	bnd jmp \*%r12
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret

[a-f0-9]+ <foo>:
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret

[a-f0-9]+ <bad>:
[ 	]*[a-f0-9]+:	0f 1a 30             	bndldx \(%rax\),\(bad\)
[ 	]*[a-f0-9]+:	66 0f 1a c4          	bndmov \(bad\),%bnd0
[ 	]*[a-f0-9]+:	66 41 0f 1a c0       	bndmov \(bad\),%bnd0
[ 	]*[a-f0-9]+:	66 44 0f 1a c0       	bndmov %bnd0,\(bad\)
[ 	]*[a-f0-9]+:	f3 0f 1b 05 90 90 90 90 	bndmk  \(bad\),%bnd0
#pass
