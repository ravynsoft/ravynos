#as: -I${srcdir}/$subdir
#objdump: -drw -Mi8086
#name: i386 MPX (16-bit)

.*: +file format .*


Disassembly of section .text:

0+ <start>:
[ 	]*[a-f0-9]+:	67 f3 0f 1b 08       	bndmk  \(%eax\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1b 0d 99 03 00 00 	addr32 bndmk 0x399,%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1b 4a 03    	bndmk  0x3\(%edx\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1b 0c 08    	bndmk  \(%eax,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1b 0c 0d 00 00 00 00 	bndmk  0x0\(,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1b 4c 01 03 	bndmk  0x3\(%ecx,%eax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1a 08       	bndmov \(%eax\),%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1a 0d 99 03 00 00 	addr32 bndmov 0x399,%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1a 52 03    	bndmov 0x3\(%edx\),%bnd2
[ 	]*[a-f0-9]+:	67 66 0f 1a 14 10    	bndmov \(%eax,%edx,1\),%bnd2
[ 	]*[a-f0-9]+:	67 66 0f 1a 14 05 00 00 00 00 	bndmov 0x0\(,%eax,1\),%bnd2
[ 	]*[a-f0-9]+:	67 66 0f 1a 4c 01 03 	bndmov 0x3\(%ecx,%eax,1\),%bnd1
[ 	]*[a-f0-9]+:	66 0f 1a c2          	bndmov %bnd2,%bnd0
[ 	]*[a-f0-9]+:	67 66 0f 1b 08       	bndmov %bnd1,\(%eax\)
[ 	]*[a-f0-9]+:	67 66 0f 1b 0d 99 03 00 00 	addr32 bndmov %bnd1,0x399
[ 	]*[a-f0-9]+:	67 66 0f 1b 52 03    	bndmov %bnd2,0x3\(%edx\)
[ 	]*[a-f0-9]+:	67 66 0f 1b 14 10    	bndmov %bnd2,\(%eax,%edx,1\)
[ 	]*[a-f0-9]+:	67 66 0f 1b 14 05 00 00 00 00 	bndmov %bnd2,0x0\(,%eax,1\)
[ 	]*[a-f0-9]+:	67 66 0f 1b 4c 01 03 	bndmov %bnd1,0x3\(%ecx,%eax,1\)
[ 	]*[a-f0-9]+:	66 0f 1a d0          	bndmov %bnd0,%bnd2
[ 	]*[a-f0-9]+:	67 f3 0f 1a 09       	bndcl  \(%ecx\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1a c9          	bndcl  %ecx,%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 0d 99 03 00 00 	addr32 bndcl 0x399,%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 4a 03    	bndcl  0x3\(%edx\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 0c 08    	bndcl  \(%eax,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 0c 0d 00 00 00 00 	bndcl  0x0\(,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 4c 01 03 	bndcl  0x3\(%ecx,%eax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 09       	bndcu  \(%ecx\),%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1a c9          	bndcu  %ecx,%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 0d 99 03 00 00 	addr32 bndcu 0x399,%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 4a 03    	bndcu  0x3\(%edx\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 0c 08    	bndcu  \(%eax,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 0c 0d 00 00 00 00 	bndcu  0x0\(,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 4c 01 03 	bndcu  0x3\(%ecx,%eax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 09       	bndcn  \(%ecx\),%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1b c9          	bndcn  %ecx,%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 0d 99 03 00 00 	addr32 bndcn 0x399,%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 4a 03    	bndcn  0x3\(%edx\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 0c 08    	bndcn  \(%eax,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 0c 0d 00 00 00 00 	bndcn  0x0\(,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 4c 01 03 	bndcn  0x3\(%ecx,%eax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 0f 1b 44 18 03    	bndstx %bnd0,0x3\(%eax,%ebx,1\)
[ 	]*[a-f0-9]+:	67 0f 1b 54 13 03    	bndstx %bnd2,0x3\(%ebx,%edx,1\)
[ 	]*[a-f0-9]+:	67 0f 1b 14 15 03 00 00 00 	bndstx %bnd2,0x3\(,%edx,1\)
[ 	]*[a-f0-9]+:	67 0f 1b 9a 99 03 00 00 	bndstx %bnd3,0x399\(%edx\)
[ 	]*[a-f0-9]+:	67 0f 1b 93 34 12 00 00 	bndstx %bnd2,0x1234\(%ebx\)
[ 	]*[a-f0-9]+:	67 0f 1b 53 03       	bndstx %bnd2,0x3\(%ebx\)
[ 	]*[a-f0-9]+:	67 0f 1b 0a          	bndstx %bnd1,\(%edx\)
[ 	]*[a-f0-9]+:	67 0f 1a 44 18 03    	bndldx 0x3\(%eax,%ebx,1\),%bnd0
[ 	]*[a-f0-9]+:	67 0f 1a 54 13 03    	bndldx 0x3\(%ebx,%edx,1\),%bnd2
[ 	]*[a-f0-9]+:	67 0f 1a 14 15 03 00 00 00 	bndldx 0x3\(,%edx,1\),%bnd2
[ 	]*[a-f0-9]+:	67 0f 1a 9a 99 03 00 00 	bndldx 0x399\(%edx\),%bnd3
[ 	]*[a-f0-9]+:	67 0f 1a 93 34 12 00 00 	bndldx 0x1234\(%ebx\),%bnd2
[ 	]*[a-f0-9]+:	67 0f 1a 53 03       	bndldx 0x3\(%ebx\),%bnd2
[ 	]*[a-f0-9]+:	67 0f 1a 0a          	bndldx \(%edx\),%bnd1
[ 	]*[a-f0-9]+:	f2 e8 91 01          	bnd call [a-f0-9]+ <foo>
[ 	]*[a-f0-9]+:	67 f2 ff 10          	bnd call \*\(%eax\)
[ 	]*[a-f0-9]+:	f2 0f 84 88 01       	bnd je [a-f0-9]+ <foo>
[ 	]*[a-f0-9]+:	f2 e9 84 01          	bnd jmp [a-f0-9]+ <foo>
[ 	]*[a-f0-9]+:	67 f2 ff 21          	bnd jmp \*\(%ecx\)
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret
[ 	]*[a-f0-9]+:	67 f3 0f 1b 08       	bndmk  \(%eax\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1b 0d 99 03 00 00 	addr32 bndmk 0x399,%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1b 49 03    	bndmk  0x3\(%ecx\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1b 0c 08    	bndmk  \(%eax,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1b 0c 0d 00 00 00 00 	bndmk  0x0\(,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1b 4c 02 03 	bndmk  0x3\(%edx,%eax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1a 08       	bndmov \(%eax\),%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1a 0d 99 03 00 00 	addr32 bndmov 0x399,%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1a 49 03    	bndmov 0x3\(%ecx\),%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1a 0c 08    	bndmov \(%eax,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1a 0c 0d 00 00 00 00 	bndmov 0x0\(,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 66 0f 1a 4c 02 03 	bndmov 0x3\(%edx,%eax,1\),%bnd1
[ 	]*[a-f0-9]+:	66 0f 1a c1          	bndmov %bnd1,%bnd0
[ 	]*[a-f0-9]+:	67 66 0f 1b 08       	bndmov %bnd1,\(%eax\)
[ 	]*[a-f0-9]+:	67 66 0f 1b 0d 99 03 00 00 	addr32 bndmov %bnd1,0x399
[ 	]*[a-f0-9]+:	67 66 0f 1b 49 03    	bndmov %bnd1,0x3\(%ecx\)
[ 	]*[a-f0-9]+:	67 66 0f 1b 0c 08    	bndmov %bnd1,\(%eax,%ecx,1\)
[ 	]*[a-f0-9]+:	67 66 0f 1b 0c 0d 00 00 00 00 	bndmov %bnd1,0x0\(,%ecx,1\)
[ 	]*[a-f0-9]+:	67 66 0f 1b 4c 02 03 	bndmov %bnd1,0x3\(%edx,%eax,1\)
[ 	]*[a-f0-9]+:	66 0f 1a c8          	bndmov %bnd0,%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 08       	bndcl  \(%eax\),%bnd1
[ 	]*[a-f0-9]+:	f3 0f 1a c9          	bndcl  %ecx,%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 0d 99 03 00 00 	addr32 bndcl 0x399,%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 49 03    	bndcl  0x3\(%ecx\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 0c 08    	bndcl  \(%eax,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 0c 0d 00 00 00 00 	bndcl  0x0\(,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f3 0f 1a 4c 02 03 	bndcl  0x3\(%edx,%eax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 08       	bndcu  \(%eax\),%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1a c9          	bndcu  %ecx,%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 0d 99 03 00 00 	addr32 bndcu 0x399,%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 49 03    	bndcu  0x3\(%ecx\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 0c 08    	bndcu  \(%eax,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 0c 0d 00 00 00 00 	bndcu  0x0\(,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1a 4c 02 03 	bndcu  0x3\(%edx,%eax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 08       	bndcn  \(%eax\),%bnd1
[ 	]*[a-f0-9]+:	f2 0f 1b c9          	bndcn  %ecx,%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 0d 99 03 00 00 	addr32 bndcn 0x399,%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 49 03    	bndcn  0x3\(%ecx\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 0c 08    	bndcn  \(%eax,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 0c 0d 00 00 00 00 	bndcn  0x0\(,%ecx,1\),%bnd1
[ 	]*[a-f0-9]+:	67 f2 0f 1b 4c 02 03 	bndcn  0x3\(%edx,%eax,1\),%bnd1
[ 	]*[a-f0-9]+:	67 0f 1b 44 18 03    	bndstx %bnd0,0x3\(%eax,%ebx,1\)
[ 	]*[a-f0-9]+:	67 0f 1b 54 13 03    	bndstx %bnd2,0x3\(%ebx,%edx,1\)
[ 	]*[a-f0-9]+:	67 0f 1b 14 0d 00 00 00 00 	bndstx %bnd2,0x0\(,%ecx,1\)
[ 	]*[a-f0-9]+:	67 0f 1b 9a 99 03 00 00 	bndstx %bnd3,0x399\(%edx\)
[ 	]*[a-f0-9]+:	67 0f 1b 14 1d 03 00 00 00 	bndstx %bnd2,0x3\(,%ebx,1\)
[ 	]*[a-f0-9]+:	67 0f 1b 0a          	bndstx %bnd1,\(%edx\)
[ 	]*[a-f0-9]+:	67 0f 1a 44 18 03    	bndldx 0x3\(%eax,%ebx,1\),%bnd0
[ 	]*[a-f0-9]+:	67 0f 1a 54 13 03    	bndldx 0x3\(%ebx,%edx,1\),%bnd2
[ 	]*[a-f0-9]+:	67 0f 1a 14 0d 00 00 00 00 	bndldx 0x0\(,%ecx,1\),%bnd2
[ 	]*[a-f0-9]+:	67 0f 1a 9a 99 03 00 00 	bndldx 0x399\(%edx\),%bnd3
[ 	]*[a-f0-9]+:	67 0f 1a 14 1d 03 00 00 00 	bndldx 0x3\(,%ebx,1\),%bnd2
[ 	]*[a-f0-9]+:	67 0f 1a 0a          	bndldx \(%edx\),%bnd1
[ 	]*[a-f0-9]+:	f2 e8 10 00          	bnd call [a-f0-9]+ <foo>
[ 	]*[a-f0-9]+:	66 f2 ff d0          	bnd calll? \*%eax
[ 	]*[a-f0-9]+:	f2 74 09             	bnd je [a-f0-9]+ <foo>
[ 	]*[a-f0-9]+:	f2 eb 06             	bnd jmp [a-f0-9]+ <foo>
[ 	]*[a-f0-9]+:	66 f2 ff e1          	bnd jmpl? \*%ecx
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret

[a-f0-9]+ <foo>:
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret

[a-f0-9]+ <bad>:
#...
[a-f0-9]+ <bad16>:
[ 	]*[a-f0-9]+:	f3 0f 1b 00          	bndmk  \(bad\),%bnd0
[ 	]*[a-f0-9]+:	66 0f 1a 00          	bndmov \(bad\),%bnd0
[ 	]*[a-f0-9]+:	f3 0f 1a 00          	bndcl  \(bad\),%bnd0
[ 	]*[a-f0-9]+:	f2 0f 1b 00          	bndcn  \(bad\),%bnd0
[ 	]*[a-f0-9]+:	f2 0f 1a 00          	bndcu  \(bad\),%bnd0
[ 	]*[a-f0-9]+:	0f 1b 00             	bndstx %bnd0,\(bad\)
[ 	]*[a-f0-9]+:	0f 1a 00             	bndldx \(bad\),%bnd0
#pass
