#name: i386 size 5 (text)
#source: size-5.s
#objdump: -dtwr

.*: +file format .*

SYMBOL TABLE:
0*0 l +\.text	0*43 size
0*1000 l +\*ABS\*	0*0 val
0*0 +\*UND\*	0*0 ext

Disassembly of section .text:

0+ <size>:
[ 	]*[a-f0-9]+:	b8 43 00 00 00       	mov    \$0x43,%eax
[ 	]*[a-f0-9]+:	b8 43 10 00 00       	mov    \$0x1043,%eax
[ 	]*[a-f0-9]+:	b9 bd ff ff ff       	mov    \$0xffffffbd,%ecx
[ 	]*[a-f0-9]+:	b9 bd ff ff ff       	mov    \$0xffffffbd,%ecx
[ 	]*[a-f0-9]+:	ba bd 00 00 00       	mov    \$0xbd,%edx
[ 	]*[a-f0-9]+:	ba bd 0f 00 00       	mov    \$0xfbd,%edx
[ 	]*[a-f0-9]+:	8d 05 43 00 00 00    	lea    0x43,%eax
[ 	]*[a-f0-9]+:	8d 05 43 10 00 00    	lea    0x1043,%eax
[ 	]*[a-f0-9]+:	8d 0d bd ff ff ff    	lea    0xffffffbd,%ecx
[ 	]*[a-f0-9]+:	8d 0d bd ff ff ff    	lea    0xffffffbd,%ecx
[ 	]*[a-f0-9]+:	8d 15 bd 00 00 00    	lea    0xbd,%edx
[ 	]*[a-f0-9]+:	8d 15 bd 0f 00 00    	lea    0xfbd,%edx
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
