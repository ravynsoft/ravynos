#objdump: -dw
#name: x86-64 RTM insns

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	c6 f8 08             	xabort \$0x8
[ 	]*[a-f0-9]+:	c7 f8 fa ff ff ff    	xbegin 3 <foo\+0x3>
[ 	]*[a-f0-9]+:	c7 f8 00 00 00 00    	xbegin f <foo\+0xf>
[ 	]*[a-f0-9]+:	0f 01 d5             	xend
[ 	]*[a-f0-9]+:	c6 f8 08             	xabort \$0x8
[ 	]*[a-f0-9]+:	c7 f8 fa ff ff ff    	xbegin 15 <foo\+0x15>
[ 	]*[a-f0-9]+:	c7 f8 00 00 00 00    	xbegin 21 <foo\+0x21>
[ 	]*[a-f0-9]+:	0f 01 d5             	xend
[ 	]*[a-f0-9]+:	0f 01 d6             	xtest
#pass
